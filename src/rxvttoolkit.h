/*----------------------------------------------------------------------*
 * File:	rxvttoolkit.h - provide toolkit-functionality for rxvt.
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2003-2006 Marc Lehmann <pcg@goof.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

#ifndef RXVT_TOOLKIT_H
#define RXVT_TOOLKIT_H

#include <X11/Xlib.h>

#if XFT
# include <X11/Xft/Xft.h>
#endif

#include "ev_cpp.h"

#include "rxvtutil.h"

#include "callback.h"

// see rxvttoolkit.C:xa_names, which must be kept in sync
enum {
  XA_TEXT,
  XA_COMPOUND_TEXT,
  XA_UTF8_STRING,
  XA_MULTIPLE,
  XA_TARGETS,
  XA_TIMESTAMP,
  XA_VT_SELECTION,
  XA_INCR,
  XA_WM_PROTOCOLS,
  XA_WM_DELETE_WINDOW,
  XA_CLIPBOARD,
  XA_AVERAGE_WIDTH,
  XA_WEIGHT_NAME,
  XA_SLANT,
  XA_CHARSET_REGISTRY,
  XA_CHARSET_ENCODING,
#if ENABLE_FRILLS
  XA_MOTIF_WM_HINTS,
#endif
#if ENABLE_EWMH
  XA_NET_WM_PID,
  XA_NET_WM_NAME,
  XA_NET_WM_ICON_NAME,
  XA_NET_WM_PING,
  XA_NET_WM_ICON,
#endif
#if USE_XIM
  XA_WM_LOCALE_NAME,
  XA_XIM_SERVERS,
#endif
#if ENABLE_TRANSPARENCY
  XA_XROOTPMAP_ID,
  XA_ESETROOT_PMAP_ID,
#endif
#if ENABLE_XEMBED
  XA_XEMBED,
  XA_XEMBED_INFO,
#endif
#if !ENABLE_MINIMAL
  // these are usually allocated by other subsystems, but we do it
  // here to avoid a server roundtrip.
  XA_SCREEN_RESOURCES,
  XA_XDCCC_LINEAR_RGB_CORRECTION,
  XA_XDCCC_LINEAR_RGB_MATRICES,
  XA_WM_COLORMAP_WINDOWS,
  XA_WM_STATE,
  XA_cursor,
# if USE_XIM
  // various selection targets used by XIM
  XA_TRANSPORT,
  XA_LOCALES,
  XA__XIM_PROTOCOL,
  XA__XIM_XCONNECT,
  XA__XIM_MOREDATA,
# endif
#endif
  NUM_XA
};

struct rxvt_term;
struct rxvt_display;

struct im_watcher;
struct xevent_watcher;

template<class watcher>
struct event_vec : vector<watcher *>
{
  void erase_unordered (unsigned int pos)
  {
    watcher *w = (*this)[this->size () - 1];
    this->pop_back ();

    if (!this->empty ())
      if (((*this)[pos] = w)) // '=' is correct!
        w->active = pos + 1;
  }
};

struct rxvt_watcher
{
  int active; /* 0 == inactive, else index into respective vector */

  bool is_active () { return active; }

  rxvt_watcher () : active (0) { }
};

struct refcounted
{
  int referenced;
  char *id;

  refcounted (const char *id);
  bool ref_init () { return false; }
  void ref_next () { }
  ~refcounted ();
};

template<class T>
struct refcache : vector<T *>
{
  T *get (const char *id);
  void put (T *obj);
  void clear ();

  ~refcache ()
  {
    clear ();
  }
};

/////////////////////////////////////////////////////////////////////////////

struct rxvt_screen;

struct rxvt_drawable
{
  rxvt_screen *screen;
  Drawable drawable;
  operator Drawable() { return drawable; }

#if XFT
  XftDraw *xftdrawable;
  operator XftDraw *();
#endif

  rxvt_drawable (rxvt_screen *screen, Drawable drawable)
  : screen(screen),
#if XFT
    xftdrawable(0),
#endif
    drawable(drawable)
  { }

#if XFT
  ~rxvt_drawable ();
#endif
};

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_XIM
struct rxvt_xim : refcounted
{
  void destroy ();
  rxvt_display *display;

//public
  XIM xim;

