/*--------------------------------*-C-*--------------------------------------*
 * File:        screen.C
 *---------------------------------------------------------------------------*
 *
 * Copyright (c) 1997-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2003-2004 Marc Lehmann <pcg@goof.com>
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
 * This file handles _all_ screen updates and selections
 */

#include "../config.h"          /* NECESSARY */
#include "rxvt.h"               /* NECESSARY */

#include <X11/Xmd.h>            /* get the typedef for CARD32 */

#include <inttypes.h>
#include <wchar.h>

#include "salloc.C" // HACK, should be a seperate compile!

inline void fill_text (text_t *start, text_t value, int len)
{
  while (len--)
    *start++ = value;
}

/* ------------------------------------------------------------------------- */
#define PROP_SIZE               16384
#define TABSIZE                 8       /* default tab size */

/* ------------------------------------------------------------------------- *
 *             GENERAL SCREEN AND SELECTION UPDATE ROUTINES                  *
 * ------------------------------------------------------------------------- */
#define ZERO_SCROLLBACK()                                              \
    if (options & Opt_scrollTtyOutput)                                 \
        TermWin.view_start = 0
#define CLEAR_SELECTION()                                              \
    selection.beg.row = selection.beg.col                              \
        = selection.end.row = selection.end.col = 0
#define CLEAR_ALL_SELECTION()                                          \
    selection.beg.row = selection.beg.col                              \
        = selection.mark.row = selection.mark.col                      \
        = selection.end.row = selection.end.col = 0

#define ROW_AND_COL_IS_AFTER(A, B, C, D)                               \
    (((A) > (C)) || (((A) == (C)) && ((B) > (D))))
#define ROW_AND_COL_IS_BEFORE(A, B, C, D)                              \
    (((A) < (C)) || (((A) == (C)) && ((B) < (D))))
