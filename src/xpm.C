/*--------------------------------*-C-*---------------------------------*
 * File:	xpm.c
 *----------------------------------------------------------------------*
 * $Id: xpm.C,v 1.4 2003/12/18 02:07:12 pcg Exp $
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997      Carsten Haitzler <raster@zip.com.au>
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
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
 *---------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "xpm.intpro"		/* PROTOS for internal routines */

#ifdef XPM_BACKGROUND

/*
 * These GEOM strings indicate absolute size/position:
 * @ `WxH+X+Y'
 * @ `WxH+X'    -> Y = X
 * @ `WxH'      -> Y = X = 50
 * @ `W+X+Y'    -> H = W
 * @ `W+X'      -> H = W, Y = X
 * @ `W'        -> H = W, X = Y = 50
 * @ `0xH'      -> H *= H/100, X = Y = 50 (W unchanged)
 * @ `Wx0'      -> W *= W/100, X = Y = 50 (H unchanged)
 * @ `=+X+Y'    -> (H, W unchanged)
 * @ `=+X'      -> Y = X (H, W unchanged)
 *
 * These GEOM strings adjust position relative to current position:
 * @ `+X+Y'
 * @ `+X'       -> Y = X
 *
 * And this GEOM string is for querying current scale/position:
 * @ `?'
 */
/* EXTPROTO */
int
rxvt_scale_pixmap(pR_ const char *geom)
{
    int             flags, changed = 0;
    int             x = 0, y = 0;
    unsigned int    w = 0, h = 0;
    unsigned int    n;
    char           *p, *str;
    bgPixmap_t     *bgpixmap = &(R->bgPixmap);

#define MAXLEN_GEOM		sizeof("[1000x1000+1000+1000]")

    if (geom == NULL)
	return 0;
    str = (char *)rxvt_malloc (MAXLEN_GEOM + 1);
    if (!STRCMP(geom, "?")) {
	sprintf(str, "[%dx%d+%d+%d]",	/* can't presume snprintf() ! */
		min(bgpixmap->w, 9999), min(bgpixmap->h, 9999),
		min(bgpixmap->x, 9999), min(bgpixmap->y, 9999));
	rxvt_xterm_seq(aR_ XTerm_title, str, CHAR_ST);
	free(str);
	return 0;
    }

    if ((p = STRCHR(geom, ';')) == NULL)
	p = STRCHR(geom, '\0');
    n = (p - geom);
    if (n <= MAXLEN_GEOM) {
	STRNCPY(str, geom, n);
	str[n] = '\0';

	flags = XParseGeometry(str, &x, &y, &w, &h);
	if (!flags) {
	    flags |= WidthValue;
	    w = 0;
	}			/* default is tile */
	if (flags & WidthValue) {
	    if (!(flags & XValue))
		x = 50;
	    if (!(flags & HeightValue))
		h = w;
	    if (w && !h) {
		w = (bgpixmap->w * w) / 100;
		h = bgpixmap->h;
	    } else if (h && !w) {
		w = bgpixmap->w;
		h = (bgpixmap->h * h) / 100;
	    }
	    if (w > 1000)
		w = 1000;
	    if (h > 1000)
		h = 1000;
	    if (bgpixmap->w != (short)w) {
		bgpixmap->w = (short)w;
		changed++;
	    }
	    if (bgpixmap->h != (short)h) {
		bgpixmap->h = (short)h;
		changed++;
	    }
	}
	if (!(flags & YValue)) {
	    if (flags & XNegative)
		flags |= YNegative;
	    y = x;
	}

	if (!(flags & WidthValue) && geom[0] != '=') {
	    x += bgpixmap->x;
	    y += bgpixmap->y;
	} else {
	    if (flags & XNegative)
		x += 100;
	    if (flags & YNegative)
		y += 100;
	}
	MIN_IT(x, 100);
	MIN_IT(y, 100);
	MAX_IT(x, 0);
	MAX_IT(y, 0);
	if (bgpixmap->x != x) {
	    bgpixmap->x = x;
	    changed++;
	}
	if (bgpixmap->y != y) {
	    bgpixmap->y = y;
	    changed++;
	}
    }
    free(str);
    return changed;
}

