/*--------------------------------*-C-*---------------------------------*
 * File:        main.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *                              - original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *                              - extensive modifications
 * Copyright (c) 1995      Garrett D'Amore <garrett@netcom.com>
 * Copyright (c) 1997      mj olesen <olesen@me.QueensU.CA>
 *                              - extensive modifications
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 *                              - extensive modifications
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
 *---------------------------------------------------------------------*/

#include "../config.h"          /* NECESSARY */
#include "rxvt.h"               /* NECESSARY */
#include "main.intpro"          /* PROTOS for internal routines */

#include <csignal>
#include <cstring>

#ifdef TTY_GID_SUPPORT
# include <grp.h>
#endif

#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif

#ifdef KEYSYM_RESOURCE
# include "keyboard.h"
#endif

vector<rxvt_term *> rxvt_term::termlist;

static char curlocale[128];

bool
rxvt_set_locale (const char *locale)
{
  if (!locale || !strncmp (locale, curlocale, 128))
    return false;

  strncpy (curlocale, locale, 128);
  setlocale (LC_CTYPE, curlocale);
  return true;
}

#if ENABLE_COMBINING
class rxvt_composite_vec rxvt_composite;

text_t rxvt_composite_vec::compose (unicode_t c1, unicode_t c2)
{
  compose_char *cc;
  
  // break compose chains, as stupid readline really likes to duplicate
  // composing characters for some reason near the end of a line.
  cc = (*this)[c1];
  while (cc)
    {
      if (cc->c2 == c2) return c1;
      cc = (*this)[cc->c1];
    }

  // check to see wether this combination already exists otherwise
  for (cc = v.end (); cc-- > v.begin (); )
    {
      if (cc->c1 == c1 && cc->c2 == c2)
        return COMPOSE_LO + (cc - v.begin ());
    }

  // allocate a new combination
  if (v.size () == COMPOSE_HI - COMPOSE_LO + 1)
    {
      static int seen;

      if (!seen++)
        fprintf (stderr, "too many unrepresentable composite characters, try --enable-unicode3\n");

      return REPLACEMENT_CHAR;
    }

  v.push_back (compose_char (c1, c2));

  return v.size () - 1 + COMPOSE_LO;
}

int rxvt_composite_vec::expand (unicode_t c, wchar_t *r)
{
  compose_char *cc = (*this)[c];

  if (!cc)
    {
      if (r) *r = c;
      return 1;
    }

  int len = expand (cc->c1, r);

  if (r) r += len;

  if (cc->c2 != NOCHAR)
    {
      len++;
      if (r) *r++ = cc->c2;
    }

  return len;

}
#endif

rxvt_term::rxvt_term ()
    :
#if TRANSPARENT
    rootwin_ev (this, &rxvt_term::rootwin_cb),
#endif
#ifdef HAVE_SCROLLBARS
    scrollbar_ev (this, &rxvt_term::x_cb),
#endif
#ifdef MENUBAR
    menubar_ev (this, &rxvt_term::x_cb),
#endif
#ifdef CURSOR_BLINK
    cursor_blink_ev (this, &rxvt_term::cursor_blink_cb),
#endif
#ifdef TEXT_BLINK
    text_blink_ev (this, &rxvt_term::text_blink_cb),
#endif
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
    cont_scroll_ev (this, &rxvt_term::cont_scroll_cb),
#endif
#ifdef SELECTION_SCROLLING
    sel_scroll_ev (this, &rxvt_term::sel_scroll_cb),
#endif
#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
    slip_wheel_ev (this, &rxvt_term::slip_wheel_cb),
#endif
#ifdef POINTER_BLANK
    pointer_ev (this, &rxvt_term::pointer_cb),
#endif
#ifdef USE_XIM
    im_ev (this, &rxvt_term::im_cb),
#endif
    sw_term (this, &rxvt_term::sig_term),
    sw_int (this, &rxvt_term::sig_term),
    sw_chld (this, &rxvt_term::sig_chld),
    termwin_ev (this, &rxvt_term::x_cb),
    vt_ev (this, &rxvt_term::x_cb),
    check_ev (this, &rxvt_term::check_cb),
    flush_ev (this, &rxvt_term::flush_cb),
    destroy_ev (this, &rxvt_term::destroy_cb),
    pty_ev (this, &rxvt_term::pty_cb),
    incr_ev (this, &rxvt_term::incr_cb)
{
  cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;

  termlist.push_back (this);

#ifdef KEYSYM_RESOURCE
  keyboard = new keyboard_manager;

  if (!keyboard)
    rxvt_fatal ("out of memory, aborting.\n");
#endif
}

// clean up the most important stuff, do *not* call x or free mem etc.
// for use before an emergency exit
void rxvt_term::emergency_cleanup ()
{
  if (cmd_pid)
    kill (-cmd_pid, SIGHUP);

#ifdef UTMP_SUPPORT
  privileged_utmp (RESTORE);
#endif

  pty.put ();
}

