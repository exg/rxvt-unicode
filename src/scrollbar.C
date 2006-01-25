/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar.C
 *----------------------------------------------------------------------*
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 *				- N*XTstep like scrollbars
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

/*----------------------------------------------------------------------*/

/*
 * Map or unmap a scrollbar.  Returns non-zero upon change of state
 */
int
rxvt_term::scrollbar_mapping (int map)
{
  int             change = 0;

#ifdef HAVE_SCROLLBARS
  if (map)
    {
      scrollBar.setIdle ();

      if (!scrollBar.win)
        resize_scrollbar ();

      if (scrollBar.win)
        {
          XMapWindow (xdisp, scrollBar.win);
          change = 1;
        }
    }
  else
    {
      scrollBar.state = 0;
      XUnmapWindow (xdisp, scrollBar.win);
      change = 1;
    }
#endif
  return change;
}

void
rxvt_term::resize_scrollbar ()
{
#ifdef HAVE_SCROLLBARS
  int delayed_init = 0;

#define R_SCROLLBEG_XTERM	0
#define R_SCROLLEND_XTERM	szHint.height
#define R_SCROLLBEG_NEXT	0
#define R_SCROLLEND_NEXT	szHint.height - (SB_BUTTON_TOTAL_HEIGHT + \
                                                    SB_PADDING)
#define R_SCROLLBEG_RXVT	(scrollBar.width + 1) + sb_shadow
#define R_SCROLLEND_RXVT	szHint.height - R_SCROLLBEG_RXVT - \
                                    (2 * sb_shadow)

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
      scrollBar.win = XCreateSimpleWindow (xdisp,
                                           parent[0],
                                           window_sb_x, 0,
                                           scrollbar_TotalWidth (),
                                           szHint.height,
                                           0,
                                           pix_colors[Color_fg],
                                           pix_colors[Color_border]);
      XDefineCursor (xdisp, scrollBar.win, leftptr_cursor);

      XSelectInput (xdisp, scrollBar.win,
                   ExposureMask | ButtonPressMask | ButtonReleaseMask
                   | Button1MotionMask | Button2MotionMask
                   | Button3MotionMask);
      scrollbar_ev.start (display, scrollBar.win);

      delayed_init = 1;
    }

  scrollbar_show (1);

  if (delayed_init)
    XMapWindow (xdisp, scrollBar.win);
#endif
}

/*
 * Update current scrollbar view w.r.t. slider heights, etc.
 */
int
rxvt_term::scrollbar_show (int update)
{
  int             ret = 0;
#ifdef HAVE_SCROLLBARS
  int             top, bot, len, adj;

  if (!scrollBar.state)
    return 0;

  if (update)
    {
      top = view_start - top_row;
      bot = top + (nrow - 1);
      len = max (nrow - 1 - top_row, 1);
      adj = (((bot - top) * scrollbar_size ()) % len) > 0 ? 1 : 0;

      scrollBar.top = (scrollBar.beg + (top * scrollbar_size ()) / len);
      scrollbar_len = ((bot - top) * scrollbar_size ()) / len +
                      scrollbar_minheight () + adj;
      scrollBar.bot = (scrollBar.top + scrollbar_len);
      /* no change */
      if (scrollBar.top == last_top
          && scrollBar.bot == last_bot
          && (scrollBar.state == last_state || !scrollbar_isUpDn ()))
        return 0;
    }

  ret = (this->*scrollBar.update) (update, last_top, last_bot, scrollbar_len);

  last_top = scrollBar.top;
  last_bot = scrollBar.bot;
  last_state = scrollBar.state;
#endif

  return ret;
}

void
rxvt_term::setup_scrollbar (const char *scrollalign, const char *scrollstyle, const char *thickness)
{
#ifdef HAVE_SCROLLBARS
  int             i;
  short           style, width;

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
  if (! OPTION (Opt_scrollBar_floating) && style == R_SB_RXVT)
    sb_shadow = 2;
# endif

  scrollBar.style = style;
  scrollBar.width = width;

  /* scrollbar_align = R_SB_ALIGN_CENTRE; */
  if (scrollalign)
    {
      if (strncasecmp (scrollalign, "top", 3) == 0)
        scrollbar_align = R_SB_ALIGN_TOP;
      else if (strncasecmp (scrollalign, "bottom", 6) == 0)
        scrollbar_align = R_SB_ALIGN_BOTTOM;
    }
#endif
}

/*----------------------- end-of-file (C source) -----------------------*/

