/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar-next.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar-next.C,v 1.4 2003/12/18 02:07:12 pcg Exp $
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 *				- N*XTstep like scrollbars
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
#include "scrollbar-next.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/
#if defined(NEXT_SCROLLBAR)

#define n_stp_width	8
#define n_stp_height	2
const unsigned char n_stp_bits[] = { 0x55, 0xaa };

/*
 * N*XTSTEP like scrollbar - written by Alfredo K. Kojima
 */
#define SCROLLER_DIMPLE_WIDTH   6
#define SCROLLER_DIMPLE_HEIGHT  6
#define ARROW_WIDTH   13
#define ARROW_HEIGHT  13

const char     *const SCROLLER_DIMPLE[] = {
    ".%###.",
    "%#%%%%",
    "#%%...",
    "#%..  ",
    "#%.   ",
    ".%.  ."
};
const char     *const SCROLLER_ARROW_UP[] = {
    ".............",
    ".............",
    "......%......",
    "......#......",
    ".....%#%.....",
    ".....###.....",
    "....%###%....",
    "....#####....",
    "...%#####%...",
    "...#######...",
    "..%#######%..",
    ".............",
    "............."
};
const char     *const SCROLLER_ARROW_DOWN[] = {
    ".............",
    ".............",
    "..%#######%..",
    "...#######...",
    "...%#####%...",
    "....#####....",
    "....%###%....",
    ".....###.....",
    ".....%#%.....",
    "......#......",
    "......%......",
    ".............",
    "............."
};
const char     *const HI_SCROLLER_ARROW_UP[] = {
    "             ",
    "             ",
    "      %      ",
    "      %      ",
    "     %%%     ",
    "     %%%     ",
    "    %%%%%    ",
    "    %%%%%    ",
    "   %%%%%%%   ",
    "   %%%%%%%   ",
    "  %%%%%%%%%  ",
    "             ",
    "             "
};
const char     *const HI_SCROLLER_ARROW_DOWN[] = {
    "             ",
    "             ",
    "  %%%%%%%%%  ",
    "   %%%%%%%   ",
    "   %%%%%%%   ",
    "    %%%%%    ",
    "    %%%%%    ",
    "     %%%     ",
    "     %%%     ",
    "      %      ",
    "      %      ",
    "             ",
    "             "
};

/* INTPROTO */
Pixmap
rxvt_renderPixmap(pR_ const char *const *data, int width, int height)
{
    char            a;
    int             x, y;
    Pixmap          d;
    GC              pointcolour;

    d = XCreatePixmap(R->Xdisplay, R->scrollBar.win, width, height, XDEPTH);

    for (y = 0; y < height; y++) {
	for (x = 0; x < width; x++) {
	    if ((a = data[y][x]) == ' ' || a == 'w')
		pointcolour = R->whiteGC;
	    else if (a == '.' || a == 'l')
		pointcolour = R->grayGC;
	    else if (a == '%' || a == 'd')
		pointcolour = R->darkGC;
	    else		/* if (a == '#' || a == 'b' || a) */
		pointcolour = R->blackGC;
	    XDrawPoint(R->Xdisplay, d, pointcolour, x, y);
	}
    }
    return d;
}

