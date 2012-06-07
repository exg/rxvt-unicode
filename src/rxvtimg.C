#include <math.h>
#include "../config.h"
#include "rxvt.h"

#if HAVE_IMG

rxvt_img::rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int x, int y, int width, int height, int repeat)
: s(screen), x(x), y(y), w(width), h(height), format(format), repeat(repeat),
  pm(0), ref(0)
{
}

rxvt_img::rxvt_img (const rxvt_img &img)
: s(img.s), x(img.x), y(img.y), w(img.w), h(img.h), format(img.format), repeat(img.repeat), pm(img.pm), ref(img.ref)
{
  ++ref->cnt;
}

#if 0
rxvt_img::rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int width, int height, Pixmap pixmap)
: s(screen), x(0), y(0), w(width), h(height), format(format), repeat(RepeatNormal), shared(false), pm(pixmap)
{
}
#endif

rxvt_img *
rxvt_img::new_from_root (rxvt_screen *s)
{
  Display *dpy = s->display->dpy;
  unsigned int root_pm_w, root_pm_h;
  Pixmap root_pixmap = s->display->get_pixmap_property (s->display->xa[XA_XROOTPMAP_ID]);
  if (root_pixmap == None)
    root_pixmap = s->display->get_pixmap_property (s->display->xa[XA_ESETROOT_PMAP_ID]);

  if (root_pixmap == None)
    return 0;

  Window wdummy;
  int idummy;
  unsigned int udummy;

  if (!XGetGeometry (dpy, root_pixmap, &wdummy, &idummy, &idummy, &root_pm_w, &root_pm_h, &udummy, &udummy))
    return 0;

  rxvt_img *img = new rxvt_img (
     s,
     XRenderFindVisualFormat (dpy, DefaultVisual (dpy, s->display->screen)),
     0,
     0,
     root_pm_w,
     root_pm_h
  );

  img->pm = root_pixmap;
  img->ref = new pixref (root_pm_w, root_pm_h);
  img->ref->ours = false;

  return img;
}

rxvt_img *
rxvt_img::new_from_pixbuf (rxvt_screen *s, GdkPixbuf *pb)
{
  Display *dpy = s->display->dpy;

  int width  = gdk_pixbuf_get_width  (pb);
  int height = gdk_pixbuf_get_height (pb);

  if (width > 32767 || height > 32767) // well, we *could* upload in chunks
    rxvt_fatal ("rxvt_img::new_from_pixbuf: image too big (maximum size 32768x32768).\n");

  // since we require rgb24/argb32 formats from xrender we assume
  // that both 24 and 32 bpp MUST be supported by any screen that supports xrender
  int depth = gdk_pixbuf_get_has_alpha (pb) ? 32 : 24;

  XImage xi;

  xi.width            = width;
  xi.height           = height;
  xi.xoffset          = 0;
  xi.format           = ZPixmap;
  xi.byte_order       = LSBFirst; // maybe go for host byte order, because servers are usually local?
  xi.bitmap_unit      = 32;         //XY only, unused
  xi.bitmap_bit_order = LSBFirst;   //XY only, unused
  xi.bitmap_pad       = BitmapPad (dpy);
  xi.depth            = depth;
  xi.bytes_per_line   = 0;
  xi.bits_per_pixel   = 32;         //Z only
  xi.red_mask         = 0x00000000; //Z only, unused
  xi.green_mask       = 0x00000000; //Z only, unused
  xi.blue_mask        = 0x00000000; //Z only, unused
  xi.obdata           = 0;          // probably unused

  if (!XInitImage (&xi))
    rxvt_fatal ("unable to initialise ximage, please report.\n");

  if (height > INT_MAX / xi.bytes_per_line)
    rxvt_fatal ("rxvt_img::new_from_pixbuf: image too big for Xlib.\n");

  xi.data = (char *)rxvt_malloc (height * xi.bytes_per_line);

  int rowstride = gdk_pixbuf_get_rowstride (pb);

  assert (3 + (depth == 32) == gdk_pixbuf_get_n_channels (pb));
  unsigned char *row = gdk_pixbuf_get_pixels (pb);
  char *line = xi.data;

  for (int y = 0; y < height; y++)
    {
      unsigned char *src = row;
      uint32_t      *dst = (uint32_t *)line;

      if (depth == 24)
        for (int x = 0; x < width; x++)
          {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;

            uint32_t v = (r << 16) | (g << 8) | b;
            
            if (ecb_big_endian ())
              v = ecb_bswap32 (v);

            *dst++ = v;
          }
      else
        for (int x = 0; x < width; x++)
          {
            uint32_t v = *(uint32_t *)src; src += 4;

            if (ecb_little_endian ())
              v = ecb_bswap32 (v);

            v = ecb_rotr32 (v, 8);

            *dst++ = v;
          }

      row += rowstride;
      line += xi.bytes_per_line;
    }

  rxvt_img *img = new rxvt_img (s, XRenderFindStandardFormat (dpy, depth == 24 ? PictStandardRGB24 : PictStandardARGB32), 0, 0, width, height);
  img->alloc ();

  GC gc = XCreateGC (dpy, img->pm, 0, 0);
  XPutImage (dpy, img->pm, gc, &xi, 0, 0, 0, 0, width, height);
  XFreeGC (dpy, gc);

  free (xi.data);

  return img;
}

