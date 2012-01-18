/*---------------------------------------------------------------------------*
 * File:        screen.C
 *---------------------------------------------------------------------------*
 *
 * Copyright (c) 1997-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2003-2007 Marc Lehmann <schmorp@schmorp.de>
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
#include "rxvtperl.h"           /* NECESSARY */

#include <inttypes.h>

#include "salloc.C" // HACK, should be a separate compile!

static inline void
fill_text (text_t *start, text_t value, int len)
{
  while (len--)
    *start++ = value;
}

/* ------------------------------------------------------------------------- */
#define TABSIZE                 8       /* default tab size */

/* ------------------------------------------------------------------------- *
 *             GENERAL SCREEN AND SELECTION UPDATE ROUTINES                  *
 * ------------------------------------------------------------------------- */
#define ZERO_SCROLLBACK()                                              \
    if (option (Opt_scrollTtyOutput))                                  \
        view_start = 0
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
 * CLEAR_CHARS: clear <num> chars starting from pixel position <x,y>
 */
#define CLEAR_CHARS(x, y, num)                                         \
    if (mapped)                                                        \
        XClearArea (dpy, vt, x, y,                                     \
                    (unsigned int)Width2Pixel (num),                   \
                    (unsigned int)Height2Pixel (1), False)

/* ------------------------------------------------------------------------- *
 *                        SCREEN `COMMON' ROUTINES                           *
 * ------------------------------------------------------------------------- */

/* Fill part/all of a line with blanks. */
void
rxvt_term::scr_blank_line (line_t &l, unsigned int col, unsigned int width, rend_t efs) const NOTHROW
{
  if (!l.t)
    {
      lalloc (l);
      col = 0;
      width = ncol;
    }

  l.touch ();

  efs &= ~RS_baseattrMask; // remove italic etc. fontstyles
  efs = SET_FONT (efs, FONTSET (efs)->find_font (' '));

  text_t *et = l.t + col;
  rend_t *er = l.r + col;

  while (width--)
    {
      *et++ = ' ';
      *er++ = efs;
    }
}

/* ------------------------------------------------------------------------- */
/* Fill a full line with blanks - make sure it is allocated first */
void
rxvt_term::scr_blank_screen_mem (line_t &l, rend_t efs) const NOTHROW
{
  scr_blank_line (l, 0, ncol, efs);

  l.l = 0;
  l.f = 0;
}

// nuke a single wide character at the given column
void
rxvt_term::scr_kill_char (line_t &l, int col) const NOTHROW
{
  // find begin
  while (col > 0 && l.t[col] == NOCHAR)
    col--;

  rend_t rend = l.r[col] & ~RS_baseattrMask;
  rend = SET_FONT (rend, FONTSET (rend)->find_font (' '));

  l.touch ();

  // found start, nuke
  do {
    l.t[col] = ' ';
    l.r[col] = rend;
    col++;
  } while (col < ncol && l.t[col] == NOCHAR);
}

/* ------------------------------------------------------------------------- *
 *                          SCREEN INITIALISATION                            *
 * ------------------------------------------------------------------------- */

void
rxvt_term::scr_reset ()
{
#if ENABLE_OVERLAY
  scr_overlay_off ();
#endif

  view_start = 0;
  num_scr = 0;

  if (ncol == 0)
    ncol = 80;

  if (nrow == 0)
    nrow = 24;

  if (ncol == prev_ncol && nrow == prev_nrow)
    return;

  // we need at least two lines for wrapping to work correctly
  while (nrow + saveLines < 2)
    {
      //TODO//FIXME
      saveLines++;
      prev_nrow--;
      top_row--;
    }

  want_refresh = 1;

  int prev_total_rows = prev_nrow + saveLines;
  total_rows = nrow + saveLines;

  screen.tscroll = 0;
  screen.bscroll = nrow - 1;

  if (!row_buf)
    {
      /*
       * first time called so just malloc everything: don't rely on realloc
       */
      top_row    = 0;
      term_start = 0;

      talloc = new rxvt_salloc (ncol * sizeof (text_t));
      ralloc = new rxvt_salloc (ncol * sizeof (rend_t));

      row_buf   = (line_t *)rxvt_calloc (total_rows       , sizeof (line_t));
      drawn_buf = (line_t *)rxvt_calloc (nrow             , sizeof (line_t));
      swap_buf  = (line_t *)rxvt_calloc (nrow             , sizeof (line_t));

      for (int row = nrow; row--; )
        {
          scr_blank_screen_mem (ROW (row), DEFAULT_RSTYLE);
          scr_blank_screen_mem (swap_buf [row], DEFAULT_RSTYLE);
          scr_blank_screen_mem (drawn_buf[row], DEFAULT_RSTYLE);
        }

      memset (charsets, 'B', sizeof (charsets));
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
      selection.clip_text = NULL;
      selection.clip_len = 0;
    }
  else
    {
      /*
       * add or delete rows as appropriate
       */

      rxvt_salloc *old_ta = talloc; talloc = new rxvt_salloc (ncol * sizeof (text_t));
      rxvt_salloc *old_ra = ralloc; ralloc = new rxvt_salloc (ncol * sizeof (rend_t));

#if 0
      if (nrow < prev_nrow)
        {
          for (int row = nrow; row < prev_nrow; row++)
            {
              lfree (swap_buf [row]);
              lfree (drawn_buf[row]);
            }
        }
#endif

      drawn_buf = (line_t *)rxvt_realloc (drawn_buf, nrow * sizeof (line_t));
      swap_buf  = (line_t *)rxvt_realloc (swap_buf , nrow * sizeof (line_t));

      for (int row = min (nrow, prev_nrow); row--; )
        {
          lresize (drawn_buf[row]);
          lresize (swap_buf [row]);
        }

      for (int row = prev_nrow; row < nrow; row++)
        {
          swap_buf [row].clear (); scr_blank_screen_mem (swap_buf [row], DEFAULT_RSTYLE);
          drawn_buf[row].clear (); scr_blank_screen_mem (drawn_buf[row], DEFAULT_RSTYLE);
        }

      line_t *old_buf = row_buf;
      row_buf = (line_t *)rxvt_calloc (total_rows, sizeof (line_t));

      int p    = MOD (term_start + prev_nrow, prev_total_rows);  // previous row
      int pend = MOD (term_start + top_row  , prev_total_rows);
      int q    = total_rows; // rewrapped row

      if (top_row)
        {
          // Re-wrap lines. This is rather ugly, possibly because I am too dumb
          // to come up with a lean and mean algorithm.
          // TODO: maybe optimise when width didn't change

          row_col_t ocur = screen.cur;
          ocur.row = MOD (term_start + ocur.row, prev_total_rows);

          do
            {
              p = MOD (p - 1, prev_total_rows);
              assert (old_buf [MOD (p, prev_total_rows)].t);
              int plines = 1;
              int llen = old_buf [MOD (p, prev_total_rows)].l;

              while (p != pend && old_buf [MOD (p - 1, prev_total_rows)].is_longer ())
                {
                  p = MOD (p - 1, prev_total_rows);

                  plines++;
                  llen += prev_ncol;
                }

              int qlines = max (0, (llen - 1) / ncol) + 1;

              // drop partial lines completely
              if (q < qlines)
                break;

              q -= qlines;

              int lofs = 0;
              line_t *qline;

              // re-assemble the full line by destination lines
              for (int qrow = q; qlines--; qrow++)
                {
                  qline = row_buf + qrow;
                  lalloc (*qline);
                  qline->l = ncol;
                  qline->is_longer (1);

                  int qcol = 0;

                  // see below for cursor adjustment rationale
                  if (p == ocur.row)
                    screen.cur.row = q - (total_rows - nrow);

                  // fill a single destination line
                  while (lofs < llen && qcol < ncol)
                    {
                      int prow = lofs / prev_ncol;
                      int pcol = lofs % prev_ncol;

                      prow = MOD (p + prow, prev_total_rows);

                      // we only adjust the cursor _row_ and put it into
                      // the topmost line of "long line" it was in, as
                      // this seems to upset applications/shells/readline
                      // least.
                      if (prow == ocur.row)
                        screen.cur.row = q - (total_rows - nrow);

                      line_t &pline = old_buf [prow];

                      int len = min (min (prev_ncol - pcol, ncol - qcol), llen - lofs);

                      memcpy (qline->t + qcol, pline.t + pcol, len * sizeof (text_t));
                      memcpy (qline->r + qcol, pline.r + pcol, len * sizeof (rend_t));

                      lofs += len;
                      qcol += len;
                    }
                }

              qline->l = llen ? MOD (llen - 1, ncol) + 1 : 0;
              qline->is_longer (0);
              scr_blank_line (*qline, qline->l, ncol - qline->l, DEFAULT_RSTYLE);
            }
          while (p != pend && q > 0);

          term_start = total_rows - nrow;
          top_row = q - term_start;

          // make sure all terminal lines exist
          while (top_row > 0)
            scr_blank_screen_mem (ROW (--top_row), DEFAULT_RSTYLE);
        }
      else
        {
          // if no scrollback exists (yet), wing, instead of wrap

          for (int row = min (nrow, prev_nrow); row--; )
            {
              line_t &pline = old_buf [MOD (term_start + row, prev_total_rows)];
              line_t &qline = row_buf [row];

              qline = pline;
              lresize (qline);
            }

          for (int row = prev_nrow; row < nrow; row++)
            {
              row_buf [row].clear (); scr_blank_screen_mem (row_buf [row], DEFAULT_RSTYLE);
            }

          term_start = 0;
        }

      free (old_buf);
      delete old_ta;
      delete old_ra;

      clamp_it (screen.cur.row, 0, nrow - 1);
      clamp_it (screen.cur.col, 0, ncol - 1);
    }

  free (tabs);
  tabs = (char *)rxvt_malloc (ncol);

  for (int col = ncol; col--; )
    tabs [col] = col % TABSIZE == 0;

  CLEAR_ALL_SELECTION ();

  prev_nrow = nrow;
  prev_ncol = ncol;

  tt_winch ();

  HOOK_INVOKE ((this, HOOK_RESET, DT_END));
}

