#ifndef RXVT_COLOR_H
#define RXVT_COLOR_H

#include <X11/Xlib.h>

#if XFT
# include <X11/Xft/Xft.h>
#endif

#include "rxvtlib.h"

struct rxvt_vars;

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

   void get (pR_ unsigned short &cr, unsigned short &cg, unsigned short &cb);
  
   bool set (pR_ Pixel p);
   bool set (pR_ const char *name);
   bool set (pR_ unsigned short cr, unsigned short cg, unsigned short cb);
};

#endif

