/*--------------------------------*-C-*---------------------------------*;
 * File:        main.c
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

#include <signal.h>

#ifdef TTY_GID_SUPPORT
# include <grp.h>
#endif

#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif

#include <cstring>

static char curlocale[128];

void
rxvt_set_locale (const char *locale)
{
  if (locale && STRNCMP (locale, curlocale, 128))
    {
      STRNCPY (curlocale, locale, 128);
      setlocale (LC_CTYPE, curlocale);
    }
}

void *
rxvt_term::operator new (size_t s)
{
  void *p = malloc (s);

  MEMSET (p, 0, s);
  return p;
}

void
rxvt_term::operator delete (void *p, size_t s)
{
  free (p);
}

rxvt_term::rxvt_term ()
    :
    rootwin_ev (this, &rxvt_term::rootwin_cb),
    termwin_ev (this, &rxvt_term::x_cb),
    vt_ev (this, &rxvt_term::x_cb),
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
#ifdef POINTER_BLANK
    pointer_ev (this, &rxvt_term::pointer_cb),
#endif
#ifdef USE_XIM
    im_ev (this, &rxvt_term::im_cb),
#endif
    check_ev (this, &rxvt_term::check_cb),
    destroy_ev (this, &rxvt_term::destroy_cb),
    pty_ev (this, &rxvt_term::pty_cb),
    incr_ev (this, &rxvt_term::incr_cb)
{
  cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;
}

rxvt_term::~rxvt_term ()
{
  if (cmd_fd >= 0)
    close (cmd_fd);

#ifndef NO_SETOWNER_TTYDEV
  privileged_ttydev (RESTORE);
#endif
#ifdef UTMP_SUPPORT
  privileged_utmp (RESTORE);
#endif

  delete TermWin.fontset;

  if (display)
    {
      if (TermWin.parent[0])
        XDestroyWindow (display->display, TermWin.parent[0]);
#if defined(MENUBAR) && (MENUBAR_MAX > 1)
      if (menuBar.win)
        XDestroyWindow (display->display, menuBar.win);
      delete menuBar.drawable;
#endif
    }

  // TODO: free pixcolours, colours should become part of rxvt_display

  delete PixColors;

  displays.put (display);

  scr_release ();

  free (env_windowid);
  free (env_display);
  free (env_term);
  free (env_colorfgbg);
  free (locale);
  free (codeset);

  delete envv;
  delete argv;
}

void
rxvt_term::destroy ()
{
  if (display)
    {
      rootwin_ev.stop (display);
      termwin_ev.stop (display);
      vt_ev.stop (display);
#ifdef USE_XIM
      im_destroy ();
      im_ev.stop (display);
#endif
#ifdef HAVE_SCROLLBARS
      scrollbar_ev.stop (display);
#endif
#ifdef MENUBAR
      menubar_ev.stop (display);
#endif
    }

  check_ev.stop ();
  pty_ev.stop ();
#ifdef CURSOR_BLINK
  cursor_blink_ev.stop ();
#endif
#ifdef TEXT_BLINK
  text_blink_ev.stop ();
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
/* rxvt_init () */
/* LIBPROTO */
rxvt_t
rxvt_init (int argc, const char *const *argv)
{
  SET_R (new rxvt_term);

  if (!GET_R->init_vars () || !GET_R->init (argc, argv))
    {
      delete GET_R;
      SET_R (0);
    }

  return GET_R;
}

static int (*old_xerror_handler) (Display *dpy, XErrorEvent *event);

void
rxvt_init_signals ()
{
  /* install exit handler for cleanup */
#if 0
#ifdef HAVE_ATEXIT
  atexit (rxvt_clean_exit);
#else
#endif
#endif

  struct sigaction sa;

  sigfillset (&sa.sa_mask);
  sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;
  sa.sa_handler = SIG_IGN;           sigaction (SIGHUP , &sa, 0);
  sa.sa_handler = SIG_IGN;           sigaction (SIGPIPE, &sa, 0);
  sa.sa_handler = rxvt_Exit_signal;  sigaction (SIGINT , &sa, 0);
  sa.sa_handler = rxvt_Exit_signal;  sigaction (SIGQUIT, &sa, 0);
  sa.sa_handler = rxvt_Exit_signal;  sigaction (SIGTERM, &sa, 0);
  sa.sa_handler = rxvt_Child_signal; sigaction (SIGCHLD, &sa, 0);

  /* need to trap SIGURG for SVR4 (Unixware) rlogin */
  /* signal (SIGURG, SIG_DFL); */

  old_xerror_handler = XSetErrorHandler ((XErrorHandler) rxvt_xerror_handler);
  //XSetIOErrorHandler ((XErrorHandler) rxvt_xioerror_handler);
}