/* EXTPROTO */
void
rxvt_resize_pixmap(pR)
{
    XGCValues       gcvalue;
    GC              gc;
    unsigned int    width = TermWin_TotalWidth();
    unsigned int    height = TermWin_TotalHeight();

    if (R->TermWin.pixmap != None)
	XFreePixmap(R->Xdisplay, R->TermWin.pixmap);

    if (R->bgPixmap.pixmap == None) { /* So be it: I'm not using pixmaps */
	R->TermWin.pixmap = None;
	if (!(R->Options & Opt_transparent) || R->am_transparent == 0)
	    XSetWindowBackground(R->Xdisplay, R->TermWin.vt,
				 R->PixColors[Color_bg]);
	return;
    }

    gcvalue.foreground = R->PixColors[Color_bg];
    gc = XCreateGC(R->Xdisplay, R->TermWin.vt, GCForeground, &gcvalue);

    if (R->bgPixmap.pixmap != None) {	/* we have a specified pixmap */
	unsigned int    w = R->bgPixmap.w, h = R->bgPixmap.h,
			x = R->bgPixmap.x, y = R->bgPixmap.y;
        unsigned int    xpmh = R->xpmAttr.height,
			xpmw = R->xpmAttr.width;

	/*
	 * don't zoom pixmap too much nor expand really small pixmaps
	 */
	if (w > 1000 || h > 1000)
	    w = 1;
	else if (width > (10 * xpmw)
		 || height > (10 * xpmh))
	    w = 0;		/* tile */

	if (w == 0) {
	    /* basic X tiling - let the X server do it */
	    R->TermWin.pixmap = XCreatePixmap(R->Xdisplay, R->TermWin.vt,
					      xpmw, xpmh,
					      (unsigned int)XDEPTH);
	    XCopyArea(R->Xdisplay, R->bgPixmap.pixmap, R->TermWin.pixmap, gc,
		      0, 0, xpmw, xpmh, 0, 0);
	} else {
	    float           incr, p;
	    Pixmap          tmp;

	    R->TermWin.pixmap = XCreatePixmap(R->Xdisplay, R->TermWin.vt,
					      width, height,
					      (unsigned int)XDEPTH);
	    /*
	     * horizontal scaling
	     */
	    rxvt_pixmap_incr(&w, &x, &incr, &p, width, xpmw);

	    tmp = XCreatePixmap(R->Xdisplay, R->TermWin.vt,
				width, xpmh, (unsigned int)XDEPTH);
	    XFillRectangle(R->Xdisplay, tmp, gc, 0, 0, width,
			   xpmh);

	    for ( /*nil */ ; x < w; x++, p += incr) {
		if (p >= xpmw)
		    p = 0;
		/* copy one column from the original pixmap to the tmp pixmap */
		XCopyArea(R->Xdisplay, R->bgPixmap.pixmap, tmp, gc,
			  (int)p, 0, 1, xpmh, (int)x, 0);
	    }

	    /*
	     * vertical scaling
	     */
	    rxvt_pixmap_incr(&h, &y, &incr, &p, height, xpmh);

	    if (y > 0)
		XFillRectangle(R->Xdisplay, R->TermWin.pixmap, gc, 0, 0, width,
			       y);
	    if (h < height)
		XFillRectangle(R->Xdisplay, R->TermWin.pixmap, gc, 0, (int)h,
			       width, height - h + 1);
	    for ( /*nil */ ; y < h; y++, p += incr) {
		if (p >= xpmh)
		    p = 0;
		/* copy one row from the tmp pixmap to the main pixmap */
		XCopyArea(R->Xdisplay, tmp, R->TermWin.pixmap, gc,
			  0, (int)p, width, 1, 0, (int)y);
	    }
	    XFreePixmap(R->Xdisplay, tmp);
	}
    }
    XSetWindowBackgroundPixmap(R->Xdisplay, R->TermWin.vt, R->TermWin.pixmap);
    XFreeGC(R->Xdisplay, gc);
    R->am_transparent = 0;

    XClearWindow(R->Xdisplay, R->TermWin.vt);

    XSync(R->Xdisplay, False);
}

