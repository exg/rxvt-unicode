#include <math.h>
#include "../config.h"
#include "rxvt.h"

#if HAVE_IMG

#define float_to_component(d) ((d) * 65535.99)

rxvt_img::rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int width, int height)
: s(screen), w(width), h(height), format(format), shared(false)
{
  pm = XCreatePixmap (s->display->dpy, s->display->root, w, h, format->depth);
}

rxvt_img::rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int width, int height, Pixmap pixmap)
: s(screen), pm(pixmap), w(width), h(height), format(format), shared(false)
{
}

rxvt_img *
rxvt_img::new_from_file (rxvt_screen *s, const char *filename)
{
  GError *err = 0;
  GdkPixbuf *pb = gdk_pixbuf_new_from_file (filename, &err);

  if (!pb)
    rxvt_fatal ("rxvt_img::new_from_file: %s\n", err->message);

  rxvt_img *img = new rxvt_img (
     s,
     XRenderFindStandardFormat (s->display->dpy, gdk_pixbuf_get_has_alpha (pb) ? PictStandardARGB32 : PictStandardRGB24),
     gdk_pixbuf_get_width (pb),
     gdk_pixbuf_get_height (pb)
  );

  img->render (pb, 0, 0, img->w, img->h, 0, 0);

  return img;
}

rxvt_img::~rxvt_img ()
{
  if (!shared)
    XFreePixmap (s->display->dpy, pm);
}

void
rxvt_img::unshare ()
{
  if (!shared)
    return;

  rxvt_img *img = clone ();

  ::swap (pm    , img->pm);
  ::swap (shared, img->shared);

  delete img;
}

void
rxvt_img::fill (const rxvt_color &c)
{
  XGCValues gcv;
  gcv.foreground = c;
  GC gc = XCreateGC (s->display->dpy, pm, GCForeground, &gcv);
  XFillRectangle (s->display->dpy, pm, gc, 0, 0, w, h);
  XFreeGC (s->display->dpy, gc);
}

static void
get_gaussian_kernel (int radius, int width, double *kernel, XFixed *params)
{
  double sigma = radius / 2.0;
  double scale = sqrt (2.0 * M_PI) * sigma;
  double sum = 0.0;

  for (int i = 0; i < width; i++)
    {
      double x = i - width / 2;
      kernel[i] = exp (-(x * x) / (2.0 * sigma * sigma)) / scale;
      sum += kernel[i];
    }

  params[0] = XDoubleToFixed (width);
  params[1] = XDoubleToFixed (1);

  for (int i = 0; i < width; i++)
    params[i+2] = XDoubleToFixed (kernel[i] / sum);
}

void
rxvt_img::blur (int rh, int rv)
{
  if (!(s->display->flags & DISPLAY_HAS_RENDER_CONV))
    return;

  Display *dpy = s->display->dpy;
  int size = max (rh, rv) * 2 + 1;
  double *kernel = (double *)malloc (size * sizeof (double));
  XFixed *params = (XFixed *)malloc ((size + 2) * sizeof (XFixed));

  XRenderPictureAttributes pa;

  pa.repeat = RepeatPad;
  Picture src = XRenderCreatePicture (dpy, pm, format, CPRepeat, &pa);
  Pixmap tmp = XCreatePixmap (dpy, pm, w, h, format->depth);
  Picture dst = XRenderCreatePicture (dpy, tmp, format, CPRepeat, &pa);
  XFreePixmap (dpy, tmp);

  if (kernel && params)
    {
      size = rh * 2 + 1;
      get_gaussian_kernel (rh, size, kernel, params);

      XRenderSetPictureFilter (dpy, src, FilterConvolution, params, size+2);
      XRenderComposite (dpy,
                        PictOpSrc,
                        src,
                        None,
                        dst,
                        0, 0,
                        0, 0,
                        0, 0,
                        w, h);

      ::swap (src, dst);

      size = rv * 2 + 1;
      get_gaussian_kernel (rv, size, kernel, params);
      ::swap (params[0], params[1]);

      XRenderSetPictureFilter (dpy, src, FilterConvolution, params, size+2);
      XRenderComposite (dpy,
                        PictOpSrc,
                        src,
                        None,
                        dst,
                        0, 0,
                        0, 0,
                        0, 0,
                        w, h);
    }

  free (kernel);
  free (params);
  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);
}

