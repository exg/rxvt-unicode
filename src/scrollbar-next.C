/*----------------------------------------------------------------------*
 * File:	scrollbar-next.C
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

const char     *const SCROLLER_DIMPLE[] =
  {
    ".%###.",
    "%#%%%%",
    "#%%...",
    "#%..  ",
    "#%.   ",
    ".%.  ."
  };
const char     *const SCROLLER_ARROW_UP[] =
  {
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
const char     *const SCROLLER_ARROW_DOWN[] =
  {
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
const char     *const HI_SCROLLER_ARROW_UP[] =
  {
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
const char     *const HI_SCROLLER_ARROW_DOWN[] =
  {
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

static Pixmap
renderPixmap (scrollBar_t *sb, const char *const *data, int width, int height)
{
  char            a;
  int             x, y;
  Pixmap          d;
  GC              pointcolour;

  d = XCreatePixmap (sb->term->dpy, sb->win, width, height, sb->term->depth);

  for (y = 0; y < height; y++)
    {
      for (x = 0; x < width; x++)
        {
          if ((a = data[y][x]) == ' ' || a == 'w')
            pointcolour = sb->whiteGC;
          else if (a == '.' || a == 'l')
            pointcolour = sb->grayGC;
          else if (a == '%' || a == 'd')
            pointcolour = sb->darkGC;
          else		/* if (a == '#' || a == 'b' || a) */
            pointcolour = sb->blackGC;

          XDrawPoint (sb->term->dpy, d, pointcolour, x, y);
        }
    }
  return d;
}

void
scrollBar_t::init_next ()
{
  XGCValues       gcvalue;
  XColor          xcol;
  Pixmap          stipple;
  unsigned long   light, dark;

  gcvalue.graphics_exposures = False;

  gcvalue.foreground = term->pix_colors_focused[Color_Black];
  blackGC = XCreateGC (term->dpy, win,
                       GCForeground | GCGraphicsExposures, &gcvalue);

  gcvalue.foreground = term->pix_colors_focused[Color_White];
  whiteGC = XCreateGC (term->dpy, win,
                       GCForeground | GCGraphicsExposures, &gcvalue);

  xcol.red = 0xaeba;
  xcol.green = 0xaaaa;
  xcol.blue = 0xaeba;
  xcol.pixel = term->pix_colors_focused[Color_scroll];
  light = gcvalue.foreground = xcol.pixel;
  grayGC = XCreateGC (term->dpy, win,
                      GCForeground | GCGraphicsExposures, &gcvalue);

  xcol.red = 0x51aa;
  xcol.green = 0x5555;
  xcol.blue = 0x5144;
  //if (!rXAllocColor (&xcol, "dark gray"))//TODO//D//
  xcol.pixel = term->pix_colors_focused[Color_Grey25];
  dark = gcvalue.foreground = xcol.pixel;
  darkGC = XCreateGC (term->dpy, win,
                     GCForeground | GCGraphicsExposures, &gcvalue);

  stipple = XCreateBitmapFromData (term->dpy, win,
                                   (char *)n_stp_bits, n_stp_width,
                                   n_stp_height);

  gcvalue.foreground = dark;
  gcvalue.background = light;
  gcvalue.fill_style = FillOpaqueStippled;
  gcvalue.stipple = stipple;

  /*    XSetWindowBackground (dpy, scrollBar.win, pix_colors_focused[Color_Red]); */

  stippleGC = XCreateGC (term->dpy, win,
                         GCForeground | GCBackground | GCStipple
                         | GCFillStyle | GCGraphicsExposures, &gcvalue);

  dimple = renderPixmap (this, SCROLLER_DIMPLE, SCROLLER_DIMPLE_WIDTH,
                         SCROLLER_DIMPLE_HEIGHT);

  upArrow = renderPixmap (this, SCROLLER_ARROW_UP, ARROW_WIDTH,
                          ARROW_HEIGHT);
  downArrow = renderPixmap (this, SCROLLER_ARROW_DOWN, ARROW_WIDTH,
                            ARROW_HEIGHT);
  upArrowHi = renderPixmap (this, HI_SCROLLER_ARROW_UP, ARROW_WIDTH,
                            ARROW_HEIGHT);
  downArrowHi = renderPixmap (this, HI_SCROLLER_ARROW_DOWN,
                              ARROW_WIDTH, ARROW_HEIGHT);
}

