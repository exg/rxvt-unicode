#ifndef RXVT_COLOR_H
#define RXVT_COLOR_H

#include <X11/Xlib.h>

#if XFT
# include <X11/Xft/Xft.h>
#endif

#include "iom.h"

#include "rxvtlib.h"
#include "rxvtstl.h"

#include "callback.h"

struct rxvt_vars;

struct xevent_watcher;

struct rxvt_display {
  int referenced;
  char *name;
  
  io_manager_vec<xevent_watcher> xw;
  io_watcher x_watcher; void x_event (io_watcher &w, short revents);

//public
  Display  *display;
  int      depth;
  int      screen;
  Visual   *visual;
  Colormap cmap;
  Window   root;

  bool open ();
  void close ();

  void reg (xevent_watcher *w);
  void unreg (xevent_watcher *w);

  rxvt_display (const char *name);
  ~rxvt_display ();
};

struct xevent_watcher : watcher, callback1<void, XEvent &> {
  Window window;

  template<class O1, class O2>
  xevent_watcher (O1 *object, void (O2::*method)(XEvent &))
  : callback1<void, XEvent &>(object,method)
  { }

  void start (rxvt_display *display, Window window)
  {
    this->window = window;
    display->reg (this);
  }
  void stop (rxvt_display *display)
  {
    display->unreg (this);
  }
};

struct rxvt_displays {
  vector<rxvt_display *> list;
  
  rxvt_display *get (const char *name);
  void release (rxvt_display *display);
};

extern rxvt_displays displays;

typedef unsigned long Pixel;

struct rxvt_color {
#if XFT
  XftColor c;
  operator Pixel() const { return c.pixel; }
#else
  Pixel p;
  operator Pixel() const { return p; }
#endif

  bool operator == (const rxvt_color &b) const { return Pixel(*this) == Pixel(b); }
  bool operator != (const rxvt_color &b) const { return Pixel(*this) != Pixel(b); }

  void get (rxvt_display *display, unsigned short &cr, unsigned short &cg, unsigned short &cb);
 
  bool set (rxvt_display *display, Pixel p);
  bool set (rxvt_display *display, const char *name);
  bool set (rxvt_display *display, unsigned short cr, unsigned short cg, unsigned short cb);

  void free (rxvt_display *display);
};

#endif

