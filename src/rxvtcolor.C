#include "../config.h"
#include <rxvt.h>

// TODO: free colors again

bool
rxvt_color::set (pR_ Pixel p)
{
#if XFT
  XColor xc;

  xc.pixel = p;
  if (!XQueryColor (R->Xdisplay, XCMAP, &xc))
    return false;

  XRenderColor d;

  d.red   = xc.red;
  d.green = xc.green;
  d.blue  = xc.blue;
  d.alpha = 0xffff;

  return
    XftColorAllocValue (R->Xdisplay, 
                        XVISUAL,
                        XCMAP,
                        &d,
                        &c);
#else
  this->p = p;
#endif

  return true;
}

bool
rxvt_color::set (pR_ const char *name)
{
  XColor xc;

  if (XParseColor (R->Xdisplay, XCMAP, name, &xc))
    return set (aR_ xc.red, xc.green, xc.blue);

  return false;
}

bool
rxvt_color::set (pR_ unsigned short cr, unsigned short cg, unsigned short cb)
{
  XColor xc;

  xc.red   = cr;
  xc.green = cg;
  xc.blue  = cb;
  xc.flags = DoRed | DoGreen | DoBlue;

  if (XAllocColor (R->Xdisplay, XCMAP, &xc))
    return set (aR_ xc.pixel);

  return false;
}

void 
rxvt_color::get (pR_ unsigned short &cr, unsigned short &cg, unsigned short &cb)
{
#if XFT
  cr = c.color.red;
  cg = c.color.green;
  cb = c.color.blue;
#else
  XColor c;

  c.pixel = p;
  XQueryColor (R->Xdisplay, XCMAP, &c);

  cr = c.red;
  cg = c.green;
  cb = c.blue;
#endif
}

