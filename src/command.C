/*--------------------------------*-C-*---------------------------------*
 * File:	command.c
 *----------------------------------------------------------------------*
 * $Id: command.C,v 1.24 2003/12/19 06:17:03 pcg Exp $
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
/* INTPROTO */
void
rxvt_lookup_key(pR_ XKeyEvent *ev)
{
    int             ctrl, meta, shft, len;
    unsigned int    newlen;
    KeySym          keysym;
#ifdef DEBUG_CMD
    static int      debug_key = 1;	/* accessible by a debugger only */
#endif
    int             valid_keysym;
    unsigned char  *kbuf = R->kbuf;

/*
 * use Num_Lock to toggle Keypad on/off.  If Num_Lock is off, allow an
 * escape sequence to toggle the Keypad.
 *
 * Always permit `shift' to override the current setting
 */
    shft = (ev->state & ShiftMask);
    ctrl = (ev->state & ControlMask);
    meta = (ev->state & R->ModMetaMask);

    if (R->numlock_state || (ev->state & R->ModNumLockMask))
      {
	R->numlock_state = (ev->state & R->ModNumLockMask);
	PrivMode((!R->numlock_state), PrivMode_aplKP);
      }

    kbuf[0] = 0;

#ifdef USE_XIM
    if (R->Input_Context)
      {
	Status status_return;

#ifdef X_HAVE_UTF8_STRING
        if (R->enc_utf8 && 0)
          len = Xutf8LookupString (R->Input_Context, ev, (char *)kbuf,
                                   KBUFSZ, &keysym, &status_return);
        else
#endif
          {
            wchar_t wkbuf[KBUFSZ + 1];

            // the XOpenIM manpage lies about hardcoding the locale
            // at the point of XOpenIM, so temporarily switch locales
            if (R->rs[Rs_imLocale])
              SET_LOCALE (R->rs[Rs_imLocale]);
            // assume wchar_t == unicode or better
            len = XwcLookupString (R->Input_Context, ev, wkbuf,
                                   KBUFSZ, &keysym, &status_return);
            if (R->rs[Rs_imLocale])
              SET_LOCALE (R->locale);

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
	len = XLookupString (ev, (char *)kbuf, KBUFSZ, &keysym, &R->compose);
	valid_keysym = !len;
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
	    if (keysym == R->ks_bigfont)
              {
		rxvt_change_font(aR_ 0, FONT_UP);
		return;
	      }
            else if (keysym == R->ks_smallfont)
              {
		rxvt_change_font(aR_ 0, FONT_DN);
		return;
	      }
	  }
#endif

	if (R->TermWin.saveLines) {
#ifdef UNSHIFTED_SCROLLKEYS
	    if (!ctrl && !meta) {
#else
	    if (IS_SCROLL_MOD) {
#endif
		int             lnsppg;

#ifdef PAGING_CONTEXT_LINES
		lnsppg = R->TermWin.nrow - PAGING_CONTEXT_LINES;
#else
		lnsppg = R->TermWin.nrow * 4 / 5;
#endif
		if (keysym == XK_Prior) {
		    rxvt_scr_page(aR_ UP, lnsppg);
		    return;
		} else if (keysym == XK_Next) {
		    rxvt_scr_page(aR_ DN, lnsppg);
		    return;
		}
	    }
#ifdef SCROLL_ON_UPDOWN_KEYS
	    if (IS_SCROLL_MOD) {
		if (keysym == XK_Up) {
		    rxvt_scr_page(aR_ UP, 1);
		    return;
		} else if (keysym == XK_Down) {
		    rxvt_scr_page(aR_ DN, 1);
		    return;
		}
	    }
#endif
#ifdef SCROLL_ON_HOMEEND_KEYS
	    if (IS_SCROLL_MOD) {
		if (keysym == XK_Home) {
		    rxvt_scr_move_to(aR_ 0, 1);
		    return;
		} else if (keysym == XK_End) {
		    rxvt_scr_move_to(aR_ 1, 0);
		    return;
		}
	    }
#endif
	}

	if (shft) {
	/* Shift + F1 - F10 generates F11 - F20 */
	    if (keysym >= XK_F1 && keysym <= XK_F10) {
		keysym += (XK_F11 - XK_F1);
		shft = 0;	/* turn off Shift */
	    } else if (!ctrl && !meta && (R->PrivateModes & PrivMode_ShiftKeys)) {
		switch (keysym) {
		/* normal XTerm key bindings */
		case XK_Insert:	/* Shift+Insert = paste mouse selection */
		    rxvt_selection_request(aR_ ev->time, 0, 0);
		    return;
		/* rxvt extras */
		case XK_KP_Add:	/* Shift+KP_Add = bigger font */
		    rxvt_change_font(aR_ 0, FONT_UP);
		    return;
		case XK_KP_Subtract:	/* Shift+KP_Subtract = smaller font */
		    rxvt_change_font(aR_ 0, FONT_DN);
		    return;
		}
	    }
	}
#ifdef PRINTPIPE
	if (keysym == XK_Print) {
	    rxvt_scr_printscreen(aR_ ctrl | shft);
	    return;
	}
#endif
#ifdef GREEK_SUPPORT
	if (keysym == R->ks_greekmodeswith) {
	    R->greek_mode = !R->greek_mode;
	    if (R->greek_mode) {
		rxvt_xterm_seq(aR_ XTerm_title,
		               (greek_getmode() == GREEK_ELOT928
				? "[Greek: iso]" : "[Greek: ibm]"), CHAR_ST);
		greek_reset();
	    } else
		rxvt_xterm_seq(aR_ XTerm_title, APL_NAME "-" VERSION, CHAR_ST);
	    return;
	}
#endif

	if (keysym >= 0xFF00 && keysym <= 0xFFFF) {
#ifdef KEYSYM_RESOURCE
	    if (!(shft | ctrl) && R->Keysym_map[keysym & 0xFF] != NULL) {
		unsigned int    l;
		const unsigned char *kbuf0;
		const unsigned char ch = C0_ESC;

		kbuf0 = (R->Keysym_map[keysym & 0xFF]);
		l = (unsigned int)*kbuf0++;

	    /* escape prefix */
		if (meta)
# ifdef META8_OPTION
		    if (R->meta_char == C0_ESC)
# endif
			R->tt_write (&ch, 1);
		R->tt_write (kbuf0, l);
		return;
	    } else
#endif
	    {
		newlen = 1;
		switch (keysym) {
#ifndef NO_BACKSPACE_KEY
		case XK_BackSpace:
		    if (R->PrivateModes & PrivMode_HaveBackSpace) {
			kbuf[0] = (!!(R->PrivateModes & PrivMode_BackSpace)
				   ^ !!ctrl) ? '\b' : '\177';
			kbuf[1] = '\0';
		    } else
			STRCPY(kbuf, R->key_backspace);
# ifdef MULTICHAR_SET
		    if ((R->Options & Opt_mc_hack) && R->screen.cur.col > 0) {
			int             col, row;

			newlen = STRLEN(kbuf);
			col = R->screen.cur.col - 1;
			row = R->screen.cur.row + R->TermWin.saveLines;
			if (IS_MULTI2(R->screen.rend[row][col]))
			    MEMMOVE(kbuf + newlen, kbuf, newlen + 1);
		    }
# endif
		    break;
#endif
#ifndef NO_DELETE_KEY
		case XK_Delete:
		    STRCPY(kbuf, R->key_delete);
# ifdef MULTICHAR_SET
		    if (R->Options & Opt_mc_hack) {
			int             col, row;
 
			newlen = STRLEN(kbuf);
			col = R->screen.cur.col;
			row = R->screen.cur.row + R->TermWin.saveLines;
			if (IS_MULTI1(R->screen.rend[row][col]))
			    MEMMOVE(kbuf + newlen, kbuf, newlen + 1);
		    }
# endif
		    break;
#endif
		case XK_Tab:
		    if (shft)
			STRCPY(kbuf, "\033[Z");
		    else {
#ifdef CTRL_TAB_MAKES_META
			if (ctrl)
			    meta = 1;
#endif
#ifdef MOD4_TAB_MAKES_META
			if (ev->state & Mod4Mask)
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
		    if ((R->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033OZ");
			kbuf[2] = ("txvr"[keysym - XK_KP_Left]);
			break;
		    } else
		    /* translate to std. cursor key */
			keysym = XK_Left + (keysym - XK_KP_Left);
		/* FALLTHROUGH */
#endif
		case XK_Up:	/* "\033[A" */
		case XK_Down:	/* "\033[B" */
		case XK_Right:	/* "\033[C" */
		case XK_Left:	/* "\033[D" */
		    STRCPY(kbuf, "\033[Z");
		    kbuf[2] = ("DACB"[keysym - XK_Left]);
		/* do Shift first */
		    if (shft)
			kbuf[2] = ("dacb"[keysym - XK_Left]);
		    else if (ctrl) {
			kbuf[1] = 'O';
			kbuf[2] = ("dacb"[keysym - XK_Left]);
		    } else if (R->PrivateModes & PrivMode_aplCUR)
			kbuf[1] = 'O';
#ifdef MULTICHAR_SET
                    //TODO: ??
		    if (R->Options & Opt_mc_hack) {
			int             col, row, m;

			col = R->screen.cur.col;
			row = R->screen.cur.row + R->TermWin.saveLines;
			m = 0;
			if (keysym == XK_Right
			    && IS_MULTI1(R->screen.rend[row][col]))
			    m = 1;
			else if (keysym == XK_Left) {
			    if (col > 0) {
				if (IS_MULTI2(R->screen.rend[row][col - 1]))
				    m = 1;
			    } else if (R->screen.cur.row > 0) {
				col = R->screen.tlen[--row];
				if (col == -1)
				    col = R->TermWin.ncol - 1;
				else
				    col--;
				if (col > 0
				    && IS_MULTI2(R->screen.rend[row][col]))
				    m = 1;
			    }
			}
			if (m)
			    MEMMOVE(kbuf + 3, kbuf, 3 + 1);
		    }
#endif
		    break;

#ifndef UNSHIFTED_SCROLLKEYS
# ifdef XK_KP_Prior
		case XK_KP_Prior:
		/* allow shift to override */
		    if ((R->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Oy");
			break;
		    }
		/* FALLTHROUGH */
# endif
		case XK_Prior:
		    STRCPY(kbuf, "\033[5~");
		    break;
# ifdef XK_KP_Next
		case XK_KP_Next:
		/* allow shift to override */
		    if ((R->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Os");
			break;
		    }
		/* FALLTHROUGH */
# endif
		case XK_Next:
		    STRCPY(kbuf, "\033[6~");
		    break;
#endif
		case XK_KP_Enter:
		/* allow shift to override */
		    if ((R->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033OM");
		    } else {
			kbuf[0] = '\r';
			kbuf[1] = '\0';
		    }
		    break;

#ifdef XK_KP_Begin
		case XK_KP_Begin:
		    STRCPY(kbuf, "\033Ou");
		    break;

		case XK_KP_Insert:
		    STRCPY(kbuf, "\033Op");
		    break;

		case XK_KP_Delete:
		    STRCPY(kbuf, "\033On");
		    break;
#endif
		case XK_KP_F1:	/* "\033OP" */
		case XK_KP_F2:	/* "\033OQ" */
		case XK_KP_F3:	/* "\033OR" */
		case XK_KP_F4:	/* "\033OS" */
		    STRCPY(kbuf, "\033OP");
		    kbuf[2] += (keysym - XK_KP_F1);
		    break;

		case XK_KP_Multiply:	/* "\033Oj" : "*" */
		case XK_KP_Add:		/* "\033Ok" : "+" */
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
		    if ((R->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Oj");
			kbuf[2] += (keysym - XK_KP_Multiply);
		    } else {
			kbuf[0] = ('*' + (keysym - XK_KP_Multiply));
			kbuf[1] = '\0';
		    }
		    break;

		case XK_Find:
		    STRCPY(kbuf, "\033[1~");
		    break;
		case XK_Insert:
		    STRCPY(kbuf, "\033[2~");
		    break;
#ifdef DXK_Remove		/* support for DEC remove like key */
		case DXK_Remove:
		/* FALLTHROUGH */
#endif
		case XK_Execute:
		    STRCPY(kbuf, "\033[3~");
		    break;
		case XK_Select:
		    STRCPY(kbuf, "\033[4~");
		    break;
#ifdef XK_KP_End
		case XK_KP_End:
		/* allow shift to override */
		    if ((R->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Oq");
			break;
		    }
		/* FALLTHROUGH */
#endif
		case XK_End:
		    STRCPY(kbuf, KS_END);
		    break;
#ifdef XK_KP_Home
		case XK_KP_Home:
		/* allow shift to override */
		    if ((R->PrivateModes & PrivMode_aplKP) ? !shft : shft) {
			STRCPY(kbuf, "\033Ow");
			break;
		    }
		/* FALLTHROUGH */
#endif
		case XK_Home:
		    STRCPY(kbuf, KS_HOME);
		    break;

#define FKEY(n, fkey)							\
    sprintf((char *)kbuf,"\033[%2d~", (int)((n) + (keysym - fkey)))

		case XK_F1:	/* "\033[11~" */
		case XK_F2:	/* "\033[12~" */
		case XK_F3:	/* "\033[13~" */
		case XK_F4:	/* "\033[14~" */
		case XK_F5:	/* "\033[15~" */
		    FKEY(11, XK_F1);
		    break;
		case XK_F6:	/* "\033[17~" */
		case XK_F7:	/* "\033[18~" */
		case XK_F8:	/* "\033[19~" */
		case XK_F9:	/* "\033[20~" */
		case XK_F10:	/* "\033[21~" */
		    FKEY(17, XK_F6);
		    break;
		case XK_F11:	/* "\033[23~" */
		case XK_F12:	/* "\033[24~" */
		case XK_F13:	/* "\033[25~" */
		case XK_F14:	/* "\033[26~" */
		    FKEY(23, XK_F11);
		    break;
		case XK_F15:	/* "\033[28~" */
		case XK_F16:	/* "\033[29~" */
		    FKEY(28, XK_F15);
		    break;
		case XK_Help:	/* "\033[28~" */
		    FKEY(28, XK_Help);
		    break;
		case XK_Menu:	/* "\033[29~" */
		    FKEY(29, XK_Menu);
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
		    FKEY(31, XK_F17);
		    break;
#undef FKEY
		default:
		    newlen = 0;
		    break;
		}
		if (newlen)
		    len = STRLEN(kbuf);
	    }
	/*
	 * Pass meta for all function keys, if 'meta' option set
	 */
#ifdef META8_OPTION
	    if (meta && (R->meta_char == 0x80) && len > 0)
		kbuf[len - 1] |= 0x80;
#endif
	} else if (ctrl && keysym == XK_minus) {
	    len = 1;
	    kbuf[0] = '\037';	/* Ctrl-Minus generates ^_ (31) */
	} else {
#ifdef META8_OPTION
	/* set 8-bit on */
	    if (meta && (R->meta_char == 0x80)) {
		unsigned char  *ch;

		for (ch = kbuf; ch < kbuf + len; ch++)
		    *ch |= 0x80;
		meta = 0;
	    }
#endif
#ifdef GREEK_SUPPORT
	    if (R->greek_mode)
		len = greek_xlat(kbuf, len);
#endif
	/* nil */ ;
	}
      }

    if (len <= 0)
	return;			/* not mapped */

    if (R->Options & Opt_scrollTtyKeypress)
	if (R->TermWin.view_start) {
	    R->TermWin.view_start = 0;
	    R->want_refresh = 1;
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
	&& (R->meta_char == C0_ESC)
#endif
	) {
	const unsigned char ch = C0_ESC;

	R->tt_write(&ch, 1);
    }
#ifdef DEBUG_CMD
    if (debug_key) {		/* Display keyboard buffer contents */
	char           *p;
	int             i;

	fprintf(stderr, "key 0x%04X [%d]: `", (unsigned int)keysym, len);
	for (i = 0, p = kbuf; i < len; i++, p++)
	    fprintf(stderr, (*p >= ' ' && *p < '\177' ? "%c" : "\\%03o"), *p);
	fprintf(stderr, "'\n");
    }
#endif				/* DEBUG_CMD */
    R->tt_write (kbuf, (unsigned int)len);
}
/*}}} */

#if (MENUBAR_MAX)
/*{{{ rxvt_cmd_write(), rxvt_cmd_getc() */
/* attempt to `write' count to the input buffer */
/* EXTPROTO */
unsigned int
rxvt_cmd_write(pR_ const unsigned char *str, unsigned int count)
{
    unsigned int    n, s;
    unsigned char  *cmdbuf_base = R->cmdbuf_base,
                   *cmdbuf_endp = R->cmdbuf_endp,
                   *cmdbuf_ptr  = R->cmdbuf_ptr;

    n = cmdbuf_ptr - cmdbuf_base;
    s = cmdbuf_base + BUFSIZ - 1 - cmdbuf_endp;
    if (n > 0 && s < count) {
	MEMMOVE(cmdbuf_base, cmdbuf_ptr,
		(unsigned int)(cmdbuf_endp - cmdbuf_ptr));
	cmdbuf_ptr = cmdbuf_base;
	cmdbuf_endp -= n;
	s += n;
    }
    if (count > s) {
	rxvt_print_error("data loss: cmd_write too large");
	count = s;
    }
    for (; count--;)
	*cmdbuf_endp++ = *str++;
    R->cmdbuf_ptr = cmdbuf_ptr;
    R->cmdbuf_endp = cmdbuf_endp;
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
      rxvt_scrollbar_show (this, 1);
#ifdef USE_XIM
      rxvt_IMSendSpot (this);
#endif
    }

  XFlush (Xdisplay);
}

void
rxvt_term::check_cb (check_watcher &w)
{
  SET_R (this);
  SET_LOCALE (locale);

  flush ();
}

void
rxvt_term::process_x_events ()
{
  do
    {
      XEvent          xev;

      XNextEvent (Xdisplay, &xev);

#if defined(CURSOR_BLINK)
      if ((Options & Opt_cursorBlink)
          && xev.type == KeyPress)
        {
          if (hidden_cursor)
            {
              hidden_cursor = 0;
              want_refresh = 1;
            }

          blink_ev.start (NOW + BLINK_INTERVAL);
        }
#endif

#if defined(POINTER_BLANK)
      if ((Options & Opt_pointerBlank)
          && (pointerBlankDelay > 0))
        {
          if (xev.type == MotionNotify
              || xev.type == ButtonPress
              || xev.type == ButtonRelease)
            if (hidden_pointer)
              pointer_unblank ();

          if (xev.type == KeyPress && hidden_pointer == 0)
            pointer_blank ();
        }
#endif

#ifdef USE_XIM
      if (!XFilterEvent (&xev, xev.xany.window))
#endif
        rxvt_process_x_event (this, &xev);
    }
  while (XPending (Xdisplay));
}

void
rxvt_term::blink_cb (time_watcher &w)
{
  w.at += BLINK_INTERVAL;
  hidden_cursor = !hidden_cursor;
  want_refresh = 1;
}

void
rxvt_term::x_cb (io_watcher &w, short revents)
{
  SET_R (this);
  SET_LOCALE (locale);

  process_x_events ();
}

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

                              if (!(Options & Opt_jumpScroll)
                                  || (refresh_count >= (refresh_limit * (TermWin.nrow - 1))))
                                {
                                  refreshnow = true;
                                  flag = false;
                                  ch = NOCHAR;
                                  break;
                                }
                            }

                          if (str >= buf + BUFSIZ)
                            {
                              ch = NOCHAR;
                              break;
                            }
                        }
                    }

                  rxvt_scr_add_lines (this, buf, nlines, str - buf);

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
                        rxvt_process_nonprinting (this, ch);
                        break;
                      case C0_ESC:	/* escape char */
                        rxvt_process_escape_seq (this);
                        break;
                      /*case 0x9b: */	/* CSI */
                      /*  rxvt_process_csi_seq (this); */
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
      int len = mbrtowc (&wc, (char *)cmdbuf_ptr, cmdbuf_endp - cmdbuf_ptr, &mbstate.mbs);

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

/* rxvt_cmd_getc() - Return next input character */
/*
 * Return the next input character after first passing any keyboard input
 * to the command.
 */
/* INTPROTO */
uint32_t
rxvt_cmd_getc(pR)
{
  for (;;)
    {
      uint32_t c = R->next_char ();
      if (c != NOCHAR)
        return c;

      // incomplete sequences should occur rarely, still, a better solution
      // would be preferred. either setjmp/longjmp or better design.
      fcntl (R->cmd_fd, F_SETFL, 0);
      R->pty_fill ();
      fcntl (R->cmd_fd, F_SETFL, O_NONBLOCK);
    }

#if 0
#define TIMEOUT_USEC	5000
    fd_set          readfds;
    int             quick_timeout, select_res;
    int             want_motion_time, want_keypress_time;
    struct timeval  value;
#if defined(POINTER_BLANK) || defined(CURSOR_BLINK)
    struct timeval  tp;
#endif

    for (;;) {
    /* loop until we can return something */

	if (R->v_bufstr < R->v_bufptr)	/* output any pending chars */
	    R->tt_write(NULL, 0);

#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
	if (R->mouse_slip_wheel_speed) {
	    quick_timeout = 1;
	    if (!R->mouse_slip_wheel_delay--
		&& rxvt_scr_page(aR_ R->mouse_slip_wheel_speed > 0 ? UP : DN,
				 abs(R->mouse_slip_wheel_speed))) {
		R->mouse_slip_wheel_delay = SCROLLBAR_CONTINUOUS_DELAY;
		R->refresh_type |= SMOOTH_REFRESH;
		R->want_refresh = 1;
	    }
	}
#endif /* MOUSE_WHEEL && MOUSE_SLIP_WHEELING */
#ifdef SELECTION_SCROLLING
	if (R->pending_scroll_selection) {
	    quick_timeout = 1;
	    if (!R->scroll_selection_delay--
		&& rxvt_scr_page(aR_ R->scroll_selection_dir,
		    R->scroll_selection_lines)) {
		R->selection_extend (R->selection_save_x,
		    R->selection_save_y, R->selection_save_state);
		R->scroll_selection_delay = SCROLLBAR_CONTINUOUS_DELAY;
		R->refresh_type |= SMOOTH_REFRESH;
		R->want_refresh = 1;
	    }
	}
#endif
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	if (scrollbar_isUp() || scrollbar_isDn()) {
	    quick_timeout = 1;
	    if (!R->scroll_arrow_delay--
		&& rxvt_scr_page(aR_ scrollbar_isUp() ? UP : DN, 1)) {
		R->scroll_arrow_delay = SCROLLBAR_CONTINUOUS_DELAY;
		R->refresh_type |= SMOOTH_REFRESH;
		R->want_refresh = 1;
	    }
	}
#endif				/* NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING */

#ifdef TRANSPARENT
	    quick_timeout |= R->want_full_refresh;
#endif
#endif
}

void
rxvt_term::pointer_unblank ()
{
  XDefineCursor (Xdisplay, TermWin.vt, TermWin_cursor);
  recolour_cursor ();

#ifdef POINTER_BLANK
  hidden_pointer = 0;

  if (Options & Opt_pointerBlank)
    pointer_ev.start (NOW + pointerBlankDelay);
#endif
}

#ifdef POINTER_BLANK
void
rxvt_term::pointer_blank ()
{
  pointer_ev.stop ();

  if (!(Options & Opt_pointerBlank))
    return;

  XDefineCursor (Xdisplay, TermWin.vt, blank_cursor);
  XFlush (Xdisplay);

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

/* INTPROTO */
void
rxvt_mouse_report(pR_ const XButtonEvent *ev)
{
    int             button_number, key_state = 0;
    int             x, y;

    x = ev->x;
    y = ev->y;
    R->pixel_position (&x, &y);

    if (R->MEvent.button == AnyButton) {
	button_number = 3;
    } else {
	button_number = R->MEvent.button - Button1;
	/* add 0x3D for wheel events, like xterm does */
	if (button_number >= 3)
	    button_number += (64 - 3);
    }

    if (R->PrivateModes & PrivMode_MouseX10) {
    /*
     * do not report ButtonRelease
     * no state info allowed
     */
	key_state = 0;
	if (button_number == 3)
	    return;
    } else {
    /* XTerm mouse reporting needs these values:
     *   4 = Shift
     *   8 = Meta
     *  16 = Control
     * plus will add in our own Double-Click reporting
     *  32 = Double Click
     */
	key_state = ((R->MEvent.state & ShiftMask) ? 4 : 0)
		     + ((R->MEvent.state & R->ModMetaMask) ? 8 : 0)
		     + ((R->MEvent.state & ControlMask) ? 16 : 0);
#ifdef MOUSE_REPORT_DOUBLECLICK
	key_state += ((R->MEvent.clicks > 1) ? 32 : 0);
#endif
    }

#ifdef DEBUG_MOUSEREPORT
    fprintf(stderr, "Mouse [");
    if (key_state & 16)
	fputc('C', stderr);
    if (key_state & 4)
	fputc('S', stderr);
    if (key_state & 8)
	fputc('A', stderr);
    if (key_state & 32)
	fputc('2', stderr);
    fprintf(stderr, "]: <%d>, %d/%d\n",
	    button_number,
	    x + 1,
	    y + 1);
#else
    R->tt_printf("\033[M%c%c%c",
	      (32 + button_number + key_state),
	      (32 + x + 1),
	      (32 + y + 1));
#endif
}

#ifdef USING_W11LIB
/* EXTPROTO */
void
rxvt_W11_process_x_event(XEvent *ev)
{
    rxvt_t         *r = rxvt_get_r();

    rxvt_process_x_event(aR_ ev);
}
#endif

/*{{{ process an X event */
/* INTPROTO */
void
rxvt_process_x_event(pR_ XEvent *ev)
{
    int             i, want_timeout;
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
    (void)gettimeofday(&tp, NULL);
    ltt = localtime(&(tp.tv_sec));
    D_X((stderr, "Event: %-16s %-7s %08lx (%4d-%02d-%02d %02d:%02d:%02d.%.6ld) %s %lu", eventnames[ev->type], (ev->xany.window == R->TermWin.parent[0] ? "parent" : (ev->xany.window == R->TermWin.vt ? "vt" : (ev->xany.window == R->scrollBar.win ? "scroll" : (ev->xany.window == R->menuBar.win ? "menubar" : "UNKNOWN")))), (ev->xany.window == R->TermWin.parent[0] ? R->TermWin.parent[0] : (ev->xany.window == R->TermWin.vt ? R->TermWin.vt : (ev->xany.window == R->scrollBar.win ? R->scrollBar.win : (ev->xany.window == R->menuBar.win ? R->menuBar.win : 0)))), ltt->tm_year + 1900, ltt->tm_mon + 1, ltt->tm_mday, ltt->tm_hour, ltt->tm_min, ltt->tm_sec, tp.tv_usec, ev->xany.send_event ? "S" : " ", ev->xany.serial));
#endif

    switch (ev->type) {
    case KeyPress:
	rxvt_lookup_key(aR_ (XKeyEvent *)ev);
	break;

#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
    case KeyRelease:
	{
	    if (!(ev->xkey.state & ControlMask))
		R->mouse_slip_wheel_speed = 0;
	    else {
		KeySym          ks;
		
		ks = XKeycodeToKeysym(R->Xdisplay, ev->xkey.keycode, 0);
		if (ks == XK_Control_L || ks == XK_Control_R)
		    R->mouse_slip_wheel_speed = 0;
	    }
	    break;
	}
#endif

    case ButtonPress:
	rxvt_button_press(aR_ (XButtonEvent *)ev);
	break;

    case ButtonRelease:
	rxvt_button_release(aR_ (XButtonEvent *)ev);
	break;

    case ClientMessage:
	if (ev->xclient.format == 32
	    && (Atom)ev->xclient.data.l[0] == R->xa[XA_WMDELETEWINDOW])
          R->destroy ();
#ifdef OFFIX_DND
    /* OffiX Dnd (drag 'n' drop) protocol */
        else if (ev->xclient.message_type == R->xa[XA_DNDPROTOCOL]
	    && (ev->xclient.data.l[0] == DndFile
		|| ev->xclient.data.l[0] == DndDir
		|| ev->xclient.data.l[0] == DndLink)) {
	/* Get Dnd data */
	    Atom            ActualType;
	    int             ActualFormat;
	    unsigned char  *data;
	    unsigned long   Size, RemainingBytes;

	    XGetWindowProperty(R->Xdisplay, Xroot,
			       R->xa[XA_DNDSELECTION],
			       0L, 1000000L,
			       False, AnyPropertyType,
			       &ActualType, &ActualFormat,
			       &Size, &RemainingBytes,
			       &data);
	    XChangeProperty(R->Xdisplay, Xroot,
			    XA_CUT_BUFFER0, XA_STRING,
			    8, PropModeReplace,
			    data, STRLEN(data));
	    rxvt_selection_paste(aR_ Xroot, XA_CUT_BUFFER0, True);
	    XSetInputFocus(R->Xdisplay, Xroot, RevertToNone, CurrentTime);
	}
#endif				/* OFFIX_DND */
	break;

    case MappingNotify:
	XRefreshKeyboardMapping(&(ev->xmapping));
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
	switch (ev->xvisibility.state)
          {
            case VisibilityUnobscured:
              R->refresh_type = FAST_REFRESH;
              break;
            case VisibilityPartiallyObscured:
              R->refresh_type = SLOW_REFRESH;
              break;
            default:
              R->refresh_type = NO_REFRESH;
              break;
          }
	break;

    case FocusIn:
	if (!R->TermWin.focus) {
	    R->TermWin.focus = 1;
	    R->want_refresh = 1;
#ifdef USE_XIM
	    if (R->Input_Context != NULL)
		XSetICFocus(R->Input_Context);
#endif
#ifdef CURSOR_BLINK
            if (R->Options & Opt_cursorBlink)
              R->blink_ev.start (NOW + BLINK_INTERVAL);
#endif
	}
	break;

    case FocusOut:
	if (R->TermWin.focus) {
	    R->TermWin.focus = 0;
	    R->want_refresh = 1;
#ifdef USE_XIM
	    if (R->Input_Context != NULL)
		XUnsetICFocus(R->Input_Context);
#endif
#ifdef CURSOR_BLINK
            if (R->Options & Opt_cursorBlink)
              R->blink_ev.stop ();
            R->hidden_cursor = 0;
#endif
	}
	break;

    case ConfigureNotify:
	if (ev->xconfigure.window == R->TermWin.parent[0]) {
	    int             height, width;

	    do {	/* Wrap lots of configures into one */
		width = ev->xconfigure.width;
		height = ev->xconfigure.height;
		D_SIZE((stderr, "Size: ConfigureNotify: %4d x %4d", width, height));
	    } while (XCheckTypedWindowEvent(R->Xdisplay, ev->xconfigure.window,
					    ConfigureNotify, ev));
	    if (R->szHint.width != width || R->szHint.height != height) {
		D_SIZE((stderr, "Size: Resizing from: %4d x %4d", R->szHint.width, R->szHint.height));
		R->resize_all_windows (width, height, 1);
	    }
#ifdef DEBUG_SIZE
	    else {
		D_SIZE((stderr, "Size: Not resizing"));
	    }
#endif
#ifdef TRANSPARENT		/* XXX: maybe not needed - leave in for now */
	    if (R->Options & Opt_transparent) {
		rxvt_check_our_parents(aR);
		if (R->am_transparent)
		    R->want_full_refresh = 1;
	    }
#endif
	}
	break;

    case SelectionClear:
	rxvt_selection_clear(aR);
	break;

    case SelectionNotify:
	if (R->selection_wait == Sel_normal)
	    rxvt_selection_paste(aR_ ev->xselection.requestor,
				 ev->xselection.property, True);
	break;

    case SelectionRequest:
	rxvt_selection_send(aR_ &(ev->xselectionrequest));
	break;

    case UnmapNotify:
	R->TermWin.mapped = 0;
	break;

    case MapNotify:
	R->TermWin.mapped = 1;
	break;

    case PropertyNotify:
	if (ev->xproperty.atom == R->xa[XA_VT_SELECTION]) {
	    if (ev->xproperty.state == PropertyNewValue)
		rxvt_selection_property(aR_ ev->xproperty.window,
					ev->xproperty.atom);
	    break;
	}
#ifdef TRANSPARENT
    /*
     * if user used some Esetroot compatible prog to set the root bg,
     * use the property to determine the pixmap.  We use it later on.
     */
	if (R->xa[XA_XROOTPMAPID] == 0)
	    R->xa[XA_XROOTPMAPID] = XInternAtom(R->Xdisplay,
					       "_XROOTPMAP_ID", False);
	if (ev->xproperty.atom != R->xa[XA_XROOTPMAPID])
	    break;
    /* FALLTHROUGH */
    case ReparentNotify:
	if ((R->Options & Opt_transparent) && rxvt_check_our_parents(aR)) {
	    if (R->am_transparent)
		R->want_full_refresh = 1;
	}
#endif				/* TRANSPARENT */
	break;

    case GraphicsExpose:
    case Expose:
	if (ev->xany.window == R->TermWin.vt) {
#ifdef NO_SLOW_LINK_SUPPORT
	    R->scr_expose (ev->xexpose.x, ev->xexpose.y,
			   ev->xexpose.width, ev->xexpose.height, False);
#else
            // don't understand this, so commented it out
	    R->scr_expose (ev->xexpose.x, ev->xexpose.y,
			   ev->xexpose.width, ev->xexpose.height, False);
	    //rxvt_scr_expose(aR_ ev->xexpose.x, 0,
	    //		    ev->xexpose.width, R->TermWin.height, False);
#endif
	    R->want_refresh = 1;
	} else {
	    XEvent          unused_xevent;

	    while (XCheckTypedWindowEvent(R->Xdisplay, ev->xany.window,
					  Expose,
					  &unused_xevent)) ;
	    while (XCheckTypedWindowEvent(R->Xdisplay, ev->xany.window,
					  GraphicsExpose,
					  &unused_xevent)) ;
	    if (isScrollbarWindow(ev->xany.window)) {
		R->scrollBar.setIdle();
		rxvt_scrollbar_show(aR_ 0);
	    }
#ifdef MENUBAR
	    if (menubar_visible(aR) && isMenuBarWindow(ev->xany.window))
		rxvt_menubar_expose(aR);
#endif
#ifdef RXVT_GRAPHICS
	    rxvt_Gr_expose(aR_ ev->xany.window);
#endif
	}
	break;

    case MotionNotify:
#ifdef POINTER_BLANK
	if (R->hidden_pointer)
	    R->pointer_unblank ();
#endif
#if MENUBAR
	if (isMenuBarWindow(ev->xany.window)) {
	    rxvt_menubar_control(aR_ &(ev->xbutton));
	    break;
	}
#endif
	if ((R->PrivateModes & PrivMode_mouse_report) && !(R->bypass_keystate))
	    break;

	if (ev->xany.window == R->TermWin.vt) {
	    if ((ev->xbutton.state & (Button1Mask | Button3Mask))) {
		while (XCheckTypedWindowEvent(R->Xdisplay, R->TermWin.vt,
					      MotionNotify, ev)) ;
		XQueryPointer(R->Xdisplay, R->TermWin.vt,
			      &unused_root, &unused_child,
			      &unused_root_x, &unused_root_y,
			      &(ev->xbutton.x), &(ev->xbutton.y),
			      &unused_mask);
#ifdef MOUSE_THRESHOLD
	    /* deal with a `jumpy' mouse */
		if ((ev->xmotion.time - R->MEvent.time) > MOUSE_THRESHOLD) {
#endif
		    R->selection_extend ((ev->xbutton.x), (ev->xbutton.y),
				  (ev->xbutton.state & Button3Mask) ? 2 : 0);
#ifdef SELECTION_SCROLLING
		    if (ev->xbutton.y<R->TermWin.int_bwidth ||
			Pixel2Row(ev->xbutton.y)>(R->TermWin.nrow-1)) {
			int dist;
			
			R->pending_scroll_selection=1;
			
			  /* don't clobber the current delay if we are
			   * already in the middle of scrolling.
			   */
			if (R->scroll_selection_delay<=0)
			    R->scroll_selection_delay=SCROLLBAR_CONTINUOUS_DELAY;

			  /* save the event params so we can highlight
			   * the selection in the pending-scroll loop
			   */
			R->selection_save_x=ev->xbutton.x;
			R->selection_save_y=ev->xbutton.y;
			R->selection_save_state=
			    (ev->xbutton.state & Button3Mask) ? 2 : 0;

			  /* calc number of lines to scroll */
			if (ev->xbutton.y<R->TermWin.int_bwidth) {
			    R->scroll_selection_dir = UP;
			    dist = R->TermWin.int_bwidth - ev->xbutton.y;
			}
			else {
			    R->scroll_selection_dir = DN;
			    dist = ev->xbutton.y -
				(R->TermWin.int_bwidth + R->TermWin.height);
			}
			R->scroll_selection_lines=(Pixel2Height(dist)/
			    SELECTION_SCROLL_LINE_SPEEDUP)+1;
			MIN_IT(R->scroll_selection_lines,
			    SELECTION_SCROLL_MAX_LINES);
		    }
		    else {
			  /* we are within the text window, so we
			   * shouldn't be scrolling
			   */
			R->pending_scroll_selection = 0;
		    }
#endif
#ifdef MOUSE_THRESHOLD
		}
#endif
	    }
	} else if (isScrollbarWindow(ev->xany.window) && scrollbar_isMotion()) {
	    while (XCheckTypedWindowEvent(R->Xdisplay, R->scrollBar.win,
					  MotionNotify, ev)) ;
	    XQueryPointer(R->Xdisplay, R->scrollBar.win,
			  &unused_root, &unused_child,
			  &unused_root_x, &unused_root_y,
			  &(ev->xbutton.x), &(ev->xbutton.y),
			  &unused_mask);
	    rxvt_scr_move_to(aR_ scrollbar_position(ev->xbutton.y) - R->csrO,
			     scrollbar_size());
	    R->scr_refresh (R->refresh_type);
	    R->refresh_limit = 0;
	    rxvt_scrollbar_show(aR_ 1);
	}
	break;
    }
}

/* INTPROTO */
void
rxvt_button_press(pR_ XButtonEvent *ev)
{
    int             reportmode = 0, clickintime;

    R->bypass_keystate = ev->state & (R->ModMetaMask | ShiftMask);
    if (!R->bypass_keystate)
	reportmode = !!(R->PrivateModes & PrivMode_mouse_report);
/*
 * VT window processing of button press
 */
    if (ev->window == R->TermWin.vt)
      {
#if RXVT_GRAPHICS
	if (ev->subwindow != None)
	  rxvt_Gr_ButtonPress (ev->x, ev->y);
	else
#endif
          {
	    clickintime = ev->time - R->MEvent.time < MULTICLICK_TIME;
	    if (reportmode)
              {
		/* mouse report from vt window */
		/* save the xbutton state (for ButtonRelease) */
		R->MEvent.state = ev->state;
#ifdef MOUSE_REPORT_DOUBLECLICK
		if (ev->button == R->MEvent.button && clickintime)
                  {
		    /* same button, within alloted time */
		    R->MEvent.clicks++;
		    if (R->MEvent.clicks > 1)
                      {
			/* only report double clicks */
			R->MEvent.clicks = 2;
			rxvt_mouse_report(aR_ ev);

			/* don't report the release */
			R->MEvent.clicks = 0;
			R->MEvent.button = AnyButton;
		      }
		  }
                else
                  {
		    /* different button, or time expired */
		    R->MEvent.clicks = 1;
		    R->MEvent.button = ev->button;
		    rxvt_mouse_report(aR_ ev);
		  }
#else
		R->MEvent.button = ev->button;
		rxvt_mouse_report(aR_ ev);
#endif				/* MOUSE_REPORT_DOUBLECLICK */
	      }
            else
              {
		if (ev->button != R->MEvent.button)
		    R->MEvent.clicks = 0;
		switch (ev->button)
                  {
                    case Button1:
                        /* allow shift+left click to extend selection */
                        if (ev->state & ShiftMask)
                          {
                            if (R->MEvent.button == Button1 && clickintime)
                              R->selection_rotate (ev->x, ev->y);
                            else
                              R->selection_extend (ev->x, ev->y, 1);
                          }
                        else
                          {
                            if (R->MEvent.button == Button1 && clickintime)
                              R->MEvent.clicks++;
                            else
                              R->MEvent.clicks = 1;

                            R->selection_click (R->MEvent.clicks, ev->x, ev->y);
                          }

                        R->MEvent.button = Button1;
                        break;

                    case Button3:
                        if (R->MEvent.button == Button3 && clickintime)
                          R->selection_rotate (ev->x, ev->y);
                        else
                          R->selection_extend (ev->x, ev->y, 1);
                        R->MEvent.button = Button3;
                        break;
	          }
	      }
	    R->MEvent.time = ev->time;
	    return;
	  }
      }

/*
 * Scrollbar window processing of button press
 */
    if (isScrollbarWindow(ev->window))
      {
	R->scrollBar.setIdle ();
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
	if (reportmode) {
	    /*
	     * Mouse report disabled scrollbar:
	     * arrow buttons - send up/down
	     * click on scrollbar - send pageup/down
	     */
	    if ((R->scrollBar.style == R_SB_NEXT
		 && scrollbarnext_upButton(ev->y))
		|| (R->scrollBar.style == R_SB_RXVT
		    && scrollbarrxvt_upButton(ev->y)))
		R->tt_printf("\033[A");
	    else if ((R->scrollBar.style == R_SB_NEXT
		      && scrollbarnext_dnButton(ev->y))
		     || (R->scrollBar.style == R_SB_RXVT
			 && scrollbarrxvt_dnButton(ev->y)))
		R->tt_printf("\033[B");
	    else
		switch (ev->button) {
		case Button2:
		    R->tt_printf("\014");
		    break;
		case Button1:
		    R->tt_printf("\033[6~");
		    break;
		case Button3:
		    R->tt_printf("\033[5~");
		    break;
		}
	  }
        else
#endif				/* NO_SCROLLBAR_REPORT */
	  {
	    char            upordown = 0;

	    if (R->scrollBar.style == R_SB_NEXT) {
		if (scrollbarnext_upButton(ev->y))
		    upordown = -1;	/* up */
		else if (scrollbarnext_dnButton(ev->y))
		    upordown = 1;	/* down */
	    } else if (R->scrollBar.style == R_SB_RXVT) {
		if (scrollbarrxvt_upButton(ev->y))
		    upordown = -1;	/* up */
		else if (scrollbarrxvt_dnButton(ev->y))
		    upordown = 1;	/* down */
	    }
	    if (upordown) { 
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
		R->scroll_arrow_delay = SCROLLBAR_INITIAL_DELAY;
#endif
		if (rxvt_scr_page(aR_ upordown < 0 ? UP : DN, 1)) {
		    if (upordown < 0)
			R->scrollBar.setUp ();
		    else
			R->scrollBar.setDn ();
		}
	    } else
		switch (ev->button) {
		case Button2:
		    switch (R->scrollbar_align) {
		    case R_SB_ALIGN_TOP:
			R->csrO = 0;
			break;
		    case R_SB_ALIGN_CENTRE:
			R->csrO = (R->scrollBar.bot - R->scrollBar.top) / 2;
			break;
		    case R_SB_ALIGN_BOTTOM:
			R->csrO = R->scrollBar.bot - R->scrollBar.top;
			break;
		    }
		    if (R->scrollBar.style == R_SB_XTERM
			|| scrollbar_above_slider(ev->y)
			|| scrollbar_below_slider(ev->y))
			rxvt_scr_move_to(aR_
					 scrollbar_position(ev->y) - R->csrO,
					 scrollbar_size());
		    R->scrollBar.setMotion ();
		    break;

		case Button1:
		    if (R->scrollbar_align == R_SB_ALIGN_CENTRE)
			R->csrO = ev->y - R->scrollBar.top;
		    /* FALLTHROUGH */

		case Button3:
		    if (R->scrollBar.style != R_SB_XTERM) {
			if (scrollbar_above_slider(ev->y))
# ifdef RXVT_SCROLL_FULL
			    rxvt_scr_page(aR_ UP, R->TermWin.nrow - 1);
# else
			    rxvt_scr_page(aR_ UP, R->TermWin.nrow / 4);
# endif
			else if (scrollbar_below_slider(ev->y))
# ifdef RXVT_SCROLL_FULL
			    rxvt_scr_page(aR_ DN, R->TermWin.nrow - 1);
# else
			    rxvt_scr_page(aR_ DN, R->TermWin.nrow / 4);
# endif
			else
			    R->scrollBar.setMotion ();
		    } else {
			rxvt_scr_page(aR_ (ev->button == Button1 ? DN : UP),
				      (R->TermWin.nrow
				       * scrollbar_position(ev->y)
				       / scrollbar_size()));
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
    if (isMenuBarWindow(ev->window))
	rxvt_menubar_control(aR_ ev);
#endif
}

/* INTPROTO */
void
rxvt_button_release(pR_ XButtonEvent *ev)
{
    int             reportmode = 0;

    R->csrO = 0;		/* reset csr Offset */
    if (!R->bypass_keystate)
	reportmode = !!(R->PrivateModes & PrivMode_mouse_report);

    if (scrollbar_isUpDn()) {
	R->scrollBar.setIdle ();
	rxvt_scrollbar_show(aR_ 0);
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
	R->refresh_type &= ~SMOOTH_REFRESH;
#endif
    }
#ifdef SELECTION_SCROLLING
    R->pending_scroll_selection=0;
#endif
    if (ev->window == R->TermWin.vt) 
      {
#ifdef RXVT_GRAPHICS
	if (ev->subwindow != None)
	  rxvt_Gr_ButtonRelease(ev->x, ev->y);
	else 
#endif
          {
	    if (reportmode) 
              {
		/* mouse report from vt window */
		/* don't report release of wheel "buttons" */
		if (ev->button >= 4)
		    return;
#ifdef MOUSE_REPORT_DOUBLECLICK
		/* only report the release of 'slow' single clicks */
		if (R->MEvent.button != AnyButton
		    && (ev->button != R->MEvent.button
			|| (ev->time - R->MEvent.time
			    > MULTICLICK_TIME / 2))) 
                  {
		    R->MEvent.clicks = 0;
		    R->MEvent.button = AnyButton;
		    rxvt_mouse_report(aR_ ev);
		  }
#else				/* MOUSE_REPORT_DOUBLECLICK */
		R->MEvent.button = AnyButton;
		rxvt_mouse_report(aR_ ev);
#endif				/* MOUSE_REPORT_DOUBLECLICK */
		return;
	    }
	    /*
	     * dumb hack to compensate for the failure of click-and-drag
	     * when overriding mouse reporting
	     */
	    if (R->PrivateModes & PrivMode_mouse_report
		&& R->bypass_keystate
		&& ev->button == Button1 && R->MEvent.clicks <= 1)
		R->selection_extend (ev->x, ev->y, 0);

	    switch (ev->button) {
	    case Button1:
	    case Button3:
		rxvt_selection_make(aR_ ev->time);
		break;
	    case Button2:
		rxvt_selection_request(aR_ ev->time, ev->x, ev->y);
		break;
#ifdef MOUSE_WHEEL
	    case Button4:
	    case Button5:
		  {
		    int i;
                    page_dirn v;

		    v = (ev->button == Button4) ? UP : DN;
		    if (ev->state & ShiftMask)
			i = 1;
		    else if ((R->Options & Opt_mouseWheelScrollPage))
			i = R->TermWin.nrow - 1;
		    else
			i = 5;
# ifdef MOUSE_SLIP_WHEELING
                    if (ev->state & ControlMask) 
                      {
			R->mouse_slip_wheel_speed += (v ? -1 : 1);
			R->mouse_slip_wheel_delay = SCROLLBAR_CONTINUOUS_DELAY;
		      }
# endif
# ifdef JUMP_MOUSE_WHEEL
		    rxvt_scr_page(aR_ v, i);
		    R->scr_refresh (SMOOTH_REFRESH);
		    rxvt_scrollbar_show(aR_ 1);
# else
		    while (i--)
                      {
			rxvt_scr_page(aR_ v, 1);
			R->scr_refresh (SMOOTH_REFRESH);
			rxvt_scrollbar_show(aR_ 1);
		      }
# endif
		  }
		break;
#endif
	      }
	  }
      }
#ifdef MENUBAR
    else if (isMenuBarWindow(ev->window))
	rxvt_menubar_control(aR_ ev);
#endif
}


#ifdef TRANSPARENT
/*
 * Check our parents are still who we think they are.
 * Do transparency updates if required
 */
/* EXTPROTO */
int
rxvt_check_our_parents(pR)
{
    int             i, pchanged, aformat, have_pixmap, rootdepth;
    unsigned long   nitems, bytes_after;
    Atom            atype;
    unsigned char   *prop = NULL;
    Window          root, oldp, *list;
    Pixmap          rootpixmap = None;
    XWindowAttributes wattr, wrootattr;

    pchanged = 0;

    if (!(R->Options & Opt_transparent))
	return pchanged;	/* Don't try any more */

    XGetWindowAttributes(R->Xdisplay, Xroot, &wrootattr);
    rootdepth = wrootattr.depth;

    XGetWindowAttributes(R->Xdisplay, R->TermWin.parent[0], &wattr);
    if (rootdepth != wattr.depth) {
	if (R->am_transparent) {
	    pchanged = 1;
	    XSetWindowBackground(R->Xdisplay, R->TermWin.vt,
				 R->PixColors[Color_bg]);
	    R->am_transparent = R->am_pixmap_trans = 0;
	}
	return pchanged;	/* Don't try any more */
    }

/* Get all X ops out of the queue so that our information is up-to-date. */
    XSync(R->Xdisplay, False);

/*
 * Make the frame window set by the window manager have
 * the root background. Some window managers put multiple nested frame
 * windows for each client, so we have to take care about that.
 */
    i = (R->xa[XA_XROOTPMAPID] != 0
	 && (XGetWindowProperty(R->Xdisplay, Xroot, R->xa[XA_XROOTPMAPID],
				0L, 1L, False, XA_PIXMAP, &atype, &aformat,
				&nitems, &bytes_after, &prop) == Success));
    if (!i || prop == NULL)
	have_pixmap = 0;
    else {
	have_pixmap = 1;
	rootpixmap = *((Pixmap *)prop);
	XFree(prop);
    }
    if (have_pixmap) {
/*
 * Copy Xroot pixmap transparency
 */
	int             sx, sy, nx, ny;
	unsigned int    nw, nh;
	Window          cr;
	XImage         *image;
	GC              gc;
	XGCValues       gcvalue;

	XTranslateCoordinates(R->Xdisplay, R->TermWin.parent[0], Xroot,
			      0, 0, &sx, &sy, &cr);
	nw = (unsigned int)R->szHint.width;
	nh = (unsigned int)R->szHint.height;
	nx = ny = 0;
	if (sx < 0) {
	    nw += sx;
	    nx = -sx;
	    sx = 0;
	}
	if (sy < 0) {
	    nh += sy;
	    ny = -sy;
	    sy = 0;
	}
	MIN_IT(nw, (unsigned int)(wrootattr.width - sx));
	MIN_IT(nh, (unsigned int)(wrootattr.height - sy));
	R->allowedxerror = -1;
	image = XGetImage(R->Xdisplay, rootpixmap, sx, sy, nw, nh, AllPlanes,
			  ZPixmap);
	/* XXX: handle BadMatch - usually because we're outside the pixmap */
	/* XXX: may need a delay here? */
	R->allowedxerror = 0;
	if (image == NULL) {
	    if (R->am_transparent && R->am_pixmap_trans) {
		pchanged = 1;
		if (R->TermWin.pixmap != None) {
		    XFreePixmap(R->Xdisplay, R->TermWin.pixmap);
		    R->TermWin.pixmap = None;
		}
	    }
	    R->am_pixmap_trans = 0;
	} else {
	    if (R->TermWin.pixmap != None)
		XFreePixmap(R->Xdisplay, R->TermWin.pixmap);
	    R->TermWin.pixmap = XCreatePixmap(R->Xdisplay, R->TermWin.vt,
					      (unsigned int)R->szHint.width,
					      (unsigned int)R->szHint.height,
					      (unsigned int)image->depth);
	    gc = XCreateGC(R->Xdisplay, R->TermWin.vt, 0UL, &gcvalue);
	    XPutImage(R->Xdisplay, R->TermWin.pixmap, gc, image, 0, 0,
		      nx, ny, (unsigned int)image->width,
		      (unsigned int)image->height);
	    XFreeGC(R->Xdisplay, gc);
	    XDestroyImage(image);
	    XSetWindowBackgroundPixmap(R->Xdisplay, R->TermWin.vt,
				       R->TermWin.pixmap);
	    if (!R->am_transparent || !R->am_pixmap_trans)
		pchanged = 1;
	    R->am_transparent = R->am_pixmap_trans = 1;
	}
    }
    if (!R->am_pixmap_trans) {
	unsigned int    n;
/*
 * InheritPixmap transparency
 */
	D_X((stderr, "InheritPixmap Seeking to  %08lx", Xroot));
	for (i = 1; i < (int)(sizeof(R->TermWin.parent) / sizeof(Window));
	     i++) {
	    oldp = R->TermWin.parent[i];
	    XQueryTree(R->Xdisplay, R->TermWin.parent[i - 1], &root,
		       &R->TermWin.parent[i], &list, &n);
	    XFree(list);
	    D_X((stderr, "InheritPixmap Parent[%d] = %08lx", i, R->TermWin.parent[i]));
	    if (R->TermWin.parent[i] == Xroot) {
		if (oldp != None)
		    pchanged = 1;
		break;
	    }
	    if (oldp != R->TermWin.parent[i])
		pchanged = 1;
	}
	n = 0;
	if (pchanged) {
	    for (; n < (unsigned int)i; n++) {
		XGetWindowAttributes(R->Xdisplay, R->TermWin.parent[n], &wattr);
		D_X((stderr, "InheritPixmap Checking Parent[%d]: %s", n, (wattr.depth == rootdepth && wattr.class != InputOnly) ? "OK" : "FAIL"));
		if (wattr.depth != rootdepth || wattr.c_class == InputOnly) {
		    n = (int)(sizeof(R->TermWin.parent) / sizeof(Window)) + 1;
		    break;
		}
	    }
	}
	if (n > (int)(sizeof(R->TermWin.parent)
		      / sizeof(R->TermWin.parent[0]))) {
	    D_X((stderr, "InheritPixmap Turning off"));
	    XSetWindowBackground(R->Xdisplay, R->TermWin.parent[0],
				 R->PixColors[Color_fg]);
	    XSetWindowBackground(R->Xdisplay, R->TermWin.vt,
				 R->PixColors[Color_bg]);
	    R->am_transparent = 0;
	    /* XXX: also turn off Opt_transparent? */
	} else {
	    /* wait (an arbitrary period) for the WM to do its thing
	     * needed for fvwm2.2.2 (and before?) */
# ifdef HAVE_NANOSLEEP
	    struct timespec rqt;

	    rqt.tv_sec = 1;
	    rqt.tv_nsec = 0;
	    nanosleep(&rqt, NULL);
# else
	    sleep(1);	
# endif
	    D_X((stderr, "InheritPixmap Turning on (%d parents)", i - 1));
	    for (n = 0; n < (unsigned int)i; n++)
		XSetWindowBackgroundPixmap(R->Xdisplay, R->TermWin.parent[n],
					   ParentRelative);
	    XSetWindowBackgroundPixmap(R->Xdisplay, R->TermWin.vt,
				       ParentRelative);
	    R->am_transparent = 1;
	}
	for (; i < (int)(sizeof(R->TermWin.parent) / sizeof(Window)); i++)
	    R->TermWin.parent[i] = None;
    }
    return pchanged;
}
#endif

/*}}} */

/*{{{ print pipe */
/*----------------------------------------------------------------------*/
#ifdef PRINTPIPE
/* EXTPROTO */
FILE           *
rxvt_popen_printer(pR)
{
    FILE           *stream = popen(R->rs[Rs_print_pipe], "w");

    if (stream == NULL)
	rxvt_print_error("can't open printer pipe");
    return stream;
}

/* EXTPROTO */
int
rxvt_pclose_printer (FILE *stream)
{
  fflush (stream);
/* pclose() reported not to work on SunOS 4.1.3 */
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
/* INTPROTO */
void
rxvt_process_print_pipe(pR)
{
    int             done;
    FILE           *fd;

    if ((fd = rxvt_popen_printer(aR)) == NULL)
	return;

/*
 * Send all input to the printer until either ESC[4i or ESC[?4i
 * is received.
 */
    for (done = 0; !done;) {
	unsigned char   buf[8];
	unsigned char   ch;
	unsigned int    i, len;

	if ((ch = rxvt_cmd_getc(aR)) != C0_ESC) {
	    if (putc(ch, fd) == EOF)
		break;		/* done = 1 */
	} else {
	    len = 0;
	    buf[len++] = ch;

	    if ((buf[len++] = rxvt_cmd_getc(aR)) == '[') {
		if ((ch = rxvt_cmd_getc(aR)) == '?') {
		    buf[len++] = '?';
		    ch = rxvt_cmd_getc(aR);
		}
		if ((buf[len++] = ch) == '4') {
		    if ((buf[len++] = rxvt_cmd_getc(aR)) == 'i')
			break;	/* done = 1 */
		}
	    }
	    for (i = 0; i < len; i++)
		if (putc(buf[i], fd) == EOF) {
		    done = 1;
		    break;
		}
	}
    }
    rxvt_pclose_printer(fd);
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
/* INTPROTO */
void
rxvt_process_nonprinting(pR_ unsigned char ch)
{
    switch (ch) {
    case C0_ENQ:	/* terminal Status */
	if (R->rs[Rs_answerbackstring])
	    R->tt_write(
		(const unsigned char *)R->rs[Rs_answerbackstring],
		(unsigned int)STRLEN(R->rs[Rs_answerbackstring]));
	else
	    R->tt_write((unsigned char *)VT100_ANS,
			  (unsigned int)STRLEN(VT100_ANS));
	break;
    case C0_BEL:	/* bell */
	rxvt_scr_bell(aR);
	break;
    case C0_BS:		/* backspace */
	rxvt_scr_backspace(aR);
	break;
    case C0_HT:		/* tab */
	rxvt_scr_tab(aR_ 1);
	break;
    case C0_CR:		/* carriage return */
	rxvt_scr_gotorc(aR_ 0, 0, R_RELATIVE);
	break;
    case C0_VT:		/* vertical tab, form feed */
    case C0_FF:
    case C0_LF:		/* line feed */
	rxvt_scr_index(aR_ UP);
	break;
    case C0_SO:		/* shift out - acs */
	rxvt_scr_charset_choose(aR_ 1);
	break;
    case C0_SI:		/* shift in - acs */
	rxvt_scr_charset_choose(aR_ 0);
	break;
    }
}
/*}}} */


/*{{{ process VT52 escape sequences */
/* INTPROTO */
void
rxvt_process_escape_vt52(pR_ unsigned char ch)
{
    int row, col;
    
    switch (ch) {
    case 'A':		/* cursor up */
	rxvt_scr_gotorc(aR_ -1, 0, R_RELATIVE | C_RELATIVE);	
	break;
    case 'B':		/* cursor down */
	rxvt_scr_gotorc(aR_ 1, 0, R_RELATIVE | C_RELATIVE);	
	break;
    case 'C':		/* cursor right */
	rxvt_scr_gotorc(aR_ 0, 1, R_RELATIVE | C_RELATIVE);	
	break;
    case 'D':		/* cursor left */
	rxvt_scr_gotorc(aR_ 0, -1, R_RELATIVE | C_RELATIVE);	
	break;
    case 'H':		/* cursor home */
	rxvt_scr_gotorc(aR_ 0, 0, 0);	
	break;
    case 'I':		/* cursor up and scroll down if needed */
	rxvt_scr_index(aR_ DN);
	break;
    case 'J':		/* erase to end of screen */
	R->scr_erase_screen (0);
	break;
    case 'K':		/* erase to end of line */
	rxvt_scr_erase_line(aR_ 0);
	break;
    case 'Y':         	/* move to specified row and col */
	  /* full command is 'ESC Y row col' where row and col
	   * are encoded by adding 32 and sending the ascii
	   * character.  eg. SPACE = 0, '+' = 13, '0' = 18,
	   * etc. */
	row = rxvt_cmd_getc(aR) - ' ';
	col = rxvt_cmd_getc(aR) - ' ';
	rxvt_scr_gotorc(aR_ row, col, 0);
	break;
    case 'Z':		/* identify the terminal type */
	R->tt_printf("\033/Z");	/* I am a VT100 emulating a VT52 */
        break;
    case '<':		/* turn off VT52 mode */
        PrivMode(0, PrivMode_vt52);
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
/* INTPROTO */
void
rxvt_process_escape_seq(pR)
{
    unsigned char   ch = rxvt_cmd_getc(aR);

    if (R->PrivateModes & PrivMode_vt52) {
	rxvt_process_escape_vt52(aR_ ch);
	return;
    }
    
    switch (ch) {
    /* case 1:        do_tek_mode (); break; */
    case '#':
	if (rxvt_cmd_getc(aR) == '8')
	    rxvt_scr_E(aR);
	break;
    case '(':
	rxvt_scr_charset_set(aR_ 0, (unsigned int)rxvt_cmd_getc(aR));
	break;
    case ')':
	rxvt_scr_charset_set(aR_ 1, (unsigned int)rxvt_cmd_getc(aR));
	break;
    case '*':
	rxvt_scr_charset_set(aR_ 2, (unsigned int)rxvt_cmd_getc(aR));
	break;
    case '+':
	rxvt_scr_charset_set(aR_ 3, (unsigned int)rxvt_cmd_getc(aR));
	break;
#ifdef MULTICHAR_SET
    case '$':
	rxvt_scr_charset_set(aR_ -2, (unsigned int)rxvt_cmd_getc(aR));
	break;
#endif
#ifndef NO_FRILLS
    case '6':
	rxvt_scr_backindex(aR);
	break;
#endif
    case '7':
	rxvt_scr_cursor(aR_ SAVE);
	break;
    case '8':
	rxvt_scr_cursor(aR_ RESTORE);
	break;
#ifndef NO_FRILLS
    case '9':
	rxvt_scr_forwardindex(aR);
	break;
#endif
    case '=':
    case '>':
	PrivMode((ch == '='), PrivMode_aplKP);
	break;

    case C1_40:
	rxvt_cmd_getc(aR);
	break;
    case C1_44:
	rxvt_scr_index(aR_ UP);
	break;

    /* 8.3.87: NEXT LINE */
    case C1_NEL:		/* ESC E */
        {
          uint32_t nlcr[] = { '\n', '\r' };
          rxvt_scr_add_lines(aR_ nlcr, 1, 2);
        }
	break;

    /* kidnapped escape sequence: Should be 8.3.48 */
    case C1_ESA:		/* ESC G */
	rxvt_process_graphics(aR);
	break;

    /* 8.3.63: CHARACTER TABULATION SET */
    case C1_HTS:		/* ESC H */
	rxvt_scr_set_tab(aR_ 1);
	break;

    /* 8.3.105: REVERSE LINE FEED */
    case C1_RI:			/* ESC M */
	rxvt_scr_index(aR_ DN);
	break;

    /* 8.3.142: SINGLE-SHIFT TWO */
    /*case C1_SS2: scr_single_shift (2);   break; */

    /* 8.3.143: SINGLE-SHIFT THREE */
    /*case C1_SS3: scr_single_shift (3);   break; */

    /* 8.3.27: DEVICE CONTROL STRING */
    case C1_DCS:		/* ESC P */
	rxvt_process_dcs_seq(aR);
	break;

    /* 8.3.110: SINGLE CHARACTER INTRODUCER */
    case C1_SCI:		/* ESC Z */
	R->tt_write((const unsigned char *)ESCZ_ANSWER,
		      (unsigned int)(sizeof(ESCZ_ANSWER) - 1));
	break;			/* steal obsolete ESC [ c */

    /* 8.3.16: CONTROL SEQUENCE INTRODUCER */
    case C1_CSI:		/* ESC [ */
	rxvt_process_csi_seq(aR);
	break;

    /* 8.3.90: OPERATING SYSTEM COMMAND */
    case C1_OSC:		/* ESC ] */
	rxvt_process_osc_seq(aR);
	break;

    /* 8.3.106: RESET TO INITIAL STATE */
    case 'c':
	rxvt_scr_poweron(aR);
	rxvt_scrollbar_show(aR_ 1);
	break;

    /* 8.3.79: LOCKING-SHIFT TWO (see ISO2022) */
    case 'n':
	rxvt_scr_charset_choose(aR_ 2);
	break;

    /* 8.3.81: LOCKING-SHIFT THREE (see ISO2022) */
    case 'o':
	rxvt_scr_charset_choose(aR_ 3);
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
    (!!((array)[(bit) / 8] & (128 >> ((bit) & 7))))

const unsigned char csi_defaults[] = {
    make_byte(1,1,1,1,1,1,1,1),	/* @, A, B, C, D, E, F, G, */
    make_byte(1,1,0,0,1,1,0,0),	/* H, I, J, K, L, M, N, O, */
    make_byte(1,0,1,1,1,1,1,0),	/* P, Q, R, S, T, U, V, W, */
    make_byte(1,1,1,0,0,0,1,0),	/* X, Y, Z, [, \, ], ^, _, */
    make_byte(1,1,1,0,1,1,1,0),	/* `, a, b, c, d, e, f, g, */
    make_byte(0,0,1,1,0,0,0,0),	/* h, i, j, k, l, m, n, o, */
    make_byte(0,0,0,0,0,0,0,0),	/* p, q, r, s, t, u, v, w, */
    make_byte(0,0,0,0,0,0,0,0)	/* x, y, z, {, |, }, ~,    */
};
/* *INDENT-ON* */

/* INTPROTO */
void
rxvt_process_csi_seq(pR)
{
    unsigned char   ch, priv, i;
    unsigned int    nargs, p;
    int             n, ndef;
    int             arg[ESC_ARGS];

    for (nargs = ESC_ARGS; nargs > 0;)
	arg[--nargs] = 0;

    priv = 0;
    ch = rxvt_cmd_getc(aR);
    if (ch >= '<' && ch <= '?') {	/* '<' '=' '>' '?' */
	priv = ch;
	ch = rxvt_cmd_getc(aR);
    }
/* read any numerical arguments */
    for (n = -1; ch < CSI_ICH; ) {
	if (isdigit(ch)) {
	    if (n < 0)
		n = ch - '0';
	    else
		n = n * 10 + ch - '0';
	} else if (ch == ';') {
	    if (nargs < ESC_ARGS)
		arg[nargs++] = n;
	    n = -1;
	} else if (ch == '\b') {
	    rxvt_scr_backspace(aR);
	} else if (ch == C0_ESC) {
	    rxvt_process_escape_seq(aR);
	    return;
	} else if (ch < ' ') {
	    rxvt_process_nonprinting(aR_ ch);
	}
	ch = rxvt_cmd_getc(aR);
    }

    if (ch > CSI_7F)
	return;

    if (nargs < ESC_ARGS)
	arg[nargs++] = n;

    i = ch - CSI_ICH;
    ndef = get_byte_array_bit(csi_defaults, i);
    for (p = 0; p < nargs; p++)
	if (arg[p] == -1)
	    arg[p] = ndef;

#ifdef DEBUG_CMD
    fprintf(stderr, "CSI ");
    for (p = 0; p < nargs; p++)
	fprintf(stderr, "%d%s", arg[p], p < nargs - 1 ? ";" : "");
    fprintf(stderr, "%c\n", ch);
#endif

/*
 * private mode handling
 */
    if (priv) {
	switch (priv) {
	case '>':
	    if (ch == CSI_DA)	/* secondary device attributes */
		R->tt_printf("\033[>%d;%-.8s;0c", 'R', VSTRING);
	    break;
	case '?':
	    if (ch == 'h' || ch == 'l' || ch == 'r' || ch == 's' || ch == 't')
		rxvt_process_terminal_mode(aR_ ch, priv, nargs, arg);
	    break;
	}
	return;
    }

    switch (ch) {
/*
 * ISO/IEC 6429:1992(E) CSI sequences (defaults in parentheses)
 */
#ifdef PRINTPIPE
    case CSI_MC:		/* 8.3.83: (0) MEDIA COPY */
	switch (arg[0]) {
	case 0:			/* initiate transfer to primary aux device */
	    rxvt_scr_printscreen(aR_ 0);
	    break;
	case 5:			/* start relay to primary aux device */
	    rxvt_process_print_pipe(aR);
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
	rxvt_scr_gotorc(aR_ arg[0], 0, RELATIVE);
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
	rxvt_scr_gotorc(aR_ 0, arg[0], RELATIVE);
#else				/* emulate common DEC VTs */
	rxvt_scr_gotorc(aR_ 0, arg[0] ? arg[0] : 1, RELATIVE);
#endif
	break;

    case CSI_CPL:		/* 8.3.13: (1) CURSOR PRECEDING LINE */
	arg[0] = -arg[0];
    /* FALLTHROUGH */
    case CSI_CNL:		/* 8.3.12: (1) CURSOR NEXT LINE */
	rxvt_scr_gotorc(aR_ arg[0], 0, R_RELATIVE);
	break;

    case CSI_CHA:		/* 8.3.9: (1) CURSOR CHARACTER ABSOLUTE */
    case CSI_HPA:		/* 8.3.58: (1) CURSOR POSITION ABSOLUTE */
	rxvt_scr_gotorc(aR_ 0, arg[0] - 1, R_RELATIVE);
	break;

    case CSI_VPA:		/* 8.3.159: (1) LINE POSITION ABSOLUTE */
	rxvt_scr_gotorc(aR_ arg[0] - 1, 0, C_RELATIVE);
	break;

    case CSI_CUP:		/* 8.3.21: (1,1) CURSOR POSITION */
    case CSI_HVP:		/* 8.3.64: (1,1) CHARACTER AND LINE POSITION */
	rxvt_scr_gotorc(aR_ arg[0] - 1, nargs < 2 ? 0 : (arg[1] - 1), 0);
	break;

    case CSI_CBT:		/* 8.3.7: (1) CURSOR BACKWARD TABULATION */
	arg[0] = -arg[0];
    /* FALLTHROUGH */
    case CSI_CHT:		/* 8.3.10: (1) CURSOR FORWARD TABULATION */
	rxvt_scr_tab(aR_ arg[0]);
	break;

    case CSI_ED:		/* 8.3.40: (0) ERASE IN PAGE */
	R->scr_erase_screen (arg[0]);
	break;

    case CSI_EL:		/* 8.3.42: (0) ERASE IN LINE */
	rxvt_scr_erase_line(aR_ arg[0]);
	break;

    case CSI_ICH:		/* 8.3.65: (1) INSERT CHARACTER */
	rxvt_scr_insdel_chars(aR_ arg[0], INSERT);
	break;

    case CSI_IL:		/* 8.3.68: (1) INSERT LINE */
	rxvt_scr_insdel_lines(aR_ arg[0], INSERT);
	break;

    case CSI_DL:		/* 8.3.33: (1) DELETE LINE */
	rxvt_scr_insdel_lines(aR_ arg[0], DELETE);
	break;

    case CSI_ECH:		/* 8.3.39: (1) ERASE CHARACTER */
	rxvt_scr_insdel_chars(aR_ arg[0], ERASE);
	break;

    case CSI_DCH:		/* 8.3.26: (1) DELETE CHARACTER */
	rxvt_scr_insdel_chars(aR_ arg[0], DELETE);
	break;

    case CSI_SD:		/* 8.3.114: (1) SCROLL DOWN */
	arg[0] = -arg[0];
    /* FALLTHROUGH */
    case CSI_SU:		/* 8.3.148: (1) SCROLL UP */
	R->scr_scroll_text (R->screen.tscroll, R->screen.bscroll, arg[0], 0);
	break;

    case CSI_DA:		/* 8.3.24: (0) DEVICE ATTRIBUTES */
	R->tt_write((const unsigned char *)VT100_ANS,
		      (unsigned int)(sizeof(VT100_ANS) - 1));
	break;

    case CSI_SGR:		/* 8.3.118: (0) SELECT GRAPHIC RENDITION */
	rxvt_process_sgr_mode(aR_ nargs, arg);
	break;

    case CSI_DSR:		/* 8.3.36: (0) DEVICE STATUS REPORT */
	switch (arg[0]) {
	case 5:			/* DSR requested */
	    R->tt_printf("\033[0n");
	    break;
	case 6:			/* CPR requested */
	    rxvt_scr_report_position(aR);
	    break;
#if defined (ENABLE_DISPLAY_ANSWER)
	case 7:			/* unofficial extension */
	    R->tt_printf("%-.250s\n", R->rs[Rs_display_name]);
	    break;
#endif
	case 8:			/* unofficial extension */
	    rxvt_xterm_seq(aR_ XTerm_title, APL_NAME "-" VERSION, CHAR_ST);
	    break;
	}
	break;

    case CSI_TBC:		/* 8.3.155: (0) TABULATION CLEAR */
	switch (arg[0]) {
	case 0:			/* char tab stop cleared at active position */
	    rxvt_scr_set_tab(aR_ 0);
	    break;
	/* case 1: */		/* line tab stop cleared in active line */
	/* case 2: */		/* char tab stops cleared in active line */
	case 3:			/* all char tab stops are cleared */
	/* case 4: */		/* all line tab stops are cleared */
	case 5:			/* all tab stops are cleared */
	    rxvt_scr_set_tab(aR_ -1);
	    break;
	}
	break;

    case CSI_CTC:		/* 8.3.17: (0) CURSOR TABULATION CONTROL */
	switch (arg[0]) {
	case 0:			/* char tab stop set at active position */
	    rxvt_scr_set_tab(aR_ 1);
	    break;		/* = ESC H */
	/* case 1: */		/* line tab stop set at active line */
	case 2:			/* char tab stop cleared at active position */
	    rxvt_scr_set_tab(aR_ 0);
	    break;		/* = ESC [ 0 g */
	/* case 3: */		/* line tab stop cleared at active line */
	/* case 4: */		/* char tab stops cleared at active line */
	case 5:			/* all char tab stops are cleared */
	    rxvt_scr_set_tab(aR_ -1);
	    break;		/* = ESC [ 3 g */
	/* case 6: */		/* all line tab stops are cleared */
	}
	break;

    case CSI_RM:		/* 8.3.107: RESET MODE */
	if (arg[0] == 4)
	    rxvt_scr_insert_mode(aR_ 0);
	break;

    case CSI_SM:		/* 8.3.126: SET MODE */
	if (arg[0] == 4)
	    rxvt_scr_insert_mode(aR_ 1);
	break;

/*
 * PRIVATE USE beyond this point.  All CSI_7? sequences here
 */ 
    case CSI_72:		/* DECSTBM: set top and bottom margins */
	if (nargs == 1)
	    rxvt_scr_scroll_region(aR_ arg[0] - 1, MAX_ROWS - 1);
	else if (nargs == 0 || arg[0] >= arg[1])
	    rxvt_scr_scroll_region(aR_ 0, MAX_ROWS - 1);
	else 
	    rxvt_scr_scroll_region(aR_ arg[0] - 1, arg[1] - 1);
	break;

    case CSI_73:
	rxvt_scr_cursor(aR_ SAVE);
	break;
    case CSI_75:
	rxvt_scr_cursor(aR_ RESTORE);
	break;

#ifndef NO_FRILLS
    case CSI_74:
	rxvt_process_window_ops(aR_ arg, nargs);
	break;
#endif

    case CSI_78:		/* DECREQTPARM */
	if (arg[0] == 0 || arg[0] == 1)
	    R->tt_printf("\033[%d;1;1;112;112;1;0x", arg[0] + 2);
    /* FALLTHROUGH */

    default:
	break;
    }
}
/*}}} */

#ifndef NO_FRILLS
/* ARGSUSED */
/* INTPROTO */
void
rxvt_process_window_ops(pR_ const int *args, unsigned int nargs)
{
    int             x, y;
#if 0
    char           *s;
#endif
    XWindowAttributes wattr;
    Window          wdummy;

    if (nargs == 0)
	return;
    switch (args[0]) {
    /*
     * commands
     */
    case 1:			/* deiconify window */
	XMapWindow(R->Xdisplay, R->TermWin.parent[0]);
	break;
    case 2:			/* iconify window */
	XIconifyWindow(R->Xdisplay, R->TermWin.parent[0],
		       DefaultScreen(R->Xdisplay));
	break;
    case 3:			/* set position (pixels) */
	XMoveWindow(R->Xdisplay, R->TermWin.parent[0], args[1], args[2]);
	break;
    case 4:			/* set size (pixels) */
	rxvt_set_widthheight(aR_ (unsigned int)args[2], (unsigned int)args[1]);
	break;
    case 5:			/* raise window */
	XRaiseWindow(R->Xdisplay, R->TermWin.parent[0]);
	break;
    case 6:			/* lower window */
	XLowerWindow(R->Xdisplay, R->TermWin.parent[0]);
	break;
    case 7:			/* refresh window */
	R->scr_touch (true);
	break;
    case 8:			/* set size (chars) */
	rxvt_set_widthheight(aR_ (unsigned int)(args[2] * R->TermWin.fwidth),
			     (unsigned int)(args[1] * R->TermWin.fheight));
	break;
    default:
	if (args[0] >= 24)	/* set height (chars) */
	    rxvt_set_widthheight(aR_ (unsigned int)R->TermWin.width,
				 (unsigned int)(args[1] * R->TermWin.fheight));
	break;
    /*
     * reports - some output format copied from XTerm
     */
    case 11:			/* report window state */
	XGetWindowAttributes(R->Xdisplay, R->TermWin.parent[0], &wattr);
	R->tt_printf("\033[%dt", wattr.map_state == IsViewable ? 1 : 2);
	break;
    case 13:			/* report window position */
	XGetWindowAttributes(R->Xdisplay, R->TermWin.parent[0], &wattr);
	XTranslateCoordinates(R->Xdisplay, R->TermWin.parent[0], wattr.root,
			      -wattr.border_width, -wattr.border_width,
			      &x, &y, &wdummy);
	R->tt_printf("\033[3;%d;%dt", x, y);
	break;
    case 14:			/* report window size (pixels) */
	XGetWindowAttributes(R->Xdisplay, R->TermWin.parent[0], &wattr);
	R->tt_printf("\033[4;%d;%dt", wattr.height, wattr.width);
	break;
    case 18:			/* report window size (chars) */
	R->tt_printf("\033[8;%d;%dt", R->TermWin.nrow, R->TermWin.ncol);
	break;
#if 0 /* XXX: currently disabled due to security concerns */
    case 20:			/* report icon label */
	XGetIconName(R->Xdisplay, R->TermWin.parent[0], &s);
	R->tt_printf("\033]L%-.200s\234", s ? s : "");	/* 8bit ST */
	break;
    case 21:			/* report window title */
	XFetchName(R->Xdisplay, R->TermWin.parent[0], &s);
	R->tt_printf("\033]l%-.200s\234", s ? s : "");	/* 8bit ST */
	break;
#endif
    }
}
#endif

/*----------------------------------------------------------------------*/
/*
 * get input up until STRING TERMINATOR (or BEL)
 * ends_how is terminator used.  returned input must be free()d
 */
/* INTPROTO */
unsigned char  *
rxvt_get_to_st(pR_ unsigned char *ends_how)
{
    int             seen_esc = 0;	/* seen escape? */
    unsigned int    n = 0;
    unsigned char  *s;
    unsigned char   ch, string[STRING_MAX];

    for (; (ch = rxvt_cmd_getc(aR));) {
	if (ch == C0_BEL
	    || ch == CHAR_ST
	    || (ch == 0x5c && seen_esc))	/* 7bit ST */
	    break;
	if (ch == C0_ESC) {
	    seen_esc = 1;
	    continue;
	} else if (ch == '\t')
	    ch = ' ';	/* translate '\t' to space */
	else if (ch < 0x08 || (ch > 0x0d && ch < 0x20))
	    return NULL;	/* other control character - exit */
	if (n < sizeof(string) - 1)
	    string[n++] = ch;
	seen_esc = 0;
    }
    string[n++] = '\0';
    if ((s = (unsigned char *)rxvt_malloc(n)) == NULL)
	return NULL;
    *ends_how = (ch == 0x5c ? C0_ESC : ch);
    STRNCPY(s, string, n);
    return s;
}

/*----------------------------------------------------------------------*/
/*
 * process DEVICE CONTROL STRING `ESC P ... (ST|BEL)' or `0x90 ... (ST|BEL)'
 */
/* INTPROTO */
void
rxvt_process_dcs_seq(pR)
{
    unsigned char    eh, *s;
/*
 * Not handled yet
 */
    s = rxvt_get_to_st(aR_ &eh);
    if (s)
	free(s);
    return;
}

/*----------------------------------------------------------------------*/
/*
 * process OPERATING SYSTEM COMMAND sequence `ESC ] Ps ; Pt (ST|BEL)'
 */
/* INTPROTO */
void
rxvt_process_osc_seq(pR)
{
    unsigned char   ch, eh, *s;
    int             arg;

    ch = rxvt_cmd_getc(aR);
    for (arg = 0; isdigit(ch); ch = rxvt_cmd_getc(aR))
	arg = arg * 10 + (ch - '0');

    if (ch == ';') {
	s = rxvt_get_to_st(aR_ &eh);
	if (s) {
    /*
     * rxvt_menubar_dispatch() violates the constness of the string,
     * so do it here
     */
	    if (arg == XTerm_Menu)
#if 0 /* XXX: currently disabled due to security concerns */
		rxvt_menubar_dispatch(aR_ (char *)s);
#else
		0;
#endif
	    else
		rxvt_xterm_seq(aR_ arg, (char *)s, eh);
	    free(s);
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
/* EXTPROTO */
void
rxvt_xterm_seq(pR_ int op, const char *str, unsigned char resp __attribute__((unused)))
{
    int             changed = 0;
    int             color;
    char           *buf, *name;

    assert(str != NULL);
    switch (op) {
    case XTerm_name:
	rxvt_set_title(aR_ str);
    /* FALLTHROUGH */
    case XTerm_iconName:
	rxvt_set_iconName(aR_ str);
	break;
    case XTerm_title:
	rxvt_set_title(aR_ str);
	break;
    case XTerm_Color:
	for (buf = (char *)str; buf && *buf;) {
	    if ((name = STRCHR(buf, ';')) == NULL)
		break;
	    *name++ = '\0';
	    color = atoi(buf);
	    if (color < 0 || color >= TOTAL_COLORS)
		break;
	    if ((buf = STRCHR(name, ';')) != NULL)
		*buf++ = '\0';
	    rxvt_set_window_color(aR_ color + minCOLOR, name);
	}
	break;
#ifndef NO_CURSORCOLOR
    case XTerm_Color_cursor:
	rxvt_set_window_color(aR_ Color_cursor, str);
	break;
#endif
    case XTerm_Color_pointer:
	rxvt_set_window_color(aR_ Color_pointer, str);
	break;
#ifndef NO_BOLD_UNDERLINE_REVERSE
    case XTerm_Color_BD:
	rxvt_set_window_color(aR_ Color_BD, str);
	break;
    case XTerm_Color_UL:
	rxvt_set_window_color(aR_ Color_UL, str);
	break;
    case XTerm_Color_RV:
	rxvt_set_window_color(aR_ Color_RV, str);
	break;
#endif

    case XTerm_Menu:
        /*
         * rxvt_menubar_dispatch() violates the constness of the string,
         * so DON'T do it here
         */
	break;
    case XTerm_Pixmap:
	if (*str != ';') {
#if XPM_BACKGROUND
	    rxvt_scale_pixmap(aR_ "");	/* reset to default scaling */
	    rxvt_set_bgPixmap(aR_ str);	/* change pixmap */
#endif
	    R->scr_touch (true);
	}
	while ((str = STRCHR(str, ';')) != NULL) {
	    str++;
#if XPM_BACKGROUND
	    changed += rxvt_scale_pixmap(aR_ str);
#endif
	}
	if (changed) {
#ifdef XPM_BACKGROUND
	    rxvt_resize_pixmap(aR);
#endif
	    R->scr_touch (true);
	}
	break;

    case XTerm_restoreFG:
	rxvt_set_window_color(aR_ Color_fg, str);
	break;
    case XTerm_restoreBG:
	rxvt_set_window_color(aR_ Color_bg, str);
	break;
    case XTerm_logfile:
	break;
    case XTerm_font:
	rxvt_change_font(aR_ 0, str);
	break;
#if 0
    case XTerm_dumpscreen:	/* no error notices */
	{
    	    int             fd;
	    if ((fd = open(str, O_RDWR | O_CREAT | O_EXCL, 0600)) >= 0) {
		rxvt_scr_dump(aR_ fd);
		close(fd);
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
/* INTPROTO */
int
rxvt_privcases(pR_ int mode, unsigned long bit)
{
    int             state;

    if (mode == 's') {
	R->SavedModes |= (R->PrivateModes & bit);
	return -1;
    } else {
	if (mode == 'r')
	    state = (R->SavedModes & bit) ? 1 : 0;	/* no overlapping */
	else
	    state = (mode == 't') ? !(R->PrivateModes & bit) : mode;
	PrivMode(state, bit);
    }
    return state;
}

/* we're not using priv _yet_ */
/* INTPROTO */
void
rxvt_process_terminal_mode(pR_ int mode, int priv __attribute__((unused)), unsigned int nargs, const int *arg)
{
    unsigned int    i, j;
    int             state;
    static const struct {
	const int       argval;
	const unsigned long bit;
    } argtopriv[] = {
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
    };

    if (nargs == 0)
	return;

/* make lo/hi boolean */
    if (mode == 'l')
	mode = 0;		/* reset */
    else if (mode == 'h')
	mode = 1;		/* set */

    for (i = 0; i < nargs; i++) {
	state = -1;

	/* basic handling */
	for (j = 0; j < (sizeof(argtopriv)/sizeof(argtopriv[0])); j++)
	    if (argtopriv[j].argval == arg[i]) {
		state = rxvt_privcases(aR_ mode, argtopriv[j].bit);
		break;
	    }
	
	/* extra handling for values with state unkept  */
	if (state == -1)
	    switch (arg[i]) {
	    case 1048:		/* alternative cursor save */
		if (mode == 0)
		    rxvt_scr_cursor(aR_ RESTORE);
		else if (mode == 1)
		    rxvt_scr_cursor(aR_ SAVE);
	    /* FALLTHROUGH */
	    default:
		continue;	/* for(;i;) */
	    }

	/* extra handling for values with valid 0 or 1 state */
	switch (arg[i]) {
	/* case 1:	- application cursor keys */
	case 2:			/* VT52 mode */
	      /* oddball mode.  should be set regardless of set/reset
	       * parameter.  Return from VT52 mode with an ESC < from
	       * within VT52 mode
	       */
	    PrivMode(1, PrivMode_vt52);
	    break;
	case 3:			/* 80/132 */
	    if (R->PrivateModes & PrivMode_132OK)
		rxvt_set_widthheight(aR_
		    (unsigned int)((state ? 132 : 80) * R->TermWin.fwidth),
		    (unsigned int)R->TermWin.height);
	    break;
	case 4:			/* smooth scrolling */
	    if (state)
		R->Options &= ~Opt_jumpScroll;
	    else
		R->Options |= Opt_jumpScroll;
	    break;
	case 5:			/* reverse video */
	    rxvt_scr_rvideo_mode(aR_ state);
	    break;
	case 6:			/* relative/absolute origins  */
	    rxvt_scr_relative_origin(aR_ state);
	    break;
	case 7:			/* autowrap */
	    rxvt_scr_autowrap(aR_ state);
	    break;
	/* case 8:	- auto repeat, can't do on a per window basis */
	case 9:			/* X10 mouse reporting */
	    if (state)		/* orthogonal */
		R->PrivateModes &= ~(PrivMode_MouseX11);
	    break;
#ifdef menuBar_esc
	case menuBar_esc:
#ifdef MENUBAR
	    rxvt_map_menuBar(aR_ state);
#endif
	    break;
#endif
#ifdef scrollBar_esc
	case scrollBar_esc:
	    if (rxvt_scrollbar_mapping(aR_ state)) {
		R->resize_all_windows (0, 0, 0);
                R->scr_touch (true);
	    }
	    break;
#endif
	case 25:		/* visible/invisible cursor */
	    rxvt_scr_cursor_visible(aR_ state);
	    break;
	/* case 35:	- shift keys */
	/* case 40:	- 80 <--> 132 mode */
	case 47:		/* secondary screen */
	    rxvt_scr_change_screen(aR_ state);
	    break;
	/* case 66:	- application key pad */
	/* case 67:	- backspace key */
	case 1000:		/* X11 mouse reporting */
	    if (state)		/* orthogonal */
		R->PrivateModes &= ~(PrivMode_MouseX10);
	    break;
#if 0
	case 1001:
	    break;		/* X11 mouse highlighting */
#endif
	case 1010:		/* scroll to bottom on TTY output inhibit */
	    if (state)
		R->Options &= ~Opt_scrollTtyOutput;
	    else
		R->Options |= Opt_scrollTtyOutput;
	    break;
	case 1011:		/* scroll to bottom on key press */
	    if (state)
		R->Options |= Opt_scrollTtyKeypress;
	    else
		R->Options &= ~Opt_scrollTtyKeypress;
	    break;
	case 1047:		/* secondary screen w/ clearing */
	    if (R->current_screen != PRIMARY)
		R->scr_erase_screen (2);
	    rxvt_scr_change_screen(aR_ state);
	/* FALLTHROUGH */
	default:
	    break;
	}
    }
}
/*}}} */

/*{{{ process sgr sequences */
/* INTPROTO */
void
rxvt_process_sgr_mode(pR_ unsigned int nargs, const int *arg)
{
    unsigned int    i;
    short           rendset;
    int             rendstyle;

    if (nargs == 0) {
	rxvt_scr_rendition(aR_ 0, ~RS_None);
	return;
    }
    for (i = 0; i < nargs; i++) {
	rendset = -1;
	switch (arg[i]) {
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
	if (rendset != -1) {
	    rxvt_scr_rendition(aR_ rendset, rendstyle);
	    continue;		/* for(;i;) */
	}

	switch (arg[i]) {
	case 30:
	case 31:		/* set fg color */
	case 32:
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	    rxvt_scr_color(aR_ (unsigned int)(minCOLOR + (arg[i] - 30)),
			   Color_fg);
	    break;
#ifdef TTY_256COLOR
	case 38:
	    if (nargs > i + 2 && arg[i + 1] == 5) {
		rxvt_scr_color(aR_ (unsigned int)(minCOLOR + arg[i + 2]),
			       Color_fg);
		i += 2;
	    }
	    break;
#endif
	case 39:		/* default fg */
	    rxvt_scr_color(aR_ Color_fg, Color_fg);
	    break;

	case 40:
	case 41:		/* set bg color */
	case 42:
	case 43:
	case 44:
	case 45:
	case 46:
	case 47:
	    rxvt_scr_color(aR_ (unsigned int)(minCOLOR + (arg[i] - 40)),
			   Color_bg);
	    break;
#ifdef TTY_256COLOR
	case 48:
	    if (nargs > i + 2 && arg[i + 1] == 5) {
		rxvt_scr_color(aR_ (unsigned int)(minCOLOR + arg[i + 2]),
			       Color_bg);
		i += 2;
	    }
	    break;
#endif
	case 49:		/* default bg */
	    rxvt_scr_color(aR_ Color_bg, Color_bg);
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
	    rxvt_scr_color(aR_ (unsigned int)(minBrightCOLOR + (arg[i] - 90)),
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
	    rxvt_scr_color(aR_ (unsigned int)(minBrightCOLOR + (arg[i] - 100)),
			   Color_bg);
	    break;
#endif
	}
    }
}
/*}}} */

/*{{{ process Rob Nation's own graphics mode sequences */
/* INTPROTO */
void
rxvt_process_graphics(pR)
{
    unsigned char   ch, cmd = rxvt_cmd_getc(aR);

#ifndef RXVT_GRAPHICS
    if (cmd == 'Q') {		/* query graphics */
	R->tt_printf("\033G0\n");	/* no graphics */
	return;
    }
/* swallow other graphics sequences until terminating ':' */
    do
	ch = rxvt_cmd_getc(aR);
    while (ch != ':');
#else
    unsigned int    nargs;
    int             args[NGRX_PTS];
    unsigned char  *text = NULL;

    if (cmd == 'Q') {		/* query graphics */
	R->tt_printf("\033G1\n");	/* yes, graphics (color) */
	return;
    }
    for (nargs = 0; nargs < (sizeof(args) / sizeof(args[0])) - 1;) {
	int             neg;

	ch = rxvt_cmd_getc(aR);
	neg = (ch == '-');
	if (neg || ch == '+')
	    ch = rxvt_cmd_getc(aR);

	for (args[nargs] = 0; isdigit(ch); ch = rxvt_cmd_getc(aR))
	    args[nargs] = args[nargs] * 10 + (ch - '0');
	if (neg)
	    args[nargs] = -args[nargs];

	nargs++;
	args[nargs] = 0;
	if (ch != ';')
	    break;
    }

    if ((cmd == 'T') && (nargs >= 5)) {
	int             i, len = args[4];

	text = (unsigned char *)rxvt_malloc((len + 1) * sizeof(char));

	if (text != NULL) {
	    for (i = 0; i < len; i++)
		text[i] = rxvt_cmd_getc(aR);
	    text[len] = '\0';
	}
    }
    rxvt_Gr_do_graphics(aR_ cmd, nargs, args, text);
#endif
}
/*}}} */

/* ------------------------------------------------------------------------- */

/*
 * Send printf() formatted output to the command.
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
          int written = write (cmd_fd, data, min (MAX_PTY_WRITE, len));
          if (written == len)
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

