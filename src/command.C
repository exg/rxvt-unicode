/*--------------------------------*-C-*---------------------------------*
 * File:	command.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *				- original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 * 				- extensive modifications
 * Copyright (c) 1995      Garrett D'Amore <garrett@netcom.com>
 *				- vt100 printing
 * Copyright (c) 1995      Steven Hirsch <hirsch@emba.uvm.edu>
 *				- X11 mouse report mode and support for
 *				  DEC "private mode" save/restore functions.
 * Copyright (c) 1995      Jakub Jelinek <jj@gnu.ai.mit.edu>
 *				- key-related changes to handle Shift+function
 *				  keys properly.
 * Copyright (c) 1997      MJ Olesen <olesen@me.queensu.ca>
 *				- extensive modifications
 * Copyright (c) 1997      Raul Garcia Garcia <rgg@tid.es>
 *				- modification and cleanups for Solaris 2.x
 *				  and Linux 1.2.x
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 * 				- extensive modifications
 * Copyright (c) 1998      Alfredo K. Kojima <kojima@windowmaker.org>
 * Copyright (c) 2001      Marius Gedminas
 *				- Ctrl/Mod4+Tab works like Meta+Tab (options)
 * Copyright (c) 2003      Rob McMullen <robm@flipturn.org>
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
 *----------------------------------------------------------------------*/

/*{{{ includes: */
#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "version.h"
#include "command.h"

#include <wchar.h>

/*----------------------------------------------------------------------*/

