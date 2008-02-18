/*----------------------------------------------------------------------*
 * File:	scrollbar.C
 *----------------------------------------------------------------------*
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 *				- N*XTstep like scrollbars
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004-2006 Marc Lehmann <pcg@goof.com>
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

/*
 * Map or unmap a scrollbar.  Returns non-zero upon change of state
 */
int
rxvt_term::scrollbar_mapping (int map)
{
  int change = 0;

  if (map)
    {
      scrollBar.state = STATE_IDLE;

      if (!scrollBar.win)
        resize_scrollbar ();

      if (scrollBar.win)
        {
          XMapWindow (dpy, scrollBar.win);
          change = 1;
        }
    }
  else
    {
      scrollBar.state = 0;
      XUnmapWindow (dpy, scrollBar.win);
      change = 1;
    }

  return change;
}

void
rxvt_term::resize_scrollbar ()
{
  int delayed_init = 0;
  int window_sb_x = 0;

  if (option (Opt_scrollBar_right))
    window_sb_x = szHint.width - scrollBar.total_width ();

#define R_SCROLLBEG_XTERM	0
#define R_SCROLLEND_XTERM	szHint.height
#define R_SCROLLBEG_NEXT	0
#define R_SCROLLEND_NEXT	szHint.height - (SB_BUTTON_TOTAL_HEIGHT + \
                                                    SB_PADDING)
#define R_SCROLLBEG_RXVT	(scrollBar.width + 1) + scrollBar.shadow
#define R_SCROLLEND_RXVT	szHint.height - R_SCROLLBEG_RXVT - \
                                    (2 * scrollBar.shadow)

#if defined(PLAIN_SCROLLBAR)
  if (scrollBar.style == R_SB_PLAIN)
    {
      scrollBar.beg = R_SCROLLBEG_XTERM;
      scrollBar.end = R_SCROLLEND_XTERM;
      scrollBar.update = &rxvt_term::scrollbar_show_plain;
    }
#endif
#if defined(XTERM_SCROLLBAR)
  if (scrollBar.style == R_SB_XTERM)
    {
      scrollBar.beg = R_SCROLLBEG_XTERM;
      scrollBar.end = R_SCROLLEND_XTERM;
      scrollBar.update = &rxvt_term::scrollbar_show_xterm;
    }
#endif
#if defined(NEXT_SCROLLBAR)
  if (scrollBar.style == R_SB_NEXT)
    {
      scrollBar.beg = R_SCROLLBEG_NEXT;
      scrollBar.end = R_SCROLLEND_NEXT;
      scrollBar.update = &rxvt_term::scrollbar_show_next;
    }
#endif
#if defined(RXVT_SCROLLBAR)
  if (scrollBar.style == R_SB_RXVT)
    {
      scrollBar.beg = R_SCROLLBEG_RXVT;
      scrollBar.end = R_SCROLLEND_RXVT;
      scrollBar.update = &rxvt_term::scrollbar_show_rxvt;
    }
#endif

  if (!scrollBar.win)
    {
      /* create the scrollbar window */
      scrollBar.win = XCreateSimpleWindow (dpy,
                                           parent[0],
                                           window_sb_x, 0,
                                           scrollBar.total_width (),
                                           szHint.height,
                                           0,
                                           pix_colors[Color_fg],
                                           pix_colors[Color_border]);
      XDefineCursor (dpy, scrollBar.win, scrollBar.leftptr_cursor);

      XSelectInput (dpy, scrollBar.win,
                   ExposureMask | ButtonPressMask | ButtonReleaseMask
                   | Button1MotionMask | Button2MotionMask
                   | Button3MotionMask);
      scrollbar_ev.start (display, scrollBar.win);

      delayed_init = 1;
    }
  else
    XMoveResizeWindow (dpy, scrollBar.win,
                       window_sb_x, 0,
                       scrollBar.total_width (), szHint.height);

  scrollbar_show (1);

  if (delayed_init)
    XMapWindow (dpy, scrollBar.win);
}

/*
 * Update current scrollbar view w.r.t. slider heights, etc.
 */
