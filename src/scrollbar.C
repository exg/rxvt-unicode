/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar.c
 *----------------------------------------------------------------------*
 * $Id: scrollbar.C,v 1.5 2003/12/18 05:45:11 pcg Exp $
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
#include "scrollbar.intpro"	/* PROTOS for internal routines */

/*----------------------------------------------------------------------*/

/*
 * Map or unmap a scrollbar.  Returns non-zero upon change of state
 */
/* EXTPROTO */
int
rxvt_scrollbar_mapping(pR_ int map)
{
    int             change = 0;
#ifdef HAVE_SCROLLBARS

    if (map && !scrollbar_visible(R)) {
	R->scrollBar.setIdle ();
	if (!R->scrollBar.win)
	    rxvt_Resize_scrollBar(aR);
	if (R->scrollBar.win) {
	    XMapWindow(R->Xdisplay, R->scrollBar.win);
	    change = 1;
	}
    } else if (!map && scrollbar_visible(R)) {
	R->scrollBar.state = 0;
	XUnmapWindow(R->Xdisplay, R->scrollBar.win);
	change = 1;
    }
#endif
    return change;
}

/* EXTPROTO */
void
rxvt_Resize_scrollBar(pR)
{
#ifdef HAVE_SCROLLBARS
    int             delayed_init = 0;

#define R_SCROLLBEG_XTERM	0
#define R_SCROLLEND_XTERM	R->szHint.height
#define R_SCROLLBEG_NEXT	0
#define R_SCROLLEND_NEXT	R->szHint.height - (SB_BUTTON_TOTAL_HEIGHT + \
						    SB_PADDING)
#define R_SCROLLBEG_RXVT	(R->scrollBar.width + 1) + R->sb_shadow
#define R_SCROLLEND_RXVT	R->szHint.height - R_SCROLLBEG_RXVT - \
				    (2 * R->sb_shadow)

#if defined(XTERM_SCROLLBAR)
    if (R->scrollBar.style == R_SB_XTERM) {
	R->scrollBar.beg = R_SCROLLBEG_XTERM;
	R->scrollBar.end = R_SCROLLEND_XTERM;
	R->scrollBar.update = rxvt_scrollbar_show_xterm;
    }
#endif
#if defined(NEXT_SCROLLBAR)
    if (R->scrollBar.style == R_SB_NEXT) {
	R->scrollBar.beg = R_SCROLLBEG_NEXT;
	R->scrollBar.end = R_SCROLLEND_NEXT;
	R->scrollBar.update = rxvt_scrollbar_show_next;
    }
#endif
#if defined(RXVT_SCROLLBAR)
    if (R->scrollBar.style == R_SB_RXVT) {
	R->scrollBar.beg = R_SCROLLBEG_RXVT;
	R->scrollBar.end = R_SCROLLEND_RXVT;
	R->scrollBar.update = rxvt_scrollbar_show_rxvt;
    }
#endif

    if (!R->scrollBar.win) {
/* create the scrollbar window */
	R->scrollBar.win = XCreateSimpleWindow(R->Xdisplay,
					       R->TermWin.parent[0],
					       R->window_sb_x, 0,
					       scrollbar_TotalWidth(),
					       R->szHint.height,
					       0,
					       R->PixColors[Color_fg],
					       R->PixColors[Color_bg]);
#ifdef DEBUG_X
	XStoreName(R->Xdisplay, R->scrollBar.win, "scrollbar");
#endif
	XDefineCursor(R->Xdisplay, R->scrollBar.win, R->leftptr_cursor);
	XSelectInput(R->Xdisplay, R->scrollBar.win,
		     (ExposureMask | ButtonPressMask | ButtonReleaseMask
		      | Button1MotionMask | Button2MotionMask
		      | Button3MotionMask));
	delayed_init = 1;
    }
    rxvt_scrollbar_show(aR_ 1);
    if (delayed_init)
	XMapWindow(R->Xdisplay, R->scrollBar.win);
#endif
}

/*
 * Update current scrollbar view w.r.t. slider heights, etc.
 */
/* EXTPROTO */
int
rxvt_scrollbar_show(pR_ int update)
{
    int             ret = 0;
#ifdef HAVE_SCROLLBARS
    int             top, bot, len, adj;

    if (!scrollbar_visible(R))
	return 0;

    if (update) {
	top = (R->TermWin.nscrolled - R->TermWin.view_start);
	bot = top + (R->TermWin.nrow - 1);
	len = max((R->TermWin.nscrolled + (R->TermWin.nrow - 1)), 1);
	adj = (((bot - top) * scrollbar_size()) % len) > 0 ? 1 : 0;

	R->scrollBar.top = (R->scrollBar.beg + (top * scrollbar_size()) / len);
	R->scrollbar_len = ((bot - top) * scrollbar_size()) / len +
			      scrollbar_minheight() + adj;
	R->scrollBar.bot = (R->scrollBar.top + R->scrollbar_len);
	/* no change */
	if (R->scrollBar.top == R->last_top
	    && R->scrollBar.bot == R->last_bot
	    && (R->scrollBar.state == R->last_state || !scrollbar_isUpDn()))
	    return 0;
    }

    ret = R->scrollBar.update(aR_ update, R->last_top, R->last_bot,
			      R->scrollbar_len);

    R->last_top = R->scrollBar.top;
    R->last_bot = R->scrollBar.bot;
    R->last_state = R->scrollBar.state;

#endif
    return ret;
}

/* EXTPROTO */
void
rxvt_setup_scrollbar(pR_ const char *scrollalign, const char *scrollstyle, const char *thickness)
{
#ifdef HAVE_SCROLLBARS
    int             i;
    short           style, width;

# if defined(RXVT_SCROLLBAR) || !(defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR))
    style = R_SB_RXVT;
# else
#  ifdef NEXT_SCROLLBAR
    style = R_SB_NEXT;
#  elif defined(XTERM_SCROLLBAR)
    style = R_SB_XTERM;
#  endif
# endif

# if (defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR))
    if (scrollstyle) {
#  ifdef NEXT_SCROLLBAR
	if (STRNCASECMP(scrollstyle, "next", 4) == 0)
	    style = R_SB_NEXT;
#  endif
#  ifdef XTERM_SCROLLBAR
	if (STRNCASECMP(scrollstyle, "xterm", 5) == 0)
	    style = R_SB_XTERM;
#  endif
    }
# endif
    if (style == R_SB_NEXT)
	width = SB_WIDTH_NEXT;
    else if (style == R_SB_XTERM)
	width = SB_WIDTH_XTERM;
    else /* if (style == R_SB_RXVT) */
	width = SB_WIDTH_RXVT;

    if (style != R_SB_NEXT)	/* dishonour request - for now */
    if (thickness && (i = atoi(thickness)) >= SB_WIDTH_MINIMUM)
	width = min(i, SB_WIDTH_MAXIMUM);

# if defined(RXVT_SCROLLBAR)
    if (!(R->Options & Opt_scrollBar_floating) && style == R_SB_RXVT)
	R->sb_shadow = SHADOW;
# endif

    R->scrollBar.style = style;
    R->scrollBar.width = width;

    /* R->scrollbar_align = R_SB_ALIGN_CENTRE; */
    if (scrollalign) {
	if (STRNCASECMP(scrollalign, "top", 3) == 0)
	    R->scrollbar_align = R_SB_ALIGN_TOP;
	else if (STRNCASECMP(scrollalign, "bottom", 6) == 0)
	    R->scrollbar_align = R_SB_ALIGN_BOTTOM;
    }
#endif
}

/*----------------------- end-of-file (C source) -----------------------*/