/*{{{ Convert the keypress event into a string */
void
rxvt_term::lookup_key (XKeyEvent &ev)
{
  int ctrl, meta, shft, len;
  unsigned int newlen;
  KeySym keysym;
#ifdef DEBUG_CMD
  static int debug_key = 1;	/* accessible by a debugger only */
#endif
  int valid_keysym;
  unsigned char kbuf[KBUFSZ];

  /*
   * use Num_Lock to toggle Keypad on/off.  If Num_Lock is off, allow an
   * escape sequence to toggle the Keypad.
   *
   * Always permit `shift' to override the current setting
   */
  shft = (ev.state & ShiftMask);
  ctrl = (ev.state & ControlMask);
  meta = (ev.state & ModMetaMask);

  if (numlock_state || (ev.state & ModNumLockMask))
    {
      numlock_state = (ev.state & ModNumLockMask);
      PrivMode ((!numlock_state), PrivMode_aplKP);
    }

  kbuf[0] = 0;

#ifdef USE_XIM
  if (Input_Context)
    {
      Status status_return;

#ifdef X_HAVE_UTF8_STRING
      if (enc_utf8 && 0) // currently disabled, doesn't seem to work, nor is useful
        len = Xutf8LookupString (Input_Context, &ev, (char *)kbuf,
                                 KBUFSZ, &keysym, &status_return);
      else
#endif
        {
          wchar_t wkbuf[KBUFSZ + 1];

          // the XOpenIM manpage lies about hardcoding the locale
          // at the point of XOpenIM, so temporarily switch locales
          if (rs[Rs_imLocale])
            SET_LOCALE (rs[Rs_imLocale]);
          // assume wchar_t == unicode or better
          len = XwcLookupString (Input_Context, &ev, wkbuf,
                                 KBUFSZ, &keysym, &status_return);
          if (rs[Rs_imLocale])
            SET_LOCALE (locale);

          if (status_return == XLookupChars
              || status_return == XLookupBoth)
            {
              wkbuf[len] = 0;
              len = wcstombs ((char *)kbuf, wkbuf, KBUFSZ);
              if (len < 0)
                len = 0;
            }
          else
            len = 0;
        }

      valid_keysym = status_return == XLookupKeySym
                     || status_return == XLookupBoth;
    }
  else
#endif
    {
      len = XLookupString (&ev, (char *)kbuf, KBUFSZ, &keysym, &compose);
      valid_keysym = keysym != NoSymbol;
    }

  if (valid_keysym)
    {
      /* for some backwards compatibility */
#if defined(HOTKEY_CTRL) || defined(HOTKEY_META)
# ifdef HOTKEY_CTRL
      if (ctrl)
# else
      if (meta)
# endif
        {
          if (keysym == ks_bigfont)
            {
              change_font (0, FONT_UP);
              return;
            }
          else if (keysym == ks_smallfont)
            {
              change_font (0, FONT_DN);
              return;
            }
        }
#endif

      if (TermWin.saveLines)
        {
#ifdef UNSHIFTED_SCROLLKEYS
          if (!ctrl && !meta)
            {
#else
          if (IS_SCROLL_MOD)
            {
#endif
              int lnsppg;

#ifdef PAGING_CONTEXT_LINES
              lnsppg = TermWin.nrow - PAGING_CONTEXT_LINES;
#else
              lnsppg = TermWin.nrow * 4 / 5;
#endif
              if (keysym == XK_Prior)
                {
                  scr_page (UP, lnsppg);
                  return;
                }
              else if (keysym == XK_Next)
                {
                  scr_page (DN, lnsppg);
                  return;
                }
            }
#ifdef SCROLL_ON_UPDOWN_KEYS
          if (IS_SCROLL_MOD)
            {
              if (keysym == XK_Up)
                {
                  scr_page (UP, 1);
                  return;
                }
              else if (keysym == XK_Down)
                {
                  scr_page (DN, 1);
                  return;
                }
            }
#endif
#ifdef SCROLL_ON_HOMEEND_KEYS
          if (IS_SCROLL_MOD)
            {
              if (keysym == XK_Home)
                {
                  scr_move_to (0, 1);
                  return;
                }
              else if (keysym == XK_End)
                {
                  scr_move_to (1, 0);
                  return;
                }
            }
#endif
        }

      if (shft)
        {
          /* Shift + F1 - F10 generates F11 - F20 */
          if (keysym >= XK_F1 && keysym <= XK_F10)
            {
              keysym += (XK_F11 - XK_F1);
              shft = 0;	/* turn off Shift */
            }
          else if (!ctrl && !meta && (PrivateModes & PrivMode_ShiftKeys))
            {
              switch (keysym)
                {
                    /* normal XTerm key bindings */
                  case XK_Insert:	/* Shift+Insert = paste mouse selection */
                    selection_request (ev.time, 0, 0);
                    return;
                    /* rxvt extras */
                  case XK_KP_Add:	/* Shift+KP_Add = bigger font */
                    change_font (0, FONT_UP);
                    return;
                  case XK_KP_Subtract:	/* Shift+KP_Subtract = smaller font */
                    change_font (0, FONT_DN);
                    return;
                }
            }
        }
#ifdef PRINTPIPE
      if (keysym == XK_Print)
        {
          scr_printscreen (ctrl | shft);
          return;
        }
#endif
#ifdef GREEK_SUPPORT
      if (keysym == ks_greekmodeswith)
        {
          greek_mode = !greek_mode;
          if (greek_mode)
            {
              xterm_seq (XTerm_title,
                         (greek_getmode () == GREEK_ELOT928
                          ? "[Greek: iso]" : "[Greek: ibm]"), CHAR_ST);
              greek_reset ();
            }
          else
            xterm_seq (XTerm_title, APL_NAME "-" VERSION, CHAR_ST);
          return;
        }
#endif

      if (keysym >= 0xFF00 && keysym <= 0xFFFF)
        {
#ifdef KEYSYM_RESOURCE
          if (! (shft | ctrl) && Keysym_map[keysym & 0xFF] != NULL)
            {
              unsigned int    l;
              const unsigned char *kbuf0;
              const unsigned char ch = C0_ESC;

              kbuf0 = (Keysym_map[keysym & 0xFF]);
              l = (unsigned int)*kbuf0++;

              /* escape prefix */
              if (meta)
# ifdef META8_OPTION
                if (meta_char == C0_ESC)
# endif
                  tt_write (&ch, 1);
              tt_write (kbuf0, l);
              return;
            }
          else
#endif
            {
              newlen = 1;
              switch (keysym)
                {
#ifndef NO_BACKSPACE_KEY
                  case XK_BackSpace:
                    if (PrivateModes & PrivMode_HaveBackSpace)
                      {
                        kbuf[0] = (!! (PrivateModes & PrivMode_BackSpace)
                                   ^ !!ctrl) ? '\b' : '\177';
                        kbuf[1] = '\0';
                      }
                    else
                      STRCPY (kbuf, key_backspace);
# ifdef MULTICHAR_SET
                    if ((Options & Opt_mc_hack) && screen.cur.col > 0)
                      {
                        int             col, row;

                        newlen = STRLEN (kbuf);
                        col = screen.cur.col - 1;
                        row = screen.cur.row + TermWin.saveLines;
                        if (IS_MULTI2 (screen.rend[row][col]))
                          MEMMOVE (kbuf + newlen, kbuf, newlen + 1);
                      }
# endif
                    break;
#endif
#ifndef NO_DELETE_KEY
                  case XK_Delete:
                    STRCPY (kbuf, key_delete);
# ifdef MULTICHAR_SET
                    if (Options & Opt_mc_hack)
                      {
                        int             col, row;

                        newlen = STRLEN (kbuf);
                        col = screen.cur.col;
                        row = screen.cur.row + TermWin.saveLines;
                        if (IS_MULTI1 (screen.rend[row][col]))
                          MEMMOVE (kbuf + newlen, kbuf, newlen + 1);
                      }
# endif
                    break;
#endif
                  case XK_Tab:
                    if (shft)
                      STRCPY (kbuf, "\033[Z");
                    else
                      {
#ifdef CTRL_TAB_MAKES_META
                        if (ctrl)
                          meta = 1;
#endif
#ifdef MOD4_TAB_MAKES_META
                        if (ev.state & Mod4Mask)
                          meta = 1;
#endif
                        newlen = 0;
                      }
                    break;

#ifdef XK_KP_Left
                  case XK_KP_Up:		/* \033Ox or standard */
                  case XK_KP_Down:	/* \033Or or standard */
                  case XK_KP_Right:	/* \033Ov or standard */
                  case XK_KP_Left:	/* \033Ot or standard */
                    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
                      {
                        STRCPY (kbuf, "\033OZ");
                        kbuf[2] = "txvr"[keysym - XK_KP_Left];
                        break;
                      }
                    else
                      /* translate to std. cursor key */
                      keysym = XK_Left + (keysym - XK_KP_Left);
                    /* FALLTHROUGH */
#endif
                  case XK_Up:	/* "\033[A" */
                  case XK_Down:	/* "\033[B" */
                  case XK_Right:	/* "\033[C" */
                  case XK_Left:	/* "\033[D" */
                    STRCPY (kbuf, "\033[Z");
                    kbuf[2] = "DACB"[keysym - XK_Left];
                    /* do Shift first */
                    if (shft)
                      kbuf[2] = "dacb"[keysym - XK_Left];
                    else if (ctrl)
                      {
                        kbuf[1] = 'O';
                        kbuf[2] = "dacb"[keysym - XK_Left];
                      }
                    else if (PrivateModes & PrivMode_aplCUR)
                      kbuf[1] = 'O';
#ifdef MULTICHAR_SET
                    //TODO: ??
                    if (Options & Opt_mc_hack)
                      {
                        int             col, row, m;

                        col = screen.cur.col;
                        row = screen.cur.row + TermWin.saveLines;
                        m = 0;
                        if (keysym == XK_Right
                            && IS_MULTI1 (screen.rend[row][col]))
                          m = 1;
                        else if (keysym == XK_Left)
                          {
                            if (col > 0)
                              {
                                if (IS_MULTI2 (screen.rend[row][col - 1]))
                                  m = 1;
                              }
                            else if (screen.cur.row > 0)
                              {
                                col = screen.tlen[--row];
                                if (col == -1)
                                  col = TermWin.ncol - 1;
                                else
                                  col--;
                                if (col > 0
                                    && IS_MULTI2 (screen.rend[row][col]))
                                  m = 1;
                              }
                          }
                        if (m)
                          MEMMOVE (kbuf + 3, kbuf, 3 + 1);
                      }
#endif
                    break;

#ifndef UNSHIFTED_SCROLLKEYS
# ifdef XK_KP_Prior
                  case XK_KP_Prior:
                    /* allow shift to override */
                    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
                      {
                        STRCPY (kbuf, "\033Oy");
                        break;
                      }
                    /* FALLTHROUGH */
# endif
                  case XK_Prior:
                    STRCPY (kbuf, "\033[5~");
                    break;
# ifdef XK_KP_Next
                  case XK_KP_Next:
                    /* allow shift to override */
                    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
                      {
                        STRCPY (kbuf, "\033Os");
                        break;
                      }
                    /* FALLTHROUGH */
# endif
                  case XK_Next:
                    STRCPY (kbuf, "\033[6~");
                    break;
#endif
                  case XK_KP_Enter:
                    /* allow shift to override */
                    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
                      {
                        STRCPY (kbuf, "\033OM");
                      }
                    else
                      {
                        kbuf[0] = '\r';
                        kbuf[1] = '\0';
                      }
                    break;

#ifdef XK_KP_Begin
                  case XK_KP_Begin:
                    STRCPY (kbuf, "\033Ou");
                    break;

                  case XK_KP_Insert:
                    STRCPY (kbuf, "\033Op");
                    break;

                  case XK_KP_Delete:
                    STRCPY (kbuf, "\033On");
                    break;
#endif
                  case XK_KP_F1:	/* "\033OP" */
                  case XK_KP_F2:	/* "\033OQ" */
                  case XK_KP_F3:	/* "\033OR" */
                  case XK_KP_F4:	/* "\033OS" */
                    STRCPY (kbuf, "\033OP");
                    kbuf[2] += (keysym - XK_KP_F1);
                    break;

                  case XK_KP_Multiply:	/* "\033Oj" : "*" */
                  case XK_KP_Add:	/* "\033Ok" : "+" */
                  case XK_KP_Separator:	/* "\033Ol" : "," */
                  case XK_KP_Subtract:	/* "\033Om" : "-" */
                  case XK_KP_Decimal:	/* "\033On" : "." */
                  case XK_KP_Divide:	/* "\033Oo" : "/" */
                  case XK_KP_0:		/* "\033Op" : "0" */
                  case XK_KP_1:		/* "\033Oq" : "1" */
                  case XK_KP_2:		/* "\033Or" : "2" */
                  case XK_KP_3:		/* "\033Os" : "3" */
                  case XK_KP_4:		/* "\033Ot" : "4" */
                  case XK_KP_5:		/* "\033Ou" : "5" */
                  case XK_KP_6:		/* "\033Ov" : "6" */
                  case XK_KP_7:		/* "\033Ow" : "7" */
                  case XK_KP_8:		/* "\033Ox" : "8" */
                  case XK_KP_9:		/* "\033Oy" : "9" */
                    /* allow shift to override */
                    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
                      {
                        STRCPY (kbuf, "\033Oj");
                        kbuf[2] += (keysym - XK_KP_Multiply);
                      }
                    else
                      {
                        kbuf[0] = ('*' + (keysym - XK_KP_Multiply));
                        kbuf[1] = '\0';
                      }
                    break;

                  case XK_Find:
                    STRCPY (kbuf, "\033[1~");
                    break;
                  case XK_Insert:
                    STRCPY (kbuf, "\033[2~");
                    break;
#ifdef DXK_Remove		/* support for DEC remove like key */
                  case DXK_Remove:
                    /* FALLTHROUGH */
#endif
                  case XK_Execute:
                    STRCPY (kbuf, "\033[3~");
                    break;
                  case XK_Select:
                    STRCPY (kbuf, "\033[4~");
                    break;
#ifdef XK_KP_End
                  case XK_KP_End:
                    /* allow shift to override */
                    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
                      {
                        STRCPY (kbuf, "\033Oq");
                        break;
                      }
                    /* FALLTHROUGH */
#endif
                  case XK_End:
                    STRCPY (kbuf, KS_END);
                    break;
#ifdef XK_KP_Home
                  case XK_KP_Home:
                    /* allow shift to override */
                    if ((PrivateModes & PrivMode_aplKP) ? !shft : shft)
                      {
                        STRCPY (kbuf, "\033Ow");
                        break;
                      }
                    /* FALLTHROUGH */
#endif
                  case XK_Home:
                    STRCPY (kbuf, KS_HOME);
                    break;

#define FKEY(n, fkey)							\
    sprintf ((char *)kbuf,"\033[%2d~", (int) ((n) + (keysym - fkey)))

                  case XK_F1:	/* "\033[11~" */
                  case XK_F2:	/* "\033[12~" */
                  case XK_F3:	/* "\033[13~" */
                  case XK_F4:	/* "\033[14~" */
                  case XK_F5:	/* "\033[15~" */
                    FKEY (11, XK_F1);
                    break;
                  case XK_F6:	/* "\033[17~" */
                  case XK_F7:	/* "\033[18~" */
                  case XK_F8:	/* "\033[19~" */
                  case XK_F9:	/* "\033[20~" */
                  case XK_F10:	/* "\033[21~" */
                    FKEY (17, XK_F6);
                    break;
                  case XK_F11:	/* "\033[23~" */
                  case XK_F12:	/* "\033[24~" */
                  case XK_F13:	/* "\033[25~" */
                  case XK_F14:	/* "\033[26~" */
                    FKEY (23, XK_F11);
                    break;
                  case XK_F15:	/* "\033[28~" */
                  case XK_F16:	/* "\033[29~" */
                    FKEY (28, XK_F15);
                    break;
                  case XK_Help:	/* "\033[28~" */
                    FKEY (28, XK_Help);
                    break;
                  case XK_Menu:	/* "\033[29~" */
                    FKEY (29, XK_Menu);
                    break;
                  case XK_F17:	/* "\033[31~" */
                  case XK_F18:	/* "\033[32~" */
                  case XK_F19:	/* "\033[33~" */
                  case XK_F20:	/* "\033[34~" */
                  case XK_F21:	/* "\033[35~" */
                  case XK_F22:	/* "\033[36~" */
                  case XK_F23:	/* "\033[37~" */
                  case XK_F24:	/* "\033[38~" */
                  case XK_F25:	/* "\033[39~" */
                  case XK_F26:	/* "\033[40~" */
                  case XK_F27:	/* "\033[41~" */
                  case XK_F28:	/* "\033[42~" */
                  case XK_F29:	/* "\033[43~" */
                  case XK_F30:	/* "\033[44~" */
                  case XK_F31:	/* "\033[45~" */
                  case XK_F32:	/* "\033[46~" */
                  case XK_F33:	/* "\033[47~" */
                  case XK_F34:	/* "\033[48~" */
                  case XK_F35:	/* "\033[49~" */
                    FKEY (31, XK_F17);
                    break;
#undef FKEY
                  default:
                    newlen = 0;
                    break;
                }
              if (newlen)
                len = STRLEN (kbuf);
            }
          /*
           * Pass meta for all function keys, if 'meta' option set
           */
#ifdef META8_OPTION
          if (meta && (meta_char == 0x80) && len > 0)
            kbuf[len - 1] |= 0x80;
#endif

        }
      else if (ctrl && keysym == XK_minus)
        {
          len = 1;
          kbuf[0] = '\037';	/* Ctrl-Minus generates ^_ (31) */
        }
      else
        {
#ifdef META8_OPTION
          /* set 8-bit on */
          if (meta && (meta_char == 0x80))
            {
              unsigned char  *ch;

              for (ch = kbuf; ch < kbuf + len; ch++)
                *ch |= 0x80;
              meta = 0;
            }
#endif
#ifdef GREEK_SUPPORT
          if (greek_mode)
            len = greek_xlat (kbuf, len);
#endif
          /* nil */ ;
        }
    }

  if (len <= 0)
    return;			/* not mapped */

  if (Options & Opt_scrollTtyKeypress)
    if (TermWin.view_start)
      {
        TermWin.view_start = 0;
        want_refresh = 1;
      }

  /*
   * these modifications only affect the static keybuffer
   * pass Shift/Control indicators for function keys ending with `~'
   *
   * eg,
   *   Prior = "ESC[5~"
   *   Shift+Prior = "ESC[5$"
   *   Ctrl+Prior = "ESC[5^"
   *   Ctrl+Shift+Prior = "ESC[5@"
   * Meta adds an Escape prefix (with META8_OPTION, if meta == <escape>).
   */
  if (kbuf[0] == C0_ESC && kbuf[1] == '[' && kbuf[len - 1] == '~')
    kbuf[len - 1] = (shft ? (ctrl ? '@' : '$') : (ctrl ? '^' : '~'));

  /* escape prefix */
  if (meta
#ifdef META8_OPTION
      && (meta_char == C0_ESC)
#endif
     )
    {
      const unsigned char ch = C0_ESC;

      tt_write (&ch, 1);
    }
#ifdef DEBUG_CMD
  if (debug_key)
    {		/* Display keyboard buffer contents */
      char           *p;
      int             i;

      fprintf (stderr, "key 0x%04X [%d]: `", (unsigned int)keysym, len);
      for (i = 0, p = kbuf; i < len; i++, p++)
        fprintf (stderr, (*p >= ' ' && *p < '\177' ? "%c" : "\\%03o"), *p);
      fprintf (stderr, "'\n");
    }
#endif				/* DEBUG_CMD */
  tt_write (kbuf, (unsigned int)len);
}
/*}}} */

#if (MENUBAR_MAX)
/*{{{ rxvt_cmd_write (), rxvt_cmd_getc () */
/* attempt to `write' count to the input buffer */
unsigned int
rxvt_term::cmd_write (const unsigned char *str, unsigned int count)
{
  unsigned int    n, s;

  n = cmdbuf_ptr - cmdbuf_base;
  s = cmdbuf_base + BUFSIZ - 1 - cmdbuf_endp;
  if (n > 0 && s < count)
    {
      MEMMOVE (cmdbuf_base, cmdbuf_ptr,
              (unsigned int) (cmdbuf_endp - cmdbuf_ptr));
      cmdbuf_ptr = cmdbuf_base;
      cmdbuf_endp -= n;
      s += n;
    }
  if (count > s)
    {
      rxvt_print_error ("data loss: cmd_write too large");
      count = s;
    }
  for (; count--;)
    *cmdbuf_endp++ = *str++;
  return 0;
}
#endif				/* MENUBAR_MAX */

void
rxvt_term::flush ()
{
#ifdef TRANSPARENT
  if (want_full_refresh)
    {
      want_full_refresh = 0;
      scr_clear ();
      scr_touch (false);
      want_refresh = 1;
    }
#endif

  if (want_refresh)
    {
      scr_refresh (refresh_type);
      scrollbar_show (1);
#ifdef USE_XIM
      IMSendSpot ();
#endif

    }

  display->flush ();
}

void
rxvt_term::check_cb (check_watcher &w)
{
  SET_R (this);
  SET_LOCALE (locale);

  flush ();
}

#ifdef CURSOR_BLINK
void
rxvt_term::cursor_blink_cb (time_watcher &w)
{
  hidden_cursor = !hidden_cursor;
  want_refresh = 1;

  w.start (w.at + BLINK_INTERVAL);
}
#endif

#ifdef TEXT_BLINK
void
rxvt_term::text_blink_cb (time_watcher &w)
{
  if (scr_refresh_rend (RS_Blink, RS_Blink))
    {
      hidden_text = !hidden_text;
      want_refresh = 1;
      w.start (w.at + TEXT_BLINK_INTERVAL);
    }
}
#endif

bool
rxvt_term::pty_fill ()
{
  ssize_t n = cmdbuf_endp - cmdbuf_ptr;

  memmove (cmdbuf_base, cmdbuf_ptr, n);
  cmdbuf_ptr = cmdbuf_base;
  cmdbuf_endp = cmdbuf_ptr + n;

  n = read (cmd_fd, cmdbuf_endp, BUFSIZ - n);

  if (n > 0)
    {
      cmdbuf_endp += n;
      return true;
    }
  else if (n < 0 && errno != EAGAIN)
    destroy ();

  return false;
}

void
rxvt_term::pty_cb (io_watcher &w, short revents)
{
  SET_R (this);
  SET_LOCALE (locale);

  if (revents & EVENT_WRITE)
    tt_write (0, 0);
  else if (revents & EVENT_READ)
    {
      bool flag = true;

      // loop, but don't allow a single term to monopolize us
      // the number of loops is fully arbitrary, and thus wrong
      while (flag && pty_fill ())
        {
          if (!seen_input)
            {
              seen_input = 1;
              /* once we know the shell is running, send the screen size.  Again! */
              tt_winch ();
            }

          uint32_t ch = NOCHAR;

          for (;;)
            {
              if (ch == NOCHAR)
                ch = next_char ();

              if (ch == NOCHAR) // TODO: improve
                break;

              if (ch >= ' ' || ch == '\t' || ch == '\n' || ch == '\r')
                {
                  /* Read a text string from the input buffer */
                  uint32_t buf[BUFSIZ];
                  bool refreshnow = false;
                  int nlines = 0;
                  uint32_t *str = buf;

                  *str++ = ch;

                  for (;;)
                    {
                      ch = next_char ();

                      if (ch == NOCHAR || (ch < ' ' && ch != '\t' && ch != '\n' && ch != '\r'))
                        break;
                      else
                        {
                          *str++ = ch;

                          if (ch == '\n')
                            {
                              nlines++;
                              refresh_count++;

                              if (! (Options & Opt_jumpScroll)
                                  || (refresh_count >= (refresh_limit * (TermWin.nrow - 1))))
                                {
                                  refreshnow = true;
                                  flag = false;
                                  ch = NOCHAR;
                                  break;
                                }

                              // scr_add_lines only works for nlines < TermWin.nrow - 1.
                              if (nlines >= TermWin.nrow - 1)
                                {
                                  scr_add_lines (buf, nlines, str - buf);
                                  nlines = 0;
                                  str = buf;
                                }
                            }

                          if (str >= buf + BUFSIZ)
                            {
                              ch = NOCHAR;
                              break;
                            }
                        }
                    }

                  scr_add_lines (buf, nlines, str - buf);

                  /*
                   * If there have been a lot of new lines, then update the screen
                   * What the heck I'll cheat and only refresh less than every page-full.
                   * the number of pages between refreshes is refresh_limit, which
                   * is incremented here because we must be doing flat-out scrolling.
                   *
                   * refreshing should be correct for small scrolls, because of the
                   * time-out
                   */
                  if (refreshnow)
                    {
                      if ((Options & Opt_jumpScroll) && refresh_limit < REFRESH_PERIOD)
                        refresh_limit++;

                      scr_refresh (refresh_type);
                    }

                }
              else
                {
                  switch (ch)
                    {
                      default:
                        process_nonprinting (ch);
                        break;
                      case C0_ESC:	/* escape char */
                        process_escape_seq ();
                        break;
                        /*case 0x9b: */	/* CSI */
                        /*  process_csi_seq (); */
                    }

                  ch = NOCHAR;
                }
            }
        }
    }
}

// read the next character, currently handles UTF-8
// will probably handle all sorts of other stuff in the future
uint32_t
rxvt_term::next_char ()
{
  while (cmdbuf_ptr < cmdbuf_endp)
    {
      if (*cmdbuf_ptr < 0x80) // assume < 0x80 to be ascii ALWAYS (all shift-states etc.) uh-oh
        return *cmdbuf_ptr++;

      wchar_t wc;
      size_t len = mbrtowc (&wc, (char *)cmdbuf_ptr, cmdbuf_endp - cmdbuf_ptr, mbstate);

      if (len == (size_t)-2)
        return NOCHAR;

      if (len == (size_t)-1)
        return *cmdbuf_ptr++; // the _occasional_ latin1 character is allowed to slip through

      // assume wchar == unicode
      cmdbuf_ptr += len;
      return wc;
    }

  return NOCHAR;
}

/* rxvt_cmd_getc () - Return next input character */
/*
 * Return the next input character after first passing any keyboard input
 * to the command.
 */
uint32_t
rxvt_term::cmd_getc ()
{
  for (;;)
    {
      uint32_t c = next_char ();
      if (c != NOCHAR)
        return c;

      // incomplete sequences should occur rarely, still, a better solution
      // would be preferred. either setjmp/longjmp or better design.
      fcntl (cmd_fd, F_SETFL, 0);
      pty_fill ();
      fcntl (cmd_fd, F_SETFL, O_NONBLOCK);
    }
}

#ifdef POINTER_BLANK
void
rxvt_term::pointer_unblank ()
{
  XDefineCursor (display->display, TermWin.vt, TermWin_cursor);
  recolour_cursor ();

  hidden_pointer = 0;

  if (Options & Opt_pointerBlank)
    pointer_ev.start (NOW + pointerBlankDelay);
}

void
rxvt_term::pointer_blank ()
{
  if (! (Options & Opt_pointerBlank))
    return;

  XDefineCursor (display->display, TermWin.vt, blank_cursor);
  XFlush (display->display);

  hidden_pointer = 1;
}

void
rxvt_term::pointer_cb (time_watcher &w)
{
  SET_R (this);
  SET_LOCALE (locale);

  pointer_blank ();
}
#endif

void
rxvt_term::mouse_report (const XButtonEvent &ev)
{
  int             button_number, key_state = 0;
  int             x, y;

  x = ev.x;
  y = ev.y;
  pixel_position (&x, &y);

  if (MEvent.button == AnyButton)
    {
      button_number = 3;
    }
  else
    {
      button_number = MEvent.button - Button1;
      /* add 0x3D for wheel events, like xterm does */
      if (button_number >= 3)
        button_number += (64 - 3);
    }

  if (PrivateModes & PrivMode_MouseX10)
    {
      /*
       * do not report ButtonRelease
       * no state info allowed
       */
      key_state = 0;
      if (button_number == 3)
        return;
    }
  else
    {
      /* XTerm mouse reporting needs these values:
       *   4 = Shift
       *   8 = Meta
       *  16 = Control
       * plus will add in our own Double-Click reporting
       *  32 = Double Click
       */
      key_state = ((MEvent.state & ShiftMask) ? 4 : 0)
                  + ((MEvent.state & ModMetaMask) ? 8 : 0)
                  + ((MEvent.state & ControlMask) ? 16 : 0);
#ifdef MOUSE_REPORT_DOUBLECLICK
      key_state += ((MEvent.clicks > 1) ? 32 : 0);
#endif

    }

#ifdef DEBUG_MOUSEREPORT
  fprintf (stderr, "Mouse [");
  if (key_state & 16)
    fputc ('C', stderr);
  if (key_state & 4)
    fputc ('S', stderr);
  if (key_state & 8)
    fputc ('A', stderr);
  if (key_state & 32)
    fputc ('2', stderr);
  fprintf (stderr, "]: <%d>, %d/%d\n",
          button_number,
          x + 1,
          y + 1);
#else
  tt_printf ("\033[M%c%c%c",
            (32 + button_number + key_state),
            (32 + x + 1),
            (32 + y + 1));
#endif
}

#ifdef USING_W11LIB
void
rxvt_W11_process_x_event (XEvent *ev)
{
  rxvt_t *r = rxvt_get_r ();

  x_cb (*ev);
}
#endif

/*{{{ process an X event */
void
rxvt_term::x_cb (XEvent &ev)
{
  SET_R (this);
  SET_LOCALE (locale);

#if defined(CURSOR_BLINK)
  if ((Options & Opt_cursorBlink) && ev.type == KeyPress)
    {
      if (hidden_cursor)
        {
          hidden_cursor = 0;
          want_refresh = 1;
        }

      cursor_blink_ev.start (NOW + BLINK_INTERVAL);
    }
#endif

#if defined(POINTER_BLANK)
  if ((Options & Opt_pointerBlank) && pointerBlankDelay > 0)
    {
      if (ev.type == MotionNotify
          || ev.type == ButtonPress
          || ev.type == ButtonRelease)
        if (hidden_pointer)
          pointer_unblank ();

      if (ev.type == KeyPress && hidden_pointer == 0)
        pointer_blank ();
    }
#endif

#ifdef USE_XIM
  if (XFilterEvent (&ev, None))
    return;
#endif

  Window          unused_root, unused_child;
  int             unused_root_x, unused_root_y;
  unsigned int    unused_mask;

#ifdef DEBUG_X
  const char *const eventnames[] =
    {				/* mason - this matches my system */
      "",
      "",
      "KeyPress",
      "KeyRelease",
      "ButtonPress",
      "ButtonRelease",
      "MotionNotify",
      "EnterNotify",
      "LeaveNotify",
      "FocusIn",
      "FocusOut",
      "KeymapNotify",
      "Expose",
      "GraphicsExpose",
      "NoExpose",
      "VisibilityNotify",
      "CreateNotify",
      "DestroyNotify",
      "UnmapNotify",
      "MapNotify",
      "MapRequest",
      "ReparentNotify",
      "ConfigureNotify",
      "ConfigureRequest",
      "GravityNotify",
      "ResizeRequest",
      "CirculateNotify",
      "CirculateRequest",
      "PropertyNotify",
      "SelectionClear",
      "SelectionRequest",
      "SelectionNotify",
      "ColormapNotify",
      "ClientMessage",
      "MappingNotify"
    };
#endif

#ifdef DEBUG_X
  struct timeval  tp;
  struct tm      *ltt;
  (void)gettimeofday (&tp, NULL);
  ltt = localtime (& (tp.tv_sec));
  D_X ((stderr, "Event: %-16s %-7s %08lx (%4d-%02d-%02d %02d:%02d:%02d.%.6ld) %s %lu", eventnames[ev.type], (ev.xany.window == TermWin.parent[0] ? "parent" : (ev.xany.window == TermWin.vt ? "vt" : (ev.xany.window == scrollBar.win ? "scroll" : (ev.xany.window == menuBar.win ? "menubar" : "UNKNOWN")))), (ev.xany.window == TermWin.parent[0] ? TermWin.parent[0] : (ev.xany.window == TermWin.vt ? TermWin.vt : (ev.xany.window == scrollBar.win ? scrollBar.win : (ev.xany.window == menuBar.win ? menuBar.win : 0)))), ltt->tm_year + 1900, ltt->tm_mon + 1, ltt->tm_mday, ltt->tm_hour, ltt->tm_min, ltt->tm_sec, tp.tv_usec, ev.xany.send_event ? "S" : " ", ev.xany.serial));
#endif

  switch (ev.type)
    {
      case KeyPress:
        lookup_key (ev.xkey);
        break;

#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
      case KeyRelease:
        {
          if (! (ev.xkey.state & ControlMask))
            mouse_slip_wheel_speed = 0;
          else
            {
              KeySym          ks;

              ks = XKeycodeToKeysym (display->display, ev.xkey.keycode, 0);
              if (ks == XK_Control_L || ks == XK_Control_R)
                mouse_slip_wheel_speed = 0;
            }
          break;
        }
#endif

      case ButtonPress:
        button_press (ev.xbutton);
        break;

      case ButtonRelease:
        button_release (ev.xbutton);
        break;

      case ClientMessage:
        if (ev.xclient.format == 32
            && (Atom)ev.xclient.data.l[0] == xa[XA_WMDELETEWINDOW])
          destroy ();
#ifdef OFFIX_DND
        /* OffiX Dnd (drag 'n' drop) protocol */
        else if (ev.xclient.message_type == xa[XA_DNDPROTOCOL]
                 && (ev.xclient.data.l[0] == DndFile
                     || ev.xclient.data.l[0] == DndDir
                     || ev.xclient.data.l[0] == DndLink))
          {
            /* Get Dnd data */
            Atom            ActualType;
            int             ActualFormat;
            unsigned char  *data;
            unsigned long   Size, RemainingBytes;

            XGetWindowProperty (display->display, display->root,
                               xa[XA_DNDSELECTION],
                               0L, 1000000L,
                               False, AnyPropertyType,
                               &ActualType, &ActualFormat,
                               &Size, &RemainingBytes,
                               &data);
            XChangeProperty (display->display, display->root,
                            XA_CUT_BUFFER0, XA_STRING,
                            8, PropModeReplace,
                            data, STRLEN (data));
            selection_paste (display->root, XA_CUT_BUFFER0, True);
            XSetInputFocus (display->display, display->root, RevertToNone, CurrentTime);
          }
#endif				/* OFFIX_DND */
        break;

      case MappingNotify:
        XRefreshKeyboardMapping (& (ev.xmapping));
        break;

        /*
         * XXX: this is not the _current_ arrangement
         * Here's my conclusion:
         * If the window is completely unobscured, use bitblt's
         * to scroll. Even then, they're only used when doing partial
         * screen scrolling. When partially obscured, we have to fill
         * in the GraphicsExpose parts, which means that after each refresh,
         * we need to wait for the graphics expose or Noexpose events,
         * which ought to make things real slow!
         */
      case VisibilityNotify:
        switch (ev.xvisibility.state)
          {
            case VisibilityUnobscured:
              refresh_type = FAST_REFRESH;
              break;
            case VisibilityPartiallyObscured:
              refresh_type = SLOW_REFRESH;
              break;
            default:
              refresh_type = NO_REFRESH;
              break;
          }
        break;

      case FocusIn:
        if (!TermWin.focus)
          {
            TermWin.focus = 1;
            want_refresh = 1;
#ifdef USE_XIM
            if (Input_Context != NULL)
              XSetICFocus (Input_Context);
#endif
#ifdef CURSOR_BLINK
            if (Options & Opt_cursorBlink)
              cursor_blink_ev.start (NOW + BLINK_INTERVAL);
#endif

          }
        break;

      case FocusOut:
        if (TermWin.focus)
          {
            TermWin.focus = 0;
            want_refresh = 1;
#ifdef USE_XIM
            if (Input_Context != NULL)
              XUnsetICFocus (Input_Context);
#endif
#ifdef CURSOR_BLINK
            if (Options & Opt_cursorBlink)
              cursor_blink_ev.stop ();
            hidden_cursor = 0;
#endif

          }
        break;

      case ConfigureNotify:
        if (ev.xconfigure.window == TermWin.parent[0])
          {
            int height, width;

            do
              {	/* Wrap lots of configures into one */
                width = ev.xconfigure.width;
                height = ev.xconfigure.height;
                D_SIZE ((stderr, "Size: ConfigureNotify: %4d x %4d", width, height));
              }
            while (XCheckTypedWindowEvent (display->display, ev.xconfigure.window, ConfigureNotify, &ev));

            if (szHint.width != width || szHint.height != height)
              {
                D_SIZE ((stderr, "Size: Resizing from: %4d x %4d", szHint.width, szHint.height));
                resize_all_windows (width, height, 1);
              }
#ifdef DEBUG_SIZE
            else
              {
                D_SIZE ((stderr, "Size: Not resizing"));
              }
#endif
#ifdef TRANSPARENT		/* XXX: maybe not needed - leave in for now */
            if (Options & Opt_transparent)
              {
                check_our_parents ();
                if (am_transparent)
                  want_full_refresh = 1;
              }
#endif

          }
        break;

      case SelectionClear:
        display->set_selection_owner (0);
        break;

      case SelectionNotify:
        if (selection_wait == Sel_normal)
          selection_paste (ev.xselection.requestor,
                           ev.xselection.property, True);
        break;

      case SelectionRequest:
        selection_send (ev.xselectionrequest);
        break;

      case UnmapNotify:
        TermWin.mapped = 0;
#ifdef TEXT_BLINK
        text_blink_ev.stop ();
#endif
        break;

      case MapNotify:
        TermWin.mapped = 1;
#ifdef TEXT_BLINK
        text_blink_ev.start (NOW + TEXT_BLINK_INTERVAL);
#endif
        break;

#ifdef TRANSPARENT
      case ReparentNotify:
        rootwin_cb (ev);
        break;
#endif				/* TRANSPARENT */

      case GraphicsExpose:
      case Expose:
        if (ev.xany.window == TermWin.vt)
          {
#ifdef NO_SLOW_LINK_SUPPORT
            scr_expose (ev.xexpose.x, ev.xexpose.y,
                        ev.xexpose.width, ev.xexpose.height, False);
#else
            // don't understand this, so commented it out
            scr_expose (ev.xexpose.x, ev.xexpose.y,
                        ev.xexpose.width, ev.xexpose.height, False);
            //scr_expose (ev.xexpose.x, 0,
            //		    ev.xexpose.width, TermWin.height, False);
#endif
            want_refresh = 1;
          }
        else
          {
            XEvent unused_event;

            while (XCheckTypedWindowEvent (display->display, ev.xany.window,
                                          Expose,
                                          &unused_event)) ;
            while (XCheckTypedWindowEvent (display->display, ev.xany.window,
                                          GraphicsExpose,
                                          &unused_event)) ;
            if (isScrollbarWindow (ev.xany.window))
              {
                scrollBar.setIdle ();
                scrollbar_show (0);
              }
#ifdef MENUBAR
            if (menubar_visible () && isMenuBarWindow (ev.xany.window))
              menubar_expose ();
#endif
#ifdef RXVT_GRAPHICS
            Gr_expose (ev.xany.window);
#endif

          }
        break;

      case MotionNotify:
#ifdef POINTER_BLANK
        if (hidden_pointer)
          pointer_unblank ();
#endif
#if MENUBAR
        if (isMenuBarWindow (ev.xany.window))
          {
            menubar_control (& (ev.xbutton));
            break;
          }
#endif
        if ((PrivateModes & PrivMode_mouse_report) && ! (bypass_keystate))
          break;

        if (ev.xany.window == TermWin.vt)
          {
            if ((ev.xbutton.state & (Button1Mask | Button3Mask)))
              {
                while (XCheckTypedWindowEvent (display->display, TermWin.vt, MotionNotify, &ev))
                  ;

                XQueryPointer (display->display, TermWin.vt,
                              &unused_root, &unused_child,
                              &unused_root_x, &unused_root_y,
                              & (ev.xbutton.x), & (ev.xbutton.y),
                              &unused_mask);
#ifdef MOUSE_THRESHOLD
                /* deal with a `jumpy' mouse */
                if ((ev.xmotion.time - MEvent.time) > MOUSE_THRESHOLD)
                  {
#endif
                    selection_extend ((ev.xbutton.x), (ev.xbutton.y),
                                      (ev.xbutton.state & Button3Mask) ? 2 : 0);
#ifdef SELECTION_SCROLLING
                    if (ev.xbutton.y < TermWin.int_bwidth
                        || Pixel2Row (ev.xbutton.y) > (TermWin.nrow-1))
                      {
                        int dist;

                        pending_scroll_selection=1;

                        /* don't clobber the current delay if we are
                         * already in the middle of scrolling.
                         */
                        if (scroll_selection_delay<=0)
                          scroll_selection_delay=SCROLLBAR_CONTINUOUS_DELAY;

                        /* save the event params so we can highlight
                         * the selection in the pending-scroll loop
                         */
                        selection_save_x=ev.xbutton.x;
                        selection_save_y=ev.xbutton.y;
                        selection_save_state=
                          (ev.xbutton.state & Button3Mask) ? 2 : 0;

                        /* calc number of lines to scroll */
                        if (ev.xbutton.y<TermWin.int_bwidth)
                          {
                            scroll_selection_dir = UP;
                            dist = TermWin.int_bwidth - ev.xbutton.y;
                          }
                        else
                          {
                            scroll_selection_dir = DN;
                            dist = ev.xbutton.y -
                                   (TermWin.int_bwidth + TermWin.height);
                          }
                        scroll_selection_lines= (Pixel2Height (dist)/
                                                SELECTION_SCROLL_LINE_SPEEDUP)+1;
                        MIN_IT (scroll_selection_lines,
                               SELECTION_SCROLL_MAX_LINES);
                      }
                    else
                      {
                        /* we are within the text window, so we
                         * shouldn't be scrolling
                         */
                        pending_scroll_selection = 0;
                      }
#endif
#ifdef MOUSE_THRESHOLD

                  }
#endif

              }
          }
        else if (isScrollbarWindow (ev.xany.window) && scrollbar_isMotion ())
          {
            while (XCheckTypedWindowEvent (display->display, scrollBar.win,
                                          MotionNotify, &ev)) ;
            XQueryPointer (display->display, scrollBar.win,
                          &unused_root, &unused_child,
                          &unused_root_x, &unused_root_y,
                          & (ev.xbutton.x), & (ev.xbutton.y),
                          &unused_mask);
            scr_move_to (scrollbar_position (ev.xbutton.y) - csrO,
                         scrollbar_size ());
            scr_refresh (refresh_type);
            refresh_limit = 0;
            scrollbar_show (1);
          }
        break;
    }
}

void
rxvt_term::rootwin_cb (XEvent &ev)
{
  SET_R (this);
  SET_LOCALE (locale);

  switch (ev.type)
    {
      case PropertyNotify:
        if (ev.xproperty.atom == xa[XA_VT_SELECTION])
          {
            if (ev.xproperty.state == PropertyNewValue)
              selection_property (ev.xproperty.window, ev.xproperty.atom);
            break;
          }
#ifdef TRANSPARENT
        else
          {
            /*
             * if user used some Esetroot compatible prog to set the root bg,
             * use the property to determine the pixmap.  We use it later on.
             */
            if (xa[XA_XROOTPMAPID] == 0)
              xa[XA_XROOTPMAPID] = XInternAtom (display->display, "_XROOTPMAP_ID", False);

            if (ev.xproperty.atom != xa[XA_XROOTPMAPID])
              return;
          }

        /* FALLTHROUGH */
      case ReparentNotify:
        if ((Options & Opt_transparent) && check_our_parents ())
          if (am_transparent)
            want_full_refresh = 1;
        break;
#endif
    }
}

void
rxvt_term::button_press (const XButtonEvent &ev)
{
  int reportmode = 0, clickintime;

  bypass_keystate = ev.state & (ModMetaMask | ShiftMask);
  if (!bypass_keystate)
    reportmode = !! (PrivateModes & PrivMode_mouse_report);
  /*
   * VT window processing of button press
   */
  if (ev.window == TermWin.vt)
    {
#if RXVT_GRAPHICS
      if (ev.subwindow != None)
        rxvt_Gr_ButtonPress (ev.x, ev.y);
      else
#endif

        {
          clickintime = ev.time - MEvent.time < MULTICLICK_TIME;
          if (reportmode)
            {
              /* mouse report from vt window */
              /* save the xbutton state (for ButtonRelease) */
              MEvent.state = ev.state;
#ifdef MOUSE_REPORT_DOUBLECLICK
              if (ev.button == MEvent.button && clickintime)
                {
                  /* same button, within alloted time */
                  MEvent.clicks++;
                  if (MEvent.clicks > 1)
                    {
                      /* only report double clicks */
                      MEvent.clicks = 2;
                      mouse_report (ev);

                      /* don't report the release */
                      MEvent.clicks = 0;
                      MEvent.button = AnyButton;
                    }
                }
              else
                {
                  /* different button, or time expired */
                  MEvent.clicks = 1;
                  MEvent.button = ev.button;
                  mouse_report (ev);
                }
#else
              MEvent.button = ev.button;
              mouse_report (ev);
#endif				/* MOUSE_REPORT_DOUBLECLICK */

            }
          else
            {
              if (ev.button != MEvent.button)
                MEvent.clicks = 0;
              switch (ev.button)
                {
                  case Button1:
                    /* allow shift+left click to extend selection */
                    if (ev.state & ShiftMask && ! (PrivateModes & PrivMode_mouse_report))
                      {
                        if (MEvent.button == Button1 && clickintime)
                          selection_rotate (ev.x, ev.y);
                        else
                          selection_extend (ev.x, ev.y, 1);
                      }
                    else
                      {
                        if (MEvent.button == Button1 && clickintime)
                          MEvent.clicks++;
                        else
                          MEvent.clicks = 1;

                        selection_click (MEvent.clicks, ev.x, ev.y);
                      }

                    MEvent.button = Button1;
                    break;

                  case Button3:
                    if (MEvent.button == Button3 && clickintime)
                      selection_rotate (ev.x, ev.y);
                    else
                      selection_extend (ev.x, ev.y, 1);
                    MEvent.button = Button3;
                    break;
                }
            }
          MEvent.time = ev.time;
          return;
        }
    }

  /*
   * Scrollbar window processing of button press
   */
  if (isScrollbarWindow (ev.window))
    {
      scrollBar.setIdle ();
      /*
       * Rxvt-style scrollbar:
       * move up if mouse is above slider
       * move dn if mouse is below slider
       *
       * XTerm-style scrollbar:
       * Move display proportional to pointer location
       * pointer near top -> scroll one line
       * pointer near bot -> scroll full page
       */
#ifndef NO_SCROLLBAR_REPORT
      if (reportmode)
        {
          /*
           * Mouse report disabled scrollbar:
           * arrow buttons - send up/down
           * click on scrollbar - send pageup/down
           */
          if ((scrollBar.style == R_SB_NEXT
               && scrollbarnext_upButton (ev.y))
              || (scrollBar.style == R_SB_RXVT
                  && scrollbarrxvt_upButton (ev.y)))
            tt_printf ("\033[A");
          else if ((scrollBar.style == R_SB_NEXT
                    && scrollbarnext_dnButton (ev.y))
                   || (scrollBar.style == R_SB_RXVT
                       && scrollbarrxvt_dnButton (ev.y)))
            tt_printf ("\033[B");
          else
            switch (ev.button)
              {
                case Button2:
                  tt_printf ("\014");
                  break;
                case Button1:
                  tt_printf ("\033[6~");
                  break;
                case Button3:
                  tt_printf ("\033[5~");
                  break;
              }
        }
      else
#endif				/* NO_SCROLLBAR_REPORT */

        {
          char            upordown = 0;

          if (scrollBar.style == R_SB_NEXT)
            {
              if (scrollbarnext_upButton (ev.y))
                upordown = -1;	/* up */
              else if (scrollbarnext_dnButton (ev.y))
                upordown = 1;	/* down */
            }
          else if (scrollBar.style == R_SB_RXVT)
            {
              if (scrollbarrxvt_upButton (ev.y))
                upordown = -1;	/* up */
              else if (scrollbarrxvt_dnButton (ev.y))
                upordown = 1;	/* down */
            }
          if (upordown)
            {
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
              scroll_arrow_delay = SCROLLBAR_INITIAL_DELAY;
#endif
              if (scr_page (upordown < 0 ? UP : DN, 1))
                {
                  if (upordown < 0)
                    scrollBar.setUp ();
                  else
                    scrollBar.setDn ();
                }
            }
          else
            switch (ev.button)
              {
                case Button2:
                  switch (scrollbar_align)
                    {
                      case R_SB_ALIGN_TOP:
                        csrO = 0;
                        break;
                      case R_SB_ALIGN_CENTRE:
                        csrO = (scrollBar.bot - scrollBar.top) / 2;
                        break;
                      case R_SB_ALIGN_BOTTOM:
                        csrO = scrollBar.bot - scrollBar.top;
                        break;
                    }
                  if (scrollBar.style == R_SB_XTERM
                      || scrollbar_above_slider (ev.y)
                      || scrollbar_below_slider (ev.y))
                    scr_move_to (					 scrollbar_position (ev.y) - csrO,
                                       scrollbar_size ());
                  scrollBar.setMotion ();
                  break;

                case Button1:
                  if (scrollbar_align == R_SB_ALIGN_CENTRE)
                    csrO = ev.y - scrollBar.top;
                  /* FALLTHROUGH */

                case Button3:
                  if (scrollBar.style != R_SB_XTERM)
                    {
                      if (scrollbar_above_slider (ev.y))
# ifdef RXVT_SCROLL_FULL
                        scr_page (UP, TermWin.nrow - 1);
# else
                        scr_page (UP, TermWin.nrow / 4);
# endif
                      else if (scrollbar_below_slider (ev.y))
# ifdef RXVT_SCROLL_FULL
                        scr_page (DN, TermWin.nrow - 1);
# else
                        scr_page (DN, TermWin.nrow / 4);
# endif
                      else
                        scrollBar.setMotion ();
                    }
                  else
                    {
                      scr_page ((ev.button == Button1 ? DN : UP),
                                (TermWin.nrow
                                 * scrollbar_position (ev.y)
                                 / scrollbar_size ()));
                    }
                  break;
              }
        }
      return;
    }
#if MENUBAR
  /*
   * Menubar window processing of button press
   */
  if (isMenuBarWindow (ev.window))
    menubar_control (ev);
#endif
}

void
rxvt_term::button_release (const XButtonEvent &ev)
{
  int reportmode = 0;

  csrO = 0;		/* reset csr Offset */
  if (!bypass_keystate)
    reportmode = !! (PrivateModes & PrivMode_mouse_report);

  if (scrollbar_isUpDn ())
    {
      scrollBar.setIdle ();
      scrollbar_show (0);
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
      refresh_type &= ~SMOOTH_REFRESH;
#endif

    }
#ifdef SELECTION_SCROLLING
  pending_scroll_selection=0;
#endif
  if (ev.window == TermWin.vt)
    {
#ifdef RXVT_GRAPHICS
      if (ev.subwindow != None)
        rxvt_Gr_ButtonRelease (ev.x, ev.y);
      else
#endif

        {
          if (reportmode)
            {
              /* mouse report from vt window */
              /* don't report release of wheel "buttons" */
              if (ev.button >= 4)
                return;
#ifdef MOUSE_REPORT_DOUBLECLICK
              /* only report the release of 'slow' single clicks */
              if (MEvent.button != AnyButton
                  && (ev.button != MEvent.button
                      || (ev.time - MEvent.time
                          > MULTICLICK_TIME / 2)))
                {
                  MEvent.clicks = 0;
                  MEvent.button = AnyButton;
                  mouse_report (ev);
                }
#else				/* MOUSE_REPORT_DOUBLECLICK */
              MEvent.button = AnyButton;
              mouse_report (ev);
#endif				/* MOUSE_REPORT_DOUBLECLICK */
              return;
            }
          /*
           * dumb hack to compensate for the failure of click-and-drag
           * when overriding mouse reporting
           */
          if (PrivateModes & PrivMode_mouse_report
              && bypass_keystate
              && ev.button == Button1 && MEvent.clicks <= 1)
            selection_extend (ev.x, ev.y, 0);

          switch (ev.button)
            {
              case Button1:
              case Button3:
                selection_make (ev.time);
                break;
              case Button2:
                selection_request (ev.time, ev.x, ev.y);
                break;
#ifdef MOUSE_WHEEL
              case Button4:
              case Button5:
                {
                  int i;
                  page_dirn v;

                  v = (ev.button == Button4) ? UP : DN;
                  if (ev.state & ShiftMask)
                    i = 1;
                  else if ((Options & Opt_mouseWheelScrollPage))
                    i = TermWin.nrow - 1;
                  else
                    i = 5;
# ifdef MOUSE_SLIP_WHEELING
                  if (ev.state & ControlMask)
                    {
                      mouse_slip_wheel_speed += (v ? -1 : 1);
                      mouse_slip_wheel_delay = SCROLLBAR_CONTINUOUS_DELAY;
                    }
# endif
# ifdef JUMP_MOUSE_WHEEL
                  scr_page (v, i);
                  scr_refresh (SMOOTH_REFRESH);
                  scrollbar_show (1);
# else
                  while (i--)
                    {
                      scr_page (v, 1);
                      scr_refresh (SMOOTH_REFRESH);
                      scrollbar_show (1);
                    }
# endif

                }
                break;
#endif

            }
        }
    }
#ifdef MENUBAR
  else if (isMenuBarWindow (ev.window))
    menubar_control (ev);
#endif
}


#ifdef TRANSPARENT
/*
 * Check our parents are still who we think they are.
 * Do transparency updates if required
 */
int
rxvt_term::check_our_parents ()
{
  int             i, pchanged, aformat, have_pixmap, rootdepth;
  unsigned long   nitems, bytes_after;
  Atom            atype;
  unsigned char   *prop = NULL;
  Window          root, oldp, *list;
  Pixmap          rootpixmap = None;
  XWindowAttributes wattr, wrootattr;

  pchanged = 0;

  if (! (Options & Opt_transparent))
    return pchanged;	/* Don't try any more */

  XGetWindowAttributes (display->display, display->root, &wrootattr);
  rootdepth = wrootattr.depth;

  XGetWindowAttributes (display->display, TermWin.parent[0], &wattr);
  if (rootdepth != wattr.depth)
    {
      if (am_transparent)
        {
          pchanged = 1;
          XSetWindowBackground (display->display, TermWin.vt,
                               PixColors[Color_bg]);
          am_transparent = am_pixmap_trans = 0;
        }
      return pchanged;	/* Don't try any more */
    }

  /* Get all X ops out of the queue so that our information is up-to-date. */
  XSync (display->display, False);

  /*
   * Make the frame window set by the window manager have
   * the root background. Some window managers put multiple nested frame
   * windows for each client, so we have to take care about that.
   */
  i = (xa[XA_XROOTPMAPID] != 0
       && (XGetWindowProperty (display->display, display->root, xa[XA_XROOTPMAPID],
                              0L, 1L, False, XA_PIXMAP, &atype, &aformat,
                              &nitems, &bytes_after, &prop) == Success));
  if (!i || prop == NULL)
    have_pixmap = 0;
  else
    {
      have_pixmap = 1;
      rootpixmap = * ((Pixmap *)prop);
      XFree (prop);
    }
  if (have_pixmap)
    {
      /*
       * Copy display->root pixmap transparency
       */
      int             sx, sy, nx, ny;
      unsigned int    nw, nh;
      Window          cr;
      XImage         *image;
      GC              gc;
      XGCValues       gcvalue;

      XTranslateCoordinates (display->display, TermWin.parent[0], display->root,
                            0, 0, &sx, &sy, &cr);
      nw = (unsigned int)szHint.width;
      nh = (unsigned int)szHint.height;
      nx = ny = 0;
      if (sx < 0)
        {
          nw += sx;
          nx = -sx;
          sx = 0;
        }
      if (sy < 0)
        {
          nh += sy;
          ny = -sy;
          sy = 0;
        }
      MIN_IT (nw, (unsigned int) (wrootattr.width - sx));
      MIN_IT (nh, (unsigned int) (wrootattr.height - sy));
      allowedxerror = -1;
      image = XGetImage (display->display, rootpixmap, sx, sy, nw, nh, AllPlanes,
                        ZPixmap);
      /* XXX: handle BadMatch - usually because we're outside the pixmap */
      /* XXX: may need a delay here? */
      allowedxerror = 0;
      if (image == NULL)
        {
          if (am_transparent && am_pixmap_trans)
            {
              pchanged = 1;
              if (TermWin.pixmap != None)
                {
                  XFreePixmap (display->display, TermWin.pixmap);
                  TermWin.pixmap = None;
                }
            }
          am_pixmap_trans = 0;
        }
      else
        {
          if (TermWin.pixmap != None)
            XFreePixmap (display->display, TermWin.pixmap);
          TermWin.pixmap = XCreatePixmap (display->display, TermWin.vt,
                                         (unsigned int)szHint.width,
                                         (unsigned int)szHint.height,
                                         (unsigned int)image->depth);
          gc = XCreateGC (display->display, TermWin.vt, 0UL, &gcvalue);
          XPutImage (display->display, TermWin.pixmap, gc, image, 0, 0,
                    nx, ny, (unsigned int)image->width,
                    (unsigned int)image->height);
          XFreeGC (display->display, gc);
          XDestroyImage (image);
          XSetWindowBackgroundPixmap (display->display, TermWin.vt,
                                     TermWin.pixmap);
          if (!am_transparent || !am_pixmap_trans)
            pchanged = 1;
          am_transparent = am_pixmap_trans = 1;
        }
    }
  if (!am_pixmap_trans)
    {
      unsigned int    n;
      /*
       * InheritPixmap transparency
       */
      D_X ((stderr, "InheritPixmap Seeking to  %08lx", display->root));
      for (i = 1; i < (int) (sizeof (TermWin.parent) / sizeof (Window));
           i++)
        {
          oldp = TermWin.parent[i];
          XQueryTree (display->display, TermWin.parent[i - 1], &root,
                     &TermWin.parent[i], &list, &n);
          XFree (list);
          D_X ((stderr, "InheritPixmap Parent[%d] = %08lx", i, TermWin.parent[i]));
          if (TermWin.parent[i] == display->root)
            {
              if (oldp != None)
                pchanged = 1;
              break;
            }
          if (oldp != TermWin.parent[i])
            pchanged = 1;
        }
      n = 0;
      if (pchanged)
        {
          for (; n < (unsigned int)i; n++)
            {
              XGetWindowAttributes (display->display, TermWin.parent[n], &wattr);
              D_X ((stderr, "InheritPixmap Checking Parent[%d]: %s", n, (wattr.depth == rootdepth && wattr.class != InputOnly) ? "OK" : "FAIL"));
              if (wattr.depth != rootdepth || wattr.c_class == InputOnly)
                {
                  n = (int) (sizeof (TermWin.parent) / sizeof (Window)) + 1;
                  break;
                }
            }
        }
      if (n > (int) (sizeof (TermWin.parent)
                    / sizeof (TermWin.parent[0])))
        {
          D_X ((stderr, "InheritPixmap Turning off"));
          XSetWindowBackground (display->display, TermWin.parent[0],
                               PixColors[Color_fg]);
          XSetWindowBackground (display->display, TermWin.vt,
                               PixColors[Color_bg]);
          am_transparent = 0;
          /* XXX: also turn off Opt_transparent? */
        }
      else
        {
          /* wait (an arbitrary period) for the WM to do its thing
           * needed for fvwm2.2.2 (and before?) */
# ifdef HAVE_NANOSLEEP
          struct timespec rqt;

          rqt.tv_sec = 1;
          rqt.tv_nsec = 0;
          nanosleep (&rqt, NULL);
# else
          sleep (1);
# endif
          D_X ((stderr, "InheritPixmap Turning on (%d parents)", i - 1));
          for (n = 0; n < (unsigned int)i; n++)
            XSetWindowBackgroundPixmap (display->display, TermWin.parent[n],
                                       ParentRelative);
          XSetWindowBackgroundPixmap (display->display, TermWin.vt,
                                     ParentRelative);
          am_transparent = 1;
        }
      for (; i < (int) (sizeof (TermWin.parent) / sizeof (Window)); i++)
        TermWin.parent[i] = None;
    }
  return pchanged;
}
#endif

/*}}} */

/*{{{ print pipe */
/*----------------------------------------------------------------------*/
#ifdef PRINTPIPE
FILE *
rxvt_term::popen_printer ()
{
  FILE           *stream = popen (rs[Rs_print_pipe], "w");

  if (stream == NULL)
    rxvt_print_error ("can't open printer pipe");
  return stream;
}

int
rxvt_term::pclose_printer (FILE *stream)
{
  fflush (stream);
  /* pclose () reported not to work on SunOS 4.1.3 */
# if defined (__sun__)		/* TODO: RESOLVE THIS */
  /* pclose works provided SIGCHLD handler uses waitpid */
  return pclose (stream);	/* return fclose (stream); */
# else
  return pclose (stream);
# endif
}

/*
 * simulate attached vt100 printer
 */
void
rxvt_term::process_print_pipe ()
{
  int             done;
  FILE           *fd;

  if ((fd = popen_printer ()) == NULL)
    return;

  /*
   * Send all input to the printer until either ESC[4i or ESC[?4i
   * is received.
   */
  for (done = 0; !done;)
    {
      unsigned char   buf[8];
      unsigned char   ch;
      unsigned int    i, len;

      if ((ch = cmd_getc ()) != C0_ESC)
        {
          if (putc (ch, fd) == EOF)
            break;		/* done = 1 */
        }
      else
        {
          len = 0;
          buf[len++] = ch;

          if ((buf[len++] = cmd_getc ()) == '[')
            {
              if ((ch = cmd_getc ()) == '?')
                {
                  buf[len++] = '?';
                  ch = cmd_getc ();
                }
              if ((buf[len++] = ch) == '4')
                {
                  if ((buf[len++] = cmd_getc ()) == 'i')
                    break;	/* done = 1 */
                }
            }
          for (i = 0; i < len; i++)
            if (putc (buf[i], fd) == EOF)
              {
                done = 1;
                break;
              }
        }
    }
  pclose_printer (fd);
}
#endif				/* PRINTPIPE */
/*}}} */

/* *INDENT-OFF* */
enum {
  C1_40 = 0x40,
  C1_41 , C1_BPH, C1_NBH, C1_44 , C1_NEL, C1_SSA, C1_ESA,
  C1_HTS, C1_HTJ, C1_VTS, C1_PLD, C1_PLU, C1_RI , C1_SS2, C1_SS3,
  C1_DCS, C1_PU1, C1_PU2, C1_STS, C1_CCH, C1_MW , C1_SPA, C1_EPA,
  C1_SOS, C1_59 , C1_SCI, C1_CSI, CS_ST , C1_OSC, C1_PM , C1_APC
};
/* *INDENT-ON* */

/*{{{ process non-printing single characters */
void
rxvt_term::process_nonprinting (unsigned char ch)
{
  switch (ch)
    {
      case C0_ENQ:	/* terminal Status */
        if (rs[Rs_answerbackstring])
          tt_write (
            (const unsigned char *)rs[Rs_answerbackstring],
            (unsigned int)STRLEN (rs[Rs_answerbackstring]));
        else
          tt_write ((unsigned char *)VT100_ANS,
                   (unsigned int)STRLEN (VT100_ANS));
        break;
      case C0_BEL:	/* bell */
        scr_bell ();
        break;
      case C0_BS:		/* backspace */
        scr_backspace ();
        break;
      case C0_HT:		/* tab */
        scr_tab (1);
        break;
      case C0_CR:		/* carriage return */
        scr_gotorc (0, 0, R_RELATIVE);
        break;
      case C0_VT:		/* vertical tab, form feed */
      case C0_FF:
      case C0_LF:		/* line feed */
        scr_index (UP);
        break;
      case C0_SO:		/* shift out - acs */
        scr_charset_choose (1);
        break;
      case C0_SI:		/* shift in - acs */
        scr_charset_choose (0);
        break;
    }
}
/*}}} */


/*{{{ process VT52 escape sequences */
void
rxvt_term::process_escape_vt52 (unsigned char ch)
{
  int row, col;

  switch (ch)
    {
      case 'A':		/* cursor up */
        scr_gotorc (-1, 0, R_RELATIVE | C_RELATIVE);
        break;
      case 'B':		/* cursor down */
        scr_gotorc (1, 0, R_RELATIVE | C_RELATIVE);
        break;
      case 'C':		/* cursor right */
        scr_gotorc (0, 1, R_RELATIVE | C_RELATIVE);
        break;
      case 'D':		/* cursor left */
        scr_gotorc (0, -1, R_RELATIVE | C_RELATIVE);
        break;
      case 'H':		/* cursor home */
        scr_gotorc (0, 0, 0);
        break;
      case 'I':		/* cursor up and scroll down if needed */
        scr_index (DN);
        break;
      case 'J':		/* erase to end of screen */
        scr_erase_screen (0);
        break;
      case 'K':		/* erase to end of line */
        scr_erase_line (0);
        break;
      case 'Y':         	/* move to specified row and col */
        /* full command is 'ESC Y row col' where row and col
         * are encoded by adding 32 and sending the ascii
         * character.  eg. SPACE = 0, '+' = 13, '0' = 18,
         * etc. */
        row = cmd_getc () - ' ';
        col = cmd_getc () - ' ';
        scr_gotorc (row, col, 0);
        break;
      case 'Z':		/* identify the terminal type */
        tt_printf ("\033/Z");	/* I am a VT100 emulating a VT52 */
        break;
      case '<':		/* turn off VT52 mode */
        PrivMode (0, PrivMode_vt52);
        break;
      case 'F':     	/* use special graphics character set */
      case 'G':           /* use regular character set */
        /* unimplemented */
        break;
      case '=':     	/* use alternate keypad mode */
      case '>':           /* use regular keypad mode */
        /* unimplemented */
        break;
    }
}
/*}}} */


/*{{{ process escape sequences */
void
rxvt_term::process_escape_seq ()
{
  unsigned char   ch = cmd_getc ();

  if (PrivateModes & PrivMode_vt52)
    {
      process_escape_vt52 (ch);
      return;
    }

  switch (ch)
    {
        /* case 1:        do_tek_mode (); break; */
      case '#':
        if (cmd_getc () == '8')
          scr_E ();
        break;
      case '(':
        scr_charset_set (0, (unsigned int)cmd_getc ());
        break;
      case ')':
        scr_charset_set (1, (unsigned int)cmd_getc ());
        break;
      case '*':
        scr_charset_set (2, (unsigned int)cmd_getc ());
        break;
      case '+':
        scr_charset_set (3, (unsigned int)cmd_getc ());
        break;
#ifdef MULTICHAR_SET
      case '$':
        scr_charset_set (-2, (unsigned int)cmd_getc ());
        break;
#endif
#ifndef NO_FRILLS
      case '6':
        scr_backindex ();
        break;
#endif
      case '7':
        scr_cursor (SAVE);
        break;
      case '8':
        scr_cursor (RESTORE);
        break;
#ifndef NO_FRILLS
      case '9':
        scr_forwardindex ();
        break;
#endif
      case '=':
      case '>':
        PrivMode ((ch == '='), PrivMode_aplKP);
        break;

      case C1_40:
        cmd_getc ();
        break;
      case C1_44:
        scr_index (UP);
        break;

        /* 8.3.87: NEXT LINE */
      case C1_NEL:		/* ESC E */
        {
          uint32_t nlcr[] = { '\n', '\r' };
          scr_add_lines (nlcr, 1, 2);
        }
        break;

        /* kidnapped escape sequence: Should be 8.3.48 */
      case C1_ESA:		/* ESC G */
        process_graphics ();
        break;

        /* 8.3.63: CHARACTER TABULATION SET */
      case C1_HTS:		/* ESC H */
        scr_set_tab (1);
        break;

        /* 8.3.105: REVERSE LINE FEED */
      case C1_RI:			/* ESC M */
        scr_index (DN);
        break;

        /* 8.3.142: SINGLE-SHIFT TWO */
        /*case C1_SS2: scr_single_shift (2);   break; */

        /* 8.3.143: SINGLE-SHIFT THREE */
        /*case C1_SS3: scr_single_shift (3);   break; */

        /* 8.3.27: DEVICE CONTROL STRING */
      case C1_DCS:		/* ESC P */
        process_dcs_seq ();
        break;

        /* 8.3.110: SINGLE CHARACTER INTRODUCER */
      case C1_SCI:		/* ESC Z */
        tt_write ((const unsigned char *)ESCZ_ANSWER,
                 (unsigned int) (sizeof (ESCZ_ANSWER) - 1));
        break;			/* steal obsolete ESC [ c */

        /* 8.3.16: CONTROL SEQUENCE INTRODUCER */
      case C1_CSI:		/* ESC [ */
        process_csi_seq ();
        break;

        /* 8.3.90: OPERATING SYSTEM COMMAND */
      case C1_OSC:		/* ESC ] */
        process_osc_seq ();
        break;

        /* 8.3.106: RESET TO INITIAL STATE */
      case 'c':
        scr_poweron ();
        scrollbar_show (1);
        break;

        /* 8.3.79: LOCKING-SHIFT TWO (see ISO2022) */
      case 'n':
        scr_charset_choose (2);
        break;

        /* 8.3.81: LOCKING-SHIFT THREE (see ISO2022) */
      case 'o':
        scr_charset_choose (3);
        break;
    }
}
/*}}} */

/*{{{ process CONTROL SEQUENCE INTRODUCER (CSI) sequences `ESC[' */
/* *INDENT-OFF* */
enum {
  CSI_ICH = 0x40,
  CSI_CUU, CSI_CUD, CSI_CUF, CSI_CUB, CSI_CNL, CSI_CPL, CSI_CHA,
  CSI_CUP, CSI_CHT, CSI_ED , CSI_EL , CSI_IL , CSI_DL , CSI_EF , CSI_EA ,
  CSI_DCH, CSI_SEE, CSI_CPR, CSI_SU , CSI_SD , CSI_NP , CSI_PP , CSI_CTC,
  CSI_ECH, CSI_CVT, CSI_CBT, CSI_SRS, CSI_PTX, CSI_SDS, CSI_SIMD, CSI_5F,
  CSI_HPA, CSI_HPR, CSI_REP, CSI_DA , CSI_VPA, CSI_VPR, CSI_HVP, CSI_TBC,
  CSI_SM , CSI_MC , CSI_HPB, CSI_VPB, CSI_RM , CSI_SGR, CSI_DSR, CSI_DAQ,
  CSI_70 , CSI_71 , CSI_72 , CSI_73 , CSI_74 , CSI_75 , CSI_76 , CSI_77 ,
  CSI_78 , CSI_79 , CSI_7A , CSI_7B , CSI_7C , CSI_7D , CSI_7E , CSI_7F
};

#define make_byte(b7,b6,b5,b4,b3,b2,b1,b0)			\
    (((b7) << 7) | ((b6) << 6) | ((b5) << 5) | ((b4) << 4)	\
     | ((b3) << 3) | ((b2) << 2) | ((b1) << 1) | (b0))
#define get_byte_array_bit(array, bit)				\
    (!! ((array)[ (bit) / 8] & (128 >> ((bit) & 7))))

const unsigned char csi_defaults[] =
  {
    make_byte (1,1,1,1,1,1,1,1),	/* @, A, B, C, D, E, F, G, */
    make_byte (1,1,0,0,1,1,0,0),	/* H, I, J, K, L, M, N, O, */
    make_byte (1,0,1,1,1,1,1,0),	/* P, Q, R, S, T, U, V, W, */
    make_byte (1,1,1,0,0,0,1,0),	/* X, Y, Z, [, \, ], ^, _, */
    make_byte (1,1,1,0,1,1,1,0),	/* `, a, b, c, d, e, f, g, */
    make_byte (0,0,1,1,0,0,0,0),	/* h, i, j, k, l, m, n, o, */
    make_byte (0,0,0,0,0,0,0,0),	/* p, q, r, s, t, u, v, w, */
    make_byte (0,0,0,0,0,0,0,0)	/* x, y, z, {, |, }, ~,    */
  };
/* *INDENT-ON* */

void
rxvt_term::process_csi_seq ()
{
  unsigned char   ch, priv, i;
  unsigned int    nargs, p;
  int             n, ndef;
  int             arg[ESC_ARGS];

  for (nargs = ESC_ARGS; nargs > 0;)
    arg[--nargs] = 0;

  priv = 0;
  ch = cmd_getc ();
  if (ch >= '<' && ch <= '?')
    {	/* '<' '=' '>' '?' */
      priv = ch;
      ch = cmd_getc ();
    }
  /* read any numerical arguments */
  for (n = -1; ch < CSI_ICH; )
    {
      if (isdigit (ch))
        {
          if (n < 0)
            n = ch - '0';
          else
            n = n * 10 + ch - '0';
        }
      else if (ch == ';')
        {
          if (nargs < ESC_ARGS)
            arg[nargs++] = n;
          n = -1;
        }
      else if (ch == '\b')
        {
          scr_backspace ();
        }
      else if (ch == C0_ESC)
        {
          process_escape_seq ();
          return;
        }
      else if (ch < ' ')
        {
          process_nonprinting (ch);
        }
      ch = cmd_getc ();
    }

  if (ch > CSI_7F)
    return;

  if (nargs < ESC_ARGS)
    arg[nargs++] = n;

  i = ch - CSI_ICH;
  ndef = get_byte_array_bit (csi_defaults, i);
  for (p = 0; p < nargs; p++)
    if (arg[p] == -1)
      arg[p] = ndef;

#ifdef DEBUG_CMD
  fprintf (stderr, "CSI ");
  for (p = 0; p < nargs; p++)
    fprintf (stderr, "%d%s", arg[p], p < nargs - 1 ? ";" : "");
  fprintf (stderr, "%c\n", ch);
#endif

  /*
   * private mode handling
   */
  if (priv)
    {
      switch (priv)
        {
          case '>':
            if (ch == CSI_DA)	/* secondary device attributes */
              tt_printf ("\033[>%d;%-.8s;0c", 'R', VSTRING);
            break;
          case '?':
            if (ch == 'h' || ch == 'l' || ch == 'r' || ch == 's' || ch == 't')
              process_terminal_mode (ch, priv, nargs, arg);
            break;
        }
      return;
    }

  switch (ch)
    {
        /*
         * ISO/IEC 6429:1992 (E) CSI sequences (defaults in parentheses)
         */
#ifdef PRINTPIPE
      case CSI_MC:		/* 8.3.83: (0) MEDIA COPY */
        switch (arg[0])
          {
            case 0:			/* initiate transfer to primary aux device */
              scr_printscreen (0);
              break;
            case 5:			/* start relay to primary aux device */
              process_print_pipe ();
              break;
          }
        break;
#endif

      case CSI_CUU:		/* 8.3.22: (1) CURSOR UP */
      case CSI_VPR:		/* 8.3.161: (1) LINE POSITION FORWARD */
        arg[0] = -arg[0];
        /* FALLTHROUGH */
      case CSI_CUD:		/* 8.3.19: (1) CURSOR DOWN */
      case CSI_VPB:		/* 8.3.160: (1) LINE POSITION BACKWARD */
        scr_gotorc (arg[0], 0, RELATIVE);
        break;

      case CSI_CUB:		/* 8.3.18: (1) CURSOR LEFT */
      case CSI_HPB: 		/* 8.3.59: (1) CHARACTER POSITION BACKWARD */
#ifdef ISO6429
        arg[0] = -arg[0];
#else				/* emulate common DEC VTs */
        arg[0] = arg[0] ? -arg[0] : -1;
#endif
        /* FALLTHROUGH */
      case CSI_CUF:		/* 8.3.20: (1) CURSOR RIGHT */
      case CSI_HPR:		/* 8.3.60: (1) CHARACTER POSITION FORWARD */
#ifdef ISO6429
        scr_gotorc (0, arg[0], RELATIVE);
#else				/* emulate common DEC VTs */
        scr_gotorc (0, arg[0] ? arg[0] : 1, RELATIVE);
#endif
        break;

      case CSI_CPL:		/* 8.3.13: (1) CURSOR PRECEDING LINE */
        arg[0] = -arg[0];
        /* FALLTHROUGH */
      case CSI_CNL:		/* 8.3.12: (1) CURSOR NEXT LINE */
        scr_gotorc (arg[0], 0, R_RELATIVE);
        break;

      case CSI_CHA:		/* 8.3.9: (1) CURSOR CHARACTER ABSOLUTE */
      case CSI_HPA:		/* 8.3.58: (1) CURSOR POSITION ABSOLUTE */
        scr_gotorc (0, arg[0] - 1, R_RELATIVE);
        break;

      case CSI_VPA:		/* 8.3.159: (1) LINE POSITION ABSOLUTE */
        scr_gotorc (arg[0] - 1, 0, C_RELATIVE);
        break;

      case CSI_CUP:		/* 8.3.21: (1,1) CURSOR POSITION */
      case CSI_HVP:		/* 8.3.64: (1,1) CHARACTER AND LINE POSITION */
        scr_gotorc (arg[0] - 1, nargs < 2 ? 0 : (arg[1] - 1), 0);
        break;

      case CSI_CBT:		/* 8.3.7: (1) CURSOR BACKWARD TABULATION */
        arg[0] = -arg[0];
        /* FALLTHROUGH */
      case CSI_CHT:		/* 8.3.10: (1) CURSOR FORWARD TABULATION */
        scr_tab (arg[0]);
        break;

      case CSI_ED:		/* 8.3.40: (0) ERASE IN PAGE */
        scr_erase_screen (arg[0]);
        break;

      case CSI_EL:		/* 8.3.42: (0) ERASE IN LINE */
        scr_erase_line (arg[0]);
        break;

      case CSI_ICH:		/* 8.3.65: (1) INSERT CHARACTER */
        scr_insdel_chars (arg[0], INSERT);
        break;

      case CSI_IL:		/* 8.3.68: (1) INSERT LINE */
        scr_insdel_lines (arg[0], INSERT);
        break;

      case CSI_DL:		/* 8.3.33: (1) DELETE LINE */
        scr_insdel_lines (arg[0], DELETE);
        break;

      case CSI_ECH:		/* 8.3.39: (1) ERASE CHARACTER */
        scr_insdel_chars (arg[0], ERASE);
        break;

      case CSI_DCH:		/* 8.3.26: (1) DELETE CHARACTER */
        scr_insdel_chars (arg[0], DELETE);
        break;

      case CSI_SD:		/* 8.3.114: (1) SCROLL DOWN */
        arg[0] = -arg[0];
        /* FALLTHROUGH */
      case CSI_SU:		/* 8.3.148: (1) SCROLL UP */
        scr_scroll_text (screen.tscroll, screen.bscroll, arg[0], 0);
        break;

      case CSI_DA:		/* 8.3.24: (0) DEVICE ATTRIBUTES */
        tt_write ((const unsigned char *)VT100_ANS,
                 (unsigned int) (sizeof (VT100_ANS) - 1));
        break;

      case CSI_SGR:		/* 8.3.118: (0) SELECT GRAPHIC RENDITION */
        process_sgr_mode (nargs, arg);
        break;

      case CSI_DSR:		/* 8.3.36: (0) DEVICE STATUS REPORT */
        switch (arg[0])
          {
            case 5:			/* DSR requested */
              tt_printf ("\033[0n");
              break;
            case 6:			/* CPR requested */
              scr_report_position ();
              break;
#if defined (ENABLE_DISPLAY_ANSWER)
            case 7:			/* unofficial extension */
              tt_printf ("%-.250s\n", rs[Rs_display_name]);
              break;
#endif
            case 8:			/* unofficial extension */
              xterm_seq (XTerm_title, APL_NAME "-" VERSION, CHAR_ST);
              break;
          }
        break;

      case CSI_TBC:		/* 8.3.155: (0) TABULATION CLEAR */
        switch (arg[0])
          {
            case 0:			/* char tab stop cleared at active position */
              scr_set_tab (0);
              break;
              /* case 1: */		/* line tab stop cleared in active line */
              /* case 2: */		/* char tab stops cleared in active line */
            case 3:			/* all char tab stops are cleared */
              /* case 4: */		/* all line tab stops are cleared */
            case 5:			/* all tab stops are cleared */
              scr_set_tab (-1);
              break;
          }
        break;

      case CSI_CTC:		/* 8.3.17: (0) CURSOR TABULATION CONTROL */
        switch (arg[0])
          {
            case 0:			/* char tab stop set at active position */
              scr_set_tab (1);
              break;		/* = ESC H */
              /* case 1: */		/* line tab stop set at active line */
            case 2:			/* char tab stop cleared at active position */
              scr_set_tab (0);
              break;		/* = ESC [ 0 g */
              /* case 3: */		/* line tab stop cleared at active line */
              /* case 4: */		/* char tab stops cleared at active line */
            case 5:			/* all char tab stops are cleared */
              scr_set_tab (-1);
              break;		/* = ESC [ 3 g */
              /* case 6: */		/* all line tab stops are cleared */
          }
        break;

      case CSI_RM:		/* 8.3.107: RESET MODE */
        if (arg[0] == 4)
          scr_insert_mode (0);
        break;

      case CSI_SM:		/* 8.3.126: SET MODE */
        if (arg[0] == 4)
          scr_insert_mode (1);
        break;

        /*
         * PRIVATE USE beyond this point.  All CSI_7? sequences here
         */
      case CSI_72:		/* DECSTBM: set top and bottom margins */
        if (nargs == 1)
          scr_scroll_region (arg[0] - 1, MAX_ROWS - 1);
        else if (nargs == 0 || arg[0] >= arg[1])
          scr_scroll_region (0, MAX_ROWS - 1);
        else
          scr_scroll_region (arg[0] - 1, arg[1] - 1);
        break;

      case CSI_73:
        scr_cursor (SAVE);
        break;
      case CSI_75:
        scr_cursor (RESTORE);
        break;

#ifndef NO_FRILLS
      case CSI_74:
        process_window_ops (arg, nargs);
        break;
#endif

      case CSI_78:		/* DECREQTPARM */
        if (arg[0] == 0 || arg[0] == 1)
          tt_printf ("\033[%d;1;1;128;128;1;0x", arg[0] + 2);
        /* FALLTHROUGH */

      default:
        break;
    }
}
/*}}} */

#ifndef NO_FRILLS
/* ARGSUSED */
void
rxvt_term::process_window_ops (const int *args, unsigned int nargs)
{
  int             x, y;
#if 0
  char           *s;
#endif
  XWindowAttributes wattr;
  Window          wdummy;

  if (nargs == 0)
    return;
  switch (args[0])
    {
        /*
         * commands
         */
      case 1:			/* deiconify window */
        XMapWindow (display->display, TermWin.parent[0]);
        break;
      case 2:			/* iconify window */
        XIconifyWindow (display->display, TermWin.parent[0],
                       DefaultScreen (display->display));
        break;
      case 3:			/* set position (pixels) */
        XMoveWindow (display->display, TermWin.parent[0], args[1], args[2]);
        break;
      case 4:			/* set size (pixels) */
        set_widthheight ((unsigned int)args[2], (unsigned int)args[1]);
        break;
      case 5:			/* raise window */
        XRaiseWindow (display->display, TermWin.parent[0]);
        break;
      case 6:			/* lower window */
        XLowerWindow (display->display, TermWin.parent[0]);
        break;
      case 7:			/* refresh window */
        scr_touch (true);
        break;
      case 8:			/* set size (chars) */
        set_widthheight ((unsigned int) (args[2] * TermWin.fwidth),
                         (unsigned int) (args[1] * TermWin.fheight));
        break;
      default:
        if (args[0] >= 24)	/* set height (chars) */
          set_widthheight ((unsigned int)TermWin.width,
                           (unsigned int) (args[1] * TermWin.fheight));
        break;
        /*
         * reports - some output format copied from XTerm
         */
      case 11:			/* report window state */
        XGetWindowAttributes (display->display, TermWin.parent[0], &wattr);
        tt_printf ("\033[%dt", wattr.map_state == IsViewable ? 1 : 2);
        break;
      case 13:			/* report window position */
        XGetWindowAttributes (display->display, TermWin.parent[0], &wattr);
        XTranslateCoordinates (display->display, TermWin.parent[0], wattr.root,
                              -wattr.border_width, -wattr.border_width,
                              &x, &y, &wdummy);
        tt_printf ("\033[3;%d;%dt", x, y);
        break;
      case 14:			/* report window size (pixels) */
        XGetWindowAttributes (display->display, TermWin.parent[0], &wattr);
        tt_printf ("\033[4;%d;%dt", wattr.height, wattr.width);
        break;
      case 18:			/* report window size (chars) */
        tt_printf ("\033[8;%d;%dt", TermWin.nrow, TermWin.ncol);
        break;
#if 0 /* XXX: currently disabled due to security concerns */
      case 20:			/* report icon label */
        XGetIconName (display->display, TermWin.parent[0], &s);
        tt_printf ("\033]L%-.200s\234", s ? s : "");	/* 8bit ST */
        break;
      case 21:			/* report window title */
        XFetchName (display->display, TermWin.parent[0], &s);
        tt_printf ("\033]l%-.200s\234", s ? s : "");	/* 8bit ST */
        break;
#endif

    }
}
#endif

/*----------------------------------------------------------------------*/
/*
 * get input up until STRING TERMINATOR (or BEL)
 * ends_how is terminator used.  returned input must be free ()d
 */
unsigned char  *
rxvt_term::get_to_st (unsigned char *ends_how)
{
  int             seen_esc = 0;	/* seen escape? */
  unsigned int    n = 0;
  unsigned char  *s;
  unsigned char   ch, string[STRING_MAX];

  for (; (ch = cmd_getc ());)
    {
      if (ch == C0_BEL
          || ch == CHAR_ST
          || (ch == 0x5c && seen_esc))	/* 7bit ST */
        break;
      if (ch == C0_ESC)
        {
          seen_esc = 1;
          continue;
        }
      else if (ch == '\t')
        ch = ' ';	/* translate '\t' to space */
      else if (ch < 0x08 || (ch > 0x0d && ch < 0x20))
        return NULL;	/* other control character - exit */
      if (n < sizeof (string) - 1)
        string[n++] = ch;
      seen_esc = 0;
    }
  string[n++] = '\0';
  if ((s = (unsigned char *)rxvt_malloc (n)) == NULL)
    return NULL;
  *ends_how = (ch == 0x5c ? C0_ESC : ch);
  STRNCPY (s, string, n);
  return s;
}

/*----------------------------------------------------------------------*/
/*
 * process DEVICE CONTROL STRING `ESC P ... (ST|BEL)' or `0x90 ... (ST|BEL)'
 */
void
rxvt_term::process_dcs_seq ()
{
  unsigned char    eh, *s;
  /*
   * Not handled yet
   */
  s = get_to_st (&eh);
  if (s)
    free (s);
  return;
}

/*----------------------------------------------------------------------*/
/*
 * process OPERATING SYSTEM COMMAND sequence `ESC ] Ps ; Pt (ST|BEL)'
 */
void
rxvt_term::process_osc_seq ()
{
  unsigned char   ch, eh, *s;
  int             arg;

  ch = cmd_getc ();
  for (arg = 0; isdigit (ch); ch = cmd_getc ())
    arg = arg * 10 + (ch - '0');

  if (ch == ';')
    {
      s = get_to_st (&eh);
      if (s)
        {
          /*
           * rxvt_menubar_dispatch () violates the constness of the string,
           * so do it here
           */
          if (arg == XTerm_Menu)
#if 0 /* XXX: currently disabled due to security concerns */
            menubar_dispatch ((char *)s);
#else
            (void)0;
#endif
          else
            xterm_seq (arg, (char *)s, eh);
          free (s);
        }
    }
}
/*
 * XTerm escape sequences: ESC ] Ps;Pt (ST|BEL)
 *       0 = change iconName/title
 *       1 = change iconName
 *       2 = change title
 *       4 = change color
 *      12 = change text color
 *      13 = change mouse foreground color 
 *      17 = change highlight character colour
 *      18 = change bold character color
 *      19 = change underlined character color 
 *      46 = change logfile (not implemented)
 *      50 = change font
 *
 * rxvt extensions:
 *      10 = menu (may change in future)
 *      20 = bg pixmap
 *      39 = change default fg color
 *      49 = change default bg color
 *      55 = dump scrollback buffer and all of screen
 */
void
rxvt_term::xterm_seq (int op, const char *str, unsigned char resp __attribute__ ((unused)))
{
  int             changed = 0;
  int             color;
  char           *buf, *name;

  assert (str != NULL);
  switch (op)
    {
      case XTerm_name:
        set_title (str);
        /* FALLTHROUGH */
      case XTerm_iconName:
        set_iconName (str);
        break;
      case XTerm_title:
        set_title (str);
        break;
      case XTerm_Color:
        for (buf = (char *)str; buf && *buf;)
          {
            if ((name = STRCHR (buf, ';')) == NULL)
              break;
            *name++ = '\0';
            color = atoi (buf);
            if (color < 0 || color >= TOTAL_COLORS)
              break;
            if ((buf = STRCHR (name, ';')) != NULL)
              *buf++ = '\0';
            set_window_color (color + minCOLOR, name);
          }
        break;
#ifndef NO_CURSORCOLOR
      case XTerm_Color_cursor:
        set_window_color (Color_cursor, str);
        break;
#endif
      case XTerm_Color_pointer:
        set_window_color (Color_pointer, str);
        break;
#ifndef NO_BOLD_UNDERLINE_REVERSE
      case XTerm_Color_BD:
        set_window_color (Color_BD, str);
        break;
      case XTerm_Color_UL:
        set_window_color (Color_UL, str);
        break;
      case XTerm_Color_RV:
        set_window_color (Color_RV, str);
        break;
#endif

      case XTerm_Menu:
        /*
         * rxvt_menubar_dispatch () violates the constness of the string,
         * so DON'T do it here
         */
        break;
      case XTerm_Pixmap:
        if (*str != ';')
          {
#if XPM_BACKGROUND
            scale_pixmap ("");	/* reset to default scaling */
            set_bgPixmap (str);	/* change pixmap */
#endif
            scr_touch (true);
          }
        while ((str = STRCHR (str, ';')) != NULL)
          {
            str++;
#if XPM_BACKGROUND
            changed += scale_pixmap (str);
#endif

          }
        if (changed)
          {
#ifdef XPM_BACKGROUND
            resize_pixmap ();
#endif
            scr_touch (true);
          }
        break;

      case XTerm_restoreFG:
        set_window_color (Color_fg, str);
        break;
      case XTerm_restoreBG:
        set_window_color (Color_bg, str);
        break;
      case XTerm_logfile:
        break;
      case XTerm_font:
        change_font (0, str);
        break;
#if 0
      case XTerm_dumpscreen:	/* no error notices */
        {
          int             fd;
          if ((fd = open (str, O_RDWR | O_CREAT | O_EXCL, 0600)) >= 0)
            {
              scr_dump (fd);
              close (fd);
            }
        }
        break;
#endif

    }
}
/*----------------------------------------------------------------------*/

/*{{{ process DEC private mode sequences `ESC [ ? Ps mode' */
/*
 * mode can only have the following values:
 *      'l' = low
 *      'h' = high
 *      's' = save
 *      'r' = restore
 *      't' = toggle
 * so no need for fancy checking
 */
int
rxvt_term::privcases (int mode, unsigned long bit)
{
  int             state;

  if (mode == 's')
    {
      SavedModes |= (PrivateModes & bit);
      return -1;
    }
  else
    {
      if (mode == 'r')
        state = (SavedModes & bit) ? 1 : 0;	/* no overlapping */
      else
        state = (mode == 't') ? ! (PrivateModes & bit) : mode;
      PrivMode (state, bit);
    }
  return state;
}

/* we're not using priv _yet_ */
void
rxvt_term::process_terminal_mode (int mode, int priv __attribute__ ((unused)), unsigned int nargs, const int *arg)
{
  unsigned int    i, j;
  int             state;
  static const struct
    {
      const int       argval;
      const unsigned long bit;
    }
  argtopriv[] = {
                  { 1, PrivMode_aplCUR },
                  { 2, PrivMode_vt52 },
                  { 3, PrivMode_132 },
                  { 4, PrivMode_smoothScroll },
                  { 5, PrivMode_rVideo },
                  { 6, PrivMode_relOrigin },
                  { 7, PrivMode_Autowrap },
                  { 9, PrivMode_MouseX10 },
#ifdef menuBar_esc
                  { menuBar_esc, PrivMode_menuBar },
#endif
#ifdef scrollBar_esc
                  { scrollBar_esc, PrivMode_scrollBar },
#endif
                  { 25, PrivMode_VisibleCursor },
                  { 35, PrivMode_ShiftKeys },
                  { 40, PrivMode_132OK },
                  { 47, PrivMode_Screen },
                  { 66, PrivMode_aplKP },
#ifndef NO_BACKSPACE_KEY
                  { 67, PrivMode_BackSpace },
#endif
                  { 1000, PrivMode_MouseX11 },
                  { 1010, PrivMode_TtyOutputInh },
                  { 1011, PrivMode_Keypress },
                  { 1047, PrivMode_Screen },
                  { 1049, PrivMode_Screen }, /* xterm extension, not fully implemented */
                };

  if (nargs == 0)
    return;

  /* make lo/hi boolean */
  if (mode == 'l')
    mode = 0;		/* reset */
  else if (mode == 'h')
    mode = 1;		/* set */

  for (i = 0; i < nargs; i++)
    {
      state = -1;

      /* basic handling */
      for (j = 0; j < (sizeof (argtopriv)/sizeof (argtopriv[0])); j++)
        if (argtopriv[j].argval == arg[i])
          {
            state = privcases (mode, argtopriv[j].bit);
            break;
          }

      /* extra handling for values with state unkept  */
      if (state == -1)
        switch (arg[i])
          {
            case 1048:		/* alternative cursor save */
              if (mode == 0)
                scr_cursor (RESTORE);
              else if (mode == 1)
                scr_cursor (SAVE);
              /* FALLTHROUGH */
            default:
              continue;	/* for (;i;) */
          }

      /* extra handling for values with valid 0 or 1 state */
      switch (arg[i])
        {
            /* case 1:	- application cursor keys */
          case 2:			/* VT52 mode */
            /* oddball mode.  should be set regardless of set/reset
             * parameter.  Return from VT52 mode with an ESC < from
             * within VT52 mode
             */
            PrivMode (1, PrivMode_vt52);
            break;
          case 3:			/* 80/132 */
            if (PrivateModes & PrivMode_132OK)
              set_widthheight (		    (unsigned int) ((state ? 132 : 80) * TermWin.fwidth),
                                     (unsigned int)TermWin.height);
            break;
          case 4:			/* smooth scrolling */
            if (state)
              Options &= ~Opt_jumpScroll;
            else
              Options |= Opt_jumpScroll;
            break;
          case 5:			/* reverse video */
            scr_rvideo_mode (state);
            break;
          case 6:			/* relative/absolute origins  */
            scr_relative_origin (state);
            break;
          case 7:			/* autowrap */
            scr_autowrap (state);
            break;
            /* case 8:	- auto repeat, can't do on a per window basis */
          case 9:			/* X10 mouse reporting */
            if (state)		/* orthogonal */
              PrivateModes &= ~ (PrivMode_MouseX11);
            break;
#ifdef menuBar_esc
          case menuBar_esc:
#ifdef MENUBAR
            map_menuBar (state);
#endif
            break;
#endif
#ifdef scrollBar_esc
          case scrollBar_esc:
            if (scrollbar_mapping (state))
              {
                resize_all_windows (0, 0, 0);
                scr_touch (true);
              }
            break;
#endif
          case 25:		/* visible/invisible cursor */
            scr_cursor_visible (state);
            break;
            /* case 35:	- shift keys */
            /* case 40:	- 80 <--> 132 mode */
          case 47:		/* secondary screen */
            scr_change_screen (state);
            break;
            /* case 66:	- application key pad */
            /* case 67:	- backspace key */
          case 1000:		/* X11 mouse reporting */
            if (state)		/* orthogonal */
              PrivateModes &= ~ (PrivMode_MouseX10);
            break;
#if 0
          case 1001:
            break;		/* X11 mouse highlighting */
#endif
          case 1010:		/* scroll to bottom on TTY output inhibit */
            if (state)
              Options &= ~Opt_scrollTtyOutput;
            else
              Options |= Opt_scrollTtyOutput;
            break;
          case 1011:		/* scroll to bottom on key press */
            if (state)
              Options |= Opt_scrollTtyKeypress;
            else
              Options &= ~Opt_scrollTtyKeypress;
            break;
          case 1047:		/* secondary screen w/ clearing */
          case 1049:		/* better secondary screen w/ clearing, but not fully implemented */
            if (current_screen != PRIMARY)
              scr_erase_screen (2);
            scr_change_screen (state);
            /* FALLTHROUGH */
          default:
            break;
        }
    }
}
/*}}} */

/*{{{ process sgr sequences */
void
rxvt_term::process_sgr_mode (unsigned int nargs, const int *arg)
{
  unsigned int    i;
  short           rendset;
  int             rendstyle;

  if (nargs == 0)
    {
      scr_rendition (0, ~RS_None);
      return;
    }
  for (i = 0; i < nargs; i++)
    {
      rendset = -1;
      switch (arg[i])
        {
          case 0:
            rendset = 0, rendstyle = ~RS_None;
            break;
          case 1:
            rendset = 1, rendstyle = RS_Bold;
            break;
          case 4:
            rendset = 1, rendstyle = RS_Uline;
            break;
          case 5:
            rendset = 1, rendstyle = RS_Blink;
            break;
          case 7:
            rendset = 1, rendstyle = RS_RVid;
            break;
          case 22:
            rendset = 0, rendstyle = RS_Bold;
            break;
          case 24:
            rendset = 0, rendstyle = RS_Uline;
            break;
          case 25:
            rendset = 0, rendstyle = RS_Blink;
            break;
          case 27:
            rendset = 0, rendstyle = RS_RVid;
            break;
        }
      if (rendset != -1)
        {
          scr_rendition (rendset, rendstyle);
          continue;		/* for (;i;) */
        }

      switch (arg[i])
        {
          case 30:
          case 31:		/* set fg color */
          case 32:
          case 33:
          case 34:
          case 35:
          case 36:
          case 37:
            scr_color ((unsigned int) (minCOLOR + (arg[i] - 30)),
                       Color_fg);
            break;
#ifdef TTY_256COLOR
          case 38:
            if (nargs > i + 2 && arg[i + 1] == 5)
              {
                scr_color ((unsigned int) (minCOLOR + arg[i + 2]),
                           Color_fg);
                i += 2;
              }
            break;
#endif
          case 39:		/* default fg */
            scr_color (Color_fg, Color_fg);
            break;

          case 40:
          case 41:		/* set bg color */
          case 42:
          case 43:
          case 44:
          case 45:
          case 46:
          case 47:
            scr_color ((unsigned int) (minCOLOR + (arg[i] - 40)),
                       Color_bg);
            break;
#ifdef TTY_256COLOR
          case 48:
            if (nargs > i + 2 && arg[i + 1] == 5)
              {
                scr_color ((unsigned int) (minCOLOR + arg[i + 2]),
                           Color_bg);
                i += 2;
              }
            break;
#endif
          case 49:		/* default bg */
            scr_color (Color_bg, Color_bg);
            break;

#ifndef NO_BRIGHTCOLOR
          case 90:
          case 91:		/* set bright fg color */
          case 92:
          case 93:
          case 94:
          case 95:
          case 96:
          case 97:
            scr_color ((unsigned int) (minBrightCOLOR + (arg[i] - 90)),
                       Color_fg);
            break;
          case 100:
          case 101:		/* set bright bg color */
          case 102:
          case 103:
          case 104:
          case 105:
          case 106:
          case 107:
            scr_color ((unsigned int) (minBrightCOLOR + (arg[i] - 100)),
                       Color_bg);
            break;
#endif

        }
    }
}
/*}}} */

/*{{{ process Rob Nation's own graphics mode sequences */
void
rxvt_term::process_graphics ()
{
  unsigned char   ch, cmd = cmd_getc ();

#ifndef RXVT_GRAPHICS
  if (cmd == 'Q')
    {		/* query graphics */
      tt_printf ("\033G0\n");	/* no graphics */
      return;
    }
  /* swallow other graphics sequences until terminating ':' */
  do
    ch = cmd_getc ();
  while (ch != ':');
#else
  unsigned int    nargs;
  int             args[NGRX_PTS];
  unsigned char  *text = NULL;

  if (cmd == 'Q')
    {		/* query graphics */
      tt_printf ("\033G1\n");	/* yes, graphics (color) */
      return;
    }
  for (nargs = 0; nargs < (sizeof (args) / sizeof (args[0])) - 1;)
    {
      int             neg;

      ch = cmd_getc ();
      neg = (ch == '-');
      if (neg || ch == '+')
        ch = cmd_getc ();

      for (args[nargs] = 0; isdigit (ch); ch = cmd_getc ())
        args[nargs] = args[nargs] * 10 + (ch - '0');
      if (neg)
        args[nargs] = -args[nargs];

      nargs++;
      args[nargs] = 0;
      if (ch != ';')
        break;
    }

  if ((cmd == 'T') && (nargs >= 5))
    {
      int             i, len = args[4];

      text = (unsigned char *)rxvt_malloc ((len + 1) * sizeof (char));

      if (text != NULL)
        {
          for (i = 0; i < len; i++)
            text[i] = cmd_getc ();
          text[len] = '\0';
        }
    }
  Gr_do_graphics (cmd, nargs, args, text);
#endif
}
/*}}} */

/* ------------------------------------------------------------------------- */

/*
 * Send printf () formatted output to the command.
 * Only use for small amounts of data.
 */
void
rxvt_term::tt_printf (const char *fmt,...)
{
  va_list arg_ptr;
  unsigned char buf[256];

  va_start (arg_ptr, fmt);
  vsnprintf ((char *)buf, 256, fmt, arg_ptr);
  va_end (arg_ptr);
  tt_write (buf, STRLEN (buf));
}

/* ---------------------------------------------------------------------- */
/* Write data to the pty as typed by the user, pasted with the mouse,
 * or generated by us in response to a query ESC sequence.
 */
void
rxvt_term::tt_write (const unsigned char *data, unsigned int len)
{
  enum { MAX_PTY_WRITE = 255 }; // minimum MAX_INPUT

  if (len)
    {
      if (v_buflen == 0)
        {
          ssize_t written = write (cmd_fd, data, min (MAX_PTY_WRITE, len));

          if ((unsigned int)written == len)
            return;

          data += written;
          len -= written;
        }


      v_buffer = (unsigned char *)realloc (v_buffer, v_buflen + len);

      memcpy (v_buffer + v_buflen, data, len);
      v_buflen += len;
    }

  for (;;)
    {
      int written = write (cmd_fd, v_buffer, min (MAX_PTY_WRITE, v_buflen));

      if (written > 0)
        {
          v_buflen -= written;

          if (v_buflen == 0)
            {
              free (v_buffer);
              v_buffer = 0;
              v_buflen = 0;

              pty_ev.set (EVENT_READ);
              return;
            }

          memmove (v_buffer, v_buffer + written, v_buflen);
        }
      else if (written != -1 || (errno != EAGAIN && errno != EINTR))
        // original code just ignores this...
        destroy ();
      else
        {
          pty_ev.set (EVENT_READ | EVENT_WRITE);
          return;
        }
    }
}

/*----------------------- end-of-file (C source) -----------------------*/

