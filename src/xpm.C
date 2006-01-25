/*--------------------------------*-C-*---------------------------------*
 * File:	xpm.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997      Carsten Haitzler <raster@zip.com.au>
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
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
 *---------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */

#ifdef XPM_BACKGROUND

static void      rxvt_pixmap_incr                 (unsigned int *wh, unsigned int *xy, float *incr, float *p, unsigned int widthheight, unsigned int xpmwidthheight);

/*
 * These GEOM strings indicate absolute size/position:
 * @ `WxH+X+Y'
 * @ `WxH+X'    -> Y = X
 * @ `WxH'      -> Y = X = 50
 * @ `W+X+Y'    -> H = W
 * @ `W+X'      -> H = W, Y = X
 * @ `W'        -> H = W, X = Y = 50
 * @ `0xH'      -> H *= H/100, X = Y = 50 (W unchanged)
 * @ `Wx0'      -> W *= W/100, X = Y = 50 (H unchanged)
 * @ `=+X+Y'    -> (H, W unchanged)
 * @ `=+X'      -> Y = X (H, W unchanged)
 *
 * These GEOM strings adjust position relative to current position:
 * @ `+X+Y'
 * @ `+X'       -> Y = X
 *
 * And this GEOM string is for querying current scale/position:
 * @ `?'
 */
int
rxvt_term::scale_pixmap (const char *geom)
{
  int flags, changed = 0;
  int x = 0, y = 0;
  unsigned int w = 0, h = 0;
  unsigned int n;
  char *p;
  bgPixmap_t *bgpixmap = & (bgPixmap);

#define MAXLEN_GEOM		sizeof("[10000x10000+10000+10000]")

  if (geom == NULL)
    return 0;

  char str[MAXLEN_GEOM];

  if (!strcmp (geom, "?"))
    {
      sprintf (str, "[%dx%d+%d+%d]",	/* can't presume snprintf () ! */
              min (bgpixmap->w, 32767), min (bgpixmap->h, 32767),
              min (bgpixmap->x, 32767), min (bgpixmap->y, 32767));
      process_xterm_seq (XTerm_title, str, CHAR_ST);
      return 0;
    }

  if ((p = strchr (geom, ';')) == NULL)
    p = strchr (geom, '\0');

  n = (p - geom);
  if (n <= MAXLEN_GEOM)
    {
      strncpy (str, geom, n);
      str[n] = '\0';

      flags = XParseGeometry (str, &x, &y, &w, &h);

      if (!flags)
        {
          flags |= WidthValue;
          w = 0;
        }			/* default is tile */

      if (flags & WidthValue)
        {
          if (!(flags & XValue))
            x = 50;

          if (!(flags & HeightValue))
            h = w;

          if (w && !h)
            {
              w = (bgpixmap->w * w) / 100;
              h = bgpixmap->h;
            }
          else if (h && !w)
            {
              w = bgpixmap->w;
              h = (bgpixmap->h * h) / 100;
            }

          min_it (w, 32767);
          min_it (h, 32767);

          if (bgpixmap->w != (short)w)
            {
              bgpixmap->w = (short)w;
              changed++;
            }

          if (bgpixmap->h != (short)h)
            {
              bgpixmap->h = (short)h;
              changed++;
            }
        }

      if (!(flags & YValue))
        {
          if (flags & XNegative)
            flags |= YNegative;

          y = x;
        }

      if (!(flags & WidthValue) && geom[0] != '=')
        {
          x += bgpixmap->x;
          y += bgpixmap->y;
        }

      if (xpmAttr.width && xpmAttr.height)
        {
          x = MOD(x, xpmAttr.width);
          y = MOD(y, xpmAttr.height);
        }

      if (bgpixmap->x != x)
        {
          bgpixmap->x = x;
          changed++;
        }

      if (bgpixmap->y != y)
        {
          bgpixmap->y = y;
          changed++;
        }
    }

  return changed;
}

