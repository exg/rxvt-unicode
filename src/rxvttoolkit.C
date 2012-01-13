/*----------------------------------------------------------------------*
 * File:	rxvttoolkit.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2003-2011 Marc Lehmann <schmorp@schmorp.de>
 * Copyright (c) 2011      Emanuele Giaquinta <e.giaquinta@glauco.it>
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
#include <sys/socket.h>
#include <sys/un.h>

#if XFT
# include <X11/extensions/Xrender.h>
#endif

static const char *const xa_names[] =
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
  "_NET_WM_ICON",
#endif
#if USE_XIM
  "WM_LOCALE_NAME",
  "XIM_SERVERS",
#endif
#ifdef ENABLE_TRANSPARENCY
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

  xim = XOpenIM (display->dpy, 0, 0, 0);

  if (!xim)
    return false;

  XIMCallback ximcallback;
  ximcallback.client_data = (XPointer)this;
  ximcallback.callback = im_destroy_cb;

  XSetIMValues (xim, XNDestroyCallback, &ximcallback, (char *)0);

  return true;
}

rxvt_xim::~rxvt_xim ()
{
  if (xim)
    XCloseIM (xim);
}

#endif

/////////////////////////////////////////////////////////////////////////////

#if XFT
rxvt_drawable::~rxvt_drawable ()
{
  if (xftdrawable)
    XftDrawDestroy (xftdrawable);
}

rxvt_drawable::operator XftDraw *()
{
  if (!xftdrawable)
    xftdrawable = XftDrawCreate (screen->dpy, drawable, screen->visual, screen->cmap);

  return xftdrawable;
}
#endif

/////////////////////////////////////////////////////////////////////////////

#if XFT

// not strictly necessary as it is only used with superclass of zero_initialised
rxvt_screen::rxvt_screen ()
: scratch_area (0)
{
}

rxvt_drawable &rxvt_screen::scratch_drawable (int w, int h)
{
  if (!scratch_area || w > scratch_w || h > scratch_h)
    {
      if (scratch_area)
        {
          XFreePixmap (dpy, scratch_area->drawable);
          delete scratch_area;
        }

      Pixmap pm = XCreatePixmap (dpy, RootWindowOfScreen (ScreenOfDisplay (dpy, display->screen)),
                                 scratch_w = w, scratch_h = h, depth);

      scratch_area = new rxvt_drawable (this, pm);
    }

  return *scratch_area;
}

#endif

void
rxvt_screen::set (rxvt_display *disp)
{
  display = disp;
  dpy     = disp->dpy;

  Screen *screen = ScreenOfDisplay (dpy, disp->screen);

  depth   = DefaultDepthOfScreen    (screen);
  visual  = DefaultVisualOfScreen   (screen);
  cmap    = DefaultColormapOfScreen (screen);
}

void
rxvt_screen::select_visual (int bitdepth)
{
#if XFT
  XVisualInfo vinfo;

  if (XMatchVisualInfo (dpy, display->screen, bitdepth, TrueColor, &vinfo))
    {
      depth  = bitdepth;
      visual = vinfo.visual;
      cmap   = XCreateColormap (dpy, display->root, visual, AllocNone);
    }
#endif
}

void
rxvt_screen::clear ()
{
#if XFT
  if (scratch_area)
    {
      XFreePixmap (dpy, scratch_area->drawable);
      delete scratch_area;
    }
#endif

  if (cmap != DefaultColormapOfScreen (ScreenOfDisplay (dpy, display->screen)))
    XFreeColormap (dpy, cmap);
}

/////////////////////////////////////////////////////////////////////////////

rxvt_display::rxvt_display (const char *id)
: refcounted (id)
, selection_owner (0)
, clipboard_owner (0)
{
  x_ev    .set<rxvt_display, &rxvt_display::x_cb    > (this);
  flush_ev.set<rxvt_display, &rxvt_display::flush_cb> (this);
}

XrmDatabase
rxvt_display::get_resources (bool refresh)
{
  char *homedir = getenv ("HOME");
  char fname[1024];

  /*
   * get resources using the X library function
   */
  char *displayResource, *xe;
  XrmDatabase rdb1, database = 0;

