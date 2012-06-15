#include <string.h>
#include <math.h>
#include "../config.h"
#include "rxvt.h"

#if HAVE_IMG

typedef rxvt_img::nv nv;

struct mat3x3
{
  nv v[3][3];

  mat3x3 ()
  {
  }

  mat3x3 (nv matrix[3][3])
  {
    memcpy (v, matrix, sizeof (v));
  }

  mat3x3 (nv v11, nv v12, nv v13, nv v21, nv v22, nv v23, nv v31, nv v32, nv v33)
  {
    v[0][0] = v11; v[0][1] = v12; v[0][2] = v13;
    v[1][0] = v21; v[1][1] = v22; v[1][2] = v23;
    v[2][0] = v31; v[2][1] = v32; v[2][2] = v33;
  }

  mat3x3 invert ();

        nv *operator [](int i)       { return &v[i][0]; }
  const nv *operator [](int i) const { return &v[i][0]; }

  // quite inefficient, hopefully gcc pulls the w calc out of any loops
  nv apply1 (int i, nv x, nv y)
  {
    mat3x3 &m = *this;

    nv v = m[i][0] * x + m[i][1] * y + m[i][2];
    nv w = m[2][0] * x + m[2][1] * y + m[2][2];

    return v * (1. / w);
  }

  static mat3x3 translate (nv x, nv y);
};

mat3x3
mat3x3::invert ()
{
  mat3x3 &m = *this;
  mat3x3 inv;

  nv s0 = m[2][2] * m[1][1] - m[2][1] * m[1][2];
  nv s1 = m[2][1] * m[0][2] - m[2][2] * m[0][1];
  nv s2 = m[1][2] * m[0][1] - m[1][1] * m[0][2];

  nv invdet = 1. / (m[0][0] * s0 + m[1][0] * s1 + m[2][0] * s2);

  inv[0][0] = invdet * s0;
  inv[0][1] = invdet * s1;
  inv[0][2] = invdet * s2;

  inv[1][0] = invdet * (m[2][0] * m[1][2] - m[2][2] * m[1][0]);
  inv[1][1] = invdet * (m[2][2] * m[0][0] - m[2][0] * m[0][2]);
  inv[1][2] = invdet * (m[1][0] * m[0][2] - m[1][2] * m[0][0]);

  inv[2][0] = invdet * (m[2][1] * m[1][0] - m[2][0] * m[1][1]);
  inv[2][1] = invdet * (m[2][0] * m[0][1] - m[2][1] * m[0][0]);
  inv[2][2] = invdet * (m[1][1] * m[0][0] - m[1][0] * m[0][1]);

  return inv;
}

static mat3x3
operator *(const mat3x3 &a, const mat3x3 &b)
{
  mat3x3 r;

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      r[i][j] = a[i][0] * b[0][j]
              + a[i][1] * b[1][j]
              + a[i][2] * b[2][j];

  return r;
}

mat3x3
mat3x3::translate (nv x, nv y)
{
  return mat3x3 (
    1, 0, x,
    0, 1, y,
    0, 0, 1
  );
}

#if 0
struct pict
{
  Display *dpy;
  Picture pic;

  operator Picture () const
  {
    return pic;
  }

  pict ()
  : pic (0)
  {
  }

  pict (rxvt_img *img, XRenderPictFormat *format = 0)
  : dpy (img->s->display->dpy)
  {
    XRenderPictureAttributes pa;
    pa.repeat = img->repeat;
    pic = XRenderCreatePicture (dpy, img->pm, format ? format : img->format, CPRepeat, &pa);
  }

  ~pict ()
  {
    if (pic)
      XRenderFreePicture (dpy, pic);
  }
};
#endif

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