rxvt_term::~rxvt_term ()
{
  termlist.erase (find (termlist.begin (), termlist.end(), this));

  emergency_cleanup ();

#if ENABLE_STYLES
  for (int i = RS_styleCount; --i; )
    if (TermWin.fontset[i] != TermWin.fontset[0])
      delete TermWin.fontset[i];
#endif
  delete TermWin.fontset[0];

  if (display)
    {
      dDisp;

      selection_clear ();

#ifdef USE_XIM
      im_destroy ();
#endif
#ifdef MENUBAR
      if (menubarGC)    XFreeGC (disp, menubarGC);
#endif
#ifdef XTERM_SCROLLBAR
      if (xscrollbarGC) XFreeGC (disp, xscrollbarGC);
      if (ShadowGC)     XFreeGC (disp, ShadowGC);
#endif
#ifdef PLAIN_SCROLLBAR
      if (pscrollbarGC) XFreeGC (disp, pscrollbarGC);
#endif
#ifdef NEXT_SCROLLBAR
      if (blackGC)      XFreeGC (disp, blackGC);
      if (whiteGC)      XFreeGC (disp, whiteGC);
      if (grayGC)       XFreeGC (disp, grayGC);
      if (darkGC)       XFreeGC (disp, darkGC);
      if (stippleGC)    XFreeGC (disp, stippleGC);
      if (dimple)       XFreePixmap (disp, dimple);
      if (upArrow)      XFreePixmap (disp, upArrow);
      if (downArrow)    XFreePixmap (disp, downArrow);
      if (upArrowHi)    XFreePixmap (disp, upArrowHi);
      if (downArrowHi)  XFreePixmap (disp, downArrowHi);
#endif
#if defined(MENUBAR) || defined(RXVT_SCROLLBAR)
      if (topShadowGC)  XFreeGC (disp, topShadowGC);
      if (botShadowGC)  XFreeGC (disp, botShadowGC);
      if (scrollbarGC)  XFreeGC (disp, scrollbarGC);
#endif
      if (TermWin.gc)   XFreeGC (disp, TermWin.gc);

#if defined(MENUBAR) && (MENUBAR_MAX > 1)
      delete menuBar.drawable;
      //if (menuBar.win)
      //  XDestroyWindow (disp, menuBar.win);
#endif
      delete TermWin.drawable;
      // destroy all windows
      if (TermWin.parent[0] && !rs[Rs_embed])
        XDestroyWindow (disp, TermWin.parent[0]);
    }

  // TODO: free pixcolours, colours should become part of rxvt_display

  delete pix_colors_focused;
#ifdef OFF_FOCUS_FADING
  delete pix_colors_unfocused;
#endif

  displays.put (display);

  scr_release ();

  /* clear all resources */
  for (int i = 0; i < allocated.size (); i++)
    free (allocated [i]);

  free (selection.text);
  // TODO: manage env vars in child only(!)
  free (env_windowid);
  free (env_display);
  free (env_term);
  free (env_colorfgbg);
  free (locale);
  free (v_buffer);
  free (incr_buf);

  delete envv;
  delete argv;

#ifdef KEYSYM_RESOURCE
  delete keyboard;
#endif
}

void
rxvt_term::destroy ()
{
  if (destroy_ev.active)
    return;

#if ENABLE_OVERLAY
  scr_overlay_off ();
#endif

  if (display)
    {
#if USE_XIM
      im_ev.stop (display);
#endif
#if HAVE_SCROLLBARS
      scrollbar_ev.stop (display);
#endif
#if MENUBAR
      menubar_ev.stop (display);
#endif
#if TRANSPARENT
      rootwin_ev.stop (display);
#endif
      incr_ev.stop ();
      termwin_ev.stop (display);
      vt_ev.stop (display);
    }

  check_ev.stop ();
  pty_ev.stop ();
#ifdef CURSOR_BLINK
  cursor_blink_ev.stop ();
#endif
#ifdef TEXT_BLINK
  text_blink_ev.stop ();
#endif
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
  cont_scroll_ev.stop ();
#endif
#ifdef SELECTION_SCROLLING
  sel_scroll_ev.stop ();
#endif
#ifdef POINTER_BLANK
  pointer_ev.stop ();
#endif

  destroy_ev.start (0);
}

void
rxvt_term::destroy_cb (time_watcher &w)
{
  SET_R (this);

  delete this;
}

/*----------------------------------------------------------------------*/
/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 * TODO: if debugging, this should free up any known resources if we can
 */
static XErrorHandler old_xerror_handler;

static void
rxvt_emergency_cleanup ()
{
  for (rxvt_term **t = rxvt_term::termlist.begin (); t < rxvt_term::termlist.end (); t++)
    (*t)->emergency_cleanup ();
}

#if ENABLE_FRILLS
static void
print_x_error (Display *dpy, XErrorEvent *event)
{
    char buffer[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    char *mtype = "XlibMessage";
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    rxvt_warn ("An X Error occured, trying to continue after report.\n");
    rxvt_warn ("%s:  %s\n", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", mesg, BUFSIZ);
    rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->request_code);
    sprintf(number, "%d", event->request_code);
    XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
    rxvt_warn ("(which is %s)\n", buffer);
    if (event->request_code >= 128) {
	XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code %d",
			      mesg, BUFSIZ);
        rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->minor_code);
    }
    if ((event->error_code == BadWindow) ||
	       (event->error_code == BadPixmap) ||
	       (event->error_code == BadCursor) ||
	       (event->error_code == BadFont) ||
	       (event->error_code == BadDrawable) ||
	       (event->error_code == BadColor) ||
	       (event->error_code == BadGC) ||
	       (event->error_code == BadIDChoice) ||
	       (event->error_code == BadValue) ||
	       (event->error_code == BadAtom)) {
	if (event->error_code == BadValue)
	    XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x",
				  mesg, BUFSIZ);
	else if (event->error_code == BadAtom)
	    XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
				  mesg, BUFSIZ);
	else
	    XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
				  mesg, BUFSIZ);
	rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->resourceid);
    }
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", 
			  mesg, BUFSIZ);
    rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->serial);
}
#endif

int
rxvt_xerror_handler (Display *display, XErrorEvent *event)
{
  if (GET_R->allowedxerror == -1)
    GET_R->allowedxerror = event->error_code;
  else
    {
      //TODO: GET_R is most likely not the terminal which caused the error
      //TODO: maybe just output the error and continue?
#if ENABLE_FRILLS
      print_x_error (display, event);
#else
      old_xerror_handler (display, event);
#endif
    }

  return 0;
}

