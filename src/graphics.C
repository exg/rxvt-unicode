/*--------------------------------*-C-*---------------------------------*
 * File:	graphics.c
 *----------------------------------------------------------------------*
 * $Id: graphics.C,v 1.1 2003/11/24 17:28:08 pcg Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1994      Rob Nation <nation@rocket.sanders.lockheed.com>
 *				- original version
 * Copyright (c) 1997      Raul Garcia Garcia <rgg@tid.es>
 * Copyright (c) 1997,1998 mj olesen <olesen@me.queensu.ca>
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
#ifdef RXVT_GRAPHICS
#include "rxvt.h"		/* NECESSARY */
#include "rxvtgrx.h"

typedef struct grcmd_t {
    char            cmd;
    short           color;
    short           ncoords;
    int            *coords;
    unsigned char  *text;
    struct grcmd_t *next;
} grcmd_t;

typedef struct grwin_t {
    Window          win;
    int             x, y;
    unsigned int    w, h;
    short           screen;
    grcmd_t        *graphics;
    struct grwin_t *prev, *next;
} grwin_t;

#include "graphics.intpro"	/* PROTOS for internal routines */
#include <X11/cursorfont.h>

/* commands:
 * 'C' = Clear
 * 'F' = Fill
 * 'G' = Geometry
 * 'L' = Line
 * 'P' = Points
 * 'T' = Text
 * 'W' = Window
 */

#ifndef GRX_SCALE
# define GRX_SCALE	10000
#endif

/*----------------------------------------------------------------------*
 * local functions
 */
