/*----------------------------------------------------------------------*
 * File:	scrollbar-xterm.C
 *----------------------------------------------------------------------*
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004-2006 Marc Lehmann <schmorp@schmorp.de>
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

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */

/*----------------------------------------------------------------------*/
#if defined(XTERM_SCROLLBAR)

#define x_stp_width	8
#define x_stp_height	2
const unsigned char x_stp_bits[] = { 0xaa, 0x55 };

int
scrollBar_t::show_xterm (int update)
{
  int xsb = 0;
  int sbwidth = width - 1;

  if ((init & SB_STYLE_XTERM) == 0)
    {
      XGCValues       gcvalue;

      init |= SB_STYLE_XTERM;
      gcvalue.stipple = XCreateBitmapFromData (term->dpy, win,
                                               (char *)x_stp_bits, x_stp_width,
                                               x_stp_height);
      if (!gcvalue.stipple)
        rxvt_fatal ("can't create bitmap\n");

      gcvalue.fill_style = FillOpaqueStippled;
      gcvalue.foreground = term->pix_colors_focused[Color_scroll];
      gcvalue.background = term->pix_colors_focused[Color_bg];

      xscrollbarGC = XCreateGC (term->dpy, win,
                                GCForeground | GCBackground
                                | GCFillStyle | GCStipple, &gcvalue);
      gcvalue.foreground = term->pix_colors_focused[Color_border];
      ShadowGC = XCreateGC (term->dpy, win, GCForeground, &gcvalue);
    }

  xsb = term->option (Opt_scrollBar_right) ? 1 : 0;

  if (update)
    {
      if (last_top < top)
        XClearArea (term->dpy, win,
                    xsb, last_top,
                    sbwidth, (top - last_top), False);

      if (bot < last_bot)
        XClearArea (term->dpy, win,
                    xsb, bot,
                    sbwidth, (last_bot - bot), False);
    }
  else
    XClearWindow (term->dpy, win);

  /* scrollbar slider */
  XFillRectangle (term->dpy, win, xscrollbarGC,
                  xsb + 1, top, sbwidth - 2, bot - top);

  XDrawLine (term->dpy, win, ShadowGC,
             xsb ? 0 : sbwidth, beg,
             xsb ? 0 : sbwidth, end);
  return 1;
}
#endif /* XTERM_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/