/* Draw bevel & arrows */
static void
drawBevel (scrollBar_t *sb, int x1, int y1, int w, int h)
{
  int x2, y2;
  Drawable d = sb->win;
  Display *dpy = sb->term->dpy;

  x2 = x1 + w - 1;		/* right  point */
  y2 = y1 + h - 1;		/* bottom point */
  /* white top and left */
  XDrawLine (dpy, d, sb->whiteGC, x1, y1, x2, y1);
  XDrawLine (dpy, d, sb->whiteGC, x1, y1, x1, y2);
  /* black bottom and right */
  XDrawLine (dpy, d, sb->blackGC, x1, y2, x2, y2);
  XDrawLine (dpy, d, sb->blackGC, x2, y1, x2, y2);
  /* dark inside bottom and right */
  x1++, y1++, x2--, y2--;	/* move in one point */
  XDrawLine (dpy, d, sb->darkGC, x1, y2, x2, y2);
  XDrawLine (dpy, d, sb->darkGC, x2, y1, x2, y2);
}

int
scrollBar_t::show_next (int update)
{
  int height = end + SB_BUTTON_TOTAL_HEIGHT + SB_PADDING;
  Drawable src;

  if ((init & SB_STYLE_NEXT) == 0)
    {
      init |= SB_STYLE_NEXT;
      init_next ();
    }

  if (term->top_row == 0 || !update)
    {
      XFillRectangle (term->dpy, win, grayGC, 0, 0,
                      SB_WIDTH_NEXT + 1, height);
      XDrawRectangle (term->dpy, win, blackGC, 0,
                      -SB_BORDER_WIDTH, SB_WIDTH_NEXT,
                      height + SB_BORDER_WIDTH);
      XFillRectangle (term->dpy, win, stippleGC,
                      SB_LEFT_PADDING, 0, SB_BUTTON_WIDTH, height);
    }

  if (term->top_row)
    {
      if (last_top < top || !update)
        XFillRectangle (term->dpy, win, stippleGC,
                        SB_LEFT_PADDING, SB_PADDING + last_top,
                        SB_BUTTON_WIDTH, top - last_top);

      if (bot < last_bot || !update)
        XFillRectangle (term->dpy, win, stippleGC,
                        SB_LEFT_PADDING, bot + SB_PADDING,
                        SB_BUTTON_WIDTH, (last_bot - bot));

      XFillRectangle (term->dpy, win, grayGC,
                      SB_LEFT_PADDING, top + SB_PADDING,
                      SB_BUTTON_WIDTH, bot - top);

      XCopyArea (term->dpy, dimple, win, whiteGC, 0, 0,
                 SCROLLER_DIMPLE_WIDTH, SCROLLER_DIMPLE_HEIGHT,
                 (SB_WIDTH_NEXT - SCROLLER_DIMPLE_WIDTH) / 2,
                 top + SB_BEVEL_WIDTH_UPPER_LEFT +
                 (bot - top - SCROLLER_DIMPLE_HEIGHT) / 2);

      drawBevel (this, SB_BUTTON_BEVEL_X,
                 top + SB_PADDING, SB_BUTTON_WIDTH,
                 bot - top);
      drawBevel (this, SB_BUTTON_BEVEL_X,
                 height - SB_BUTTON_BOTH_HEIGHT, SB_BUTTON_WIDTH,
                 SB_BUTTON_HEIGHT);
      drawBevel (this, SB_BUTTON_BEVEL_X,
                 height - SB_BUTTON_SINGLE_HEIGHT, SB_BUTTON_WIDTH,
                 SB_BUTTON_HEIGHT);

      src = state == SB_STATE_UP ? upArrowHi : upArrow;
      XCopyArea (term->dpy, src, win, whiteGC, 0, 0,
                 ARROW_WIDTH, ARROW_HEIGHT, SB_BUTTON_FACE_X,
                 height - SB_BUTTON_BOTH_HEIGHT + SB_BEVEL_WIDTH_UPPER_LEFT);

      src = state == SB_STATE_DOWN ? downArrowHi : downArrow;
      XCopyArea (term->dpy, src, win, whiteGC, 0, 0,
                 ARROW_WIDTH, ARROW_HEIGHT, SB_BUTTON_FACE_X,
                 height - SB_BUTTON_SINGLE_HEIGHT + SB_BEVEL_WIDTH_UPPER_LEFT);
    }

  return 1;
}
#endif /* NEXT_SCROLLBAR */
/*----------------------- end-of-file (C source) -----------------------*/