void
rxvt_term::resize_pixmap ()
{
  XGCValues gcvalue;
  GC gc;

  if (pixmap != None)
    XFreePixmap (xdisp, pixmap);

  if (bgPixmap.pixmap == None)
    { /* So be it: I'm not using pixmaps */
      pixmap = None;

      if (!OPTION (Opt_transparent) || !am_transparent)
        XSetWindowBackground (xdisp, vt, pix_colors[Color_bg]);

      return;
    }

  gcvalue.foreground = pix_colors[Color_bg];
  gc = XCreateGC (xdisp, vt, GCForeground, &gcvalue);

  if (bgPixmap.pixmap != None)
    {	/* we have a specified pixmap */
      unsigned int w = bgPixmap.w, h = bgPixmap.h,
                   x = bgPixmap.x, y = bgPixmap.y;
      unsigned int xpmh = xpmAttr.height,
                   xpmw = xpmAttr.width;

      /*
       * don't zoom pixmap too much nor expand really small pixmaps
       */
      if (w > 32767 || h > 32767)
        w = 1;
      else if (width > (10 * xpmw)
               || height > (10 * xpmh))
        w = 0;		/* tile */

      if (!w)
        {
          /* basic X tiling - let the X server do it */
          pixmap = XCreatePixmap (xdisp, vt, xpmw, xpmh, depth);

          XCopyArea (xdisp, bgPixmap.pixmap, pixmap, gc, x, y, xpmw - x, xpmh - y,        0,        0);
          XCopyArea (xdisp, bgPixmap.pixmap, pixmap, gc, x, 0, xpmw - x,        y,        0, xpmh - y);
          XCopyArea (xdisp, bgPixmap.pixmap, pixmap, gc, 0, y,        x, xpmh - y, xpmw - x,        0);
          XCopyArea (xdisp, bgPixmap.pixmap, pixmap, gc, 0, 0,        x,        y, xpmw - x, xpmh - y);
        }
      else
        {
          float incr, p;
          Pixmap tmp;

          pixmap = XCreatePixmap (xdisp, vt, width, height, depth);
          /*
           * horizontal scaling
           */
          rxvt_pixmap_incr (&w, &x, &incr, &p, width, xpmw);

          tmp = XCreatePixmap (xdisp, vt, width, xpmh, depth);
          XFillRectangle (xdisp, tmp, gc, 0, 0, width, xpmh);

          for ( /*nil */ ; x < w; x++, p += incr)
            {
              if (p >= xpmw)
                p = 0;

              /* copy one column from the original pixmap to the tmp pixmap */
              XCopyArea (xdisp, bgPixmap.pixmap, tmp, gc, (int)p, 0, 1, xpmh, (int)x, 0);
            }

          /*
           * vertical scaling
           */
          rxvt_pixmap_incr (&h, &y, &incr, &p, height, xpmh);

          if (y > 0)
            XFillRectangle (xdisp, pixmap, gc, 0, 0, width, y);

          if (h < height)
            XFillRectangle (xdisp, pixmap, gc, 0, (int)h, width, height - h + 1);

          for ( /*nil */ ; y < h; y++, p += incr)
            {
              if (p >= xpmh)
                p = 0;

              /* copy one row from the tmp pixmap to the main pixmap */
              XCopyArea (xdisp, tmp, pixmap, gc, 0, (int)p, width, 1, 0, (int)y);
            }

          XFreePixmap (xdisp, tmp);
        }
    }

  XSetWindowBackgroundPixmap (xdisp, vt, pixmap);

  if (pixmap != None)
    {
      XFreePixmap (xdisp, pixmap);
      pixmap = None;
    }

  XFreeGC (xdisp, gc);
  am_transparent = 0;
}

/*
 * Calculate tiling sizes and increments
 * At start, p == 0, incr == xpmwidthheight
 */
static void
rxvt_pixmap_incr (unsigned int *wh, unsigned int *xy, float *incr, float *p, unsigned int widthheight, unsigned int xpmwidthheight)
{
  unsigned int    cwh, cxy;
  float           cincr, cp;

  cp = 0;
  cincr = (float)xpmwidthheight;
  cxy = *xy;
  cwh = *wh;
  if (cwh == 1)
    {	/* display one image, no horizontal/vertical scaling */
      cincr = (float)widthheight;
      if (xpmwidthheight <= widthheight)
        {
          cwh = xpmwidthheight;
          cxy = (cxy * (widthheight - cwh)) / 100;	/* beware! order */
          cwh += cxy;
        }
      else
        {
          cxy = 0;
          cwh = widthheight;
        }
    }
  else if (cwh < 10)
    {	/* fit WH images across/down screen */
      cincr *= cwh;
      cxy = 0;
      cwh = widthheight;
    }
  else
    {
      cincr *= 100.0 / cwh;
      if (cwh < 100)
        {	/* contract */
          float           pos;

          cwh = (cwh * widthheight) / 100;
          pos = (float)cxy / 100 * widthheight - (cwh / 2);

          cxy = (widthheight - cwh);
          if (pos <= 0)
            cxy = 0;
          else if (pos < cxy)
            cxy = (int) pos;
          cwh += cxy;
        }
      else
        {	/* expand */
          if (cxy > 0)
            {	/* position */
              float           pos;

              pos = (float)cxy / 100 * xpmwidthheight - (cincr / 2);
              cp = xpmwidthheight - cincr;
              if (pos <= 0)
                cp = 0;
              else if (pos < cp)
                cp = pos;
            }
          cxy = 0;
          cwh = widthheight;
        }
    }
  cincr /= widthheight;
  *wh = cwh;
  *xy = cxy;
  *incr = cincr;
  *p = cp;
}

Pixmap
rxvt_term::set_bgPixmap (const char *file)
{
  char *f;

  assert (file != NULL);

  if (bgPixmap.pixmap != None)
    {
      XFreePixmap (xdisp, bgPixmap.pixmap);
      bgPixmap.pixmap = None;
    }

  XSetWindowBackground (xdisp, vt, pix_colors[Color_bg]);

  if (*file != '\0')
    {
      /*      XWindowAttributes attr; */

      /*
       * we already have the required attributes
       */
      /*      XGetWindowAttributes (xdisp, vt, &attr); */

      xpmAttr.closeness = 30000;
      xpmAttr.colormap = cmap;
      xpmAttr.visual = visual;
      xpmAttr.depth = depth;
      xpmAttr.valuemask = (XpmCloseness | XpmColormap | XpmVisual
                           | XpmDepth | XpmSize | XpmReturnPixels);

      /* search environment variables here too */
      f = (char *)rxvt_File_find (file, ".xpm", rs[Rs_path]);
      if (f == NULL
          || XpmReadFileToPixmap (xdisp, display->root, f,
                                  &bgPixmap.pixmap, NULL,
                                  &xpmAttr))
        {
          char *p;

          /* semi-colon delimited */
          if ((p = strchr (file, ';')) == NULL)
            p = strchr (file, '\0');

          rxvt_warn ("couldn't load XPM file \"%.*s\", ignoring.\n", (p - file), file);
        }

      free (f);
    }

  resize_pixmap ();
  return bgPixmap.pixmap;
}

#endif				/* XPM_BACKGROUND */
