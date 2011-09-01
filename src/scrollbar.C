/*----------------------------------------------------------------------*
 * File:	scrollbar.C
 *----------------------------------------------------------------------*
 *
 * Copyright (c) 1997,1998 mj olesen <olesen@me.QueensU.CA>
 * Copyright (c) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 *				- N*XTstep like scrollbars
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

/*
 * Map or unmap a scrollbar.  Returns non-zero upon change of state
 */
void
scrollBar_t::map (int map)
{
  if (map)
    {
      state = SB_STATE_IDLE;

      if (!win)
        resize ();
      else
        XMapWindow (term->dpy, win);
    }
  else
    {
      state = SB_STATE_OFF;

      if (win)
        XUnmapWindow (term->dpy, win);
    }
}

void
scrollBar_t::resize ()
{
  int delayed_init = 0;
  int window_sb_x = 0;

  if (term->option (Opt_scrollBar_right))
    window_sb_x = term->szHint.width - total_width ();

  update_data ();

  if (!win)
    {
      /* create the scrollbar window */
      win = XCreateSimpleWindow (term->dpy,
                                 term->parent,
                                 window_sb_x, 0,
                                 total_width (),
                                 term->szHint.height,
                                 0,
                                 term->pix_colors[Color_fg],
                                 term->pix_colors[Color_border]);
      XDefineCursor (term->dpy, win, leftptr_cursor);

      XSelectInput (term->dpy, win,
                   ExposureMask | ButtonPressMask | ButtonReleaseMask
                   | Button1MotionMask | Button2MotionMask
                   | Button3MotionMask);
      term->scrollbar_ev.start (term->display, win);

      delayed_init = 1;
    }
  else
    XMoveResizeWindow (term->dpy, win,
                       window_sb_x, 0,
                       total_width (), term->szHint.height);

  show (1);

  if (delayed_init)
    XMapWindow (term->dpy, win);
}

/*
 * Update current scrollbar view w.r.t. slider heights, etc.
 */
int
scrollBar_t::show (int refresh)
{
  int ret;

  if (!state)
    return 0;

  if (refresh)
    {
      int sb_top = term->view_start - term->top_row;
      int sb_bot = sb_top + (term->nrow - 1);
      int sb_len = max (term->nrow - 1 - term->top_row, 1);
      int n = min (min_height (), size ());

      top = beg + (sb_top * (size () - n)) / sb_len;
      bot = top + ecb_div_ru ((sb_bot - sb_top) * (size () - n), sb_len) + n;
      /* no change */
      if (top == last_top
          && bot == last_bot
          && (state == last_state
              || !(state == SB_STATE_UP || state == SB_STATE_DOWN)))
        return 0;
    }

  ret = (this->*update) (refresh);

  last_top = top;
  last_bot = bot;
  last_state = state;

  return ret;
}

