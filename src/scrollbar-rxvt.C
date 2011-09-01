/*----------------------------------------------------------------------*
 * File:	scrollbar-rxvt.C
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
#if defined(RXVT_SCROLLBAR)

static void
draw_shadow (scrollBar_t *sb, int x, int y, int w, int h)
{
  int shadow;
  Drawable d = sb->win;
  Display *dpy = sb->term->dpy;

  shadow = (w == 0 || h == 0) ? 1 : SHADOW_WIDTH;
  w += x - 1;
  h += y - 1;

  for (; shadow-- > 0; x++, y++, w--, h--)
    {
      XDrawLine (dpy, d, sb->topShadowGC, x, y, w    , y    );
      XDrawLine (dpy, d, sb->topShadowGC, x, y, x    , h    );
      XDrawLine (dpy, d, sb->botShadowGC, w, h, w    , y + 1);
      XDrawLine (dpy, d, sb->botShadowGC, w, h, x + 1, h    );
    }
}

/* draw triangular button with a shadow of 2 pixels */
static void
draw_button (scrollBar_t *sb, int x, int y, int dirn)
{
  unsigned int sz, sz2;
  XPoint pt[3];
  GC top, bot;
  Drawable d = sb->win;
  Display *dpy = sb->term->dpy;

  sz = sb->width;
  sz2 = sz / 2;

  if ((dirn == UP && sb->state == SB_STATE_UP)
      || (dirn == DN && sb->state == SB_STATE_DOWN))
    {
      top = sb->botShadowGC;
      bot = sb->topShadowGC;
    }
  else
    {
      top = sb->topShadowGC;
      bot = sb->botShadowGC;
    }

  /* fill triangle */
  pt[0].x = x;
  pt[1].x = x + sz - 1;
  pt[2].x = x + sz2;

  if (dirn == UP)
    {
      pt[0].y = pt[1].y = y + sz - 1;
      pt[2].y = y;
    }
  else
    {
      pt[0].y = pt[1].y = y;
      pt[2].y = y + sz - 1;
    }

  XFillPolygon (dpy, d, sb->scrollbarGC,
                pt, 3, Convex, CoordModeOrigin);

  /* draw base */
  XDrawLine (dpy, d, (dirn == UP ? bot : top),
             pt[0].x, pt[0].y, pt[1].x, pt[1].y);

  /* draw shadow on left */
  pt[1].x = x + sz2 - 1;
  pt[1].y = y + (dirn == UP ? 0 : sz - 1);
  XDrawLine (dpy, d, top,
             pt[0].x, pt[0].y, pt[1].x, pt[1].y);

#if SHADOW_WIDTH > 1
  /* doubled */
  pt[0].x++;

  if (dirn == UP)
    {
      pt[0].y--;
      pt[1].y++;
    }
  else
    {
      pt[0].y++;
      pt[1].y--;
    }

  XDrawLine (dpy, d, top,
             pt[0].x, pt[0].y, pt[1].x, pt[1].y);
#endif

  /* draw shadow on right */
  pt[1].x = x + sz - 1;
  /*  pt[2].x = x + sz2; */
  pt[1].y = y + (dirn == UP ? sz - 1 : 0);
  pt[2].y = y + (dirn == UP ? 0 : sz - 1);
  XDrawLine (dpy, d, bot,
             pt[2].x, pt[2].y, pt[1].x, pt[1].y);

#if SHADOW_WIDTH > 1
  /* doubled */
  pt[1].x--;
  if (dirn == UP)
    {
      pt[2].y++;
      pt[1].y--;
    }
  else
    {
      pt[2].y--;
      pt[1].y++;
    }

  XDrawLine (dpy, d, bot,
             pt[2].x, pt[2].y, pt[1].x, pt[1].y);
#endif
}

int
scrollBar_t::show_rxvt (int update)
{
  int sbwidth = (int)width;

  if ((init & SB_STYLE_RXVT) == 0)
    {
      XGCValues gcvalue;

      init |= SB_STYLE_RXVT;

      gcvalue.foreground = term->pix_colors[Color_topShadow];
      topShadowGC = XCreateGC (term->dpy, term->vt, GCForeground, &gcvalue);
      gcvalue.foreground = term->pix_colors[Color_bottomShadow];
      botShadowGC = XCreateGC (term->dpy, term->vt, GCForeground, &gcvalue);
      gcvalue.foreground = term->pix_colors[ (term->depth <= 2 ? Color_fg : Color_scroll)];
      scrollbarGC = XCreateGC (term->dpy, term->vt, GCForeground, &gcvalue);
      if (shadow)
        {
          XSetWindowBackground (term->dpy, win, term->pix_colors_focused[Color_trough]);
          XClearWindow (term->dpy, win);
        }
    }
  else
    {
      if (update)
        {
          if (last_top < top)
            XClearArea (term->dpy, win,
                        shadow, last_top,
                        sbwidth, (top - last_top),
                        False);

          if (bot < last_bot)
            XClearArea (term->dpy, win,
                        shadow, bot,
                        sbwidth, (last_bot - bot),
                        False);
        }
      else
        XClearWindow (term->dpy, win);
    }

  /* scrollbar slider */
#ifdef SB_BORDER
  {
    int xofs;

    if (term->option (Opt_scrollBar_right))
      xofs = 0;
    else
      xofs = shadow ? sbwidth : sbwidth - 1;

    XDrawLine (term->dpy, win, botShadowGC,
               xofs, 0, xofs, end + sbwidth);
  }
#endif

  XFillRectangle (term->dpy, win, scrollbarGC,
                  shadow, top, sbwidth,
                  bot - top);

  if (shadow)
    /* trough shadow */
    draw_shadow (this, 0, 0, sbwidth + 2 * shadow, end + (sbwidth + 1) + shadow);

  /* shadow for scrollbar slider */
  draw_shadow (this, shadow, top, sbwidth, bot - top);

  /* Redraw scrollbar arrows */
  draw_button (this, shadow, shadow, UP);
  draw_button (this, shadow, end + 1,  DN);

  return 1;
}
#endif /* RXVT_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/