/* ------------------------------------------------------------------------- */
/*
 * Free everything.  That way malloc debugging can find leakage.
 */
void
rxvt_term::scr_release () NOTHROW
{
  if (row_buf)
    {
      delete talloc; talloc = 0;
      delete ralloc; ralloc = 0;

      free (row_buf);
      free (swap_buf);
      free (drawn_buf);
      row_buf = 0; // signal that we freed all the arrays above

      free (tabs);
      tabs = 0;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Hard/Soft reset
 */
void
rxvt_term::scr_poweron ()
{
  scr_release ();
  prev_nrow = prev_ncol = 0;
  rvideo_mode = false;
  scr_soft_reset ();
  scr_reset ();

  scr_clear (true);
  scr_refresh ();
}

void
rxvt_term::scr_soft_reset () NOTHROW
{
  /* only affects modes, nothing drastic such as clearing the screen */
#if ENABLE_OVERLAY
  scr_overlay_off ();
#endif

  if (current_screen != PRIMARY)
    scr_swap_screen ();

  scr_scroll_region (0, MAX_ROWS - 1);
  scr_rendition (0, ~RS_None);
  scr_insert_mode (0);
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
rxvt_term::scr_cursor (cursor_mode mode) NOTHROW
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
  min_it (s->cur.row, nrow - 1);
  min_it (s->cur.col, ncol - 1);
  assert (s->cur.row >= 0);
  assert (s->cur.col >= 0);
}

void
rxvt_term::scr_swap_screen () NOTHROW
{
  if (!option (Opt_secondaryScreen))
    return;

  for (int i = prev_nrow; i--; )
    ::swap (ROW(i), swap_buf [i]);

  ::swap (screen.cur, swap.cur);

  screen.cur.row = clamp (screen.cur.row, 0, prev_nrow - 1);
  screen.cur.col = clamp (screen.cur.col, 0, prev_ncol - 1);
}

/* ------------------------------------------------------------------------- */
/*
 * Swap between primary and secondary screens
 * XTERM_SEQ: Primary screen  : ESC [ ? 4 7 h
 * XTERM_SEQ: Secondary screen: ESC [ ? 4 7 l
 */
void
rxvt_term::scr_change_screen (int scrn)
{
  if (scrn == current_screen)
    return;

  want_refresh = 1;
  view_start = 0;

  /* check for boundary cross */
  row_col_t pos;
  pos.row = pos.col = 0;
  if (ROWCOL_IS_BEFORE (selection.beg, pos)
      && ROWCOL_IS_AFTER (selection.end, pos))
    CLEAR_SELECTION ();

  current_screen = scrn;

#if NSCREENS
  if (option (Opt_secondaryScreen))
    {
      num_scr = 0;

      scr_swap_screen ();

      ::swap (screen.charset, swap.charset);
      ::swap (screen.flags,   swap.flags);
      screen.flags |= Screen_VisibleCursor;
      swap.flags   |= Screen_VisibleCursor;
    }
  else
#endif
    if (option (Opt_secondaryScroll))
      scr_scroll_text (0, prev_nrow - 1, prev_nrow);
}

// clear WrapNext indicator, solidifying position on next line
void
rxvt_term::scr_do_wrap () NOTHROW
{
  if (!(screen.flags & Screen_WrapNext))
    return;

  screen.flags &= ~Screen_WrapNext;

  screen.cur.col = 0;

  if (screen.cur.row == screen.bscroll)
    scr_scroll_text (screen.tscroll, screen.bscroll, 1);
  else if (screen.cur.row < nrow - 1)
    screen.cur.row++;
}

/* ------------------------------------------------------------------------- */
/*
 * Change the colour for following text
 */
void
rxvt_term::scr_color (unsigned int color, int fgbg) NOTHROW
{
  if (!IN_RANGE_INC (color, minCOLOR, maxTermCOLOR))
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
rxvt_term::scr_rendition (int set, int style) NOTHROW
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
 */
int
rxvt_term::scr_scroll_text (int row1, int row2, int count) NOTHROW
{
  if (count == 0 || (row1 > row2))
    return 0;

  want_refresh = 1;
  num_scr += count;

  if (count > 0
      && row1 == 0
      && (current_screen == PRIMARY || option (Opt_secondaryScroll)))
    {
      min_it (count, total_rows - (nrow - (row2 + 1)));

      top_row = max (top_row - count, -saveLines);

      // sever bottommost line
      {
        line_t &l = ROW(row2);
        l.is_longer (0);
        l.touch ();
      }

      // scroll everything up 'count' lines
      term_start = (term_start + count) % total_rows;

      // now copy lines below the scroll region bottom to the
      // bottom of the screen again, so they look as if they
      // hadn't moved.
      for (int i = nrow; --i > row2; )
        {
          line_t &l1 = ROW(i - count);
          line_t &l2 = ROW(i);

          ::swap (l1, l2);
          l2.touch ();
        }

      // erase newly scrolled-in lines
      for (int i = count; i--; )
        {
          line_t &l = ROW(row2 - i);

          // optimise if already cleared, can be significant on slow machines
          // could be rolled into scr_blank_screen_mem
          if (l.r && l.l < ncol - 1 && !((l.r[l.l + 1] ^ rstyle) & (RS_fgMask | RS_bgMask)))
            {
              scr_blank_line (l, 0, l.l, rstyle);
              l.l = 0;
              l.f = 0;
            }
          else
            scr_blank_screen_mem (l, rstyle);
        }

      // move and/or clear selection, if any
      if (selection.op && current_screen == selection.screen
          && selection.beg.row <= row2)
        {
          selection.beg.row  -= count;
          selection.end.row  -= count;
          selection.mark.row -= count;

          selection_check (0);
        }

      // finally move the view window, if desired
      if (option (Opt_scrollWithBuffer)
          && view_start != 0
          && view_start != -saveLines)
        scr_page (UP, count);

      if (SHOULD_INVOKE (HOOK_SCROLL_BACK))
        HOOK_INVOKE ((this, HOOK_SCROLL_BACK, DT_INT, count, DT_INT, top_row, DT_END));
    }
  else
    {
      if (selection.op && current_screen == selection.screen)
        {
          if ((selection.beg.row < row1 && selection.end.row > row1)
              || (selection.beg.row < row2 && selection.end.row > row2)
              || (selection.beg.row - count < row1 && selection.beg.row >= row1)
              || (selection.beg.row - count > row2 && selection.beg.row <= row2)
              || (selection.end.row - count < row1 && selection.end.row >= row1)
              || (selection.end.row - count > row2 && selection.end.row <= row2))
            {
              CLEAR_ALL_SELECTION ();
              selection.op = SELECTION_CLEAR;
            }
          else if (selection.end.row >= row1 && selection.end.row <= row2)
            {
              /* move selected region too */
              selection.beg.row  -= count;
              selection.end.row  -= count;
              selection.mark.row -= count;

              selection_check (0);
            }
        }

      // use a simple and robust scrolling algorithm, this
      // part of scr_scroll_text is not time-critical.

      // sever line above scroll region
      if (row1)
        {
          line_t &l = ROW(row1 - 1);
          l.is_longer (0);
          l.touch ();
        }

      int rows = row2 - row1 + 1;

      min_it (count, rows);

      line_t *temp_buf = rxvt_temp_buf<line_t> (rows);

      for (int row = 0; row < rows; row++)
        {
          temp_buf [row] = ROW(row1 + (row + count + rows) % rows);

          if (!IN_RANGE_EXC (row + count, 0, rows))
            scr_blank_screen_mem (temp_buf [row], rstyle);
        }

      for (int row = 0; row < rows; row++)
        ROW(row1 + row) = temp_buf [row];

      // sever bottommost line
      {
        line_t &l = ROW(row2);
        l.is_longer (0);
        l.touch ();
      }
    }

  return count;
}

/* ------------------------------------------------------------------------- */
/*
 * Add text given in <str> of length <len> to screen struct
 */
void
rxvt_term::scr_add_lines (const wchar_t *str, int len, int minlines) NOTHROW
{
  if (len <= 0)               /* sanity */
    return;

  bool checksel;
  unicode_t c;
  int ncol = this->ncol;
  const wchar_t *strend = str + len;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  if (minlines > 0)
    {
      minlines += screen.cur.row - screen.bscroll;
      min_it (minlines, screen.cur.row - top_row);

      if (minlines > 0
          && screen.tscroll == 0
          && screen.bscroll == nrow - 1)
        {
          /* _at least_ this many lines need to be scrolled */
          scr_scroll_text (screen.tscroll, screen.bscroll, minlines);
          screen.cur.row -= minlines;
        }
    }

  assert (screen.cur.col < ncol);
  assert (screen.cur.row < nrow
          && screen.cur.row >= top_row);
  int row = screen.cur.row;

  checksel = selection.op && current_screen == selection.screen ? 1 : 0;

  line_t *line = &ROW(row);

  while (str < strend)
    {
      c = (unicode_t)*str++; // convert to rxvt-unicodes representation

      if (ecb_unlikely (c < 0x20))
        if (c == C0_LF)
          {
            max_it (line->l, screen.cur.col);

            screen.flags &= ~Screen_WrapNext;

            if (screen.cur.row == screen.bscroll)
              scr_scroll_text (screen.tscroll, screen.bscroll, 1);
            else if (screen.cur.row < (nrow - 1))
              row = ++screen.cur.row;

            line = &ROW(row);  /* _must_ refresh */
            continue;
          }
        else if (c == C0_CR)
          {
            max_it (line->l, screen.cur.col);

            screen.flags &= ~Screen_WrapNext;
            screen.cur.col = 0;
            continue;
          }
        else if (c == C0_HT)
          {
            scr_tab (1, true);
            continue;
          }

      if (ecb_unlikely (
            checksel            /* see if we're writing within selection */
            && !ROWCOL_IS_BEFORE (screen.cur, selection.beg)
            && ROWCOL_IS_BEFORE (screen.cur, selection.end)
         ))
        {
          checksel = 0;
          /*
           * If we wrote anywhere in the selected area, kill the selection
           * XXX: should we kill the mark too?  Possibly, but maybe that
           *      should be a similar check.
           */
          CLEAR_SELECTION ();
        }

      if (ecb_unlikely (screen.flags & Screen_WrapNext))
        {
          scr_do_wrap ();

          line->l = ncol;
          line->is_longer (1);

          row = screen.cur.row;
          line = &ROW(row);   /* _must_ refresh */
        }

      // some utf-8 decoders "decode" surrogate characters: let's fix this.
      if (ecb_unlikely (IN_RANGE_INC (c, 0xd800, 0xdfff)))
        c = 0xfffd;

      // rely on wcwidth to tell us the character width, do wcwidth before
      // further replacements, as wcwidth might return -1 for the line
      // drawing characters below as they might be invalid in the current
      // locale.
      int width = WCWIDTH (c);

      if (ecb_unlikely (charsets [screen.charset] == '0')) // DEC SPECIAL
        {
          // vt100 special graphics and line drawing
          // 5f-7e standard vt100
          // 40-5e rxvt extension for extra curses acs chars
          static uint16_t vt100_0[62] = { // 41 .. 7e
                    0x2191, 0x2193, 0x2192, 0x2190, 0x2588, 0x259a, 0x2603, // 41-47 hi mr. snowman!
                 0,      0,      0,      0,      0,      0,      0,      0, // 48-4f
                 0,      0,      0,      0,      0,      0,      0,      0, // 50-57
                 0,      0,      0,      0,      0,      0,      0, 0x0020, // 58-5f
            0x25c6, 0x2592, 0x2409, 0x240c, 0x240d, 0x240a, 0x00b0, 0x00b1, // 60-67
            0x2424, 0x240b, 0x2518, 0x2510, 0x250c, 0x2514, 0x253c, 0x23ba, // 68-6f
            0x23bb, 0x2500, 0x23bc, 0x23bd, 0x251c, 0x2524, 0x2534, 0x252c, // 70-77
            0x2502, 0x2264, 0x2265, 0x03c0, 0x2260, 0x00a3, 0x00b7,         // 78-7e
          };

          if (c >= 0x41 && c <= 0x7e && vt100_0[c - 0x41])
            {
              c = vt100_0[c - 0x41];
              width = 1; // vt100 line drawing characters are always single-width
            }
        }

      if (ecb_unlikely (screen.flags & Screen_Insert))
        scr_insdel_chars (width, INSERT);

      if (width != 0)
        {
#if !UNICODE_3
          // trim characters we can't store directly :(
          if (c >= 0x10000)
# if ENABLE_COMBINING
            c = rxvt_composite.compose (c); // map to lower 16 bits
# else
            c = 0xfffd;
# endif
#endif

          rend_t rend = SET_FONT (rstyle, FONTSET (rstyle)->find_font (c));

          // if the character doesn't fit into the remaining columns...
          if (ecb_unlikely (screen.cur.col > ncol - width && ncol >= width))
            {
              if (screen.flags & Screen_Autowrap)
                {
                  // ... artificially enlargen the previous one
                  c = NOCHAR;
                  // and try the same character next loop iteration
                  --str;
                }
              else
                screen.cur.col = ncol - width;
            }

          // nuke the character at this position, if required
          // due to wonderful coincidences everywhere else in this loop
          // we never have to check for overwriting a wide char itself,
          // only its tail.
          if (ecb_unlikely (line->t[screen.cur.col] == NOCHAR))
            scr_kill_char (*line, screen.cur.col);

          line->touch ();

          do
            {
              line->t[screen.cur.col] = c;
              line->r[screen.cur.col] = rend;

              if (ecb_likely (screen.cur.col < ncol - 1))
                screen.cur.col++;
              else
                {
                  line->l = ncol;
                  if (screen.flags & Screen_Autowrap)
                    screen.flags |= Screen_WrapNext;

                  goto end_of_line;
                }

              c = NOCHAR;
            }
          while (ecb_unlikely (--width > 0));

          // pad with spaces when overwriting wide character with smaller one
          for (int c = screen.cur.col; ecb_unlikely (c < ncol && line->t[c] == NOCHAR); c++)
            {
              line->t[c] = ' ';
              line->r[c] = rend;
            }

end_of_line:
          ;
        }
#if ENABLE_COMBINING
      else // width == 0
        {
          if (c != 0xfeff) // ignore BOM
            {
              // handle combining characters
              // we just tag the accent on the previous on-screen character.
              // this is arguably not correct, but also arguably not wrong.
              // we don't handle double-width characters nicely yet.
              line_t *linep;
              text_t *tp;
              rend_t *rp;

              if (screen.cur.col > 0)
                {
                  linep = line;
                  tp = line->t + screen.cur.col - 1;
                  rp = line->r + screen.cur.col - 1;
                }
              else if (screen.cur.row > 0
                       && ROW(screen.cur.row - 1).is_longer ())
                {
                  linep = &ROW(screen.cur.row - 1);
                  tp = line->t + ncol - 1;
                  rp = line->r + ncol - 1;
                }
              else
                continue;

              linep->touch ();

              while (*tp == NOCHAR && tp > linep->t)
                tp--, rp--;

              // first try to find a precomposed character
              unicode_t n = rxvt_compose (*tp, c);
              if (n == NOCHAR)
                n = rxvt_composite.compose (*tp, c);

              *tp = n;
              *rp = SET_FONT (*rp, FONTSET (*rp)->find_font (*tp));
            }
        }
#endif
    }

  max_it (line->l, screen.cur.col);

  assert (screen.cur.row >= 0);
}

/* ------------------------------------------------------------------------- */
/*
 * Process Backspace.  Move back the cursor back a position, wrap if have to
 * XTERM_SEQ: CTRL-H
 */
void
rxvt_term::scr_backspace () NOTHROW
{
  if (screen.cur.col == 0)
    {
      if (screen.cur.row > 0)
        {
#ifdef TERMCAP_HAS_BW
          screen.cur.col = ncol - 1;
          --screen.cur.row;

          want_refresh = 1;
#endif
        }
    }
  else if (screen.flags & Screen_WrapNext)
    screen.flags &= ~Screen_WrapNext;
  else
    scr_gotorc (0, -1, RELATIVE);
}

/* ------------------------------------------------------------------------- */
/*
 * Process Horizontal Tab
 * count: +ve = forward; -ve = backwards
 * XTERM_SEQ: CTRL-I
 */
void
rxvt_term::scr_tab (int count, bool ht) NOTHROW
{
  int i, x;

  want_refresh = 1;
  i = x = screen.cur.col;

  if (count == 0)
    return;
  else if (count > 0)
    {
      line_t &l = ROW(screen.cur.row);
      rend_t base_rend = l.r[i];
      ht &= l.t[i] == ' ';

      for (; ++i < ncol; )
        if (tabs[i])
          {
            x = i;

            if (!--count)
              break;
          }
        else
          ht &= l.t[i] == ' '
                && RS_SAME (l.r[i], base_rend);

      if (count)
        x = ncol - 1;

      // store horizontal tab commands as characters inside the text
      // buffer so they can be selected and pasted.
      if (ht && option (Opt_pastableTabs))
        {
          base_rend = SET_FONT (base_rend, 0);

          l.touch (x);

          i = screen.cur.col;

          l.t[i] = '\t';
          l.r[i] = base_rend;

          while (++i < x)
            {
              l.t[i] = NOCHAR;
              l.r[i] = base_rend;
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
#if !ENABLE_MINIMAL
void
rxvt_term::scr_backindex () NOTHROW
{
  if (screen.cur.col > 0)
    scr_gotorc (0, -1, R_RELATIVE | C_RELATIVE);
  else
    scr_insdel_chars (1, INSERT);
}
#endif
/* ------------------------------------------------------------------------- */
/*
 * Process DEC Forward Index
 * XTERM_SEQ: ESC 9
 * Move cursor right in row.  If we're at the right boundary, shift everything
 * in that row left.  Clear right column.
 */
#if !ENABLE_MINIMAL
void
rxvt_term::scr_forwardindex () NOTHROW
{
  if (screen.cur.col < ncol - 1)
    scr_gotorc (0, 1, R_RELATIVE | C_RELATIVE);
  else
    {
      line_t &l = ROW(screen.cur.row);

      l.touch ();
      l.is_longer (0);

      scr_gotorc (0, 0, R_RELATIVE);
      scr_insdel_chars (1, DELETE);
      scr_gotorc (0, ncol - 1, R_RELATIVE);
    }
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Goto Row/Column
 */
void
rxvt_term::scr_gotorc (int row, int col, int relative) NOTHROW
{
  want_refresh = 1;
  ZERO_SCROLLBACK ();

  screen.cur.col = relative & C_RELATIVE ? screen.cur.col + col : col;
  clamp_it (screen.cur.col, 0, ncol - 1);

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
        {
          /* relative origin mode */
          screen.cur.row = row + screen.tscroll;
          min_it (screen.cur.row, screen.bscroll);
        }
      else
        screen.cur.row = row;
    }

  clamp_it (screen.cur.row, 0, nrow - 1);
}

/* ------------------------------------------------------------------------- */
/*
 * direction should be UP or DN
 */
void
rxvt_term::scr_index (enum page_dirn direction) NOTHROW
{
  int dirn;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  dirn = ((direction == UP) ? 1 : -1);

  screen.flags &= ~Screen_WrapNext;

  if ((screen.cur.row == screen.bscroll && direction == UP)
      || (screen.cur.row == screen.tscroll && direction == DN))
    scr_scroll_text (screen.tscroll, screen.bscroll, dirn);
  else
    screen.cur.row += dirn;

  clamp_it (screen.cur.row, 0, nrow - 1);
  selection_check (0);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part or whole of a line
 * XTERM_SEQ: Clear line to right: ESC [ 0 K
 * XTERM_SEQ: Clear line to left : ESC [ 1 K
 * XTERM_SEQ: Clear whole line   : ESC [ 2 K
 * extension: clear to right unless wrapped: ESC [ 3 K
 */
void
rxvt_term::scr_erase_line (int mode) NOTHROW
{
  unsigned int col, num;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  selection_check (1);

  line_t &line = ROW(screen.cur.row);

  line.touch ();
  line.is_longer (0);

  switch (mode)
    {
      case 3:
        if (screen.flags & Screen_WrapNext)
          return;

        /* fall through */

      case 0:                     /* erase to end of line */
        col = screen.cur.col;
        num = ncol - col;
        min_it (line.l, col);

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
        num = ncol;
        line.l = 0;
        if (selection.beg.row <= screen.cur.row
            && selection.end.row >= screen.cur.row)
          CLEAR_SELECTION ();
        break;
      default:
        return;
    }

  scr_blank_line (line, col, num, rstyle);
}

/* ------------------------------------------------------------------------- */
/*
 * Erase part or whole of the screen
 * XTERM_SEQ: Clear screen after cursor : ESC [ 0 J
 * XTERM_SEQ: Clear screen before cursor: ESC [ 1 J
 * XTERM_SEQ: Clear whole screen        : ESC [ 2 J
 */
void
rxvt_term::scr_erase_screen (int mode) NOTHROW
{
  int num;
  int32_t row;
  rend_t ren;
  XGCValues gcvalue;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  switch (mode)
    {
      case 0:                     /* erase to end of screen */
        scr_erase_line (0);
        row = screen.cur.row + 1;    /* possible OOB */
        num = nrow - row;
        break;
      case 1:                     /* erase to beginning of screen */
        scr_erase_line (1);
        row = 0;
        num = screen.cur.row;
        break;
      case 2:                     /* erase whole screen */
        row = 0;
        num = nrow;
        break;
      default:
        return;
    }

  if (selection.op && current_screen == selection.screen
      && ((selection.beg.row >= row && selection.beg.row <= row + num)
          || (selection.end.row >= row
              && selection.end.row <= row + num)))
    CLEAR_SELECTION ();

  if (row >= nrow) /* Out Of Bounds */
    return;

  min_it (num, nrow - row);

  if (rstyle & (RS_Blink | RS_RVid | RS_Uline))
    ren = (rend_t) ~RS_None;
  else if (GET_BASEBG (rstyle) == Color_bg)
    {
      ren = DEFAULT_RSTYLE;

      if (mapped)
        XClearArea (dpy, vt, 0,
                    Row2Pixel (row - view_start), (unsigned int)vt_width,
                    (unsigned int)Height2Pixel (num), False);
    }
  else
    {
      ren = rstyle & (RS_fgMask | RS_bgMask);

      if (mapped)
        {
          gcvalue.foreground = pix_colors[bgcolor_of (rstyle)];
          XChangeGC (dpy, gc, GCForeground, &gcvalue);
          XFillRectangle (dpy, vt, gc,
                          0, Row2Pixel (row - view_start),
                          (unsigned int)vt_width,
                          (unsigned int)Height2Pixel (num));
          gcvalue.foreground = pix_colors[Color_fg];
          XChangeGC (dpy, gc, GCForeground, &gcvalue);
        }
    }

  for (; num--; row++)
    {
      scr_blank_screen_mem (ROW(row), rstyle);

      if (row - view_start < nrow)
        scr_blank_line (drawn_buf [row - view_start], 0, ncol, ren);
    }
}

#if !ENABLE_MINIMAL
void
rxvt_term::scr_erase_savelines () NOTHROW
{
  want_refresh = 1;
  ZERO_SCROLLBACK ();

  top_row = 0;
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Fill the screen with `E's
 * XTERM_SEQ: Screen Alignment Test: ESC # 8
 */
void
rxvt_term::scr_E () NOTHROW
{
  rend_t fs;

  want_refresh = 1;
  ZERO_SCROLLBACK ();

  num_scr_allow = 0;

  row_col_t pos;
  pos.row = pos.col = 0;
  if (ROWCOL_IS_AFTER (selection.end, pos))
    CLEAR_SELECTION ();

  fs = SET_FONT (rstyle, FONTSET (rstyle)->find_font ('E'));
  for (int row = nrow; row--; )
    {
      line_t &line = ROW(row);

      fill_text (line.t, 'E', ncol);
      rend_t *r1 = line.r;

      for (int j = ncol; j--; )
        *r1++ = fs;

      line.is_longer (0);
      line.touch (ncol);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> lines
 */
void
rxvt_term::scr_insdel_lines (int count, int insdel) NOTHROW
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

  scr_scroll_text (screen.cur.row, screen.bscroll, insdel * count);
}

/* ------------------------------------------------------------------------- */
/*
 * Insert/Delete <count> characters from the current position
 */
void
rxvt_term::scr_insdel_chars (int count, int insdel) NOTHROW
{
  want_refresh = 1;
  ZERO_SCROLLBACK ();

  if (count <= 0)
    return;

  scr_do_wrap ();

  selection_check (1);
  min_it (count, ncol - screen.cur.col);

  int row = screen.cur.row;

  line_t *line = &ROW(row);

  line->touch ();
  line->is_longer (0);

  // nuke wide spanning the start
  if (line->t[screen.cur.col] == NOCHAR)
    scr_kill_char (*line, screen.cur.col);

  switch (insdel)
    {
      case INSERT:
        line->l = min (line->l + count, ncol);

        if (line->t[screen.cur.col] == NOCHAR)
          scr_kill_char (*line, screen.cur.col);

        for (int col = ncol - 1; (col - count) >= screen.cur.col; col--)
          {
            line->t[col] = line->t[col - count];
            line->r[col] = line->r[col - count];
          }

        if (selection.op && current_screen == selection.screen
            && ROWCOL_IN_ROW_AT_OR_AFTER (selection.beg, screen.cur))
          {
            if (selection.end.row != screen.cur.row
                || (selection.end.col + count >= ncol))
              CLEAR_SELECTION ();
            else
              {
                /* shift selection */
                selection.beg.col  += count;
                selection.mark.col += count; /* XXX: yes? */
                selection.end.col  += count;
              }
          }

        scr_blank_line (*line, screen.cur.col, count, rstyle);
        break;

      case ERASE:
        screen.cur.col += count;     /* don't worry if > ncol */
        selection_check (1);
        screen.cur.col -= count;

        // nuke wide char after the end
        if (screen.cur.col + count < ncol && line->t[screen.cur.col + count] == NOCHAR)
          scr_kill_char (*line, screen.cur.col + count);

        scr_blank_line (*line, screen.cur.col, count, rstyle);
        break;

      case DELETE:
        line->l = max (line->l - count, 0);

        // nuke wide char spanning the end
        if (screen.cur.col + count < ncol && line->t[screen.cur.col + count] == NOCHAR)
          scr_kill_char (*line, screen.cur.col + count);

        for (int col = screen.cur.col; (col + count) < ncol; col++)
          {
            line->t[col] = line->t[col + count];
            line->r[col] = line->r[col + count];
          }

        scr_blank_line (*line, ncol - count, count, rstyle);

        if (selection.op && current_screen == selection.screen
            && ROWCOL_IN_ROW_AT_OR_AFTER (selection.beg, screen.cur))
          {
            if (selection.end.row != screen.cur.row
                || (screen.cur.col >= selection.beg.col - count)
                || selection.end.col >= ncol)
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
rxvt_term::scr_scroll_region (int top, int bot) NOTHROW
{
  max_it (top, 0);
  min_it (bot, nrow - 1);

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
rxvt_term::scr_cursor_visible (int mode) NOTHROW
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
rxvt_term::scr_autowrap (int mode) NOTHROW
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
rxvt_term::scr_relative_origin (int mode) NOTHROW
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
rxvt_term::scr_insert_mode (int mode) NOTHROW
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
rxvt_term::scr_set_tab (int mode) NOTHROW
{
  if (mode < 0)
    memset (tabs, 0, ncol);
  else if (screen.cur.col < ncol)
    tabs [screen.cur.col] = !!mode;
}

/* ------------------------------------------------------------------------- */
/*
 * Set reverse/normal video
 * XTERM_SEQ: Reverse video: ESC [ ? 5 h
 * XTERM_SEQ: Normal video : ESC [ ? 5 l
 */
void
rxvt_term::scr_rvideo_mode (bool on) NOTHROW
{
  rvideo_mode = on;

#ifndef NO_BELL
  on ^= rvideo_bell;
#endif

  if (rvideo_state != on)
    {
      rvideo_state = on;

      ::swap (pix_colors[Color_fg], pix_colors[Color_bg]);
#ifdef HAVE_BG_PIXMAP
      if (bg_pixmap == None)
#endif
          XSetWindowBackground (dpy, vt, pix_colors[Color_bg]);

      XGCValues gcvalue;
      gcvalue.foreground = pix_colors[Color_fg];
      gcvalue.background = pix_colors[Color_bg];
      XChangeGC (dpy, gc, GCBackground | GCForeground, &gcvalue);

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
rxvt_term::scr_report_position () NOTHROW
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
rxvt_term::set_font_style () NOTHROW
{
#if 0
  switch (charsets [screen.charset])
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
#endif
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
rxvt_term::scr_charset_choose (int set) NOTHROW
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
rxvt_term::scr_charset_set (int set, unsigned int ch) NOTHROW
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
rxvt_term::scr_refresh_rend (rend_t mask, rend_t value) NOTHROW
{
  bool found = false;

  for (int i = 0; i < nrow; i++)
    {
      rend_t *drp = drawn_buf[i].r;

      for (int col = 0; col < ncol; col++, drp++)
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
rxvt_term::scr_expose (int x, int y, int ewidth, int eheight, bool refresh) NOTHROW
{
  int i;
  row_col_t rc[RC_COUNT];

  if (!drawn_buf)  /* sanity check */
    return;

#ifndef NO_SLOW_LINK_SUPPORT
  if (refresh_type == FAST_REFRESH && !display->is_local)
    {
      y = 0;
      eheight = height;
    }
#endif

  /* round down */
  rc[PART_BEG].col = Pixel2Col (x);
  rc[PART_BEG].row = Pixel2Row (y);
  /* round up */
  rc[PART_END].col = Pixel2Width (x + ewidth  + fwidth  - 1);
  rc[PART_END].row = Pixel2Row   (y + eheight + fheight - 1);

  /* sanity checks */
  for (i = PART_BEG; i < RC_COUNT; i++)
    {
      min_it (rc[i].col, ncol - 1);
      min_it (rc[i].row, nrow - 1);
    }

  for (i = rc[PART_BEG].row; i <= rc[PART_END].row; i++)
    fill_text (&drawn_buf[i].t[rc[PART_BEG].col], 0, rc[PART_END].col - rc[PART_BEG].col + 1);

  num_scr_allow = 0;

  if (refresh)
    scr_refresh ();
}

/* ------------------------------------------------------------------------- */
/*
 * Refresh the entire screen
 */
void
rxvt_term::scr_touch (bool refresh) NOTHROW
{
  scr_expose (0, 0, vt_width, vt_height, refresh);
}

/* ------------------------------------------------------------------------- */
/*
 * Move the display so that the line represented by scrollbar value Y is at
 * the top of the screen
 */
void
rxvt_term::scr_move_to (int y, int len) NOTHROW
{
  scr_changeview ((top_row - nrow) * (len - y) / len + (nrow - 1));
}

/* ------------------------------------------------------------------------- */
/*
 * Page the screen up/down nlines
 * direction should be UP or DN
 */
bool
rxvt_term::scr_page (enum page_dirn direction, int nlines) NOTHROW
{
  int new_view_start =
    direction == UP ? view_start - nlines
                    : view_start + nlines;

  return scr_changeview (new_view_start);
}

bool
rxvt_term::scr_changeview (int new_view_start) NOTHROW
{
  clamp_it (new_view_start, top_row, 0);

  if (new_view_start == view_start)
    return false;

  num_scr += new_view_start - view_start;
  view_start = new_view_start;
  want_refresh = 1;

  HOOK_INVOKE ((this, HOOK_VIEW_CHANGE, DT_INT, view_start, DT_END));

  return true;
}

#ifndef NO_BELL
void
rxvt_term::bell_cb (ev::timer &w, int revents)
{
  rvideo_bell = false;
  scr_rvideo_mode (rvideo_mode);
  refresh_check ();
}
#endif

/* ------------------------------------------------------------------------- */
void
rxvt_term::scr_bell () NOTHROW
{
#ifndef NO_BELL

# ifndef NO_MAPALERT
#  ifdef MAPALERT_OPTION
  if (option (Opt_mapAlert))
#  endif
    XMapWindow (dpy, parent);
# endif

# if ENABLE_FRILLS
  if (option (Opt_urgentOnBell))
    set_urgency (1);
# endif

  if (option (Opt_visualBell))
    {
      rvideo_bell = true;
      scr_rvideo_mode (rvideo_mode);
      flush ();

      bell_ev.start (VISUAL_BELL_DURATION);
    }
  else
    XBell (dpy, 0);
  HOOK_INVOKE ((this, HOOK_BELL, DT_END));
#endif
}

/* ------------------------------------------------------------------------- */
void
rxvt_term::scr_printscreen (int fullhist) NOTHROW
{
#ifdef PRINTPIPE
  int nrows, row_start;
  FILE *fd = popen_printer ();

  if (!fd)
    return;

  if (fullhist)
    {
      nrows = nrow - top_row;
      row_start = top_row;
    }
  else
    {
      nrows = nrow;
      row_start = view_start;
    }

  wctomb (0, 0);

  for (int r1 = row_start; r1 < row_start + nrows; r1++)
    {
      text_t *tp = ROW(r1).t;
      int len    = ROW(r1).l;

      for (int i = len >= 0 ? len : ncol - 1; i--; ) //TODO//FIXME//LEN
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
void
rxvt_term::scr_refresh () NOTHROW
{
  int16_t col, row,   /* column/row we're processing               */
          ocrow;      /* old cursor row                            */
  int i;              /* tmp                                       */
#ifndef NO_CURSORCOLOR
  rend_t cc1;         /* store colours at cursor position (s)      */
#endif
  rend_t *crp;        // cursor rendition pointer
  rend_t ccol1,  /* Cursor colour       */
         ccol2;  /* Cursor colour2      */

  want_refresh = 0;        /* screen is current */

  if (refresh_type == NO_REFRESH || !mapped)
    return;

  /*
   * A: set up vars
   */
  refresh_count = 0;

  unsigned int old_screen_flags = screen.flags;
  bool have_bg = 0;
#ifdef HAVE_BG_PIXMAP
  have_bg = bg_pixmap != None;
#endif
  ocrow = oldcursor.row; /* is there an old outline cursor on screen? */

  /*
   * B: reverse any characters which are selected
   */
  scr_reverse_selection ();

  HOOK_INVOKE ((this, HOOK_REFRESH_BEGIN, DT_END));
#if ENABLE_OVERLAY
  scr_swap_overlay ();
#endif

  bool showcursor = screen.flags & Screen_VisibleCursor;

  /*
   * C: set the cursor character (s)
   */
  {
    bool setoldcursor;

#ifdef CURSOR_BLINK
    if (hidden_cursor)
      showcursor = 0;
#endif

    if (showcursor)
      {
        int col = screen.cur.col;

        while (col && ROW(screen.cur.row).t[col] == NOCHAR)
          col--;

        crp = &ROW(screen.cur.row).r[col];

#ifndef NO_CURSORCOLOR
        cc1 = *crp & (RS_fgMask | RS_bgMask);
        if (ISSET_PIXCOLOR (Color_cursor))
          ccol1 = Color_cursor;
        else
#endif
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
          ccol1 = fgcolor_of (rstyle);
#else
          ccol1 = Color_fg;
#endif

#ifndef NO_CURSORCOLOR
        if (ISSET_PIXCOLOR (Color_cursor2))
          ccol2 = Color_cursor2;
        else
#endif
#ifdef CURSOR_COLOR_IS_RENDITION_COLOR
          ccol2 = bgcolor_of (rstyle);
#else
          ccol2 = Color_bg;
#endif

        if (focus)
          {
            if (option (Opt_cursorUnderline))
              *crp ^= RS_Uline;
            else
              {
                *crp ^= RS_RVid;
                *crp = SET_FGCOLOR (*crp, ccol1);
                *crp = SET_BGCOLOR (*crp, ccol2);
              }
          }
      }

    /* make sure no outline cursor is left around */
    setoldcursor = 0;
    if (ocrow != -1)
      {
        if (screen.cur.row - view_start != ocrow
            || screen.cur.col != oldcursor.col)
          {
            if (ocrow < nrow
                && oldcursor.col < ncol)
              drawn_buf[ocrow].r[oldcursor.col] ^= (RS_RVid | RS_Uline);

            if (focus || !showcursor)
              oldcursor.row = -1;
            else
              setoldcursor = 1;
          }
      }
    else if (!focus)
      setoldcursor = 1;

    if (setoldcursor)
      {
        if (screen.cur.row - view_start >= nrow)
          oldcursor.row = -1;
        else
          {
            oldcursor.row = screen.cur.row - view_start;
            oldcursor.col = screen.cur.col;
          }
      }
  }

#ifndef NO_SLOW_LINK_SUPPORT
  /*
   * D: CopyArea pass - very useful for slower links
   *    This has been deliberately kept simple.
   */
  if (!display->is_local
      && refresh_type == FAST_REFRESH && num_scr_allow && num_scr
      && abs (num_scr) < nrow && !have_bg)
    {
      int16_t nits;
      int i = num_scr;
      int j;
      int len, wlen;

      j = nrow;
      wlen = len = -1;
      row = i > 0 ? 0 : j - 1;

      for (; j-- >= 0; row += (i > 0 ? 1 : -1))
        {
          if (row + i >= 0 && row + i < nrow && row + i != ocrow)
            {
              line_t s  = ROW(view_start + row);
              line_t d  = drawn_buf[row];
              line_t d2 = drawn_buf[row + i];

              for (nits = 0, col = ncol; col--; )
                if (s.t[col] != d2.t[col] || s.r[col] != d2.r[col])
                  nits--;
                else if (s.t[col] != d.t[col] || s.r[col] != d.r[col])
                  nits++;

              if (nits > 8) /* XXX: arbitrary choice */
                {
                  for (col = ncol; col--; )
                    {
                      *d.t++ = *d2.t++;
                      *d.r++ = *d2.r++;
                    }

                  if (len == -1)
                    len = row;

                  wlen = row;
                  continue;
                }
            }

          if (len >= 0)
            {
              /* also comes here at end if needed because of >= above */
              if (wlen < len)
                ::swap (wlen, len);

              XGCValues gcv;

              gcv.graphics_exposures = 1; XChangeGC (dpy, gc, GCGraphicsExposures, &gcv);
              XCopyArea (dpy, vt, vt,
                         gc, 0, Row2Pixel (len + i),
                         (unsigned int)this->width,
                         (unsigned int)Height2Pixel (wlen - len + 1),
                         0, Row2Pixel (len));
              gcv.graphics_exposures = 0; XChangeGC (dpy, gc, GCGraphicsExposures, &gcv);

              len = -1;
            }
        }
    }
#endif

  /*
   * E: main pass across every character
   */
  for (row = 0; row < nrow; row++)
    {
      text_t *stp = ROW(view_start + row).t;
      rend_t *srp = ROW(view_start + row).r;
      text_t *dtp = drawn_buf[row].t;
      rend_t *drp = drawn_buf[row].r;

      /*
       * E2: OK, now the real pass
       */
      int ypixel = (int)Row2Pixel (row);

      for (col = 0; col < ncol; col++)
        {
          /* compare new text with old - if exactly the same then continue */
          if (stp[col] == dtp[col]    /* Must match characters to skip. */
              && (RS_SAME (srp[col], drp[col])    /* Either rendition the same or   */
                  || (stp[col] == ' ' /* space w/ no background change  */
                      && GET_BGATTR (srp[col]) == GET_BGATTR (drp[col]))))
            continue;

          // redraw one or more characters

          // seek to the beginning of wide characters
          while (ecb_unlikely (stp[col] == NOCHAR && col > 0))
            --col;

          rend_t rend = srp[col];     /* screen rendition (target rendition) */
          text_t *text = stp + col;
          int count = 1;

          dtp[col] = stp[col];
          drp[col] = rend;

          int xpixel = Col2Pixel (col);

          for (i = 0; ++col < ncol; )
            {
              if (stp[col] == NOCHAR)
                {
                  dtp[col] = stp[col];
                  drp[col] = srp[col];

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
                  if (have_bg && (i++ > count / 2))
                    break;

                  dtp[col] = stp[col];
                  drp[col] = rend;
                  i = 0;
                }
              else if (have_bg || (stp[col] != ' ' && ++i >= 16))
                break;
            }

          col--;      /* went one too far.  move back */
          count -= i; /* dump any matching trailing chars */

          // sometimes we optimize away the trailing NOCHAR's, add them back
          while (ecb_unlikely (i && text[count] == NOCHAR))
            count++, i--;

          /*
           * Determine the attributes for the string
           */
          int fore = fgcolor_of (rend); // desired foreground
          int back = bgcolor_of (rend); // desired background

          // only do special processing if any attributes are set, which is unlikely
          if (ecb_unlikely (rend & (RS_baseattrMask | RS_Careful | RS_Sel)))
            {
              bool invert = rend & RS_RVid;

#ifndef NO_BOLD_UNDERLINE_REVERSE
              if (rend & RS_Bold && fore == Color_fg)
                {
                  if (ISSET_PIXCOLOR (Color_BD))
                    fore = Color_BD;
# if !ENABLE_STYLES
                  else
                    invert = !invert;
# endif
                }

              if (rend & RS_Italic && fore == Color_fg)
                {
                  if (ISSET_PIXCOLOR (Color_IT))
                    fore = Color_IT;
# if !ENABLE_STYLES
                  else
                    invert = !invert;
# endif
                }

              if (rend & RS_Uline && fore == Color_fg && ISSET_PIXCOLOR (Color_UL))
                fore = Color_UL;
#endif

#ifdef OPTION_HC
              if (rend & RS_Sel)
                {
                  /* invert the column if no highlightColor is set or it is the
                   * current cursor column */
                  if (!(showcursor && row == screen.cur.row && text - stp == screen.cur.col)
                      && ISSET_PIXCOLOR (Color_HC))
                    {
                      if (ISSET_PIXCOLOR (Color_HTC))
                        fore = Color_HTC;
                      // if invert is 0 reverse video is set so we use bg color as fg color
                      else if (!invert)
                        fore = back;

                      back = Color_HC;
                      invert = 0;
                    }
                }
#endif

              if (invert)
                {
                    ::swap (fore, back);

#ifndef NO_BOLD_UNDERLINE_REVERSE
                  if (fore == back)
                    {
                      fore = Color_bg;
                      back = Color_fg;
                    }
#endif
                }

#ifdef TEXT_BLINK
              if (rend & RS_Blink && (back == Color_bg || fore == Color_bg))
                {
                  if (!text_blink_ev.is_active ())
                    {
                      text_blink_ev.again ();
                      hidden_text = 0;
                    }
                  else if (hidden_text)
                    fore = back;
                }
#endif

#if ENABLE_STYLES
              // "careful" (too wide) character handling

              // include previous careful character(s) if possible, looks nicer (best effort...)
              while (text > stp
                  && srp[text - stp - 1] & RS_Careful
                  && RS_SAME (rend, srp[text - stp - 1]))
                text--, count++, xpixel -= fwidth;

              // force redraw after "careful" characters to avoid pixel droppings
              for (int i = 0; srp[col + i] & RS_Careful && col + i < ncol - 1; i++)
                drp[col + i + 1] = srp[col + i + 1] ^ RS_redraw;

              // force redraw before "careful" characters to avoid pixel droppings
              for (int i = 0; srp[text - stp - i] & RS_Careful && text - i > stp; i++)
                drp[text - stp - i - 1] = srp[text - stp - i - 1] ^ RS_redraw;
#endif
            }

          /*
           * Actually do the drawing of the string here
           */
          rxvt_font *font = (*fontset[GET_STYLE (rend)])[GET_FONT (rend)];

          if (ecb_likely (have_bg && back == Color_bg))
            {
              // this is very ugly, maybe push it into ->draw?

              for (i = 0; i < count; i++) /* don't draw empty strings */
                if (text[i] != ' ')
                  {
                    font->draw (*drawable, xpixel, ypixel, text, count, fore, Color_transparent);
                    goto did_clear;
                  }

              CLEAR_CHARS (xpixel, ypixel, count);
              did_clear: ;
            }
          else
            font->draw (*drawable, xpixel, ypixel, text, count, fore, back);

          if (ecb_unlikely (rend & RS_Uline && font->descent > 1 && fore != back))
            {
#if ENABLE_FRILLS
              if (ISSET_PIXCOLOR (Color_underline))
                XSetForeground (dpy, gc, pix_colors[Color_underline]);
              else
#endif
                XSetForeground (dpy, gc, pix_colors[fore]);

              XDrawLine (dpy, vt, gc,
                         xpixel, ypixel + font->ascent + 1,
                         xpixel + Width2Pixel (count) - 1, ypixel + font->ascent + 1);
            }
        }                     /* for (col....) */
    }                         /* for (row....) */

  /*
   * G: cleanup cursor and display outline cursor if necessary
   */
  if (showcursor)
    {
      if (focus)
        {
          if (option (Opt_cursorUnderline))
            *crp ^= RS_Uline;
          else
            {
              *crp ^= RS_RVid;
#ifndef NO_CURSORCOLOR
              *crp = (*crp & ~ (RS_fgMask | RS_bgMask)) | cc1;
#endif
            }
        }
      else if (oldcursor.row >= 0)
        {
          int cursorwidth = 1;
          int col = oldcursor.col;

          while (col && ROW(screen.cur.row).t[col] == NOCHAR)
            col--;

          while (col + cursorwidth < ncol
                 && drawn_buf[oldcursor.row].t[col + cursorwidth] == NOCHAR)
            cursorwidth++;

#ifndef NO_CURSORCOLOR
          if (ISSET_PIXCOLOR (Color_cursor))
            XSetForeground (dpy, gc, pix_colors[Color_cursor]);
          else
#endif
            XSetForeground (dpy, gc, pix_colors[ccol1]);

          XDrawRectangle (dpy, vt, gc,
                          Col2Pixel (col),
                          Row2Pixel (oldcursor.row),
                          (unsigned int) (Width2Pixel (cursorwidth) - 1),
                          (unsigned int) (Height2Pixel (1) - lineSpace - 1));
        }
    }

  /*
   * H: cleanup selection
   */
#if ENABLE_OVERLAY
  scr_swap_overlay ();
#endif
  HOOK_INVOKE ((this, HOOK_REFRESH_END, DT_END));

  scr_reverse_selection ();

  screen.flags = old_screen_flags;
  num_scr = 0;
  num_scr_allow = 1;
}

void
rxvt_term::scr_remap_chars (line_t &l) NOTHROW
{
  if (!l.t)
    return;

  l.touch (); // maybe a bit of an overkill, but it's not performance-relevant

  for (int i = ncol; i--; )
    l.r[i] = SET_FONT (l.r[i], FONTSET (l.r[i])->find_font (l.t[i]));
}

void
rxvt_term::scr_remap_chars () NOTHROW
{
  for (int i = total_rows; i--; )
    scr_remap_chars (row_buf [i]);

  for (int i = nrow; i--; )
    {
      scr_remap_chars (drawn_buf [i]);
      scr_remap_chars (swap_buf [i]);
    }
}

void
rxvt_term::scr_recolour (bool refresh) NOTHROW
{
  bool transparent = false;

#ifdef HAVE_BG_PIXMAP
  if (bg_pixmap != None)
    {
# ifdef ENABLE_TRANSPARENCY
      if (bg_flags & BG_IS_TRANSPARENT)
        {
          XSetWindowBackgroundPixmap (dpy, parent, bg_pixmap);
          XSetWindowBackgroundPixmap (dpy, vt, ParentRelative);

          transparent = true;
        }
      else
# endif
        {
          XSetWindowBackground (dpy, parent, pix_colors[Color_border]);
          XSetWindowBackgroundPixmap (dpy, vt, bg_pixmap);
        }
    }
  else
#endif
    {
      XSetWindowBackground (dpy, parent, pix_colors[Color_border]);
      XSetWindowBackground (dpy, vt, pix_colors[Color_bg]);
    }

  XClearWindow (dpy, parent);

  if (scrollBar.win)
    {
      if (transparent)
        XSetWindowBackgroundPixmap (dpy, scrollBar.win, ParentRelative);
      else
        XSetWindowBackground (dpy, scrollBar.win, pix_colors[Color_border]);
      scrollBar.state = SB_STATE_IDLE;
      scrollBar.show (0);
    }

  if (refresh)
    {
      scr_clear ();
      scr_touch (true);
    }
  want_refresh = 1;
}

/* ------------------------------------------------------------------------- */
void
rxvt_term::scr_clear (bool really) NOTHROW
{
  if (!mapped)
    return;

  num_scr_allow = 0;
  want_refresh = 1;

  if (really)
    XClearWindow (dpy, vt);
}

void
rxvt_term::scr_xor_rect (int beg_row, int beg_col, int end_row, int end_col, rend_t rstyle1, rend_t rstyle2) NOTHROW
{
  int view_end = view_start + nrow;
  int row, col;

  for (row = max (beg_row, view_start); row <= min (end_row, view_end); row++)
    {
      text_t *stp = ROW(row).t;
      rend_t *srp = ROW(row).r;

      for (col = beg_col; col < end_col; col++)
        srp[col] ^= rstyle1;

      while (col-- > beg_col && (stp[col] == NOCHAR || unicode::is_space (stp[col])))
        srp[col] ^= rstyle2;

      if (++col < end_col)
        srp[col] ^= rstyle2;
    }
}

void
rxvt_term::scr_xor_span (int beg_row, int beg_col, int end_row, int end_col, rend_t rstyle) NOTHROW
{
  int view_end = view_start + nrow;
  int row, col;

  if (beg_row >= view_start)
    {
      col = beg_col;
      row = beg_row;
    }
  else
    {
      col = 0;
      row = view_start;
    }

  for (; row < min (end_row, view_end); row++, col = 0)
    for (rend_t *srp = ROW(row).r; col < ncol; col++)
      srp[col] ^= rstyle;

  if (row == end_row)
    for (rend_t *srp = ROW(row).r; col < end_col; col++)
      srp[col] ^= rstyle;
}

/* ------------------------------------------------------------------------- */
void
rxvt_term::scr_reverse_selection () NOTHROW
{
  if (selection.op
      && current_screen == selection.screen
      && selection.end.row >= view_start)
    {
#if !ENABLE_MINIMAL
      if (selection.rect)
        scr_xor_rect (selection.beg.row, selection.beg.col,
                      selection.end.row, selection.end.col,
                      RS_Sel | RS_RVid, RS_Sel | RS_RVid | RS_Uline);
      else
#endif
        scr_xor_span (selection.beg.row, selection.beg.col,
                      selection.end.row, selection.end.col,
                      RS_Sel | RS_RVid);
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Dump the whole scrollback and screen to the passed filedescriptor.  The
 * invoking routine must close the fd.
 */
#if 0
void
rxvt_term::scr_dump (int fd) NOTHROW
{
  // if this method is needed, it can be implemented by factoring the
  // relevant code in scr_printscreen
}
#endif

/* ------------------------------------------------------------------------- *
 *                           CHARACTER SELECTION                             *
 * ------------------------------------------------------------------------- */
void
rxvt_term::selection_check (int check_more) NOTHROW
{
  if (!selection.op)
    return;

  if (!IN_RANGE_EXC (selection.beg.row, top_row, nrow)
      || !IN_RANGE_EXC (selection.mark.row, top_row, nrow)
      || !IN_RANGE_EXC (selection.end.row, top_row, nrow)
      || (check_more == 1
          && current_screen == selection.screen
          && !ROWCOL_IS_BEFORE (screen.cur, selection.beg)
          && ROWCOL_IS_BEFORE (screen.cur, selection.end)))
    CLEAR_ALL_SELECTION ();
}

/* ------------------------------------------------------------------------- */
/*
 * Paste a selection direct to the command fd
 */
void
rxvt_term::tt_paste (char *data, unsigned int len) NOTHROW
{
  /* convert normal newline chars into common keyboard Return key sequence */
  for (unsigned int i = 0; i < len; i++)
    if (data[i] == C0_LF)
      data[i] = C0_CR;

  if (priv_modes & PrivMode_BracketPaste)
    tt_printf ("\x1b[200~");

  tt_write (data, len);

  if (priv_modes & PrivMode_BracketPaste)
    tt_printf ("\x1b[201~");
}

void
rxvt_term::paste (char *data, unsigned int len) NOTHROW
{
  if (HOOK_INVOKE ((this, HOOK_TT_PASTE, DT_STR_LEN, data, len, DT_END)))
    return;

  tt_paste (data, len);
}

/* ------------------------------------------------------------------------- */
/*
 * Request PRIMARY, SECONDARY or CLIPBOARD selection.
 * if the requested selection has no owner or is empty CUT_BUFFER0 is used
 * as fallback
 * EXT: button 2 release
 */
void
rxvt_term::selection_request (Time tm, int selnum) NOTHROW
{
  if (!selection_req)
    {
      selection_req = new rxvt_selection (display, selnum, tm, vt, xa[XA_VT_SELECTION], this);
      selection_req->run ();
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Clear all selected text
 * EXT: SelectionClear
 */
void
rxvt_term::selection_clear (bool clipboard) NOTHROW
{
  if (!clipboard)
    {
      want_refresh = 1;
      free (selection.text);
      selection.text = NULL;
      selection.len = 0;
      CLEAR_SELECTION ();

      if (display->selection_owner == this)
        display->selection_owner = 0;
    }
  else
    {
      free (selection.clip_text);
      selection.clip_text = NULL;
      selection.clip_len = 0;

      if (display->clipboard_owner == this)
        display->clipboard_owner = 0;
    }
}

/* ------------------------------------------------------------------------- */
/*
 * Copy a selection into the cut buffer
 * EXT: button 1 or 3 release
 */
void
rxvt_term::selection_make (Time tm)
{
  int size;
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

  if (HOOK_INVOKE ((this, HOOK_SEL_MAKE, DT_LONG, (long)tm, DT_END)))
    return;

  size = (selection.end.row - selection.beg.row + 1) * (ncol + 1);
  new_selection_text = (wchar_t *)rxvt_malloc ((size + 4) * sizeof (wchar_t));

  int ofs = 0;
  int extra = 0;

  int col = selection.beg.col;
  int row = selection.beg.row;

  int end_col;

  for (; row <= selection.end.row; row++, col = 0)
    {
#if !ENABLE_MINIMAL
      if (selection.rect)
        {
          col = selection.beg.col;
          end_col = selection.end.col;
        }
      else
#endif
        end_col = ROW(row).l;

      col = max (col, 0);

      if (row == selection.end.row)
        min_it (end_col, selection.end.col);

      t = ROW(row).t + col;

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
                  extra += size;
                  size += size;
                  new_selection_text = (wchar_t *)rxvt_realloc (new_selection_text, (size + 4) * sizeof (wchar_t));
                }

              ofs += rxvt_composite.expand (*t++, new_selection_text + ofs);
            }
#endif
          else
            new_selection_text[ofs++] = *t++;
        }

#if !ENABLE_MINIMAL
      if (selection.rect)
        {
          while (ofs
                 && new_selection_text[ofs - 1] != C0_LF
                 && unicode::is_space (new_selection_text[ofs - 1]))
            --ofs;

          new_selection_text[ofs++] = C0_LF;
        }
      else
#endif
        if (!ROW(row).is_longer ()
            && (row != selection.end.row || end_col != selection.end.col)
            && (row != selection.beg.row || selection.beg.col < ncol))
          new_selection_text[ofs++] = C0_LF;
    }

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

  if (HOOK_INVOKE ((this, HOOK_SEL_GRAB, DT_LONG, (long)tm, DT_END)))
    return;

  selection_grab (tm);
}

bool
rxvt_term::selection_grab (Time tm, bool clipboard) NOTHROW
{
  Atom sel;

  if (!clipboard)
    {
      selection_time = tm;
      sel = XA_PRIMARY;
    }
  else
    {
      clipboard_time = tm;
      sel = xa[XA_CLIPBOARD];
    }

  XSetSelectionOwner (dpy, sel, vt, tm);
  if (XGetSelectionOwner (dpy, sel) == vt)
    {
      display->set_selection_owner (this, clipboard);
      return true;
    }
  else
    {
      selection_clear (clipboard);
      return false;
    }

#if 0
  XTextProperty ct;

  if (XwcTextListToTextProperty (dpy, &selection.text, 1, XStringStyle, &ct) >= 0)
    {
      set_string_property (XA_CUT_BUFFER0, ct.value, ct.nitems);
      XFree (ct.value);
    }
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Mark or select text based upon number of clicks: 1, 2, or 3
 * EXT: button 1 press
 */
void
rxvt_term::selection_click (int clicks, int x, int y) NOTHROW
{
  clicks = ((clicks - 1) % 3) + 1;
  selection.clicks = clicks;       /* save clicks so extend will work */

  if (clicks == 2 && !selection.rect
      && HOOK_INVOKE ((this, HOOK_SEL_EXTEND, DT_END)))
    {
      MEvent.clicks = 1; // what a mess
      selection.screen = current_screen;
      selection.op = SELECTION_CONT;
      return;
    }

  selection_start_colrow (Pixel2Col (x), Pixel2Row (y));

  if (clicks == 2 || clicks == 3)
    selection_extend_colrow (selection.mark.col,
                             selection.mark.row - view_start,
                             0, /* button 3     */
                             1, /* button press */
                             0);        /* click change */
}

/* ------------------------------------------------------------------------- */
/*
 * Mark a selection at the specified col/row
 */
void
rxvt_term::selection_start_colrow (int col, int row) NOTHROW
{
  want_refresh = 1;

  selection.mark.row = row + view_start;
  selection.mark.col = col;

  selection.mark.row = clamp (selection.mark.row, top_row, nrow - 1);
  selection.mark.col = clamp (selection.mark.col,       0, ncol - 1);

  while (selection.mark.col > 0
         && ROW(selection.mark.row).t[selection.mark.col] == NOCHAR)
    --selection.mark.col;

  if (selection.op)
    {
      /* clear the old selection */
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
rxvt_term::selection_delimit_word (enum page_dirn dirn, const row_col_t *mark, row_col_t *ret) NOTHROW
{
  int col, row, dirnadd, tcol, trow, w1, w2;
  row_col_t bound;
  text_t *stp;
  rend_t *srp;

  if (dirn == UP)
    {
      bound.row = top_row - 1;
      bound.col = 0;
      dirnadd = -1;
    }
  else
    {
      bound.row = nrow;
      bound.col = ncol - 1;
      dirnadd = 1;
    }

  row = mark->row;
  col = max (mark->col, 0);

  /* find the edge of a word */
  stp = ROW(row).t + col; w1 = DELIMIT_TEXT (*stp);
  srp = ROW(row).r + col; w2 = DELIMIT_REND (*srp);

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
          if (ROW(row - (dirn == UP ? 1 : 0)).is_longer ())
            {
              trow = row + dirnadd;
              tcol = dirn == UP ? ncol - 1 : 0;

              if (!ROW(trow).t)
                break;

              stp = ROW(trow).t + tcol;
              srp = ROW(trow).r + tcol;

              if (DELIMIT_TEXT (*stp) != w1 || DELIMIT_REND (*srp) != w2)
                break;

              row = trow;
              col = tcol;
              continue;
            }
        }
      break;
    }

  if (dirn == DN)
    col++;                  /* put us on one past the end */

  /* Poke the values back in */
  ret->row = row;
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
rxvt_term::selection_extend (int x, int y, int flag) NOTHROW
{
  int col = clamp (Pixel2Col (x), 0, ncol);
  int row = clamp (Pixel2Row (y), 0, nrow - 1);

  /*
  * If we're selecting characters (single click) then we must check first
  * if we are at the same place as the original mark.  If we are then
  * select nothing.  Otherwise, if we're to the right of the mark, you have to
  * be _past_ a character for it to be selected.
  */
  if (((selection.clicks % 3) == 1) && !flag
      && (col == selection.mark.col
          && (row == selection.mark.row - view_start)))
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
rxvt_term::selection_extend_colrow (int32_t col, int32_t row, int button3, int buttonpress, int clickchange) NOTHROW
{
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
  pos.row = view_start + row;

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
    {
      /* button3 press */
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
    {
      /* button1 drag or button3 drag */
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
      if (selection.beg.col > ROW(selection.beg.row).l //TODO//FIXME//LEN
          && !ROW(selection.beg.row).is_longer ()
#if !ENABLE_MINIMAL
          && !selection.rect
#endif
         )
        selection.beg.col = ncol;

      if (
          selection.end.col > ROW(selection.end.row).l //TODO//FIXME//LEN
          && !ROW(selection.end.row).is_longer ()
#if !ENABLE_MINIMAL
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
      if (option (Opt_tripleclickwords))
        {
          selection_delimit_word (UP, &selection.beg, &selection.beg);

          for (int end_row = selection.mark.row; end_row < nrow; end_row++)
            {
              if (!ROW(end_row).is_longer ())
                {
                  selection.end.row = end_row;
                  selection.end.col = ROW(end_row).l;
# if !ENABLE_MINIMAL
                  selection_remove_trailing_spaces ();
# endif
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

          // select a complete logical line
          while (selection.beg.row > -saveLines
                 && ROW(selection.beg.row - 1).is_longer ())
            selection.beg.row--;

          while (selection.end.row < nrow
                 && ROW(selection.end.row).is_longer ())
            selection.end.row++;
        }
    }

  if (button3 && buttonpress)
    {
      /* mark may need to be changed */
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

#if !ENABLE_MINIMAL
  if (selection.rect && selection.beg.col > selection.end.col)
    ::swap (selection.beg.col, selection.end.col);
#endif
}

#if !ENABLE_MINIMAL
void
rxvt_term::selection_remove_trailing_spaces () NOTHROW
{
  int32_t end_col, end_row;
  text_t *stp;

  end_col = selection.end.col;
  end_row = selection.end.row;

  for (; end_row >= selection.beg.row; )
    {
      stp = ROW(end_row).t;

      while (--end_col >= 0)
        {
          if (stp[end_col] != NOCHAR
              && !unicode::is_space (stp[end_col]))
            break;
        }

      if (end_col >= 0
          || !ROW(end_row - 1).is_longer ())
        {
          selection.end.col = end_col + 1;
          selection.end.row = end_row;
          break;
        }

      end_row--;
      end_col = ncol;
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
rxvt_term::selection_rotate (int x, int y) NOTHROW
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
rxvt_term::selection_send (const XSelectionRequestEvent &rq) NOTHROW
{
  Atom property = rq.property == None ? rq.target : rq.property;
  XSelectionEvent ev;

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

      XChangeProperty (dpy, rq.requestor, property, XA_ATOM,
                       32, PropModeReplace,
                       (unsigned char *)target_list, target - target_list);
      ev.property = property;
    }
#if TODO // TODO
  else if (rq.target == xa[XA_MULTIPLE])
    {
      /* TODO: Handle MULTIPLE */
    }
#endif
  else if (rq.target == xa[XA_TIMESTAMP] && rq.selection == XA_PRIMARY && selection.text)
    {
      XChangeProperty (dpy, rq.requestor, property, rq.target,
                       32, PropModeReplace, (unsigned char *)&selection_time, 1);
      ev.property = property;
    }
  else if (rq.target == xa[XA_TIMESTAMP] && rq.selection == xa[XA_CLIPBOARD] && selection.clip_text)
    {
      XChangeProperty (dpy, rq.requestor, property, rq.target,
                       32, PropModeReplace, (unsigned char *)&clipboard_time, 1);
      ev.property = property;
    }
  else if (rq.target == XA_STRING
           || rq.target == xa[XA_TEXT]
           || rq.target == xa[XA_COMPOUND_TEXT]
           || rq.target == xa[XA_UTF8_STRING]
          )
    {
      XTextProperty ct;
      Atom target = rq.target;
      short freect = 0;
      int selectlen;
      wchar_t *cl;
      enum {
        enc_string        = XStringStyle,
        enc_text          = XStdICCTextStyle,
        enc_compound_text = XCompoundTextStyle,
#ifdef X_HAVE_UTF8_STRING
        enc_utf8          = XUTF8StringStyle,
#else
        enc_utf8          = -1,
#endif
      } style;

      if (target == XA_STRING)
        // we actually don't do XA_STRING, but who cares, as i18n clients
        // will ask for another format anyways.
        style = enc_string;
      else if (target == xa[XA_TEXT])
        style = enc_text;
      else if (target == xa[XA_COMPOUND_TEXT])
        style = enc_compound_text;
#if !ENABLE_MINIMAL
      else if (target == xa[XA_UTF8_STRING])
        style = enc_utf8;
#endif
      else
        {
          target = xa[XA_COMPOUND_TEXT];
          style = enc_compound_text;
        }

      if (rq.selection == XA_PRIMARY && selection.text)
        {
          cl = selection.text;
          selectlen = selection.len;
        }
      else if (rq.selection == xa[XA_CLIPBOARD] && selection.clip_text)
        {
          cl = selection.clip_text;
          selectlen = selection.clip_len;
        }
      else
        {
          cl = L"";
          selectlen = 0;
        }

#if !ENABLE_MINIMAL
      // xlib is horribly broken with respect to UTF8_STRING, and nobody cares to fix it
      // so recode it manually
      if (style == enc_utf8)
        {
          freect = 1;
          ct.encoding = target;
          ct.format = 8;
          ct.value = (unsigned char *)rxvt_wcstoutf8 (cl, selectlen);
          ct.nitems = strlen ((char *)ct.value);
        }
      else
#endif
      if (XwcTextListToTextProperty (dpy, &cl, 1, (XICCEncodingStyle) style, &ct) >= 0)
        freect = 1;
      else
        {
          /* if we failed to convert then send it raw */
          ct.value = (unsigned char *)cl;
          ct.nitems = selectlen;
          ct.encoding = target;
        }

      XChangeProperty (dpy, rq.requestor, property,
                       ct.encoding, 8, PropModeReplace,
                       ct.value, (int)ct.nitems);
      ev.property = property;

      if (freect)
        XFree (ct.value);
    }

  XSendEvent (dpy, rq.requestor, False, 0L, (XEvent *)&ev);
}

/* ------------------------------------------------------------------------- */
#ifdef USE_XIM
void
rxvt_term::im_set_position (XPoint &pos) NOTHROW
{
  XWindowAttributes xwa;

  XGetWindowAttributes (dpy, vt, &xwa);

  pos.x = xwa.x + Col2Pixel    (screen.cur.col);
  pos.y = xwa.y + Height2Pixel (screen.cur.row) + fbase;
}
#endif

#if ENABLE_OVERLAY
void
rxvt_term::scr_overlay_new (int x, int y, int w, int h) NOTHROW
{
  if (nrow < 1 || ncol < 1)
    return;

  want_refresh = 1;

  scr_overlay_off ();

  if (x < 0) x = ncol - w;
  if (y < 0) y = nrow - h;

  // make space for border
  w += 2; min_it (w, ncol);
  h += 2; min_it (h, nrow);

  x -= 1; clamp_it (x, 0, ncol - w);
  y -= 1; clamp_it (y, 0, nrow - h);

  ov.x = x; ov.y = y;
  ov.w = w; ov.h = h;

  ov.text = new text_t *[h];
  ov.rend = new rend_t *[h];

  for (y = 0; y < h; y++)
    {
      text_t *tp = ov.text[y] = new text_t[w];
      rend_t *rp = ov.rend[y] = new rend_t[w];

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
rxvt_term::scr_overlay_off () NOTHROW
{
  if (!ov.text)
    return;

  want_refresh = 1;

  for (int y = 0; y < ov.h; y++)
    {
      delete [] ov.text[y];
      delete [] ov.rend[y];
    }

  delete [] ov.text; ov.text = 0;
  delete [] ov.rend; ov.rend = 0;
}

void
rxvt_term::scr_overlay_set (int x, int y, text_t text, rend_t rend) NOTHROW
{
  if (!ov.text || x >= ov.w - 2 || y >= ov.h - 2)
    return;

  x++, y++;

  ov.text[y][x] = text;
  ov.rend[y][x] = rend;
}

void
rxvt_term::scr_overlay_set (int x, int y, const char *s) NOTHROW
{
  while (*s)
    scr_overlay_set (x++, y, *s++);
}

void
rxvt_term::scr_overlay_set (int x, int y, const wchar_t *s) NOTHROW
{
  while (*s)
    {
      text_t t = *s++;
      int width = WCWIDTH (t);

      while (width--)
        {
          scr_overlay_set (x++, y, t);
          t = NOCHAR;
        }
    }
}

void
rxvt_term::scr_swap_overlay () NOTHROW
{
  if (!ov.text)
    return;

  // hide cursor if it is within the overlay area
  if (IN_RANGE_EXC (screen.cur.col - ov.x, 0, ov.w)
      && IN_RANGE_EXC (screen.cur.row - ov.y, 0, ov.h))
    screen.flags &= ~Screen_VisibleCursor;

  // swap screen mem with overlay
  for (int y = ov.h; y--; )
    {
      text_t *t1 = ov.text[y];
      rend_t *r1 = ov.rend[y];

      text_t *t2 = ROW(y + ov.y + view_start).t + ov.x;
      rend_t *r2 = ROW(y + ov.y + view_start).r + ov.x;

      for (int x = ov.w; x--; )
        {
          text_t t = *t1; *t1++ = *t2; *t2++ = t;
          rend_t r = *r1; *r1++ = *r2; *r2++ = SET_FONT (r, FONTSET (r)->find_font (t));
        }
    }
}

#endif
/* ------------------------------------------------------------------------- */

