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

struct rxvt_term;

struct im_watcher;
struct xevent_watcher;

struct refcounted {
  int referenced;
  char *id;

  refcounted (const char *id);
  bool init () { return false; }
  ~refcounted ();
};

template<class T>
struct refcache : vector<T *> {
  T *get (const char *id);
  void put (T *obj);
  ~refcache ();
};

/////////////////////////////////////////////////////////////////////////////

struct rxvt_xim : refcounted {
  void destroy ();
  rxvt_display *display;

//public
  XIM xim;

  rxvt_xim (const char *id) : refcounted (id) { }
  bool init ();
  ~rxvt_xim ();
};

struct rxvt_display : refcounted {
  Atom xa_xim_servers;

  rxvt_term *selection_owner;
  
  io_manager_vec<xevent_watcher> xw;

  io_watcher x_ev; void x_cb (io_watcher &w, short revents);

  refcache<rxvt_xim> xims;
  vector<im_watcher *> imw;

  void im_change_cb ();

//public
  Display  *display;
  int      depth;
  int      screen;
  Visual   *visual;
  Colormap cmap;
  Window   root;

  rxvt_display (const char *id);
  bool init ();
  ~rxvt_display ();

  void flush ();

  void reg (xevent_watcher *w);
  void unreg (xevent_watcher *w);
  void reg (im_watcher *w);
  void unreg (im_watcher *w);

  void set_selection_owner (rxvt_term *owner);

  rxvt_xim *get_xim (const char *locale, const char *modifiers);
  void put_xim (rxvt_xim *xim);
};

struct im_watcher : watcher, callback0<void> {
  template<class O1, class O2>
  im_watcher (O1 *object, void (O2::*method) ())
  : callback0<void> (object,method)
  { }

  void start (rxvt_display *display)
  {
    display->reg (this);
  }
  void stop (rxvt_display *display)
  {
    display->unreg (this);
  }
};

struct xevent_watcher : watcher, callback1<void, XEvent &> {
  Window window;

  template<class O1, class O2>
  xevent_watcher (O1 *object, void (O2::*method) (XEvent &))
  : callback1<void, XEvent &> (object,method)
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

extern refcache<rxvt_display> displays;

/////////////////////////////////////////////////////////////////////////////

typedef unsigned long Pixel;

struct rxvt_color {
#if XFT
  XftColor c;
  operator Pixel () const { return c.pixel; }
#else
  Pixel p;
  operator Pixel () const { return p; }
#endif

  bool operator == (const rxvt_color &b) const { return Pixel (*this) == Pixel (b); }
  bool operator != (const rxvt_color &b) const { return Pixel (*this) != Pixel (b); }

  void get (rxvt_display *display, unsigned short &cr, unsigned short &cg, unsigned short &cb);
 
  bool set (rxvt_display *display, Pixel p);
  bool set (rxvt_display *display, const char *name);
  bool set (rxvt_display *display, unsigned short cr, unsigned short cg, unsigned short cb);

  void free (rxvt_display *display);
};

#endif