# if HAVE_PIXBUF

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

  int byte_order = ecb_big_endian () ? MSBFirst : LSBFirst;

  XImage xi;

  xi.width            = width;
  xi.height           = height;
  xi.xoffset          = 0;
  xi.format           = ZPixmap;
  xi.byte_order       = ImageByteOrder (dpy);
  xi.bitmap_unit      = 0;         //XY only, unused
  xi.bitmap_bit_order = 0;         //XY only, unused
  xi.bitmap_pad       = BitmapPad (dpy);
  xi.depth            = 32;
  xi.bytes_per_line   = 0;
  xi.bits_per_pixel   = 32;         //Z only
  xi.red_mask         = 0x00000000; //Z only, unused
  xi.green_mask       = 0x00000000; //Z only, unused
  xi.blue_mask        = 0x00000000; //Z only, unused
  xi.obdata           = 0;          // probably unused

  bool byte_order_mismatch = byte_order != xi.byte_order;

  if (!XInitImage (&xi))
    rxvt_fatal ("unable to initialise ximage, please report.\n");

  if (height > INT_MAX / xi.bytes_per_line)
    rxvt_fatal ("rxvt_img::new_from_pixbuf: image too big for Xlib.\n");

  xi.data = (char *)rxvt_malloc (height * xi.bytes_per_line);

  int rowstride = gdk_pixbuf_get_rowstride (pb);
  bool pb_has_alpha = gdk_pixbuf_get_has_alpha (pb);
  unsigned char *row = gdk_pixbuf_get_pixels (pb);

  char *line = xi.data;

  for (int y = 0; y < height; y++)
    {
      unsigned char *src = row;
      uint32_t      *dst = (uint32_t *)line;

      if (!pb_has_alpha)
        for (int x = 0; x < width; x++)
          {
            uint8_t r = *src++;
            uint8_t g = *src++;
            uint8_t b = *src++;

            uint32_t v = (255 << 24) | (r << 16) | (g << 8) | b;
            
            if (ecb_big_endian () ? !byte_order_mismatch : byte_order_mismatch)
              v = ecb_bswap32 (v);

            *dst++ = v;
          }
      else
        for (int x = 0; x < width; x++)
          {
            uint32_t v = *(uint32_t *)src; src += 4;

            if (ecb_big_endian ())
              v = ecb_bswap32 (v);

            v = ecb_rotl32 (v, 8); // abgr to bgra

            if (!byte_order_mismatch)
              v = ecb_bswap32 (v);

            *dst++ = v;
          }

      row += rowstride;
      line += xi.bytes_per_line;
    }

  rxvt_img *img = new rxvt_img (s, XRenderFindStandardFormat (dpy, PictStandardARGB32), 0, 0, width, height);
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

# endif

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
rxvt_img::picture ()
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

  Pixmap pm2 = XCreatePixmap (s->display->dpy, s->display->root, ref->w, ref->h, format->depth);
  GC gc = XCreateGC (s->display->dpy, pm, 0, 0);
  XCopyArea (s->display->dpy, pm, pm2, gc, 0, 0, ref->w, ref->h, 0, 0);
  XFreeGC (s->display->dpy, gc);

  destroy ();

  pm = pm2;
  ref = new pixref (ref->w, ref->h);
}

void
rxvt_img::fill (const rgba &c)
{
  XRenderColor rc = { c.r, c.g, c.b, c.a };

  Display *dpy = s->display->dpy;
  Picture src = picture ();
  XRenderFillRectangle (dpy, PictOpSrc, src, &rc, 0, 0, w, h);
  XRenderFreePicture (dpy, src);
}

void
rxvt_img::add_alpha ()
{
  if (format->direct.alphaMask)
    return;

  Display *dpy = s->display->dpy;

  rxvt_img *img = new rxvt_img (s, find_alpha_format_for (dpy, format), x, y, w, h, repeat);
  img->alloc ();

  Picture src = picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, img->format, 0, 0);
  
  XRenderComposite (dpy, PictOpSrc, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  ::swap (img->ref, ref);
  ::swap (img->pm , pm );

  delete img;
}

