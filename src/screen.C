/*--------------------------------*-C-*--------------------------------------*
 * File:	screen.c
 *---------------------------------------------------------------------------*
 * $Id: screen.C,v 1.3 2003/11/25 11:52:42 pcg Exp $
 *
 * Copyright (c) 1997-2001 Geoff Wing <gcw@pobox.com>
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
 *--------------------------------------------------------------------------*/
/*
 * We handle _all_ screen updates and selections
 */

#include "../config.h"		/* NECESSARY */
#define INTERN_SCREEN
#include "rxvt.h"		/* NECESSARY */
#include "screen.intpro"	/* PROTOS for internal routines */

#include <X11/Xmd.h>		/* get the typedef for CARD32 */

#include <stdint.h>
#include <wchar.h>

inline void fill_text (text_t *start, text_t value, int len)
{
  while (len--)
    *start++ = value;
}

#define RESET_CHSTAT(H)				\
    if ((H)->chstat == WBYTE)			\
	(H)->chstat = SBYTE, (H)->lost_multi = 1

/* ------------------------------------------------------------------------- */
#define PROP_SIZE		16384
#define TABSIZE			8	/* default tab size */

/* ------------------------------------------------------------------------- *
 *             GENERAL SCREEN AND SELECTION UPDATE ROUTINES                  *
 * ------------------------------------------------------------------------- */
#define ZERO_SCROLLBACK(R)						\
    if (((R)->Options & Opt_scrollTtyOutput) == Opt_scrollTtyOutput)	\
	(R)->TermWin.view_start = 0
#define CLEAR_SELECTION(R)						\
    (R)->selection.beg.row = (R)->selection.beg.col			\
	= (R)->selection.end.row = (R)->selection.end.col = 0
#define CLEAR_ALL_SELECTION(R)						\
    (R)->selection.beg.row = (R)->selection.beg.col			\
	= (R)->selection.mark.row = (R)->selection.mark.col		\
	= (R)->selection.end.row = (R)->selection.end.col = 0

#define ROW_AND_COL_IS_AFTER(A, B, C, D)				\
    (((A) > (C)) || (((A) == (C)) && ((B) > (D))))
#define ROW_AND_COL_IS_BEFORE(A, B, C, D)				\
    (((A) < (C)) || (((A) == (C)) && ((B) < (D))))
