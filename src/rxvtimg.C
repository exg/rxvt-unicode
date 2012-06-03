#include "../config.h"
#include "rxvt.h"

#if HAVE_IMG

rxvt_img::rxvt_img (rxvt_screen *screen, Pixmap *pm, XRenderPictFormat *format, int width, int height)
: s(screen), pm(pixmap) w(width), h(height), format(format)
{
}

rxvt_img::rxvt_img (rxvt_screen *screen, GdkPixbuf *pixbuf,
          int x, int y, int width, int height)
: s(screen), w(width), h(height)
{
  argb  = gdk_pixbuf_get_has_alpha (pixbuf);
  depth = gdk_pixbuf_get_bits_per_sample (pixbuf);
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
}

rxvt_img *copy ()
{
  Pixmap pm2 = XCreatePixmap (s->display->dpy, pm, w, h, depth);
  XCopyArea (s->display->dpy, pm, pm2, 0, 0, w, h, 0, 0);
  return new rxvt_img (s, pm2, format, w, h);
}

#endif

