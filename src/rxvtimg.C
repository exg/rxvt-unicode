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

  img->render_pixbuf (pb, 0, 0, img->w, img->h, 0, 0);

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
  Picture src = XRenderCreatePicture (dpy, pm , format, CPRepeat, &pa);
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

bool
rxvt_img::render_pixbuf (GdkPixbuf *pixbuf, int src_x, int src_y, int width, int height, int dst_x, int dst_y)
{
  Display *dpy = s->display->dpy;

  if (s->visual->c_class != TrueColor)
    return false;

  uint32_t red_mask, green_mask, blue_mask, alpha_mask;

  red_mask   = (uint32_t)format->direct.redMask   << format->direct.red;
  green_mask = (uint32_t)format->direct.greenMask << format->direct.green;
  blue_mask  = (uint32_t)format->direct.blueMask  << format->direct.blue;
  alpha_mask = (uint32_t)format->direct.alphaMask << format->direct.alpha;

  int width_r = ecb_popcount32 (red_mask);
  int width_g = ecb_popcount32 (green_mask);
  int width_b = ecb_popcount32 (blue_mask);
  int width_a = ecb_popcount32 (alpha_mask);

  if (width_r > 8 || width_g > 8 || width_b > 8 || width_a > 8)
    return false;

  int sh_r = ecb_ctz32 (red_mask);
  int sh_g = ecb_ctz32 (green_mask);
  int sh_b = ecb_ctz32 (blue_mask);
  int sh_a = ecb_ctz32 (alpha_mask);

  if (width > 32767 || height > 32767)
    return false;

  XImage *ximage = XCreateImage (dpy, s->visual, format->depth, ZPixmap, 0, 0,
                                 width, height, 32, 0);
  if (!ximage)
    return false;

  if (height > INT_MAX / ximage->bytes_per_line
      || !(ximage->data = (char *)malloc (height * ximage->bytes_per_line)))
    {
      XDestroyImage (ximage);
      return false;
    }

  GC gc = XCreateGC (dpy, pm, 0, 0);

  ximage->byte_order = ecb_big_endian () ? MSBFirst : LSBFirst;

  int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  int channels = gdk_pixbuf_get_n_channels (pixbuf);
  unsigned char *row = gdk_pixbuf_get_pixels (pixbuf) + src_y * rowstride + src_x * channels;
  char *line = ximage->data;

  rgba c (0, 0, 0);

  if (channels == 4 && alpha_mask == 0)
    {
      //pix_colors[Color_bg].get (c);
      //TODO
      c.r = 0xffff; c.g = 0xc0c0; c.b = 0xcbcb;//D
      c.r >>= 8;
      c.g >>= 8;
      c.b >>= 8;
    }

  for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
        {
          unsigned char *pixel = row + x * channels;
          uint32_t value;
          unsigned char r, g, b, a;

          if (channels == 4)
            {
              a = pixel[3];
              r = (pixel[0] * a + c.r * (0xff - a)) / 0xff;
              g = (pixel[1] * a + c.g * (0xff - a)) / 0xff;
              b = (pixel[2] * a + c.b * (0xff - a)) / 0xff;
            }
          else
            {
              a = 0xff;
              r = pixel[0];
              g = pixel[1];
              b = pixel[2];
            }

          value  = ((r >> (8 - width_r)) << sh_r)
                 | ((g >> (8 - width_g)) << sh_g)
                 | ((b >> (8 - width_b)) << sh_b)
                 | ((a >> (8 - width_a)) << sh_a);

          if (ximage->bits_per_pixel == 32)
            ((uint32_t *)line)[x] = value;
          else
            XPutPixel (ximage, x, y, value);
        }

      row += rowstride;
      line += ximage->bytes_per_line;
    }

  XPutImage (dpy, pm, gc, ximage, 0, 0, dst_x, dst_y, width, height);
  XDestroyImage (ximage);
  XFreeGC (dpy, gc);

  return true;
}

rxvt_img *
rxvt_img::clone ()
{
  rxvt_img *img = new rxvt_img (s, format, w, h);

  GC gc = XCreateGC (s->display->dpy, pm, 0, 0);
  XCopyArea (s->display->dpy, pm, img->pm, gc, 0, 0, w, h, 0, 0);
  XFreeGC (s->display->dpy, gc);

  return img;
}

rxvt_img *
rxvt_img::sub_rect (int x, int y, int width, int height, int repeat)
{
  rxvt_img *img = new rxvt_img (s, format, width, height);

  Display *dpy = s->display->dpy;
  XRenderPictureAttributes pa;
  pa.repeat = repeat;
  Picture src = XRenderCreatePicture (dpy,      pm,      format, CPRepeat, &pa);
  Picture dst = XRenderCreatePicture (dpy, img->pm, img->format,        0,   0);

  XRenderComposite (dpy, PictOpSrc, src, None, dst, x, y, 0, 0, 0, 0, width, height);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

rxvt_img *
rxvt_img::transform (int new_width, int new_height, double matrix[9], int repeat)
{
  rxvt_img *img = new rxvt_img (s, format, new_width, new_height);

  Display *dpy = s->display->dpy;
  XRenderPictureAttributes pa;
  pa.repeat = repeat;
  Picture src = XRenderCreatePicture (dpy,      pm,      format, CPRepeat, &pa);
  Picture dst = XRenderCreatePicture (dpy, img->pm, img->format,        0,   0);

  XTransform xfrm;

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      xfrm.matrix [i][j] = XDoubleToFixed (matrix [i * 3 + j]);

  XRenderSetPictureFilter (dpy, src, "good", 0, 0);
  XRenderSetPictureTransform (dpy, src, &xfrm);
  XRenderComposite (dpy, PictOpSrc, src, None, dst, 0, 0, 0, 0, 0, 0, new_width, new_height);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

rxvt_img *
rxvt_img::scale (int new_width, int new_height)
{
  double matrix[9] = {
    w  / (double)new_width, 0, 0,
    0, h / (double)new_height, 0,
    0,                      0, 1
  };

  return transform (new_width, new_height, matrix);
}

rxvt_img *
rxvt_img::rotate (int new_width, int new_height, int x, int y, double phi, int repeat)
{
  double s = sin (phi);
  double c = cos (phi);

  double matrix[9] = {
    c, -s, -c * x + s * y + x,
    s,  c, -s * x - c * y + y,
    0,  0,                  1
  };

  return transform (new_width, new_height, matrix, repeat);
}

rxvt_img *
rxvt_img::convert_to (XRenderPictFormat *new_format)
{
  rxvt_img *img = new rxvt_img (s, new_format, w, h);

  Display *dpy = s->display->dpy;
  Picture src = XRenderCreatePicture (dpy,      pm,     format, 0, 0);
  Picture dst = XRenderCreatePicture (dpy, img->pm, new_format, 0, 0);

  XRenderComposite (dpy, PictOpSrc, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

#endif