rxvt_img *
rxvt_img::new_from_file (rxvt_screen *s, const char *filename)
{
  GError *err = 0;
  GdkPixbuf *pb = gdk_pixbuf_new_from_file (filename, &err);

  if (!pb)
    rxvt_fatal ("rxvt_img::new_from_file: %s\n", err->message);

  rxvt_img *img = new_from_pixbuf (s, pb);

  g_object_unref (pb);

  return img;
}

void
rxvt_img::destroy ()
{
  if (--ref->cnt)
    return;

  if (pm && ref->ours)
    XFreePixmap (s->display->dpy, pm);

  delete ref;
}

rxvt_img::~rxvt_img ()
{
  destroy ();
}

void
rxvt_img::alloc ()
{
  pm = XCreatePixmap (s->display->dpy, s->display->root, w, h, format->depth);
  ref = new pixref (w, h);
}

Picture
rxvt_img::src_picture ()
{
  Display *dpy = s->display->dpy;

  XRenderPictureAttributes pa;
  pa.repeat = repeat;
  Picture pic = XRenderCreatePicture (dpy, pm, format, CPRepeat, &pa);

  return pic;
}

void
rxvt_img::unshare ()
{
  if (ref->cnt == 1 && ref->ours)
    return;

  //TODO: maybe should reify instead
  Pixmap pm2 = XCreatePixmap (s->display->dpy, s->display->root, ref->w, ref->h, format->depth);
  GC gc = XCreateGC (s->display->dpy, pm, 0, 0);
  XCopyArea (s->display->dpy, pm, pm2, gc, 0, 0, ref->w, ref->h, 0, 0);
  XFreeGC (s->display->dpy, gc);

  destroy ();

  pm = pm2;
  ref = new pixref (ref->w, ref->h);
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

rxvt_img *
rxvt_img::blur (int rh, int rv)
{
  if (!(s->display->flags & DISPLAY_HAS_RENDER_CONV))
    return clone ();

  Display *dpy = s->display->dpy;
  int size = max (rh, rv) * 2 + 1;
  double *kernel = (double *)malloc (size * sizeof (double));
  XFixed *params = (XFixed *)malloc ((size + 2) * sizeof (XFixed));
  rxvt_img *img = new rxvt_img (s, format, x, y, w, h, repeat);
  img->alloc ();

  Picture src = src_picture ();

  XRenderPictureAttributes pa;
  pa.repeat = RepeatPad;
  Picture dst = XRenderCreatePicture (dpy, img->pm, format, CPRepeat, &pa);

  Pixmap tmp_pm = XCreatePixmap (dpy, pm, w, h, format->depth);
  Picture tmp = XRenderCreatePicture (dpy, tmp_pm , format, CPRepeat, &pa);
  XFreePixmap (dpy, tmp_pm);

  if (kernel && params)
    {
      size = rh * 2 + 1;
      get_gaussian_kernel (rh, size, kernel, params);

      XRenderSetPictureFilter (dpy, src, FilterConvolution, params, size+2);
      XRenderComposite (dpy,
                        PictOpSrc,
                        src,
                        None,
                        tmp,
                        0, 0,
                        0, 0,
                        0, 0,
                        w, h);

      size = rv * 2 + 1;
      get_gaussian_kernel (rv, size, kernel, params);
      ::swap (params[0], params[1]);

      XRenderSetPictureFilter (dpy, src, FilterConvolution, params, size+2);
      XRenderComposite (dpy,
                        PictOpSrc,
                        tmp,
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
  XRenderFreePicture (dpy, tmp);

  return img;
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
rxvt_img::brightness (unsigned short r, unsigned short g, unsigned short b, unsigned short a)
{
  Display *dpy = s->display->dpy;
  Picture src = create_xrender_mask (dpy, pm, True);
  Picture dst = XRenderCreatePicture (dpy, pm, format, 0, 0);

  XRenderColor mask_c;
  mask_c.red   = r;
  mask_c.green = g;
  mask_c.blue  = b;
  mask_c.alpha = a;
  XRenderFillRectangle (dpy, PictOpSrc, src, &mask_c, 0, 0, 1, 1);

  XRenderComposite (dpy, PictOpAdd, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);
}

void
rxvt_img::contrast (unsigned short r, unsigned short g, unsigned short b, unsigned short a)
{
  if (!(s->display->flags & DISPLAY_HAS_RENDER_MUL))
    return;

  Display *dpy = s->display->dpy;
  Picture src = create_xrender_mask (dpy, pm, True);
  Picture dst = XRenderCreatePicture (dpy, pm, format, 0, 0);

  XRenderColor mask_c;
  mask_c.red   = r;
  mask_c.green = g;
  mask_c.blue  = b;
  mask_c.alpha = a;
  XRenderFillRectangle (dpy, PictOpSrc, src, &mask_c, 0, 0, 1, 1);

  XRenderComposite (dpy, PictOpMultiply, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);
}

rxvt_img *
rxvt_img::clone ()
{
  return new rxvt_img (*this);
}

static XRenderPictFormat *
find_alpha_format_for (Display *dpy, XRenderPictFormat *format)
{
  if (format->direct.alphaMask)
    return format; // already has alpha

  // try to find a suitable alpha format, one bit alpha is enough for our purposes
  if (format->type == PictTypeDirect)
    for (int n = 0; XRenderPictFormat *f = XRenderFindFormat (dpy, 0, 0, n); ++n)
      if (f->direct.alphaMask
          && f->type == PictTypeDirect
          && ecb_popcount32 (f->direct.redMask  ) >= ecb_popcount32 (format->direct.redMask  )
          && ecb_popcount32 (f->direct.greenMask) >= ecb_popcount32 (format->direct.greenMask)
          && ecb_popcount32 (f->direct.blueMask ) >= ecb_popcount32 (format->direct.blueMask ))
        return f;

  // should be a very good fallback
  return XRenderFindStandardFormat (dpy, PictStandardARGB32);
}

rxvt_img *
rxvt_img::reify ()
{
  if (x == 0 && y == 0 && w == ref->w && h == ref->h)
    return clone ();

  Display *dpy = s->display->dpy;

  bool alpha = !format->direct.alphaMask
               && (x || y)
               && repeat == RepeatNone;

  rxvt_img *img = new rxvt_img (s, alpha ? find_alpha_format_for (dpy, format) : format, 0, 0, w, h, repeat);
  img->alloc ();

  Picture src = src_picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, img->format, 0, 0);
  
  if (alpha)
    {
      XRenderColor rc = { 0, 0, 0, 0 };
      XRenderFillRectangle (dpy, PictOpSrc, dst, &rc, 0, 0, w, h);//TODO: split into four fillrectangles
      XRenderComposite (dpy, PictOpSrc, src, None, dst, 0, 0, 0, 0, -x, -y, ref->w, ref->h);
    }
  else
    XRenderComposite (dpy, PictOpSrc, src, None, dst, x, y, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

rxvt_img *
rxvt_img::sub_rect (int x, int y, int width, int height)
{
  rxvt_img *img = clone ();

  img->x += x;
  img->y += y;

  if (w != width || h != height)
    {
      img->w = width;
      img->h = height;

      rxvt_img *img2 = img->reify ();
      delete img;
      img = img2;
    }

  return img;
}

rxvt_img *
rxvt_img::transform (int new_width, int new_height, double matrix[9])
{
  rxvt_img *img = new rxvt_img (s, format, 0, 0, new_width, new_height, repeat);
  img->alloc ();

  Display *dpy = s->display->dpy;
  Picture src = src_picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, img->format, 0, 0);

  XTransform xfrm;

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      xfrm.matrix [i][j] = XDoubleToFixed (matrix [i * 3 + j]);

#if 0
  xfrm.matrix [0][2] -= XDoubleToFixed (x);//TODO
  xfrm.matrix [1][2] -= XDoubleToFixed (y);
#endif

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
  if (w == new_width && h == new_height)
    return clone ();

  double matrix[9] = {
    w  / (double)new_width, 0, 0,
    0, h / (double)new_height, 0,
    0,                      0, 1
  };

  int old_repeat_mode = repeat;
  repeat = RepeatPad; // not right, but xrender can't proeprly scale it seems

  rxvt_img *img = transform (new_width, new_height, matrix);

  repeat = old_repeat_mode;
  img->repeat = repeat;

  return img;
}

rxvt_img *
rxvt_img::rotate (int new_width, int new_height, int x, int y, double phi)
{
  double s = sin (phi);
  double c = cos (phi);

  double matrix[9] = {
    c, -s, -c * x + s * y + x,
    s,  c, -s * x - c * y + y,
    0,  0,                  1
  };

  return transform (new_width, new_height, matrix);
}

rxvt_img *
rxvt_img::convert_format (XRenderPictFormat *new_format, const rxvt_color &bg)
{
  if (new_format == format)
    return clone ();

  rxvt_img *img = new rxvt_img (s, new_format, x, y, w, h, repeat);
  img->alloc ();

  Display *dpy = s->display->dpy;
  Picture src = src_picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, new_format, 0, 0);
  int op = PictOpSrc;

  if (format->direct.alphaMask && !new_format->direct.alphaMask)
    {
      // does it have to be that complicated
      rgba c;
      bg.get (c);

      XRenderColor rc = { c.r, c.g, c.b, 0xffff };
      XRenderFillRectangle (dpy, PictOpSrc, dst, &rc, 0, 0, w, h);

      op = PictOpOver;
    }

  XRenderComposite (dpy, op, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

rxvt_img *
rxvt_img::blend (rxvt_img *img, double factor)
{
  rxvt_img *img2 = clone ();
  Display *dpy = s->display->dpy;
  Picture src = img->src_picture ();
  Picture dst = XRenderCreatePicture (dpy, img2->pm, img2->format, 0, 0);
  Picture mask = create_xrender_mask (dpy, img->pm, False);

  XRenderColor mask_c;

  mask_c.alpha = float_to_component (factor);
  mask_c.red   =
  mask_c.green =
  mask_c.blue  = 0;
  XRenderFillRectangle (dpy, PictOpSrc, mask, &mask_c, 0, 0, 1, 1);

  XRenderComposite (dpy, PictOpOver, src, mask, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);
  XRenderFreePicture (dpy, mask);

  return img2;
}

#endif

