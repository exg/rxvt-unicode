#include "../config.h"
#include <rxvt.h>
#include <rxvtcolor.h>

#include <unistd.h>
#include <fcntl.h>

class byteorder byteorder;

byteorder::byteorder ()
{
  union {
    uint32_t u;
    uint8_t b[4];
  } w;

  w.b[0] = 0x11;
  w.b[1] = 0x22;
  w.b[2] = 0x33;
  w.b[3] = 0x44;

  e = w.u;
}

refcounted::refcounted (const char *id)
{
  this->id = STRDUP (id);
}

refcounted::~refcounted ()
{
  free (id);
}

template<class T>
T *refcache<T>::get (const char *id)
{
  for (T **i = begin (); i < end (); ++i)
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
      push_back (obj);
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
      erase (find (begin (), end (), obj));
      delete obj;
    }
}

template<class T>
refcache<T>::~refcache ()
{
  while (size ())
    put (*begin ());
}

/////////////////////////////////////////////////////////////////////////////

static void
im_destroy_cb (XIM unused1, XPointer client_data, XPointer unused3)
{
  rxvt_xim *xim = (rxvt_xim *)client_data;
  rxvt_display *display = xim->display;

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

/////////////////////////////////////////////////////////////////////////////

rxvt_display::rxvt_display (const char *id)
: refcounted (id)
, x_ev (this, &rxvt_display::x_cb)
, selection_owner (0)
{
}

bool rxvt_display::init ()
{
  display = XOpenDisplay (id);

  if (!display)
    return false;

  screen = DefaultScreen (display);
  root   = DefaultRootWindow (display);
  visual = DefaultVisual (display, screen);
  cmap   = DefaultColormap (display, screen);
  depth  = DefaultDepth (display, screen);

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

  int fd = XConnectionNumber (display);
  x_ev.start (fd, EVENT_READ);
  fcntl (fd, F_SETFL, FD_CLOEXEC);

  XSelectInput (display, root, PropertyChangeMask);
  xa_xim_servers = XInternAtom (display, "XIM_SERVERS", 0);

  flush ();

  return true;
}

rxvt_display::~rxvt_display ()
{
  x_ev.stop ();

  if (display)
    XCloseDisplay (display);
}

void rxvt_display::im_change_cb ()
{
  for (im_watcher **i = imw.begin (); i != imw.end (); ++i)
    (*i)->call ();
}

void rxvt_display::x_cb (io_watcher &w, short revents)
{
  do
    {
      XEvent xev;
      XNextEvent (display, &xev);

      //printf ("T %d w %lx\n", xev.type, xev.xany.window);//D

      if (xev.type == PropertyNotify
          && xev.xany.window == root
          && xev.xproperty.atom == xa_xim_servers)
        im_change_cb ();

      for (int i = xw.size (); i--; )
        {
          if (!xw[i])
            xw.erase_unordered (i);
          else if (xw[i]->window == xev.xany.window)
            xw[i]->call (xev);
        }
    }
  while (XPending (display));

  flush ();
}

void rxvt_display::flush ()
{
  for (;;)
    {
      XFlush (display);

      if (!XPending (display))
        break;

      x_cb (x_ev, 0);
    }
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

void rxvt_display::reg (im_watcher *w)
{
  imw.push_back (w);
}

void rxvt_display::unreg (im_watcher *w)
{
  imw.erase (find (imw.begin (), imw.end (), w));
}

void rxvt_display::set_selection_owner (rxvt_term *owner)
{
  if (selection_owner && selection_owner != owner)
    selection_owner->selection_clear ();

  selection_owner = owner;
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
  xims.put (xim);
}

Atom rxvt_display::atom (const char *name)
{
  return XInternAtom (display, name, False);
}

/////////////////////////////////////////////////////////////////////////////

template refcache<rxvt_display>;
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
  XColor xc;

  if (XParseColor (display->display, display->cmap, name, &xc))
    return set (display, xc.red, xc.green, xc.blue);

  return false;
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