/* INTPROTO */
void
rxvt_init_scrollbar_stuff(pR)
{
    XGCValues       gcvalue;
    XColor          xcol;
    Pixmap          stipple;
    unsigned long   light, dark;

    gcvalue.graphics_exposures = False;

    gcvalue.foreground = R->PixColors[Color_Black];
    R->blackGC = XCreateGC(R->Xdisplay, R->scrollBar.win,
			      GCForeground | GCGraphicsExposures, &gcvalue);

    gcvalue.foreground = R->PixColors[Color_White];
    R->whiteGC = XCreateGC(R->Xdisplay, R->scrollBar.win,
			      GCForeground | GCGraphicsExposures, &gcvalue);

    xcol.red = 0xaeba;
    xcol.green = 0xaaaa;
    xcol.blue = 0xaeba;
    //if (!rxvt_rXAllocColor(aR_ &xcol, "light gray"))//TODO//D//
	xcol.pixel = R->PixColors[Color_AntiqueWhite];
    light = gcvalue.foreground = xcol.pixel;
    R->grayGC = XCreateGC(R->Xdisplay, R->scrollBar.win,
			     GCForeground | GCGraphicsExposures, &gcvalue);

    xcol.red = 0x51aa;
    xcol.green = 0x5555;
    xcol.blue = 0x5144;
    //if (!rxvt_rXAllocColor(aR_ &xcol, "dark gray"))//TODO//D//
	xcol.pixel = R->PixColors[Color_Grey25];
    dark = gcvalue.foreground = xcol.pixel;
    R->darkGC = XCreateGC(R->Xdisplay, R->scrollBar.win,
			     GCForeground | GCGraphicsExposures, &gcvalue);

    stipple = XCreateBitmapFromData(R->Xdisplay, R->scrollBar.win,
				    (char *)n_stp_bits, n_stp_width,
				    n_stp_height);

    gcvalue.foreground = dark;
    gcvalue.background = light;
    gcvalue.fill_style = FillOpaqueStippled;
    gcvalue.stipple = stipple;

/*    XSetWindowBackground(R->Xdisplay, R->scrollBar.win, R->PixColors[Color_Red]); */

    R->stippleGC = XCreateGC(R->Xdisplay, R->scrollBar.win,
				GCForeground | GCBackground | GCStipple
				| GCFillStyle | GCGraphicsExposures, &gcvalue);

    R->dimple = rxvt_renderPixmap(aR_ SCROLLER_DIMPLE, SCROLLER_DIMPLE_WIDTH,
				     SCROLLER_DIMPLE_HEIGHT);

    R->upArrow = rxvt_renderPixmap(aR_ SCROLLER_ARROW_UP, ARROW_WIDTH,
				      ARROW_HEIGHT);
    R->downArrow = rxvt_renderPixmap(aR_ SCROLLER_ARROW_DOWN, ARROW_WIDTH,
					ARROW_HEIGHT);
    R->upArrowHi = rxvt_renderPixmap(aR_ HI_SCROLLER_ARROW_UP, ARROW_WIDTH,
					ARROW_HEIGHT);
    R->downArrowHi = rxvt_renderPixmap(aR_ HI_SCROLLER_ARROW_DOWN,
					  ARROW_WIDTH, ARROW_HEIGHT);
}

/* Draw bevel & arrows */
/* INTPROTO */
void
rxvt_drawBevel(pR_ Drawable d, int x1, int y1, int w, int h)
{
    int             x2, y2;

    x2 = x1 + w - 1;		/* right  point */
    y2 = y1 + h - 1;		/* bottom point */
/* white top and left */
    XDrawLine(R->Xdisplay, d, R->whiteGC, x1, y1, x2, y1);
    XDrawLine(R->Xdisplay, d, R->whiteGC, x1, y1, x1, y2);
/* black bottom and right */
    XDrawLine(R->Xdisplay, d, R->blackGC, x1, y2, x2, y2);
    XDrawLine(R->Xdisplay, d, R->blackGC, x2, y1, x2, y2);
/* dark inside bottom and right */
    x1++, y1++, x2--, y2--;	/* move in one point */
    XDrawLine(R->Xdisplay, d, R->darkGC, x1, y2, x2, y2);
    XDrawLine(R->Xdisplay, d, R->darkGC, x2, y1, x2, y2);
}