bool
rxvt_term::init (int argc, const char *const *argv)
{
  /*
   * Save and then give up any super-user privileges
   * If we need privileges in any area then we must specifically request it.
   * We should only need to be root in these cases:
   *  1.  write utmp entries on some systems
   *  2.  chown tty on some systems
   */
  privileges (SAVE);
  privileges (IGNORE);

  init_secondary ();

  const char **cmd_argv = init_resources (argc, argv);

  set_locale ("");

#if MENUBAR_MAX
  menubar_read (rs[Rs_menu]);
#endif
#ifdef HAVE_SCROLLBARS
  if (Options & Opt_scrollBar)
    scrollBar.setIdle ();    /* set existence for size calculations */
#endif

  create_windows (argc, argv);

  init_xlocale ();

  scr_reset ();         /* initialize screen */

#if 0
  XSynchronize (display->display, True);
#endif

#ifdef HAVE_SCROLLBARS
  if (Options & Opt_scrollBar)
    resize_scrollbar ();      /* create and map scrollbar */
#endif
#if (MENUBAR_MAX)
  if (menubar_visible ())
    XMapWindow (display->display, menuBar.win);
#endif
#ifdef TRANSPARENT
  if (Options & Opt_transparent)
    {
      XSelectInput (display->display, display->root, PropertyChangeMask);
      check_our_parents ();
    }
#endif

  rootwin_ev.start (display, display->root);

  XMapWindow (display->display, TermWin.vt);
  XMapWindow (display->display, TermWin.parent[0]);

  init_command (cmd_argv);

  pty_ev.start (cmd_fd, EVENT_READ);

  check_ev.start ();

  return true;
}

/* ------------------------------------------------------------------------- *
 *                       SIGNAL HANDLING & EXIT HANDLER                      *
 * ------------------------------------------------------------------------- */
/*
 * Catch a SIGCHLD signal and exit if the direct child has died
 */
/* ARGSUSED */
/* EXTPROTO */
RETSIGTYPE
rxvt_Child_signal (int sig __attribute__ ((unused)))
{
  int pid, save_errno = errno;
  while ((pid = waitpid (-1, NULL, WNOHANG)) == -1 && errno == EINTR)
    ;
  errno = save_errno;

#if 0
  if (pid == cmd_pid)
    exit (EXIT_SUCCESS);
#endif
}

/*
 * Catch a fatal signal and tidy up before quitting
 */
/* EXTPROTO */
RETSIGTYPE
rxvt_Exit_signal (int sig)
{
  signal (sig, SIG_DFL);
#ifdef DEBUG_CMD
  rxvt_print_error ("signal %d", sig);
#endif
  rxvt_clean_exit ();
  kill (getpid (), sig);
}

/* INTPROTO */
int
rxvt_xerror_handler (Display *display, XErrorEvent *event)
{
  if (GET_R->allowedxerror == -1)
    GET_R->allowedxerror = event->error_code;
  else
    {
      old_xerror_handler (display, event);
      GET_R->destroy ();
    }

  return 0;
}

/*----------------------------------------------------------------------*/
/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 * TODO: if debugging, this should free up any known resources if we can
 */
/* EXTPROTO */
void
rxvt_clean_exit ()
{
  // TODO: rxvtd should clean up all ressources
  if (GET_R)
    GET_R->destroy ();
}

/* ------------------------------------------------------------------------- *
 *                         MEMORY ALLOCATION WRAPPERS                        *
 * ------------------------------------------------------------------------- */
/* EXTPROTO */
void           *
rxvt_malloc (size_t size)
{
  void           *p;

  p = malloc (size);
  if (p)
    return p;

  fprintf (stderr, APL_NAME ": memory allocation failure.  Aborting");
  rxvt_clean_exit ();
  exit (EXIT_FAILURE);
  /* NOTREACHED */
}