#if !XLIB_ILLEGAL_ACCESS
  /* work around a bug in XrmSetDatabase where it frees the db, see ref_next */
  database = XrmGetStringDatabase ("");
#endif

  // for ordering, see for example http://www.faqs.org/faqs/Xt-FAQ/ Subject: 20
  // as opposed to "standard practise", we always read in ~/.Xdefaults

  // 6. System wide per application default file.

  /* Add in $XAPPLRESDIR/Rxvt only; not bothering with XUSERFILESEARCHPATH */
  if ((xe = getenv ("XAPPLRESDIR")))
    {
      snprintf (fname, sizeof (fname), "%s/%s", xe, RESCLASS);

      if ((rdb1 = XrmGetFileDatabase (fname)))
        XrmMergeDatabases (rdb1, &database);
    }

  // 5. User's per application default file.
  // none

  // 4. User's defaults file.
  if (homedir)
    {
      snprintf (fname, sizeof (fname), "%s/.Xdefaults", homedir);

      if ((rdb1 = XrmGetFileDatabase (fname)))
        XrmMergeDatabases (rdb1, &database);
    }

  /* Get any Xserver defaults */
  if (refresh)
    {
      // fucking xlib keeps a copy of the rm string
      Atom actual_type;
      int actual_format;
      unsigned long nitems, nremaining;
      char *val = 0;

#if XLIB_ILLEGAL_ACCESS
      if (dpy->xdefaults)
        XFree (dpy->xdefaults);
#endif

      if (XGetWindowProperty (dpy, RootWindow (dpy, 0), XA_RESOURCE_MANAGER,
                              0L, 100000000L, False,
                              XA_STRING, &actual_type, &actual_format,
                              &nitems, &nremaining,
                              (unsigned char **)&val) == Success
          && actual_type == XA_STRING
          && actual_format == 8)
        displayResource = val;
      else
        {
          displayResource = 0;

          if (val)
            XFree (val);
        }

#if XLIB_ILLEGAL_ACCESS
      dpy->xdefaults = displayResource;
#endif
    }
  else
    displayResource = XResourceManagerString (dpy);

  if (displayResource)
    {
      if ((rdb1 = XrmGetStringDatabase (displayResource)))
        XrmMergeDatabases (rdb1, &database);
    }

#if !XLIB_ILLEGAL_ACCESS
  if (refresh && displayResource)
    XFree (displayResource);
#endif

  /* Get screen specific resources */
  displayResource = XScreenResourceString (ScreenOfDisplay (dpy, screen));

  if (displayResource)
    {
      if ((rdb1 = XrmGetStringDatabase (displayResource)))
        /* Merge with screen-independent resources */
        XrmMergeDatabases (rdb1, &database);

      XFree (displayResource);
    }

  // 3. User's per host defaults file
  /* Add in XENVIRONMENT file */
  if ((xe = getenv ("XENVIRONMENT"))
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
      if (!(val = rxvt_temp_buf<char> (5 + strlen (id) + 1)))
        return false;
      strcpy (val, "unix/");
      strcat (val, id);
      dpy = XOpenDisplay (val);
    }
  else
#endif
    dpy = 0;

  if (!dpy)
    dpy = XOpenDisplay (id);

  if (!dpy)
    return false;

  screen = DefaultScreen     (dpy);
  root   = DefaultRootWindow (dpy);

  assert (ecb_array_length (xa_names) == NUM_XA);
  XInternAtoms (dpy, (char **)xa_names, NUM_XA, False, xa);

  XrmSetDatabase (dpy, get_resources (false));

#ifdef POINTER_BLANK
  XColor blackcolour;
  blackcolour.red   = 0;
  blackcolour.green = 0;
  blackcolour.blue  = 0;
  Font f = XLoadFont (dpy, "fixed");
  blank_cursor = XCreateGlyphCursor (dpy, f, f, ' ', ' ',
                                     &blackcolour, &blackcolour);
  XUnloadFont (dpy, f);