/* EXTPROTO */
int
rxvt_scrollbar_show_next(pR_ int update, int last_top, int last_bot, int scrollbar_len)
{
    int             height = R->scrollBar.end + SB_BUTTON_TOTAL_HEIGHT + SB_PADDING;
    Drawable        s;

    if ((R->scrollBar.init & R_SB_NEXT) == 0) {
	R->scrollBar.init |= R_SB_NEXT;
	rxvt_init_scrollbar_stuff(aR);
    }

    if (R->TermWin.nscrolled == 0 || !update) {
	XFillRectangle(R->Xdisplay, R->scrollBar.win, R->grayGC, 0, 0,
		       SB_WIDTH_NEXT + 1, height);
	XDrawRectangle(R->Xdisplay, R->scrollBar.win, R->blackGC, 0,
		       -SB_BORDER_WIDTH, SB_WIDTH_NEXT,
		       height + SB_BORDER_WIDTH);
	XFillRectangle(R->Xdisplay, R->scrollBar.win, R->stippleGC,
		       SB_LEFT_PADDING, 0, SB_BUTTON_WIDTH, height);
    }
    if (R->TermWin.nscrolled) {
	if (last_top < R->scrollBar.top || !update)
	    XFillRectangle(R->Xdisplay, R->scrollBar.win, R->stippleGC,
			   SB_LEFT_PADDING, SB_PADDING + last_top,
			   SB_BUTTON_WIDTH, R->scrollBar.top - last_top);
	if (R->scrollBar.bot < last_bot || !update)
	    XFillRectangle(R->Xdisplay, R->scrollBar.win, R->stippleGC,
			   SB_LEFT_PADDING, R->scrollBar.bot + SB_PADDING,
			   SB_BUTTON_WIDTH, (last_bot - R->scrollBar.bot));
	XFillRectangle(R->Xdisplay, R->scrollBar.win, R->grayGC,
		       SB_LEFT_PADDING, R->scrollBar.top + SB_PADDING,
		       SB_BUTTON_WIDTH, scrollbar_len);
	XCopyArea(R->Xdisplay, R->dimple, R->scrollBar.win, R->whiteGC, 0, 0,
		  SCROLLER_DIMPLE_WIDTH, SCROLLER_DIMPLE_HEIGHT,
		  (SB_WIDTH_NEXT - SCROLLER_DIMPLE_WIDTH) / 2,
		  R->scrollBar.top + SB_BEVEL_WIDTH_UPPER_LEFT +
		  (scrollbar_len - SCROLLER_DIMPLE_HEIGHT) / 2);

	rxvt_drawBevel(aR_ R->scrollBar.win, SB_BUTTON_BEVEL_X,
		       R->scrollBar.top + SB_PADDING, SB_BUTTON_WIDTH,
		       scrollbar_len);
	rxvt_drawBevel(aR_ R->scrollBar.win, SB_BUTTON_BEVEL_X,
		       height - SB_BUTTON_BOTH_HEIGHT, SB_BUTTON_WIDTH,
		       SB_BUTTON_HEIGHT);
	rxvt_drawBevel(aR_ R->scrollBar.win, SB_BUTTON_BEVEL_X,
		       height - SB_BUTTON_SINGLE_HEIGHT, SB_BUTTON_WIDTH,
		       SB_BUTTON_HEIGHT);

	s = (scrollbar_isUp()) ? R->upArrowHi : R->upArrow;
	XCopyArea(R->Xdisplay, s, R->scrollBar.win, R->whiteGC, 0, 0,
		  ARROW_WIDTH, ARROW_HEIGHT, SB_BUTTON_FACE_X,
		  height - SB_BUTTON_BOTH_HEIGHT + SB_BEVEL_WIDTH_UPPER_LEFT);

	s = (scrollbar_isDn()) ? R->downArrowHi : R->downArrow;
	XCopyArea(R->Xdisplay, s, R->scrollBar.win, R->whiteGC, 0, 0,
		  ARROW_WIDTH, ARROW_HEIGHT, SB_BUTTON_FACE_X,
		  height - SB_BUTTON_SINGLE_HEIGHT + SB_BEVEL_WIDTH_UPPER_LEFT);
    }
    return 1;
}
#endif				/* NEXT_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/