int
rxvt_xioerror_handler (Display *display)
{
  rxvt_warn ("X connection to '%s' broken, unable to recover, exiting.\n",
             DisplayString (display));
  rxvt_emergency_cleanup ();
  _exit (EXIT_FAILURE);
}

/*
 * Catch a fatal signal and tidy up before quitting
 */
void
rxvt_term::sig_term (sig_watcher &w)
{
#ifdef DEBUG_CMD
  rxvt_warn ("caught signal %d, exiting.\n", w.signum);
#endif
  rxvt_emergency_cleanup ();
  signal (w.signum, SIG_DFL);
  kill (getpid (), w.signum);
}

/*----------------------------------------------------------------------*/
/* rxvt_init () */
bool
rxvt_term::init (int argc, const char *const *argv)
{
  SET_R (this);

  set_locale ("");

  if (!init_vars ())
    return false;

  init_secondary ();

  const char **cmd_argv = init_resources (argc, argv);

#ifdef KEYSYM_RESOURCE
  keyboard->register_done ();
#endif

#if MENUBAR_MAX
  menubar_read (rs[Rs_menu]);
#endif
#ifdef HAVE_SCROLLBARS
  if (options & Opt_scrollBar)
    scrollBar.setIdle ();    /* set existence for size calculations */
#endif

  create_windows (argc, argv);

  dDisp;

  init_xlocale ();

  scr_reset ();         /* initialize screen */

#if 0
  XSynchronize (disp, True);
#endif

#ifdef HAVE_SCROLLBARS
  if (options & Opt_scrollBar)
    resize_scrollbar ();      /* create and map scrollbar */
#endif
#if (MENUBAR_MAX)
  if (menubar_visible ())
    XMapWindow (disp, menuBar.win);
#endif
#ifdef TRANSPARENT
  if (options & Opt_transparent)
    {
      XSelectInput (disp, display->root, PropertyChangeMask);
      check_our_parents ();
      rootwin_ev.start (display, display->root);
    }
#endif

  XMapWindow (disp, TermWin.vt);
  XMapWindow (disp, TermWin.parent[0]);

  set_colorfgbg ();

  init_command (cmd_argv);

  free (cmd_argv);

  pty_ev.start (pty.pty, EVENT_READ);

  check_ev.start ();

  return true;
}

void
rxvt_init ()
{
  /*
   * Save and then give up any super-user privileges
   * If we need privileges in any area then we must specifically request it.
   * We should only need to be root in these cases:
   *  1.  write utmp entries on some systems
   *  2.  chown tty on some systems
   */
  rxvt_privileges (SAVE);
  rxvt_privileges (IGNORE);

  signal (SIGHUP,  SIG_IGN);
  signal (SIGPIPE, SIG_IGN);

  /* need to trap SIGURG for SVR4 (Unixware) rlogin */
  /* signal (SIGURG, SIG_DFL); */

  old_xerror_handler = XSetErrorHandler ((XErrorHandler) rxvt_xerror_handler);
  // TODO: handle this with exceptions and tolerate the memory loss
  XSetIOErrorHandler (rxvt_xioerror_handler);
}

/* ------------------------------------------------------------------------- *
 *                       SIGNAL HANDLING & EXIT HANDLER                      *
 * ------------------------------------------------------------------------- */
/*
 * Catch a SIGCHLD signal and exit if the direct child has died
 */

void
rxvt_term::sig_chld (sig_watcher &w)
{
  // we are being called for every SIGCHLD, not just ours
  int pid;

  while ((pid = waitpid (-1, NULL, WNOHANG)) > 0)
    for (rxvt_term **t = termlist.begin (); t < termlist.end (); t++)
      if (pid == (*t)->cmd_pid)
        {
          (*t)->destroy ();
          break;
        }
}

/* ------------------------------------------------------------------------- *
 *                         MEMORY ALLOCATION WRAPPERS                        *
 * ------------------------------------------------------------------------- */
void *
rxvt_malloc (size_t size)
{
  void *p = malloc (size);

  if (!p)
    rxvt_fatal ("memory allocation failure. aborting.\n");

  return p;
}

/* INTPROTO */
void           *
rxvt_calloc (size_t number, size_t size)
{
  void *p = calloc (number, size);

  if (!p)
    rxvt_fatal ("memory allocation failure. aborting.\n");

  return p;
}

/* INTPROTO */
void           *
rxvt_realloc (void *ptr, size_t size)
{
  void *p = realloc (ptr, size);

  if (!p)
    rxvt_fatal ("memory allocation failure. aborting.\n");

  return p;
}

/* ------------------------------------------------------------------------- *
 *                            PRIVILEGED OPERATIONS                          *
 * ------------------------------------------------------------------------- */
/* take care of suid/sgid super-user (root) privileges */
void
rxvt_privileges (rxvt_privaction action)
{
#if (defined(HAVE_SETEUID) || defined(HAVE_SETREUID)) && !defined(__CYGWIN32__)
  static uid_t euid;
  static gid_t egid;
#endif

#if ! defined(__CYGWIN32__)
# if !defined(HAVE_SETEUID) && defined(HAVE_SETREUID)
  /* setreuid () is the poor man's setuid (), seteuid () */
#  define seteuid(a)    setreuid(-1, (a))
#  define setegid(a)    setregid(-1, (a))
#  define HAVE_SETEUID
# endif
# ifdef HAVE_SETEUID
  switch (action)
    {
      case IGNORE:
        /*
         * change effective uid/gid - not real uid/gid - so we can switch
         * back to root later, as required
         */
        seteuid (getuid ());
        setegid (getgid ());
        break;
      case SAVE:
        euid = geteuid ();
        egid = getegid ();
        break;
      case RESTORE:
        seteuid (euid);
        setegid (egid);
        break;
    }
# else
  switch (action)
    {
      case IGNORE:
        setuid (getuid ());
        setgid (getgid ());
        /* FALLTHROUGH */
      case SAVE:
        /* FALLTHROUGH */
      case RESTORE:
        break;
    }
# endif
#endif
}

