/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar-xterm.C
 *----------------------------------------------------------------------*
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004      Marc Lehmann <pcg@goof.com>
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
#include "scrollbar-xterm.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/
#if defined(XTERM_SCROLLBAR)

#define x_stp_width	8
#define x_stp_height	2
const unsigned char x_stp_bits[] = { 0xaa, 0x55 };

int
rxvt_term::scrollbar_show_xterm (int update __attribute__ ((unused)), int last_top, int last_bot, int scrollbar_len)
{
  int             xsb = 0;
  int             sbwidth = scrollBar.width - 1;

  if ((scrollBar.init & R_SB_XTERM) == 0)
    {
      XGCValues       gcvalue;

      scrollBar.init |= R_SB_XTERM;
      gcvalue.stipple = XCreateBitmapFromData (display->display, scrollBar.win,
                                              (char *)x_stp_bits, x_stp_width,
                                              x_stp_height);
      if (!gcvalue.stipple)
        rxvt_fatal ("can't create bitmap");

      gcvalue.fill_style = FillOpaqueStippled;
      gcvalue.foreground = PixColors[Color_fg];
      gcvalue.background = PixColors[Color_bg];

      xscrollbarGC = XCreateGC (display->display, scrollBar.win,
                                GCForeground | GCBackground
                                | GCFillStyle | GCStipple, &gcvalue);
      gcvalue.foreground = PixColors[Color_border];
      ShadowGC = XCreateGC (display->display, scrollBar.win, GCForeground, &gcvalue);
    }
  /* instead of XClearWindow (display->display, scrollBar.win); */
  xsb = (Options & Opt_scrollBar_right) ? 1 : 0;
  if (last_top < scrollBar.top)
    XClearArea (display->display, scrollBar.win,
               sb_shadow + xsb, last_top,
               sbwidth, (scrollBar.top - last_top), False);

  if (scrollBar.bot < last_bot)
    XClearArea (display->display, scrollBar.win,
               sb_shadow + xsb, scrollBar.bot,
               sbwidth, (last_bot - scrollBar.bot), False);

  /* scrollbar slider */
  XFillRectangle (display->display, scrollBar.win, xscrollbarGC,
                 xsb + 1, scrollBar.top, sbwidth - 2, scrollbar_len);

  XDrawLine (display->display, scrollBar.win, ShadowGC,
            xsb ? 0 : sbwidth, scrollBar.beg,
            xsb ? 0 : sbwidth, scrollBar.end);
  return 1;
}
#endif				/* XTERM_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/
