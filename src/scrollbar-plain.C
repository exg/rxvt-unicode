/*----------------------------------------------------------------------*
 * File:	scrollbar-plain.C
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
#include "scrollbar-plain.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/
#if defined(PLAIN_SCROLLBAR)

int
rxvt_term::scrollbar_show_plain (int update __attribute__ ((unused)), int last_top, int last_bot, int scrollbar_len)
{
  int xsb = 0;
  int sbwidth = scrollBar.width - 1;

  if ((scrollBar.init & R_SB_PLAIN) == 0)
    {
      XGCValues gcvalue;

      scrollBar.init |= R_SB_PLAIN;
      gcvalue.foreground = PixColors[Color_fg];
      gcvalue.background = PixColors[Color_bg];

      pscrollbarGC = XCreateGC (display->display, scrollBar.win,
                                GCForeground | GCBackground, &gcvalue);
    }
  /* instead of XClearWindow (display->display, scrollBar.win); */
  xsb = (Options & Opt_scrollBar_right) ? 1 : 0;
  if (last_top < scrollBar.top)
    XClearArea (display->display, scrollBar.win,
                sb_shadow + xsb, last_top,
                sbwidth + 1, (scrollBar.top - last_top), False);

  if (scrollBar.bot < last_bot)
    XClearArea (display->display, scrollBar.win,
                sb_shadow + xsb, scrollBar.bot,
                sbwidth + 1, (last_bot - scrollBar.bot), False);

  /* scrollbar slider */
  XFillRectangle (display->display, scrollBar.win, pscrollbarGC,
                  xsb + 1, scrollBar.top, sbwidth, scrollbar_len);

  return 1;
}

#endif