#define ROW_AND_COL_IN_ROW_AFTER(A, B, C, D)				\
    (((A) == (C)) && ((B) > (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_AFTER(A, B, C, D)			\
    (((A) == (C)) && ((B) >= (D)))
#define ROW_AND_COL_IN_ROW_BEFORE(A, B, C, D)				\
    (((A) == (C)) && ((B) < (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_BEFORE(A, B, C, D)			\
    (((A) == (C)) && ((B) <= (D)))

/* these must be row_col_t */
#define ROWCOL_IS_AFTER(X, Y)						\
    ROW_AND_COL_IS_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IS_BEFORE(X, Y)						\
    ROW_AND_COL_IS_BEFORE((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AFTER(X, Y)					\
    ROW_AND_COL_IN_ROW_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_BEFORE(X, Y)					\
    ROW_AND_COL_IN_ROW_BEFORE((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AT_OR_AFTER(X, Y)					\
    ROW_AND_COL_IN_ROW_AT_OR_AFTER((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AT_OR_BEFORE(X, Y)				\
    ROW_AND_COL_IN_ROW_AT_OR_BEFORE((X).row, (X).col, (Y).row, (Y).col)

/*
 * CLEAR_ROWS : clear <num> rows starting from row <row>
 * CLEAR_CHARS: clear <num> chars starting from pixel position <x,y>
 * ERASE_ROWS : set <num> rows starting from row <row> to the foreground colour
 */
#define drawBuffer	(R->TermWin.vt)

#define CLEAR_ROWS(row, num)						\
    if (R->TermWin.mapped)						\
	XClearArea(R->Xdisplay, drawBuffer, R->TermWin.int_bwidth,	\
		   Row2Pixel(row), (unsigned int)R->TermWin.width,	\
		   (unsigned int)Height2Pixel(num), False)

#define CLEAR_CHARS(x, y, num)						\
    if (R->TermWin.mapped)						\
	XClearArea(R->Xdisplay, drawBuffer, x, y,			\
		   (unsigned int)Width2Pixel(num),			\
		   (unsigned int)Height2Pixel(1), False)

#define ERASE_ROWS(row, num)						\
    XFillRectangle(R->Xdisplay, drawBuffer, R->TermWin.gc,		\
		   R->TermWin.int_bwidth, Row2Pixel(row),		\
		   (unsigned int)R->TermWin.width,			\
		   (unsigned int)Height2Pixel(num))

/* ------------------------------------------------------------------------- *
 *                        SCREEN `COMMON' ROUTINES                           *
 * ------------------------------------------------------------------------- */
/* Fill part/all of a line with blanks. */
/* INTPROTO */
void
rxvt_blank_line(pR_ text_t *et, rend_t *er, unsigned int width, rend_t efs)
{
    efs &= ~RS_baseattrMask;
    efs = SET_FONT (efs, R->TermWin.fontset->find_font (' '));

    while (width--)
      {
        *et++ = ' ';
	*er++ = efs;
      }
}

/* ------------------------------------------------------------------------- */
/* Fill a full line with blanks - make sure it is allocated first */
/* INTPROTO */
void
rxvt_blank_screen_mem(pR_ text_t **tp, rend_t **rp, unsigned int row, rend_t efs)
{
    int             width = R->TermWin.ncol;
    rend_t         *er;

#ifdef DEBUG_STRICT
    assert((tp[row] && rp[row]) || (tp[row] == NULL && rp[row] == NULL));
#endif
    if (tp[row] == NULL) {
	tp[row] = (text_t *)rxvt_malloc(sizeof(text_t) * width);
	rp[row] = (rend_t *)rxvt_malloc(sizeof(rend_t) * width);
    }
    rxvt_blank_line (aR_ tp[row], rp[row], width, efs);
}

/* ------------------------------------------------------------------------- *
 *                          SCREEN INITIALISATION                            *
 * ------------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_scr_reset(pR)
{
    unsigned int    ncol, nrow, prev_ncol, prev_nrow,
		    total_rows, prev_total_rows;
    unsigned int    p, q;
    int             k;
    rend_t          setrstyle;

    D_SCREEN((stderr, "rxvt_scr_reset()"));

    R->TermWin.view_start = 0;
    RESET_CHSTAT(R);
    R->num_scr = 0;

    prev_ncol = R->prev_ncol;
    prev_nrow = R->prev_nrow;
    if (R->TermWin.ncol == 0)
	R->TermWin.ncol = 80;
    if (R->TermWin.nrow == 0)
	R->TermWin.nrow = 24;
    ncol = R->TermWin.ncol;
    nrow = R->TermWin.nrow;
    if (ncol == prev_ncol && nrow == prev_nrow)
	return;

    R->want_refresh = 1;

    total_rows = nrow + R->TermWin.saveLines;
    prev_total_rows = prev_nrow + R->TermWin.saveLines;

    R->screen.tscroll = 0;
    R->screen.bscroll = nrow - 1;

    if (prev_nrow == 0) {
/*
 * A: first time called so just malloc everything : don't rely on realloc
 *    Note: this is still needed so that all the scrollback lines are NULL
 */
	R->screen.text = (text_t **)rxvt_calloc(total_rows, sizeof(text_t *));
	R->buf_text = (text_t **)rxvt_calloc(total_rows, sizeof(text_t *));
	R->drawn_text = (text_t **)rxvt_calloc(nrow, sizeof(text_t *));
	R->swap.text = (text_t **)rxvt_calloc(nrow, sizeof(text_t *));

	R->screen.tlen = (int16_t *)rxvt_calloc(total_rows, sizeof(int16_t));
	R->swap.tlen = (int16_t *)rxvt_calloc(nrow, sizeof(int16_t));

	R->screen.rend = (rend_t **)rxvt_calloc(total_rows, sizeof(rend_t *));
	R->buf_rend = (rend_t **)rxvt_calloc(total_rows, sizeof(rend_t *));
	R->drawn_rend = (rend_t **)rxvt_calloc(nrow, sizeof(rend_t *));
	R->swap.rend = (rend_t **)rxvt_calloc(nrow, sizeof(rend_t *));

	for (p = 0; p < nrow; p++) {
	    q = p + R->TermWin.saveLines;
	    rxvt_blank_screen_mem(aR_ R->screen.text, R->screen.rend,
				  q, DEFAULT_RSTYLE);
	    rxvt_blank_screen_mem(aR_ R->swap.text, R->swap.rend,
				  p, DEFAULT_RSTYLE);
	    R->screen.tlen[q] = R->swap.tlen[p] = 0;
	    rxvt_blank_screen_mem(aR_ R->drawn_text, R->drawn_rend,
				  p, DEFAULT_RSTYLE);
	}
	MEMSET(R->charsets, 'B', sizeof(R->charsets));
	R->TermWin.nscrolled = 0;	/* no saved lines */
	R->rstyle = DEFAULT_RSTYLE;
	R->screen.flags = Screen_DefaultFlags;
	R->screen.cur.row = R->screen.cur.col = 0;
	R->screen.charset = 0;
	R->current_screen = PRIMARY;
	rxvt_scr_cursor(aR_ SAVE);
#if NSCREENS
	R->swap.flags = Screen_DefaultFlags;
	R->swap.cur.row = R->swap.cur.col = 0;
	R->swap.charset = 0;
	R->current_screen = SECONDARY;
	rxvt_scr_cursor(aR_ SAVE);
	R->current_screen = PRIMARY;
#endif
	R->selection.text = NULL;
	R->selection.len = 0;
	R->selection.op = SELECTION_CLEAR;
	R->selection.screen = PRIMARY;
	R->selection.clicks = 0;
	CLEAR_ALL_SELECTION(R);
	R->rvideo = 0;

    } else {
/*
 * B1: add or delete rows as appropriate
 */
	setrstyle = DEFAULT_RSTYLE;

	if (nrow < prev_nrow) {
	    /* delete rows */
	    k = min(R->TermWin.nscrolled, prev_nrow - nrow);
	    rxvt_scroll_text(aR_ 0, (int)prev_nrow - 1, k, 1);
	    for (p = nrow; p < prev_nrow; p++) {
		q = p + R->TermWin.saveLines;
		if (R->screen.text[q]) {
#ifdef DEBUG_STRICT
		    assert(R->screen.rend[q]);
#endif
		    free(R->screen.text[q]);
		    free(R->screen.rend[q]);
		}
		if (R->swap.text[p]) {
#ifdef DEBUG_STRICT
		    assert(R->swap.rend[p]);
#endif
		    free(R->swap.text[p]);
		    free(R->swap.rend[p]);
		}
#ifdef DEBUG_STRICT
		assert(R->drawn_text[p] && R->drawn_rend[p]);
#endif
		free(R->drawn_text[p]);
		free(R->drawn_rend[p]);
	    }
	    /* we have fewer rows so fix up cursor position */
	    MIN_IT(R->screen.cur.row, (int32_t)nrow - 1);
	    MIN_IT(R->swap.cur.row, (int32_t)nrow - 1);

	    rxvt_scr_reset_realloc(aR);	/* realloc _last_ */

	} else if (nrow > prev_nrow) {
	    /* add rows */
	    rxvt_scr_reset_realloc(aR);	/* realloc _first_ */

	    k = min(R->TermWin.nscrolled, nrow - prev_nrow);
	    for (p = prev_total_rows; p < total_rows; p++) {
		R->screen.tlen[p] = 0;
		R->screen.text[p] = NULL;
		R->screen.rend[p] = NULL;
	    }
	    for (p = prev_total_rows; p < total_rows - k; p++)
		rxvt_blank_screen_mem(aR_ R->screen.text, R->screen.rend,
				      p, setrstyle);
	    for (p = prev_nrow; p < nrow; p++) {
		R->swap.tlen[p] = 0;
		R->swap.text[p] = NULL;
		R->swap.rend[p] = NULL;
		R->drawn_text[p] = NULL;
		R->drawn_rend[p] = NULL;
		rxvt_blank_screen_mem(aR_ R->swap.text, R->swap.rend,
				      p, setrstyle);
		rxvt_blank_screen_mem(aR_ R->drawn_text, R->drawn_rend,
				      p, setrstyle);
	    }
	    if (k > 0) {
		rxvt_scroll_text(aR_ 0, (int)nrow - 1, -k, 1);
		R->screen.cur.row += k;
		R->screen.s_cur.row += k;
		R->TermWin.nscrolled -= k;
	    }
#ifdef DEBUG_STRICT
	    assert(R->screen.cur.row < R->TermWin.nrow);
	    assert(R->swap.cur.row < R->TermWin.nrow);
#else				/* drive with your eyes closed */
	    MIN_IT(R->screen.cur.row, nrow - 1);
	    MIN_IT(R->swap.cur.row, nrow - 1);
#endif
	}
/* B2: resize columns */
	if (ncol != prev_ncol) {
	    for (p = 0; p < total_rows; p++) {
		if (R->screen.text[p]) {
		    R->screen.text[p] = (text_t *)rxvt_realloc(R->screen.text[p], ncol * sizeof(text_t));
		    R->screen.rend[p] = (rend_t *)rxvt_realloc(R->screen.rend[p], ncol * sizeof(rend_t));
		    MIN_IT(R->screen.tlen[p], (int16_t)ncol);
		    if (ncol > prev_ncol)
			rxvt_blank_line(aR_
                                        &(R->screen.text[p][prev_ncol]),
					&(R->screen.rend[p][prev_ncol]),
					ncol - prev_ncol,
					setrstyle);
		}
	    }
	    for (p = 0; p < nrow; p++) {
		R->drawn_text[p] = (text_t *)rxvt_realloc(R->drawn_text[p], ncol * sizeof(text_t));
		R->drawn_rend[p] = (rend_t *)rxvt_realloc(R->drawn_rend[p], ncol * sizeof(rend_t));
		if (R->swap.text[p]) {
		    R->swap.text[p] = (text_t *)rxvt_realloc(R->swap.text[p], ncol * sizeof(text_t));
		    R->swap.rend[p] = (rend_t *)rxvt_realloc(R->swap.rend[p], ncol * sizeof(rend_t));
		    MIN_IT(R->swap.tlen[p], (int16_t)ncol);
		    if (ncol > prev_ncol)
			rxvt_blank_line(aR_
                                        &(R->swap.text[p][prev_ncol]),
					&(R->swap.rend[p][prev_ncol]),
					ncol - prev_ncol, setrstyle);
		}
		if (ncol > prev_ncol)
		    rxvt_blank_line(aR_
                                    &(R->drawn_text[p][prev_ncol]),
				    &(R->drawn_rend[p][prev_ncol]),
				    ncol - prev_ncol, setrstyle);
	    }
	    MIN_IT(R->screen.cur.col, (int16_t)ncol - 1);
	    MIN_IT(R->swap.cur.col, (int16_t)ncol - 1);
	}
	if (R->tabs)
	    free(R->tabs);
    }

    R->tabs = (char *)rxvt_malloc(ncol * sizeof(char));

    for (p = 0; p < ncol; p++)
	R->tabs[p] = (p % TABSIZE == 0) ? 1 : 0;

    R->prev_nrow = nrow;
    R->prev_ncol = ncol;

    rxvt_tt_winsize(R->cmd_fd, R->TermWin.ncol, R->TermWin.nrow, R->cmd_pid);
}

/* INTPROTO */
void
rxvt_scr_reset_realloc(pR)
{
    uint16_t       total_rows, nrow;

    nrow = R->TermWin.nrow;
    total_rows = nrow + R->TermWin.saveLines;
/* *INDENT-OFF* */
    R->screen.text = (text_t **)rxvt_realloc(R->screen.text, total_rows * sizeof(text_t *));
    R->buf_text    = (text_t **)rxvt_realloc(R->buf_text   , total_rows * sizeof(text_t *));
    R->drawn_text  = (text_t **)rxvt_realloc(R->drawn_text , nrow       * sizeof(text_t *));
    R->swap.text   = (text_t **)rxvt_realloc(R->swap.text  , nrow       * sizeof(text_t *));

    R->screen.tlen = (int16_t *)rxvt_realloc(R->screen.tlen, total_rows * sizeof(int16_t));
    R->swap.tlen   = (int16_t *)rxvt_realloc(R->swap.tlen  , total_rows * sizeof(int16_t));

    R->screen.rend = (rend_t **)rxvt_realloc(R->screen.rend, total_rows * sizeof(rend_t *));
    R->buf_rend    = (rend_t **)rxvt_realloc(R->buf_rend   , total_rows * sizeof(rend_t *));
    R->drawn_rend  = (rend_t **)rxvt_realloc(R->drawn_rend , nrow       * sizeof(rend_t *));
    R->swap.rend   = (rend_t **)rxvt_realloc(R->swap.rend  , nrow       * sizeof(rend_t *));
/* *INDENT-ON* */
}

/* ------------------------------------------------------------------------- */
/*
 * Free everything.  That way malloc debugging can find leakage.
 */
/* EXTPROTO */
void
rxvt_scr_release(pR)
{
    uint16_t       total_rows;
    int             i;

    total_rows = R->TermWin.nrow + R->TermWin.saveLines;
    for (i = 0; i < total_rows; i++) {
	if (R->screen.text[i]) {	/* then so is R->screen.rend[i] */
	    free(R->screen.text[i]);
#ifdef DEBUG_STRICT
	    assert(R->screen.rend[i]);
#endif
	    free(R->screen.rend[i]);
	}
    }
    for (i = 0; i < R->TermWin.nrow; i++) {
	free(R->drawn_text[i]);
	free(R->drawn_rend[i]);
	free(R->swap.text[i]);
	free(R->swap.rend[i]);
    }
    free(R->screen.text);
    free(R->screen.tlen);
    free(R->screen.rend);
    free(R->drawn_text);
    free(R->drawn_rend);
    free(R->swap.text);
    free(R->swap.tlen);
    free(R->swap.rend);
    free(R->buf_text);
    free(R->buf_rend);
    free(R->tabs);

/* NULL these so if anything tries to use them, we'll know about it */
    R->screen.text = R->drawn_text = R->swap.text = NULL;
    R->screen.rend = R->drawn_rend = R->swap.rend = NULL;
    R->screen.tlen = R->swap.tlen = NULL;
    R->buf_text = NULL;
    R->buf_rend = NULL;
    R->tabs = NULL;
}

/* ------------------------------------------------------------------------- */
/*
 * Hard reset
 */
/* EXTPROTO */
void
rxvt_scr_poweron(pR)
{
    D_SCREEN((stderr, "rxvt_scr_poweron()"));

    rxvt_scr_release(aR);
    R->prev_nrow = R->prev_ncol = 0;
    rxvt_scr_reset(aR);

    rxvt_scr_clear(aR);
    rxvt_scr_refresh(aR_ SLOW_REFRESH);
#ifdef RXVT_GRAPHICS
    rxvt_Gr_reset(aR);
#endif
}

/* ------------------------------------------------------------------------- *
 *                         PROCESS SCREEN COMMANDS                           *
 * ------------------------------------------------------------------------- */
/*
 * Save and Restore cursor
 * XTERM_SEQ: Save cursor   : ESC 7
 * XTERM_SEQ: Restore cursor: ESC 8
 */
/* EXTPROTO */
void
rxvt_scr_cursor(pR_ int mode)
{
    screen_t       *s;

    D_SCREEN((stderr, "rxvt_scr_cursor(%c)", mode));

#if NSCREENS && !defined(NO_SECONDARY_SCREEN_CURSOR)
    if (R->current_screen == SECONDARY)
	s = &(R->swap);
    else
#endif
	s = &(R->screen);
    switch (mode) {
    case SAVE:
	s->s_cur.row = s->cur.row;
	s->s_cur.col = s->cur.col;
	s->s_rstyle = R->rstyle;
	s->s_charset = s->charset;
	s->s_charset_char = R->charsets[s->charset];
	break;
    case RESTORE:
	R->want_refresh = 1;
	s->cur.row = s->s_cur.row;
	s->cur.col = s->s_cur.col;
	s->flags &= ~Screen_WrapNext;
	R->rstyle = s->s_rstyle;
	s->charset = s->s_charset;
	R->charsets[s->charset] = s->s_charset_char;
	rxvt_set_font_style(aR);
	break;
    }
/* boundary check in case screen size changed between SAVE and RESTORE */
    MIN_IT(s->cur.row, R->TermWin.nrow - 1);
    MIN_IT(s->cur.col, R->TermWin.ncol - 1);
#ifdef DEBUG_STRICT
    assert(s->cur.row >= 0);
    assert(s->cur.col >= 0);
#else				/* drive with your eyes closed */
    MAX_IT(s->cur.row, 0);
    MAX_IT(s->cur.col, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Swap between primary and secondary screens
 * XTERM_SEQ: Primary screen  : ESC [ ? 4 7 h
 * XTERM_SEQ: Secondary screen: ESC [ ? 4 7 l
 */
/* EXTPROTO */
int
rxvt_scr_change_screen(pR_ int scrn)
{
    int             i;
#if NSCREENS
    int             offset;
#endif

    R->want_refresh = 1;

    D_SCREEN((stderr, "rxvt_scr_change_screen(%d)", scrn));

    R->TermWin.view_start = 0;
    RESET_CHSTAT(R);

    if (R->current_screen == scrn)
	return R->current_screen;

    rxvt_selection_check(aR_ 2);	/* check for boundary cross */

    SWAP_IT(R->current_screen, scrn, int);
#if NSCREENS
    R->num_scr = 0;
    offset = R->TermWin.saveLines;
    for (i = R->prev_nrow; i--;) {
	SWAP_IT(R->screen.text[i + offset], R->swap.text[i], text_t *);
	SWAP_IT(R->screen.tlen[i + offset], R->swap.tlen[i], int16_t);
	SWAP_IT(R->screen.rend[i + offset], R->swap.rend[i], rend_t *);
    }
    SWAP_IT(R->screen.cur.row, R->swap.cur.row, int16_t);
    SWAP_IT(R->screen.cur.col, R->swap.cur.col, int16_t);
# ifdef DEBUG_STRICT
    assert((R->screen.cur.row >= 0) && (R->screen.cur.row < R->prev_nrow));
    assert((R->screen.cur.col >= 0) && (R->screen.cur.col < R->prev_ncol));
# else				/* drive with your eyes closed */
    MAX_IT(R->screen.cur.row, 0);
    MIN_IT(R->screen.cur.row, (int32_t)R->prev_nrow - 1);
    MAX_IT(R->screen.cur.col, 0);
    MIN_IT(R->screen.cur.col, (int32_t)R->prev_ncol - 1);
# endif
    SWAP_IT(R->screen.charset, R->swap.charset, int16_t);
    SWAP_IT(R->screen.flags, R->swap.flags, int);
    R->screen.flags |= Screen_VisibleCursor;
    R->swap.flags |= Screen_VisibleCursor;

# ifdef RXVT_GRAPHICS

    if (rxvt_Gr_Displayed(aR)) {
	rxvt_Gr_scroll(aR_ 0);
	rxvt_Gr_ChangeScreen(aR);
    }
# endif
#else
# ifdef SCROLL_ON_NO_SECONDARY
#  ifdef RXVT_GRAPHICS
    if (rxvt_Gr_Displayed(aR))
	rxvt_Gr_ClearScreen(aR);
#  endif
    if (R->current_screen == PRIMARY
#  ifdef RXVT_GRAPHICS
        && !rxvt_Gr_Displayed(aR)
#  endif
        )
	rxvt_scroll_text(aR_ 0, (R->prev_nrow - 1), R->prev_nrow, 0);
# endif
#endif
    return scrn;
}

/* ------------------------------------------------------------------------- */
/*
 * Change the colour for following text
 */
/* EXTPROTO */
void
rxvt_scr_color(pR_ unsigned int color, int fgbg)
{
    color &= RS_fgMask;
    if (fgbg == Color_fg)
	R->rstyle = SET_FGCOLOR(R->rstyle, color);
    else 
	R->rstyle = SET_BGCOLOR(R->rstyle, color);
}

/* ------------------------------------------------------------------------- */
/*
 * Change the rendition style for following text
 */
/* EXTPROTO */
void
rxvt_scr_rendition(pR_ int set, int style)
{
    if (set)
	R->rstyle |= style;
    else if (style == ~RS_None)
	R->rstyle = DEFAULT_RSTYLE;
    else
	R->rstyle &= ~style;
}

/* ------------------------------------------------------------------------- */
/*
 * Scroll text between <row1> and <row2> inclusive, by <count> lines
 * count positive ==> scroll up
 * count negative ==> scroll down
 * spec == 0 for normal routines
 */
/* EXTPROTO */
int
rxvt_scroll_text(pR_ int row1, int row2, int count, int spec)
{
    int             i, j;
    long            nscrolled;

    if (count == 0 || (row1 > row2))
	return 0;

    R->want_refresh = 1;
    D_SCREEN((stderr, "rxvt_scroll_text(%d,%d,%d,%d): %s", row1, row2, count, spec, (R->current_screen == PRIMARY) ? "Primary" : "Secondary"));

    if ((count > 0) && (row1 == 0) && (R->current_screen == PRIMARY)) {
	nscrolled = (long)R->TermWin.nscrolled + (long)count;;
	if (nscrolled > (long)R->TermWin.saveLines)
	    R->TermWin.nscrolled = R->TermWin.saveLines;
	else
	    R->TermWin.nscrolled = (uint16_t)nscrolled;
	if ((R->Options & Opt_scrollWithBuffer)
	    && R->TermWin.view_start != 0
	    && R->TermWin.view_start != R->TermWin.saveLines)
	    rxvt_scr_page(aR_ UP, count);
    } else if (!spec)
	row1 += R->TermWin.saveLines;
    row2 += R->TermWin.saveLines;

    if (R->selection.op && R->current_screen == R->selection.screen) {
	i = R->selection.beg.row + R->TermWin.saveLines;
	j = R->selection.end.row + R->TermWin.saveLines;
	if ((i < row1 && j > row1)
	    || (i < row2 && j > row2)
	    || (i - count < row1 && i >= row1)
	    || (i - count > row2 && i <= row2)
	    || (j - count < row1 && j >= row1)
	    || (j - count > row2 && j <= row2)) {
	    CLEAR_ALL_SELECTION(R);
	    R->selection.op = SELECTION_CLEAR;	/* XXX: too aggressive? */
	} else if (j >= row1 && j <= row2) {
	    /* move selected region too */
	    R->selection.beg.row -= count;
	    R->selection.end.row -= count;
	    R->selection.mark.row -= count;
	}
    }
    rxvt_selection_check(aR_ 0);	/* _after_ R->TermWin.nscrolled update */

    R->num_scr += count;
    j = count;
    if (count < 0)
	count = -count;
    i = row2 - row1 + 1;
    MIN_IT(count, i);

    if (j > 0) {
/* A: scroll up */

/* A1: Copy lines that will get clobbered by the rotation */
	for (i = 0, j = row1; i < count; i++, j++) {
	    R->buf_text[i] = R->screen.text[j];
	    R->buf_rend[i] = R->screen.rend[j];
	}
/* A2: Rotate lines */
	for (j = row1, i = j + count; i <= row2; i++, j++) {
	    R->screen.tlen[j] = R->screen.tlen[i];
	    R->screen.text[j] = R->screen.text[i];
	    R->screen.rend[j] = R->screen.rend[i];
	}
	j = row2 - count + 1, i = count;
    } else /* if (j < 0) */ {
/* B: scroll down */

/* B1: Copy lines that will get clobbered by the rotation */
	for (i = 0, j = row2; i < count; i++, j--) {
	    R->buf_text[i] = R->screen.text[j];
	    R->buf_rend[i] = R->screen.rend[j];
	}
/* B2: Rotate lines */
	for (j = row2, i = j - count; i >= row1; i--, j--) {
	    R->screen.tlen[j] = R->screen.tlen[i];
	    R->screen.text[j] = R->screen.text[i];
	    R->screen.rend[j] = R->screen.rend[i];
	}
	j = row1, i = count;
	count = -count;
    }

/* C: Resurrect lines */
    for (; i--; j++) {
	R->screen.tlen[j] = 0;
	R->screen.text[j] = R->buf_text[i];
	R->screen.rend[j] = R->buf_rend[i];
	if (!spec)		/* line length may not equal TermWin.ncol */
	    rxvt_blank_screen_mem(aR_ R->screen.text, R->screen.rend,
				  (unsigned int)j, R->rstyle);
    }

#ifdef RXVT_GRAPHICS
    if (rxvt_Gr_Displayed(aR))
	rxvt_Gr_scroll(aR_ count);
#endif
    return count;
}

/* ------------------------------------------------------------------------- */
/*
 * Add text given in <str> of length <len> to screen struct
 */
/* EXTPROTO */
void
rxvt_scr_add_lines(pR_ const uint32_t *str, int nlines, int len)
{
    unsigned char   checksel, clearsel;
    uint32_t        c;
    int             i, row, last_col;
    text_t         *stp;
    rend_t         *srp;

    if (len <= 0)		/* sanity */
	return;

    R->want_refresh = 1;
    last_col = R->TermWin.ncol;

    D_SCREEN((stderr, "rxvt_scr_add_lines(%d,%d)", nlines, len));
    ZERO_SCROLLBACK(R);
    if (nlines > 0) {
	nlines += (R->screen.cur.row - R->screen.bscroll);
	if ((nlines > 0)
	    && (R->screen.tscroll == 0)
	    && (R->screen.bscroll == (R->TermWin.nrow - 1))) {
	    /* _at least_ this many lines need to be scrolled */
	    rxvt_scroll_text(aR_ R->screen.tscroll, R->screen.bscroll, nlines,
			     0);
	    R->screen.cur.row -= nlines;
	}
    }
#ifdef DEBUG_STRICT
    assert(R->screen.cur.col < last_col);
    assert((R->screen.cur.row < R->TermWin.nrow)
	   && (R->screen.cur.row >= -(int32_t)R->TermWin.nscrolled));
#else				/* drive with your eyes closed */
    MIN_IT(R->screen.cur.col, last_col - 1);
    MIN_IT(R->screen.cur.row, (int32_t)R->TermWin.nrow - 1);
    MAX_IT(R->screen.cur.row, -(int32_t)R->TermWin.nscrolled);
#endif
    row = R->screen.cur.row + R->TermWin.saveLines;

    checksel = (R->selection.op
		&& R->current_screen == R->selection.screen) ? 1 : 0;
    clearsel = 0;

    stp = R->screen.text[row];
    srp = R->screen.rend[row];

    for (i = 0; i < len;) {
	c = str[i++];
	switch (c) {
	case '\t':
	    rxvt_scr_tab(aR_ 1);
	    continue;
	case '\n':
	    if (R->screen.tlen[row] != -1)	/* XXX: think about this */
		MAX_IT(R->screen.tlen[row], R->screen.cur.col);
	    R->screen.flags &= ~Screen_WrapNext;
	    if (R->screen.cur.row == R->screen.bscroll)
		rxvt_scroll_text(aR_ R->screen.tscroll, R->screen.bscroll, 1, 0);
	    else if (R->screen.cur.row < (R->TermWin.nrow - 1))
		row = (++R->screen.cur.row) + R->TermWin.saveLines;
	    stp = R->screen.text[row];	/* _must_ refresh */
	    srp = R->screen.rend[row];	/* _must_ refresh */
	    RESET_CHSTAT(R);
	    continue;
	case '\r':
	    if (R->screen.tlen[row] != -1)	/* XXX: think about this */
		MAX_IT(R->screen.tlen[row], R->screen.cur.col);
	    R->screen.flags &= ~Screen_WrapNext;
	    R->screen.cur.col = 0;
	    RESET_CHSTAT(R);
	    continue;
	default:
	    if (c == 127)
		continue;	/* yummmm..... */
	    break;
	}

	if (checksel		/* see if we're writing within selection */
	    && !ROWCOL_IS_BEFORE(R->screen.cur, R->selection.beg)
	    && ROWCOL_IS_BEFORE(R->screen.cur, R->selection.end)) {
	    checksel = 0;
	    clearsel = 1;
	}
	if (R->screen.flags & Screen_WrapNext) {
	    R->screen.tlen[row] = -1;
	    if (R->screen.cur.row == R->screen.bscroll)
		rxvt_scroll_text(aR_ R->screen.tscroll, R->screen.bscroll, 1, 0);
	    else if (R->screen.cur.row < (R->TermWin.nrow - 1))
		row = (++R->screen.cur.row) + R->TermWin.saveLines;
	    stp = R->screen.text[row];	/* _must_ refresh */
	    srp = R->screen.rend[row];	/* _must_ refresh */
	    R->screen.cur.col = 0;
	    R->screen.flags &= ~Screen_WrapNext;
	}
	if (R->screen.flags & Screen_Insert)
	    rxvt_scr_insdel_chars(aR_ 1, INSERT);

        if (R->charsets[R->screen.charset] == '0') // DEC SPECIAL
          switch (c)
            {
              case '+': c = 0x2192; break; case ',': c = 0x2190; break; case '-': c = 0x2191; break;
              case '.': c = 0x2193; break; case '0': c = 0x25ae; break; case '`': c = 0x25c6; break;
              case 'a': c = 0x2592; break; case 'f': c = 0x00b0; break; case 'g': c = 0x00b1; break;
              case 'h': c = 0x2592; break; case 'i': c = 0x2603; break; case 'j': c = 0x2518; break;
              case 'k': c = 0x2510; break; case 'l': c = 0x250c; break; case 'm': c = 0x2514; break;
              case 'n': c = 0x253c; break; case 'o': c = 0x23ba; break; case 'p': c = 0x23bb; break;
              case 'q': c = 0x2500; break; case 'r': c = 0x23bc; break; case 's': c = 0x23bd; break;
              case 't': c = 0x251c; break; case 'u': c = 0x2524; break; case 'v': c = 0x2534; break;
              case 'w': c = 0x252c; break; case 'x': c = 0x2502; break; case 'y': c = 0x2264; break;
              case 'z': c = 0x2265; break; case '{': c = 0x03c0; break; case '|': c = 0x2260; break;
              case '}': c = 0x00a3; break; case '~': c = 0x00b7; break;
            }

        rend_t rend = SET_FONT (R->rstyle, R->TermWin.fontset->find_font (c));

	stp[R->screen.cur.col] = c;
	srp[R->screen.cur.col] = rend;

        if (c > 255)
          {
            // rely on wcwidth to tell us the character width, at least for non iso-8859-1
            int width = wcwidth (c);
            
            if (width > 1)
              {
                while (--width > 0 && R->screen.cur.col < last_col - 1)
                  {

                    srp[R->screen.cur.col] |= RS_wide;

                    R->screen.cur.col++;
                    stp[R->screen.cur.col] = NOCHAR;
                    srp[R->screen.cur.col] = rend;
                  }
              }
          }

	if (R->screen.cur.col < last_col - 1)
	    R->screen.cur.col++;
	else {
	    R->screen.tlen[row] = last_col;
	    if (R->screen.flags & Screen_Autowrap)
		R->screen.flags |= Screen_WrapNext;
	}
    }
    if (R->screen.tlen[row] != -1)	/* XXX: think about this */
	MAX_IT(R->screen.tlen[row], R->screen.cur.col);

/*
 * If we wrote anywhere in the selected area, kill the selection
 * XXX: should we kill the mark too?  Possibly, but maybe that
 *      should be a similar check.
 */
    if (clearsel)
	CLEAR_SELECTION(R);

#ifdef DEBUG_STRICT
    assert(R->screen.cur.row >= 0);
#else				/* drive with your eyes closed */
    MAX_IT(R->screen.cur.row, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Process Backspace.  Move back the cursor back a position, wrap if have to
 * XTERM_SEQ: CTRL-H
 */
/* EXTPROTO */
void
rxvt_scr_backspace(pR)
{
    RESET_CHSTAT(R);
    R->want_refresh = 1;
    if (R->screen.cur.col == 0) {
	if (R->screen.cur.row > 0) {
#ifdef TERMCAP_HAS_BW
	    R->screen.cur.col = R->TermWin.ncol - 1;
	    R->screen.cur.row--;
	    return;
#endif
	}
    } else if ((R->screen.flags & Screen_WrapNext) == 0)
	rxvt_scr_gotorc(aR_ 0, -1, RELATIVE);
    R->screen.flags &= ~Screen_WrapNext;
}

/* ------------------------------------------------------------------------- */
/*
 * Process Horizontal Tab
 * count: +ve = forward; -ve = backwards
 * XTERM_SEQ: CTRL-I
 */
/* EXTPROTO */
void
rxvt_scr_tab(pR_ int count)
{
    int             i, x;

    D_SCREEN((stderr, "rxvt_scr_tab(%d)", count));
    R->want_refresh = 1;
    RESET_CHSTAT(R);
    i = x = R->screen.cur.col;
    if (count == 0)
	return;
    else if (count > 0) {
	for (; ++i < R->TermWin.ncol; )
	    if (R->tabs[i]) {
		x = i;
		if (!--count)
		    break;
	    }
	if (count)
	    x = R->TermWin.ncol - 1;
    } else /* if (count < 0) */ {
	for (; --i >= 0; )
	    if (R->tabs[i]) {
		x = i;
		if (!++count)
		    break;
	    }
	if (count)
	    x = 0;
    }
    if (x != R->screen.cur.col)
	rxvt_scr_gotorc(aR_ 0, x, R_RELATIVE);
}

/* ------------------------------------------------------------------------- */
/*
 * Process DEC Back Index
 * XTERM_SEQ: ESC 6
 * Move cursor left in row.  If we're at the left boundary, shift everything
 * in that row right.  Clear left column.
 */
#ifndef NO_FRILLS
/* EXTPROTO */
void
rxvt_scr_backindex(pR)
{
    if (R->screen.cur.col > 0)
	rxvt_scr_gotorc(aR_ 0, -1, R_RELATIVE | C_RELATIVE);
    else {
	if (R->screen.tlen[R->screen.cur.row + R->TermWin.saveLines] == 0)
	    return;		/* um, yeah? */
	rxvt_scr_insdel_chars(aR_ 1, INSERT);
    }
}
#endif
/* ------------------------------------------------------------------------- */
/*
 * Process DEC Forward Index
 * XTERM_SEQ: ESC 9
 * Move cursor right in row.  If we're at the right boundary, shift everything
 * in that row left.  Clear right column.
 */
#ifndef NO_FRILLS
/* EXTPROTO */
void
rxvt_scr_forwardindex(pR)
{
    int             row;

    if (R->screen.cur.col < R->TermWin.ncol - 1)
	rxvt_scr_gotorc(aR_ 0, 1, R_RELATIVE | C_RELATIVE);
    else {
	row = R->screen.cur.row + R->TermWin.saveLines;
	if (R->screen.tlen[row] == 0)
	    return;		/* um, yeah? */
	else if (R->screen.tlen[row] == -1)
	    R->screen.tlen[row] = R->TermWin.ncol;
	rxvt_scr_gotorc(aR_ 0, 0, R_RELATIVE);
	rxvt_scr_insdel_chars(aR_ 1, DELETE);
	rxvt_scr_gotorc(aR_ 0, R->TermWin.ncol - 1, R_RELATIVE);
    }
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Goto Row/Column
 */
/* EXTPROTO */
void
rxvt_scr_gotorc(pR_ int row, int col, int relative)
{
    R->want_refresh = 1;
    ZERO_SCROLLBACK(R);
    RESET_CHSTAT(R);
#ifdef RXVT_GRAPHICS
    if (rxvt_Gr_Displayed(aR))
	rxvt_Gr_scroll(aR_ 0);
#endif

    D_SCREEN((stderr, "rxvt_scr_gotorc(r:%s%d,c:%s%d): from (r:%d,c:%d)", (relative & R_RELATIVE ? "+" : ""), row, (relative & C_RELATIVE ? "+" : ""), col, R->screen.cur.row, R->screen.cur.col));

    R->screen.cur.col = ((relative & C_RELATIVE) ? (R->screen.cur.col + col)
						 : col);
    MAX_IT(R->screen.cur.col, 0);
    MIN_IT(R->screen.cur.col, (int32_t)R->TermWin.ncol - 1);

    R->screen.flags &= ~Screen_WrapNext;
    if (relative & R_RELATIVE) {
	if (row > 0) {
	    if (R->screen.cur.row <= R->screen.bscroll
		&& (R->screen.cur.row + row) > R->screen.bscroll)
		R->screen.cur.row = R->screen.bscroll;
	    else
		R->screen.cur.row += row;
	} else if (row < 0) {
	    if (R->screen.cur.row >= R->screen.tscroll
		&& (R->screen.cur.row + row) < R->screen.tscroll)
		R->screen.cur.row = R->screen.tscroll;
	    else
		R->screen.cur.row += row;
	}
    } else {
	if (R->screen.flags & Screen_Relative) {	/* relative origin mode */
	    R->screen.cur.row = row + R->screen.tscroll;
	    MIN_IT(R->screen.cur.row, R->screen.bscroll);
	} else
	    R->screen.cur.row = row;
    }
    MAX_IT(R->screen.cur.row, 0);
    MIN_IT(R->screen.cur.row, (int32_t)R->TermWin.nrow - 1);
}

/* ------------------------------------------------------------------------- */
/*
 * direction  should be UP or DN
 */
/* EXTPROTO */
void
rxvt_scr_index(pR_ enum page_dirn direction)
{
    int             dirn;

    R->want_refresh = 1;
    dirn = ((direction == UP) ? 1 : -1);
    D_SCREEN((stderr, "rxvt_scr_index(%d)", dirn));

    ZERO_SCROLLBACK(R);
    RESET_CHSTAT(R);

#ifdef RXVT_GRAPHICS
    if (rxvt_Gr_Displayed(aR))
	rxvt_Gr_scroll(aR_ 0);
#endif

    R->screen.flags &= ~Screen_WrapNext;
    if ((R->screen.cur.row == R->screen.bscroll && direction == UP)
	|| (R->screen.cur.row == R->screen.tscroll && direction == DN))
	rxvt_scroll_text(aR_ R->screen.tscroll, R->screen.bscroll, dirn, 0);
    else
	R->screen.cur.row += dirn;
    MAX_IT(R->screen.cur.row, 0);
    MIN_IT(R->screen.cur.row, (int32_t)R->TermWin.nrow - 1);
    rxvt_selection_check(aR_ 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part or whole of a line
 * XTERM_SEQ: Clear line to right: ESC [ 0 K
 * XTERM_SEQ: Clear line to left : ESC [ 1 K
 * XTERM_SEQ: Clear whole line   : ESC [ 2 K
 */
/* EXTPROTO */
void
rxvt_scr_erase_line(pR_ int mode)
{
    unsigned int    row, col, num;

    R->want_refresh = 1;
    D_SCREEN((stderr, "rxvt_scr_erase_line(%d) at screen row: %d", mode, R->screen.cur.row));
    ZERO_SCROLLBACK(R);
    RESET_CHSTAT(R);

#ifdef RXVT_GRAPHICS
    if (rxvt_Gr_Displayed(aR))
	rxvt_Gr_scroll(aR_ 0);
#endif

    rxvt_selection_check(aR_ 1);

    R->screen.flags &= ~Screen_WrapNext;

    row = R->TermWin.saveLines + R->screen.cur.row;
    switch (mode) {
    case 0:			/* erase to end of line */
	col = R->screen.cur.col;
	num = R->TermWin.ncol - col;
	MIN_IT(R->screen.tlen[row], (int16_t)col);
	if (ROWCOL_IN_ROW_AT_OR_AFTER(R->selection.beg, R->screen.cur)
	    || ROWCOL_IN_ROW_AT_OR_AFTER(R->selection.end, R->screen.cur))
	    CLEAR_SELECTION(R);
	break;
    case 1:			/* erase to beginning of line */
	col = 0;
	num = R->screen.cur.col + 1;
	if (ROWCOL_IN_ROW_AT_OR_BEFORE(R->selection.beg, R->screen.cur)
	    || ROWCOL_IN_ROW_AT_OR_BEFORE(R->selection.end, R->screen.cur))
	    CLEAR_SELECTION(R);
	break;
    case 2:			/* erase whole line */
	col = 0;
	num = R->TermWin.ncol;
	R->screen.tlen[row] = 0;
	if (R->selection.beg.row <= R->screen.cur.row
	    && R->selection.end.row >= R->screen.cur.row)
	    CLEAR_SELECTION(R);
	break;
    default:
	return;
    }
    if (R->screen.text[row])
	rxvt_blank_line(aR_
                        &(R->screen.text[row][col]),
			&(R->screen.rend[row][col]), num, R->rstyle);
    else
	rxvt_blank_screen_mem(aR_ R->screen.text, R->screen.rend, row,
			      R->rstyle);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part of whole of the screen
 * XTERM_SEQ: Clear screen after cursor : ESC [ 0 J
 * XTERM_SEQ: Clear screen before cursor: ESC [ 1 J
 * XTERM_SEQ: Clear whole screen        : ESC [ 2 J
 */
/* EXTPROTO */
void
rxvt_scr_erase_screen(pR_ int mode)
{
    int             num;
    int32_t         row, row_offset;
    rend_t          ren;
    XGCValues       gcvalue;

    R->want_refresh = 1;
    D_SCREEN((stderr, "rxvt_scr_erase_screen(%d) at screen row: %d", mode, R->screen.cur.row));
    ZERO_SCROLLBACK(R);
    RESET_CHSTAT(R);
    row_offset = (int32_t)R->TermWin.saveLines;

    switch (mode) {
    case 0:			/* erase to end of screen */
	rxvt_selection_check(aR_ 1);
	rxvt_scr_erase_line(aR_ 0);
	row = R->screen.cur.row + 1;	/* possible OOB */
	num = R->TermWin.nrow - row;
	break;
    case 1:			/* erase to beginning of screen */
	rxvt_selection_check(aR_ 3);
	rxvt_scr_erase_line(aR_ 1);
	row = 0;
	num = R->screen.cur.row;
	break;
    case 2:			/* erase whole screen */
	rxvt_selection_check(aR_ 3);
#ifdef RXVT_GRAPHICS
	rxvt_Gr_ClearScreen(aR);
#endif
	row = 0;
	num = R->TermWin.nrow;
	break;
    default:
	return;
    }
    R->refresh_type |= REFRESH_BOUNDS;
    if (R->selection.op && R->current_screen == R->selection.screen
	&& ((R->selection.beg.row >= row && R->selection.beg.row <= row + num)
	    || (R->selection.end.row >= row
		&& R->selection.end.row <= row + num)))
	CLEAR_SELECTION(R);
    if (row >= R->TermWin.nrow)	/* Out Of Bounds */
	return;
    MIN_IT(num, (R->TermWin.nrow - row));
    if (R->rstyle & (RS_RVid | RS_Uline))
	ren = (rend_t) ~RS_None;
    else if (GET_BASEBG(R->rstyle) == Color_bg) {
	ren = DEFAULT_RSTYLE;
	CLEAR_ROWS(row, num);
    } else {
	ren = (R->rstyle & (RS_fgMask | RS_bgMask));
	gcvalue.foreground = R->PixColors[GET_BGCOLOR(R->rstyle)];
	XChangeGC(R->Xdisplay, R->TermWin.gc, GCForeground, &gcvalue);
	ERASE_ROWS(row, num);
	gcvalue.foreground = R->PixColors[Color_fg];
	XChangeGC(R->Xdisplay, R->TermWin.gc, GCForeground, &gcvalue);
    }
    for (; num--; row++) {
	rxvt_blank_screen_mem(aR_ R->screen.text, R->screen.rend,
			      (unsigned int)(row + row_offset), R->rstyle);
	R->screen.tlen[row + row_offset] = 0;
	rxvt_blank_line(aR_
                        R->drawn_text[row], R->drawn_rend[row],
			(unsigned int)R->TermWin.ncol, ren);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Fill the screen with `E's
 * XTERM_SEQ: Screen Alignment Test: ESC # 8
 */
/* EXTPROTO */
void
rxvt_scr_E(pR)
{
    int             i, j, k;
    rend_t         *r1, fs;

    R->want_refresh = 1;
    R->num_scr_allow = 0;
    ZERO_SCROLLBACK(R);
    RESET_CHSTAT(R);
    rxvt_selection_check(aR_ 3);

    fs = SET_FONT (R->rstyle, R->TermWin.fontset->find_font ('E'));
    for (k = R->TermWin.saveLines, i = R->TermWin.nrow; i--; k++) {
	R->screen.tlen[k] = R->TermWin.ncol;	/* make the `E's selectable */
	fill_text (R->screen.text[k], 'E', R->TermWin.ncol);
	for (r1 = R->screen.rend[k], j = R->TermWin.ncol; j--; )
	    *r1++ = fs;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> lines
 */
/* EXTPROTO */
void
rxvt_scr_insdel_lines(pR_ int count, int insdel)
{
    int             end;

    ZERO_SCROLLBACK(R);
    RESET_CHSTAT(R);

#ifdef RXVT_GRAPHICS
    if (rxvt_Gr_Displayed(aR))
	rxvt_Gr_scroll(aR_ 0);
#endif

    rxvt_selection_check(aR_ 1);

    if (R->screen.cur.row > R->screen.bscroll)
	return;

    end = R->screen.bscroll - R->screen.cur.row + 1;
    if (count > end) {
	if (insdel == DELETE)
	    return;
	else if (insdel == INSERT)
	    count = end;
    }
    R->screen.flags &= ~Screen_WrapNext;

    rxvt_scroll_text(aR_ R->screen.cur.row, R->screen.bscroll, insdel * count,
		     0);
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> characters from the current position
 */
/* EXTPROTO */
void
rxvt_scr_insdel_chars(pR_ int count, int insdel)
{
    int             col, row;
    rend_t          tr;
    text_t         *stp;
    rend_t         *srp;
    int16_t        *slp;

    R->want_refresh = 1;
    ZERO_SCROLLBACK(R);
#if 0
    RESET_CHSTAT(R);
#endif

#ifdef RXVT_GRAPHICS
    if (rxvt_Gr_Displayed(aR))
	rxvt_Gr_scroll(aR_ 0);
#endif

    if (count <= 0)
	return;

    rxvt_selection_check(aR_ 1);
    MIN_IT(count, (R->TermWin.ncol - R->screen.cur.col));

    row = R->screen.cur.row + R->TermWin.saveLines;
    R->screen.flags &= ~Screen_WrapNext;

    stp = R->screen.text[row];
    srp = R->screen.rend[row];
    slp = &(R->screen.tlen[row]);
    switch (insdel) {
    case INSERT:
	for (col = R->TermWin.ncol - 1; (col - count) >= R->screen.cur.col;
	     col--) {
	    stp[col] = stp[col - count];
	    srp[col] = srp[col - count];
	}
	if (*slp != -1) {
	    *slp += count;
	    MIN_IT(*slp, R->TermWin.ncol);
	}
	if (R->selection.op && R->current_screen == R->selection.screen
	    && ROWCOL_IN_ROW_AT_OR_AFTER(R->selection.beg, R->screen.cur)) {
	    if (R->selection.end.row != R->screen.cur.row
		|| (R->selection.end.col + count >= R->TermWin.ncol))
		CLEAR_SELECTION(R);
	    else {		/* shift selection */
		R->selection.beg.col += count;
		R->selection.mark.col += count;	/* XXX: yes? */
		R->selection.end.col += count;
	    }
	}
	rxvt_blank_line(aR_ &(stp[R->screen.cur.col]), &(srp[R->screen.cur.col]),
			(unsigned int)count, R->rstyle);
	break;
    case ERASE:
	R->screen.cur.col += count;	/* don't worry if > R->TermWin.ncol */
	rxvt_selection_check(aR_ 1);
	R->screen.cur.col -= count;
	rxvt_blank_line(aR_ &(stp[R->screen.cur.col]), &(srp[R->screen.cur.col]),
			(unsigned int)count, R->rstyle);
	break;
    case DELETE:
	tr = srp[R->TermWin.ncol - 1]
	     & (RS_fgMask | RS_bgMask | RS_baseattrMask);
	for (col = R->screen.cur.col; (col + count) < R->TermWin.ncol; col++) {
	    stp[col] = stp[col + count];
	    srp[col] = srp[col + count];
	}
	rxvt_blank_line(aR_
                        &(stp[R->TermWin.ncol - count]),
			&(srp[R->TermWin.ncol - count]),
			(unsigned int)count, tr);
	if (*slp == -1)	/* break line continuation */
	    *slp = R->TermWin.ncol;
	*slp -= count;
	MAX_IT(*slp, 0);
	if (R->selection.op && R->current_screen == R->selection.screen
	    && ROWCOL_IN_ROW_AT_OR_AFTER(R->selection.beg, R->screen.cur)) {
	    if (R->selection.end.row != R->screen.cur.row
		|| (R->screen.cur.col >= R->selection.beg.col - count)
		|| R->selection.end.col >= R->TermWin.ncol)
		CLEAR_SELECTION(R);
	    else {
		/* shift selection */
		R->selection.beg.col -= count;
		R->selection.mark.col -= count;	/* XXX: yes? */
		R->selection.end.col -= count;
	    }
	}
	break;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Set the scrolling region
 * XTERM_SEQ: Set region <top> - <bot> inclusive: ESC [ <top> ; <bot> r
 */
/* EXTPROTO */
void
rxvt_scr_scroll_region(pR_ int top, int bot)
{
    MAX_IT(top, 0);
    MIN_IT(bot, (int)R->TermWin.nrow - 1);
    if (top > bot)
	return;
    R->screen.tscroll = top;
    R->screen.bscroll = bot;
    rxvt_scr_gotorc(aR_ 0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Make the cursor visible/invisible
 * XTERM_SEQ: Make cursor visible  : ESC [ ? 25 h
 * XTERM_SEQ: Make cursor invisible: ESC [ ? 25 l
 */
/* EXTPROTO */
void
rxvt_scr_cursor_visible(pR_ int mode)
{
    R->want_refresh = 1;
    if (mode)
	R->screen.flags |= Screen_VisibleCursor;
    else
	R->screen.flags &= ~Screen_VisibleCursor;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset automatic wrapping
 * XTERM_SEQ: Set Wraparound  : ESC [ ? 7 h
 * XTERM_SEQ: Unset Wraparound: ESC [ ? 7 l
 */
/* EXTPROTO */
void
rxvt_scr_autowrap(pR_ int mode)
{
    if (mode)
	R->screen.flags |= Screen_Autowrap;
    else
	R->screen.flags &= ~(Screen_Autowrap | Screen_WrapNext);
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset margin origin mode
 * Absolute mode: line numbers are counted relative to top margin of screen
 *      and the cursor can be moved outside the scrolling region.
 * Relative mode: line numbers are relative to top margin of scrolling region
 *      and the cursor cannot be moved outside.
 * XTERM_SEQ: Set Absolute: ESC [ ? 6 h
 * XTERM_SEQ: Set Relative: ESC [ ? 6 l
 */
/* EXTPROTO */
void
rxvt_scr_relative_origin(pR_ int mode)
{
    if (mode)
	R->screen.flags |= Screen_Relative;
    else
	R->screen.flags &= ~Screen_Relative;
    rxvt_scr_gotorc(aR_ 0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set insert/replace mode
 * XTERM_SEQ: Set Insert mode : ESC [ ? 4 h
 * XTERM_SEQ: Set Replace mode: ESC [ ? 4 l
 */
/* EXTPROTO */
void
rxvt_scr_insert_mode(pR_ int mode)
{
    if (mode)
	R->screen.flags |= Screen_Insert;
    else
	R->screen.flags &= ~Screen_Insert;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/Unset tabs
 * XTERM_SEQ: Set tab at current column  : ESC H
 * XTERM_SEQ: Clear tab at current column: ESC [ 0 g
 * XTERM_SEQ: Clear all tabs             : ESC [ 3 g
 */
/* EXTPROTO */
void
rxvt_scr_set_tab(pR_ int mode)
{
    if (mode < 0)
	MEMSET(R->tabs, 0, R->TermWin.ncol * sizeof(char));
    else if (R->screen.cur.col < R->TermWin.ncol)
	R->tabs[R->screen.cur.col] = (mode ? 1 : 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set reverse/normal video
 * XTERM_SEQ: Reverse video: ESC [ ? 5 h
 * XTERM_SEQ: Normal video : ESC [ ? 5 l
 */
/* EXTPROTO */
void
rxvt_scr_rvideo_mode(pR_ int mode)
{
    XGCValues       gcvalue;

    if (R->rvideo != mode) {
	R->rvideo = mode;
	SWAP_IT(R->PixColors[Color_fg], R->PixColors[Color_bg], rxvt_color);
#if defined(XPM_BACKGROUND)
	if (R->bgPixmap.pixmap == None)
#endif
#if defined(TRANSPARENT)
	    if (!(R->Options & Opt_transparent) || R->am_transparent == 0)
#endif
	    XSetWindowBackground(R->Xdisplay, R->TermWin.vt,
				 R->PixColors[Color_bg]);

	gcvalue.foreground = R->PixColors[Color_fg];
	gcvalue.background = R->PixColors[Color_bg];
	XChangeGC(R->Xdisplay, R->TermWin.gc, GCBackground | GCForeground,
		  &gcvalue);
	rxvt_scr_clear(aR);
	rxvt_scr_touch(aR_ True);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Report current cursor position
 * XTERM_SEQ: Report position: ESC [ 6 n
 */
/* EXTPROTO */
void
rxvt_scr_report_position(pR)
{
    rxvt_tt_printf(aR_ "\033[%d;%dR", R->screen.cur.row + 1,
		   R->screen.cur.col + 1);
}

/* ------------------------------------------------------------------------- *
 *                                  FONTS                                    *
 * ------------------------------------------------------------------------- */

/*
 * Set font style
 */
/* INTPROTO */
void
rxvt_set_font_style(pR)
{
    switch (R->charsets[R->screen.charset]) {
    case '0':			/* DEC Special Character & Line Drawing Set */
	break;
    case 'A':			/* United Kingdom (UK) */
	break;
    case 'B':			/* United States (USASCII) */
	break;
    case '<':			/* Multinational character set */
	break;
    case '5':			/* Finnish character set */
	break;
    case 'C':			/* Finnish character set */
	break;
    case 'K':			/* German character set */
	break;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Choose a font
 * XTERM_SEQ: Invoke G0 character set: CTRL-O
 * XTERM_SEQ: Invoke G1 character set: CTRL-N
 * XTERM_SEQ: Invoke G2 character set: ESC N
 * XTERM_SEQ: Invoke G3 character set: ESC O
 */
/* EXTPROTO */
void
rxvt_scr_charset_choose(pR_ int set)
{
    R->screen.charset = set;
    rxvt_set_font_style(aR);
}

/* ------------------------------------------------------------------------- */
/*
 * Set a font
 * XTERM_SEQ: Set G0 character set: ESC ( <C>
 * XTERM_SEQ: Set G1 character set: ESC ) <C>
 * XTERM_SEQ: Set G2 character set: ESC * <C>
 * XTERM_SEQ: Set G3 character set: ESC + <C>
 * See set_font_style for possible values for <C>
 */
/* EXTPROTO */
void
rxvt_scr_charset_set(pR_ int set, unsigned int ch)
{
    R->charsets[set] = (unsigned char)ch;
    rxvt_set_font_style(aR);
}


/* ------------------------------------------------------------------------- *
 *                           GRAPHICS COLOURS                                *
 * ------------------------------------------------------------------------- */

#ifdef RXVT_GRAPHICS
/* EXTPROTO */
int
rxvt_scr_get_fgcolor(pR)
{
    return GET_FGCOLOR(R->rstyle);
}

/* ------------------------------------------------------------------------- */
/* EXTPROTO */
int
rxvt_scr_get_bgcolor(pR)
{
    return GET_BGCOLOR(R->rstyle);
}
#endif

/* ------------------------------------------------------------------------- *
 *                        MAJOR SCREEN MANIPULATION                          *
 * ------------------------------------------------------------------------- */

/*
 * Refresh an area
 */
enum {
    PART_BEG = 0,
    PART_END,
    RC_COUNT
};

/* EXTPROTO */
void
rxvt_scr_expose(pR_ int x, int y, int width, int height, Bool refresh)
{
    int             i;
    row_col_t       rc[RC_COUNT];

    if (R->drawn_text == NULL)	/* sanity check */
	return;

#ifdef DEBUG_STRICT
    x = max(x, (int)R->TermWin.int_bwidth);
    x = min(x, (int)R->TermWin.width);
    y = max(y, (int)R->TermWin.int_bwidth);
    y = min(y, (int)R->TermWin.height);
#endif

/* round down */
    rc[PART_BEG].col = Pixel2Col(x);
    rc[PART_BEG].row = Pixel2Row(y);
/* round up */
    rc[PART_END].col = Pixel2Width(x + width + R->TermWin.fwidth - 1);
    rc[PART_END].row = Pixel2Row(y + height + R->TermWin.fheight - 1);

/* sanity checks */
    for (i = PART_BEG; i < RC_COUNT; i++) {
	MIN_IT(rc[i].col, R->TermWin.ncol - 1);
	MIN_IT(rc[i].row, R->TermWin.nrow - 1);
    }

    D_SCREEN((stderr, "rxvt_scr_expose(x:%d, y:%d, w:%d, h:%d) area (c:%d,r:%d)-(c:%d,r:%d)", x, y, width, height, rc[PART_BEG].col, rc[PART_BEG].row, rc[PART_END].col, rc[PART_END].row));

    for (i = rc[PART_BEG].row; i <= rc[PART_END].row; i++)
	fill_text (&(R->drawn_text[i][rc[PART_BEG].col]), 0,
	           (rc[PART_END].col - rc[PART_BEG].col + 1));

    if (refresh)
	rxvt_scr_refresh(aR_ SLOW_REFRESH | REFRESH_BOUNDS);
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the entire screen
 */
/* EXTPROTO */
void
rxvt_scr_touch(pR_ Bool refresh)
{
    rxvt_scr_expose(aR_ 0, 0, R->TermWin.width, R->TermWin.height, refresh);
}

/* ------------------------------------------------------------------------- */
/*
 * Move the display so that the line represented by scrollbar value Y is at
 * the top of the screen
 */
/* EXTPROTO */
int
rxvt_scr_move_to(pR_ int y, int len)
{
    long            p = 0;
    uint16_t       oldviewstart;

    oldviewstart = R->TermWin.view_start;
    if (y < len) {
	p = (R->TermWin.nrow + R->TermWin.nscrolled) * (len - y) / len;
	p -= (long)(R->TermWin.nrow - 1);
	p = max(p, 0);
    }
    R->TermWin.view_start = (uint16_t)min(p, R->TermWin.nscrolled);
    D_SCREEN((stderr, "rxvt_scr_move_to(%d, %d) view_start:%d", y, len, R->TermWin.view_start));

    return rxvt_scr_changeview(aR_ oldviewstart);
}

/* ------------------------------------------------------------------------- */
/*
 * Page the screen up/down nlines
 * direction should be UP or DN
 */
/* EXTPROTO */
int
rxvt_scr_page(pR_ enum page_dirn direction, int nlines)
{
    int             n;
    uint16_t       oldviewstart;

    D_SCREEN((stderr, "rxvt_scr_page(%s, %d) view_start:%d", ((direction == UP) ? "UP" : "DN"), nlines, R->TermWin.view_start));
#ifdef DEBUG_STRICT
    assert((nlines >= 0) && (nlines <= R->TermWin.nrow));
#endif
    oldviewstart = R->TermWin.view_start;
    if (direction == UP) {
	n = R->TermWin.view_start + nlines;
	R->TermWin.view_start = min(n, R->TermWin.nscrolled);
    } else {
	n = R->TermWin.view_start - nlines;
	R->TermWin.view_start = max(n, 0);
    }
    return rxvt_scr_changeview(aR_ oldviewstart);
}

/* INTPROTO */
int
rxvt_scr_changeview(pR_ uint16_t oldviewstart)
{
    if (R->TermWin.view_start != oldviewstart) {
	R->want_refresh = 1;
#ifdef RXVT_GRAPHICS
	if (rxvt_Gr_Displayed(aR))
	    rxvt_Gr_scroll(aR_ 0);
#endif
	R->num_scr -= (R->TermWin.view_start - oldviewstart);
    }
    return (int)(R->TermWin.view_start - oldviewstart);
}

/* ------------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_scr_bell(pR)
{
#ifndef NO_BELL
# ifndef NO_MAPALERT
#  ifdef MAPALERT_OPTION
    if (R->Options & Opt_mapAlert)
#  endif
	XMapWindow(R->Xdisplay, R->TermWin.parent[0]);
# endif
    if (R->Options & Opt_visualBell) {
	rxvt_scr_rvideo_mode(aR_ !R->rvideo); /* refresh also done */
	rxvt_scr_rvideo_mode(aR_ !R->rvideo); /* refresh also done */
    } else
	XBell(R->Xdisplay, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_scr_printscreen(pR_ int fullhist)
{
#ifdef PRINTPIPE
    int             i, r1, nrows, row_offset;
    text_t         *t;
    FILE           *fd;

    if ((fd = rxvt_popen_printer(aR)) == NULL)
	return;
    nrows = R->TermWin.nrow;
    row_offset = R->TermWin.saveLines;
    if (!fullhist)
	row_offset -= R->TermWin.view_start;
    else {
	nrows += R->TermWin.nscrolled;
	row_offset -= R->TermWin.nscrolled;
    }

    for (r1 = 0; r1 < nrows; r1++) {
	t = R->screen.text[r1 + row_offset];
	for (i = R->TermWin.ncol - 1; i >= 0; i--)
	    if (!isspace(t[i]))
		break;
	fprintf(fd, "%.*s\n", (i + 1), t);
    }
    rxvt_pclose_printer(fd);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the screen
 * R->drawn_text/R->drawn_rend contain the screen information before the update.
 * R->screen.text/R->screen.rend contain what the screen will change to.
 */

#if defined (NO_BRIGHTCOLOR) || defined (VERYBOLD)
# define MONO_BOLD(x)		((x) & (RS_Bold|RS_Blink))
# define MONO_BOLD_FG(x, fg)	MONO_BOLD(x)
#else
# define MONO_BOLD(x)						\
    (((x) & (RS_Bold | RS_fgMask)) == (RS_Bold | Color_fg))
# define MONO_BOLD_FG(x, fg)	(((x) & RS_Bold) && (fg) == Color_fg)
#endif

#define FONT_WIDTH(X, Y)						\
    (X)->per_char[(Y) - (X)->min_char_or_byte2].width
#define FONT_RBEAR(X, Y)						\
    (X)->per_char[(Y) - (X)->min_char_or_byte2].rbearing
#define FONT_LBEAR(X, Y)						\
    (X)->per_char[(Y) - (X)->min_char_or_byte2].lbearing
#define IS_FONT_CHAR(X, Y)						\
    ((Y) >= (X)->min_char_or_byte2 && (Y) <= (X)->max_char_or_byte2)

/* EXTPROTO */
void
rxvt_scr_refresh(pR_ unsigned char refresh_type)
{
    unsigned char   clearfirst,	/* first character writes before cell        */
		    clearlast,	/* last character writes beyond cell         */
		    must_clear,	/* use draw_string not draw_image_string     */
#ifndef NO_BOLDFONT
		    bfont,	/* we've changed font to bold font           */
#endif
		    rvid,	/* reverse video this position               */
		    wbyte,	/* we're in multibyte                        */
		    showcursor;	/* show the cursor                           */
    int             fore, back;	/* desired foreground/background             */
    int16_t         col, row,	/* column/row we're processing               */
                    ocrow;	/* old cursor row                            */
    int             cursorwidth;
    int             i,		/* tmp                                       */
		    row_offset;	/* basic offset in screen structure          */
#ifndef NO_CURSORCOLOR
    rend_t	    cc1;	/* store colours at cursor position(s)       */
    rend_t          cc2;	/* store colours at cursor position(s)       */
#endif
    rend_t         *drp, *srp;	/* drawn-rend-pointer, screen-rend-pointer   */
    text_t         *dtp, *stp;	/* drawn-text-pointer, screen-text-pointer   */
    char           *buffer;	/* local copy of R->buffer                */

    if (refresh_type == NO_REFRESH || !R->TermWin.mapped)
	return;

/*
 * A: set up vars
 */
    clearfirst = clearlast = must_clear = wbyte = 0;
#ifndef NO_BOLDFONT
    bfont = 0;
#endif

    if (R->currmaxcol < R->TermWin.ncol) {
	R->currmaxcol = R->TermWin.ncol;
	R->buffer = (char *)rxvt_realloc (R->buffer,
                                          sizeof(char) * (R->currmaxcol + 1) * MB_CUR_MAX);
    }
    buffer = R->buffer;
    R->refresh_count = 0;

    row_offset = R->TermWin.saveLines - R->TermWin.view_start;

    if ((refresh_type & REFRESH_BOUNDS)) {
	clearfirst = clearlast = 1;
	R->refresh_type &= ~REFRESH_BOUNDS;
    }
#if defined(XPM_BACKGROUND)
    must_clear |= (R->bgPixmap.pixmap != None);
#endif
#if defined(TRANSPARENT)
    must_clear |= ((R->Options & Opt_transparent) && R->am_transparent);
#endif
    ocrow = R->oldcursor.row; /* is there an old outline cursor on screen? */

/*
 * B: reverse any characters which are selected
 */
    rxvt_scr_reverse_selection(aR);

/*
 * C: set the cursor character(s)
 */
    {
	unsigned char   setoldcursor;
	rend_t          ccol1,	/* Cursor colour       */
	                ccol2;	/* Cursor colour2      */

	showcursor = (R->screen.flags & Screen_VisibleCursor);
        cursorwidth = 0;
#ifdef CURSOR_BLINK
	if (R->hidden_cursor)
	    showcursor = 0;
#endif

        cursorwidth = 0;

        if (showcursor)
          {
            cursorwidth++;

            srp = &(R->screen.rend[R->screen.cur.row + R->TermWin.saveLines]
                                  [R->screen.cur.col]);

            if (showcursor && R->TermWin.focus)
              {
                *srp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
                cc1 = *srp & (RS_fgMask | RS_bgMask);
                if (XDEPTH > 2 && ISSET_PIXCOLOR(R, Color_cursor))
                    ccol1 = Color_cursor;
                else
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
                    ccol1 = GET_FGCOLOR(R->rstyle);
#else
                    ccol1 = Color_fg;
#endif
                if (XDEPTH > 2 && ISSET_PIXCOLOR(R, Color_cursor2))
                    ccol2 = Color_cursor2;
                else
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
                    ccol2 = GET_BGCOLOR(R->rstyle);
#else
                    ccol2 = Color_bg;
#endif
                *srp = SET_FGCOLOR(*srp, ccol1);
                *srp = SET_BGCOLOR(*srp, ccol2);
#endif
              }

            while (IS_WIDE (*srp))
              cursorwidth++, srp++;
          }

	/* make sure no outline cursor is left around */
	setoldcursor = 0;
	if (ocrow != -1) {
	    if (R->screen.cur.row + R->TermWin.view_start != ocrow
		|| R->screen.cur.col != R->oldcursor.col) {
		if (ocrow < R->TermWin.nrow
		    && R->oldcursor.col < R->TermWin.ncol) {
		    R->drawn_rend[ocrow][R->oldcursor.col] ^= (RS_RVid | RS_Uline);
		}
		if (R->TermWin.focus || !showcursor)
		    R->oldcursor.row = -1;
		else
		    setoldcursor = 1;
	    }
	} else if (!R->TermWin.focus)
	    setoldcursor = 1;
	if (setoldcursor) {
	    if (R->screen.cur.row + R->TermWin.view_start >= R->TermWin.nrow)
		R->oldcursor.row = -1;
	    else {
		R->oldcursor.row = R->screen.cur.row + R->TermWin.view_start;
		R->oldcursor.col = R->screen.cur.col;
	    }
	}
    }

#ifndef NO_SLOW_LINK_SUPPORT
/*
 * D: CopyArea pass - very useful for slower links
 *    This has been deliberately kept simple.
 */
    i = R->num_scr;
    if (refresh_type == FAST_REFRESH && R->num_scr_allow && i
	&& abs(i) < R->TermWin.nrow && !must_clear) {
	int16_t         nits;
	int             j;
	rend_t         *drp2;
	text_t         *dtp2;
        int             len, wlen;

	j = R->TermWin.nrow;
	wlen = len = -1;
	row = i > 0 ? 0 : j - 1;
	for (; j-- >= 0; row += (i > 0 ? 1 : -1)) {
	    if (row + i >= 0 && row + i < R->TermWin.nrow && row + i != ocrow) {
		stp = R->screen.text[row + row_offset];
		srp = R->screen.rend[row + row_offset];
		dtp = R->drawn_text[row];
		dtp2 = R->drawn_text[row + i];
		drp = R->drawn_rend[row];
		drp2 = R->drawn_rend[row + i];
		for (nits = 0, col = R->TermWin.ncol; col--; )
		    if (stp[col] != dtp2[col] || srp[col] != drp2[col])
			nits--;
		    else if (stp[col] != dtp[col] || srp[col] != drp[col])
			nits++;
		if (nits > 8) {	/* XXX: arbitrary choice */
		    for (col = R->TermWin.ncol; col--; ) {
			*dtp++ = *dtp2++;
			*drp++ = *drp2++;
		    }
		    if (len == -1)
			len = row;
		    wlen = row;
		    continue;
		}
	    }
	    if (len != -1) {
		/* also comes here at end if needed because of >= above */
		if (wlen < len)
		    SWAP_IT(wlen, len, int);
		D_SCREEN((stderr, "rxvt_scr_refresh(): XCopyArea: %d -> %d (height: %d)", len + i, len, wlen - len + 1));
		XCopyArea(R->Xdisplay, R->TermWin.vt, R->TermWin.vt,
			  R->TermWin.gc, 0, Row2Pixel(len + i),
			  (unsigned int)TermWin_TotalWidth(),
			  (unsigned int)Height2Pixel(wlen - len + 1),
			  0, Row2Pixel(len));
		len = -1;
	    }
	}
    }
#endif

/*
 * E: main pass across every character
 */
    for (row = 0; row < R->TermWin.nrow; row++)
      {
        stp = R->screen.text[row + row_offset];
        srp = R->screen.rend[row + row_offset];
        dtp = R->drawn_text[row];
        drp = R->drawn_rend[row];

/*
 * E2: OK, now the real pass
 */
	int ypixel = (int)Row2Pixel(row);

	for (col = 0; col < R->TermWin.ncol; col++) {
	    /* compare new text with old - if exactly the same then continue */
	    rend_t rend = srp[col];	/* screen rendition (target rendtion) */

	    if (stp[col] == dtp[col]	/* Must match characters to skip. */
		&& (rend == drp[col]	/* Either rendition the same or   */
		    || (stp[col] == ' '	/* space w/ no background change  */
			&& GET_BGATTR(rend) == GET_BGATTR(drp[col]))))
	      continue;

            text_t *text = stp + col;
            int count = 1;

	    /* redraw one or more characters */

            dtp[col] = stp[col];
	    drp[col] = rend;

            if (*text == NOCHAR) // never start redrawing at invisible characters. */
              continue;

	    int xpixel = Col2Pixel(col);

            // this loop looks very messy, it can probably be optimized
            // and cleaned a bit by you?
            for (i = 0; ++col < R->TermWin.ncol; )
              {
                if (stp[col] == NOCHAR)
                  {
                    dtp[col] = stp[col];
                    drp[col] = rend;
                    count++;

                    if (i) // only possible skip if char unchanged
                      i++;

                    continue;
                  }

                if (((rend ^ srp[col]) & ~RS_wide) != 0)
                  break;

                count++;

                if (stp[col] != dtp[col]
                    || srp[col] != drp[col])
                  {
                    if (must_clear && (i++ > (count / 2)))
                      break;

                    dtp[col] = stp[col];
                    drp[col] = rend;
                    i = 0;
                  }
                else if (must_clear || (stp[col] != ' ' && ++i >= 32))
                  break;
              }

            col--;	/* went one too far.  move back */
            count -= i;	/* dump any matching trailing chars */

/*
 * Determine the attributes for the string
 */
            int fid = GET_FONT(rend);
	    fore = GET_FGCOLOR(rend);
	    back = GET_BGCOLOR(rend);
	    rend = GET_ATTR(rend);

	    rvid = (rend & RS_RVid) ? 1 : 0;
#ifdef OPTION_HC
	    if (!rvid && (rend & RS_Blink))
              {
		if (XDEPTH > 2 && ISSET_PIXCOLOR(R, Color_HC))
		  back = Color_HC;
		else
		  rvid = !rvid;	/* fall back */
              }
#endif
	    if (rvid)
              {
		SWAP_IT(fore, back, int);

#ifndef NO_BOLD_UNDERLINE_REVERSE
		if (XDEPTH > 2 && ISSET_PIXCOLOR(R, Color_RV)
# ifndef NO_CURSORCOLOR
		    && !ISSET_PIXCOLOR(R, Color_cursor)
# endif
                    )
		  back = Color_RV;
#endif
	      }
#ifndef NO_BOLD_UNDERLINE_REVERSE
	    else if (rend & RS_Bold)
              {
		if (XDEPTH > 2 && ISSET_PIXCOLOR(R, Color_BD))
                  {
		    fore = Color_BD;
# ifndef VERYBOLD
		    rend &= ~RS_Bold;	/* we've taken care of it */
# endif
                  }
	      }
            else if (rend & RS_Uline)
              {
		if (XDEPTH > 2 && ISSET_PIXCOLOR(R, Color_UL))
                  {
		    fore = Color_UL;
		    rend &= ~RS_Uline;	/* we've taken care of it */
                  }
	      }
#endif

/*
 * Actually do the drawing of the string here
 */
            rxvt_font *font = (*R->TermWin.fontset)[fid];

            if (back == Color_bg)
              {
	        if (must_clear)
                  {
                    for (i = 0; i < count; i++)	/* don't draw empty strings */
                        if (text[i] != ' ') {
                            font->draw (xpixel, ypixel, text, count, fore, -1);
                            goto nodraw;
                        }
                    CLEAR_CHARS(xpixel, ypixel, count);
nodraw: ;
                  }
                else
                  font->draw (xpixel, ypixel, text, count, fore, Color_bg);
	    } else
                font->draw (xpixel, ypixel, text, count, fore, back);

	    if ((rend & RS_Uline) && (font->descent > 1))
		XDrawLine(R->Xdisplay, drawBuffer, R->TermWin.gc,
			  xpixel, ypixel + font->ascent + 1,
			  xpixel + Width2Pixel(count) - 1, ypixel + font->ascent + 1);
	}			/* for (col....) */
    }				/* for (row....) */

/*
 * G: cleanup cursor and display outline cursor if necessary
 */
    if (showcursor) {
	if (R->TermWin.focus) {
	    srp = &(R->screen.rend[R->screen.cur.row + R->TermWin.saveLines]
				  [R->screen.cur.col]);
	    *srp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
	    *srp = (*srp & ~(RS_fgMask | RS_bgMask)) | cc1;
#endif
	} else if (R->oldcursor.row >= 0) {
#ifndef NO_CURSORCOLOR
	    unsigned long   gcmask;	/* Graphics Context mask */

	    if (XDEPTH > 2 && ISSET_PIXCOLOR(R, Color_cursor)) {
                XSetForeground (R->Xdisplay, R->TermWin.gc, R->PixColors[Color_cursor]);
	    }
#endif
	    XDrawRectangle(R->Xdisplay, drawBuffer, R->TermWin.gc,
			   Col2Pixel(R->oldcursor.col),
			   Row2Pixel(R->oldcursor.row),
			   (unsigned int)(Width2Pixel(cursorwidth) - 1),
			   (unsigned int)(Height2Pixel(1) - R->TermWin.lineSpace - 1));
	}
    }
/*
 * H: cleanup selection
 */
    rxvt_scr_reverse_selection(aR);

/*
 * I: other general cleanup
 */
    if (clearfirst && R->TermWin.int_bwidth)
	/* 
	 * clear the whole screen height, note that width == 0 is treated
	 * specially by XClearArea
	 */
	XClearArea(R->Xdisplay, R->TermWin.vt, 0, 0,
		   (unsigned int)R->TermWin.int_bwidth,
		   (unsigned int)TermWin_TotalHeight(), False);
    if (clearlast && R->TermWin.int_bwidth)
	/* 
	 * clear the whole screen height, note that width == 0 is treated
	 * specially by XClearArea
	 */
	XClearArea(R->Xdisplay, R->TermWin.vt,
		   R->TermWin.width + R->TermWin.int_bwidth, 0,
		   (unsigned int)R->TermWin.int_bwidth,
		   (unsigned int)TermWin_TotalHeight(), False);
    if (refresh_type & SMOOTH_REFRESH)
	XSync(R->Xdisplay, False);

    R->num_scr = 0;
    R->num_scr_allow = 1;
    R->want_refresh = 0;	/* screen is current */
}
/* ------------------------------------------------------------------------- */

/* EXTPROTO */
void
rxvt_scr_clear(pR)
{
    if (!R->TermWin.mapped)
	return;
    R->num_scr_allow = 0;
    R->want_refresh = 1;
#ifdef TRANSPARENT
    if ((R->Options & Opt_transparent) && (R->am_pixmap_trans == 0)) {
	int             i;

	if (!(R->Options & Opt_transparent_all))
	    i = 0;
	else
	    i = (int)(sizeof(R->TermWin.parent) / sizeof(Window));
	for (; i--;)
	    if (R->TermWin.parent[i] != None)
		XClearWindow(R->Xdisplay, R->TermWin.parent[i]);
    }
#endif
    XClearWindow(R->Xdisplay, R->TermWin.vt);
}

/* ------------------------------------------------------------------------- */
/* INTPROTO */
void
rxvt_scr_reverse_selection(pR)
{
    int             i, col, row, end_row;
    rend_t         *srp;

    if (R->selection.op && R->current_screen == R->selection.screen) {
	end_row = R->TermWin.saveLines - R->TermWin.view_start;
	i = R->selection.beg.row + R->TermWin.saveLines;
	row = R->selection.end.row + R->TermWin.saveLines;
	if (i >= end_row)
	    col = R->selection.beg.col;
	else {
	    col = 0;
	    i = end_row;
	}
	end_row += R->TermWin.nrow;
	for (; i < row && i < end_row; i++, col = 0)
	    for (srp = R->screen.rend[i]; col < R->TermWin.ncol; col++)
#ifndef OPTION_HC
		srp[col] ^= RS_RVid;
#else
		srp[col] ^= RS_Blink;
#endif
	if (i == row && i < end_row)
	    for (srp = R->screen.rend[i]; col < R->selection.end.col; col++)
#ifndef OPTION_HC
		srp[col] ^= RS_RVid;
#else
		srp[col] ^= RS_Blink;
#endif
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Dump the whole scrollback and screen to the passed filedescriptor.  The
 * invoking routine must close the fd.
 */
#if 0
/* EXTPROTO */
void
rxvt_scr_dump(pR_ int fd)
{
    int             row, wrote;
    unsigned int    width, towrite;
    char            r1[] = "\n";

    for (row = R->TermWin.saveLines - R->TermWin.nscrolled;
	 row < R->TermWin.saveLines + R->TermWin.nrow - 1; row++) {
	width = R->screen.tlen[row] >= 0 ? R->screen.tlen[row]
					 : R->TermWin.ncol;
	for (towrite = width; towrite; towrite -= wrote) {
	    wrote = write(fd, &(R->screen.text[row][width - towrite]),
			  towrite);
	    if (wrote < 0)
		return;		/* XXX: death, no report */
	}
	if (R->screen.tlen[row] >= 0)
	    if (write(fd, r1, 1) <= 0)
		return;	/* XXX: death, no report */
    }
}
#endif

/* ------------------------------------------------------------------------- *
 *                           CHARACTER SELECTION                             *
 * ------------------------------------------------------------------------- */

/*
 * -R->TermWin.nscrolled <= (selection row) <= R->TermWin.nrow - 1
 */
/* EXTPROTO */
void
rxvt_selection_check(pR_ int check_more)
{
    row_col_t       pos;

    if (!R->selection.op)
	return;

    pos.row = pos.col = 0;
    if ((R->selection.beg.row < -(int32_t)R->TermWin.nscrolled)
	|| (R->selection.beg.row >= R->TermWin.nrow)
	|| (R->selection.mark.row < -(int32_t)R->TermWin.nscrolled)
	|| (R->selection.mark.row >= R->TermWin.nrow)
	|| (R->selection.end.row < -(int32_t)R->TermWin.nscrolled)
	|| (R->selection.end.row >= R->TermWin.nrow)
        || (check_more == 1
	    && R->current_screen == R->selection.screen
	    && !ROWCOL_IS_BEFORE(R->screen.cur, R->selection.beg)
	    && ROWCOL_IS_BEFORE(R->screen.cur, R->selection.end))
	|| (check_more == 2
	    && ROWCOL_IS_BEFORE(R->selection.beg, pos)
	    && ROWCOL_IS_AFTER(R->selection.end, pos))
	|| (check_more == 3
	    && ROWCOL_IS_AFTER(R->selection.end, pos))
	|| (check_more == 4	/* screen width change */
	    && (R->selection.beg.row != R->selection.end.row
		|| R->selection.end.col > R->TermWin.ncol)))
	CLEAR_SELECTION(R);
}

/* ------------------------------------------------------------------------- */
/*
 * Paste a selection direct to the command fd
 */
/* INTPROTO */
void
rxvt_PasteIt(pR_ const unsigned char *data, unsigned int nitems)
{
    unsigned int    i, j, n;
    unsigned char  *ds = (unsigned char *)rxvt_malloc(PROP_SIZE);
    
/* convert normal newline chars into common keyboard Return key sequence */
    for (i = 0; i < nitems; i += PROP_SIZE) {
	n = min(nitems - i, PROP_SIZE);
	MEMCPY(ds, data + i, n);
	for (j = 0; j < n; j++)
	    if (ds[j] == '\n')
		ds[j] = '\r';
	rxvt_tt_write(aR_ ds, (int)n);
    }
    free(ds);
}

/* ------------------------------------------------------------------------- */
/*
 * Respond to a notification that a primary selection has been sent
 * EXT: SelectionNotify
 */
/* EXTPROTO */
int
rxvt_selection_paste(pR_ Window win, Atom prop, Bool delete_prop)
{
    long            nread = 0;
    unsigned long   bytes_after;
    XTextProperty   ct;
#ifdef MULTICHAR_SET
    int             dummy_count;
    char          **cl;
#endif

    D_SELECT((stderr, "rxvt_selection_paste(%08lx, %lu, %d), wait=%2x", win, (unsigned long)prop, (int)delete_prop, R->selection_wait));

    if (prop == None) {		/* check for failed XConvertSelection */
#ifdef MULTICHAR_SET
	if ((R->selection_type & Sel_CompoundText)) {
	    int             selnum = R->selection_type & Sel_whereMask;

	    R->selection_type = 0;
	    if (selnum != Sel_direct)
		rxvt_selection_request_other(aR_ XA_STRING, selnum);
	}
#endif
	return 0;
    }
    for (;;) {
	if (XGetWindowProperty(R->Xdisplay, win, prop, (long)(nread / 4),
			       (long)(PROP_SIZE / 4), delete_prop,
			       AnyPropertyType, &ct.encoding, &ct.format,
			       &ct.nitems, &bytes_after,
			       &ct.value) != Success)
	    break;
	if (ct.encoding == 0) {
	    D_SELECT((stderr, "rxvt_selection_paste: property didn't exist!"));
	    break;
	}
	if (ct.value == NULL) {
	    D_SELECT((stderr, "rxvt_selection_paste: property shooting blanks!"));
	    continue;
	}
	if (ct.nitems == 0) {
	    D_SELECT((stderr, "rxvt_selection_paste: property empty - also INCR end"));
	    if (R->selection_wait == Sel_normal && nread == 0) {
	    /*
	     * pass through again trying CUT_BUFFER0 if we've come from
	     * XConvertSelection() but nothing was presented
	     */
		D_SELECT((stderr, "rxvt_selection_request: pasting CUT_BUFFER0"));
		rxvt_selection_paste(aR_ Xroot, XA_CUT_BUFFER0, False);
	    }
	    nread = -1;		/* discount any previous stuff */
	    break;
	}
	nread += ct.nitems;
#ifdef MULTICHAR_SET
	if (XmbTextPropertyToTextList(R->Xdisplay, &ct, &cl,
				      &dummy_count) == Success && cl) {
	    rxvt_PasteIt(aR_ cl[0], STRLEN(cl[0]));
	    XFreeStringList(cl);
	} else
#endif
	    rxvt_PasteIt(aR_ ct.value, (unsigned int)ct.nitems);
	if (bytes_after == 0)
	    break;
	XFree(ct.value);
    }
    if (ct.value)
	XFree(ct.value);
    if (R->selection_wait == Sel_normal)
	R->selection_wait = Sel_none;
    D_SELECT((stderr, "rxvt_selection_paste: bytes written: %ld", nread));
    return (int)nread;
}

/*
 * INCR support originally provided by Paul Sheer <psheer@obsidian.co.za>
 */
/* EXTPROTO */
void
rxvt_selection_property(pR_ Window win, Atom prop)
{
    int             reget_time = 0;

    if (prop == None)
	return;
    D_SELECT((stderr, "rxvt_selection_property(%08lx, %lu)", win, (unsigned long)prop));
    if (R->selection_wait == Sel_normal) {
	int             a, afmt;
	Atom            atype;
	unsigned long   bytes_after, nitems;
	unsigned char  *s = NULL;

	a = XGetWindowProperty(R->Xdisplay, win, prop, 0L, 1L, False,
			       R->xa[XA_INCR], &atype, &afmt, &nitems,
			       &bytes_after, &s);
	if (s)
	    XFree(s);
	if (a != Success)
	    return;
#ifndef __CYGWIN32__
	if (atype == R->xa[XA_INCR]) {	/* start an INCR transfer */
	    D_SELECT((stderr, "rxvt_selection_property: INCR: starting transfer"));
	    XDeleteProperty(R->Xdisplay, win, prop);
	    XFlush(R->Xdisplay);
	    reget_time = 1;
	    R->selection_wait = Sel_incr;
	}
#endif
    } else if (R->selection_wait == Sel_incr) {
	reget_time = 1;
	if (rxvt_selection_paste(aR_ win, prop, True) == -1) {
	    D_SELECT((stderr, "rxvt_selection_property: INCR: clean end"));
	    R->selection_wait = Sel_none;
	    R->timeout[TIMEOUT_INCR].tv_sec = 0;	/* turn off timer */
	}
    }
    if (reget_time) {	/* received more data so reget time */
	(void)gettimeofday(&(R->timeout[TIMEOUT_INCR]), NULL);
	R->timeout[TIMEOUT_INCR].tv_sec += 10;	/* ten seconds wait */
    }
}
/* ------------------------------------------------------------------------- */
/*
 * Request the current selection: 
 * Order: > internal selection if available
 *        > PRIMARY, SECONDARY, CLIPBOARD if ownership is claimed (+)
 *        > CUT_BUFFER0
 * (+) if ownership is claimed but property is empty, rxvt_selection_paste()
 *     will auto fallback to CUT_BUFFER0
 * EXT: button 2 release
 */
/* EXTPROTO */
void
rxvt_selection_request(pR_ Time tm, int x, int y)
{
    D_SELECT((stderr, "rxvt_selection_request(%lu, %d, %d)", tm, x, y));
    if (x < 0 || x >= R->TermWin.width || y < 0 || y >= R->TermWin.height)
	return;			/* outside window */

    if (R->selection.text != NULL) {	/* internal selection */
	D_SELECT((stderr, "rxvt_selection_request: pasting internal"));
	rxvt_PasteIt(aR_ R->selection.text, R->selection.len);
	return;
    } else {
	int             i;

	R->selection_request_time = tm;
	R->selection_wait = Sel_normal;
	for (i = Sel_Primary; i <= Sel_Clipboard; i++) {
#ifdef MULTICHAR_SET
	    R->selection_type = Sel_CompoundText;
#else
	    R->selection_type = 0;
#endif
	    if (rxvt_selection_request_other(aR_
#ifdef MULTICHAR_SET
					     R->xa[XA_COMPOUND_TEXT],
#else
					     XA_STRING,
#endif
					     i))
		return;
	}
    }
    R->selection_wait = Sel_none;	/* don't loop in rxvt_selection_paste() */
    D_SELECT((stderr, "rxvt_selection_request: pasting CUT_BUFFER0"));
    rxvt_selection_paste(aR_ Xroot, XA_CUT_BUFFER0, False);
}

/* INTPROTO */
int
rxvt_selection_request_other(pR_ Atom target, int selnum)
{
    Atom            sel;
#ifdef DEBUG_SELECT
    char           *debug_xa_names[] = { "PRIMARY", "SECONDARY", "CLIPBOARD" };
#endif

    R->selection_type |= selnum;
    if (selnum == Sel_Primary)
	sel = XA_PRIMARY;
    else if (selnum == Sel_Secondary)
	sel = XA_SECONDARY;
    else
	sel = R->xa[XA_CLIPBOARD];
    if (XGetSelectionOwner(R->Xdisplay, sel) != None) {
	D_SELECT((stderr, "rxvt_selection_request_other: pasting %s", debug_xa_names[selnum]));
	XConvertSelection(R->Xdisplay, sel, target, R->xa[XA_VT_SELECTION],
			  R->TermWin.vt, R->selection_request_time);
	return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
/*
 * Clear all selected text
 * EXT: SelectionClear
 */
/* EXTPROTO */
void
rxvt_selection_clear(pR)
{
    D_SELECT((stderr, "rxvt_selection_clear()"));

    R->want_refresh = 1;
    if (R->selection.text)
	free(R->selection.text);
    R->selection.text = NULL;
    R->selection.len = 0;
    CLEAR_SELECTION(R);
}

/* ------------------------------------------------------------------------- */
/*
 * Copy a selection into the cut buffer
 * EXT: button 1 or 3 release
 */
/* EXTPROTO */
void
rxvt_selection_make(pR_ Time tm)
{
    int             i, col, end_col, row, end_row;
    unsigned char  *new_selection_text;
    char           *str;
    text_t         *t;
#ifdef ACS_ASCII
    rend_t         *re;
#endif

    D_SELECT((stderr, "rxvt_selection_make(): R->selection.op=%d, R->selection.clicks=%d", R->selection.op, R->selection.clicks));
    switch (R->selection.op) {
    case SELECTION_CONT:
	break;
    case SELECTION_INIT:
	CLEAR_SELECTION(R);
    /* FALLTHROUGH */
    case SELECTION_BEGIN:
	R->selection.op = SELECTION_DONE;
    /* FALLTHROUGH */
    default:
	return;
    }
    R->selection.op = SELECTION_DONE;

    if (R->selection.clicks == 4)
	return;			/* nothing selected, go away */

    i = (R->selection.end.row - R->selection.beg.row + 1) * (R->TermWin.ncol + 1) + 1;
    str = (char *)rxvt_malloc(i * MB_CUR_MAX + 1);

    new_selection_text = (unsigned char *)str;

    col = R->selection.beg.col;
    MAX_IT(col, 0);
    row = R->selection.beg.row + R->TermWin.saveLines;
    end_row = R->selection.end.row + R->TermWin.saveLines;

    for (; row <= end_row; row++, col = 0)
      {
	t = &(R->screen.text[row][col]);

        end_col = R->screen.tlen[row];

        if (end_col == -1)
	  end_col = R->TermWin.ncol;

        if (row == end_row)
          MIN_IT (end_col, R->selection.end.col);

	for (; col < end_col; col++)
          if (*t == NOCHAR)
            t++;
          else
            {
              int len = wctomb (str, *t++);
              if (len > 0)
                str += len;
            }

	if (R->screen.tlen[row] != -1 && row != end_row)
	    *str++ = '\n';
      }

#ifndef NO_OLD_SELECTION
    if (R->selection_style == OLD_SELECT)
	if (end_col == R->TermWin.ncol)
	    *str++ = '\n';
#endif
#ifndef NO_NEW_SELECTION
    if (R->selection_style != OLD_SELECT)
	if (end_col != R->selection.end.col)
	    *str++ = '\n';
#endif
    *str = '\0';

    i = str - (char *)new_selection_text;
    if (i == 0)
      {
	free (new_selection_text);
	return;
      }

    // due to MB_MAX_CUR, selection wastage is usually high
    if (str - (char *)new_selection_text > 1024)
      new_selection_text = (unsigned char *)rxvt_realloc (new_selection_text, i + 1);

    R->selection.len = i;
    if (R->selection.text)
	free (R->selection.text);

    R->selection.text = new_selection_text;

    XSetSelectionOwner(R->Xdisplay, XA_PRIMARY, R->TermWin.vt, tm);
    if (XGetSelectionOwner(R->Xdisplay, XA_PRIMARY) != R->TermWin.vt)
	rxvt_print_error("can't get primary selection");
    XChangeProperty(R->Xdisplay, Xroot, XA_CUT_BUFFER0, XA_STRING, 8,
		    PropModeReplace, R->selection.text, (int)R->selection.len);
    R->selection_time = tm;
    D_SELECT((stderr, "rxvt_selection_make(): R->selection.len=%d", R->selection.len));
}

/* ------------------------------------------------------------------------- */
/*
 * Mark or select text based upon number of clicks: 1, 2, or 3
 * EXT: button 1 press
 */
/* EXTPROTO */
void
rxvt_selection_click(pR_ int clicks, int x, int y)
{
    D_SELECT((stderr, "rxvt_selection_click(%d, %d, %d)", clicks, x, y));

    clicks = ((clicks - 1) % 3) + 1;
    R->selection.clicks = clicks;	/* save clicks so extend will work */

    rxvt_selection_start_colrow(aR_ Pixel2Col(x), Pixel2Row(y));
    if (clicks == 2 || clicks == 3)
	rxvt_selection_extend_colrow(aR_ R->selection.mark.col,
				     R->selection.mark.row
				     + R->TermWin.view_start,
				     0,	/* button 3     */
				     1,	/* button press */
				     0);	/* click change */
}

/* ------------------------------------------------------------------------- */
/*
 * Mark a selection at the specified col/row
 */
/* INTPROTO */
void
rxvt_selection_start_colrow(pR_ int col, int row)
{
    R->want_refresh = 1;
    R->selection.mark.col = col;
    R->selection.mark.row = row - R->TermWin.view_start;
    MAX_IT(R->selection.mark.row, -(int32_t)R->TermWin.nscrolled);
    MIN_IT(R->selection.mark.row, (int32_t)R->TermWin.nrow - 1);
    MAX_IT(R->selection.mark.col, 0);
    MIN_IT(R->selection.mark.col, (int32_t)R->TermWin.ncol - 1);

    if (R->selection.op) {	/* clear the old selection */
	R->selection.beg.row = R->selection.end.row = R->selection.mark.row;
	R->selection.beg.col = R->selection.end.col = R->selection.mark.col;
    }
    R->selection.op = SELECTION_INIT;
    R->selection.screen = R->current_screen;
}

/* ------------------------------------------------------------------------- */
/*
 * Word select: select text for 2 clicks
 * We now only find out the boundary in one direction
 */

/* what do we want: spaces/tabs are delimiters or cutchars or non-cutchars */
#define DELIMIT_TEXT(x) \
    (((x) == ' ' || (x) == '\t') ? 2 : (STRCHR(R->rs[Rs_cutchars], (x)) != NULL))
#ifdef MULTICHAR_SET
# define DELIMIT_REND(x)	(((x) & RS_multiMask) ? 1 : 0)
#else
# define DELIMIT_REND(x)	1
#endif

/* INTPROTO */
void
rxvt_selection_delimit_word(pR_ enum page_dirn dirn, const row_col_t *mark, row_col_t *ret)
{
    int             col, row, dirnadd, tcol, trow, w1, w2;
    row_col_t       bound;
    text_t         *stp;
    rend_t         *srp;

    if (dirn == UP) {
	bound.row = R->TermWin.saveLines - R->TermWin.nscrolled - 1;
	bound.col = 0;
	dirnadd = -1;
    } else {
	bound.row = R->TermWin.saveLines + R->TermWin.nrow;
	bound.col = R->TermWin.ncol - 1;
	dirnadd = 1;
    }
    row = mark->row + R->TermWin.saveLines;
    col = mark->col;
    MAX_IT(col, 0);
/* find the edge of a word */
    stp = &(R->screen.text[row][col]);
    w1 = DELIMIT_TEXT(*stp);

    if (R->selection_style != NEW_SELECT) {
	if (w1 == 1) {
	    stp += dirnadd;
	    if (DELIMIT_TEXT(*stp) == 1)
		goto Old_Word_Selection_You_Die;
	    col += dirnadd;
	}
	w1 = 0;
    }
    srp = (&R->screen.rend[row][col]);
    w2 = DELIMIT_REND(*srp);

    for (;;) {
	for (; col != bound.col; col += dirnadd) {
	    stp += dirnadd;
	    if (DELIMIT_TEXT(*stp) != w1)
		break;
	    srp += dirnadd;
	    if (DELIMIT_REND(*srp) != w2)
		break;
	}
	if ((col == bound.col) && (row != bound.row)) {
	    if (R->screen.tlen[(row - (dirn == UP ? 1 : 0))] == -1) {
		trow = row + dirnadd;
		tcol = dirn == UP ? R->TermWin.ncol - 1 : 0;
		if (R->screen.text[trow] == NULL)
		    break;
		stp = &(R->screen.text[trow][tcol]);
		srp = &(R->screen.rend[trow][tcol]);
		if (DELIMIT_TEXT(*stp) != w1 || DELIMIT_REND(*srp) != w2)
		    break;
		row = trow;
		col = tcol;
		continue;
	    }
	}
	break;
    }
  Old_Word_Selection_You_Die:
    D_SELECT((stderr, "rxvt_selection_delimit_word(%s,...) @ (r:%3d, c:%3d) has boundary (r:%3d, c:%3d)", (dirn == UP ? "up	" : "down"), mark->row, mark->col, row - R->TermWin.saveLines, col));

    if (dirn == DN)
	col++;			/* put us on one past the end */

/* Poke the values back in */
    ret->row = row - R->TermWin.saveLines;
    ret->col = col;
}

/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified x/y pixel location
 * EXT: button 3 press; button 1 or 3 drag
 * flag == 0 ==> button 1
 * flag == 1 ==> button 3 press
 * flag == 2 ==> button 3 motion
 */
/* EXTPROTO */
void
rxvt_selection_extend(pR_ int x, int y, int flag)
{
    int             col, row;

    col = Pixel2Col(x);
    row = Pixel2Row(y);
    MAX_IT(row, 0);
    MIN_IT(row, (int)R->TermWin.nrow - 1);
    MAX_IT(col, 0);
    MIN_IT(col, (int)R->TermWin.ncol);
#ifndef NO_NEW_SELECTION
/*
 * If we're selecting characters (single click) then we must check first
 * if we are at the same place as the original mark.  If we are then
 * select nothing.  Otherwise, if we're to the right of the mark, you have to
 * be _past_ a character for it to be selected.
 */
    if (R->selection_style != OLD_SELECT) {
	if (((R->selection.clicks % 3) == 1) && !flag
	    && (col == R->selection.mark.col
		&& (row == R->selection.mark.row + R->TermWin.view_start))) {
	    /* select nothing */
	    R->selection.beg.row = R->selection.end.row = 0;
	    R->selection.beg.col = R->selection.end.col = 0;
	    R->selection.clicks = 4;
	    R->want_refresh = 1;
	    D_SELECT((stderr, "rxvt_selection_extend() R->selection.clicks = 4"));
	    return;
	}
    }
#endif
    if (R->selection.clicks == 4)
	R->selection.clicks = 1;
    rxvt_selection_extend_colrow(aR_ col, row, !!flag,	/* ? button 3      */
				 flag == 1 ? 1 : 0,	/* ? button press  */
				 0);	/* no click change */
}

/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified col/row
 */
/* INTPROTO */
void
rxvt_selection_extend_colrow(pR_ int32_t col, int32_t row, int button3, int buttonpress, int clickchange)
{
    int16_t         ncol = R->TermWin.ncol;
    int             end_col;
    row_col_t       pos;
    enum {
	LEFT, RIGHT
    } closeto = RIGHT;

    D_SELECT((stderr, "rxvt_selection_extend_colrow(c:%d, r:%d, %d, %d) clicks:%d, op:%d", col, row, button3, buttonpress, R->selection.clicks, R->selection.op));
    D_SELECT((stderr, "rxvt_selection_extend_colrow() ENT  b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", R->selection.beg.row, R->selection.beg.col, R->selection.mark.row, R->selection.mark.col, R->selection.end.row, R->selection.end.col));

    R->want_refresh = 1;
    switch (R->selection.op) {
    case SELECTION_INIT:
	CLEAR_SELECTION(R);
	R->selection.op = SELECTION_BEGIN;
    /* FALLTHROUGH */
    case SELECTION_BEGIN:
	if (row != R->selection.mark.row || col != R->selection.mark.col
	    || (!button3 && buttonpress))
	    R->selection.op = SELECTION_CONT;
	break;
    case SELECTION_DONE:
	R->selection.op = SELECTION_CONT;
    /* FALLTHROUGH */
    case SELECTION_CONT:
	break;
    case SELECTION_CLEAR:
	rxvt_selection_start_colrow(aR_ col, row);
    /* FALLTHROUGH */
    default:
	return;
    }
    if (R->selection.beg.col == R->selection.end.col
	&& R->selection.beg.col != R->selection.mark.col
	&& R->selection.beg.row == R->selection.end.row
	&& R->selection.beg.row != R->selection.mark.row) {
	R->selection.beg.col = R->selection.end.col = R->selection.mark.col;
	R->selection.beg.row = R->selection.end.row = R->selection.mark.row;
	D_SELECT((stderr, "rxvt_selection_extend_colrow() ENT2 b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", R->selection.beg.row, R->selection.beg.col, R->selection.mark.row, R->selection.mark.col, R->selection.end.row, R->selection.end.col));
    }

    pos.col = col;
    pos.row = row;

    pos.row -= R->TermWin.view_start;	/* adjust for scroll */

#ifndef NO_OLD_SELECTION
/*
 * This mimics some of the selection behaviour of version 2.20 and before.
 * There are no ``selection modes'', button3 is always character extension.
 * Note: button3 drag is always available, c.f. v2.20
 * Selection always terminates (left or right as appropriate) at the mark.
 */
    if (R->selection_style == OLD_SELECT) {
	if (R->selection.clicks == 1 || button3) {
	    if (R->hate_those_clicks) {
		R->hate_those_clicks = 0;
		if (R->selection.clicks == 1) {
		    R->selection.beg.row = R->selection.mark.row;
		    R->selection.beg.col = R->selection.mark.col;
		} else {
		    R->selection.mark.row = R->selection.beg.row;
		    R->selection.mark.col = R->selection.beg.col;
		}
	    }
	    if (ROWCOL_IS_BEFORE(pos, R->selection.mark)) {
		R->selection.end.row = R->selection.mark.row;
		R->selection.end.col = R->selection.mark.col + 1;
		R->selection.beg.row = pos.row;
		R->selection.beg.col = pos.col;
	    } else {
		R->selection.beg.row = R->selection.mark.row;
		R->selection.beg.col = R->selection.mark.col;
		R->selection.end.row = pos.row;
		R->selection.end.col = pos.col + 1;
	    }
	} else if (R->selection.clicks == 2) {
	    rxvt_selection_delimit_word(aR_ UP, &(R->selection.mark),
					&(R->selection.beg));
	    rxvt_selection_delimit_word(aR_ DN, &(R->selection.mark),
					&(R->selection.end));
	    R->hate_those_clicks = 1;
	} else if (R->selection.clicks == 3) {
	    R->selection.beg.row = R->selection.end.row = R->selection.mark.row;
	    R->selection.beg.col = 0;
	    R->selection.end.col = ncol;
	    R->hate_those_clicks = 1;
	}
	D_SELECT((stderr, "rxvt_selection_extend_colrow() EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", R->selection.beg.row, R->selection.beg.col, R->selection.mark.row, R->selection.mark.col, R->selection.end.row, R->selection.end.col));
	return;
    }
#endif				/* ! NO_OLD_SELECTION */
#ifndef NO_NEW_SELECTION
/* selection_style must not be OLD_SELECT to get here */
/*
 * This is mainly xterm style selection with a couple of differences, mainly
 * in the way button3 drag extension works.
 * We're either doing: button1 drag; button3 press; or button3 drag
 *  a) button1 drag : select around a midpoint/word/line - that point/word/line
 *     is always at the left/right edge of the R->selection.
 *  b) button3 press: extend/contract character/word/line at whichever edge of
 *     the selection we are closest to.
 *  c) button3 drag : extend/contract character/word/line - we select around
 *     a point/word/line which is either the start or end of the selection
 *     and it was decided by whichever point/word/line was `fixed' at the
 *     time of the most recent button3 press
 */
    if (button3 && buttonpress) {	/* button3 press */
	/*
	 * first determine which edge of the selection we are closest to
	 */
	if (ROWCOL_IS_BEFORE(pos, R->selection.beg)
	    || (!ROWCOL_IS_AFTER(pos, R->selection.end)
		&& (((pos.col - R->selection.beg.col)
		     + ((pos.row - R->selection.beg.row) * ncol))
		    < ((R->selection.end.col - pos.col)
		       + ((R->selection.end.row - pos.row) * ncol)))))
	     closeto = LEFT;
	if (closeto == LEFT) {
	    R->selection.beg.row = pos.row;
	    R->selection.beg.col = pos.col;
	    R->selection.mark.row = R->selection.end.row;
	    R->selection.mark.col = R->selection.end.col
				    - (R->selection.clicks == 2);
	} else {
	    R->selection.end.row = pos.row;
	    R->selection.end.col = pos.col;
	    R->selection.mark.row = R->selection.beg.row;
	    R->selection.mark.col = R->selection.beg.col;
	}
    } else {			/* button1 drag or button3 drag */
	if (ROWCOL_IS_AFTER(R->selection.mark, pos)) {
	    if ((R->selection.mark.row == R->selection.end.row)
		&& (R->selection.mark.col == R->selection.end.col)
		&& clickchange && R->selection.clicks == 2)
		R->selection.mark.col--;
	    R->selection.beg.row = pos.row;
	    R->selection.beg.col = pos.col;
	    R->selection.end.row = R->selection.mark.row;
	    R->selection.end.col = R->selection.mark.col
				   + (R->selection.clicks == 2);
	} else {
	    R->selection.beg.row = R->selection.mark.row;
	    R->selection.beg.col = R->selection.mark.col;
	    R->selection.end.row = pos.row;
	    R->selection.end.col = pos.col;
	}
    }

    if (R->selection.clicks == 1) {
	end_col = R->screen.tlen[R->selection.beg.row + R->TermWin.saveLines];
	if (end_col != -1 && R->selection.beg.col > end_col) {
#if 1
	    R->selection.beg.col = ncol;
#else
	    if (R->selection.beg.row != R->selection.end.row)
		R->selection.beg.col = ncol;
	    else
		R->selection.beg.col = R->selection.mark.col;
#endif
	}
	end_col = R->screen.tlen[R->selection.end.row + R->TermWin.saveLines];
	if (end_col != -1 && R->selection.end.col > end_col)
	    R->selection.end.col = ncol;

    } else if (R->selection.clicks == 2) {
	if (ROWCOL_IS_AFTER(R->selection.end, R->selection.beg))
	    R->selection.end.col--;
	rxvt_selection_delimit_word(aR_ UP, &(R->selection.beg),
				    &(R->selection.beg));
	rxvt_selection_delimit_word(aR_ DN, &(R->selection.end),
				    &(R->selection.end));
    } else if (R->selection.clicks == 3) {
#ifndef NO_FRILLS
	if ((R->Options & Opt_tripleclickwords)) {
	    int             end_row;

	    rxvt_selection_delimit_word(aR_ UP, &(R->selection.beg),
					&(R->selection.beg));
	    end_row = R->screen.tlen[R->selection.mark.row
				     + R->TermWin.saveLines];
	    for (end_row = R->selection.mark.row; end_row < R->TermWin.nrow;
		 end_row++) {
		end_col = R->screen.tlen[end_row + R->TermWin.saveLines];
		if (end_col != -1) {
		    R->selection.end.row = end_row;
		    R->selection.end.col = end_col;
		    rxvt_selection_remove_trailing_spaces(aR);
		    break;
		}
	    }
	} else
#endif
	{
	    if (ROWCOL_IS_AFTER(R->selection.mark, R->selection.beg))
		R->selection.mark.col++;
	    R->selection.beg.col = 0;
	    R->selection.end.col = ncol;
	}
    }
    if (button3 && buttonpress) {	/* mark may need to be changed */
	if (closeto == LEFT) {
	    R->selection.mark.row = R->selection.end.row;
	    R->selection.mark.col = R->selection.end.col
				    - (R->selection.clicks == 2);
	} else {
	    R->selection.mark.row = R->selection.beg.row;
	    R->selection.mark.col = R->selection.beg.col;
	}
    }
    D_SELECT((stderr, "rxvt_selection_extend_colrow() EXIT b:(r:%d,c:%d) m:(r:%d,c:%d), e:(r:%d,c:%d)", R->selection.beg.row, R->selection.beg.col, R->selection.mark.row, R->selection.mark.col, R->selection.end.row, R->selection.end.col));
#endif				/* ! NO_NEW_SELECTION */
}

#ifndef NO_FRILLS
/* INTPROTO */
void
rxvt_selection_remove_trailing_spaces(pR)
{
    int32_t         end_col, end_row;
    text_t         *stp; 

    end_col = R->selection.end.col;
    end_row = R->selection.end.row;
    for ( ; end_row >= R->selection.beg.row; ) {
	stp = R->screen.text[end_row + R->TermWin.saveLines];
	while (--end_col >= 0) {
	    if (stp[end_col] != ' ' && stp[end_col] != '\t')
		break;
	}
	if (end_col >= 0
	    || R->screen.tlen[end_row - 1 + R->TermWin.saveLines] != -1) {
	    R->selection.end.col = end_col + 1;
	    R->selection.end.row = end_row;
	    break;
	}
	end_row--;
	end_col = R->TermWin.ncol;
    }
    if (R->selection.mark.row > R->selection.end.row) {
	R->selection.mark.row = R->selection.end.row;
	R->selection.mark.col = R->selection.end.col;
    } else if (R->selection.mark.row == R->selection.end.row
	       && R->selection.mark.col > R->selection.end.col)
	R->selection.mark.col = R->selection.end.col;
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Double click on button 3 when already selected
 * EXT: button 3 double click
 */
/* EXTPROTO */
void
rxvt_selection_rotate(pR_ int x, int y)
{
    R->selection.clicks = R->selection.clicks % 3 + 1;
    rxvt_selection_extend_colrow(aR_ Pixel2Col(x), Pixel2Row(y), 1, 0, 1);
}

/* ------------------------------------------------------------------------- */
/*
 * On some systems, the Atom typedef is 64 bits wide.  We need to have a type
 * that is exactly 32 bits wide, because a format of 64 is not allowed by
 * the X11 protocol.
 */
typedef CARD32  Atom32;

/* ------------------------------------------------------------------------- */
/*
 * Respond to a request for our current selection
 * EXT: SelectionRequest
 */
/* EXTPROTO */
void
rxvt_selection_send(pR_ const XSelectionRequestEvent *rq)
{
    XSelectionEvent ev;
#ifdef USE_XIM
    Atom32          target_list[4];
#else
    Atom32          target_list[3];
#endif
    Atom            target;
    XTextProperty   ct;
    XICCEncodingStyle style;
    char           *cl[2], dummy[1];

    ev.type = SelectionNotify;
    ev.property = None;
    ev.display = rq->display;
    ev.requestor = rq->requestor;
    ev.selection = rq->selection;
    ev.target = rq->target;
    ev.time = rq->time;

    if (rq->target == R->xa[XA_TARGETS]) {
	target_list[0] = (Atom32) R->xa[XA_TARGETS];
	target_list[1] = (Atom32) XA_STRING;
	target_list[2] = (Atom32) R->xa[XA_TEXT];
#ifdef USE_XIM
	target_list[3] = (Atom32) R->xa[XA_COMPOUND_TEXT];
#endif
	XChangeProperty(R->Xdisplay, rq->requestor, rq->property, XA_ATOM,
			(8 * sizeof(target_list[0])), PropModeReplace,
			(unsigned char *)target_list,
			(sizeof(target_list) / sizeof(target_list[0])));
	ev.property = rq->property;
    } else if (rq->target == R->xa[XA_MULTIPLE]) {
	/* TODO: Handle MULTIPLE */
    } else if (rq->target == R->xa[XA_TIMESTAMP] && R->selection.text) {
	XChangeProperty(R->Xdisplay, rq->requestor, rq->property, XA_INTEGER,
			(8 * sizeof(Time)), PropModeReplace,
			(unsigned char *)&R->selection_time, 1);
	ev.property = rq->property;
    } else if (rq->target == XA_STRING
	       || rq->target == R->xa[XA_COMPOUND_TEXT]
	       || rq->target == R->xa[XA_TEXT]) {
#ifdef USE_XIM
	short           freect = 0;
#endif
	int             selectlen;

#ifdef USE_XIM
	if (rq->target != XA_STRING) {
	    target = R->xa[XA_COMPOUND_TEXT];
	    style = (rq->target == R->xa[XA_COMPOUND_TEXT])
		    ? XCompoundTextStyle : XStdICCTextStyle;
	} else
#endif
	{
	    target = XA_STRING;
	    style = XStringStyle;
	}
	if (R->selection.text) {
	    cl[0] = (char *)R->selection.text;
	    selectlen = R->selection.len;
	} else {
	    cl[0] = dummy;
	    *dummy = '\0';
	    selectlen = 0;
	}
#ifdef USE_XIM
	if (XmbTextListToTextProperty(R->Xdisplay, cl, 1, style, &ct)
	    == Success)		/* if we failed to convert then send it raw */
	    freect = 1;
	else
#endif
	{
	    ct.value = (unsigned char *)cl[0];
	    ct.nitems = selectlen;
	}
	XChangeProperty(R->Xdisplay, rq->requestor, rq->property,
			target, 8, PropModeReplace,
			ct.value, (int)ct.nitems);
	ev.property = rq->property;
#ifdef USE_XIM
	if (freect)
	    XFree(ct.value);
#endif
    }
    XSendEvent(R->Xdisplay, rq->requestor, False, 0L, (XEvent *)&ev);
}

/* ------------------------------------------------------------------------- *
 *                              MOUSE ROUTINES                               *
 * ------------------------------------------------------------------------- */

/*
 * return col/row values corresponding to x/y pixel values
 */
/* EXTPROTO */
void
rxvt_pixel_position(pR_ int *x, int *y)
{
    *x = Pixel2Col(*x);
/* MAX_IT(*x, 0); MIN_IT(*x, (int)R->TermWin.ncol - 1); */
    *y = Pixel2Row(*y);
/* MAX_IT(*y, 0); MIN_IT(*y, (int)R->TermWin.nrow - 1); */
}
/* ------------------------------------------------------------------------- */
#ifdef USE_XIM
/* EXTPROTO */
void
rxvt_setPosition(pR_ XPoint *pos)
{
    XWindowAttributes xwa;

    XGetWindowAttributes(R->Xdisplay, R->TermWin.vt, &xwa);
    pos->x = Col2Pixel(R->screen.cur.col) + xwa.x;
    pos->y = Height2Pixel((R->screen.cur.row + 1)) + xwa.y
	     - R->TermWin.lineSpace;
}
#endif
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- *
 *                              DEBUG ROUTINES                               *
 * ------------------------------------------------------------------------- */
#if 0
/* INTPROTO */
void
rxvt_debug_colors(void)
{
    int             color;
    const char     *name[] = {
	"fg", "bg",
	"black", "red", "green", "yellow", "blue", "magenta", "cyan", "white"
    };

    fprintf(stderr, "Color ( ");
    if (R->rstyle & RS_RVid)
	fprintf(stderr, "rvid ");
    if (R->rstyle & RS_Bold)
	fprintf(stderr, "bold ");
    if (R->rstyle & RS_Blink)
	fprintf(stderr, "blink ");
    if (R->rstyle & RS_Uline)
	fprintf(stderr, "uline ");
    fprintf(stderr, "): ");

    color = GET_FGCOLOR(R->rstyle);
#ifndef NO_BRIGHTCOLOR
    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
	color -= (minBrightCOLOR - minCOLOR);
	fprintf(stderr, "bright ");
    }
#endif
    fprintf(stderr, "%s on ", name[color]);

    color = GET_BGCOLOR(R->rstyle);
#ifndef NO_BRIGHTCOLOR
    if (color >= minBrightCOLOR && color <= maxBrightCOLOR) {
	color -= (minBrightCOLOR - minCOLOR);
	fprintf(stderr, "bright ");
    }
#endif
    fprintf(stderr, "%s\n", name[color]);
}
#endif
