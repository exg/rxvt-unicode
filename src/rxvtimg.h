#ifndef IMG_H
#define IMG_H

#if HAVE_BG_PIXMAP
  #define HAVE_IMG 1
#endif

#if HAVE_IMG

#define float_to_component(d) (int32_t)((d) * 65535.99)

#include <X11/extensions/Xrender.h>

class rxvt_img
{
  void destroy ();
  Picture src_picture ();

public:
  // *could* also hold the Pixmap itself
  struct pixref
  {
    int cnt;
    int w, h;
    bool ours; // false if we don't own the pixmap at all

    pixref (int w, int h)
    : cnt(1), w(w), h(h), ours(true)
    {
    }
  };

  rxvt_screen *s;
  Pixmap pm;
  pixref *ref; // shared refcnt
  int x, y, w, h, repeat;
  XRenderPictFormat *format;

  rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int x, int y, int width, int height, int repeat = RepeatNormal);
  rxvt_img (const rxvt_img &img);
  void alloc ();

# if HAVE_PIXBUF
  static rxvt_img *new_from_pixbuf (rxvt_screen *s, GdkPixbuf *pb); // from pixbuf
  static rxvt_img *new_from_file (rxvt_screen *s, const char *filename); // via pixbuf
# endif
  static rxvt_img *new_from_root (rxvt_screen *s); // get root pixmap

  ~rxvt_img ();

  Pixmap steal ()
  {
    ref->ours = false;
    return pm;
  }

  // inplace
  void move (int dx, int dy)
  {
    x -= dx;
    y -= dy;
  }

  void repeat_mode (int repeat)
  {
    this->repeat = repeat;
  }

  void unshare (); // prepare for write
  void fill (const rgba &c);
  void add_alpha ();
  //void linear_gradient  (const XLinearGradient  *gradient, const XFixed *stops, const XRenderColor *colors, int nstops);
  //void radial_gradient  (const XRadialGradient  *gradient, const XFixed *stops, const XRenderColor *colors, int nstops);
  //void conical_gradient (const XConicalGradient *gradient, const XFixed *stops, const XRenderColor *colors, int nstops);
  void brightness (int32_t r, int32_t g, int32_t b, int32_t a);
  void contrast (int32_t r, int32_t g, int32_t b, int32_t a);

  void brightness (double r, double g, double b, double a = 1.)
  {
    brightness (float_to_component (r),
                float_to_component (g),
                float_to_component (b),
                float_to_component (a));
  }

  void contrast (double r, double g, double b, double a = 1.)
  {
    contrast (float_to_component (r),
              float_to_component (g),
              float_to_component (b),
              float_to_component (a));
  }

  // copy
  rxvt_img *reify (); // make x, y 0, make real width/height
  rxvt_img *blur (int rh, int rv);
  rxvt_img *clone ();
  rxvt_img *sub_rect (int x, int y, int width, int height);
  rxvt_img *transform (double matrix[3][3]);
  rxvt_img *scale (int new_width, int new_height);
  rxvt_img *rotate (int cx, int cy, double phi);
  rxvt_img *convert_format (XRenderPictFormat *format, const rgba &bg);
  rxvt_img *blend (rxvt_img *img, double factor = 1.);

  // egregiuous helper category
  rxvt_img *replace (rxvt_img *&p)
  {
    delete p;
    p = this;
    return this;
  }
};

#endif

#endif