#endif

  int fd = XConnectionNumber (dpy);

  // try to detect whether we have a local connection.
  // assume unix domain socket == local, everything else not
  // TODO: might want to check for inet/127.0.0.1
  is_local = 0;
  sockaddr_un sa;
  socklen_t sl = sizeof (sa);

  if (!getsockname (fd, (sockaddr *)&sa, &sl))
    is_local = sa.sun_family == AF_UNIX;

  flush_ev.start ();
  x_ev.start (fd, ev::READ);
  fcntl (fd, F_SETFD, FD_CLOEXEC);

  XSelectInput (dpy, root, PropertyChangeMask);

  flush ();

  return true;
}

void
rxvt_display::ref_next ()
{
  // TODO: somehow check whether the database files/resources changed
  // before affording re-loading/parsing
  XrmDestroyDatabase (XrmGetDatabase (dpy));
#if XLIB_ILLEGAL_ACCESS
  /* work around a bug in XrmSetDatabase where it frees the db */
  dpy->db = 0;
#endif
  XrmSetDatabase (dpy, get_resources (true));
}

rxvt_display::~rxvt_display ()
{
  if (!dpy)
    return;

#ifdef POINTER_BLANK
  XFreeCursor (dpy, blank_cursor);
#endif
  x_ev.stop ();
  flush_ev.stop ();
#ifdef USE_XIM
  xims.clear ();
#endif
  XrmDestroyDatabase (XrmGetDatabase (dpy));
  XCloseDisplay (dpy);
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

  if (XGetWindowProperty (dpy, root, xa[XA_XIM_SERVERS], 0L, 1000000L,
                          False, XA_ATOM, &actual_type, &actual_format,
                          &nitems, &bytes_after, (unsigned char **)&atoms)
      != Success)
    return;

  if (actual_type == XA_ATOM && actual_format == 32)
    for (int i = 0; i < nitems; i++)
      if (XGetSelectionOwner (dpy, atoms[i]))
        {
          im_change_cb ();
          break;
        }

  XFree (atoms);
}
#endif

void rxvt_display::x_cb (ev::io &w, int revents)
{
  flush_ev.start ();
}

