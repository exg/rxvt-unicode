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
  "cursor",
# if USE_XIM
  "TRANSPORT",
  "LOCALES",
  "_XIM_PROTOCOL",
  "_XIM_XCONNECT",
  "_XIM_MOREDATA",
# endif
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
rxvt_color::alloc (rxvt_screen *screen, const rxvt_rgba &color)
{
#if XFT
  XRenderPictFormat *format;

  // FUCKING Xft gets it wrong, of course, so work around it
  // transparency users should eat shit and die, and then
  // XRenderQueryPictIndexValues themselves plenty.
  if ((screen->visual->c_class == TrueColor || screen->visual->c_class == DirectColor)
      && (format = XRenderFindVisualFormat (screen->xdisp, screen->visual)))
    {
      // the fun lies in doing everything manually...
      c.color.red   = color.r;
      c.color.green = color.g;
      c.color.blue  = color.b;
      c.color.alpha = color.a;

      c.pixel = ((color.r * format->direct.redMask   / rxvt_rgba::MAX_CC) << format->direct.red  )
              | ((color.g * format->direct.greenMask / rxvt_rgba::MAX_CC) << format->direct.green)
              | ((color.b * format->direct.blueMask  / rxvt_rgba::MAX_CC) << format->direct.blue )
              | ((color.a * format->direct.alphaMask / rxvt_rgba::MAX_CC) << format->direct.alpha);

      return true;
    }
  else
    {
      XRenderColor d;

      d.red   = color.r;
      d.green = color.g;
      d.blue  = color.b;
      d.alpha = color.a;

      return XftColorAllocValue (screen->xdisp, screen->visual, screen->cmap, &d, &c);
    }
#else
  if (screen->visual->c_class == TrueColor || screen->visual->c_class == DirectColor)
    {
      p = (color.r * (screen->visual->red_mask   >> ctz (screen->visual->red_mask  ))
                   / rxvt_rgba::MAX_CC) << ctz (screen->visual->red_mask  )
        | (color.g * (screen->visual->green_mask >> ctz (screen->visual->green_mask))
                   / rxvt_rgba::MAX_CC) << ctz (screen->visual->green_mask)
        | (color.b * (screen->visual->blue_mask  >> ctz (screen->visual->blue_mask ))
                   / rxvt_rgba::MAX_CC) << ctz (screen->visual->blue_mask );

      return true;
    }
  else
    {
      XColor xc;

      xc.red   = color.r;
      xc.green = color.g;
      xc.blue  = color.b;

      if (XAllocColor (screen->xdisp, screen->cmap, &xc))
	{
	  p = xc.pixel;
	  return true;
	}
      else
        p = (color.r + color.g + color.b) > 128*3
            ? WhitePixelOfScreen (DefaultScreenOfDisplay (screen->xdisp))
            : BlackPixelOfScreen (DefaultScreenOfDisplay (screen->xdisp));
    }
#endif

  return false;
}

bool
rxvt_color::set (rxvt_screen *screen, const char *name)
{
  rxvt_rgba c;
  char eos;
  int skip;

  if (1 <= sscanf (name, "[%hx]%n", &c.a, &skip))
    {
      switch (skip)
        {
          case 2 + 1: c.a *= rxvt_rgba::MAX_CC / 0x000f; break;
          case 2 + 2: c.a *= rxvt_rgba::MAX_CC / 0x00ff; break;
          case 2 + 3: c.a *= rxvt_rgba::MAX_CC / 0x0fff; break;
          case 2 + 4: c.a *= rxvt_rgba::MAX_CC / 0xffff; break;
        }

      name += skip;
    }
  else
    c.a = rxvt_rgba::MAX_CC;

  // parse the non-standard rgba format
  if (strlen (name) != 4+5*4 || 4 != sscanf (name, "rgba:%hx/%hx/%hx/%hx%c", &c.r, &c.g, &c.b, &c.a, &eos))
    {
      XColor xc, xc_exact;

      if (XParseColor (screen->xdisp, screen->cmap, name, &xc))
        {
          c.r = xc.red;
          c.g = xc.green;
          c.b = xc.blue;
        }
      else
        {
          c.r = 0xffff;
          c.g = 0x6969;
          c.b = 0xb4b4;

          rxvt_warn ("unable to parse color '%s', using pink instead.\n", name);
        }
    }

  return set (screen, c);
}

bool
rxvt_color::set (rxvt_screen *screen, const rxvt_rgba &color)
{
  bool got = alloc (screen, color);

#if !ENABLE_MINIMAL
  int cmap_size = screen->visual->map_entries;

  if (!got
      && screen->visual->c_class == PseudoColor
      && cmap_size < 4096)
    {
      XColor *colors = new XColor [screen->visual->map_entries];

      for (int i = 0; i < cmap_size; i++)
        colors [i].pixel = i;
 
      XQueryColors (screen->xdisp, screen->cmap, colors, cmap_size);

      int diff = 0x7fffffffUL;
      XColor *best = colors;

      for (int i = 0; i < cmap_size; i++)
        {
          int d = (squared_diff<int> (color.r >> 2, colors [i].red   >> 2))
                + (squared_diff<int> (color.g >> 2, colors [i].green >> 2))
                + (squared_diff<int> (color.b >> 2, colors [i].blue  >> 2));

          if (d < diff)
            {
              diff = d;
              best = colors + i;
            }
        }

      //rxvt_warn ("could not allocate %04x %04x %04x, getting %04x %04x %04x instead (%d)\n",
      //    color.r, color.g, color.b, best->red, best->green, best->blue, diff);
          
      got = alloc (screen, rxvt_rgba (best->red, best->green, best->blue));

      delete colors;
    }
#endif

  return got;
}

void
rxvt_color::get (rxvt_screen *screen, rxvt_rgba &color)
{
#if XFT
  color.r = c.color.red;
  color.g = c.color.green;
  color.b = c.color.blue;
  color.a = c.color.alpha;
#else
  XColor c;

  c.pixel = p;
  XQueryColor (screen->xdisp, screen->cmap, &c);

  color.r = c.red;
  color.g = c.green;
  color.b = c.blue;
  color.a = rxvt_rgba::MAX_CC;
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
rxvt_color::fade (rxvt_screen *screen, int percent, const rxvt_rgba &to)
{
  rxvt_rgba c;
  get (screen, c);

  rxvt_color faded;
  faded.set (
    screen,
    rxvt_rgba (
      lerp (to.r, c.r, percent),
      lerp (to.g, c.g, percent),
      lerp (to.b, c.b, percent),
      lerp (to.a, c.a, percent)
    )
  );

  return faded;
}

