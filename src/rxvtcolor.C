#include "../config.h"
#include <rxvt.h>

// TODO: free colors again

bool
rxvt_color::set (rxvt_term *t, Pixel p)
{
#if XFT
  XColor xc;

  xc.pixel = p;
  if (!XQueryColor (t->Xdisplay, t->Xcmap, &xc))
    return false;

  XRenderColor d;

  d.red   = xc.red;
  d.green = xc.green;
  d.blue  = xc.blue;
  d.alpha = 0xffff;

  return
    XftColorAllocValue (t->Xdisplay, 
                        t->Xvisual,
                        t->Xcmap,
                        &d,
                        &c);
#else
  this->p = p;
#endif

  return true;
}

bool
rxvt_color::set (rxvt_term *t, const char *name)
{
  XColor xc;

  if (XParseColor (t->Xdisplay, t->Xcmap, name, &xc))
    return set (t, xc.red, xc.green, xc.blue);

  return false;
}

bool
rxvt_color::set (rxvt_term *t, unsigned short cr, unsigned short cg, unsigned short cb)
{
  XColor xc;

  xc.red   = cr;
  xc.green = cg;
  xc.blue  = cb;
  xc.flags = DoRed | DoGreen | DoBlue;

  if (XAllocColor (t->Xdisplay, t->Xcmap, &xc))
    return set (t, xc.pixel);

  return false;
}

void 
rxvt_color::get (rxvt_term *t, unsigned short &cr, unsigned short &cg, unsigned short &cb)
{
#if XFT
  cr = c.color.red;
  cg = c.color.green;
  cb = c.color.blue;
#else
  XColor c;

  c.pixel = p;
  XQueryColor (t->Xdisplay, t->Xcmap, &c);

  cr = c.red;
  cg = c.green;
  cb = c.blue;
#endif
}