/*
 * Calculate tiling sizes and increments
 * At start, p == 0, incr == xpmwidthheight
 */
/* INTPROTO */
void
rxvt_pixmap_incr(unsigned int *wh, unsigned int *xy, float *incr, float *p, unsigned int widthheight, unsigned int xpmwidthheight)
{
    unsigned int    cwh, cxy;
    float           cincr, cp;

    cp = 0;
    cincr = (float)xpmwidthheight;
    cxy = *xy;
    cwh = *wh;
    if (cwh == 1) {	/* display one image, no horizontal/vertical scaling */
	cincr = (float)widthheight;
	if (xpmwidthheight <= widthheight) {
	    cwh = xpmwidthheight;
	    cxy = (cxy * (widthheight - cwh)) / 100;	/* beware! order */
	    cwh += cxy;
	} else {
	    cxy = 0;
	    cwh = widthheight;
	}
    } else if (cwh < 10) {	/* fit WH images across/down screen */
	cincr *= cwh;
	cxy = 0;
	cwh = widthheight;
    } else {
	cincr *= 100.0 / cwh;
	if (cwh < 100) {	/* contract */
	    float           pos;

	    cwh = (cwh * widthheight) / 100;
	    pos = (float)cxy / 100 * widthheight - (cwh / 2);

	    cxy = (widthheight - cwh);
	    if (pos <= 0)
		cxy = 0;
	    else if (pos < cxy)
		cxy = (int) pos;
	    cwh += cxy;
	} else {	/* expand */
	    if (cxy > 0) {	/* position */
		float           pos;

		pos = (float)cxy / 100 * xpmwidthheight - (cincr / 2);
		cp = xpmwidthheight - cincr;
		if (pos <= 0)
		    cp = 0;
		else if (pos < cp)
		    cp = pos;
	    }
	    cxy = 0;
	    cwh = widthheight;
	}
    }
    cincr /= widthheight;
    *wh = cwh;
    *xy = cxy;
    *incr = cincr;
    *p = cp;
}

/* EXTPROTO */
Pixmap
rxvt_set_bgPixmap(pR_ const char *file)
{
    char           *f;

    assert(file != NULL);

    if (R->bgPixmap.pixmap != None) {
	XFreePixmap(R->Xdisplay, R->bgPixmap.pixmap);
	R->bgPixmap.pixmap = None;
    }
    XSetWindowBackground(R->Xdisplay, R->TermWin.vt, R->PixColors[Color_bg]);

    if (*file != '\0') {
/*      XWindowAttributes attr; */

	/*
	 * we already have the required attributes
	 */
/*      XGetWindowAttributes(R->Xdisplay, R->TermWin.vt, &attr); */

	R->xpmAttr.closeness = 30000;
	R->xpmAttr.colormap = XCMAP;
	R->xpmAttr.visual = XVISUAL;
	R->xpmAttr.depth = XDEPTH;
	R->xpmAttr.valuemask = (XpmCloseness | XpmColormap | XpmVisual |
				   XpmDepth | XpmSize | XpmReturnPixels);

	/* search environment variables here too */
	f = (char *)rxvt_File_find(file, ".xpm", R->rs[Rs_path]);
	if (f == NULL
	    || XpmReadFileToPixmap(R->Xdisplay, Xroot, f,
				   &R->bgPixmap.pixmap, NULL,
				   &R->xpmAttr)) {
	    char           *p;

	    /* semi-colon delimited */
	    if ((p = STRCHR(file, ';')) == NULL)
		p = STRCHR(file, '\0');

	    rxvt_print_error("couldn't load XPM file \"%.*s\"", (p - file),
			     file);
	}
	free(f);
    }
    rxvt_resize_pixmap(aR);
    return R->bgPixmap.pixmap;
}

#endif				/* XPM_BACKGROUND */
