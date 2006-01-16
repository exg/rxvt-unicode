/*--------------------------------*-C-*---------------------------------*
 * File:	scrollbar-rxvt.C
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

/*----------------------------------------------------------------------*/
#if defined(RXVT_SCROLLBAR)

#define SHADOW_WIDTH 1

static void  
draw_shadow (rxvt_term *term, int x, int y, int w, int h)
{       
  int shadow;
    
  shadow = (w == 0 || h == 0) ? 1 : SHADOW_WIDTH;
  w += x - 1;
  h += y - 1;
  for (; shadow-- > 0; x++, y++, w--, h--)
    {     
      XDrawLine (term->display->display, term->scrollBar.win, term->topShadowGC, x, y, w, y);
      XDrawLine (term->display->display, term->scrollBar.win, term->topShadowGC, x, y, x, h);
      XDrawLine (term->display->display, term->scrollBar.win, term->botShadowGC, w, h, w, y + 1);
      XDrawLine (term->display->display, term->scrollBar.win, term->botShadowGC, w, h, x + 1, h);
    }           
}                 

/* draw triangular button with a shadow of 2 pixels */
static void
draw_button (rxvt_term *term, int x, int y, int state, int dirn)
{
  unsigned int sz, sz2;
  XPoint pt[3];
  GC top, bot;

  sz = term->scrollBar.width;
  sz2 = sz / 2;
  switch (state)
    {
      case +1:
        top = term->topShadowGC;
        bot = term->botShadowGC;
        break;
      case -1:
        top = term->botShadowGC;
        bot = term->topShadowGC;
        break;
      default:
        top = bot = term->scrollbarGC;
        break;
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

  XFillPolygon (term->display->display, term->scrollBar.win, term->scrollbarGC,
                pt, 3, Convex, CoordModeOrigin);

  /* draw base */
  XDrawLine (term->display->display, term->scrollBar.win, (dirn == UP ? bot : top),
             pt[0].x, pt[0].y, pt[1].x, pt[1].y);

  /* draw shadow on left */
  pt[1].x = x + sz2 - 1;
  pt[1].y = y + (dirn == UP ? 0 : sz - 1);
  XDrawLine (term->display->display, term->scrollBar.win, top,
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

  XDrawLine (term->display->display, term->scrollBar.win, top,
             pt[0].x, pt[0].y, pt[1].x, pt[1].y);
#endif

  /* draw shadow on right */
  pt[1].x = x + sz - 1;
  /*  pt[2].x = x + sz2; */
  pt[1].y = y + (dirn == UP ? sz - 1 : 0);
  pt[2].y = y + (dirn == UP ? 0 : sz - 1);
  XDrawLine (term->display->display, term->scrollBar.win, bot,
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

  XDrawLine (term->display->display, term->scrollBar.win, bot,
             pt[2].x, pt[2].y, pt[1].x, pt[1].y);
#endif
}

int
rxvt_term::scrollbar_show_rxvt (int update, int last_top, int last_bot, int scrollbar_len)
{
  int sbshadow = sb_shadow;
  int sbwidth = (int)scrollBar.width;

  if ((scrollBar.init & R_SB_RXVT) == 0)
    {
      scrollBar.init |= R_SB_RXVT;

      if (sbshadow)
        {
          XSetWindowBackground (display->display, scrollBar.win, pix_colors_focused[Color_trough]);
          XClearWindow (display->display, scrollBar.win);
        }
    }
  else
    {
      if (update)
        {
          if (last_top < scrollBar.top)
            XClearArea (display->display, scrollBar.win,
                        sbshadow, last_top,
                        sbwidth, (scrollBar.top - last_top),
                        False);

          if (scrollBar.bot < last_bot)
            XClearArea (display->display, scrollBar.win,
                        sbshadow, scrollBar.bot,
                        sbwidth, (last_bot - scrollBar.bot),
                        False);
        }
      else
        XClearWindow (display->display, scrollBar.win);
    }

  /* scrollbar slider */
#ifdef SB_BORDER
  {
    int xofs;

    if (OPTION (Opt_scrollBar_right))
      xofs = 0;
    else
      xofs = sbshadow ? sbwidth : sbwidth - 1;

    XDrawLine (display->display, scrollBar.win, botShadowGC,
               xofs, 0, xofs, scrollBar.end + sbwidth);
  }
#endif

  XFillRectangle (display->display, scrollBar.win, scrollbarGC,
                  sbshadow, scrollBar.top, sbwidth,
                  scrollbar_len);

  if (sbshadow)
    /* trough shadow */
    draw_shadow (this, 0, 0, sbwidth + 2 * sbshadow, scrollBar.end + (sbwidth + 1) + sbshadow);

  /* shadow for scrollbar slider */
  draw_shadow (this, sbshadow, scrollBar.top, sbwidth, scrollbar_len);

  /* Redraw scrollbar arrows */
  draw_button (this, sbshadow, sbshadow,          (scrollbar_isUp () ? -1 : +1), UP);
  draw_button (this, sbshadow, scrollBar.end + 1, (scrollbar_isDn () ? -1 : +1), DN);

  return 1;
}
#endif				/* RXVT_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/