static void
get_gaussian_kernel (int radius, int width, nv *kernel, XFixed *params)
{
  nv sigma = radius / 2.0;
  nv scale = sqrt (2.0 * M_PI) * sigma;
  nv sum = 0.0;

  for (int i = 0; i < width; i++)
    {
      nv x = i - width / 2;
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
  nv *kernel = (nv *)malloc (size * sizeof (nv));
  XFixed *params = (XFixed *)malloc ((size + 2) * sizeof (XFixed));
  rxvt_img *img = new rxvt_img (s, format, x, y, w, h, repeat);
  img->alloc ();

  XRenderPictureAttributes pa;
  pa.repeat = RepeatPad;
  Picture src = XRenderCreatePicture (dpy, pm, format, CPRepeat, &pa);
  Picture dst = XRenderCreatePicture (dpy, img->pm, format, 0, 0);

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

      XRenderSetPictureFilter (dpy, tmp, FilterConvolution, params, size+2);
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
create_xrender_mask (Display *dpy, Drawable drawable, Bool argb, Bool component_alpha)
{
  Pixmap pixmap = XCreatePixmap (dpy, drawable, 1, 1, argb ? 32 : 8);

  XRenderPictFormat *format = XRenderFindStandardFormat (dpy, argb ? PictStandardARGB32 : PictStandardA8);
  XRenderPictureAttributes pa;
  pa.repeat = RepeatNormal;
  pa.component_alpha = component_alpha;
  Picture mask = XRenderCreatePicture (dpy, pixmap, format, CPRepeat | CPComponentAlpha, &pa);

  XFreePixmap (dpy, pixmap);

  return mask;
}

static void
extract (int32_t cl0, int32_t cl1, int32_t &c, unsigned short &xc)
{
  int32_t x = clamp (c, cl0, cl1);
  c -= x;
  xc = x;
}

static bool
extract (int32_t cl0, int32_t cl1, int32_t &r, int32_t &g, int32_t &b, int32_t &a, unsigned short &xr, unsigned short &xg, unsigned short &xb, unsigned short &xa)
{
  extract (cl0, cl1, r, xr);
  extract (cl0, cl1, g, xg);
  extract (cl0, cl1, b, xb);
  extract (cl0, cl1, a, xa);

  return xr | xg | xb | xa;
}

void
rxvt_img::brightness (int32_t r, int32_t g, int32_t b, int32_t a)
{
  unshare ();

  Display *dpy = s->display->dpy;
  Picture dst = XRenderCreatePicture (dpy, pm, format, 0, 0);

  // loop should not be needed for brightness, as only -1..1 makes sense
  //while (r | g | b | a)
    {
      unsigned short xr, xg, xb, xa;
      XRenderColor mask_c;

      if (extract (0, 65535, r, g, b, a, mask_c.red, mask_c.green, mask_c.blue, mask_c.alpha))
        XRenderFillRectangle (dpy, PictOpAdd, dst, &mask_c, 0, 0, w, h);

      if (extract (-65535, 0, r, g, b, a, mask_c.red, mask_c.green, mask_c.blue, mask_c.alpha))
        {
          XRenderColor mask_w = { 65535, 65535, 65535, 65535 };
          XRenderFillRectangle (dpy, PictOpDifference, dst, &mask_w, 0, 0, w, h);
          mask_c.red   = -mask_c.red; //TODO: verify that doing clamp, assign, and negation does the right thing
          mask_c.green = -mask_c.green;
          mask_c.blue  = -mask_c.blue;
          mask_c.alpha = -mask_c.alpha;
          XRenderFillRectangle (dpy, PictOpAdd, dst, &mask_c, 0, 0, w, h);
          XRenderFillRectangle (dpy, PictOpDifference, dst, &mask_w, 0, 0, w, h);
        }
    }

  XRenderFreePicture (dpy, dst);
}

void
rxvt_img::contrast (int32_t r, int32_t g, int32_t b, int32_t a)
{
  if (r < 0 || g < 0 || b < 0 || a < 0)
    rxvt_fatal ("rxvt_img::contrast does not support negative values.\n");

  rxvt_img *img = new rxvt_img (s, format, x, y, w, h, repeat);
  img->alloc ();
  img->fill (rgba (0, 0, 0, 0));

  // premultiply (yeah, these are not exact, sue me or fix it)
  r = (r * (a >> 8)) >> 8;
  g = (g * (a >> 8)) >> 8;
  b = (b * (a >> 8)) >> 8;

  Display *dpy = s->display->dpy;

  Picture src = picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, format, 0, 0);
  Picture mul = create_xrender_mask (dpy, pm, True, True);

  //TODO: this operator does not yet implement some useful contrast
  while (r | g | b | a)
    {
      unsigned short xr, xg, xb, xa;
      XRenderColor mask_c;

      if (extract (0, 65535, r, g, b, a, mask_c.red, mask_c.green, mask_c.blue, mask_c.alpha))
        {
          XRenderFillRectangle (dpy, PictOpSrc, mul, &mask_c, 0, 0, 1, 1);
          XRenderComposite (dpy, PictOpAdd, src, mul, dst, 0, 0, 0, 0, 0, 0, w, h);
        }
    }

  XRenderFreePicture (dpy, mul);
  XRenderFreePicture (dpy, dst);
  XRenderFreePicture (dpy, src);

  ::swap (img->ref, ref);
  ::swap (img->pm , pm );

  delete img;
}

void
rxvt_img::draw (rxvt_img *img, int op, nv mask)
{
  unshare ();

  Display *dpy = s->display->dpy;
  Picture src = img->picture ();
  Picture dst = picture ();
  Picture mask_p = 0;
  
  if (mask != 1.)
    {
      mask_p = create_xrender_mask (dpy, img->pm, False, False);
      XRenderColor mask_c = { 0, 0, 0, float_to_component (mask) };
      XRenderFillRectangle (dpy, PictOpSrc, mask, &mask_c, 0, 0, 1, 1);
    }

  XRenderComposite (dpy, op, src, mask_p, dst, x - img->x, y - img->y, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  if (mask_p)
    XRenderFreePicture (dpy, mask_p);
}

rxvt_img *
rxvt_img::clone ()
{
  return new rxvt_img (*this);
}

rxvt_img *
rxvt_img::reify ()
{
  if (x == 0 && y == 0 && w == ref->w && h == ref->h)
    return clone ();

  Display *dpy = s->display->dpy;

  // add an alpha channel if...
  bool alpha = !format->direct.alphaMask // pixmap has none yet
               && (x || y)               // we need one because of non-zero offset
               && repeat == RepeatNone;  // and we have no good pixels to fill with

  rxvt_img *img = new rxvt_img (s, alpha ? find_alpha_format_for (dpy, format) : format, 0, 0, w, h, repeat);
  img->alloc ();

  Picture src = picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, img->format, 0, 0);
  
  if (alpha)
    {
      XRenderColor rc = { 0, 0, 0, 0 };
      XRenderFillRectangle (dpy, PictOpSrc, dst, &rc, 0, 0, w, h);//TODO: split into four fillrectangles
      XRenderComposite (dpy, PictOpSrc, src, None, dst, 0, 0, 0, 0, x, y, ref->w, ref->h);
    }
  else
    XRenderComposite (dpy, PictOpSrc, src, None, dst, -x, -y, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

rxvt_img *
rxvt_img::sub_rect (int x, int y, int width, int height)
{
  rxvt_img *img = clone ();

  img->x -= x;
  img->y -= y;

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
rxvt_img::transform (nv matrix[3][3])
{
  // calculate new pixel bounding box coordinates
  nv r[2], rmin[2], rmax[2];

  mat3x3 m (matrix);

  for (int i = 0; i < 2; ++i)
    {
      nv v;

      v = m.apply1 (i, 0+x, 0+y);         rmin [i] =            rmax [i] = v; r [i] = v;
      v = m.apply1 (i, w+x, 0+y); min_it (rmin [i], v); max_it (rmax [i],  v);
      v = m.apply1 (i, 0+x, h+y); min_it (rmin [i], v); max_it (rmax [i],  v);
      v = m.apply1 (i, w+x, h+y); min_it (rmin [i], v); max_it (rmax [i],  v);
    }

  float sx = rmin [0] - x;
  float sy = rmin [1] - y;

  // TODO: adjust matrix for subpixel accuracy
  int nx = floor (rmin [0]);
  int ny = floor (rmin [1]);

  int new_width  = ceil (rmax [0] - rmin [0]);
  int new_height = ceil (rmax [1] - rmin [1]);

  m = mat3x3::translate (-x, -y) * m * mat3x3::translate (x, y);

  mat3x3 inv = m.invert ();

  rxvt_img *img = new rxvt_img (s, format, nx, ny, new_width, new_height, repeat);
  img->alloc ();

  Display *dpy = s->display->dpy;
  Picture src = picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, img->format, 0, 0);

  XTransform xfrm;

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      xfrm.matrix [i][j] = XDoubleToFixed (inv [i][j]);

  XRenderSetPictureFilter (dpy, src, "good", 0, 0);
  XRenderSetPictureTransform (dpy, src, &xfrm);
  XRenderComposite (dpy, PictOpSrc, src, None, dst, sx, sy, 0, 0, 0, 0, new_width, new_height);
#if 1
    {
      XRenderColor rc = { 65535,0,0,65535 };
      XRenderFillRectangle (dpy, PictOpSrc, dst, &rc, 0, 0, new_width, new_height);
    }{
      XRenderColor rc = { 0,0,0,65535 };
      XRenderFillRectangle (dpy, PictOpSrc, dst, &rc, 1, 1, new_width - 2, new_height - 2);
    }
  XRenderComposite (dpy, PictOpOver, src, None, dst, sx, sy, 0, 0, 0, 0, new_width, new_height);
#endif

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

rxvt_img *
rxvt_img::scale (int new_width, int new_height)
{
  if (w == new_width && h == new_height)
    return clone ();

  nv matrix[3][3] = {
    { new_width / (nv)w,  0, 0 },
    { 0, new_height / (nv)h, 0 },
    { 0,                  0, 1 }
  };

  int old_repeat_mode = repeat;
  repeat = RepeatPad; // not right, but xrender can't properly scale it seems

  rxvt_img *img = transform (matrix);

  repeat = old_repeat_mode;
  img->repeat = repeat;

  return img;
}

rxvt_img *
rxvt_img::rotate (int cx, int cy, nv phi)
{
  nv s = sin (phi);
  nv c = cos (phi);

  nv matrix[3][3] = {
#if 0
    { c, -s, cx - c * cx + s * cy },
    { s,  c, cy - s * cx - c * cy },
    { 0,  0,                    1 }
#else
    { c, -s, 0 },
    { s,  c, 0 },
    { 0,  0, 1 }
#endif
  };

  move (-cx, -cy);
  rxvt_img *img = transform (matrix);
  move ( cx,  cy);
  img->move (cx, cy);

  return img;
}

rxvt_img *
rxvt_img::convert_format (XRenderPictFormat *new_format, const rgba &bg)
{
  if (new_format == format)
    return clone ();

  rxvt_img *img = new rxvt_img (s, new_format, x, y, w, h, repeat);
  img->alloc ();

  Display *dpy = s->display->dpy;
  Picture src = picture ();
  Picture dst = XRenderCreatePicture (dpy, img->pm, new_format, 0, 0);
  int op = PictOpSrc;

  if (format->direct.alphaMask && !new_format->direct.alphaMask)
    {
      // does it have to be that complicated
      XRenderColor rc = { bg.r, bg.g, bg.b, bg.a };
      XRenderFillRectangle (dpy, PictOpSrc, dst, &rc, 0, 0, w, h);

      op = PictOpOver;
    }

  XRenderComposite (dpy, op, src, None, dst, 0, 0, 0, 0, 0, 0, w, h);

  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);

  return img;
}

rxvt_img *
rxvt_img::blend (rxvt_img *img, nv factor)
{
  rxvt_img *img2 = clone ();
  Display *dpy = s->display->dpy;
  Picture src = img->picture ();
  Picture dst = XRenderCreatePicture (dpy, img2->pm, img2->format, 0, 0);
  Picture mask = create_xrender_mask (dpy, img->pm, False, False);

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