#define ROW_AND_COL_IN_ROW_AFTER(A, B, C, D)                           \
    (((A) == (C)) && ((B) > (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_AFTER(A, B, C, D)                     \
    (((A) == (C)) && ((B) >= (D)))
#define ROW_AND_COL_IN_ROW_BEFORE(A, B, C, D)                          \
    (((A) == (C)) && ((B) < (D)))
#define ROW_AND_COL_IN_ROW_AT_OR_BEFORE(A, B, C, D)                    \
    (((A) == (C)) && ((B) <= (D)))

/* these must be row_col_t */
#define ROWCOL_IS_AFTER(X, Y)                                          \
    ROW_AND_COL_IS_AFTER ((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IS_BEFORE(X, Y)                                         \
    ROW_AND_COL_IS_BEFORE ((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AFTER(X, Y)                                      \
    ROW_AND_COL_IN_ROW_AFTER ((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_BEFORE(X, Y)                                     \
    ROW_AND_COL_IN_ROW_BEFORE ((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AT_OR_AFTER(X, Y)                                \
    ROW_AND_COL_IN_ROW_AT_OR_AFTER ((X).row, (X).col, (Y).row, (Y).col)
#define ROWCOL_IN_ROW_AT_OR_BEFORE(X, Y)                               \
    ROW_AND_COL_IN_ROW_AT_OR_BEFORE ((X).row, (X).col, (Y).row, (Y).col)

/*
 * CLEAR_ROWS : clear <num> rows starting from row <row>
 * CLEAR_CHARS: clear <num> chars starting from pixel position <x,y>
 * ERASE_ROWS : set <num> rows starting from row <row> to the foreground colour
 */
#define drawBuffer      TermWin.vt

#define CLEAR_ROWS(row, num)                                           \
    if (TermWin.mapped)                                                \
        XClearArea (display->display, drawBuffer, 0,                   \
                    Row2Pixel (row), (unsigned int)TermWin.width,      \
                    (unsigned int)Height2Pixel (num), False)

#define CLEAR_CHARS(x, y, num)                                         \
    if (TermWin.mapped)                                                \
        XClearArea (display->display, drawBuffer, x, y,                \
                    (unsigned int)Width2Pixel (num),                   \
                    (unsigned int)Height2Pixel (1), False)

#define ERASE_ROWS(row, num)                                           \
    XFillRectangle (display->display, drawBuffer, TermWin.gc,          \
                    0, Row2Pixel (row),                                \
                    (unsigned int)TermWin.width,                       \
                    (unsigned int)Height2Pixel (num))

/* ------------------------------------------------------------------------- *
 *                        SCREEN `COMMON' ROUTINES                           *
 * ------------------------------------------------------------------------- */

/* Fill part/all of a line with blanks. */
void
rxvt_term::scr_blank_line (text_t *et, rend_t *er, unsigned int width, rend_t efs)
{
  efs &= ~RS_baseattrMask;
  efs = SET_FONT (efs, FONTSET (efs)->find_font (' '));

  while (width--)
    {
      *et++ = ' ';
      *er++ = efs;
    }
}

/* ------------------------------------------------------------------------- */
/* Fill a full line with blanks - make sure it is allocated first */
void
rxvt_term::scr_blank_screen_mem (text_t **tp, rend_t **rp, unsigned int row, rend_t efs)
{
#ifdef DEBUG_STRICT
  assert ((tp[row] && rp[row]) || (tp[row] == NULL && rp[row] == NULL));
#endif
  if (tp[row] == NULL)
    {
      tp[row] = (text_t *)talloc->alloc ();
      rp[row] = (rend_t *)ralloc->alloc ();
    }

  scr_blank_line (tp[row], rp[row], TermWin.ncol, efs);
}

/* ------------------------------------------------------------------------- *
 *                          SCREEN INITIALISATION                            *
 * ------------------------------------------------------------------------- */
void
rxvt_term::scr_reset ()
{
  unsigned int ncol, nrow, total_rows, prev_total_rows;
  unsigned int p, q;
  int k;
  rend_t setrstyle;

#if ENABLE_OVERLAY
  scr_overlay_off ();
#endif

  TermWin.view_start = 0;
  num_scr = 0;

  if (TermWin.ncol == 0)
    TermWin.ncol = 80;

  if (TermWin.nrow == 0)
    TermWin.nrow = 24;

  ncol = TermWin.ncol;
  nrow = TermWin.nrow;

  if (ncol == prev_ncol && nrow == prev_nrow)
    return;

  // we need at least two lines for wrapping to work correctly
  if (nrow + TermWin.saveLines < 2)
    {
      TermWin.saveLines++;
      prev_nrow--;
      TermWin.nscrolled++;
    }

  want_refresh = 1;

  prev_total_rows = prev_nrow + TermWin.saveLines;
  total_rows = nrow + TermWin.saveLines;

  screen.tscroll = 0;
  screen.bscroll = nrow - 1;

  if (!talloc)
    {
      talloc = new rxvt_salloc (ncol * sizeof (text_t));
      ralloc = new rxvt_salloc (ncol * sizeof (rend_t));
    }

  if (!screen.text)
    {
      /*
       * first time called so just malloc everything: don't rely on realloc
       * Note: this is still needed so that all the scrollback lines are NULL
       */
      screen.text = (text_t **)rxvt_calloc (total_rows, sizeof (text_t *));
      buf_text = (text_t **)rxvt_calloc (total_rows, sizeof (text_t *));
      drawn_text = (text_t **)rxvt_calloc (nrow, sizeof (text_t *));
      swap.text = (text_t **)rxvt_calloc (nrow, sizeof (text_t *));

      screen.tlen = (int16_t *)rxvt_calloc (total_rows, sizeof (int16_t));
      swap.tlen = (int16_t *)rxvt_calloc (nrow, sizeof (int16_t));

      screen.rend = (rend_t **)rxvt_calloc (total_rows, sizeof (rend_t *));
      buf_rend = (rend_t **)rxvt_calloc (total_rows, sizeof (rend_t *));
      drawn_rend = (rend_t **)rxvt_calloc (nrow, sizeof (rend_t *));
      swap.rend = (rend_t **)rxvt_calloc (nrow, sizeof (rend_t *));

      for (p = 0; p < nrow; p++)
        {
          q = p + TermWin.saveLines;
          scr_blank_screen_mem (screen.text, screen.rend, q, DEFAULT_RSTYLE);
          scr_blank_screen_mem (swap.text, swap.rend, p, DEFAULT_RSTYLE);
          screen.tlen[q] = swap.tlen[p] = 0;
          scr_blank_screen_mem (drawn_text, drawn_rend, p, DEFAULT_RSTYLE);
        }

      memset (charsets, 'B', sizeof (charsets));
      TermWin.nscrolled = 0;       /* no saved lines */
      rstyle = DEFAULT_RSTYLE;
      screen.flags = Screen_DefaultFlags;
      screen.cur.row = screen.cur.col = 0;
      screen.charset = 0;
      current_screen = PRIMARY;
      scr_cursor (SAVE);

#if NSCREENS
      swap.flags = Screen_DefaultFlags;
      swap.cur.row = swap.cur.col = 0;
      swap.charset = 0;
      current_screen = SECONDARY;
      scr_cursor (SAVE);
      current_screen = PRIMARY;
#endif

      selection.text = NULL;
      selection.len = 0;
      selection.op = SELECTION_CLEAR;
      selection.screen = PRIMARY;
      selection.clicks = 0;
      CLEAR_ALL_SELECTION ();
      rvideo = 0;
    }
  else
    {
      /*
       * add or delete rows as appropriate
       */
      setrstyle = DEFAULT_RSTYLE;

      if (nrow < prev_nrow)
        {
          /* delete rows */
          k = min (TermWin.nscrolled, prev_nrow - nrow);
          scr_scroll_text (0, (int)prev_nrow - 1, k, 1);

          for (p = nrow; p < prev_nrow; p++)
            {
              q = p + TermWin.saveLines;

              if (screen.text[q])
                {
#ifdef DEBUG_STRICT
                  assert (screen.rend[q]);
#endif
                  talloc->free (screen.text[q]);
                  ralloc->free (screen.rend[q]);
                }

              if (swap.text[p])
                {
#ifdef DEBUG_STRICT
                  assert (swap.rend[p]);
#endif
                  talloc->free (swap.text[p]);
                  ralloc->free (swap.rend[p]);
                }

#ifdef DEBUG_STRICT
              assert (drawn_text[p] && drawn_rend[p]);
#endif
              talloc->free (drawn_text[p]);
              ralloc->free (drawn_rend[p]);
            }

          /* we have fewer rows so fix up cursor position */
          MIN_IT (screen.cur.row, (int32_t)nrow - 1);
          MIN_IT (swap.cur.row, (int32_t)nrow - 1);

          scr_reset_realloc (); /* realloc _last_ */
        }
      else if (nrow > prev_nrow)
        {
          /* add rows */
          scr_reset_realloc (); /* realloc _first_ */

          TermWin.ncol = prev_ncol; // save b/c scr_blank_screen_mem uses this

          k = min (TermWin.nscrolled, nrow - prev_nrow);

          for (p = prev_total_rows; p < total_rows; p++)
            {
              screen.tlen[p] = 0;
              screen.text[p] = NULL;
              screen.rend[p] = NULL;
            }

          for (p = prev_total_rows; p < total_rows - k; p++)
            scr_blank_screen_mem (screen.text, screen.rend, p, setrstyle);

          for (p = prev_nrow; p < nrow; p++)
            {
              swap.tlen[p] = 0;
              swap.text[p] = NULL;
              swap.rend[p] = NULL;
              drawn_text[p] = NULL;
              drawn_rend[p] = NULL;
              scr_blank_screen_mem (swap.text,  swap.rend,  p, setrstyle);
              scr_blank_screen_mem (drawn_text, drawn_rend, p, setrstyle);
            }

          if (k > 0)
            {
              scr_scroll_text (0, (int)nrow - 1, -k, 1);
              screen.cur.row += k;
              screen.s_cur.row += k;
              TermWin.nscrolled -= k;
            }
#ifdef DEBUG_STRICT
          assert (screen.cur.row < TermWin.nrow);
          assert (swap.cur.row < TermWin.nrow);
#else                           /* drive with your eyes closed */

          MIN_IT (screen.cur.row, nrow - 1);
          MIN_IT (swap.cur.row, nrow - 1);
#endif
          TermWin.ncol =  ncol; // save b/c scr_blank_screen_mem uses this
        }

      /* resize columns */
      if (ncol != prev_ncol)
        {
          rxvt_salloc *ta = new rxvt_salloc (ncol * sizeof (text_t));
          rxvt_salloc *ra = new rxvt_salloc (ncol * sizeof (rend_t));

          for (p = 0; p < total_rows; p++)
            {
              if (screen.text[p])
                {
                  screen.text[p] = (text_t *)ta->alloc (screen.text[p], prev_ncol * sizeof (text_t));
                  screen.rend[p] = (rend_t *)ra->alloc (screen.rend[p], prev_ncol * sizeof (rend_t));

                  MIN_IT (screen.tlen[p], (int16_t)ncol);

                  if (ncol > prev_ncol)
                    scr_blank_line (&screen.text[p][prev_ncol],
                                    &screen.rend[p][prev_ncol],
                                    ncol - prev_ncol, setrstyle);
                }
            }

          for (p = 0; p < nrow; p++)
            {
              drawn_text[p] = (text_t *)ta->alloc (drawn_text[p], prev_ncol * sizeof (text_t));
              drawn_rend[p] = (rend_t *)ra->alloc (drawn_rend[p], prev_ncol * sizeof (rend_t));

              if (ncol > prev_ncol)
                scr_blank_line (&drawn_text[p][prev_ncol],
                                &drawn_rend[p][prev_ncol],
                                ncol - prev_ncol, setrstyle);

              if (swap.text[p])
                {
                  swap.text[p] = (text_t *)ta->alloc (swap.text[p], prev_ncol * sizeof (text_t));
                  swap.rend[p] = (rend_t *)ra->alloc (swap.rend[p], prev_ncol * sizeof (rend_t));

                  MIN_IT (swap.tlen[p], (int16_t)ncol);

                  if (ncol > prev_ncol)
                    scr_blank_line (&swap.text[p][prev_ncol],
                                    &swap.rend[p][prev_ncol],
                                    ncol - prev_ncol, setrstyle);
                }

            }

          MIN_IT (screen.cur.col, (int16_t)ncol - 1);
          MIN_IT (swap.cur.col, (int16_t)ncol - 1);

          delete talloc; talloc = ta;
          delete ralloc; ralloc = ra;
        }

      if (tabs)
        free (tabs);
    }

  prev_nrow = nrow;
  prev_ncol = ncol;

  tabs = (char *)rxvt_malloc (ncol * sizeof (char));

  for (p = 0; p < ncol; p++)
    tabs[p] = (p % TABSIZE == 0) ? 1 : 0;

  tt_winch ();
}

void
rxvt_term::scr_reset_realloc ()
{
  unsigned int total_rows, nrow;

  nrow = TermWin.nrow;
  total_rows = nrow + TermWin.saveLines;
  /* *INDENT-OFF* */
  screen.text = (text_t **)rxvt_realloc (screen.text, total_rows * sizeof (text_t *));
  buf_text    = (text_t **)rxvt_realloc (buf_text   , total_rows * sizeof (text_t *));
  drawn_text  = (text_t **)rxvt_realloc (drawn_text , nrow       * sizeof (text_t *));
  swap.text   = (text_t **)rxvt_realloc (swap.text  , nrow       * sizeof (text_t *));

  screen.tlen = (int16_t *)rxvt_realloc (screen.tlen, total_rows * sizeof (int16_t));
  swap.tlen   = (int16_t *)rxvt_realloc (swap.tlen  , total_rows * sizeof (int16_t));

  screen.rend = (rend_t **)rxvt_realloc (screen.rend, total_rows * sizeof (rend_t *));
  buf_rend    = (rend_t **)rxvt_realloc (buf_rend   , total_rows * sizeof (rend_t *));
  drawn_rend  = (rend_t **)rxvt_realloc (drawn_rend , nrow       * sizeof (rend_t *));
  swap.rend   = (rend_t **)rxvt_realloc (swap.rend  , nrow       * sizeof (rend_t *));
  /* *INDENT-ON* */
}

/* ------------------------------------------------------------------------- */
/*
 * Free everything.  That way malloc debugging can find leakage.
 */
void
rxvt_term::scr_release ()
{
  unsigned int total_rows;
  int i;

  total_rows = TermWin.nrow + TermWin.saveLines;

  delete talloc; talloc = 0;
  delete ralloc; ralloc = 0;

  free (screen.text);
  free (screen.tlen);
  free (screen.rend);
  free (drawn_text);
  free (drawn_rend);
  free (swap.text);
  free (swap.tlen);
  free (swap.rend);
  free (buf_text);
  free (buf_rend);
  free (tabs);

  /* NULL these so if anything tries to use them, we'll know about it */
  screen.text = drawn_text = swap.text = NULL;
  screen.rend = drawn_rend = swap.rend = NULL;
  screen.tlen = swap.tlen = NULL;
  buf_text = NULL;
  buf_rend = NULL;
  tabs = NULL;
}

/* ------------------------------------------------------------------------- */
/*
 * Hard reset
 */
void
rxvt_term::scr_poweron ()
{
  scr_release ();
  prev_nrow = prev_ncol = 0;
  scr_reset ();

  scr_clear (true);
  scr_refresh (SLOW_REFRESH);
}

/* ------------------------------------------------------------------------- *
 *                         PROCESS SCREEN COMMANDS                           *
 * ------------------------------------------------------------------------- */
/*
 * Save and Restore cursor
 * XTERM_SEQ: Save cursor   : ESC 7
 * XTERM_SEQ: Restore cursor: ESC 8
 */
void
rxvt_term::scr_cursor (int mode)
{
  screen_t *s;

#if NSCREENS && !defined(NO_SECONDARY_SCREEN_CURSOR)
  if (current_screen == SECONDARY)
    s = &swap;
  else
#endif
    s = &screen;

  switch (mode)
    {
      case SAVE:
        s->s_cur.row = screen.cur.row;
        s->s_cur.col = screen.cur.col;
        s->s_rstyle = rstyle;
        s->s_charset = screen.charset;
        s->s_charset_char = charsets[screen.charset];
        break;

      case RESTORE:
        want_refresh = 1;
        screen.cur.row = s->s_cur.row;
        screen.cur.col = s->s_cur.col;
        screen.flags &= ~Screen_WrapNext;
        rstyle = s->s_rstyle;
        screen.charset = s->s_charset;
        charsets[screen.charset] = s->s_charset_char;
        set_font_style ();
        break;
    }

  /* boundary check in case screen size changed between SAVE and RESTORE */
  MIN_IT (s->cur.row, TermWin.nrow - 1);
  MIN_IT (s->cur.col, TermWin.ncol - 1);
#ifdef DEBUG_STRICT
  assert (s->cur.row >= 0);
  assert (s->cur.col >= 0);
#else                           /* drive with your eyes closed */
  MAX_IT (s->cur.row, 0);
  MAX_IT (s->cur.col, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Swap between primary and secondary screens
 * XTERM_SEQ: Primary screen  : ESC [ ? 4 7 h
 * XTERM_SEQ: Secondary screen: ESC [ ? 4 7 l
 */
int
rxvt_term::scr_change_screen (int scrn)
{
  int i;
#if NSCREENS
  int offset;
#endif

  want_refresh = 1;

  TermWin.view_start = 0;

  if (current_screen == scrn)
    return scrn;

  selection_check (2);        /* check for boundary cross */

  SWAP_IT (current_screen, scrn, int);

  SWAP_IT (screen.cur.row, swap.cur.row, int16_t);
  SWAP_IT (screen.cur.col, swap.cur.col, int16_t);
# ifdef DEBUG_STRICT
  assert (screen.cur.row >= 0 && screen.cur.row < prev_nrow);
  assert (screen.cur.col >= 0 && screen.cur.col < prev_ncol);
# else                          /* drive with your eyes closed */
  MAX_IT (screen.cur.row, 0);
  MIN_IT (screen.cur.row, (int32_t)prev_nrow - 1);
  MAX_IT (screen.cur.col, 0);
  MIN_IT (screen.cur.col, (int32_t)prev_ncol - 1);
# endif

#if NSCREENS
  if (options & Opt_secondaryScreen)
    {
      num_scr = 0;
      offset = TermWin.saveLines;

      for (i = prev_nrow; i--;)
        {
          SWAP_IT (screen.text[i + offset], swap.text[i], text_t *);
          SWAP_IT (screen.tlen[i + offset], swap.tlen[i], int16_t);
          SWAP_IT (screen.rend[i + offset], swap.rend[i], rend_t *);
        }

      SWAP_IT (screen.charset, swap.charset, int16_t);
      SWAP_IT (screen.flags, swap.flags, int);
      screen.flags |= Screen_VisibleCursor;
      swap.flags |= Screen_VisibleCursor;
    }
  else
#endif
    if (options & Opt_secondaryScroll)
      //if (current_screen == PRIMARY)
        scr_scroll_text (0, prev_nrow - 1, prev_nrow, 0);

  return scrn;
}

// clear WrapNext indicator, solidifying position on next line
void
rxvt_term::scr_do_wrap ()
{
  if (!(screen.flags & Screen_WrapNext))
    return;

  screen.flags &= ~Screen_WrapNext;

  screen.cur.col = 0;

  if (screen.cur.row == screen.bscroll)
    scr_scroll_text (screen.tscroll, screen.bscroll, 1, 0);
  else if (screen.cur.row < TermWin.nrow - 1)
    screen.cur.row++;
}

/* ------------------------------------------------------------------------- */
/*
 * Change the colour for following text
 */
void
rxvt_term::scr_color (unsigned int color, int fgbg)
{
  if (color > maxTermCOLOR)
    color = fgbg;

  if (fgbg == Color_fg)
    rstyle = SET_FGCOLOR (rstyle, color);
  else
    rstyle = SET_BGCOLOR (rstyle, color);
}

/* ------------------------------------------------------------------------- */
/*
 * Change the rendition style for following text
 */
void
rxvt_term::scr_rendition (int set, int style)
  {
    if (set)
      rstyle |= style;
    else if (style == ~RS_None)
      rstyle = DEFAULT_RSTYLE;
    else
      rstyle &= ~style;
  }

/* ------------------------------------------------------------------------- */
/*
 * Scroll text between <row1> and <row2> inclusive, by <count> lines
 * count positive ==> scroll up
 * count negative ==> scroll down
 * spec == 0 for normal routines
 */
int
rxvt_term::scr_scroll_text (int row1, int row2, int count, int spec)
{
  int i, j;
  long nscrolled;

  if (count == 0 || (row1 > row2))
    return 0;

  want_refresh = 1;

  if (row1 == 0 && count > 0
      && (current_screen == PRIMARY || options & Opt_secondaryScroll))
    {
      nscrolled = (long)TermWin.nscrolled + (long)count;

      if (nscrolled > (long)TermWin.saveLines)
        TermWin.nscrolled = TermWin.saveLines;
      else
        TermWin.nscrolled = (unsigned int)nscrolled;

      if ((options & Opt_scrollWithBuffer)
          && TermWin.view_start != 0
          && TermWin.view_start != TermWin.saveLines)
        scr_page (UP, count);
    }
  else if (!spec)
    row1 += TermWin.saveLines;

  row2 += TermWin.saveLines;

  if (selection.op && current_screen == selection.screen)
    {
      i = selection.beg.row + TermWin.saveLines;
      j = selection.end.row + TermWin.saveLines;
      if ((i < row1 && j > row1)
          || (i < row2 && j > row2)
          || (i - count < row1 && i >= row1)
          || (i - count > row2 && i <= row2)
          || (j - count < row1 && j >= row1)
          || (j - count > row2 && j <= row2))
        {
          CLEAR_ALL_SELECTION ();
          selection.op = SELECTION_CLEAR;  /* XXX: too aggressive? */
        }
      else if (j >= row1 && j <= row2)
        {
          /* move selected region too */
          selection.beg.row -= count;
          selection.end.row -= count;
          selection.mark.row -= count;
        }
    }

  selection_check (0);        /* _after_ TermWin.nscrolled update */

  num_scr += count;
  j = count;

  if (count < 0)
    count = -count;

  i = row2 - row1 + 1;
  MIN_IT (count, i);

  if (j > 0)
    {
      /* A: scroll up */

      /* A1: Copy lines that will get clobbered by the rotation */
      memcpy (buf_text, screen.text + row1, count * sizeof (text_t *));
      memcpy (buf_rend, screen.rend + row1, count * sizeof (rend_t *));

      /* A2: Rotate lines */
      i = row2 - row1 - count + 1;
      memmove (screen.tlen + row1, screen.tlen + row1 + count, i * sizeof (int16_t));
      memmove (screen.text + row1, screen.text + row1 + count, i * sizeof (text_t *));
      memmove (screen.rend + row1, screen.rend + row1 + count, i * sizeof (rend_t *));

      j = row2 - count + 1, i = count;
    }
  else /* if (j < 0) */
    {
      /* B: scroll down */

      /* B1: Copy lines that will get clobbered by the rotation */
      for (i = 0, j = row2; i < count; i++, j--)
        {
          buf_text[i] = screen.text[j];
          buf_rend[i] = screen.rend[j];
        }

      /* B2: Rotate lines */
      for (j = row2, i = j - count; i >= row1; i--, j--)
        {
          screen.tlen[j] = screen.tlen[i];
          screen.text[j] = screen.text[i];
          screen.rend[j] = screen.rend[i];
        }

      j = row1, i = count;
      count = -count;
    }

  /* C: Resurrect lines */
  memset (screen.tlen + j, 0, i * sizeof (int16_t));
  memcpy (screen.text + j, buf_text, i * sizeof (text_t *));
  memcpy (screen.rend + j, buf_rend, i * sizeof (text_t *));
  if (!spec) /* line length may not equal TermWin.ncol */
    for (; i--; j++)
      scr_blank_screen_mem (screen.text, screen.rend, (unsigned int)j, rstyle);

  return count;
}

/* ------------------------------------------------------------------------- */
/*
 * Add text given in <str> of length <len> to screen struct
 */
void
rxvt_term::scr_add_lines (const unicode_t *str, int nlines, int len)
{
  unsigned char checksel;
  unicode_t c;
  int i, row, last_col;
  text_t *stp;
  rend_t *srp;

  if (len <= 0)               /* sanity */
    return;

  want_refresh = 1;
  ZERO_SCROLLBACK ();
  last_col = TermWin.ncol;

  if (nlines > 0)
    {
      nlines += screen.cur.row - screen.bscroll;
      if ((nlines > 0)
          && (screen.tscroll == 0)
          && (screen.bscroll == (TermWin.nrow - 1)))
        {
          /* _at least_ this many lines need to be scrolled */
          scr_scroll_text (screen.tscroll, screen.bscroll, nlines, 0);
          screen.cur.row -= nlines;
        }
    }

#ifdef DEBUG_STRICT
  assert (screen.cur.col < last_col);
  assert ((screen.cur.row < TermWin.nrow)
          && (screen.cur.row >= - (int32_t)TermWin.nscrolled));
#else                           /* drive with your eyes closed */
  MIN_IT (screen.cur.col, last_col - 1);
  MIN_IT (screen.cur.row, (int32_t)TermWin.nrow - 1);
  MAX_IT (screen.cur.row, - (int32_t)TermWin.nscrolled);
#endif
  row = screen.cur.row + TermWin.saveLines;

  checksel = selection.op && current_screen == selection.screen ? 1 : 0;

  stp = screen.text[row];
  srp = screen.rend[row];

  while (len--)
    {
      c = *str++;

      if (c < 0x20)
        switch (c)
          {
            case C0_HT:
              scr_tab (1, true);
              continue;

            case C0_LF:
              if (screen.tlen[row] != -1)      /* XXX: think about this */
                MAX_IT (screen.tlen[row], screen.cur.col);

              screen.flags &= ~Screen_WrapNext;

              if (screen.cur.row == screen.bscroll)
                scr_scroll_text (screen.tscroll, screen.bscroll, 1, 0);
              else if (screen.cur.row < (TermWin.nrow - 1))
                row = (++screen.cur.row) + TermWin.saveLines;

              stp = screen.text[row];  /* _must_ refresh */
              srp = screen.rend[row];  /* _must_ refresh */
              continue;

            case C0_CR:
              if (screen.tlen[row] != -1)      /* XXX: think about this */
                MAX_IT (screen.tlen[row], screen.cur.col);

              screen.flags &= ~Screen_WrapNext;
              screen.cur.col = 0;
              continue;
          }

      if (checksel            /* see if we're writing within selection */
          && !ROWCOL_IS_BEFORE (screen.cur, selection.beg)
          && ROWCOL_IS_BEFORE (screen.cur, selection.end))
        {
          checksel = 0;
          /*
           * If we wrote anywhere in the selected area, kill the selection
           * XXX: should we kill the mark too?  Possibly, but maybe that
           *      should be a similar check.
           */
          CLEAR_SELECTION ();
        }

      if (screen.flags & Screen_WrapNext)
        {
          screen.tlen[row] = -1;

          scr_do_wrap (); row = screen.cur.row + TermWin.saveLines;

          stp = screen.text[row];  /* _must_ refresh */
          srp = screen.rend[row];  /* _must_ refresh */
        }

      if (screen.flags & Screen_Insert)
        scr_insdel_chars (1, INSERT);

      // rely on wcwidth to tell us the character width, at least for non-latin1
      // do wcwidth before further replacements, as wcwidth says that line-drawing
      // characters have width -1 (DOH!) on GNU/Linux sometimes.
      int width = c < 0x100 ? 1 : wcwidth (c);

      if (charsets[screen.charset] == '0') // DEC SPECIAL
        {
          // vt100 special graphics and line drawing
          // 5f-7e standard vt100
          // 40-5e rxvt extension for extra curses acs chars
          static uint16_t vt100_0[63] = { // 5f .. 7e
            0x0000, 0x2191, 0x2193, 0x2192, 0x2190, 0x2588, 0x259a, 0x2603, // 40-47 hi mr. snowman!
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 48-4f
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 50-57
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, // 58-5f
            0x25c6, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0, 0x00b1, // 60-67
            0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0x23ba, // 68-6f
            0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524, 0x2534, 0x252c, // 70-77
            0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3, 0x00b7,         // 78-7e
          };

          if (c >= 0x40 && c <= 0x7e && vt100_0[c - 0x40])
            {
              c = vt100_0[c - 0x40];
              width = 1;
            }
        }

      if (width != 0)
        {
          // some utf-8 decoders decode surrogate characters.
          if (0xd800 <= c && c <= 0xdfff)
            c = 0xfffd;

#if !UNICODE_3
          // trim characters we can't store directly :(
          if (c >= 0x10000)
# if ENABLE_COMBINING
            c = rxvt_composite.compose (c); // map to lower 16 bits
# else
            c = 0xfffd;
# endif
#endif

          // nuke the character at this position, if required
          if (stp[screen.cur.col] == NOCHAR
              || (screen.cur.col < TermWin.ncol - 1
                  && stp[screen.cur.col + 1] == NOCHAR))
            {
              int col = screen.cur.col;

              // find begin
              while (col > 0 && stp[col] == NOCHAR)
                col--;

              rend_t rend = SET_FONT (srp[col], FONTSET (srp[col])->find_font (' '));

              // found begin, nuke
              do {
                stp[col] = ' ';
                srp[col] = rend;
                col++;
              } while (col < TermWin.ncol && stp[col] == NOCHAR);
            }

          rend_t rend = SET_FONT (rstyle, FONTSET (rstyle)->find_font (c));

          do
            {
              stp[screen.cur.col] = c;
              srp[screen.cur.col] = rend;

              if (screen.cur.col < last_col - 1)
                screen.cur.col++;
              else
                {
                  screen.tlen[row] = last_col;
                  if (screen.flags & Screen_Autowrap)
                    screen.flags |= Screen_WrapNext;
                  break;
                }

              c = NOCHAR;
            }
          while (--width > 0);

          // pad with spaces when overwriting wide character with smaller one
          for (int c = screen.cur.col; c < last_col && stp[c] == NOCHAR; c++)
            {
              stp[c] = ' ';
              srp[c] = rend;
            }
        }
      else if (width == 0)
        {
#if ENABLE_COMBINING
          // handle combining characters
          // we just tag the accent on the previous on-screen character.
          // this is arguably not correct, but also arguably not wrong.
          // we don't handle double-width characters nicely yet.

          text_t *tp;
          rend_t *rp;

          if (screen.cur.col > 0)
            {
              tp = stp + screen.cur.col - 1;
              rp = srp + screen.cur.col - 1;

              while (*tp == NOCHAR && tp > stp)
                tp--, rp--;
            }
          else if (screen.cur.row > 0
                   && screen.tlen [screen.cur.row - 1 + TermWin.saveLines] == -1)
            {
              int line = screen.cur.row - 1 + TermWin.saveLines;

              tp = screen.text[line] + last_col - 1;
              rp = screen.rend[line] + last_col - 1;

              while (*tp == NOCHAR && tp > screen.text[line])
                tp--, rp--;
            }
          else
            continue;

          // first try to find a precomposed character
          unicode_t n = rxvt_compose (*tp, c);
          if (n == NOCHAR)
            n = rxvt_composite.compose (*tp, c);

          *tp = n;
          *rp = SET_FONT (*rp, FONTSET (*rp)->find_font (*tp));
#endif
        }
    }

  if (screen.tlen[row] != -1)      /* XXX: think about this */
    MAX_IT (screen.tlen[row], screen.cur.col);

#ifdef DEBUG_STRICT
  assert (screen.cur.row >= 0);
#else                           /* drive with your eyes closed */
  MAX_IT (screen.cur.row, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Process Backspace.  Move back the cursor back a position, wrap if have to
 * XTERM_SEQ: CTRL-H
 */
void
rxvt_term::scr_backspace ()
{
  want_refresh = 1;

  if (screen.cur.col == 0)
    {
      if (screen.cur.row > 0)
        {
#ifdef TERMCAP_HAS_BW
          screen.cur.col = TermWin.ncol - 1;
          screen.cur.row--;
          return;
#endif
        }
    }
  else if (!(screen.flags & Screen_WrapNext))
    scr_gotorc (0, -1, RELATIVE);

  screen.flags &= ~Screen_WrapNext;
}

/* ------------------------------------------------------------------------- */
/*
 * Process Horizontal Tab
 * count: +ve = forward; -ve = backwards
 * XTERM_SEQ: CTRL-I
 */
void
rxvt_term::scr_tab (int count, bool ht)
{
  int i, x;

  want_refresh = 1;
  i = x = screen.cur.col;

  if (count == 0)
    return;
  else if (count > 0)
    {
      int row = TermWin.saveLines + screen.cur.row;
      text_t *tp = screen.text[row];
      rend_t *rp = screen.rend[row];
      rend_t base_rend = rp[i];
      ht &= tp[i] == ' ';

      for (; ++i < TermWin.ncol; )
        if (tabs[i])
          {
            x = i;
            if (!--count)
              break;
          }
        else 
          ht &= tp[i] == ' '
                && RS_SAME (rp[i], base_rend);

      if (count)
        x = TermWin.ncol - 1;

      // store horizontal tab commands as characters inside the text
      // buffer so they can be selected and pasted.
      if (ht)
        {
          base_rend = SET_FONT (base_rend, 0);

          if (screen.tlen[row] != -1)      /* XXX: think about this */
            MAX_IT (screen.tlen[row], x);

          i = screen.cur.col;

          tp[i] = '\t';
          rp[i] = base_rend;

          while (++i < x)
            {
              tp[i] = NOCHAR;
              rp[i] = base_rend;
            }
        }
    }
  else /* if (count < 0) */
    {
      for (; --i >= 0; )
        if (tabs[i])
          {
            x = i;
            if (!++count)
              break;
          }

      if (count)
        x = 0;
    }

  if (x != screen.cur.col)
    scr_gotorc (0, x, R_RELATIVE);
}

/* ------------------------------------------------------------------------- */
/*
 * Process DEC Back Index
 * XTERM_SEQ: ESC 6
 * Move cursor left in row.  If we're at the left boundary, shift everything
 * in that row right.  Clear left column.
 */
#if ENABLE_FRILLS
void
rxvt_term::scr_backindex ()
{
  if (screen.cur.col > 0)
    scr_gotorc (0, -1, R_RELATIVE | C_RELATIVE);
  else
    {
      if (screen.tlen[screen.cur.row + TermWin.saveLines] == 0)
        return;             /* um, yeah? */

      scr_insdel_chars (1, INSERT);
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
#if ENABLE_FRILLS
void
rxvt_term::scr_forwardindex ()
{
  int             row;

  if (screen.cur.col < TermWin.ncol - 1)
    scr_gotorc (0, 1, R_RELATIVE | C_RELATIVE);
  else
    {
      row = screen.cur.row + TermWin.saveLines;

      if (screen.tlen[row] == 0)
        return;             /* um, yeah? */
      else if (screen.tlen[row] == -1)
        screen.tlen[row] = TermWin.ncol;

      scr_gotorc (0, 0, R_RELATIVE);
      scr_insdel_chars (1, DELETE);
      scr_gotorc (0, TermWin.ncol - 1, R_RELATIVE);
    }
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Goto Row/Column
 */
void
rxvt_term::scr_gotorc (int row, int col, int relative)
{
  want_refresh = 1;
  ZERO_SCROLLBACK ();

  screen.cur.col = relative & C_RELATIVE ? screen.cur.col + col : col;
  MAX_IT (screen.cur.col, 0);
  MIN_IT (screen.cur.col, (int32_t)TermWin.ncol - 1);

  screen.flags &= ~Screen_WrapNext;

  if (relative & R_RELATIVE)
    {
      if (row > 0)
        {
          if (screen.cur.row <= screen.bscroll
              && (screen.cur.row + row) > screen.bscroll)
            screen.cur.row = screen.bscroll;
          else
            screen.cur.row += row;
        }
      else if (row < 0)
        {
          if (screen.cur.row >= screen.tscroll
              && (screen.cur.row + row) < screen.tscroll)
            screen.cur.row = screen.tscroll;
          else
            screen.cur.row += row;
        }
    }
  else
    {
      if (screen.flags & Screen_Relative)
        {        /* relative origin mode */
          screen.cur.row = row + screen.tscroll;
          MIN_IT (screen.cur.row, screen.bscroll);
        }
      else
        screen.cur.row = row;
    }

  MAX_IT (screen.cur.row, 0);
  MIN_IT (screen.cur.row, (int32_t)TermWin.nrow - 1);
}

/* ------------------------------------------------------------------------- */
/*
 * direction should be UP or DN
 */
void
rxvt_term::scr_index (enum page_dirn direction)
{
  int dirn;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  dirn = ((direction == UP) ? 1 : -1);

  screen.flags &= ~Screen_WrapNext;

  if ((screen.cur.row == screen.bscroll && direction == UP)
      || (screen.cur.row == screen.tscroll && direction == DN))
    scr_scroll_text (screen.tscroll, screen.bscroll, dirn, 0);
  else
    screen.cur.row += dirn;

  MAX_IT (screen.cur.row, 0);
  MIN_IT (screen.cur.row, (int32_t)TermWin.nrow - 1);
  selection_check (0);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part or whole of a line
 * XTERM_SEQ: Clear line to right: ESC [ 0 K
 * XTERM_SEQ: Clear line to left : ESC [ 1 K
 * XTERM_SEQ: Clear whole line   : ESC [ 2 K
 */
void
rxvt_term::scr_erase_line (int mode)
{
  unsigned int row, col, num;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  selection_check (1);

  row = TermWin.saveLines + screen.cur.row;
  switch (mode)
    {
      case 0:                     /* erase to end of line */
        col = screen.cur.col;
        num = TermWin.ncol - col;
        MIN_IT (screen.tlen[row], (int16_t)col);
        if (ROWCOL_IN_ROW_AT_OR_AFTER (selection.beg, screen.cur)
            || ROWCOL_IN_ROW_AT_OR_AFTER (selection.end, screen.cur))
          CLEAR_SELECTION ();
        break;
      case 1:                     /* erase to beginning of line */
        col = 0;
        num = screen.cur.col + 1;
        if (ROWCOL_IN_ROW_AT_OR_BEFORE (selection.beg, screen.cur)
            || ROWCOL_IN_ROW_AT_OR_BEFORE (selection.end, screen.cur))
          CLEAR_SELECTION ();
        break;
      case 2:                     /* erase whole line */
        col = 0;
        num = TermWin.ncol;
        screen.tlen[row] = 0;
        if (selection.beg.row <= screen.cur.row
            && selection.end.row >= screen.cur.row)
          CLEAR_SELECTION ();
        break;
      default:
        return;
    }

  if (screen.text[row])
    scr_blank_line (&screen.text[row][col], &screen.rend[row][col], num, rstyle);
  else
    scr_blank_screen_mem (screen.text, screen.rend, row, rstyle);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part of whole of the screen
 * XTERM_SEQ: Clear screen after cursor : ESC [ 0 J
 * XTERM_SEQ: Clear screen before cursor: ESC [ 1 J
 * XTERM_SEQ: Clear whole screen        : ESC [ 2 J
 */
void
rxvt_term::scr_erase_screen (int mode)
{
  int num;
  int32_t row, row_offset;
  rend_t ren;
  XGCValues gcvalue;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  row_offset = (int32_t)TermWin.saveLines;

  switch (mode)
    {
      case 0:                     /* erase to end of screen */
        selection_check (1);
        scr_erase_line (0);
        row = screen.cur.row + 1;    /* possible OOB */
        num = TermWin.nrow - row;
        break;
      case 1:                     /* erase to beginning of screen */
        selection_check (3);
        scr_erase_line (1);
        row = 0;
        num = screen.cur.row;
        break;
      case 2:                     /* erase whole screen */
        selection_check (3);
        row = 0;
        num = TermWin.nrow;
        break;
      default:
        return;
    }

  if (selection.op && current_screen == selection.screen
      && ((selection.beg.row >= row && selection.beg.row <= row + num)
          || (selection.end.row >= row
              && selection.end.row <= row + num)))
    CLEAR_SELECTION ();

  if (row >= TermWin.nrow) /* Out Of Bounds */
    return;

  MIN_IT (num, (TermWin.nrow - row));

  if (rstyle & (RS_RVid | RS_Uline))
    ren = (rend_t) ~RS_None;
  else if (GET_BASEBG (rstyle) == Color_bg)
    {
      ren = DEFAULT_RSTYLE;
      CLEAR_ROWS (row, num);
    }
  else
    {
      ren = rstyle & (RS_fgMask | RS_bgMask);
      gcvalue.foreground = pix_colors[GET_BGCOLOR (rstyle)];
      XChangeGC (display->display, TermWin.gc, GCForeground, &gcvalue);
      ERASE_ROWS (row, num);
      gcvalue.foreground = pix_colors[Color_fg];
      XChangeGC (display->display, TermWin.gc, GCForeground, &gcvalue);
    }

  for (; num--; row++)
    {
      scr_blank_screen_mem (screen.text, screen.rend, (unsigned int) (row + row_offset), rstyle);
      screen.tlen[row + row_offset] = 0;
      scr_blank_line (drawn_text[row], drawn_rend[row], (unsigned int)TermWin.ncol, ren);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Fill the screen with `E's
 * XTERM_SEQ: Screen Alignment Test: ESC # 8
 */
void
rxvt_term::scr_E ()
{
  int             i, j, k;
  rend_t         *r1, fs;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  num_scr_allow = 0;
  selection_check (3);

  fs = SET_FONT (rstyle, FONTSET (rstyle)->find_font ('E'));
  for (k = TermWin.saveLines, i = TermWin.nrow; i--; k++)
    {
      screen.tlen[k] = TermWin.ncol;    /* make the `E's selectable */
      fill_text (screen.text[k], 'E', TermWin.ncol);
      for (r1 = screen.rend[k], j = TermWin.ncol; j--; )
        *r1++ = fs;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> lines
 */
void
rxvt_term::scr_insdel_lines (int count, int insdel)
{
  int end;

  ZERO_SCROLLBACK ();

  selection_check (1);

  if (screen.cur.row > screen.bscroll)
    return;

  end = screen.bscroll - screen.cur.row + 1;
  if (count > end)
    {
      if (insdel == DELETE)
        return;
      else if (insdel == INSERT)
        count = end;
    }

  scr_do_wrap ();

  scr_scroll_text (screen.cur.row, screen.bscroll, insdel * count, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> characters from the current position
 */
void
rxvt_term::scr_insdel_chars (int count, int insdel)
{
  int col, row;
  rend_t tr;
  text_t *stp;
  rend_t *srp;
  int16_t *slp;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  if (count <= 0)
    return;

  scr_do_wrap ();

  selection_check (1);
  MIN_IT (count, (TermWin.ncol - screen.cur.col));

  row = screen.cur.row + TermWin.saveLines;

  stp = screen.text[row];
  srp = screen.rend[row];
  slp = &screen.tlen[row];

  switch (insdel)
    {
      case INSERT:
        for (col = TermWin.ncol - 1; (col - count) >= screen.cur.col; col--)
          {
            stp[col] = stp[col - count];
            srp[col] = srp[col - count];
          }

        if (*slp != -1)
          {
            *slp += count;
            MIN_IT (*slp, TermWin.ncol);
          }

        if (selection.op && current_screen == selection.screen
            && ROWCOL_IN_ROW_AT_OR_AFTER (selection.beg, screen.cur))
          {
            if (selection.end.row != screen.cur.row
                || (selection.end.col + count >= TermWin.ncol))
              CLEAR_SELECTION ();
            else
              {              /* shift selection */
                selection.beg.col  += count;
                selection.mark.col += count; /* XXX: yes? */
                selection.end.col  += count;
              }
          }

        scr_blank_line (&stp[screen.cur.col], &srp[screen.cur.col],
                        (unsigned int)count, rstyle);
        break;

      case ERASE:
        screen.cur.col += count;     /* don't worry if > TermWin.ncol */
        selection_check (1);
        screen.cur.col -= count;
        scr_blank_line (&stp[screen.cur.col], &srp[screen.cur.col],
                        (unsigned int)count, rstyle);
        break;

      case DELETE:
        tr = srp[TermWin.ncol - 1] & (RS_fgMask | RS_bgMask | RS_baseattrMask);

        for (col = screen.cur.col; (col + count) < TermWin.ncol; col++)
          {
            stp[col] = stp[col + count];
            srp[col] = srp[col + count];
          }

        scr_blank_line (&stp[TermWin.ncol - count], &srp[TermWin.ncol - count],
                        (unsigned int)count, tr);

        if (*slp == -1) /* break line continuation */
          *slp = TermWin.ncol;
        
        *slp -= count;
        MAX_IT (*slp, 0);

        if (selection.op && current_screen == selection.screen
            && ROWCOL_IN_ROW_AT_OR_AFTER (selection.beg, screen.cur))
          {
            if (selection.end.row != screen.cur.row
                || (screen.cur.col >= selection.beg.col - count)
                || selection.end.col >= TermWin.ncol)
              CLEAR_SELECTION ();
            else
              {
                /* shift selection */
                selection.beg.col  -= count;
                selection.mark.col -= count; /* XXX: yes? */
                selection.end.col  -= count;
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
void
rxvt_term::scr_scroll_region (int top, int bot)
{
  MAX_IT (top, 0);
  MIN_IT (bot, (int)TermWin.nrow - 1);

  if (top > bot)
    return;

  screen.tscroll = top;
  screen.bscroll = bot;
  scr_gotorc (0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Make the cursor visible/invisible
 * XTERM_SEQ: Make cursor visible  : ESC [ ? 25 h
 * XTERM_SEQ: Make cursor invisible: ESC [ ? 25 l
 */
void
rxvt_term::scr_cursor_visible (int mode)
{
  want_refresh = 1;

  if (mode)
    screen.flags |= Screen_VisibleCursor;
  else
    screen.flags &= ~Screen_VisibleCursor;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/unset automatic wrapping
 * XTERM_SEQ: Set Wraparound  : ESC [ ? 7 h
 * XTERM_SEQ: Unset Wraparound: ESC [ ? 7 l
 */
void
rxvt_term::scr_autowrap (int mode)
{
  if (mode)
    screen.flags |= Screen_Autowrap;
  else
    screen.flags &= ~(Screen_Autowrap | Screen_WrapNext);
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
void
rxvt_term::scr_relative_origin (int mode)
{
  if (mode)
    screen.flags |= Screen_Relative;
  else
    screen.flags &= ~Screen_Relative;

  scr_gotorc (0, 0, 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set insert/replace mode
 * XTERM_SEQ: Set Insert mode : ESC [ ? 4 h
 * XTERM_SEQ: Set Replace mode: ESC [ ? 4 l
 */
void
rxvt_term::scr_insert_mode (int mode)
{
  if (mode)
    screen.flags |= Screen_Insert;
  else
    screen.flags &= ~Screen_Insert;
}

/* ------------------------------------------------------------------------- */
/*
 * Set/Unset tabs
 * XTERM_SEQ: Set tab at current column  : ESC H
 * XTERM_SEQ: Clear tab at current column: ESC [ 0 g
 * XTERM_SEQ: Clear all tabs             : ESC [ 3 g
 */
void
rxvt_term::scr_set_tab (int mode)
{
  if (mode < 0)
    memset (tabs, 0, TermWin.ncol * sizeof (char));
  else if (screen.cur.col < TermWin.ncol)
    tabs[screen.cur.col] = (mode ? 1 : 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Set reverse/normal video
 * XTERM_SEQ: Reverse video: ESC [ ? 5 h
 * XTERM_SEQ: Normal video : ESC [ ? 5 l
 */
void
rxvt_term::scr_rvideo_mode (int mode)
{
  XGCValues gcvalue;

  if (rvideo != mode)
    {
      rvideo = mode;
      SWAP_IT (pix_colors[Color_fg], pix_colors[Color_bg], rxvt_color);
#if XPM_BACKGROUND
      if (bgPixmap.pixmap == None)
#endif
#if TRANSPARENT
        if (! (options & Opt_transparent) || am_transparent == 0)
#endif
          XSetWindowBackground (display->display, TermWin.vt,
                               pix_colors[Color_bg]);

      gcvalue.foreground = pix_colors[Color_fg];
      gcvalue.background = pix_colors[Color_bg];
      XChangeGC (display->display, TermWin.gc, GCBackground | GCForeground,
                &gcvalue);
      scr_clear ();
      scr_touch (true);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Report current cursor position
 * XTERM_SEQ: Report position: ESC [ 6 n
 */
void
rxvt_term::scr_report_position ()
{
  tt_printf ("\033[%d;%dR", screen.cur.row + 1, screen.cur.col + 1);
}

/* ------------------------------------------------------------------------- *
 *                                  FONTS                                    *
 * ------------------------------------------------------------------------- */

/*
 * Set font style
 */
void
rxvt_term::set_font_style ()
{
  switch (charsets[screen.charset])
    {
      case '0':                   /* DEC Special Character & Line Drawing Set */
        break;
      case 'A':                   /* United Kingdom (UK) */
        break;
      case 'B':                   /* United States (USASCII) */
        break;
      case '<':                   /* Multinational character set */
        break;
      case '5':                   /* Finnish character set */
        break;
      case 'C':                   /* Finnish character set */
        break;
      case 'K':                   /* German character set */
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
void
rxvt_term::scr_charset_choose (int set)
{
  screen.charset = set;
  set_font_style ();
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
void
rxvt_term::scr_charset_set (int set, unsigned int ch)
{
  charsets[set] = (unsigned char)ch;
  set_font_style ();
}


/* ------------------------------------------------------------------------- *
 *                        MAJOR SCREEN MANIPULATION                          *
 * ------------------------------------------------------------------------- */

/*
 * refresh matching text.
 */
bool
rxvt_term::scr_refresh_rend (rend_t mask, rend_t value)
{
  bool found = false;

  for (int i = 0; i < TermWin.nrow; i++)
    {
      int col = 0;
      rend_t *drp = drawn_rend [i];

      for (; col < TermWin.ncol; col++, drp++)
        if ((*drp & mask) == value)
          {
            found = true;
            *drp = ~value;
          }
    }

  return found;
}

/*
 * Refresh an area
 */
enum {
  PART_BEG = 0,
  PART_END,
  RC_COUNT
};

void
rxvt_term::scr_expose (int x, int y, int width, int height, bool refresh)
{
  int i;
  row_col_t rc[RC_COUNT];

  if (drawn_text == NULL)  /* sanity check */
    return;

#ifndef NO_SLOW_LINK_SUPPORT
  if (refresh_type == FAST_REFRESH && !display->is_local)
    {
      y = 0;
      height = TermWin.height;
    }
#endif

#ifdef DEBUG_STRICT
  x = max (x, 0);
  x = min (x, (int)TermWin.width);
  y = max (y, 0);
  y = min (y, (int)TermWin.height);
#endif

  /* round down */
  rc[PART_BEG].col = Pixel2Col (x);
  rc[PART_BEG].row = Pixel2Row (y);
  /* round up */
  rc[PART_END].col = Pixel2Width (x + width + TermWin.fwidth - 1);
  rc[PART_END].row = Pixel2Row (y + height + TermWin.fheight - 1);

  /* sanity checks */
  for (i = PART_BEG; i < RC_COUNT; i++)
    {
      MIN_IT (rc[i].col, TermWin.ncol - 1);
      MIN_IT (rc[i].row, TermWin.nrow - 1);
    }

  for (i = rc[PART_BEG].row; i <= rc[PART_END].row; i++)
    fill_text (&drawn_text[i][rc[PART_BEG].col], 0, rc[PART_END].col - rc[PART_BEG].col + 1);

  if (refresh)
    scr_refresh (SLOW_REFRESH);
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the entire screen
 */
void
rxvt_term::scr_touch (bool refresh)
{
  scr_expose (0, 0, TermWin.width, TermWin.height, refresh);
}

/* ------------------------------------------------------------------------- */
/*
 * Move the display so that the line represented by scrollbar value Y is at
 * the top of the screen
 */
int
rxvt_term::scr_move_to (int y, int len)
{
  long p = 0;
  unsigned int oldviewstart;

  oldviewstart = TermWin.view_start;

  if (y < len)
    {
      p = (TermWin.nrow + TermWin.nscrolled) * (len - y) / len;
      p -= (long) (TermWin.nrow - 1);
      p = max (p, 0);
    }

  TermWin.view_start = (unsigned int)min (p, TermWin.nscrolled);

  return scr_changeview (oldviewstart);
}

/* ------------------------------------------------------------------------- */
/*
 * Page the screen up/down nlines
 * direction should be UP or DN
 */
int
rxvt_term::scr_page (enum page_dirn direction, int nlines)
{
  int n;
  unsigned int oldviewstart;

#ifdef DEBUG_STRICT
  assert ((nlines >= 0) && (nlines <= TermWin.nrow));
#endif
  oldviewstart = TermWin.view_start;
  if (direction == UP)
    {
      n = TermWin.view_start + nlines;
      TermWin.view_start = min (n, TermWin.nscrolled);
    }
  else
    {
      n = TermWin.view_start - nlines;
      TermWin.view_start = max (n, 0);
    }
  return scr_changeview (oldviewstart);
}

int
rxvt_term::scr_changeview (unsigned int oldviewstart)
{
  if (TermWin.view_start != oldviewstart)
    {
      want_refresh = 1;
      num_scr -= (TermWin.view_start - oldviewstart);
    }

  return (int) (TermWin.view_start - oldviewstart);
}

/* ------------------------------------------------------------------------- */
void
rxvt_term::scr_bell ()
{
#ifndef NO_BELL
# ifndef NO_MAPALERT
#  ifdef MAPALERT_OPTION
  if (options & Opt_mapAlert)
#  endif
    XMapWindow (display->display, TermWin.parent[0]);
# endif
  if (options & Opt_visualBell)
    {
      scr_rvideo_mode (!rvideo); /* refresh also done */
      scr_rvideo_mode (!rvideo); /* refresh also done */
    }
  else
    XBell (display->display, 0);
#endif
}

/* ------------------------------------------------------------------------- */
/* ARGSUSED */
void
rxvt_term::scr_printscreen (int fullhist)
{
#ifdef PRINTPIPE
  int i, r1, nrows, row_offset;
  FILE *fd;

  if ((fd = popen_printer ()) == NULL)
    return;

  nrows = TermWin.nrow;
  row_offset = TermWin.saveLines;

  if (!fullhist)
    row_offset -= TermWin.view_start;
  else
    {
      nrows += TermWin.nscrolled;
      row_offset -= TermWin.nscrolled;
    }

  wctomb (0, 0);

  for (r1 = 0; r1 < nrows; r1++)
    {
      text_t *tp = screen.text[r1 + row_offset];
      int len = screen.tlen[r1 + row_offset];

      for (i = len >= 0 ? len : TermWin.ncol - 1; i--; )
        {
          char mb[MB_LEN_MAX];
          text_t t = *tp++;
          if (t == NOCHAR)
            continue;

          len = wctomb (mb, t);

          if (len <= 0)
            {
              mb[0] = ' ';
              len = 1;
            }

          fwrite (mb, 1, len, fd);
        }

      fputc ('\n', fd);
    }

  pclose_printer (fd);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the screen
 * drawn_text/drawn_rend contain the screen information before the update.
 * screen.text/screen.rend contain what the screen will change to.
 */

#define FONT_WIDTH(X, Y)                                                \
    (X)->per_char[ (Y) - (X)->min_char_or_byte2].width
#define FONT_RBEAR(X, Y)                                                \
    (X)->per_char[ (Y) - (X)->min_char_or_byte2].rbearing
#define FONT_LBEAR(X, Y)                                                \
    (X)->per_char[ (Y) - (X)->min_char_or_byte2].lbearing
#define IS_FONT_CHAR(X, Y)                                              \
    ((Y) >= (X)->min_char_or_byte2 && (Y) <= (X)->max_char_or_byte2)

void
rxvt_term::scr_refresh (unsigned char refresh_type)
{
  unsigned char must_clear, /* use draw_string not draw_image_string     */
                showcursor; /* show the cursor                           */
  int16_t col, row,   /* column/row we're processing               */
          ocrow;      /* old cursor row                            */
  int i,              /* tmp                                       */
  row_offset;         /* basic offset in screen structure          */
#ifndef NO_CURSORCOLOR
  rend_t cc1;         /* store colours at cursor position (s)      */
#endif
  rend_t *crp;        // cursor rendition pointer

  want_refresh = 0;        /* screen is current */

  if (refresh_type == NO_REFRESH || !TermWin.mapped)
    return;

  /*
   * A: set up vars
   */
  must_clear = 0;
  refresh_count = 0;

  row_offset = TermWin.saveLines - TermWin.view_start;

#if XPM_BACKGROUND
  must_clear |= (bgPixmap.pixmap != None);
#endif
#if TRANSPARENT
  must_clear |= ((options & Opt_transparent) && am_transparent);
#endif
  ocrow = oldcursor.row; /* is there an old outline cursor on screen? */

  /*
   * B: reverse any characters which are selected
   */
  scr_reverse_selection ();

  /*
   * C: set the cursor character (s)
   */
  {
    unsigned char setoldcursor;
    rend_t ccol1,  /* Cursor colour       */
           ccol2;  /* Cursor colour2      */

    showcursor = (screen.flags & Screen_VisibleCursor);
#ifdef CURSOR_BLINK
    if (hidden_cursor)
      showcursor = 0;
#endif

    if (showcursor)
      {
        int col = screen.cur.col;

        while (col && screen.text[screen.cur.row + TermWin.saveLines][col] == NOCHAR)
          col--;

        crp = &screen.rend[screen.cur.row + TermWin.saveLines][col];

        if (showcursor && TermWin.focus)
          {
            *crp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
            cc1 = *crp & (RS_fgMask | RS_bgMask);
            if (ISSET_PIXCOLOR (Color_cursor))
              ccol1 = Color_cursor;
            else
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
              ccol1 = GET_FGCOLOR (rstyle);
#else
              ccol1 = Color_fg;
#endif
            if (ISSET_PIXCOLOR (Color_cursor2))
              ccol2 = Color_cursor2;
            else
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
              ccol2 = GET_BGCOLOR (rstyle);
#else
              ccol2 = Color_bg;
#endif
            *crp = SET_FGCOLOR (*crp, ccol1);
            *crp = SET_BGCOLOR (*crp, ccol2);
#endif
          }
      }

    /* make sure no outline cursor is left around */
    setoldcursor = 0;
    if (ocrow != -1)
      {
        if (screen.cur.row + TermWin.view_start != ocrow
            || screen.cur.col != oldcursor.col)
          {
            if (ocrow < TermWin.nrow
                && oldcursor.col < TermWin.ncol)
              drawn_rend[ocrow][oldcursor.col] ^= (RS_RVid | RS_Uline);

            if (TermWin.focus || !showcursor)
              oldcursor.row = -1;
            else
              setoldcursor = 1;
          }
      }
    else if (!TermWin.focus)
      setoldcursor = 1;

    if (setoldcursor)
      {
        if (screen.cur.row + TermWin.view_start >= TermWin.nrow)
          oldcursor.row = -1;
        else
          {
            oldcursor.row = screen.cur.row + TermWin.view_start;
            oldcursor.col = screen.cur.col;
          }
      }
  }

#if ENABLE_OVERLAY
  scr_swap_overlay ();
#endif

  rend_t *drp, *srp;  /* drawn-rend-pointer, screen-rend-pointer   */
  text_t *dtp, *stp;  /* drawn-text-pointer, screen-text-pointer   */

#ifndef NO_SLOW_LINK_SUPPORT
  /*
   * D: CopyArea pass - very useful for slower links
   *    This has been deliberately kept simple.
   */
  i = num_scr;
  if (!display->is_local
      && refresh_type == FAST_REFRESH && num_scr_allow && i
      && abs (i) < TermWin.nrow && !must_clear)
    {
      int16_t nits;
      int j;
      int len, wlen;

      j = TermWin.nrow;
      wlen = len = -1;
      row = i > 0 ? 0 : j - 1;
      for (; j-- >= 0; row += (i > 0 ? 1 : -1))
        {
          if (row + i >= 0 && row + i < TermWin.nrow && row + i != ocrow)
            {
              text_t *stp  = screen.text[row + row_offset];
              rend_t *srp  = screen.rend[row + row_offset];
              text_t *dtp  = drawn_text[row];
              text_t *dtp2 = drawn_text[row + i];
              rend_t *drp  = drawn_rend[row];
              rend_t *drp2 = drawn_rend[row + i];

              for (nits = 0, col = TermWin.ncol; col--; )
                if (stp[col] != dtp2[col] || srp[col] != drp2[col])
                  nits--;
                else if (stp[col] != dtp[col] || srp[col] != drp[col])
                  nits++;

              if (nits > 8) /* XXX: arbitrary choice */
                {
                  for (col = TermWin.ncol; col--; )
                    {
                      *dtp++ = *dtp2++;
                      *drp++ = *drp2++;
                    }

                  if (len == -1)
                    len = row;

                  wlen = row;
                  continue;
                }
            }

          if (len != -1)
            {
              /* also comes here at end if needed because of >= above */
              if (wlen < len)
                SWAP_IT (wlen, len, int);

              XCopyArea (display->display, TermWin.vt, TermWin.vt,
                         TermWin.gc, 0, Row2Pixel (len + i),
                         (unsigned int)TermWin_TotalWidth (),
                         (unsigned int)Height2Pixel (wlen - len + 1),
                         0, Row2Pixel (len));
              len = -1;
            }
        }
    }
#endif

  /*
   * E: main pass across every character
   */
  for (row = 0; row < TermWin.nrow; row++)
    {
      text_t *stp = screen.text[row + row_offset];
      rend_t *srp = screen.rend[row + row_offset];
      text_t *dtp = drawn_text[row];
      rend_t *drp = drawn_rend[row];

      /*
       * E2: OK, now the real pass
       */
      int ypixel = (int)Row2Pixel (row);

      for (col = 0; col < TermWin.ncol; col++)
        {
          /* compare new text with old - if exactly the same then continue */
          if (stp[col] == dtp[col]    /* Must match characters to skip. */
              && (RS_SAME (srp[col], drp[col])    /* Either rendition the same or   */
                  || (stp[col] == ' ' /* space w/ no background change  */
                      && GET_BGATTR (srp[col]) == GET_BGATTR (drp[col]))))
            continue;

          // redraw one or more characters

          // seek to the beginning if wide characters
          while (stp[col] == NOCHAR && col > 0)
            --col;

          rend_t rend = srp[col];     /* screen rendition (target rendtion) */
          text_t *text = stp + col;
          int count = 1;

          dtp[col] = stp[col];
          drp[col] = rend;

          int xpixel = Col2Pixel (col);

          for (i = 0; ++col < TermWin.ncol; )
            {
              if (stp[col] == NOCHAR)
                {
                  dtp[col] = stp[col];
                  drp[col] = rend;
                  count++;
                  i++;

                  continue;
                }

              if (!RS_SAME (rend, srp[col]))
                break;

              count++;

              if (stp[col] != dtp[col]
                  || !RS_SAME (srp[col], drp[col]))
                {
                  if (must_clear && (i++ > count / 2))
                    break;

                  dtp[col] = stp[col];
                  drp[col] = rend;
                  i = 0;
                }
              else if (must_clear || (stp[col] != ' ' && ++i >= 16))
                break;
            }

          col--;      /* went one too far.  move back */
          count -= i; /* dump any matching trailing chars */

          // sometimes we optimize away the trailing NOCHAR's, add them back
          while (i && text[count] == NOCHAR)
            count++, i--;

#if ENABLE_STYLES
          // force redraw after "careful" characters to avoid pixel droppings
          if (srp[col] & RS_Careful && col < TermWin.ncol - 1 && 0)
            drp[col + 1] = ~srp[col + 1];

          // include previous careful character(s) if possible, looks nicer (best effort...)
          while (text > stp
              && srp[text - stp - 1] & RS_Careful
              && RS_SAME (rend, srp[text - stp - 1]))
            text--, count++, xpixel -= TermWin.fwidth;
#endif

          /*
           * Determine the attributes for the string
           */
          int fore = GET_FGCOLOR (rend); // desired foreground
          int back = GET_BGCOLOR (rend); // desired background

          // only do special processing if ana attributes are set, which is rare
          if (rend & (RS_Bold | RS_Italic | RS_Uline | RS_RVid | RS_Blink))
            {
              bool invert = rend & RS_RVid;

#ifndef NO_BOLD_UNDERLINE_REVERSE
              if (rend & RS_Bold
                  && fore == Color_fg)
                {
                  if (ISSET_PIXCOLOR (Color_BD))
                    fore = Color_BD;
# if !ENABLE_STYLES
                  else
                    invert = !invert;
# endif
                }

              if (rend & RS_Italic
                  && fore == Color_fg)
                {
                  if (ISSET_PIXCOLOR (Color_IT))
                    fore = Color_IT;
# if !ENABLE_STYLES
                  else
                    invert = !invert;
# endif
                }

              if (rend & RS_Uline && ISSET_PIXCOLOR (Color_UL))
                fore = Color_UL;
#endif

              if (invert)
                {
                  SWAP_IT (fore, back, int);

#ifndef NO_BOLD_UNDERLINE_REVERSE
                  if (ISSET_PIXCOLOR (Color_RV))
                    back = Color_RV;
#endif
                }

#ifdef TEXT_BLINK
              if (rend & RS_Blink && (back == Color_bg || fore == Color_bg))
                {
                  if (!text_blink_ev.active)
                    {
                      text_blink_ev.start (NOW + TEXT_BLINK_INTERVAL);
                      hidden_text = 0;
                    }
                  else if (hidden_text)
                    fore = back;
                }
#endif
            }

          /*
           * Actually do the drawing of the string here
           */
          rxvt_font *font = (*TermWin.fontset[GET_STYLE (rend)])[GET_FONT (rend)];

          if (back == fore)
            font->clear_rect (*TermWin.drawable, xpixel, ypixel,
                              TermWin.fwidth * count, TermWin.fheight,
                              back);
          else if (back == Color_bg)
            {
              if (must_clear)
                {
                  CLEAR_CHARS (xpixel, ypixel, count);

                  for (i = 0; i < count; i++) /* don't draw empty strings */
                    if (text[i] != ' ')
                      {
                        font->draw (*TermWin.drawable, xpixel, ypixel, text, count, fore, -1);
                        break;
                      }
                }
              else
                font->draw (*TermWin.drawable, xpixel, ypixel, text, count, fore, Color_bg);
            }
          else
            font->draw (*TermWin.drawable, xpixel, ypixel, text, count, fore, back);

          if (rend & RS_Uline && font->descent > 1 && fore != back)
            XDrawLine (display->display, drawBuffer, TermWin.gc,
                       xpixel, ypixel + font->ascent + 1,
                       xpixel + Width2Pixel (count) - 1, ypixel + font->ascent + 1);
        }                     /* for (col....) */
    }                         /* for (row....) */

#if ENABLE_OVERLAY
  scr_swap_overlay ();
#endif

  /*
   * G: cleanup cursor and display outline cursor if necessary
   */
  if (showcursor)
    {
      if (TermWin.focus)
        {
          *crp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
          *crp = (*crp & ~ (RS_fgMask | RS_bgMask)) | cc1;
#endif
        }
      else if (oldcursor.row >= 0)
        {
#ifndef NO_CURSORCOLOR
          if (ISSET_PIXCOLOR (Color_cursor))
            XSetForeground (display->display, TermWin.gc, pix_colors[Color_cursor]);
#endif
          int cursorwidth = 1;
          while (oldcursor.col + cursorwidth < TermWin.ncol
                 && drawn_text[oldcursor.row][oldcursor.col + cursorwidth] == NOCHAR)
            cursorwidth++;

          XDrawRectangle (display->display, drawBuffer, TermWin.gc,
                          Col2Pixel (oldcursor.col),
                          Row2Pixel (oldcursor.row),
                          (unsigned int) (Width2Pixel (cursorwidth) - 1),
                          (unsigned int) (Height2Pixel (1) - TermWin.lineSpace - 1));
        }
    }

  /*
   * H: cleanup selection
   */
  scr_reverse_selection ();

  if (refresh_type & SMOOTH_REFRESH)
    XFlush (display->display);

  num_scr = 0;
  num_scr_allow = 1;
}

void
rxvt_term::scr_remap_chars (text_t *tp, rend_t *rp)
{
  if (!rp || !tp)
    return;

  for (int i = TermWin.ncol; i; i--, rp++, tp++)
    *rp = SET_FONT (*rp, FONTSET (*rp)->find_font (*tp));
}

void
rxvt_term::scr_remap_chars ()
{
  for (int i = TermWin.nrow + TermWin.saveLines; i--; )
    scr_remap_chars (screen.text[i], screen.rend[i]);

  for (int i = TermWin.nrow; i--; )
    {
      scr_remap_chars (drawn_text[i], drawn_rend[i]);
      scr_remap_chars (swap.text[i], swap.rend[i]);
    }
}

void
rxvt_term::scr_recolour ()
{
  if (1
#if TRANSPARENT
      && !am_transparent
#endif
#if XPM_BACKGROUND
      && !bgPixmap.pixmap
#endif
      )
    {
      XSetWindowBackground (display->display, TermWin.parent[0], pix_colors[Color_border]);
      XClearWindow (display->display, TermWin.parent[0]);
      XSetWindowBackground (display->display, TermWin.vt, pix_colors[Color_bg]);
#if HAVE_SCROLLBARS
      if (scrollBar.win)
        {
          XSetWindowBackground (display->display, scrollBar.win, pix_colors[Color_border]);
          scrollBar.setIdle ();
          scrollbar_show (0);
        }
#endif
    }

  scr_clear ();
  scr_touch (true);
  want_refresh = 1;
}

/* ------------------------------------------------------------------------- */
void
rxvt_term::scr_clear (bool really)
{
  if (!TermWin.mapped)
    return;

  num_scr_allow = 0;
  want_refresh = 1;

#if TRANSPARENT
  if ((options & Opt_transparent) && (am_pixmap_trans == 0))
    {
      int i;

      if (!(options & Opt_transparent_all))
        i = 0;
      else
        i = (int) (sizeof (TermWin.parent) / sizeof (Window));

      while (i--)
        if (TermWin.parent[i] != None)
          XClearWindow (display->display, TermWin.parent[i]);
    }
#endif

  if (really)
    XClearWindow (display->display, TermWin.vt);
}

/* ------------------------------------------------------------------------- */
void
rxvt_term::scr_reverse_selection ()
{
  if (selection.op && current_screen == selection.screen)
    {
      int end_row = TermWin.saveLines - TermWin.view_start;
      int i = selection.beg.row + TermWin.saveLines;
      int col, row = selection.end.row + TermWin.saveLines;
      rend_t *srp;

#if ENABLE_FRILLS
      if (selection.rect)
        {
          end_row += TermWin.nrow;

          for (; i <= row && i <= end_row; i++)
            for (srp = screen.rend[i], col = selection.beg.col; col < selection.end.col; col++)
              srp[col] ^= RS_RVid;
        }
      else
#endif
        {
          if (i >= end_row)
            col = selection.beg.col;
          else
            {
              col = 0;
              i = end_row;
            }

          end_row += TermWin.nrow;

          for (; i < row && i < end_row; i++, col = 0)
            for (srp = screen.rend[i]; col < TermWin.ncol; col++)
              srp[col] ^= RS_RVid;

          if (i == row && i < end_row)
            for (srp = screen.rend[i]; col < selection.end.col; col++)
              srp[col] ^= RS_RVid;
        }
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Dump the whole scrollback and screen to the passed filedescriptor.  The
 * invoking routine must close the fd.
 */
#if 0
void
rxvt_term::scr_dump (int fd)
{
  int             row, wrote;
  unsigned int    width, towrite;
  char            r1[] = "\n";

  for (row = TermWin.saveLines - TermWin.nscrolled;
       row < TermWin.saveLines + TermWin.nrow - 1; row++)
    {
      width = screen.tlen[row] >= 0 ? screen.tlen[row]
              : TermWin.ncol;
      for (towrite = width; towrite; towrite -= wrote)
        {
          wrote = write (fd, & (screen.text[row][width - towrite]),
                        towrite);
          if (wrote < 0)
            return;         /* XXX: death, no report */
        }
      if (screen.tlen[row] >= 0)
        if (write (fd, r1, 1) <= 0)
          return; /* XXX: death, no report */
    }
}
#endif

/* ------------------------------------------------------------------------- *
 *                           CHARACTER SELECTION                             *
 * ------------------------------------------------------------------------- */

/*
 * -TermWin.nscrolled <= (selection row) <= TermWin.nrow - 1
 */
void
rxvt_term::selection_check (int check_more)
{
  row_col_t pos;

  if (!selection.op)
    return;

  pos.row = pos.col = 0;
  if ((selection.beg.row < - (int32_t)TermWin.nscrolled)
      || (selection.beg.row >= TermWin.nrow)
      || (selection.mark.row < - (int32_t)TermWin.nscrolled)
      || (selection.mark.row >= TermWin.nrow)
      || (selection.end.row < - (int32_t)TermWin.nscrolled)
      || (selection.end.row >= TermWin.nrow)
      || (check_more == 1
          && current_screen == selection.screen
          && !ROWCOL_IS_BEFORE (screen.cur, selection.beg)
          && ROWCOL_IS_BEFORE (screen.cur, selection.end))
      || (check_more == 2
          && ROWCOL_IS_BEFORE (selection.beg, pos)
          && ROWCOL_IS_AFTER (selection.end, pos))
      || (check_more == 3
          && ROWCOL_IS_AFTER (selection.end, pos))
      || (check_more == 4     /* screen width change */
          && (selection.beg.row != selection.end.row
              || selection.end.col > TermWin.ncol)))
    CLEAR_SELECTION ();
}

/* ------------------------------------------------------------------------- */
/*
 * Paste a selection direct to the command fd
 */
void
rxvt_term::paste (const unsigned char *data, unsigned int len)
{
  unsigned int i, j, n;
  unsigned char *ds = (unsigned char *)rxvt_malloc (PROP_SIZE);

  /* convert normal newline chars into common keyboard Return key sequence */
  for (i = 0; i < len; i += PROP_SIZE)
    {
      n = min (len - i, PROP_SIZE);
      memcpy (ds, data + i, n);

      for (j = 0; j < n; j++)
        if (ds[j] == C0_LF)
          ds[j] = C0_CR;

      tt_write (ds, (int)n);
    }

  free (ds);
}

/* ------------------------------------------------------------------------- */
/*
 * Respond to a notification that a primary selection has been sent
 * EXT: SelectionNotify
 */
int
rxvt_term::selection_paste (Window win, Atom prop, bool delete_prop)
{
  long nread = 0;
  unsigned long bytes_after;
  XTextProperty ct;

  if (prop == None)         /* check for failed XConvertSelection */
    {
      if ((selection_type & Sel_CompoundText))
        {
          int selnum = selection_type & Sel_whereMask;

          selection_type = 0;
          if (selnum != Sel_direct)
            selection_request_other (XA_STRING, selnum);
        }

      if ((selection_type & Sel_UTF8String))
        {
          int selnum = selection_type & Sel_whereMask;

          selection_type = Sel_CompoundText;
          if (selnum != Sel_direct)
            selection_request_other (xa[XA_COMPOUND_TEXT], selnum);
          else
            selection_type = 0;
        }

      return 0;
    }

  for (;;)
    {
      if (XGetWindowProperty (display->display, win, prop, (long) (nread / 4),
                              (long) (PROP_SIZE / 4), delete_prop,
                              AnyPropertyType, &ct.encoding, &ct.format,
                              &ct.nitems, &bytes_after,
                              &ct.value) != Success)
        break;

      if (ct.encoding == 0)
        break;

      if (ct.encoding == xa[XA_INCR])
        {
          // INCR selection, start handshake
          XDeleteProperty (display->display, win, prop);
          selection_wait = Sel_incr;
          incr_ev.start (NOW + 10);
          break;
        }

      if (ct.value == NULL)
        continue;

      if (ct.nitems == 0)
        {
          if (selection_wait == Sel_normal && nread == 0
              && (win != display->root || prop != XA_CUT_BUFFER0)) // avoid recursion
            {
              /*
               * pass through again trying CUT_BUFFER0 if we've come from
               * XConvertSelection () but nothing was presented
               */
              selection_paste (display->root, XA_CUT_BUFFER0, False);
            }

          nread = -1;         /* discount any previous stuff */
          break;
        }

      nread += ct.nitems;

      char **cl;
      int cr;
      if (XmbTextPropertyToTextList (display->display, &ct, &cl, &cr) >= 0 && cl)
        {
          for (int i = 0; i < cr; i++)
            paste ((unsigned char *)cl[i], strlen (cl[i]));

          XFreeStringList (cl);
        }
      else
        paste (ct.value, ct.nitems);

      if (bytes_after == 0)
        break;

      XFree (ct.value);
    }

  if (ct.value)
    XFree (ct.value);

  if (selection_wait == Sel_normal)
    selection_wait = Sel_none;

  return (int)nread;
}

void
rxvt_term::incr_cb (time_watcher &w)
{
  selection_wait = Sel_none;

  rxvt_warn ("data loss: timeout on INCR selection paste, ignoring.\n");
}

/*
 * INCR support originally provided by Paul Sheer <psheer@obsidian.co.za>
 */
void
rxvt_term::selection_property (Window win, Atom prop)
{
  if (prop == None || selection_wait != Sel_incr)
    return;

  if (selection_paste (win, prop, 1) > 0)
    incr_ev.start (NOW + 10);
  else
    {
      selection_wait = Sel_none;
      incr_ev.stop ();
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Request the current selection: 
 * Order: > internal selection if available
 *        > PRIMARY, SECONDARY, CLIPBOARD if ownership is claimed (+)
 *        > CUT_BUFFER0
 * (+) if ownership is claimed but property is empty, rxvt_selection_paste ()
 *     will auto fallback to CUT_BUFFER0
 * EXT: button 2 release
 */
void
rxvt_term::selection_request (Time tm, int x, int y)
{
  if (x < 0 || x >= TermWin.width || y < 0 || y >= TermWin.height)
    return;                 /* outside window */

  if (selection.text)
    { /* internal selection */
      char *str = rxvt_wcstombs (selection.text, selection.len);
      paste ((unsigned char *)str, strlen (str));
      free (str);
      return;
    }
  else
    {
      int i;

      selection_request_time = tm;
      selection_wait = Sel_normal;

      for (i = Sel_Primary; i <= Sel_Clipboard; i++)
        {
#if X_HAVE_UTF8_STRING
          selection_type = Sel_UTF8String;
          if (selection_request_other (xa[XA_UTF8_STRING], i))
            return;
#else
          selection_type = Sel_CompoundText;
          if (selection_request_other (xa[XA_COMPOUND_TEXT], i))
            return;
#endif

        }
    }

  selection_wait = Sel_none;       /* don't loop in rxvt_selection_paste () */
  selection_paste (display->root, XA_CUT_BUFFER0, False);
}

int
rxvt_term::selection_request_other (Atom target, int selnum)
{
  Atom sel;
#ifdef DEBUG_SELECT
  char *debug_xa_names[] = { "PRIMARY", "SECONDARY", "CLIPBOARD" };
#endif

  selection_type |= selnum;

  if (selnum == Sel_Primary)
    sel = XA_PRIMARY;
  else if (selnum == Sel_Secondary)
    sel = XA_SECONDARY;
  else
    sel = xa[XA_CLIPBOARD];

  if (XGetSelectionOwner (display->display, sel) != None)
    {
      XConvertSelection (display->display, sel, target, xa[XA_VT_SELECTION],
                         TermWin.vt, selection_request_time);
      return 1;
    }

  return 0;
}

/* ------------------------------------------------------------------------- */
/*
 * Clear all selected text
 * EXT: SelectionClear
 */
void
rxvt_term::selection_clear ()
{
  want_refresh = 1;
  free (selection.text);
  selection.text = NULL;
  selection.len = 0;
  CLEAR_SELECTION ();

  if (display->selection_owner == this)
    display->selection_owner = 0;
}

/* ------------------------------------------------------------------------- */
/*
 * Copy a selection into the cut buffer
 * EXT: button 1 or 3 release
 */
void
rxvt_term::selection_make (Time tm)
{
  int i, col, end_col, row, end_row;
  wchar_t *new_selection_text;
  text_t *t;

  switch (selection.op)
    {
      case SELECTION_CONT:
        break;
      case SELECTION_INIT:
        CLEAR_SELECTION ();
        /* FALLTHROUGH */
      case SELECTION_BEGIN:
        selection.op = SELECTION_DONE;
        /* FALLTHROUGH */
      default:
        return;
    }

  selection.op = SELECTION_DONE;

  if (selection.clicks == 4)
    return;                 /* nothing selected, go away */

  i = (selection.end.row - selection.beg.row + 1) * (TermWin.ncol + 1);
  new_selection_text = (wchar_t *)rxvt_malloc ((i + 4) * sizeof (wchar_t));

  col = selection.beg.col;
  row = selection.beg.row + TermWin.saveLines;
  end_row = selection.end.row + TermWin.saveLines;
  int ofs = 0;
  int extra = 0;

  for (; row <= end_row; row++, col = 0)
    {
      end_col = screen.tlen[row];

#if ENABLE_FRILLS
      if (selection.rect)
        {
          col = selection.beg.col;
          end_col = TermWin.ncol + 1;
        }
#endif

      MAX_IT (col, 0);

      if (end_col == -1)
        end_col = TermWin.ncol;

      if (row == end_row || selection.rect)
        MIN_IT (end_col, selection.end.col);

      t = &screen.text[row][col];
      for (; col < end_col; col++)
        {
          if (*t == NOCHAR)
            t++;
#if ENABLE_COMBINING
          else if (IS_COMPOSE (*t))
            {
              int len = rxvt_composite.expand (*t, 0);

              extra -= (len - 1);

              if (extra < 0)
                {
                  extra += i;
                  i += i;
                  new_selection_text = (wchar_t *)rxvt_realloc (new_selection_text, (i + 4) * sizeof (wchar_t));
                }

              ofs += rxvt_composite.expand (*t++, new_selection_text + ofs);
            }
#endif
          else
            new_selection_text[ofs++] = *t++;
        }

      if (screen.tlen[row] != -1 && row != end_row)
        new_selection_text[ofs++] = C0_LF;
    }

  if (end_col != selection.end.col)
    new_selection_text[ofs++] = C0_LF;

  new_selection_text[ofs] = 0;

  if (ofs == 0)
    {
      free (new_selection_text);
      return;
    }

  free (selection.text);

  // we usually allocate much more than necessary, so realloc it smaller again
  selection.len = ofs;
  selection.text = (wchar_t *)rxvt_realloc (new_selection_text, (ofs + 1) * sizeof (wchar_t));

  XSetSelectionOwner (display->display, XA_PRIMARY, TermWin.vt, tm);
  if (XGetSelectionOwner (display->display, XA_PRIMARY) == TermWin.vt)
    display->set_selection_owner (this);
  else
    rxvt_warn ("can't get primary selection, ignoring.\n");

#if 0
  XTextProperty ct;

  if (XwcTextListToTextProperty (display->display, &selection.text, 1, XStringStyle, &ct) >= 0)
    {
      XChangeProperty (display->display, display->root, XA_CUT_BUFFER0, XA_STRING, 8,
                       PropModeReplace, ct.value, ct.nitems);
      XFree (ct.value);
    }
#endif

  selection_time = tm;
}

/* ------------------------------------------------------------------------- */
/*
 * Mark or select text based upon number of clicks: 1, 2, or 3
 * EXT: button 1 press
 */
void
rxvt_term::selection_click (int clicks, int x, int y)
{
  clicks = ((clicks - 1) % 3) + 1;
  selection.clicks = clicks;       /* save clicks so extend will work */

  selection_start_colrow (Pixel2Col (x), Pixel2Row (y));

  if (clicks == 2 || clicks == 3)
    selection_extend_colrow (selection.mark.col,
                             selection.mark.row + TermWin.view_start,
                             0, /* button 3     */
                             1, /* button press */
                             0);        /* click change */
}

/* ------------------------------------------------------------------------- */
/*
 * Mark a selection at the specified col/row
 */
void
rxvt_term::selection_start_colrow (int col, int row)
{
  want_refresh = 1;
  selection.mark.col = col;
  selection.mark.row = row - TermWin.view_start;

  MAX_IT (selection.mark.row, - (int32_t)TermWin.nscrolled);
  MIN_IT (selection.mark.row, (int32_t)TermWin.nrow - 1);
  MAX_IT (selection.mark.col, 0);
  MIN_IT (selection.mark.col, (int32_t)TermWin.ncol - 1);

  while (selection.mark.col > 0
         && screen.text[selection.mark.row + TermWin.saveLines][selection.mark.col] == NOCHAR)
    --selection.mark.col;
  
  if (selection.op)
    {      /* clear the old selection */
      selection.beg.row = selection.end.row = selection.mark.row;
      selection.beg.col = selection.end.col = selection.mark.col;
    }

  selection.op = SELECTION_INIT;
  selection.screen = current_screen;
}

/* ------------------------------------------------------------------------- */
/*
 * Word select: select text for 2 clicks
 * We now only find out the boundary in one direction
 */

/* what do we want: spaces/tabs are delimiters or cutchars or non-cutchars */
#define DELIMIT_TEXT(x) 		\
    (unicode::is_space (x) ? 2 : (x) <= 0xff && !!strchr (rs[Rs_cutchars], (x)))
#define DELIMIT_REND(x)        1

void
rxvt_term::selection_delimit_word (enum page_dirn dirn, const row_col_t *mark, row_col_t *ret)
{
  int col, row, dirnadd, tcol, trow, w1, w2;
  row_col_t bound;
  text_t *stp;
  rend_t *srp;

  if (dirn == UP)
    {
      bound.row = TermWin.saveLines - TermWin.nscrolled - 1;
      bound.col = 0;
      dirnadd = -1;
    }
  else
    {
      bound.row = TermWin.saveLines + TermWin.nrow;
      bound.col = TermWin.ncol - 1;
      dirnadd = 1;
    }

  row = mark->row + TermWin.saveLines;
  col = mark->col;
  MAX_IT (col, 0);
  /* find the edge of a word */
  stp = &screen.text[row][col];
  w1 = DELIMIT_TEXT (*stp);

  srp = &screen.rend[row][col];
  w2 = DELIMIT_REND (*srp);

  for (;;)
    {
      for (; col != bound.col; col += dirnadd)
        {
          stp += dirnadd;
          srp += dirnadd;

          if (*stp == NOCHAR)
            continue;

          if (DELIMIT_TEXT (*stp) != w1)
            break;
          if (DELIMIT_REND (*srp) != w2)
            break;
        }

      if ((col == bound.col) && (row != bound.row))
        {
          if (screen.tlen[ (row - (dirn == UP ? 1 : 0))] == -1)
            {
              trow = row + dirnadd;
              tcol = dirn == UP ? TermWin.ncol - 1 : 0;

              if (screen.text[trow] == NULL)
                break;

              stp = & (screen.text[trow][tcol]);
              srp = & (screen.rend[trow][tcol]);

              if (DELIMIT_TEXT (*stp) != w1 || DELIMIT_REND (*srp) != w2)
                break;

              row = trow;
              col = tcol;
              continue;
            }
        }
      break;
    }

Old_Word_Selection_You_Die:
  if (dirn == DN)
    col++;                  /* put us on one past the end */

  /* Poke the values back in */
  ret->row = row - TermWin.saveLines;
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
void
rxvt_term::selection_extend (int x, int y, int flag)
{
  int col, row;

  col = Pixel2Col (x);
  row = Pixel2Row (y);
  MAX_IT (row, 0);
  MIN_IT (row, (int)TermWin.nrow - 1);
  MAX_IT (col, 0);
  MIN_IT (col, (int)TermWin.ncol);

  /*
  * If we're selecting characters (single click) then we must check first
  * if we are at the same place as the original mark.  If we are then
  * select nothing.  Otherwise, if we're to the right of the mark, you have to
  * be _past_ a character for it to be selected.
  */
  if (((selection.clicks % 3) == 1) && !flag
      && (col == selection.mark.col
          && (row == selection.mark.row + TermWin.view_start)))
    {
      /* select nothing */
      selection.beg.row = selection.end.row = 0;
      selection.beg.col = selection.end.col = 0;
      selection.clicks = 4;
      want_refresh = 1;
      return;
    }

  if (selection.clicks == 4)
    selection.clicks = 1;

  selection_extend_colrow (col, row, !!flag,  /* ? button 3      */
                           flag == 1 ? 1 : 0,     /* ? button press  */
                           0);    /* no click change */
}

/* ------------------------------------------------------------------------- */
/*
 * Extend the selection to the specified col/row
 */
void
rxvt_term::selection_extend_colrow (int32_t col, int32_t row, int button3, int buttonpress, int clickchange)
{
  int16_t ncol = TermWin.ncol;
  int end_col;
  row_col_t pos;
  enum {
    LEFT, RIGHT
  } closeto = RIGHT;

  want_refresh = 1;

  switch (selection.op)
    {
      case SELECTION_INIT:
        CLEAR_SELECTION ();
        selection.op = SELECTION_BEGIN;
        /* FALLTHROUGH */
      case SELECTION_BEGIN:
        if (row != selection.mark.row || col != selection.mark.col
            || (!button3 && buttonpress))
          selection.op = SELECTION_CONT;
        break;
      case SELECTION_DONE:
        selection.op = SELECTION_CONT;
        /* FALLTHROUGH */
      case SELECTION_CONT:
        break;
      case SELECTION_CLEAR:
        selection_start_colrow (col, row);
        /* FALLTHROUGH */
      default:
        return;
    }

  if (selection.beg.col == selection.end.col
      && selection.beg.col != selection.mark.col
      && selection.beg.row == selection.end.row
      && selection.beg.row != selection.mark.row)
    {
      selection.beg.col = selection.end.col = selection.mark.col;
      selection.beg.row = selection.end.row = selection.mark.row;
    }

  pos.col = col;
  pos.row = row;

  pos.row -= TermWin.view_start;   /* adjust for scroll */

  /*
   * This is mainly xterm style selection with a couple of differences, mainly
   * in the way button3 drag extension works.
   * We're either doing: button1 drag; button3 press; or button3 drag
   *  a) button1 drag : select around a midpoint/word/line - that point/word/line
   *     is always at the left/right edge of the selection.
   *  b) button3 press: extend/contract character/word/line at whichever edge of
   *     the selection we are closest to.
   *  c) button3 drag : extend/contract character/word/line - we select around
   *     a point/word/line which is either the start or end of the selection
   *     and it was decided by whichever point/word/line was `fixed' at the
   *     time of the most recent button3 press
   */
  if (button3 && buttonpress)
    { /* button3 press */
      /*
       * first determine which edge of the selection we are closest to
       */
      if (ROWCOL_IS_BEFORE (pos, selection.beg)
          || (!ROWCOL_IS_AFTER (pos, selection.end)
              && (((pos.col - selection.beg.col)
                   + ((pos.row - selection.beg.row) * ncol))
                  < ((selection.end.col - pos.col)
                     + ((selection.end.row - pos.row) * ncol)))))
        closeto = LEFT;

      if (closeto == LEFT)
        {
          selection.beg.row = pos.row;
          selection.beg.col = pos.col;
          selection.mark.row = selection.end.row;
          selection.mark.col = selection.end.col - (selection.clicks == 2);
        }
      else
        {
          selection.end.row = pos.row;
          selection.end.col = pos.col;
          selection.mark.row = selection.beg.row;
          selection.mark.col = selection.beg.col;
        }
    }
  else
    { /* button1 drag or button3 drag */
      if (ROWCOL_IS_AFTER (selection.mark, pos))
        {
          if (selection.mark.row == selection.end.row
              && selection.mark.col == selection.end.col
              && clickchange
              && selection.clicks == 2)
            selection.mark.col--;

          selection.beg.row = pos.row;
          selection.beg.col = pos.col;
          selection.end.row = selection.mark.row;
          selection.end.col = selection.mark.col + (selection.clicks == 2);
        }
      else
        {
          selection.beg.row = selection.mark.row;
          selection.beg.col = selection.mark.col;
          selection.end.row = pos.row;
          selection.end.col = pos.col;
        }
    }

  if (selection.clicks == 1)
    {
      end_col = screen.tlen[selection.beg.row + TermWin.saveLines];

      if (selection.beg.col > end_col
          && end_col != -1
#if ENABLE_FRILLS
          && !selection.rect
#endif
         )
        selection.beg.col = ncol;

      end_col = screen.tlen[selection.end.row + TermWin.saveLines];

      if (
          selection.end.col > end_col
          && end_col != -1
#if ENABLE_FRILLS
          && !selection.rect
#endif
         )
        selection.end.col = ncol;
    }
  else if (selection.clicks == 2)
    {
      if (ROWCOL_IS_AFTER (selection.end, selection.beg))
        selection.end.col--;

      selection_delimit_word (UP, &selection.beg, &selection.beg);
      selection_delimit_word (DN, &selection.end, &selection.end);
    }
  else if (selection.clicks == 3)
    {
#if ENABLE_FRILLS
      if ((options & Opt_tripleclickwords))
        {
          int end_row;

          selection_delimit_word (UP, &selection.beg, &selection.beg);
          end_row = screen.tlen[selection.mark.row + TermWin.saveLines];

          for (end_row = selection.mark.row; end_row < TermWin.nrow; end_row++)
            {
              end_col = screen.tlen[end_row + TermWin.saveLines];

              if (end_col != -1)
                {
                  selection.end.row = end_row;
                  selection.end.col = end_col;
                  selection_remove_trailing_spaces ();
                  break;
                }
            }
        }
      else
#endif
        {
          if (ROWCOL_IS_AFTER (selection.mark, selection.beg))
            selection.mark.col++;

          selection.beg.col = 0;
          selection.end.col = ncol;
        }
    }

  if (button3 && buttonpress)
    { /* mark may need to be changed */
      if (closeto == LEFT)
        {
          selection.mark.row = selection.end.row;
          selection.mark.col = selection.end.col - (selection.clicks == 2);
        }
      else
        {
          selection.mark.row = selection.beg.row;
          selection.mark.col = selection.beg.col;
        }
    }

#if ENABLE_FRILLS
  if (selection.rect && selection.beg.col > selection.end.col)
    SWAP_IT (selection.beg.col, selection.end.col, int);
#endif
}

#if ENABLE_FRILLS
void
rxvt_term::selection_remove_trailing_spaces ()
{
  int32_t end_col, end_row;
  text_t *stp;

  end_col = selection.end.col;
  end_row = selection.end.row;

  for ( ; end_row >= selection.beg.row; )
    {
      stp = screen.text[end_row + TermWin.saveLines];

      while (--end_col >= 0)
        {
          if (stp[end_col] != ' '
              && stp[end_col] != '\t'
              && stp[end_col] != NOCHAR)
            break;
        }

      if (end_col >= 0
          || screen.tlen[end_row - 1 + TermWin.saveLines] != -1)
        {
          selection.end.col = end_col + 1;
          selection.end.row = end_row;
          break;
        }

      end_row--;
      end_col = TermWin.ncol;
    }

  if (selection.mark.row > selection.end.row)
    {
      selection.mark.row = selection.end.row;
      selection.mark.col = selection.end.col;
    }
  else if (selection.mark.row == selection.end.row
           && selection.mark.col > selection.end.col)
    selection.mark.col = selection.end.col;
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Double click on button 3 when already selected
 * EXT: button 3 double click
 */
void
rxvt_term::selection_rotate (int x, int y)
{
  selection.clicks = selection.clicks % 3 + 1;
  selection_extend_colrow (Pixel2Col (x), Pixel2Row (y), 1, 0, 1);
}

/* ------------------------------------------------------------------------- */
/*
 * Respond to a request for our current selection
 * EXT: SelectionRequest
 */
void
rxvt_term::selection_send (const XSelectionRequestEvent &rq)
{
  XSelectionEvent ev;
  XTextProperty ct;
  XICCEncodingStyle style;
  Atom target;

  ev.type = SelectionNotify;
  ev.property = None;
  ev.display = rq.display;
  ev.requestor = rq.requestor;
  ev.selection = rq.selection;
  ev.target = rq.target;
  ev.time = rq.time;

  if (rq.target == xa[XA_TARGETS])
    {
      Atom target_list[6];
      Atom *target = target_list;

      *target++ = xa[XA_TARGETS];
      *target++ = xa[XA_TIMESTAMP];
      *target++ = XA_STRING;
      *target++ = xa[XA_TEXT];
      *target++ = xa[XA_COMPOUND_TEXT];
#if X_HAVE_UTF8_STRING
      *target++ = xa[XA_UTF8_STRING];
#endif

      XChangeProperty (display->display, rq.requestor, rq.property, XA_ATOM,
                       32, PropModeReplace,
                       (unsigned char *)target_list, target - target_list);
      ev.property = rq.property;
    }
#if TODO // TODO
  else if (rq.target == xa[XA_MULTIPLE])
    {
      /* TODO: Handle MULTIPLE */
    }
#endif
  else if (rq.target == xa[XA_TIMESTAMP] && selection.text)
    {
      XChangeProperty (display->display, rq.requestor, rq.property, rq.target,
                       32, PropModeReplace, (unsigned char *)&selection_time, 1);
      ev.property = rq.property;
    }
  else if (rq.target == XA_STRING
           || rq.target == xa[XA_TEXT]
           || rq.target == xa[XA_COMPOUND_TEXT]
           || rq.target == xa[XA_UTF8_STRING]
          )
    {
      short freect = 0;
      int selectlen;
      wchar_t *cl;

      target = rq.target;

      if (target == XA_STRING)
        // we actually don't do XA_STRING, but who cares, as i18n clients
        // will ask for another format anyways.
        style = XStringStyle;
      else if (target == xa[XA_TEXT])
        style = XStdICCTextStyle;
      else if (target == xa[XA_COMPOUND_TEXT])
        style = XCompoundTextStyle;
#if X_HAVE_UTF8_STRING
      else if (target == xa[XA_UTF8_STRING])
        style = XUTF8StringStyle;
#endif
      else
        {
          target = xa[XA_COMPOUND_TEXT];
          style = XCompoundTextStyle;
        }

      if (selection.text)
        {
          cl = selection.text;
          selectlen = selection.len;
        }
      else
        {
          cl = L"";
          selectlen = 0;
        }

      // Xwc doesn't handle iso-10646 in wchar_t gracefully, so maybe recode it
      // manually for XUTF8StringStyle.
      if (XwcTextListToTextProperty (display->display, &cl, 1, style, &ct) >= 0)
        freect = 1;
      else
        {
          /* if we failed to convert then send it raw */
          ct.value = (unsigned char *)cl;
          ct.nitems = selectlen;
        }

      XChangeProperty (display->display, rq.requestor, rq.property,
                       target, 8, PropModeReplace,
                       ct.value, (int)ct.nitems);
      ev.property = rq.property;

      if (freect)
        XFree (ct.value);
    }

  XSendEvent (display->display, rq.requestor, False, 0L, (XEvent *)&ev);
}

/* ------------------------------------------------------------------------- *
 *                              MOUSE ROUTINES                               *
 * ------------------------------------------------------------------------- */

/*
 * return col/row values corresponding to x/y pixel values
 */
void
rxvt_term::pixel_position (int *x, int *y)
{
  *x = Pixel2Col (*x);
  /* MAX_IT (*x, 0); MIN_IT (*x, (int)TermWin.ncol - 1); */
  *y = Pixel2Row (*y);
  /* MAX_IT (*y, 0); MIN_IT (*y, (int)TermWin.nrow - 1); */
}

/* ------------------------------------------------------------------------- */
#ifdef USE_XIM
void
rxvt_term::im_set_position (XPoint &pos)
{
  XWindowAttributes xwa;

  XGetWindowAttributes (display->display, TermWin.vt, &xwa);

  pos.x = xwa.x + Col2Pixel    (screen.cur.col);
  pos.y = xwa.y + Height2Pixel (screen.cur.row) + TermWin.fbase;
}
#endif

#if ENABLE_OVERLAY
void
rxvt_term::scr_overlay_new (int x, int y, int w, int h)
{
  if (TermWin.nrow < 3 || TermWin.ncol < 3)
    return;

  want_refresh = 1;

  scr_overlay_off ();

  if (x < 0) x = TermWin.ncol - w;
  if (y < 0) y = TermWin.nrow - h;

  // make space for border
  w += 2; MIN_IT (w, TermWin.ncol);
  h += 2; MIN_IT (h, TermWin.nrow);

  x -= 1; MAX_IT (x, 0);
  y -= 1; MAX_IT (y, 0);

  MIN_IT (x, TermWin.ncol - w);
  MIN_IT (y, TermWin.nrow - h);

  ov_x = x; ov_y = y;
  ov_w = w; ov_h = h;

  ov_text = new text_t *[h];
  ov_rend = new rend_t *[h];

  for (y = 0; y < h; y++)
    {
      text_t *tp = ov_text[y] = new text_t[w];
      rend_t *rp = ov_rend[y] = new rend_t[w];

      text_t t0, t1, t2;
      rend_t r = OVERLAY_RSTYLE;

      if (y == 0)
        t0 = 0x2554, t1 = 0x2550, t2 = 0x2557;
      else if (y < h - 1)
        t0 = 0x2551, t1 = 0x0020, t2 = 0x2551;
      else
        t0 = 0x255a, t1 = 0x2550, t2 = 0x255d;

      *tp++ = t0;
      *rp++ = r;

      for (x = w - 2; x > 0; --x)
        {
          *tp++ = t1;
          *rp++ = r;
        }

      *tp = t2;
      *rp = r;
    }
}

void
rxvt_term::scr_overlay_off ()
{
  if (!ov_text)
    return;

  want_refresh = 1;

  for (int y = 0; y < ov_h; y++)
    {
      delete [] ov_text[y];
      delete [] ov_rend[y];
    }

  delete [] ov_text; ov_text = 0;
  delete [] ov_rend; ov_rend = 0;
}

void
rxvt_term::scr_overlay_set (int x, int y, text_t text, rend_t rend)
{
  if (!ov_text || x >= ov_w - 2 || y >= ov_h - 2)
    return;

  x++, y++;

  ov_text[y][x] = text;
  ov_rend[y][x] = rend;
}

void
rxvt_term::scr_overlay_set (int x, int y, const char *s)
{
  while (*s)
    scr_overlay_set (x++, y, *s++);
}

void
rxvt_term::scr_swap_overlay ()
{
  if (!ov_text)
    return;

  int row_offset = ov_y + TermWin.saveLines - TermWin.view_start;

  // swap screen mem with overlay
  for (int y = ov_h; y--; )
    {
      text_t *t1 = ov_text[y];
      rend_t *r1 = ov_rend[y];

      text_t *t2 = screen.text[y + row_offset] + ov_x;
      rend_t *r2 = screen.rend[y + row_offset] + ov_x;

      for (int x = ov_w; x--; )
        {
          text_t t = *t1; *t1++ = *t2; *t2++ = t;
          rend_t r = *r1; *r1++ = *r2; *r2++ = SET_FONT (r, FONTSET (r)->find_font (t));
        }
    }
}

#endif
/* ------------------------------------------------------------------------- */