/* EXTPROTO */
void           *
rxvt_calloc (size_t number, size_t size)
{
  void           *p;

  p = calloc (number, size);
  if (p)
    return p;

  fprintf (stderr, APL_NAME ": memory allocation failure.  Aborting");
  rxvt_clean_exit ();
  exit (EXIT_FAILURE);
  /* NOTREACHED */
}

/* EXTPROTO */
void           *
rxvt_realloc (void *ptr, size_t size)
{
  void           *p;

  if (ptr)
    p = realloc (ptr, size);
  else
    p = malloc (size);
  if (p)
    return p;

  fprintf (stderr, APL_NAME ": memory allocation failure.  Aborting");
  rxvt_clean_exit ();
  exit (EXIT_FAILURE);
  /* NOTREACHED */
}

/* ------------------------------------------------------------------------- *
 *                            PRIVILEGED OPERATIONS                          *
 * ------------------------------------------------------------------------- */
/* take care of suid/sgid super-user (root) privileges */
void
rxvt_term::privileges (int mode)
{
#if ! defined(__CYGWIN32__)
# if !defined(HAVE_SETEUID) && defined(HAVE_SETREUID)
  /* setreuid () is the poor man's setuid (), seteuid () */
#  define seteuid(a)    setreuid(-1, (a))
#  define setegid(a)    setregid(-1, (a))
#  define HAVE_SETEUID
# endif
# ifdef HAVE_SETEUID
  switch (mode)
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
  switch (mode)
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
rxvt_term::privileged_utmp (char action)
{
  D_MAIN ((stderr, "rxvt_privileged_utmp (%c); waiting for: %c (pid: %d)",
          action, next_utmp_action, getpid ()));
  if (next_utmp_action != action || (action != SAVE && action != RESTORE)
      || (Options & Opt_utmpInhibit)
      || ttydev == NULL || *ttydev == '\0')
    return;

  privileges (RESTORE);
  if (action == SAVE)
    {
      next_utmp_action = RESTORE;
      makeutent (ttydev, rs[Rs_display_name]);
    }
  else
    {                    /* action == RESTORE */
      next_utmp_action = IGNORE;
      cleanutent ();
    }
  privileges (IGNORE);
}
#endif

#ifndef NO_SETOWNER_TTYDEV
void
rxvt_term::privileged_ttydev (char action)
{
  D_MAIN ((stderr,
          "privileged_ttydev (%c); waiting for: %c (pid: %d)",
          action, next_tty_action, getpid ()));
  if (next_tty_action != action || (action != SAVE && action != RESTORE)
      || ttydev == NULL || *ttydev == '\0')
    return;

  privileges (RESTORE);

  if (action == SAVE)
    {
      next_tty_action = RESTORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
      /* store original tty status for restoration rxvt_clean_exit () -- rgg 04/12/95 */
      if (lstat (ttydev, &ttyfd_stat) < 0)       /* you lose out */
        next_tty_action = IGNORE;
      else
# endif

        {
          chown (ttydev, getuid (), ttygid);      /* fail silently */
          chmod (ttydev, ttymode);
# ifdef HAVE_REVOKE
          revoke (ttydev);
# endif

        }
    }
  else
    {                    /* action == RESTORE */
      next_tty_action = IGNORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
      chmod (ttydev, ttyfd_stat.st_mode);
      chown (ttydev, ttyfd_stat.st_uid, ttyfd_stat.st_gid);
# else
      chmod (ttydev,
            (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
      chown (ttydev, 0, 0);
# endif

    }

  privileges (IGNORE);

# ifndef RESET_TTY_TO_COMMON_DEFAULTS
  D_MAIN ((stderr, "%s \"%s\": mode %03o, uid %d, gid %d",
          action == RESTORE ? "Restoring" : (action ==
                                             SAVE ? "Saving" :
                                             "UNKNOWN ERROR for"), ttydev,
          ttyfd_stat.st_mode, ttyfd_stat.st_uid,
          ttyfd_stat.st_gid));
# endif
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
  short           recalc_x, recalc_y;
  int             x, y, sb_w, mb_h, flags;
  unsigned int    w, h;
  unsigned int    max_width, max_height;

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
  window_vt_x = window_vt_y = 0;
  if (scrollbar_visible ())
    {
      sb_w = scrollbar_TotalWidth ();
      szHint.base_width += sb_w;
      if (! (Options & Opt_scrollBar_right))
        window_vt_x = sb_w;
    }
  if (menubar_visible ())
    {
      mb_h = menuBar_TotalHeight ();
      szHint.base_height += mb_h;
      window_vt_y = mb_h;
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
  if (scrollbar_visible () && (Options & Opt_scrollBar_right))
    window_sb_x = szHint.width - sb_w;

  if (recalc_x)
    szHint.x += (DisplayWidth (display->display, DefaultScreen (display->display))
                 - szHint.width - 2 * TermWin.ext_bwidth);
  if (recalc_y)
    szHint.y += (DisplayHeight (display->display, DefaultScreen (display->display))
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
  struct winsize ws;

  if (cmd_fd < 0)
    return;

  ws.ws_col = TermWin.ncol;
  ws.ws_row = TermWin.nrow;
  ws.ws_xpixel = ws.ws_ypixel = 0;
#ifndef DEBUG_SIZE
  (void)ioctl (cmd_fd, TIOCSWINSZ, &ws);
#else
  if (ioctl (cmd_fd, TIOCSWINSZ, &ws) < 0)
    D_SIZE ((stderr, "Failed to send TIOCSWINSZ to fd %d", fd));
# ifdef SIGWINCH
  else if (cmd_pid)               /* force through to the command */
    kill (cmd_pid, SIGWINCH);
# endif
#endif
}

/*----------------------------------------------------------------------*/
/* rxvt_change_font () - Switch to a new font */
/*
 * init = 1   - initialize
 *
 * fontname == FONT_UP  - switch to bigger font
 * fontname == FONT_DN  - switch to smaller font
 */
bool
rxvt_term::change_font (const char *fontname)
{
  if (fontname == FONT_UP)
    {
      // TODO
    }
  else if (fontname == FONT_DN)
    {
      // TODO
    }
  else
    {
      rxvt_fontset *fs = new rxvt_fontset (this);

      if (fs && fs->populate (fontname))
        {
          delete TermWin.fontset;
          TermWin.fontset = fs;
          TermWin.fwidth  = fs->base_font ()->width;
          TermWin.fheight = fs->base_font ()->height;
          TermWin.fbase   = fs->base_font ()->ascent;

          // TODO: screen needs to be told about new fonts
          
          if (TermWin.parent[0])
            {
              resize_all_windows (0, 0, 0);
              scr_remap_chars ();
              scr_touch (true);
            }   

          return true;
        }
    }

  return false;
}

bool
rxvt_term::font_up_down (int n, int direction)
{
  return false;
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* xterm sequences - title, iconName, color (exptl) */
void
rxvt_term::set_title (const char *str)
{
#ifndef SMART_WINDOW_TITLE
  XStoreName (display->display, TermWin.parent[0], str);
#else
  char           *name;

  if (XFetchName (display->display, TermWin.parent[0], &name) == 0)
    name = NULL;
  if (name == NULL || STRCMP (name, str))
    XStoreName (display->display, TermWin.parent[0], str);
  if (name)
    XFree (name);
#endif
}

void
rxvt_term::set_iconName (const char *str)
{
#ifndef SMART_WINDOW_TITLE
  XSetIconName (display->display, TermWin.parent[0], str);
#else
  char           *name;

  if (XGetIconName (display->display, TermWin.parent[0], &name))
    name = NULL;
  if (name == NULL || STRCMP (name, str))
    XSetIconName (display->display, TermWin.parent[0], str);
  if (name)
    XFree (name);
#endif
}

#ifdef XTERM_COLOR_CHANGE
void
rxvt_term::set_window_color (int idx, const char *color)
{
  rxvt_color      xcol;
  int             i;

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
          PixColors[idx] = PixColors[minBrightCOLOR + i];
          SET_PIXCOLOR (idx);
          goto Done;
# endif

        }
      if (i >= 0 && i <= 7)
        { /* normal colors */
          PixColors[idx] = PixColors[minCOLOR + i];
          SET_PIXCOLOR (idx);
          goto Done;
        }
    }
  if (!rXParseAllocColor (& xcol, color))
    return;
  /* XStoreColor (display->display, XCMAP, XColor*); */

  /*
   * FIXME: should free colors here, but no idea how to do it so instead,
   * so just keep gobbling up the colormap
   */
# if 0
  for (i = Color_Black; i <= Color_White; i++)
    if (PixColors[idx] == PixColors[i])
      break;
  if (i > Color_White)
    {
      /* fprintf (stderr, "XFreeColors: PixColors [%d] = %lu\n", idx, PixColors [idx]); */
      XFreeColors (display->display, XCMAP, (PixColors + idx), 1,
                  DisplayPlanes (display->display, display->screen));
    }
# endif

  PixColors[idx] = xcol;
  SET_PIXCOLOR (idx);

  /* XSetWindowAttributes attr; */
  /* Cursor cursor; */
Done:
  if (idx == Color_bg && ! (Options & Opt_transparent))
    XSetWindowBackground (display->display, TermWin.vt,
                         PixColors[Color_bg]);

  /* handle Color_BD, scrollbar background, etc. */

  set_colorfgbg ();
  recolour_cursor ();
  /* the only reasonable way to enforce a clean update */
  scr_poweron ();
}

#else
# define set_window_color (idx,color)   ((void)0)
#endif                          /* XTERM_COLOR_CHANGE */

void
rxvt_term::recolour_cursor ()
{
#if TODO
  rxvt_color xcol[2];

  xcol[0] = PixColors[Color_pointer];
  xcol[1] = PixColors[Color_bg];
  XQueryColors (display->display, XCMAP, xcol, 2);
  XRecolorCursor (display->display, TermWin_cursor, & (xcol[0]), & (xcol[1]));
#endif
}

/*----------------------------------------------------------------------*/
/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
void
rxvt_term::set_colorfgbg ()
{
  unsigned int    i;
  const char     *xpmb = "\0";
  char            fstr[sizeof ("default") + 1], bstr[sizeof ("default") + 1];

  env_colorfgbg =
    (char *)rxvt_malloc (sizeof ("COLORFGBG=default;default;bg") + 1);
  STRCPY (fstr, "default");
  STRCPY (bstr, "default");
  for (i = Color_Black; i <= Color_White; i++)
    if (PixColors[Color_fg] == PixColors[i])
      {
        sprintf (fstr, "%d", (i - Color_Black));
        break;
      }
  for (i = Color_Black; i <= Color_White; i++)
    if (PixColors[Color_bg] == PixColors[i])
      {
        sprintf (bstr, "%d", (i - Color_Black));
#ifdef XPM_BACKGROUND
        xpmb = "default;";
#endif
        break;
      }

  sprintf (env_colorfgbg, "COLORFGBG=%s;%s%s", fstr, xpmb, bstr);

#ifndef NO_BRIGHTCOLOR
  colorfgbg = DEFAULT_RSTYLE;
  for (i = minCOLOR; i <= maxCOLOR; i++)
    {
      if (PixColors[Color_fg] == PixColors[i])
        colorfgbg = SET_FGCOLOR (colorfgbg, i);
      if (PixColors[Color_bg] == PixColors[i])
        colorfgbg = SET_BGCOLOR (colorfgbg, i);
    }
#endif
}

/*----------------------------------------------------------------------*/
/*
 * Colour determination for low colour displays, routine from
 *     Hans de Goede <hans@highrise.nl>
 */

int
rxvt_term::rXParseAllocColor (rxvt_color *screen_in_out, const char *colour)
{
  if (!screen_in_out->set (display, colour))
    {
      rxvt_print_error ("can't allocate colour: %s", colour);
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

  window_calc (width, height);
  XSetWMNormalHints (display->display, TermWin.parent[0], &szHint);
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

      XTranslateCoordinates (display->display, TermWin.parent[0], display->root,
                             0, 0, &x, &y, &unused_cr);
      XGetGeometry (display->display, TermWin.parent[0], &unused_cr, &x1, &y1,
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

      x1 = (DisplayWidth (display->display, display->screen) - old_width) / 2;
      y1 = (DisplayHeight (display->display, display->screen) - old_height) / 2;
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

      XMoveResizeWindow (display->display, TermWin.parent[0], x + dx, y + dy,
                         szHint.width, szHint.height);
#else
      XResizeWindow (display->display, TermWin.parent[0], szHint.width,
                     szHint.height);
#endif

    }

  fix_screen = (TermWin.ncol != prev_ncol
                || TermWin.nrow != prev_nrow);

  if (fix_screen || width != old_width || height != old_height)
    {
      if (scrollbar_visible ())
        {
          XMoveResizeWindow (display->display, scrollBar.win, window_sb_x,
                             0, scrollbar_TotalWidth (), szHint.height);
          resize_scrollbar ();
        }

      if (menubar_visible ())
        XMoveResizeWindow (display->display, menuBar.win, window_vt_x,
                           0, TermWin_TotalWidth (), menuBar_TotalHeight ());

      XMoveResizeWindow (display->display, TermWin.vt, window_vt_x,
                         window_vt_y, TermWin_TotalWidth (),
                         TermWin_TotalHeight ());
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
rxvt_term::im_set_size (XRectangle *size)
{
  size->x = TermWin.int_bwidth;
  size->y = TermWin.int_bwidth;
  size->width = Width2Pixel (TermWin.ncol);
  size->height = Height2Pixel (TermWin.nrow);
}

void
rxvt_term::im_set_color (unsigned long *fg, unsigned long *bg)
{
  *fg = PixColors[Color_fg];
  *bg = PixColors[Color_bg];
}

/* Checking whether input method is running. */
bool
rxvt_term::IMisRunning ()
{
  char           *p;
  Atom            atom;
  Window          win;
  char            server[IMBUFSIZ];

  /* get current locale modifier */
  if ((p = XSetLocaleModifiers (NULL)) != NULL)
    {
      STRCPY (server, "@server=");
      STRNCAT (server, & (p[4]), IMBUFSIZ - 9); /* skip "@im=" */
      if ((p = STRCHR (server + 1, '@')) != NULL)      /* first one only */
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
  XPoint          spot;
  XVaNestedList   preedit_attr;

  if (Input_Context == NULL
      || !TermWin.focus || ! (input_style & XIMPreeditPosition)
      || ! (event_type == KeyPress
           || event_type == Expose
           || event_type == NoExpose
           || event_type == SelectionNotify
           || event_type == ButtonRelease || event_type == FocusIn)
      || !IMisRunning ())
    return;

  im_set_position (&spot);

  preedit_attr = XVaCreateNestedList (0, XNSpotLocation, &spot, NULL);
  XSetICValues (Input_Context, XNPreeditAttributes, preedit_attr, NULL);
  XFree (preedit_attr);
}

void
rxvt_term::im_set_preedit_area (XRectangle * preedit_rect, XRectangle * status_rect,
                                XRectangle * needed_rect)
{
  int mbh, vtx = 0;

  if (scrollbar_visible () && ! (Options & Opt_scrollBar_right))
    vtx = scrollbar_TotalWidth ();

  mbh = menubar_visible () ? menuBar_TotalHeight () : 0;
  mbh -= TermWin.lineSpace;

  preedit_rect->x = needed_rect->width + vtx;
  preedit_rect->y = Height2Pixel (TermWin.nrow - 1) + mbh;

  preedit_rect->width = Width2Pixel (TermWin.ncol + 1) - needed_rect->width + vtx;
  preedit_rect->height = Height2Pixel (1);

  status_rect->x = vtx;
  status_rect->y = Height2Pixel (TermWin.nrow - 1) + mbh;

  status_rect->width = needed_rect->width ? needed_rect->width : Width2Pixel (TermWin.ncol + 1);
  status_rect->height = Height2Pixel (1);
}

void
rxvt_term::im_destroy ()
{
  if (Input_Context)
    {
      XDestroyIC (Input_Context);
      Input_Context = NULL;
    }

  if (input_method)
    {
      display->put_xim (input_method);
      input_method = 0;
    }
}

/*
 * Try to open a XIM with the current modifiers, then see if we can
 * open a suitable preedit type
 */
bool
rxvt_term::IM_get_IC (const char *modifiers)
{
  int             i, j, found;
  XIM             xim;
  XPoint          spot;
  XRectangle      rect, status_rect, needed_rect;
  unsigned long   fg, bg;
  const char     *p;
  char          **s;
  XIMStyles      *xim_styles;
  XVaNestedList   preedit_attr, status_attr;

  if (! ((p = XSetLocaleModifiers (modifiers)) && *p))
    return false;

  D_MAIN ((stderr, "rxvt_IM_get_IC ()"));
  input_method = display->get_xim (locale, modifiers);
  if (input_method == NULL)
    return false;

  xim = input_method->xim;

  xim_styles = NULL;
  if (XGetIMValues (xim, XNQueryInputStyle, &xim_styles, NULL)
      || !xim_styles || !xim_styles->count_styles)
    {
      display->put_xim (input_method);
      return false;
    }

  p = rs[Rs_preeditType] ? rs[Rs_preeditType] : "OverTheSpot,OffTheSpot,Root";
  s = rxvt_splitcommastring (p);
  for (i = found = 0; !found && s[i]; i++)
    {
      if (!STRCMP (s[i], "OverTheSpot"))
        input_style = (XIMPreeditPosition | XIMStatusNothing);
      else if (!STRCMP (s[i], "OffTheSpot"))
        input_style = (XIMPreeditArea | XIMStatusArea);
      else if (!STRCMP (s[i], "Root"))
        input_style = (XIMPreeditNothing | XIMStatusNothing);

      for (j = 0; j < xim_styles->count_styles; j++)
        if (input_style == xim_styles->supported_styles[j])
          {
            found = 1;
            break;
          }
    }

  for (i = 0; s[i]; i++)
    free (s[i]);

  free (s);
  XFree (xim_styles);

  if (!found)
    {
      display->put_xim (input_method);
      return false;
    }

  preedit_attr = status_attr = NULL;

  if (input_style & XIMPreeditPosition)
    {
      im_set_size (&rect);
      im_set_position (&spot);
      im_set_color (&fg, &bg);

      preedit_attr = XVaCreateNestedList (0, XNArea, &rect,
                                         XNSpotLocation, &spot,
                                         XNForeground, fg, XNBackground, bg,
                                         //XNFontSet, TermWin.fontset,
                                         NULL);
    }
  else if (input_style & XIMPreeditArea)
    {
      im_set_color (&fg, &bg);

      /*
       * The necessary width of preedit area is unknown
       * until create input context.
       */
      needed_rect.width = 0;

      im_set_preedit_area (&rect, &status_rect, &needed_rect);

      preedit_attr = XVaCreateNestedList (0, XNArea, &rect,
                                         XNForeground, fg, XNBackground, bg,
                                         //XNFontSet, TermWin.fontset,
                                         NULL);
      status_attr = XVaCreateNestedList (0, XNArea, &status_rect,
                                        XNForeground, fg, XNBackground, bg,
                                        //XNFontSet, TermWin.fontset,
                                        NULL);
    }

  Input_Context = XCreateIC (xim, XNInputStyle, input_style,
                            XNClientWindow, TermWin.parent[0],
                            XNFocusWindow, TermWin.parent[0],
                            preedit_attr ? XNPreeditAttributes : NULL,
                            preedit_attr,
                            status_attr ? XNStatusAttributes : NULL,
                            status_attr, NULL);
  if (preedit_attr) XFree (preedit_attr);
  if (status_attr) XFree (status_attr);

  if (Input_Context == NULL)
    {
      rxvt_print_error ("failed to create input context");
      display->put_xim (input_method);
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
              STRCPY (buf, "@im=");
              STRNCAT (buf, s[i], IMBUFSIZ - 5);
              if (IM_get_IC (buf))
                {
                  found = true;
                  break;
                }
            }
        }
      for (i = 0; s[i]; i++)
        free (s[i]);
      free (s);

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
  XRectangle      preedit_rect, status_rect, *needed_rect;
  XVaNestedList   preedit_attr, status_attr;

  if (Input_Context == NULL
      || !TermWin.focus || ! (input_style & XIMPreeditArea)
      || !IMisRunning ())
    return;

  /* Getting the necessary width of preedit area */
  status_attr = XVaCreateNestedList (0, XNAreaNeeded, &needed_rect, NULL);
  XGetICValues (Input_Context, XNStatusAttributes, status_attr, NULL);
  XFree (status_attr);

  im_set_preedit_area (&preedit_rect, &status_rect, needed_rect);

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