/* ARGSUSED */
/* INTPROTO */
void
rxvt_Gr_NewWindow(pR_ int nargs, int args[])
{
    int             x, y;
    unsigned int    w, h;
    Window          win;
    grwin_t        *grwin;
    Cursor          cursor;

    if (nargs != 4) {
	rxvt_print_error("NewWindow: 4 args needed, got %d\n", nargs);
	return;
    }
    x = args[0] * TermWin_TotalWidth() / GRX_SCALE;
    y = args[1] * TermWin_TotalHeight() / GRX_SCALE;
    w = args[2] * TermWin_TotalWidth() / GRX_SCALE;
    h = args[3] * TermWin_TotalHeight() / GRX_SCALE;

    win = XCreateSimpleWindow(R->Xdisplay, R->TermWin.vt,
			      x, y, w, h,
			      0,
			      R->PixColors[Color_fg],
			      R->PixColors[Color_bg]);

    cursor = XCreateFontCursor(R->Xdisplay, XC_crosshair);
    XDefineCursor(R->Xdisplay, win, cursor);
    XMapWindow(R->Xdisplay, win);
    XSelectInput(R->Xdisplay, win, ExposureMask);

    grwin = (grwin_t *) rxvt_malloc(sizeof(grwin_t));
    grwin->win = win;
    grwin->x = x;
    grwin->y = y;
    grwin->w = w;
    grwin->h = h;
    grwin->screen = 0;
    grwin->prev = NULL;
    grwin->next = R->h->gr_root;
    if (grwin->next)
	grwin->next->prev = grwin;
    R->h->gr_root = grwin;
    grwin->graphics = NULL;
    R->h->graphics_up++;

    rxvt_tt_printf(aR_ "\033W%ld\n", (long)grwin->win);
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_Gr_ClearWindow(pR_ grwin_t *grwin)
{
    grcmd_t        *cmd, *next;

    for (cmd = grwin->graphics; cmd != NULL; cmd = next) {
	next = cmd->next;
	free(cmd->coords);
	if (cmd->text != NULL)
	    free(cmd->text);
	free(cmd);
    }
    grwin->graphics = NULL;
    XClearWindow(R->Xdisplay, grwin->win);
}

/*
 * arg [0] = x
 * arg [1] = y
 * arg [2] = alignment
 * arg [3] = strlen (text)
 */
/* ARGSUSED */
/* INTPROTO */
void
rxvt_Gr_Text(pR_ grwin_t *grwin, grcmd_t *data)
{
    int             x, y, align;

    if (data->ncoords < 4 || data->text == NULL || *(data->text) == '\0')
	return;

    x = data->coords[0] * grwin->w / GRX_SCALE;
    y = data->coords[1] * grwin->h / GRX_SCALE;
    align = data->coords[2];

    if ((align & HORIZONTAL_ALIGNMENT) == RIGHT_TEXT)
	x -= XTextWidth(R->TermWin.font, data->text, data->coords[3]);
    else if ((align & HORIZONTAL_ALIGNMENT) == HCENTER_TEXT)
	x -= (XTextWidth(R->TermWin.font, data->text, data->coords[3]) >> 1);

    if ((align & VERTICAL_ALIGNMENT) == TOP_TEXT)
	y += R->TermWin.font->ascent;
    else if ((align & VERTICAL_ALIGNMENT) == BOTTOM_TEXT)
	y -= R->TermWin.font->descent;

    if ((align & VERTICAL_ALIGNMENT) == VCENTER_TEXT)
	y -= R->TermWin.font->descent
	     + ((R->TermWin.font->ascent + R->TermWin.font->descent) >> 1);
    if ((align & VERTICAL_ALIGNMENT) == VCAPS_CENTER_TEXT)
	y += (R->TermWin.font->ascent >> 1);

    XPMClearArea(R->Xdisplay, grwin->win, x, y - R->TermWin.font->ascent,
		 Width2Pixel(data->coords[3]), Height2Pixel(1), 0);
    XDrawString(R->Xdisplay, grwin->win, R->TermWin.gc, x, y,
		data->text, data->coords[3]);
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_Gr_Geometry(pR_ grwin_t *grwin, grcmd_t *data __attribute__((unused)))
{
    if (grwin)
	rxvt_tt_printf(aR_ "\033G%ld %d %d %u %u %d %d %ld %ld %d\n",
		  (long)grwin->win,
		  grwin->x, grwin->y, grwin->w, grwin->h,
		  R->TermWin.fwidth,
		  R->TermWin.fheight,
		  (long)GRX_SCALE * R->TermWin.fwidth / grwin->w,
		  (long)GRX_SCALE * R->TermWin.fheight / grwin->h,
		  XDEPTH);
    else			/* rxvt terminal window size */
	rxvt_tt_printf(aR_ "\033G0 0 0 %d %d %d %d %ld %ld %d\n",
		  TermWin_TotalWidth(), TermWin_TotalHeight(),
		  R->TermWin.fwidth, R->TermWin.fheight,
		  (long)GRX_SCALE * R->TermWin.fwidth / R->TermWin.width,
		  (long)GRX_SCALE * R->TermWin.fheight / R->TermWin.height,
		  XDEPTH);
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_Gr_DestroyWindow(pR_ grwin_t *grwin)
{
    grcmd_t        *cmd, *next;

    if (grwin == NULL)
	return;

    for (cmd = grwin->graphics; cmd; cmd = next) {
	next = cmd->next;
	free(cmd->coords);
	if (cmd->text != NULL)
	    free(cmd->text);
	free(cmd);
    }

    XDestroyWindow(R->Xdisplay, grwin->win);
    if (grwin->next != NULL)
	grwin->next->prev = grwin->prev;
    if (grwin->prev != NULL)
	grwin->prev->next = grwin->next;
    else
	R->h->gr_root = grwin->next;
    free(grwin);

    R->h->graphics_up--;
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_Gr_Dispatch(pR_ grwin_t *grwin, grcmd_t *data)
{
    int             i, n;
    union {
	XPoint          pt[NGRX_PTS / 2];
	XRectangle      rect[NGRX_PTS / 4];
    } xdata;

    if (data->color != Color_fg) {
	XGCValues       gcv;

	gcv.foreground = R->PixColors[data->color];
	XChangeGC(R->Xdisplay, R->TermWin.gc, GCForeground, &gcv);
    }
    if (grwin)
	switch (data->cmd) {
	case 'L':
	    if (data->ncoords > 3) {
		for (n = i = 0; i < data->ncoords; i += 2, n++) {
		    xdata.pt[n].x = data->coords[i] * grwin->w / GRX_SCALE;
		    xdata.pt[n].y = data->coords[i + 1] * grwin->h / GRX_SCALE;
		}
		XDrawLines(R->Xdisplay,
		       grwin->win, R->TermWin.gc, xdata.pt, n, CoordModeOrigin);
	    }
	    break;

	case 'P':
	    if (data->ncoords > 3) {
		for (n = i = 0; i < data->ncoords; i += 2, n++) {
		    xdata.pt[n].x = data->coords[i] * grwin->w / GRX_SCALE;
		    xdata.pt[n].y = data->coords[i + 1] * grwin->h / GRX_SCALE;
		}
		XDrawPoints(R->Xdisplay,
		       grwin->win, R->TermWin.gc, xdata.pt, n, CoordModeOrigin);
	    }
	    break;

	case 'F':
	    if (data->ncoords > 0) {
		for (n = i = 0; i < data->ncoords; i += 4, n++) {
		    xdata.rect[n].x = data->coords[i] * grwin->w / GRX_SCALE;
		    xdata.rect[n].y = data->coords[i + 1] * grwin->h
				      / GRX_SCALE;
		    xdata.rect[n].width = ((data->coords[i + 2]
					    - data->coords[i] + 1) *
					   grwin->w / GRX_SCALE);
		    xdata.rect[n].height = ((data->coords[i + 3]
					     - data->coords[i + 1] + 1) *
					    grwin->h / GRX_SCALE);
		    XPMClearArea(R->Xdisplay, grwin->win,
				 xdata.rect[n].x, xdata.rect[n].y,
				 xdata.rect[n].width, xdata.rect[n].height,
				 0);
		}
		XFillRectangles(R->Xdisplay, grwin->win, R->TermWin.gc, xdata.rect,
				n);
	    }
	    break;
	case 'T':
	    rxvt_Gr_Text(aR_ grwin, data);
	    break;
	case 'C':
	    rxvt_Gr_ClearWindow(aR_ grwin);
	    break;
	}
    if (data->color != Color_fg) {
	XGCValues       gcv;

	gcv.foreground = R->PixColors[Color_fg];
	XChangeGC(R->Xdisplay, R->TermWin.gc, GCForeground, &gcv);
    }
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_Gr_Redraw(pR_ grwin_t *grwin)
{
    grcmd_t        *cmd;

    for (cmd = grwin->graphics; cmd != NULL; cmd = cmd->next)
	rxvt_Gr_Dispatch(aR_ grwin, cmd);
}

/*----------------------------------------------------------------------*
 * end of static functions
 */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_Gr_ButtonReport(pR_ int but, int x, int y)
{
    grwin_t        *grwin;

    for (grwin = R->h->gr_root; grwin != NULL; grwin = grwin->next)
	if ((x > grwin->x)
	    && (y > grwin->y)
	    && ((unsigned int)x < grwin->x + grwin->w)
	    && ((unsigned int)y < grwin->y + grwin->h))
	    break;

    if (grwin == NULL)
	return;

    x = GRX_SCALE * (x - grwin->x) / grwin->w;
    y = GRX_SCALE * (y - grwin->y) / grwin->h;
    rxvt_tt_printf(aR_ "\033%c%ld;%d;%d;\n", but, (long)grwin->win, x, y);
}

/* ARGSUSED */
/* EXTPROTO */
void
rxvt_Gr_do_graphics(pR_ int cmd, unsigned int nargs, int args[], unsigned char *text)
{
    int             i;
    Window          win_id;
    grwin_t        *grwin;
    grcmd_t        *newcmd, *oldcmd;

    if (cmd == 'W') {
	rxvt_Gr_NewWindow(aR_ nargs, args);
	return;
    }
    win_id = (nargs > 0) ? (Window) args[0] : None;

    if ((cmd == 'G') && (win_id == None)) {
	rxvt_Gr_Geometry(aR_ NULL, NULL);
	return;
    }
    if ((win_id == None) && (R->h->gr_last_id != None))
	win_id = R->h->gr_last_id;

    if (win_id == None)
	return;

    grwin = R->h->gr_root;
    while ((grwin != NULL) && (grwin->win != win_id))
	grwin = grwin->next;

    if (grwin == NULL)
	return;

    if (cmd == 'G') {
	rxvt_Gr_Geometry(aR_ grwin, NULL);
	return;
    }
    nargs--;
    args++;			/* skip over window id */

/* record this new command */
    newcmd = (grcmd_t *) rxvt_malloc(sizeof(grcmd_t));
    newcmd->ncoords = nargs;
    newcmd->coords = (int *)rxvt_malloc((newcmd->ncoords * sizeof(int)));

    newcmd->next = NULL;
    newcmd->cmd = cmd;
    newcmd->color = rxvt_scr_get_fgcolor(aR);
    newcmd->text = text;

    for (i = 0; i < newcmd->ncoords; i++)
	newcmd->coords[i] = args[i];

/*
 * If newcmd == fill, and rectangle is full window, drop all prior
 * commands.
 */
    if ((newcmd->cmd == 'F') && (grwin) && (grwin->graphics)) {
	for (i = 0; i < newcmd->ncoords; i += 4) {
	    if ((newcmd->coords[i] == 0)
		&& (newcmd->coords[i + 1] == 0)
		&& (newcmd->coords[i + 2] == GRX_SCALE)
		&& (newcmd->coords[i + 3] == GRX_SCALE)) {
	    /* drop previous commands */
		oldcmd = grwin->graphics;
		while (oldcmd->next != NULL) {
		    grcmd_t        *tmp = oldcmd;

		    oldcmd = oldcmd->next;
		    free(tmp);
		}
		grwin->graphics = NULL;
	    }
	}
    }
/* insert new command into command list */
    oldcmd = grwin->graphics;
    if (oldcmd == NULL)
	grwin->graphics = newcmd;
    else {
	while (oldcmd->next != NULL)
	    oldcmd = oldcmd->next;
	oldcmd->next = newcmd;
    }
    rxvt_Gr_Dispatch(aR_ grwin, newcmd);
}

/* ARGSUSED */
/* EXTPROTO */
void
rxvt_Gr_scroll(pR_ int count)
{
    grwin_t        *grwin, *next;

    if ((count == 0) && (R->h->gr_prev_start == R->TermWin.view_start))
	return;

    R->h->gr_prev_start = R->TermWin.view_start;

    for (grwin = R->h->gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	grwin->y -= (count * R->TermWin.fheight);
	if ((long)(grwin->y + grwin->h)
	    < -(long)(R->TermWin.saveLines * R->TermWin.fheight))
	    rxvt_Gr_DestroyWindow(aR_ grwin);
	else
	    XMoveWindow(R->Xdisplay, grwin->win,
			grwin->x,
			grwin->y + (R->TermWin.view_start * R->TermWin.fheight));
    }
}

/* EXTPROTO */
void
rxvt_Gr_ClearScreen(pR)
{
    grwin_t        *grwin, *next;

    for (grwin = R->h->gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	if ((grwin->screen == 0) && (grwin->y + grwin->h > 0)) {
	    if (grwin->y >= 0)
		rxvt_Gr_DestroyWindow(aR_ grwin);
	    else
		XResizeWindow(R->Xdisplay, grwin->win,
			      grwin->w, -grwin->y);
	}
    }
}

/* EXTPROTO */
void
rxvt_Gr_ChangeScreen(pR)
{
    grwin_t        *grwin, *next;

    for (grwin = R->h->gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	if (grwin->y + grwin->h > 0) {
	    if (grwin->screen == 1) {
		XMapWindow(R->Xdisplay, grwin->win);
		grwin->screen = 0;
	    } else {
		XUnmapWindow(R->Xdisplay, grwin->win);
		grwin->screen = 1;
	    }
	}
    }
}

/* ARGSUSED */
/* EXTPROTO */
void
rxvt_Gr_expose(pR_ Window win)
{
    grwin_t        *grwin;

    for (grwin = R->h->gr_root; grwin != NULL; grwin = grwin->next) {
	if (grwin->win == win) {
	    rxvt_Gr_Redraw(aR_ grwin);
	    break;
	}
    }
}

/* ARGSUSED */
/* EXTPROTO */
void
rxvt_Gr_Resize(pR_ int w __attribute__((unused)), int h)
{
    grwin_t        *grwin;

    for (grwin = R->h->gr_root; grwin != NULL; grwin = grwin->next) {
	if (R->TermWin.height != h) {
	    grwin->y += (R->TermWin.height - h);
	    XMoveWindow(R->Xdisplay, grwin->win,
			grwin->x,
			grwin->y + (R->TermWin.view_start * R->TermWin.fheight));
	}
	rxvt_Gr_Redraw(aR_ grwin);
    }
}

/* EXTPROTO */
void
rxvt_Gr_reset(pR)
{
    grwin_t        *grwin, *next;

    for (grwin = R->h->gr_root; grwin != NULL; grwin = next) {
	next = grwin->next;
	rxvt_Gr_DestroyWindow(aR_ grwin);
    }

    R->h->graphics_up = 0;
}

/* EXTPROTO */
int
rxvt_Gr_Displayed(pR)
{
    return R->h->graphics_up;
}
#endif
/*----------------------- end-of-file (C source) -----------------------*/
