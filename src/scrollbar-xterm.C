/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar-xterm.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar-xterm.C,v 1.3 2003/11/25 11:52:42 pcg Exp $
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
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
const unsigned char x_stp_bits[] = { 0xff, 0xff };

/* EXTPROTO */
int
rxvt_scrollbar_show_xterm(pR_ int update __attribute__((unused)), int last_top, int last_bot, int scrollbar_len)
{
    int             xsb = 0;
    int             sbwidth = R->scrollBar.width - 1;

    if ((R->scrollBar.init & R_SB_XTERM) == 0) {
	XGCValues       gcvalue;

	R->scrollBar.init |= R_SB_XTERM;
	gcvalue.stipple = XCreateBitmapFromData(R->Xdisplay, R->scrollBar.win,
						(char *)x_stp_bits, x_stp_width,
						x_stp_height);
	if (!gcvalue.stipple) {
	    rxvt_print_error("can't create bitmap");
	    exit(EXIT_FAILURE);
	}
	gcvalue.fill_style = FillOpaqueStippled;
	gcvalue.foreground = R->PixColors[Color_fg];
	gcvalue.background = R->PixColors[Color_bg];

	R->xscrollbarGC = XCreateGC(R->Xdisplay, R->scrollBar.win,
				       GCForeground | GCBackground
				       | GCFillStyle | GCStipple, &gcvalue);
	gcvalue.foreground = R->PixColors[Color_border];
	R->ShadowGC = XCreateGC(R->Xdisplay, R->scrollBar.win, GCForeground,
				   &gcvalue);
    }
/* instead of XClearWindow (R->Xdisplay, R->scrollBar.win); */
    xsb = (R->Options & Opt_scrollBar_right) ? 1 : 0;
    if (last_top < R->scrollBar.top)
	XClearArea(R->Xdisplay, R->scrollBar.win,
		   R->sb_shadow + xsb, last_top,
		   sbwidth + 1, (R->scrollBar.top - last_top), False);

    if (R->scrollBar.bot < last_bot)
	XClearArea(R->Xdisplay, R->scrollBar.win,
		   R->sb_shadow + xsb, R->scrollBar.bot,
		   sbwidth + 1, (last_bot - R->scrollBar.bot), False);

/* scrollbar slider */
    XFillRectangle(R->Xdisplay, R->scrollBar.win, R->xscrollbarGC,
		   xsb + 1, R->scrollBar.top, sbwidth, scrollbar_len);

    /*XDrawLine(R->Xdisplay, R->scrollBar.win, R->ShadowGC,
	      xsb ? 0 : sbwidth, R->scrollBar.beg,
	      xsb ? 0 : sbwidth, R->scrollBar.end);*/
    return 1;
}
#endif				/* XTERM_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/