#ifdef UTMP_SUPPORT
void
rxvt_term::privileged_utmp (rxvt_privaction action)
{
  if ((options & Opt_utmpInhibit)
      || !pty.name || !*pty.name)
    return;

  rxvt_privileges (RESTORE);

  if (action == SAVE)
    makeutent (pty.name, rs[Rs_display_name]);
  else
    cleanutent ();

  rxvt_privileges (IGNORE);
}
#endif

/*----------------------------------------------------------------------*/
/*
 * window size/position calculcations for XSizeHint and other storage.
 * if width/height are non-zero then override calculated width/height
 */
void
rxvt_term::window_calc (unsigned int width, unsigned int height)
{
  short recalc_x, recalc_y;
  int x, y, sb_w, mb_h, flags;
  unsigned int w, h;
  unsigned int max_width, max_height;
  dDisp;

  D_SIZE ((stderr, "< Cols/Rows: %3d x %3d ; Width/Height: %4d x %4d",
          TermWin.ncol, TermWin.nrow, szHint.width,
          szHint.height));
  szHint.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
  szHint.win_gravity = NorthWestGravity;
  /* szHint.min_aspect.x = szHint.min_aspect.y = 1; */

  recalc_x = recalc_y = 0;
  flags = 0;
  if (!parsed_geometry)
    {
      parsed_geometry = 1;
      if (rs[Rs_geometry])
        flags = XParseGeometry (rs[Rs_geometry], &x, &y, &w, &h);

      if (flags & WidthValue)
        {
          TermWin.ncol = BOUND_POSITIVE_INT16 (w);
          szHint.flags |= USSize;
        }

      if (flags & HeightValue)
        {
          TermWin.nrow = BOUND_POSITIVE_INT16 (h);
          szHint.flags |= USSize;
        }

      if (flags & XValue)
        {
          szHint.x = x;
          szHint.flags |= USPosition;
          if (flags & XNegative)
            {
              recalc_x = 1;
              szHint.win_gravity = NorthEastGravity;
            }
        }

      if (flags & YValue)
        {
          szHint.y = y;
          szHint.flags |= USPosition;
          if (flags & YNegative)
            {
              recalc_y = 1;
              if (szHint.win_gravity == NorthEastGravity)
                szHint.win_gravity = SouthEastGravity;
              else
                szHint.win_gravity = SouthWestGravity;
            }
        }
    }

  /* TODO: BOUNDS */
  TermWin.width = TermWin.ncol * TermWin.fwidth;
  TermWin.height = TermWin.nrow * TermWin.fheight;
  max_width = MAX_COLS * TermWin.fwidth;
  max_height = MAX_ROWS * TermWin.fheight;

  szHint.base_width = szHint.base_height = 2 * TermWin.int_bwidth;

  sb_w = mb_h = 0;
  window_vt_x = window_vt_y = TermWin.int_bwidth;

  if (scrollbar_visible ())
    {
      sb_w = scrollbar_TotalWidth ();
      szHint.base_width += sb_w;
      if (!(options & Opt_scrollBar_right))
        window_vt_x += sb_w;
    }

  if (menubar_visible ())
    {
      mb_h = menuBar_TotalHeight ();
      szHint.base_height += mb_h;
      window_vt_y += mb_h;
    }

  szHint.width_inc = TermWin.fwidth;
  szHint.height_inc = TermWin.fheight;
  szHint.min_width = szHint.base_width + szHint.width_inc;
  szHint.min_height = szHint.base_height + szHint.height_inc;

  if (width && width - szHint.base_width < max_width)
    {
      szHint.width = width;
      TermWin.width = width - szHint.base_width;
    }
  else
    {
      MIN_IT (TermWin.width, max_width);
      szHint.width = szHint.base_width + TermWin.width;
    }

  if (height && height - szHint.base_height < max_height)
    {
      szHint.height = height;
      TermWin.height = height - szHint.base_height;
    }
  else
    {
      MIN_IT (TermWin.height, max_height);
      szHint.height = szHint.base_height + TermWin.height;
    }

  if (scrollbar_visible () && (options & Opt_scrollBar_right))
    window_sb_x = szHint.width - sb_w;

  if (recalc_x)
    szHint.x += (DisplayWidth (disp, display->screen)
                 - szHint.width - 2 * TermWin.ext_bwidth);
  if (recalc_y)
    szHint.y += (DisplayHeight (disp, display->screen)
                 - szHint.height - 2 * TermWin.ext_bwidth);

  TermWin.ncol = TermWin.width / TermWin.fwidth;
  TermWin.nrow = TermWin.height / TermWin.fheight;
  D_SIZE ((stderr, "> Cols/Rows: %3d x %3d ; Width/Height: %4d x %4d",
          TermWin.ncol, TermWin.nrow, szHint.width,
          szHint.height));
  return;
}

/*----------------------------------------------------------------------*/
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
void
rxvt_term::tt_winch ()
{
  if (pty.pty < 0)
    return;

  struct winsize ws;

  ws.ws_col = TermWin.ncol;
  ws.ws_row = TermWin.nrow;
  ws.ws_xpixel = TermWin.width;
  ws.ws_ypixel = TermWin.height;
  (void)ioctl (pty.pty, TIOCSWINSZ, &ws);

#if 0
  // TIOCSWINSZ⎈ is supposed to do this automatically and correctly
  if (cmd_pid)               /* force through to the command */
    kill (cmd_pid, SIGWINCH);
#endif
}

