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

#include <sys/utsname.h>

#ifndef NO_SLOW_LINK_SUPPORT
# include <sys/socket.h>
# include <sys/un.h>
#endif

#if XFT
# include <X11/extensions/Xrender.h>
#endif

const char *const xa_names[] =
{
  "TEXT",
  "COMPOUND_TEXT",
  "UTF8_STRING",
  "MULTIPLE",
  "TARGETS",
  "TIMESTAMP",
  "VT_SELECTION",
  "INCR",
  "WM_PROTOCOLS",
  "WM_DELETE_WINDOW",
  "CLIPBOARD",
  "AVERAGE_WIDTH",
  "WEIGHT_NAME",
  "SLANT",
  "CHARSET_REGISTRY",
  "CHARSET_ENCODING",
#if ENABLE_FRILLS
  "_MOTIF_WM_HINTS",
#endif
#if ENABLE_EWMH
  "_NET_WM_PID",
  "_NET_WM_NAME",
  "_NET_WM_ICON_NAME",
  "_NET_WM_PING",
#endif
#if USE_XIM
  "WM_LOCALE_NAME",
  "XIM_SERVERS",
#endif
#ifdef TRANSPARENT
  "_XROOTPMAP_ID",
  "ESETROOT_PMAP_ID",
#endif
#if ENABLE_XEMBED
  "_XEMBED",
  "_XEMBED_INFO",
#endif
#if !ENABLE_MINIMAL
  "SCREEN_RESOURCES",
  "XDCCC_LINEAR_RGB_CORRECTION",
  "XDCCC_LINEAR_RGB_MATRICES",
  "WM_COLORMAP_WINDOWS",
  "WM_STATE",
#endif
};

/////////////////////////////////////////////////////////////////////////////

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
          ++(*i)->referenced;
          (*i)->ref_next ();
          return *i;
        }
    }

  T *obj = new T (id);

  if (obj && obj->ref_init ())
    {
      obj->referenced = 1;
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

bool
rxvt_xim::ref_init ()
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

void
rxvt_screen::set (rxvt_display *disp)
{
  display = disp;
  xdisp   = disp->display;

  Screen *screen = ScreenOfDisplay (xdisp, disp->screen);

  depth   = DefaultDepthOfScreen    (screen);
  visual  = DefaultVisualOfScreen   (screen);
  cmap    = DefaultColormapOfScreen (screen);
}

void
rxvt_screen::set (rxvt_display *disp, int bitdepth)
{
  set (disp);

#if XFT
  XVisualInfo vinfo;

  if (XMatchVisualInfo (xdisp, display->screen, bitdepth, TrueColor, &vinfo))
    {
      depth  = bitdepth;
      visual = vinfo.visual;
      cmap   = XCreateColormap (xdisp, disp->root, visual, AllocNone);
    }
#endif
}

void
rxvt_screen::clear ()
{
  if (cmap != DefaultColormapOfScreen (ScreenOfDisplay (xdisp, display->screen)))
    XFreeColormap (xdisp, cmap);
}

/////////////////////////////////////////////////////////////////////////////

rxvt_display::rxvt_display (const char *id)
: refcounted (id)
, x_ev (this, &rxvt_display::x_cb)
, selection_owner (0)
{
}

XrmDatabase
rxvt_display::get_resources ()
{
  char *homedir = (char *)getenv ("HOME");
  char fname[1024];

  /*
   * get resources using the X library function
   */
  char *displayResource, *xe;
  XrmDatabase database, rdb1;

  database = NULL;

  // for ordering, see for example http://www.faqs.org/faqs/Xt-FAQ/ Subject: 20

  // 6. System wide per application default file.

  /* Add in $XAPPLRESDIR/Rxvt only; not bothering with XUSERFILESEARCHPATH */
  if ((xe = (char *)getenv ("XAPPLRESDIR")))
    {
      snprintf (fname, sizeof (fname), "%s/%s", xe, RESCLASS);

      if ((rdb1 = XrmGetFileDatabase (fname)))
        XrmMergeDatabases (rdb1, &database);
    }

  // 5. User's per application default file.
  // none

  // 4. User's defaults file.
  /* Get any Xserver defaults */
  displayResource = XResourceManagerString (display);

  if (displayResource != NULL)
    {
      if ((rdb1 = XrmGetStringDatabase (displayResource)))
        XrmMergeDatabases (rdb1, &database);
    }
  else if (homedir)
    {
      snprintf (fname, sizeof (fname), "%s/.Xdefaults", homedir);

      if ((rdb1 = XrmGetFileDatabase (fname)))
        XrmMergeDatabases (rdb1, &database);
    }

  /* Get screen specific resources */
  displayResource = XScreenResourceString (ScreenOfDisplay (display, screen));

  if (displayResource != NULL)
    {
      if ((rdb1 = XrmGetStringDatabase (displayResource)))
        /* Merge with screen-independent resources */
        XrmMergeDatabases (rdb1, &database);

      XFree (displayResource);
    }

  // 3. User's per host defaults file
  /* Add in XENVIRONMENT file */
  if ((xe = (char *)getenv ("XENVIRONMENT"))
      && (rdb1 = XrmGetFileDatabase (xe)))
    XrmMergeDatabases (rdb1, &database);
  else if (homedir)
    {
      struct utsname un;

      if (!uname (&un))
        {
          snprintf (fname, sizeof (fname), "%s/.Xdefaults-%s", homedir, un.nodename);

          if ((rdb1 = XrmGetFileDatabase (fname)))
            XrmMergeDatabases (rdb1, &database);
        }
    }

  return database;
}

bool rxvt_display::ref_init ()
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

  assert (sizeof (xa_names) / sizeof (char *) == NUM_XA);
  XInternAtoms (display, (char **)xa_names, NUM_XA, False, xa);

  XrmSetDatabase (display, get_resources ());

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

  int fd = XConnectionNumber (display);

#ifndef NO_SLOW_LINK_SUPPORT
  // try to detect wether we have a local connection.
  // assume unix domains socket == local, everything else not
  // TODO: might want to check for inet/127.0.0.1
  is_local = 0;
  sockaddr_un sa;
  socklen_t sl = sizeof (sa);

  if (!getsockname (fd, (sockaddr *)&sa, &sl))
    is_local = sa.sun_family == AF_LOCAL;
#endif

  x_ev.start (fd, EVENT_READ);
  fcntl (fd, F_SETFD, FD_CLOEXEC);

  XSelectInput (display, root, PropertyChangeMask);

  flush ();

  return true;
}

