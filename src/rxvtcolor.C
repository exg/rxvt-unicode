#include "../config.h"
#include <rxvt.h>
#include <rxvtcolor.h>

#include <unistd.h>
#include <fcntl.h>

/////////////////////////////////////////////////////////////////////////////

rxvt_display::rxvt_display (const char *name)
: x_watcher (this, &rxvt_display::x_event)
{
  this->name = STRDUP (name);
}

rxvt_display::~rxvt_display ()
{
  free (name);
}

bool rxvt_display::open ()
{
  display = XOpenDisplay (name);

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
  x_watcher.start (fd, EVENT_READ);
  fcntl (fd, F_SETFL, FD_CLOEXEC);

  return true;
}

void rxvt_display::close ()
{
  x_watcher.stop ();

  XCloseDisplay (display);
}

void rxvt_display::x_event (io_watcher &w, short revents)
{
  do
    {
      XEvent xev;
      XNextEvent (display, &xev);

      for (int i = xw.size (); i--; )
        {
          if (!xw[i])
            xw.erase_unordered (i);
          else if (xw[i]->window == xev.xany.window)
            xw[i]->call (xev);
        }
    }
  while (XPending (display));
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

/////////////////////////////////////////////////////////////////////////////

rxvt_displays displays;

rxvt_display *rxvt_displays::get (const char *name)
{
  for (rxvt_display **i = list.begin (); i < list.end (); ++i)
    {
      if (!strcmp (name, (*i)->name))
        {
          (*i)->referenced++;
          return *i;
        }
    }

  rxvt_display *display = new rxvt_display (name);

  display->referenced = 1;

  if (display && display->open ())
    list.push_back (display);
  else
    {
      delete display;
      display = 0;
    }

  return display;
}

void rxvt_displays::release (rxvt_display *display)
{
  if (!--display->referenced)
    {
      display->close ();
      delete display;
      list.erase (find (list.begin (), list.end (), display));
    }
}

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
  XFreeColors (display->display, display->cmap, &c, 1, AllPlanes);
#endif
}