/*----------------------------------------------------------------------*/
/* set_fonts () - load and set the various fonts
/*
 * init = 1   - initialize
 *
 * fontname == FONT_UP  - switch to bigger font
 * fontname == FONT_DN  - switch to smaller font
 */
bool
rxvt_term::set_fonts ()
{
  rxvt_fontset *fs = new rxvt_fontset (this);
  rxvt_fontprop prop;

  if (!fs
      || !fs->populate (rs[Rs_font] ? rs[Rs_font] : "fixed")
      || !fs->realize_font (1))
    {
      delete fs;
      return false;
    }

#if ENABLE_STYLES
  for (int i = RS_styleCount; --i; )
    if (TermWin.fontset[i] != TermWin.fontset[0])
      delete TermWin.fontset[i];
#endif

  delete TermWin.fontset[0];
  TermWin.fontset[0] = fs;

  prop = (*fs)[1]->properties ();
  prop.height += TermWin.lineSpace;
  fs->set_prop (prop);

  TermWin.fwidth  = prop.width;
  TermWin.fheight = prop.height;
  TermWin.fweight = prop.weight;
  TermWin.fslant  = prop.slant;
  TermWin.fbase   = (*fs)[1]->ascent;

  for (int style = 1; style < 4; style++)
    {
#if ENABLE_STYLES
      const char *res = rs[Rs_font + style];

      if (res && !*res)
        TermWin.fontset[style] = TermWin.fontset[0];
      else
        {
          TermWin.fontset[style] = fs = new rxvt_fontset (this);
          rxvt_fontprop prop2 = prop;

          if (res)
            prop2.weight = prop2.slant = rxvt_fontprop::unset;
          else
            {
              res = TermWin.fontset[0]->fontdesc;

              if (SET_STYLE (0, style) & RS_Bold)   prop2.weight = rxvt_fontprop::bold;
              if (SET_STYLE (0, style) & RS_Italic) prop2.slant  = rxvt_fontprop::italic;
            }

          fs->populate (res);
          fs->set_prop (prop2);
        }
#else
      TermWin.fontset[style] = TermWin.fontset[0];
#endif
    }

  if (TermWin.parent[0])
    {
      resize_all_windows (0, 0, 0);
      scr_remap_chars ();
      scr_touch (true);
    }   

  return true;
}

void rxvt_term::set_string_property (Atom prop, const char *str, int len)
{
  // TODO: SMART_WINDOW_TITLE
  XChangeProperty (display->display, TermWin.parent[0],
                   prop, XA_STRING, 8, PropModeReplace,
                   (const unsigned char *)str, len >= 0 ? len : strlen (str));
}