  rxvt_xim (const char *id) : refcounted (id) { }
  bool ref_init ();
  ~rxvt_xim ();
};
#endif

struct rxvt_screen
{
  rxvt_display *display;
  Display *dpy;
  int depth;
  Visual *visual;
  Colormap cmap;

#if XFT
  // scratch pixmap
  rxvt_drawable *scratch_area;
  int scratch_w, scratch_h;

  rxvt_drawable &scratch_drawable (int w, int h);

  rxvt_screen ();
#endif

  void set (rxvt_display *disp);
  void select_visual (int bitdepth);
  void clear ();
};

struct rxvt_display : refcounted
{
  event_vec<xevent_watcher> xw;

  ev::prepare flush_ev; void flush_cb (ev::prepare &w, int revents);
  ev::io      x_ev    ; void x_cb     (ev::io      &w, int revents);

#ifdef USE_XIM
  refcache<rxvt_xim> xims;
  vector<im_watcher *> imw;

  void im_change_cb ();
  void im_change_check ();
#endif

//public
  Display   *dpy;
  int       screen;
  Window    root;
  rxvt_term *selection_owner;
  Atom      xa[NUM_XA];
  bool      is_local;
#ifdef POINTER_BLANK
  Cursor    blank_cursor;
#endif

  rxvt_display (const char *id);
  XrmDatabase get_resources (bool refresh);
  bool ref_init ();
  void ref_next ();
  ~rxvt_display ();

  void flush ()
  {
    flush_ev.start ();
  }

  Atom atom (const char *name);
  void set_selection_owner (rxvt_term *owner);

  void reg (xevent_watcher *w);
  void unreg (xevent_watcher *w);

#ifdef USE_XIM
  void reg (im_watcher *w);
  void unreg (im_watcher *w);

  rxvt_xim *get_xim (const char *locale, const char *modifiers);
  void put_xim (rxvt_xim *xim);
#endif
};

#ifdef USE_XIM
struct im_watcher : rxvt_watcher, callback<void (void)>
{
  void start (rxvt_display *display)
  {
    display->reg (this);
  }

  void stop (rxvt_display *display)
  {
    display->unreg (this);
  }
};
#endif

struct xevent_watcher : rxvt_watcher, callback<void (XEvent &)>
{
  Window window;

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

struct rgba
{
  unsigned short r, g, b, a;

  enum { MIN_CC = 0x0000, MAX_CC  = 0xffff };

  rgba ()
  { }

  rgba (unsigned short r, unsigned short g, unsigned short b, unsigned short a = MAX_CC)
  : r(r), g(g), b(b), a(a)
  { }
};

struct rxvt_color
{
#if XFT
  XftColor c;
#else
  XColor c;
#endif

  operator Pixel () const { return c.pixel; }

  bool operator == (const rxvt_color &b) const { return Pixel (*this) == Pixel (b); }
  bool operator != (const rxvt_color &b) const { return Pixel (*this) != Pixel (b); }

  bool is_opaque () const
  {
#if XFT
    return c.color.alpha == rgba::MAX_CC;
#else
    return 1;
#endif
  }

  bool alloc (rxvt_screen *screen, const rgba &color);
  void free (rxvt_screen *screen);

  void get (rgba &color);
  void get (XColor &color);

  bool set (rxvt_screen *screen, const char *name);
  bool set (rxvt_screen *screen, const rgba &color);

  void fade (rxvt_screen *screen, int percent, rxvt_color &result, const rgba &to = rgba (0, 0, 0));
};

#if TRACE_PIXMAPS
Pixmap trace_XCreatePixmap (const char *file, int line, Display *dpy, Window r, unsigned int w, unsigned int h, unsigned int d);
void trace_XFreePixmap (const char *file, int line, Display *dpy, Pixmap p);

# define XCreatePixmap(dpy,r,w,h,d) trace_XCreatePixmap (__FILE__,__LINE__,dpy,r,w,h,d)
# define XFreePixmap(dpy,p) trace_XFreePixmap (__FILE__,__LINE__,dpy,p)
#endif

#endif