void rxvt_display::flush_cb (ev::prepare &w, int revents)
{
  while (XEventsQueued (dpy, QueuedAfterFlush))
    do
      {
        XEvent xev;
        XNextEvent (dpy, &xev);

#ifdef USE_XIM
        if (!XFilterEvent (&xev, None))
          {
            if (xev.type == PropertyNotify
                && xev.xany.window == root
                && xev.xproperty.atom == xa[XA_XIM_SERVERS])
              im_change_check ();
#endif
            if (xev.type == MappingNotify)
              XRefreshKeyboardMapping (&xev.xmapping);

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
    while (XEventsQueued (dpy, QueuedAlready));

  w.stop ();
}

void rxvt_display::reg (xevent_watcher *w)
{
  if (!w->active)
    {
      xw.push_back (w);
      w->active = xw.size ();
    }
}

void rxvt_display::unreg (xevent_watcher *w)
{
  if (w->active)
    {
      xw[w->active - 1] = 0;
      w->active = 0;
    }
}

void rxvt_display::set_selection_owner (rxvt_term *owner, bool clipboard)
{
  rxvt_term * &cur_owner = !clipboard ? selection_owner : clipboard_owner;

  if (cur_owner && cur_owner != owner)
    {
      rxvt_term *term = cur_owner;
      term->selection_clear (clipboard);
      term->flush ();
    }

  cur_owner = owner;
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

  if (!(id = rxvt_temp_buf<char> (l + m + 2)))
    return 0;

  memcpy (id, locale, l); id[l] = '\n';
  memcpy (id + l + 1, modifiers, m); id[l + m + 1] = 0;

  rxvt_xim *xim = xims.get (id);

  return xim;
}

void rxvt_display::put_xim (rxvt_xim *xim)
{
# if XLIB_IS_RACEFREE
  xims.put (xim);
# endif
}

#endif

Atom rxvt_display::atom (const char *name)
{
  return XInternAtom (dpy, name, False);
}

/////////////////////////////////////////////////////////////////////////////

template class refcache<rxvt_display>;
refcache<rxvt_display> displays;

/////////////////////////////////////////////////////////////////////////////
//

static unsigned int
insert_component (unsigned int value, unsigned int mask, unsigned int shift)
{
  return (value * (mask + 1) >> 16) << shift;
}

bool
rxvt_color::alloc (rxvt_screen *screen, const rgba &color)
{
  //TODO: only supports 24 bit
  unsigned int alpha = color.a >= 0xff00 ? 0xffff : color.a;

#if XFT
  XRenderPictFormat *format;

  // FUCKING Xft gets it wrong, of course, so work around it.
  // Transparency users should eat shit and die, and then
  // XRenderQueryPictIndexValues themselves plenty.
  if ((screen->visual->c_class == TrueColor)
      && (format = XRenderFindVisualFormat (screen->dpy, screen->visual)))
    {
      // the fun lies in doing everything manually...
      c.color.red   = color.r;
      c.color.green = color.g;
      c.color.blue  = color.b;
      c.color.alpha = alpha;

      // Xft wants premultiplied alpha, but abuses the alpha channel
      // as blend factor, and doesn't allow us to set the alpha channel
      c.color.red   = c.color.red   * alpha / 0xffff;
      c.color.green = c.color.green * alpha / 0xffff;
      c.color.blue  = c.color.blue  * alpha / 0xffff;

      c.pixel = insert_component (c.color.red  , format->direct.redMask  , format->direct.red  )
              | insert_component (c.color.green, format->direct.greenMask, format->direct.green)
              | insert_component (c.color.blue , format->direct.blueMask , format->direct.blue )
              | insert_component (alpha        , format->direct.alphaMask, format->direct.alpha);

      return true;
    }
  else
    {
      XRenderColor d;

      d.red   = color.r;
      d.green = color.g;
      d.blue  = color.b;
      d.alpha = alpha;

      if (XftColorAllocValue (screen->dpy, screen->visual, screen->cmap, &d, &c))
        return true;
    }
#else
  c.red   = color.r;
  c.green = color.g;
  c.blue  = color.b;

  if (screen->visual->c_class == TrueColor)
    {
      c.pixel = (color.r >> (16 - ecb_popcount32 (screen->visual->red_mask  )) << ecb_ctz32 (screen->visual->red_mask  ))
              | (color.g >> (16 - ecb_popcount32 (screen->visual->green_mask)) << ecb_ctz32 (screen->visual->green_mask))
              | (color.b >> (16 - ecb_popcount32 (screen->visual->blue_mask )) << ecb_ctz32 (screen->visual->blue_mask ));

      return true;
    }
  else if (XAllocColor (screen->dpy, screen->cmap, &c))
    return true;
#endif

  c.pixel = (color.r + color.g + color.b) > 128*3
          ? WhitePixelOfScreen (DefaultScreenOfDisplay (screen->dpy))
          : BlackPixelOfScreen (DefaultScreenOfDisplay (screen->dpy));

  return false;
}

bool
rxvt_color::set (rxvt_screen *screen, const char *name)
{
  rgba c;
  char eos;
  int skip;

  c.a = rgba::MAX_CC;

  // parse the nonstandard "[alphapercent]" prefix
  if (1 <= sscanf (name, "[%hd]%n", &c.a, &skip))
    {
      c.a = lerp<int, int, int> (0, rgba::MAX_CC, c.a);
      name += skip;
    }

  // parse the non-standard "rgba:rrrr/gggg/bbbb/aaaa" format
  if (strlen (name) != 4+5*4 || 4 != sscanf (name, "rgba:%4hx/%4hx/%4hx/%4hx%c", &c.r, &c.g, &c.b, &c.a, &eos))
    {
      XColor xc;

      if (XParseColor (screen->dpy, screen->cmap, name, &xc))
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
rxvt_color::set (rxvt_screen *screen, const rgba &color)
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

      // many kilobytes transfer per colour, but pseudocolor isn't worth
      // many extra optimisations.
      XQueryColors (screen->dpy, screen->cmap, colors, cmap_size);

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

      got = alloc (screen, rgba (best->red, best->green, best->blue));

      delete [] colors;
    }
#endif

  return got;
}

void
rxvt_color::get (rgba &color)
{
#if XFT
  color.r = c.color.red;
  color.g = c.color.green;
  color.b = c.color.blue;
  color.a = c.color.alpha;
#else
  color.r = c.red;
  color.g = c.green;
  color.b = c.blue;
  color.a = rgba::MAX_CC;
#endif
}

void
rxvt_color::get (XColor &color)
{
  rgba c;
  get (c);

  color.red   = c.r;
  color.green = c.g;
  color.blue  = c.b;
  color.pixel = (Pixel)*this;
}

void
rxvt_color::free (rxvt_screen *screen)
{
  if (screen->visual->c_class == TrueColor)
    return; // nothing to do

#if XFT
  XftColorFree (screen->dpy, screen->visual, screen->cmap, &c);
#else
  XFreeColors (screen->dpy, screen->cmap, &c.pixel, 1, AllPlanes);
#endif
}

void
rxvt_color::fade (rxvt_screen *screen, int percent, rxvt_color &result, const rgba &to)
{
  rgba c;
  get (c);

  result.set (
    screen,
    rgba (
      lerp (c.r, to.r, percent),
      lerp (c.g, to.g, percent),
      lerp (c.b, to.b, percent),
      lerp (c.a, to.a, percent)
    )
  );
}

rxvt_selection::rxvt_selection (rxvt_display *disp, int selnum, Time tm, Window win, Atom prop, rxvt_term *term)
: display (disp), request_time (tm), request_win (win), request_prop (prop), term (term)
{
  assert (selnum >= Sel_Primary && selnum <= Sel_Clipboard);

  timer_ev.set<rxvt_selection, &rxvt_selection::timer_cb> (this);
  timer_ev.repeat = 10.;
  x_ev.set<rxvt_selection, &rxvt_selection::x_cb> (this);

  incr_buf = 0;
  incr_buf_size = incr_buf_fill = 0;
  selection_wait = Sel_normal;
  selection_type = selnum;
  cb_sv = 0;
}

void
rxvt_selection::stop ()
{
  free (incr_buf);
  incr_buf = 0;
  timer_ev.stop ();
  x_ev.stop (display);
}

rxvt_selection::~rxvt_selection ()
{
  stop ();
}

void
rxvt_selection::run ()
{
  int selnum = selection_type;

#if ENABLE_FRILLS
  if (selnum == Sel_Primary && display->selection_owner)
    {
      /* internal selection */
      char *str = rxvt_wcstombs (display->selection_owner->selection.text, display->selection_owner->selection.len);
      finish (str, strlen (str));
      free (str);
      return;
    }
#endif

#if X_HAVE_UTF8_STRING
  selection_type = Sel_UTF8String;
  if (request (display->xa[XA_UTF8_STRING], selnum))
    return;
#else
  selection_type = Sel_CompoundText;
  if (request (display->xa[XA_COMPOUND_TEXT], selnum))
    return;
#endif

  // fallback to CUT_BUFFER0 if the requested property has no owner
  handle_selection (display->root, XA_CUT_BUFFER0, false);
}

void
rxvt_selection::finish (char *data, unsigned int len)
{
  if (!cb_sv)
    {
      if (data)
        term->paste (data, len);

      term->selection_req = 0;
      delete this;
    }
#if ENABLE_PERL
  else
    {
      stop (); // we do not really trust perl callbacks
      rxvt_perl.selection_finish (this, data, len);
    }
#endif
}

bool
rxvt_selection::request (Atom target, int selnum)
{
  Atom sel;

  selection_type |= selnum;

  if (selnum == Sel_Primary)
    sel = XA_PRIMARY;
  else if (selnum == Sel_Secondary)
    sel = XA_SECONDARY;
  else
    sel = display->xa[XA_CLIPBOARD];

  if (XGetSelectionOwner (display->dpy, sel) != None)
    {
      XConvertSelection (display->dpy, sel, target, request_prop,
                         request_win, request_time);
      x_ev.start (display, request_win);
      timer_ev.again ();
      return true;
    }

  return false;
}

void
rxvt_selection::handle_selection (Window win, Atom prop, bool delete_prop)
{
  Display *dpy = display->dpy;
  char *data = 0;
  unsigned int data_len = 0;
  unsigned long bytes_after;
  XTextProperty ct;

  // check for failed XConvertSelection
  if (prop == None)
    {
      bool error = true;
      int selnum = selection_type & Sel_whereMask;

      if (selection_type & Sel_CompoundText)
        {
          selection_type = 0;
          error = !request (XA_STRING, selnum);
        }

      if (selection_type & Sel_UTF8String)
        {
          selection_type = Sel_CompoundText;
          error = !request (display->xa[XA_COMPOUND_TEXT], selnum);
        }

      if (error)
        {
          ct.value = 0;
          goto bailout;
        }

      return;
    }

  // length == (2^31 - 1) / 4, as gdk
  if (XGetWindowProperty (dpy, win, prop,
                          0, 0x1fffffff,
                          delete_prop, AnyPropertyType,
                          &ct.encoding, &ct.format,
                          &ct.nitems, &bytes_after,
                          &ct.value) != Success)
    {
      ct.value = 0;
      goto bailout;
    }

  if (ct.encoding == None)
    goto bailout;

  if (ct.value == 0)
    goto bailout;

  if (ct.encoding == display->xa[XA_INCR])
    {
      // INCR selection, start handshake
      if (!delete_prop)
        XDeleteProperty (dpy, win, prop);

      selection_wait = Sel_incr;
      timer_ev.again ();

      goto bailout;
    }

  if (ct.nitems == 0)
    {
      if (selection_wait == Sel_incr)
        {
          XFree (ct.value);

          // finally complete, now paste the whole thing
          selection_wait = Sel_normal;
          ct.value = (unsigned char *)incr_buf;
          ct.nitems = incr_buf_fill;
          incr_buf = 0;
          timer_ev.stop ();
        }
      else
        {
          // avoid recursion
          if (win != display->root || prop != XA_CUT_BUFFER0)
            {
              XFree (ct.value);

               // fallback to CUT_BUFFER0 if the requested property
               // has an owner but is empty
              handle_selection (display->root, XA_CUT_BUFFER0, False);
              return;
            }

          goto bailout;
        }
    }
  else if (selection_wait == Sel_incr)
    {
      timer_ev.again ();

      while (incr_buf_fill + ct.nitems > incr_buf_size)
        {
          incr_buf_size = incr_buf_size ? incr_buf_size * 2 : 128*1024;
          incr_buf = (char *)rxvt_realloc (incr_buf, incr_buf_size);
        }

      memcpy (incr_buf + incr_buf_fill, ct.value, ct.nitems);
      incr_buf_fill += ct.nitems;

      goto bailout;
    }

  char **cl;
  int cr;

  // we honour the first item only

#if !ENABLE_MINIMAL
  // xlib is horribly broken with respect to UTF8_STRING, and nobody cares to fix it
  // so recode it manually
  if (ct.encoding == display->xa[XA_UTF8_STRING])
    {
      wchar_t *w = rxvt_utf8towcs ((const char *)ct.value, ct.nitems);
      data = rxvt_wcstombs (w);
      free (w);
    }
  else
#endif
  if (XmbTextPropertyToTextList (dpy, &ct, &cl, &cr) >= 0
      && cl)
    {
      data = strdup (cl[0]);
      XFreeStringList (cl);
    }
  else
    {
      // paste raw
      data = strdup ((const char *)ct.value);
    }

  data_len = strlen (data);

bailout:
  XFree (ct.value);

  if (selection_wait == Sel_normal)
    {
      finish (data, data_len);
      free (data);
    }
}

void
rxvt_selection::timer_cb (ev::timer &w, int revents)
{
  if (selection_wait == Sel_incr)
    rxvt_warn ("data loss: timeout on INCR selection paste, ignoring.\n");

  finish ();
}

void
rxvt_selection::x_cb (XEvent &xev)
{
  switch (xev.type)
    {
      case PropertyNotify:
        if (selection_wait == Sel_incr
            && xev.xproperty.atom == request_prop
            && xev.xproperty.state == PropertyNewValue)
          handle_selection (xev.xproperty.window, xev.xproperty.atom, true);
        break;

      case SelectionNotify:
        if (selection_wait == Sel_normal
            && xev.xselection.time == request_time)
          {
            timer_ev.stop ();
            handle_selection (xev.xselection.requestor, xev.xselection.property, true);
          }
        break;
    }
}