void
scrollBar_t::setup (rxvt_term *term)
{
  int i;
  const char *scrollalign, *scrollstyle, *thickness;

  this->term = term;
  scrollalign = term->rs[Rs_scrollBar_align];
  scrollstyle = term->rs[Rs_scrollstyle];
  thickness = term->rs[Rs_scrollBar_thickness];

# if defined(RXVT_SCROLLBAR)
  style = SB_STYLE_RXVT;
# elif defined(XTERM_SCROLLBAR)
  style = SB_STYLE_XTERM;
# elif defined(NEXT_SCROLLBAR)
  style = SB_STYLE_NEXT;
# elif defined(PLAIN_SCROLLBAR)
  style = SB_STYLE_PLAIN;
#else
  style = SB_STYLE_RXVT;
# endif

# if (defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR) || defined(PLAIN_SCROLLBAR))
  if (scrollstyle)
    {
#  ifdef NEXT_SCROLLBAR
      if (strncasecmp (scrollstyle, "next", 4) == 0)
        style = SB_STYLE_NEXT;
#  endif
#  ifdef XTERM_SCROLLBAR
      if (strncasecmp (scrollstyle, "xterm", 5) == 0)
        style = SB_STYLE_XTERM;
#  endif
#  ifdef PLAIN_SCROLLBAR
      if (strncasecmp (scrollstyle, "plain", 5) == 0)
        style = SB_STYLE_PLAIN;
#  endif

    }
# endif
  if (style == SB_STYLE_NEXT)
    width = SB_WIDTH_NEXT;
  else if (style == SB_STYLE_XTERM)
    width = SB_WIDTH_XTERM;
  else if (style == SB_STYLE_PLAIN)
    width = SB_WIDTH_PLAIN;
  else /* if (style == SB_STYLE_RXVT) */
    width = SB_WIDTH_RXVT;

  if (style != SB_STYLE_NEXT)	/* dishonour request - for now */
    if (thickness && (i = atoi (thickness)) >= SB_WIDTH_MINIMUM)
      width = min (i, SB_WIDTH_MAXIMUM);

# ifdef RXVT_SCROLLBAR
  if (! term->option (Opt_scrollBar_floating) && style == SB_STYLE_RXVT)
    shadow = SHADOW_WIDTH;
# endif

  /* align = SB_ALIGN_CENTRE; */
  if (scrollalign)
    {
      if (strncasecmp (scrollalign, "top", 3) == 0)
        align = SB_ALIGN_TOP;
      else if (strncasecmp (scrollalign, "bottom", 6) == 0)
        align = SB_ALIGN_BOTTOM;
    }
  last_state = SB_STATE_OFF;
  /* cursor scrollBar: Black-on-White */
  leftptr_cursor = XCreateFontCursor (term->dpy, XC_left_ptr);
}

void
scrollBar_t::destroy ()
{
#ifdef XTERM_SCROLLBAR
  if (xscrollbarGC) XFreeGC (term->dpy, xscrollbarGC);
  if (ShadowGC)     XFreeGC (term->dpy, ShadowGC);
#endif
#ifdef PLAIN_SCROLLBAR
  if (pscrollbarGC) XFreeGC (term->dpy, pscrollbarGC);
#endif
#ifdef NEXT_SCROLLBAR
  if (blackGC)      XFreeGC (term->dpy, blackGC);
  if (whiteGC)      XFreeGC (term->dpy, whiteGC);
  if (grayGC)       XFreeGC (term->dpy, grayGC);
  if (darkGC)       XFreeGC (term->dpy, darkGC);
  if (stippleGC)    XFreeGC (term->dpy, stippleGC);
  if (dimple)       XFreePixmap (term->dpy, dimple);
  if (upArrow)      XFreePixmap (term->dpy, upArrow);
  if (downArrow)    XFreePixmap (term->dpy, downArrow);
  if (upArrowHi)    XFreePixmap (term->dpy, upArrowHi);
  if (downArrowHi)  XFreePixmap (term->dpy, downArrowHi);
#endif
#ifdef RXVT_SCROLLBAR
  if (topShadowGC)  XFreeGC (term->dpy, topShadowGC);
  if (botShadowGC)  XFreeGC (term->dpy, botShadowGC);
  if (scrollbarGC)  XFreeGC (term->dpy, scrollbarGC);
#endif
}

void
scrollBar_t::update_data ()
{
#if defined(PLAIN_SCROLLBAR)
  if (style == SB_STYLE_PLAIN)
    {
      beg = 0;
      end = term->szHint.height;
      update = &scrollBar_t::show_plain;
    }
#endif
#if defined(XTERM_SCROLLBAR)
  if (style == SB_STYLE_XTERM)
    {
      beg = 0;
      end = term->szHint.height;
      update = &scrollBar_t::show_xterm;
    }
#endif
#if defined(NEXT_SCROLLBAR)
  if (style == SB_STYLE_NEXT)
    {
      beg = 0;
      end = term->szHint.height - (SB_BUTTON_TOTAL_HEIGHT + SB_PADDING);
      update = &scrollBar_t::show_next;
    }
#endif
#if defined(RXVT_SCROLLBAR)
  if (style == SB_STYLE_RXVT)
    {
      beg = (width + 1) + shadow;
      end = term->szHint.height - beg - (2 * shadow);
      update = &scrollBar_t::show_rxvt;
    }
#endif
}

/*----------------------- end-of-file (C source) -----------------------*/