static Picture
create_xrender_mask (Display *dpy, Drawable drawable, Bool argb)
{
  Pixmap pixmap = XCreatePixmap (dpy, drawable, 1, 1, argb ? 32 : 8);

  XRenderPictFormat *format = XRenderFindStandardFormat (dpy, argb ? PictStandardARGB32 : PictStandardA8);
  XRenderPictureAttributes pa;
  pa.repeat = True;
  Picture mask = XRenderCreatePicture (dpy, pixmap, format, CPRepeat, &pa);

  XFreePixmap (dpy, pixmap);

  return mask;
}

void
rxvt_img::brightness (double r, double g, double b, double a)
{
  Display *dpy = s->display->dpy;
  Picture src = create_xrender_mask (dpy, pm, True);
  Picture dst = XRenderCreatePicture (dpy, pm, format, 0, 0);

  XRenderColor mask_c;
  mask_c.red   = float_to_component (r);
  mask_c.green = float_to_component (g);
  mask_c.blue  = float_to_component (b);
  mask_c.alpha = float_to_component (a);
  XRenderFillRectangle (dpy, PictOpSrc, src, &mask_c, 0, 0, 1, 1);

  XRenderComposite (dpy, PictOpAdd, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);
}

void
rxvt_img::contrast (double r, double g, double b, double a)
{
  if (!(s->display->flags & DISPLAY_HAS_RENDER_MUL))
    return;

  Display *dpy = s->display->dpy;
  Picture src = create_xrender_mask (dpy, pm, True);
  Picture dst = XRenderCreatePicture (dpy, pm, format, 0, 0);

  XRenderColor mask_c;
  mask_c.red   = float_to_component (r);
  mask_c.green = float_to_component (g);
  mask_c.blue  = float_to_component (b);
  mask_c.alpha = float_to_component (a);
  XRenderFillRectangle (dpy, PictOpSrc, src, &mask_c, 0, 0, 1, 1);

  XRenderComposite (dpy, PictOpMultiply, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);
}

void
rxvt_img::render (GdkPixbuf *pixbuf, int src_x, int src_y, int width, int height, int dst_x, int dst_y)
{
  //TODO
}

rxvt_img *
rxvt_img::clone ()
{
  GC gc = XCreateGC (s->display->dpy, pm, 0, 0);
  Pixmap pm2 = XCreatePixmap (s->display->dpy, pm, w, h, format->depth);
  XCopyArea (s->display->dpy, pm, pm2, gc, 0, 0, w, h, 0, 0);
  XFreeGC (s->display->dpy, gc);
  return new rxvt_img (s, format, w, h, pm2);
}

rxvt_img *
rxvt_img::transform (int new_width, int new_height, int repeat, double matrix[9])
{
  //TODO
}

rxvt_img *
rxvt_img::scale (int new_width, int new_height)
{
  double matrix[9] = {
    new_width  / (double)w, 0, 0,
    0, new_height / (double)h, 0,
    0,                      0, 1
  };

  return transform (new_width, new_height, RepeatNormal, matrix);
}

rxvt_img *
rxvt_img::convert_to (XRenderPictFormat *new_format)
{
  rxvt_img *img = new rxvt_img (s, new_format, w, h);

  Display *dpy = s->display->dpy;
  Picture src = XRenderCreatePicture (dpy, pm, format, 0, 0);
  Picture dst = XRenderCreatePicture (dpy, img->pm, new_format, 0, 0);

  XRenderComposite (dpy, PictOpSrc, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

#endif

