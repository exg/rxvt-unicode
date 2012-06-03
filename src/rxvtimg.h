#ifndef IMG_H
#define IMG_H

#if defined HAVE_BG_PIXMAP && defined BG_IMAGE_FROM_FILE && defined ENABLE_TRANSPARENCY && defined HAVE_PIXBUF
  #define HAVE_IMG 1
#endif

#if HAVE_IMG

#include <X11/extensions/Xrender.h>

struct rxvt_img
{
  rxvt_screen *s;
  Pixmap pm;
  int w, h;
  XRenderPictFormat *format;

  rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int width, int height);
  rxvt_img (rxvt_screen *screen, Pixmap *pixmap, XRenderPictFormat *format, int width, int height);
  ~rxvt_img ();

  void render (GdkPixbuf *pixbuf, int src_x, int src_y, int width, int height, int dst_x, int dst_y);

  Pixmap steal ()
  {
    Pixmap res = pm;
    pm = 0;
    return res;
  }

  // inplace
  void fill (const rxvt_color &c);
  void blur (int rh, int rv);
  void brightness (double r, double g, double b, double a = 1.);
  void contrast (double r, double g, double b, double a = 1.);

  // copy
  rxvt_img *copy ();
  rxvt_img *scale (int new_width, int new_height);
  rxvt_img *transform (int new_width, int new_height, double matrix[16]);
};

#endif

#endif

