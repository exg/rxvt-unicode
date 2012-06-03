#include "../config.h"
#include "rxvt.h"

#if HAVE_IMG

rxvt_img::rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int width, int height)
: s(screen), w(width), h(height), format(format)
{
  pm = XCreatePixmap (s->display->dpy, s->display->root, w, h, format->depth);
}

rxvt_img::rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int width, int height, Pixmap pixmap)
: s(screen), pm(pixmap), w(width), h(height), format(format)
{
}

rxvt_img::~rxvt_img ()
{
  if (pm)
    XFreePixmap (s->display->dpy, pm);
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

void
rxvt_img::blur (int rh, int rv)
{
  //TODO
}

void
rxvt_img::brightness (double r, double g, double b, double a)
{
  //TODO
}

void
rxvt_img::contrast (double r, double g, double b, double a)
{
  //TODO
}

void
rxvt_img::render (GdkPixbuf *pixbuf, int src_x, int src_y, int width, int height, int dst_x, int dst_y)
{
  //TODO
}

rxvt_img *
rxvt_img::copy ()
{
  GC gc = XCreateGC (s->display->dpy, pm, 0, 0);
  Pixmap pm2 = XCreatePixmap (s->display->dpy, pm, w, h, format->depth);
  XCopyArea (s->display->dpy, pm, pm2, gc, 0, 0, w, h, 0, 0);
  XFreeGC (s->display->dpy, gc);
  return new rxvt_img (s, format, w, h, pm2);
}

rxvt_img *
rxvt_img::transform (int new_width, int new_height, double matrix[16])
{
  //TODO
}

rxvt_img *
rxvt_img::scale (int new_width, int new_height)
{
  // use transform
  //TODO
}


#endif