void rxvt_term::set_utf8_property (Atom prop, const char *str, int len)
{
  // TODO: SMART_WINDOW_TITLE
  wchar_t *ws = rxvt_mbstowcs (str, len);
  char *s = rxvt_wcstoutf8 (ws);

  XChangeProperty (display->display, TermWin.parent[0],
                   prop, xa[XA_UTF8_STRING], 8, PropModeReplace,
                   (const unsigned char *)s, strlen (s));

  free (s);
  free (ws);
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* xterm sequences - title, iconName, color (exptl) */
void
rxvt_term::set_title (const char *str)
{
  set_string_property (XA_WM_NAME, str);
#if ENABLE_FRILLS
  set_utf8_property (xa[XA_NET_WM_NAME], str);
#endif
}

void
rxvt_term::set_icon_name (const char *str)
{
  set_string_property (XA_WM_ICON_NAME, str);
#if ENABLE_FRILLS
  set_utf8_property (xa[XA_NET_WM_ICON_NAME], str);
#endif
}

#ifdef XTERM_COLOR_CHANGE
void
rxvt_term::set_window_color (int idx, const char *color)
{
  rxvt_color xcol;
  int i;

  if (color == NULL || *color == '\0')
    return;

  /* handle color aliases */
  if (isdigit (*color))
    {
      i = atoi (color);

      if (i >= 8 && i <= 15)
        {        /* bright colors */
          i -= 8;
# ifndef NO_BRIGHTCOLOR
          pix_colors_focused[idx] = pix_colors_focused[minBrightCOLOR + i];
          SET_PIXCOLOR (idx);
          goto Done;
# endif
        }

      if (i >= 0 && i <= 7)
        { /* normal colors */
          pix_colors_focused[idx] = pix_colors_focused[minCOLOR + i];
          SET_PIXCOLOR (idx);
          goto Done;
        }
    }

  if (!rXParseAllocColor (&xcol, color))
    return;

  /* XStoreColor (display->display, display->cmap, XColor*); */

  /*
   * FIXME: should free colors here, but no idea how to do it so instead,
   * so just keep gobbling up the colormap
   */
# if 0
  for (i = Color_Black; i <= Color_White; i++)
    if (pix_colors[idx] == pix_colors[i])
      break;
  if (i > Color_White)
    {
      /* fprintf (stderr, "XFreeColors: pix_colors [%d] = %lu\n", idx, pix_colors [idx]); */
      XFreeColors (display->display, display->cmap, (pix_colors + idx), 1,
                  DisplayPlanes (display->display, display->screen));
    }
# endif

  pix_colors_focused[idx] = xcol;
  SET_PIXCOLOR (idx);

  /* XSetWindowAttributes attr; */
  /* Cursor cursor; */
Done:
#ifdef OFF_FOCUS_FADING
  if (rs[Rs_fade])
    pix_colors_unfocused[idx] = pix_colors_focused[idx].fade (display, atoi (rs[Rs_fade]));
#endif

  /*TODO: handle Color_BD, scrollbar background, etc. */

  recolour_cursor ();
  scr_recolour ();
}

#else
# define set_window_color (idx,color)   ((void)0)
#endif                          /* XTERM_COLOR_CHANGE */

void
rxvt_term::recolour_cursor ()
{
  XColor xcol[2];

  xcol[0].pixel = ISSET_PIXCOLOR (Color_pointer_fg)
                     ? pix_colors_focused[Color_pointer_fg]
                     : pix_colors_focused[Color_fg];
  xcol[1].pixel = ISSET_PIXCOLOR (Color_pointer_bg)
                     ? pix_colors_focused[Color_pointer_bg]
                     : pix_colors_focused[Color_bg];

  XQueryColors (display->display, display->cmap, xcol, 2);
  XRecolorCursor (display->display, TermWin_cursor, xcol + 0, xcol + 1);
}

/*----------------------------------------------------------------------*/
/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
void
rxvt_term::set_colorfgbg ()
{
  unsigned int i;
  const char *xpmb = "\0";
  char fstr[sizeof ("default") + 1], bstr[sizeof ("default") + 1];

  env_colorfgbg = (char *)rxvt_malloc (sizeof ("COLORFGBG=default;default;bg") + 1);
  strcpy (fstr, "default");
  strcpy (bstr, "default");
  for (i = Color_Black; i <= Color_White; i++)
    if (pix_colors[Color_fg] == pix_colors[i])
      {
        sprintf (fstr, "%d", (i - Color_Black));
        break;
      }

  for (i = Color_Black; i <= Color_White; i++)
    if (pix_colors[Color_bg] == pix_colors[i])
      {
        sprintf (bstr, "%d", (i - Color_Black));
#ifdef XPM_BACKGROUND
        xpmb = "default;";
#endif
        break;
      }

  sprintf (env_colorfgbg, "COLORFGBG=%s;%s%s", fstr, xpmb, bstr);
}

/*----------------------------------------------------------------------*/

int
rxvt_term::rXParseAllocColor (rxvt_color *screen_in_out, const char *colour)
{
  if (!screen_in_out->set (display, colour))
    {
      rxvt_warn ("can't get colour '%s', continuing without.\n", colour);
      return false;
    }

  return true;
}

/* -------------------------------------------------------------------- *
 * -                         WINDOW RESIZING                          - *
 * -------------------------------------------------------------------- */
void
rxvt_term::resize_all_windows (unsigned int width, unsigned int height, int ignoreparent)
{
  int fix_screen;
#ifdef SMART_RESIZE
  int old_width = szHint.width, old_height = szHint.height;
#endif
  dDisp;

  window_calc (width, height);
  XSetWMNormalHints (disp, TermWin.parent[0], &szHint);

  if (!ignoreparent)
    {
#ifdef SMART_RESIZE
      /*
       * resize by Marius Gedminas <marius.gedminas@uosis.mif.vu.lt>
       * reposition window on resize depending on placement on screen
       */
      int x, y, x1, y1;
      int dx, dy;
      unsigned int unused_w1, unused_h1, unused_b1, unused_d1;
      Window unused_cr;

      XTranslateCoordinates (disp, TermWin.parent[0], display->root,
                             0, 0, &x, &y, &unused_cr);
      XGetGeometry (disp, TermWin.parent[0], &unused_cr, &x1, &y1,
                    &unused_w1, &unused_h1, &unused_b1, &unused_d1);
      /*
       * if display->root isn't the parent window, a WM will probably have offset
       * our position for handles and decorations.  Counter it
       */
      if (x1 != x || y1 != y)
        {
          x -= x1;
          y -= y1;
        }

      x1 = (DisplayWidth (disp, display->screen) - old_width) / 2;
      y1 = (DisplayHeight (disp, display->screen) - old_height) / 2;
      dx = old_width - szHint.width;
      dy = old_height - szHint.height;

      /* Check position of the center of the window */
      if (x < x1)             /* left half */
        dx = 0;
      else if (x == x1)       /* exact center */
        dx /= 2;
      if (y < y1)             /* top half */
        dy = 0;
      else if (y == y1)       /* exact center */
        dy /= 2;

      XMoveResizeWindow (disp, TermWin.parent[0], x + dx, y + dy,
                         szHint.width, szHint.height);
#else
      XResizeWindow (disp, TermWin.parent[0], szHint.width, szHint.height);
#endif
    }

  fix_screen = TermWin.ncol != prev_ncol || TermWin.nrow != prev_nrow;

  if (fix_screen || width != old_width || height != old_height)
    {
      if (scrollbar_visible ())
        {
          XMoveResizeWindow (disp, scrollBar.win,
                             window_sb_x, 0,
                             scrollbar_TotalWidth (), szHint.height);
          resize_scrollbar ();
        }

      if (menubar_visible ())
        XMoveResizeWindow (disp, menuBar.win,
                           window_vt_x, 0,
                           TermWin_TotalWidth (), menuBar_TotalHeight ());

      XMoveResizeWindow (disp, TermWin.vt,
                         window_vt_x, window_vt_y,
                         TermWin_TotalWidth (), TermWin_TotalHeight ());

      scr_clear ();
#ifdef XPM_BACKGROUND
      resize_pixmap ();
#endif
    }

  if (fix_screen || old_height == 0)
    {
      int curr_screen = -1;
      int old_ncol = prev_ncol;

      /* scr_reset only works on the primary screen */
      if (old_height)      /* this is not the first time through */
        {
          unsigned int ncol = TermWin.ncol;
          TermWin.ncol = prev_ncol; // save b/c scr_blank_screen_mem uses this
          curr_screen = scr_change_screen (PRIMARY);
          TermWin.ncol = ncol;
        }

      scr_reset ();

      if (curr_screen >= 0) /* this is not the first time through */
        {
          scr_change_screen (curr_screen);
          selection_check (old_ncol != TermWin.ncol ? 4 : 0);
        }
    }

  old_width = szHint.width;
  old_height = szHint.height;

#ifdef XPM_BACKGROUND
  if (TermWin.pixmap)
    scr_touch (false);
#endif

#ifdef USE_XIM
  IMSetStatusPosition ();
#endif
}

/*
 * Set the width/height of the vt window in characters.  Units are pixels.
 * good for toggling 80/132 columns
 */
void
rxvt_term::set_widthheight (unsigned int width, unsigned int height)
{
  XWindowAttributes wattr;

  if (width == 0 || height == 0)
    {
      XGetWindowAttributes (display->display, display->root, &wattr);
      if (width == 0)
        width = wattr.width - szHint.base_width;
      if (height == 0)
        height = wattr.height - szHint.base_height;
    }

  if (width != TermWin.width || height != TermWin.height)
    {
      width += szHint.base_width;
      height += szHint.base_height;
      resize_all_windows (width, height, 0);
    }
}

/* -------------------------------------------------------------------- *
 * -                      X INPUT METHOD ROUTINES                     - *
 * -------------------------------------------------------------------- */
#ifdef USE_XIM

void
rxvt_term::im_set_color (unsigned long &fg, unsigned long &bg)
{
  fg = pix_colors[Color_fg];
  bg = pix_colors[Color_bg];
}

void
rxvt_term::im_set_size (XRectangle &size)
{
  // the int_bwidth terms make no sense to me
  size.x      = TermWin.int_bwidth;
  size.y      = TermWin.int_bwidth;
  size.width  = Width2Pixel (TermWin.ncol) + TermWin.int_bwidth;
  size.height = Height2Pixel (TermWin.nrow) + TermWin.int_bwidth;
}

void
rxvt_term::im_set_preedit_area (XRectangle &preedit_rect,
                                XRectangle &status_rect,
                                const XRectangle &needed_rect)
{
  preedit_rect.x      = needed_rect.width;
  preedit_rect.y      = 0;
  preedit_rect.width  = Width2Pixel (TermWin.ncol) - needed_rect.width + 1;
  preedit_rect.height = TermWin.fheight;

  status_rect.x       = 0;
  status_rect.y       = 0;
  status_rect.width   = needed_rect.width ? needed_rect.width : Width2Pixel (TermWin.ncol) + 1;
  status_rect.height  = TermWin.fheight;
}

/* Checking whether input method is running. */
bool
rxvt_term::IMisRunning ()
{
  char *p;
  Atom atom;
  Window win;
  char server[IMBUFSIZ];

  /* get current locale modifier */
  if ((p = XSetLocaleModifiers (NULL)) != NULL)
    {
      strcpy (server, "@server=");
      strncat (server, & (p[4]), IMBUFSIZ - 9); /* skip "@im=" */

      if ((p = strchr (server + 1, '@')) != NULL)      /* first one only */
        *p = '\0';

      atom = XInternAtom (display->display, server, False);
      win = XGetSelectionOwner (display->display, atom);

      if (win != None)
        return True;
    }

  return False;
}

void
rxvt_term::IMSendSpot ()
{
  XPoint nspot;
  XVaNestedList preedit_attr;

  if (!Input_Context
      || !TermWin.focus
      || !(input_style & XIMPreeditPosition))
    return;

  im_set_position (nspot);

  if (nspot.x == spot.x && nspot.y == spot.y)
    return;

  spot = nspot;

  preedit_attr = XVaCreateNestedList (0, XNSpotLocation, &spot, NULL);
  XSetICValues (Input_Context, XNPreeditAttributes, preedit_attr, NULL);
  XFree (preedit_attr);
}

void
rxvt_term::im_destroy ()
{
  if (input_method)
    {
      if (Input_Context && input_method->xim)
        XDestroyIC (Input_Context);

      display->put_xim (input_method);
      input_method = 0;
    }

  Input_Context = 0;
}

/*
 * Try to open a XIM with the current modifiers, then see if we can
 * open a suitable preedit type
 */
bool
rxvt_term::IM_get_IC (const char *modifiers)
{
  int i, j, found;
  XIM xim;
  XPoint spot;
  XRectangle rect, status_rect, needed_rect;
  unsigned long fg, bg;
  const char *p;
  char **s;
  XIMStyles *xim_styles;

  if (! ((p = XSetLocaleModifiers (modifiers)) && *p))
    return false;

  D_MAIN ((stderr, "rxvt_IM_get_IC ()"));
  input_method = display->get_xim (locale, modifiers);
  if (input_method == NULL)
    return false;

  xim = input_method->xim;
  spot.x = spot.y = -1;

  xim_styles = NULL;
  if (XGetIMValues (xim, XNQueryInputStyle, &xim_styles, NULL)
      || !xim_styles || !xim_styles->count_styles)
    {
      im_destroy ();
      return false;
    }

  const char *pet[] = { rs[Rs_preeditType], "OverTheSpot,OffTheSpot,Root,None" };

  for (int pi = 0; pi < 2; pi++)
    {
      p = pet[pi];

      if (!p)
        continue;

      s = rxvt_splitcommastring (p);

      for (i = found = 0; !found && s[i]; i++)
        {
          if (!strcmp (s[i], "OverTheSpot"))
            input_style = (XIMPreeditPosition | XIMStatusNothing);
          else if (!strcmp (s[i], "OffTheSpot"))
            input_style = (XIMPreeditArea | XIMStatusArea);
          else if (!strcmp (s[i], "Root"))
            input_style = (XIMPreeditNothing | XIMStatusNothing);
          else if (!strcmp (s[i], "None"))
            input_style = (XIMPreeditNone | XIMStatusNone);

          for (j = 0; j < xim_styles->count_styles; j++)
            if (input_style == xim_styles->supported_styles[j])
              {
                rxvt_freecommastring (s);

                found = 1;
                goto foundpet;
              }

        }

      rxvt_freecommastring (s);
    }

foundpet:

  XFree (xim_styles);

  if (!found)
    {
      im_destroy ();
      return false;
    }

  XFontSet fs = 0;
  XVaNestedList preedit_attr = 0, status_attr = 0;

  if (input_style & (XIMPreeditPosition | XIMPreeditArea))
    {
      // fake us a font-set, please
      char **missing_charset_list;
      int missing_charset_count;
      char *def_string;
      char pat[512];

      sprintf (pat,
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "*",
               TermWin.fheight,
               TermWin.fheight + 1, TermWin.fheight - 1,
               TermWin.fheight - 2, TermWin.fheight + 2);

      fs = XCreateFontSet (display->display, rs[Rs_imFont] ? rs[Rs_imFont] : pat,
                           &missing_charset_list, &missing_charset_count, &def_string);

      if (missing_charset_list)
        XFreeStringList (missing_charset_list);

      if (!fs)
        {
          input_style &= ~(XIMPreeditPosition | XIMPreeditArea);
          rxvt_warn ("unable to create fontset for input method, try \"-pt Root\". Continuing.\n");
        }
    }

  if (input_style & XIMPreeditPosition)
    {
      im_set_size (rect);
      im_set_position (spot);
      im_set_color (fg, bg);

      preedit_attr = XVaCreateNestedList (0,
                                          XNForeground, fg,
                                          XNBackground, bg,
                                          XNArea, &rect,
                                          XNSpotLocation, &spot,
                                          XNFontSet, fs,
                                          NULL);
    }
  else if (input_style & XIMPreeditArea)
    {
      im_set_color (fg, bg);

      /*
       * The necessary width of preedit area is unknown
       * until create input context.
       */
      needed_rect.width = 0;
      im_set_preedit_area (rect, status_rect, needed_rect);

      preedit_attr = XVaCreateNestedList (0,
                                          XNForeground, fg,
                                          XNBackground, bg,
                                          XNArea, &rect,
                                          XNFontSet, fs,
                                          NULL);
      status_attr = XVaCreateNestedList (0,
                                         XNForeground, fg,
                                         XNBackground, bg,
                                         XNArea, &status_rect,
                                         XNFontSet, fs,
                                         NULL);
    }

  Input_Context = XCreateIC (xim,
                             XNInputStyle, input_style,
                             XNClientWindow, TermWin.vt,
                             XNFocusWindow, TermWin.parent[0],
                             preedit_attr ? XNPreeditAttributes : NULL,
                             preedit_attr,
                             status_attr ? XNStatusAttributes : NULL,
                             status_attr, NULL);

  if (preedit_attr) XFree (preedit_attr);
  if (status_attr) XFree (status_attr);
  if (fs) XFreeFontSet (display->display, fs);

  if (Input_Context == NULL)
    {
      rxvt_warn ("failed to create input context, continuing without XIM.\n");
      im_destroy ();
      return false;
    }

  if (input_style & XIMPreeditArea)
    IMSetStatusPosition ();

  D_MAIN ((stderr, "rxvt_IM_get_IC () - successful connection"));
  return true;
}

void
rxvt_term::im_cb ()
{
  int i, found, had_im;
  const char *p;
  char **s;
  char buf[IMBUFSIZ];

  SET_R (this);

  im_destroy ();

  D_MAIN ((stderr, "rxvt_IMInstantiateCallback ()"));
  if (Input_Context)
    return;

#if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
  if (rs[Rs_imLocale])
    SET_LOCALE (rs[Rs_imLocale]);
#endif

  p = rs[Rs_inputMethod];
  if (p && *p)
    {
      bool found = false;

      s = rxvt_splitcommastring (p);

      for (i = 0; s[i]; i++)
        {
          if (*s[i])
            {
              strcpy (buf, "@im=");
              strncat (buf, s[i], IMBUFSIZ - 5);
              if (IM_get_IC (buf))
                {
                  found = true;
                  break;
                }
            }
        }

      rxvt_freecommastring (s);

      if (found)
        goto done;
    }

  /* try with XMODIFIERS env. var. */
  if (IM_get_IC (""))
    goto done;

  /* try with no modifiers base IF the user didn't specify an IM */
  if (IM_get_IC ("@im=none"))
    goto done;

done:
#if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
  if (rs[Rs_imLocale])
    SET_LOCALE (locale);
#endif
}

void
rxvt_term::IMSetStatusPosition ()
{
  XRectangle preedit_rect, status_rect, *needed_rect;
  XVaNestedList preedit_attr, status_attr;

  if (!Input_Context
      || !TermWin.focus
      || !(input_style & XIMPreeditArea)
      || !IMisRunning ())
    return;

  /* Getting the necessary width of preedit area */
  status_attr = XVaCreateNestedList (0, XNAreaNeeded, &needed_rect, NULL);
  XGetICValues (Input_Context, XNStatusAttributes, status_attr, NULL);
  XFree (status_attr);

  im_set_preedit_area (preedit_rect, status_rect, *needed_rect);
  XFree (needed_rect);

  preedit_attr = XVaCreateNestedList (0, XNArea, &preedit_rect, NULL);
  status_attr = XVaCreateNestedList (0, XNArea, &status_rect, NULL);

  XSetICValues (Input_Context,
                XNPreeditAttributes, preedit_attr,
                XNStatusAttributes, status_attr, NULL);

  XFree (preedit_attr);
  XFree (status_attr);
}
#endif                          /* USE_XIM */

/*----------------------------------------------------------------------*/
rxvt_t          rxvt_current_term;

/*----------------------- end-of-file (C source) -----------------------*/
