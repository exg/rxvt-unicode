/*--------------------------------*-C-*---------------------------------*
 * File:	rxvttoolkit.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2003-2004 Marc Lehmann <pcg@goof.com>
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

#include "../config.h"
#include <rxvt.h>
#include <rxvttoolkit.h>

#include <unistd.h>
#include <fcntl.h>

#ifndef NO_SLOW_LINK_SUPPORT
# include <sys/socket.h>
# include <sys/un.h>
#endif

refcounted::refcounted (const char *id)
{
  this->id = strdup (id);
}

refcounted::~refcounted ()
{
  free (id);
}

template<class T>
T *refcache<T>::get (const char *id)
{
  for (T **i = this->begin (); i < this->end (); ++i)
    {
      if (!strcmp (id, (*i)->id))
        {
          (*i)->referenced++;
          return *i;
        }
    }

  T *obj = new T (id);

  obj->referenced = 1;

  if (obj && obj->init ())
    {
      this->push_back (obj);
      return obj;
    }
  else
    {
      delete obj;
      return 0;
    }
}

template<class T>
void refcache<T>::put (T *obj)
{
  if (!obj)
    return;

  if (!--obj->referenced)
    {
      this->erase (find (this->begin (), this->end (), obj));
      delete obj;
    }
}

template<class T>
void refcache<T>::clear ()
{
  while (this->size ())
    put (*this->begin ());
}

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_XIM
static void
#if XIMCB_PROTO_BROKEN
im_destroy_cb (XIC unused1, XPointer client_data, XPointer unused3)
#else
im_destroy_cb (XIM unused1, XPointer client_data, XPointer unused3)
#endif
{
  rxvt_xim *xim = (rxvt_xim *)client_data;
  rxvt_display *display = xim->display;

  xim->xim = 0;

  display->xims.erase (find (display->xims.begin (), display->xims.end (), xim));
  display->im_change_cb ();
}

bool rxvt_xim::init ()
{
  display = GET_R->display; //HACK: TODO

  xim = XOpenIM (display->display, NULL, NULL, NULL);

  if (!xim)
    return false;

  XIMCallback ximcallback;
  ximcallback.client_data = (XPointer)this;
  ximcallback.callback = im_destroy_cb;

  XSetIMValues (xim, XNDestroyCallback, &ximcallback, NULL);

  return true;
}

rxvt_xim::~rxvt_xim ()
{
  if (xim)
    XCloseIM (xim);
}
#endif

/////////////////////////////////////////////////////////////////////////////

rxvt_display::rxvt_display (const char *id)
: refcounted (id)
, x_ev (this, &rxvt_display::x_cb)
, selection_owner (0)
{
}

bool rxvt_display::init ()
{
#ifdef LOCAL_X_IS_UNIX
  if (id[0] == ':')
    {
      val = rxvt_malloc (5 + strlen (id) + 1);
      strcpy (val, "unix/");
      strcat (val, id);
      display = XOpenDisplay (val);
      free (val);
    }
  else
#endif
    display = 0;

  if (!display)
    display = XOpenDisplay (id);

  if (!display)
    return false;

  screen = DefaultScreen (display);
  root   = DefaultRootWindow (display);
  visual = DefaultVisual (display, screen);
  cmap   = DefaultColormap (display, screen);
  depth  = DefaultDepth (display, screen);

  int fd = XConnectionNumber (display);

#ifndef NO_SLOW_LINK_SUPPORT
  // try to detetc wether we have a local connection.
  // assume unix domains socket == local, everything else not
  // TODO: might want to check for inet/127.0.0.1
  is_local = 0;
  sockaddr_un sa;
  socklen_t sl = sizeof (sa);

  if (!getsockname (fd, (sockaddr *)&sa, &sl))
    is_local = sa.sun_family == AF_LOCAL;
#endif

#ifdef POINTER_BLANK
  XColor blackcolour;
  blackcolour.red   = 0;
  blackcolour.green = 0;
  blackcolour.blue  = 0;
  Font f = XLoadFont (display, "fixed");
  blank_cursor = XCreateGlyphCursor (display, f, f, ' ', ' ',
                                     &blackcolour, &blackcolour);
  XUnloadFont (display, f);
#endif

#ifdef PREFER_24BIT
  /*
   * If depth is not 24, look for a 24bit visual.
   */
  if (depth != 24)
    {
      XVisualInfo vinfo;

      if (XMatchVisualInfo (display, screen, 24, TrueColor, &vinfo))
        {
          depth = 24;
          visual = vinfo.visual;
          cmap = XCreateColormap (display,
                                  RootWindow (display, screen),
                                  visual, AllocNone);
        }
    }
#endif

  x_ev.start (fd, EVENT_READ);
  fcntl (fd, F_SETFD, FD_CLOEXEC);

  XSelectInput (display, root, PropertyChangeMask);
#ifdef USE_XIM
  xa_xim_servers = XInternAtom (display, "XIM_SERVERS", 0);
#endif

  flush ();

  return true;
}

rxvt_display::~rxvt_display ()
{
  x_ev.stop ();
#ifdef USE_XIM
  xims.clear ();
#endif

  if (display)
    XCloseDisplay (display);
}

#ifdef USE_XIM
void rxvt_display::im_change_cb ()
{
  for (im_watcher **i = imw.begin (); i != imw.end (); ++i)
    (*i)->call ();
}

