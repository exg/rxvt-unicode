/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar-rxvt.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar-rxvt.C,v 1.3 2003/11/25 11:52:42 pcg Exp $
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
#include "scrollbar-rxvt.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/
#if defined(RXVT_SCROLLBAR)

/* draw triangular button with a shadow of SHADOW (1 or 2) pixels */
/* INTPROTO */
void
rxvt_Draw_button(pR_ int x, int y, int state, int dirn)
{
    unsigned int    sz, sz2;
    XPoint          pt[3];
    GC              top, bot;

    sz = R->scrollBar.width;
    sz2 = sz / 2;
    switch (state) {
    case +1:
	top = R->topShadowGC;
	bot = R->botShadowGC;
	break;
    case -1:
	top = R->botShadowGC;
	bot = R->topShadowGC;
	break;
    default:
	top = bot = R->scrollbarGC;
	break;
    }

/* fill triangle */
    pt[0].x = x;
    pt[1].x = x + sz - 1;
    pt[2].x = x + sz2;
    if (dirn == UP) {
	pt[0].y = pt[1].y = y + sz - 1;
	pt[2].y = y;
    } else {
	pt[0].y = pt[1].y = y;
	pt[2].y = y + sz - 1;
    }
    XFillPolygon(R->Xdisplay, R->scrollBar.win, R->scrollbarGC,
		 pt, 3, Convex, CoordModeOrigin);

/* draw base */
    XDrawLine(R->Xdisplay, R->scrollBar.win, (dirn == UP ? bot : top),
	      pt[0].x, pt[0].y, pt[1].x, pt[1].y);

/* draw shadow on left */
    pt[1].x = x + sz2 - 1;
    pt[1].y = y + (dirn == UP ? 0 : sz - 1);
    XDrawLine(R->Xdisplay, R->scrollBar.win, top,
	      pt[0].x, pt[0].y, pt[1].x, pt[1].y);

#if (SHADOW > 1)
/* doubled */
    pt[0].x++;
    if (dirn == UP) {
	pt[0].y--;
	pt[1].y++;
    } else {
	pt[0].y++;
	pt[1].y--;
    }
    XDrawLine(R->Xdisplay, R->scrollBar.win, top,
	      pt[0].x, pt[0].y, pt[1].x, pt[1].y);
#endif
/* draw shadow on right */
    pt[1].x = x + sz - 1;
/*  pt[2].x = x + sz2; */
    pt[1].y = y + (dirn == UP ? sz - 1 : 0);
    pt[2].y = y + (dirn == UP ? 0 : sz - 1);
    XDrawLine(R->Xdisplay, R->scrollBar.win, bot,
	      pt[2].x, pt[2].y, pt[1].x, pt[1].y);
#if (SHADOW > 1)
/* doubled */
    pt[1].x--;
    if (dirn == UP) {
	pt[2].y++;
	pt[1].y--;
    } else {
	pt[2].y--;
	pt[1].y++;
    }
    XDrawLine(R->Xdisplay, R->scrollBar.win, bot,
	      pt[2].x, pt[2].y, pt[1].x, pt[1].y);
#endif
}

/* EXTPROTO */
int
rxvt_scrollbar_show_rxvt(pR_ int update __attribute__((unused)), int last_top, int last_bot, int scrollbar_len)
{
    int             sbshadow = R->sb_shadow;
    int             sbwidth = (int)R->scrollBar.width;

    if ((R->scrollBar.init & R_SB_RXVT) == 0) {
	XGCValues       gcvalue;

	R->scrollBar.init |= R_SB_RXVT;
	gcvalue.foreground = R->PixColors[Color_trough];
	if (sbshadow) {
	    XSetWindowBackground(R->Xdisplay, R->scrollBar.win,
				 gcvalue.foreground);
	    XClearWindow(R->Xdisplay, R->scrollBar.win);
	}
    } else {
/* instead of XClearWindow (R->Xdisplay, R->scrollBar.win); */
	if (last_top < R->scrollBar.top)
	    XClearArea(R->Xdisplay, R->scrollBar.win,
		       sbshadow, last_top,
		       sbwidth, (R->scrollBar.top - last_top),
		       False);

	if (R->scrollBar.bot < last_bot)
	    XClearArea(R->Xdisplay, R->scrollBar.win,
		       sbshadow, R->scrollBar.bot,
		       sbwidth, (last_bot - R->scrollBar.bot),
		       False);
    }

/* scrollbar slider */
#ifdef SB_BORDER
    {
	int             xofs;

	if (R->Options & Opt_scrollBar_right)
	    xofs = 0;
	else
	    xofs = sbshadow ? sbwidth : sbwidth - 1;

	XDrawLine(R->Xdisplay, R->scrollBar.win, R->botShadowGC,
		  xofs, 0, xofs, R->scrollBar.end + sbwidth);
    }
#endif
    XFillRectangle(R->Xdisplay, R->scrollBar.win, R->scrollbarGC,
		   sbshadow, R->scrollBar.top, sbwidth,
		   scrollbar_len);

    if (sbshadow)
	/* trough shadow */
	rxvt_Draw_Shadow(R->Xdisplay, R->scrollBar.win,
			 R->botShadowGC, R->topShadowGC,
			 0, 0,
			 sbwidth + 2 * sbshadow, /* scrollbar_TotalWidth() */
			 R->scrollBar.end + (sbwidth + 1) + sbshadow);
/* shadow for scrollbar slider */
    rxvt_Draw_Shadow(R->Xdisplay, R->scrollBar.win,
		     R->topShadowGC, R->botShadowGC,
		     sbshadow, R->scrollBar.top, sbwidth,
		     scrollbar_len);

/*
 * Redraw scrollbar arrows
 */
    rxvt_Draw_button(aR_ sbshadow, sbshadow,
		     (scrollbar_isUp() ? -1 : +1), UP);
    rxvt_Draw_button(aR_ sbshadow, (R->scrollBar.end + 1),
		     (scrollbar_isDn() ? -1 : +1), DN);
    return 1;
}
#endif				/* RXVT_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/