int
rxvt_term::scrollbar_show (int update)
{
  int             ret = 0;

  if (!scrollBar.state)
    return 0;

  if (update)
    {
      int top = view_start - top_row;
      int bot = top + (nrow - 1);
      int len = max (nrow - 1 - top_row, 1);
      int size = (bot - top) * scrollBar.size ();

      scrollBar.top = (scrollBar.beg + (top * scrollBar.size ()) / len);
      scrollBar.len = size / len + scrollBar.min_height () + (size % len > 0);
      scrollBar.bot = (scrollBar.top + scrollBar.len);
      /* no change */
      if (scrollBar.top == scrollBar.last_top
          && scrollBar.bot == scrollBar.last_bot
          && (scrollBar.state == scrollBar.last_state
              || !(scrollBar.state == STATE_UP || scrollBar.state == STATE_DOWN)))
        return 0;
    }

  ret = (this->*scrollBar.update) (update, scrollBar.last_top, scrollBar.last_bot, scrollBar.len);

  scrollBar.last_top = scrollBar.top;
  scrollBar.last_bot = scrollBar.bot;
  scrollBar.last_state = scrollBar.state;

  return ret;
}

void
scrollBar_t::setup (rxvt_term *term)
{
  int             i;
  short           style, width;
  const char *scrollalign, *scrollstyle, *thickness;

  this->term = term;
  scrollalign = term->rs[Rs_scrollBar_align];
  scrollstyle = term->rs[Rs_scrollstyle];
  thickness = term->rs[Rs_scrollBar_thickness];

# if defined(RXVT_SCROLLBAR)
  style = R_SB_RXVT;
# elif defined(XTERM_SCROLLBAR)
  style = R_SB_XTERM;
# elif defined(NEXT_SCROLLBAR)
  style = R_SB_NEXT;
# elif defined(PLAIN_SCROLLBAR)
  style = R_SB_PLAIN;
#else
  style = R_SB_RXVT;
# endif

# if (defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR) || defined(PLAIN_SCROLLBAR))
  if (scrollstyle)
    {
#  ifdef NEXT_SCROLLBAR
      if (strncasecmp (scrollstyle, "next", 4) == 0)
        style = R_SB_NEXT;
#  endif
#  ifdef XTERM_SCROLLBAR
      if (strncasecmp (scrollstyle, "xterm", 5) == 0)
        style = R_SB_XTERM;
#  endif
#  ifdef PLAIN_SCROLLBAR
      if (strncasecmp (scrollstyle, "plain", 5) == 0)
        style = R_SB_PLAIN;
#  endif

    }
# endif
  if (style == R_SB_NEXT)
    width = SB_WIDTH_NEXT;
  else if (style == R_SB_XTERM)
    width = SB_WIDTH_XTERM;
  else if (style == R_SB_PLAIN)
    width = SB_WIDTH_PLAIN;
  else /* if (style == R_SB_RXVT) */
    width = SB_WIDTH_RXVT;

  if (style != R_SB_NEXT)	/* dishonour request - for now */
    if (thickness && (i = atoi (thickness)) >= SB_WIDTH_MINIMUM)
      width = min (i, SB_WIDTH_MAXIMUM);

# ifdef RXVT_SCROLLBAR
  if (! term->option (Opt_scrollBar_floating) && style == R_SB_RXVT)
    shadow = SHADOW_WIDTH;
# endif

  this->style = style;
  this->width = width;

  /* align = R_SB_ALIGN_CENTRE; */
  if (scrollalign)
    {
      if (strncasecmp (scrollalign, "top", 3) == 0)
        align = R_SB_ALIGN_TOP;
      else if (strncasecmp (scrollalign, "bottom", 6) == 0)
        align = R_SB_ALIGN_BOTTOM;
    }
  last_bot = last_state = -1;
  /* cursor scrollBar: Black-on-White */
  leftptr_cursor = XCreateFontCursor (term->dpy, XC_left_ptr);
}

/*----------------------- end-of-file (C source) -----------------------*/