void rxvt_display::im_change_check ()
{
  // try to only call im_change_cb when a new input method
  // registers, as xlib crashes due to a race otherwise.
  Atom actual_type, *atoms;
  int actual_format;
  unsigned long nitems, bytes_after;

  if (XGetWindowProperty (display, root, xa_xim_servers, 0L, 1000000L,
                          False, XA_ATOM, &actual_type, &actual_format,
                          &nitems, &bytes_after, (unsigned char **)&atoms)
      != Success )
    return;

  if (actual_type == XA_ATOM  && actual_format == 32)
    for (int i = 0; i < nitems; i++)
      if (XGetSelectionOwner (display, atoms[i]))
        {
          im_change_cb ();
          break;
        }

  XFree (atoms);
}
#endif

void rxvt_display::x_cb (io_watcher &w, short revents)
{
  do
    {
      XEvent xev;
      XNextEvent (display, &xev);

#ifdef USE_XIM
      if (!XFilterEvent (&xev, None))
        {

          if (xev.type == PropertyNotify
              && xev.xany.window == root
              && xev.xproperty.atom == xa_xim_servers)
            im_change_check ();
#endif
          for (int i = xw.size (); i--; )
            {
              if (!xw[i])
                xw.erase_unordered (i);
              else if (xw[i]->window == xev.xany.window)
                xw[i]->call (xev);
            }
#ifdef USE_XIM
        }
#endif
    }
  while (XEventsQueued (display, QueuedAlready));

  XFlush (display);
}

void rxvt_display::flush ()
{
  if (XEventsQueued (display, QueuedAlready))
    x_cb (x_ev, EVENT_READ);

  XFlush (display);
}

void rxvt_display::reg (xevent_watcher *w)
{
  xw.push_back (w);
  w->active = xw.size ();
}

void rxvt_display::unreg (xevent_watcher *w)
{
  if (w->active)
    xw[w->active - 1] = 0;
}

void rxvt_display::set_selection_owner (rxvt_term *owner)
{
  if (selection_owner && selection_owner != owner)
    selection_owner->selection_clear ();

  selection_owner = owner;
}

#ifdef USE_XIM
void rxvt_display::reg (im_watcher *w)
{
  imw.push_back (w);
}

void rxvt_display::unreg (im_watcher *w)
{
  imw.erase (find (imw.begin (), imw.end (), w));
}

rxvt_xim *rxvt_display::get_xim (const char *locale, const char *modifiers)
{
  char *id;
  int l, m;

  l = strlen (locale);
  m = strlen (modifiers);

  if (!(id = (char *)malloc (l + m + 2)))
    return 0;

  memcpy (id, locale, l); id[l] = '\n';
  memcpy (id + l + 1, modifiers, m); id[l + m + 1] = 0;

  rxvt_xim *xim = xims.get (id);

  free (id);

  return xim;
}

void rxvt_display::put_xim (rxvt_xim *xim)
{
#if XLIB_IS_RACEFREE
  xims.put (xim);
#endif
}
#endif

Atom rxvt_display::atom (const char *name)
{
  return XInternAtom (display, name, False);
}

/////////////////////////////////////////////////////////////////////////////

template class refcache<rxvt_display>;
refcache<rxvt_display> displays;

/////////////////////////////////////////////////////////////////////////////
  
bool
rxvt_color::set (rxvt_display *display, Pixel p)
{
#if XFT
  XColor xc;

  xc.pixel = p;
  if (!XQueryColor (display->display, display->cmap, &xc))
    return false;

  XRenderColor d;

  d.red   = xc.red;
  d.green = xc.green;
  d.blue  = xc.blue;
  d.alpha = 0xffff;

  return
    XftColorAllocValue (display->display, 
                        display->visual,
                        display->cmap,
                        &d, &c);
#else
  this->p = p;
#endif

  return true;
}

bool
rxvt_color::set (rxvt_display *display, const char *name)
{
#if XFT
  return XftColorAllocName (display->display, display->visual, display->cmap,
                            name, &c);
#else
  XColor xc;

  if (XParseColor (display->display, display->cmap, name, &xc))
    return set (display, xc.red, xc.green, xc.blue);

  return false;
#endif
}

bool
rxvt_color::set (rxvt_display *display, unsigned short cr, unsigned short cg, unsigned short cb)
{
  XColor xc;

  xc.red   = cr;
  xc.green = cg;
  xc.blue  = cb;
  xc.flags = DoRed | DoGreen | DoBlue;

  if (XAllocColor (display->display, display->cmap, &xc))
    return set (display, xc.pixel);

  return false;
}

void 
rxvt_color::get (rxvt_display *display, unsigned short &cr, unsigned short &cg, unsigned short &cb)
{
#if XFT
  cr = c.color.red;
  cg = c.color.green;
  cb = c.color.blue;
#else
  XColor c;

  c.pixel = p;
  XQueryColor (display->display, display->cmap, &c);

  cr = c.red;
  cg = c.green;
  cb = c.blue;
#endif
}

void 
rxvt_color::free (rxvt_display *display)
{
#if XFT
  XftColorFree (display->display, display->visual, display->cmap, &c);
#else
  XFreeColors (display->display, display->cmap, &p, 1, AllPlanes);
#endif
}

rxvt_color
rxvt_color::fade (rxvt_display *display, int percent)
{
  unsigned short cr, cg, cb;
  rxvt_color faded;

  get (display, cr, cg, cb);
  faded.set (display,
             cr * percent / 100,
             cg * percent / 100,
             cb * percent / 100);

  return faded;
}