void
rxvt_display::ref_next ()
{
  // TODO: somehow check wether the database files/resources changed
  // before re-loading/parsing
  XrmDestroyDatabase (XrmGetDatabase (display));
  XrmSetDatabase (display, get_resources ());
}

rxvt_display::~rxvt_display ()
{
  if (!display)
    return;

#ifdef POINTER_BLANK
  XFreeCursor (display, blank_cursor);
#endif
  x_ev.stop ();
#ifdef USE_XIM
  xims.clear ();
#endif
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

  if (XGetWindowProperty (display, root, xa[XA_XIM_SERVERS], 0L, 1000000L,
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
              && xev.xproperty.atom == xa[XA_XIM_SERVERS])
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
rxvt_color::set (rxvt_screen *screen, const char *name)
{
#if XFT
  int l = strlen (name);
  rxvt_rgba r;
  char eos;
  int mult;

  // shortcutting this saves countless server RTTs for the built-in colours
  if (l == 3+3*3 && 3 == sscanf (name, "rgb:%hx/%hx/%hx/%hx%c", &r.r, &r.g, &r.b, &r.a, &eos))
    {
      r.a  = rxvt_rgba::MAX_CC;
      mult = rxvt_rgba::MAX_CC / 0x00ff;
    }

  // parse a number of non-standard ARGB colour specifications
  else if (     l == 1+4*1 && 4 == sscanf (name, "#%1hx%1hx%1hx%1hx%c", &r.a, &r.r, &r.g, &r.b, &eos))
    mult = rxvt_rgba::MAX_CC / 0x000f;
  else if (l == 1+4*2 && 4 == sscanf (name, "#%2hx%2hx%2hx%2hx%c", &r.a, &r.r, &r.g, &r.b, &eos))
    mult = rxvt_rgba::MAX_CC / 0x00ff;
  else if (l == 1+4*4 && 4 == sscanf (name, "#%4hx%4hx%4hx%4hx%c", &r.a, &r.r, &r.g, &r.b, &eos))
    mult = rxvt_rgba::MAX_CC / 0xffff;
  else if (l == 4+5*4 && 4 == sscanf (name, "rgba:%hx/%hx/%hx/%hx%c", &r.r, &r.g, &r.b, &r.a, &eos))
    mult = rxvt_rgba::MAX_CC / 0xffff;

  // slow case: server round trip
  else
    return XftColorAllocName (screen->xdisp, screen->visual, screen->cmap, name, &c);

  r.r *= mult; r.g *= mult; r.b *= mult; r.a *= mult;

  return set (screen, r);
#else
  XColor xc;

  if (XParseColor (screen->xdisp, screen->cmap, name, &xc))
    return set (screen, rxvt_rgba (xc.red, xc.green, xc.blue));

  return false;
#endif
}

bool
rxvt_color::set (rxvt_screen *screen, rxvt_rgba rgba)
{
#if XFT
  XRenderPictFormat *format;

  // FUCKING Xft gets it wrong, of course, so work around it
  // transparency users should eat shit and die, and then
  // XRenderQueryPictIndexValues themselves plenty.
  if (screen->visual->c_class == TrueColor
      && (format = XRenderFindVisualFormat (screen->xdisp, screen->visual)))
    {
      // the fun lies in doing everything manually...
      c.color.red   = rgba.r;
      c.color.green = rgba.g;
      c.color.blue  = rgba.b;
      c.color.alpha = rgba.a;

      c.pixel = ((rgba.r * format->direct.redMask   / rxvt_rgba::MAX_CC) << format->direct.red  )
              | ((rgba.g * format->direct.greenMask / rxvt_rgba::MAX_CC) << format->direct.green)
              | ((rgba.b * format->direct.blueMask  / rxvt_rgba::MAX_CC) << format->direct.blue )
              | ((rgba.a * format->direct.alphaMask / rxvt_rgba::MAX_CC) << format->direct.alpha);

      return true;
    }
  else
    {
      XRenderColor d;

      d.red   = rgba.r;
      d.green = rgba.g;
      d.blue  = rgba.b;
      d.alpha = rgba.a;

      return XftColorAllocValue (screen->xdisp, screen->visual, screen->cmap, &d, &c);
    }

  return false;
#else
  XColor xc;

  xc.red   = rgba.r;
  xc.green = rgba.g;
  xc.blue  = rgba.b;
  xc.flags = DoRed | DoGreen | DoBlue;

  if (XAllocColor (screen->xdisp, screen->cmap, &xc))
    {
      p = xc.pixel;
      return true;
    }

  return false;
#endif
}

void 
rxvt_color::get (rxvt_screen *screen, rxvt_rgba &rgba)
{
#if XFT
  rgba.r = c.color.red;
  rgba.g = c.color.green;
  rgba.b = c.color.blue;
  rgba.a = c.color.alpha;
#else
  XColor c;

  c.pixel = p;
  XQueryColor (screen->xdisp, screen->cmap, &c);

  rgba.r = c.red;
  rgba.g = c.green;
  rgba.b = c.blue;
  rgba.a = rxvt_rgba::MAX_CC;
#endif
}

void 
rxvt_color::free (rxvt_screen *screen)
{
#if XFT
  XftColorFree (screen->xdisp, screen->visual, screen->cmap, &c);
#else
  XFreeColors (screen->xdisp, screen->cmap, &p, 1, AllPlanes);
#endif
}

rxvt_color
rxvt_color::fade (rxvt_screen *screen, int percent)
{
  rxvt_color faded;

  rxvt_rgba c;
  get (screen, c);

  c.r = lerp (0, c.r, percent);
  c.g = lerp (0, c.g, percent);
  c.b = lerp (0, c.b, percent);

  faded.set (screen, c);

  return faded;
}

rxvt_color
rxvt_color::fade (rxvt_screen *screen, int percent, rxvt_color &fadeto)
{
  rxvt_rgba c, fc;
  rxvt_color faded;
  
  get (screen, c);
  fadeto.get (screen, fc);

  faded.set (
    screen,
    rxvt_rgba (
      lerp (fc.r, c.r, percent),
      lerp (fc.g, c.g, percent),
      lerp (fc.b, c.b, percent),
      lerp (fc.a, c.a, percent)
    )
  );

  return faded;
}

