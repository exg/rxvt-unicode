/*--------------------------------*-C-*---------------------------------*
 * File:        main.c
 *----------------------------------------------------------------------*
 * $Id: main.C,v 1.5 2003/11/25 15:44:38 pcg Exp $
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
: pty_ev   (this, &rxvt_term::pty_cb),
#ifdef CURSOR_BLINK
  blink_ev (this, &rxvt_term::blink_cb),
#endif
  x_ev     (this, &rxvt_term::x_cb)
{
  cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;
}

rxvt_term::~rxvt_term ()
{
  delete PixColors;
}

/*----------------------------------------------------------------------*/
/* rxvt_init() */
/* LIBPROTO */
rxvt_t
rxvt_init(int argc, const char *const *argv)
{
  SET_R(new rxvt_term);
  dR;

  if (!R->init_vars () || !R->init (argc, argv))
    {
      delete R;
      return NULL;
    }

  return R;
}

bool
rxvt_term::init (int argc, const char *const *argv)
{
  dR;//TODO (scrollbar, setidle)

  /*
   * Save and then give up any super-user privileges
   * If we need privileges in any area then we must specifically request it.
   * We should only need to be root in these cases:
   *  1.  write utmp entries on some systems
   *  2.  chown tty on some systems
   */
  rxvt_privileges (this, SAVE);
  rxvt_privileges (this, IGNORE);

  rxvt_init_secondary (this);

  const char **cmd_argv = rxvt_init_resources (this, argc, argv);

#if (MENUBAR_MAX)
  rxvt_menubar_read (this, rs[Rs_menu]);
#endif
#ifdef HAVE_SCROLLBARS
  if (Options & Opt_scrollBar)
    scrollbar_setIdle ();    /* set existence for size calculations */
#endif

  rxvt_Create_Windows (this, argc, argv);

  rxvt_init_xlocale (this);

  rxvt_scr_reset (this);         /* initialize screen */
#ifdef RXVT_GRAPHICS
  rxvt_Gr_reset (this);          /* reset graphics */
#endif

#if 0
#ifdef DEBUG_X
  XSynchronize(Xdisplay, True);
  XSetErrorHandler((XErrorHandler) abort);
#else
  XSetErrorHandler((XErrorHandler) rxvt_xerror_handler);
#endif
#endif

#ifdef HAVE_SCROLLBARS
  if (Options & Opt_scrollBar)
    rxvt_Resize_scrollBar (this);      /* create and map scrollbar */
#endif
#if (MENUBAR_MAX)
  if (menubar_visible(r))
    XMapWindow (Xdisplay, menuBar.win);
#endif
#ifdef TRANSPARENT
  if (Options & Opt_transparent)
    {
      XSelectInput (Xdisplay, Xroot, PropertyChangeMask);
      rxvt_check_our_parents (this);
    }
#endif
  XMapWindow (Xdisplay, TermWin.vt);
  XMapWindow (Xdisplay, TermWin.parent[0]);

  rxvt_init_env (this);
  rxvt_init_command (this, cmd_argv);

  x_ev.start (Xfd, EVENT_READ);
  pty_ev.start (cmd_fd, EVENT_READ);

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
rxvt_Child_signal(int sig __attribute__ ((unused)))
{
    dR;
    int             pid, save_errno = errno;

    do {
        errno = 0;
    } while ((pid = waitpid(-1, NULL, WNOHANG)) == -1 && errno == EINTR);

    if (pid == R->cmd_pid)
        exit(EXIT_SUCCESS);

    errno = save_errno;
    signal(SIGCHLD, rxvt_Child_signal);
}

/*
 * Catch a fatal signal and tidy up before quitting
 */
/* EXTPROTO */
RETSIGTYPE
rxvt_Exit_signal(int sig)
{
    signal(sig, SIG_DFL);
#ifdef DEBUG_CMD
    rxvt_print_error("signal %d", sig);
#endif
    rxvt_clean_exit();
    kill(getpid(), sig);
}

/* ARGSUSED */
/* INTPROTO */
int
rxvt_xerror_handler(const Display * display
                    __attribute__ ((unused)), const XErrorEvent * event)
{
    dR;

    if (R->allowedxerror == -1) {
        R->allowedxerror = event->error_code;
        return 0;               /* ignored anyway */
    }
    rxvt_print_error("XError: Request: %d . %d, Error: %d",
                     event->request_code, event->minor_code,
                     event->error_code);
/* XXX: probably should call rxvt_clean_exit() bypassing X routines */
    exit(EXIT_FAILURE);
/* NOTREACHED */
}

/*----------------------------------------------------------------------*/
/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 * TODO: if debugging, this should free up any known resources if we can
 */
/* EXTPROTO */
void
rxvt_clean_exit(void)
{
    dR;

#ifdef DEBUG_SCREEN
    rxvt_scr_release(aR);
#endif
#ifndef NO_SETOWNER_TTYDEV
    rxvt_privileged_ttydev(aR_ RESTORE);
#endif
#ifdef UTMP_SUPPORT
    rxvt_privileged_utmp(aR_ RESTORE);
#endif
#ifdef USE_XIM
    if (R->Input_Context != NULL) {
        XDestroyIC(R->Input_Context);
        R->Input_Context = NULL;
    }
#endif
}

/* ------------------------------------------------------------------------- *
 *                         MEMORY ALLOCATION WRAPPERS                        *
 * ------------------------------------------------------------------------- */
/* EXTPROTO */
void           *
rxvt_malloc(size_t size)
{
    void           *p;

    p = malloc(size);
    if (p)
        return p;

    fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
    rxvt_clean_exit();
    exit(EXIT_FAILURE);
/* NOTREACHED */
}

/* EXTPROTO */
void           *
rxvt_calloc(size_t number, size_t size)
{
    void           *p;

    p = calloc(number, size);
    if (p)
        return p;

    fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
    rxvt_clean_exit();
    exit(EXIT_FAILURE);
/* NOTREACHED */
}

/* EXTPROTO */
void           *
rxvt_realloc(void *ptr, size_t size)
{
    void           *p;

    if (ptr)
        p = realloc(ptr, size);
    else
        p = malloc(size);
    if (p)
        return p;

    fprintf(stderr, APL_NAME ": memory allocation failure.  Aborting");
    rxvt_clean_exit();
    exit(EXIT_FAILURE);
/* NOTREACHED */
}

/* ------------------------------------------------------------------------- *
 *                            PRIVILEGED OPERATIONS                          *
 * ------------------------------------------------------------------------- */
/* take care of suid/sgid super-user (root) privileges */
/* INTPROTO */
void
rxvt_privileges(pR_ int mode)
{
#if ! defined(__CYGWIN32__)
# if !defined(HAVE_SETEUID) && defined(HAVE_SETREUID)
/* setreuid() is the poor man's setuid(), seteuid() */
#  define seteuid(a)    setreuid(-1, (a))
#  define setegid(a)    setregid(-1, (a))
#  define HAVE_SETEUID
# endif
# ifdef HAVE_SETEUID
    switch (mode) {
    case IGNORE:
    /*
     * change effective uid/gid - not real uid/gid - so we can switch
     * back to root later, as required
     */
        seteuid(getuid());
        setegid(getgid());
        break;
    case SAVE:
        R->euid = geteuid();
        R->egid = getegid();
        break;
    case RESTORE:
        seteuid(R->euid);
        setegid(R->egid);
        break;
    }
# else
    switch (mode) {
    case IGNORE:
        setuid(getuid());
        setgid(getgid());
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
/* EXTPROTO */
void
rxvt_privileged_utmp(pR_ char action)
{
    D_MAIN((stderr, "rxvt_privileged_utmp(%c); waiting for: %c (pid: %d)",
            action, R->next_utmp_action, getpid()));
    if (R->next_utmp_action != action || (action != SAVE && action != RESTORE)
        || (R->Options & Opt_utmpInhibit)
        || R->ttydev == NULL || *R->ttydev == '\0')
        return;

    rxvt_privileges(aR_ RESTORE);
    if (action == SAVE) {
        R->next_utmp_action = RESTORE;
        rxvt_makeutent(aR_ R->ttydev, R->rs[Rs_display_name]);
    } else {                    /* action == RESTORE */
        R->next_utmp_action = IGNORE;
        rxvt_cleanutent(aR);
    }
    rxvt_privileges(aR_ IGNORE);
}
#endif

#ifndef NO_SETOWNER_TTYDEV
/* EXTPROTO */
void
rxvt_privileged_ttydev(pR_ char action)
{
    D_MAIN((stderr,
            "rxvt_privileged_ttydev(aR_ %c); waiting for: %c (pid: %d)",
            action, R->next_tty_action, getpid()));
    if (R->next_tty_action != action || (action != SAVE && action != RESTORE)
        || R->ttydev == NULL || *R->ttydev == '\0')
        return;

    rxvt_privileges(aR_ RESTORE);

    if (action == SAVE) {
        R->next_tty_action = RESTORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
/* store original tty status for restoration rxvt_clean_exit() -- rgg 04/12/95 */
        if (lstat(R->ttydev, &R->ttyfd_stat) < 0)       /* you lose out */
            R->next_tty_action = IGNORE;
        else
# endif
        {
            chown(R->ttydev, getuid(), R->ttygid);      /* fail silently */
            chmod(R->ttydev, R->ttymode);
# ifdef HAVE_REVOKE
            revoke(R->ttydev);
# endif
        }
    } else {                    /* action == RESTORE */
        R->next_tty_action = IGNORE;
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
        chmod(R->ttydev, R->ttyfd_stat.st_mode);
        chown(R->ttydev, R->ttyfd_stat.st_uid, R->ttyfd_stat.st_gid);
# else
        chmod(R->ttydev,
              (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
        chown(R->ttydev, 0, 0);
# endif
    }

    rxvt_privileges(aR_ IGNORE);

# ifndef RESET_TTY_TO_COMMON_DEFAULTS
    D_MAIN((stderr, "%s \"%s\": mode %03o, uid %d, gid %d",
            action == RESTORE ? "Restoring" : (action ==
                                               SAVE ? "Saving" :
                                               "UNKNOWN ERROR for"), R->ttydev,
            R->ttyfd_stat.st_mode, R->ttyfd_stat.st_uid,
            R->ttyfd_stat.st_gid));
# endif
}
#endif

/*----------------------------------------------------------------------*/
/*
 * window size/position calculcations for XSizeHint and other storage.
 * if width/height are non-zero then override calculated width/height
 */
/* EXTPROTO */
void
rxvt_window_calc(pR_ unsigned int width, unsigned int height)
{
    short           recalc_x, recalc_y;
    int             x, y, sb_w, mb_h, flags;
    unsigned int    w, h;
    unsigned int    max_width, max_height;

    D_SIZE((stderr, "< Cols/Rows: %3d x %3d ; Width/Height: %4d x %4d",
            R->TermWin.ncol, R->TermWin.nrow, R->szHint.width,
            R->szHint.height));
    R->szHint.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
    R->szHint.win_gravity = NorthWestGravity;
/* R->szHint.min_aspect.x = R->szHint.min_aspect.y = 1; */

    recalc_x = recalc_y = 0;
    flags = 0;
    if (!R->parsed_geometry) {
        R->parsed_geometry = 1;
        if (R->rs[Rs_geometry])
            flags = XParseGeometry(R->rs[Rs_geometry], &x, &y, &w, &h);
        if (flags & WidthValue) {
            R->TermWin.ncol = BOUND_POSITIVE_INT16(w);
            R->szHint.flags |= USSize;
        }
        if (flags & HeightValue) {
            R->TermWin.nrow = BOUND_POSITIVE_INT16(h);
            R->szHint.flags |= USSize;
        }
        if (flags & XValue) {
            R->szHint.x = x;
            R->szHint.flags |= USPosition;
            if (flags & XNegative) {
                recalc_x = 1;
                R->szHint.win_gravity = NorthEastGravity;
            }
        }
        if (flags & YValue) {
            R->szHint.y = y;
            R->szHint.flags |= USPosition;
            if (flags & YNegative) {
                recalc_y = 1;
                if (R->szHint.win_gravity == NorthEastGravity)
                    R->szHint.win_gravity = SouthEastGravity;
                else
                    R->szHint.win_gravity = SouthWestGravity;
            }
        }
    }
/* TODO: BOUNDS */
    R->TermWin.width = R->TermWin.ncol * R->TermWin.fwidth;
    R->TermWin.height = R->TermWin.nrow * R->TermWin.fheight;
    max_width = MAX_COLS * R->TermWin.fwidth;
    max_height = MAX_ROWS * R->TermWin.fheight;

    R->szHint.base_width = R->szHint.base_height = 2 * R->TermWin.int_bwidth;

    sb_w = mb_h = 0;
    R->window_vt_x = R->window_vt_y = 0;
    if (scrollbar_visible(R)) {
        sb_w = scrollbar_TotalWidth();
        R->szHint.base_width += sb_w;
        if (!(R->Options & Opt_scrollBar_right))
            R->window_vt_x = sb_w;
    }
    if (menubar_visible(R)) {
        mb_h = menuBar_TotalHeight();
        R->szHint.base_height += mb_h;
        R->window_vt_y = mb_h;
    }
    R->szHint.width_inc = R->TermWin.fwidth;
    R->szHint.height_inc = R->TermWin.fheight;
    R->szHint.min_width = R->szHint.base_width + R->szHint.width_inc;
    R->szHint.min_height = R->szHint.base_height + R->szHint.height_inc;

    if (width && width - R->szHint.base_width < max_width) {
        R->szHint.width = width;
        R->TermWin.width = width - R->szHint.base_width;
    } else {
        MIN_IT(R->TermWin.width, max_width);
        R->szHint.width = R->szHint.base_width + R->TermWin.width;
    }
    if (height && height - R->szHint.base_height < max_height) {
        R->szHint.height = height;
        R->TermWin.height = height - R->szHint.base_height;
    } else {
        MIN_IT(R->TermWin.height, max_height);
        R->szHint.height = R->szHint.base_height + R->TermWin.height;
    }
    if (scrollbar_visible(R) && (R->Options & Opt_scrollBar_right))
        R->window_sb_x = R->szHint.width - sb_w;

    if (recalc_x)
        R->szHint.x += (DisplayWidth(R->Xdisplay, Xscreen)
                        - R->szHint.width - 2 * R->TermWin.ext_bwidth);
    if (recalc_y)
        R->szHint.y += (DisplayHeight(R->Xdisplay, Xscreen)
                        - R->szHint.height - 2 * R->TermWin.ext_bwidth);

    R->TermWin.ncol = R->TermWin.width / R->TermWin.fwidth;
    R->TermWin.nrow = R->TermWin.height / R->TermWin.fheight;
    D_SIZE((stderr, "> Cols/Rows: %3d x %3d ; Width/Height: %4d x %4d",
            R->TermWin.ncol, R->TermWin.nrow, R->szHint.width,
            R->szHint.height));
    return;
}

/*----------------------------------------------------------------------*/
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
/* EXTPROTO */
void
rxvt_tt_winsize(int fd, unsigned short col, unsigned short row, int pid)
{
    struct winsize  ws;

    if (fd < 0)
        return;
    ws.ws_col = col;
    ws.ws_row = row;
    ws.ws_xpixel = ws.ws_ypixel = 0;
#ifndef DEBUG_SIZE
    (void)ioctl(fd, TIOCSWINSZ, &ws);
#else
    if (ioctl(fd, TIOCSWINSZ, &ws) < 0) {
        D_SIZE((stderr, "Failed to send TIOCSWINSZ to fd %d", fd));
    }
# ifdef SIGWINCH
    else if (pid)               /* force through to the command */
        kill(pid, SIGWINCH);
# endif
#endif
}

/*----------------------------------------------------------------------*/
/* rxvt_change_font() - Switch to a new font */
/*
 * init = 1   - initialize
 *
 * fontname == FONT_UP  - switch to bigger font
 * fontname == FONT_DN  - switch to smaller font
 */
/* EXTPROTO */
void
rxvt_change_font(pR_ int init, const char *fontname)
{
}

/* INTPROTO */
void
rxvt_font_up_down(pR_ int n, int direction)
{
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* xterm sequences - title, iconName, color (exptl) */
/* EXTPROTO */
void
rxvt_set_title(pR_ const char *str)
{
#ifndef SMART_WINDOW_TITLE
    XStoreName(R->Xdisplay, R->TermWin.parent[0], str);
#else
    char           *name;

    if (XFetchName(R->Xdisplay, R->TermWin.parent[0], &name) == 0)
        name = NULL;
    if (name == NULL || STRCMP(name, str))
        XStoreName(R->Xdisplay, R->TermWin.parent[0], str);
    if (name)
        XFree(name);
#endif
}

/* EXTPROTO */
void
rxvt_set_iconName(pR_ const char *str)
{
#ifndef SMART_WINDOW_TITLE
    XSetIconName(R->Xdisplay, R->TermWin.parent[0], str);
#else
    char           *name;

    if (XGetIconName(R->Xdisplay, R->TermWin.parent[0], &name))
        name = NULL;
    if (name == NULL || STRCMP(name, str))
        XSetIconName(R->Xdisplay, R->TermWin.parent[0], str);
    if (name)
        XFree(name);
#endif
}

#ifdef XTERM_COLOR_CHANGE
/* EXTPROTO */
void
rxvt_set_window_color(pR_ int idx, const char *color)
{
    rxvt_color      xcol;
    int             i;

    if (color == NULL || *color == '\0')
        return;

/* handle color aliases */
    if (isdigit(*color)) {
        i = atoi(color);
        if (i >= 8 && i <= 15) {        /* bright colors */
            i -= 8;
# ifndef NO_BRIGHTCOLOR
            R->PixColors[idx] = R->PixColors[minBrightCOLOR + i];
            SET_PIXCOLOR(R, idx);
            goto Done;
# endif
        }
        if (i >= 0 && i <= 7) { /* normal colors */
            R->PixColors[idx] = R->PixColors[minCOLOR + i];
            SET_PIXCOLOR(R, idx);
            goto Done;
        }
    }
    if (!rxvt_rXParseAllocColor(aR_ & xcol, color))
        return;
/* XStoreColor (R->Xdisplay, XCMAP, XColor*); */

/*
 * FIXME: should free colors here, but no idea how to do it so instead,
 * so just keep gobbling up the colormap
 */
# if 0
    for (i = Color_Black; i <= Color_White; i++)
        if (R->PixColors[idx] == R->PixColors[i])
            break;
    if (i > Color_White) {
    /* fprintf (stderr, "XFreeColors: R->PixColors [%d] = %lu\n", idx, R->PixColors [idx]); */
        XFreeColors(R->Xdisplay, XCMAP, (R->PixColors + idx), 1,
                    DisplayPlanes(R->Xdisplay, Xscreen));
    }
# endif

    R->PixColors[idx] = xcol;
    SET_PIXCOLOR(R, idx);

/* XSetWindowAttributes attr; */
/* Cursor cursor; */
  Done:
    if (idx == Color_bg && !(R->Options & Opt_transparent))
        XSetWindowBackground(R->Xdisplay, R->TermWin.vt,
                             R->PixColors[Color_bg]);

/* handle Color_BD, scrollbar background, etc. */

    rxvt_set_colorfgbg(aR);
    rxvt_recolour_cursor(aR);
/* the only reasonable way to enforce a clean update */
    rxvt_scr_poweron(aR);
}

#else
# define rxvt_set_window_color(aR_ idx,color)   ((void)0)
#endif                          /* XTERM_COLOR_CHANGE */

/* EXTPROTO */
void
rxvt_recolour_cursor(pR)
{
    rxvt_color      xcol[2];

#if TODO
    xcol[0] = R->PixColors[Color_pointer];
    xcol[1] = R->PixColors[Color_bg];
    XQueryColors(R->Xdisplay, XCMAP, xcol, 2);
    XRecolorCursor(R->Xdisplay, R->TermWin_cursor, &(xcol[0]), &(xcol[1]));
#endif
}

/*----------------------------------------------------------------------*/
/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
/* INTPROTO */
void
rxvt_set_colorfgbg(pR)
{
    unsigned int    i;
    const char     *xpmb = "\0";
    char            fstr[sizeof("default") + 1], bstr[sizeof("default") + 1];

    R->env_colorfgbg =
        (char *)rxvt_malloc(sizeof("COLORFGBG=default;default;bg") + 1);
    STRCPY(fstr, "default");
    STRCPY(bstr, "default");
    for (i = Color_Black; i <= Color_White; i++)
        if (R->PixColors[Color_fg] == R->PixColors[i]) {
            sprintf(fstr, "%d", (i - Color_Black));
            break;
        }
    for (i = Color_Black; i <= Color_White; i++)
        if (R->PixColors[Color_bg] == R->PixColors[i]) {
            sprintf(bstr, "%d", (i - Color_Black));
#ifdef XPM_BACKGROUND
            xpmb = "default;";
#endif
            break;
        }
    sprintf(R->env_colorfgbg, "COLORFGBG=%s;%s%s", fstr, xpmb, bstr);
    putenv(R->env_colorfgbg);

#ifndef NO_BRIGHTCOLOR
    R->colorfgbg = DEFAULT_RSTYLE;
    for (i = minCOLOR; i <= maxCOLOR; i++) {
        if (R->PixColors[Color_fg] == R->PixColors[i])
            R->colorfgbg = SET_FGCOLOR(R->colorfgbg, i);
        if (R->PixColors[Color_bg] == R->PixColors[i])
            R->colorfgbg = SET_BGCOLOR(R->colorfgbg, i);
    }
#endif
}

/*----------------------------------------------------------------------*/
/*
 * Colour determination for low colour displays, routine from
 *     Hans de Goede <hans@highrise.nl>
 */

/* EXTPROTO */
int
rxvt_rXParseAllocColor(pR_ rxvt_color * screen_in_out, const char *colour)
{
    screen_in_out->set(aR_ colour);

    if (!screen_in_out->set(aR_ colour)) {
        rxvt_print_error("can't allocate colour: %s", colour);
        return false;
    }

    return true;
}

/* -------------------------------------------------------------------- *
 * -                         WINDOW RESIZING                          - *
 * -------------------------------------------------------------------- */
/* EXTPROTO */
void
rxvt_resize_all_windows(pR_ unsigned int width, unsigned int height,
                        int ignoreparent)
{
    int             fix_screen;

#ifdef SMART_RESIZE
    int             old_width = R->szHint.width, old_height = R->szHint.height;
#endif

    rxvt_window_calc(aR_ width, height);
    XSetWMNormalHints(R->Xdisplay, R->TermWin.parent[0], &R->szHint);
    if (!ignoreparent) {
#ifdef SMART_RESIZE
/*
 * resize by Marius Gedminas <marius.gedminas@uosis.mif.vu.lt>
 * reposition window on resize depending on placement on screen
 */
        int             x, y, x1, y1;
        int             dx, dy;
        unsigned int    unused_w1, unused_h1, unused_b1, unused_d1;
        Window          unused_cr;

        XTranslateCoordinates(R->Xdisplay, R->TermWin.parent[0], Xroot,
                              0, 0, &x, &y, &unused_cr);
        XGetGeometry(R->Xdisplay, R->TermWin.parent[0], &unused_cr, &x1, &y1,
                     &unused_w1, &unused_h1, &unused_b1, &unused_d1);
    /*
     * if Xroot isn't the parent window, a WM will probably have offset
     * our position for handles and decorations.  Counter it
     */
        if (x1 != x || y1 != y) {
            x -= x1;
            y -= y1;
        }

        x1 = (DisplayWidth(R->Xdisplay, Xscreen) - old_width) / 2;
        y1 = (DisplayHeight(R->Xdisplay, Xscreen) - old_height) / 2;
        dx = old_width - R->szHint.width;
        dy = old_height - R->szHint.height;

    /* Check position of the center of the window */
        if (x < x1)             /* left half */
            dx = 0;
        else if (x == x1)       /* exact center */
            dx /= 2;
        if (y < y1)             /* top half */
            dy = 0;
        else if (y == y1)       /* exact center */
            dy /= 2;

        XMoveResizeWindow(R->Xdisplay, R->TermWin.parent[0], x + dx, y + dy,
                          R->szHint.width, R->szHint.height);
#else
        XResizeWindow(R->Xdisplay, R->TermWin.parent[0], R->szHint.width,
                      R->szHint.height);
#endif
    }

    fix_screen = (R->TermWin.ncol != R->prev_ncol
                  || R->TermWin.nrow != R->prev_nrow);
    if (fix_screen || width != R->old_width || height != R->old_height) {
        if (scrollbar_visible(R)) {
            XMoveResizeWindow(R->Xdisplay, R->scrollBar.win, R->window_sb_x,
                              0, scrollbar_TotalWidth(), R->szHint.height);
            rxvt_Resize_scrollBar(aR);
        }
        if (menubar_visible(R))
            XMoveResizeWindow(R->Xdisplay, R->menuBar.win, R->window_vt_x,
                              0, TermWin_TotalWidth(), menuBar_TotalHeight());
        XMoveResizeWindow(R->Xdisplay, R->TermWin.vt, R->window_vt_x,
                          R->window_vt_y, TermWin_TotalWidth(),
                          TermWin_TotalHeight());
#ifdef RXVT_GRAPHICS
        if (R->old_height)
            rxvt_Gr_Resize(aR_ R->old_width - R->szHint.base_width,
                           R->old_height - R->szHint.base_height);
#endif
        rxvt_scr_clear(aR);
#ifdef XPM_BACKGROUND
        rxvt_resize_pixmap(aR);
#endif
    }

    if (fix_screen || R->old_height == 0) {
        int             curr_screen = -1;
        uint16_t        old_ncol = R->prev_ncol;

    /* scr_reset only works on the primary screen */
        if (R->old_height)      /* this is not the first time through */
            curr_screen = rxvt_scr_change_screen(aR_ PRIMARY);
        rxvt_scr_reset(aR);
        if (curr_screen >= 0) { /* this is not the first time through */
            rxvt_scr_change_screen(aR_ curr_screen);
            rxvt_selection_check(aR_(old_ncol != R->TermWin.ncol ? 4 : 0));
        }
    }

    R->old_width = R->szHint.width;
    R->old_height = R->szHint.height;

#ifdef USE_XIM
    rxvt_IMSetStatusPosition(aR);
#endif
}

/*
 * Set the width/height of the vt window in characters.  Units are pixels.
 * good for toggling 80/132 columns
 */
/* EXTPROTO */
void
rxvt_set_widthheight(pR_ unsigned int width, unsigned int height)
{
    XWindowAttributes wattr;

    if (width == 0 || height == 0) {
        XGetWindowAttributes(R->Xdisplay, Xroot, &wattr);
        if (width == 0)
            width = wattr.width - R->szHint.base_width;
        if (height == 0)
            height = wattr.height - R->szHint.base_height;
    }
    if (width != R->TermWin.width || height != R->TermWin.height) {
        width += R->szHint.base_width;
        height += R->szHint.base_height;
        rxvt_resize_all_windows(aR_ width, height, 0);
    }
}

/* -------------------------------------------------------------------- *
 * -                      X INPUT METHOD ROUTINES                     - *
 * -------------------------------------------------------------------- */
#ifdef USE_XIM
/* INTPROTO */
void
rxvt_setSize(pR_ XRectangle * size)
{
    size->x = R->TermWin.int_bwidth;
    size->y = R->TermWin.int_bwidth;
    size->width = Width2Pixel(R->TermWin.ncol);
    size->height = Height2Pixel(R->TermWin.nrow);
}

/* INTPROTO */
void
rxvt_setColor(pR_ unsigned long *fg, unsigned long *bg)
{
    *fg = R->PixColors[Color_fg];
    *bg = R->PixColors[Color_bg];
}

/* Checking whether input method is running. */
/* INTPROTO */
Bool
rxvt_IMisRunning(pR)
{
    char           *p;
    Atom            atom;
    Window          win;
    char            server[IMBUFSIZ];

/* get current locale modifier */
    if ((p = XSetLocaleModifiers(NULL)) != NULL) {
        STRCPY(server, "@server=");
        STRNCAT(server, &(p[4]), IMBUFSIZ - 9); /* skip "@im=" */
        if ((p = STRCHR(server + 1, '@')) != NULL)      /* first one only */
            *p = '\0';

        atom = XInternAtom(R->Xdisplay, server, False);
        win = XGetSelectionOwner(R->Xdisplay, atom);
        if (win != None)
            return True;
    }
    return False;
}

/* EXTPROTO */
void
rxvt_IMSendSpot(pR)
{
    XPoint          spot;
    XVaNestedList   preedit_attr;

    if (R->Input_Context == NULL
        || !R->TermWin.focus || !(R->input_style & XIMPreeditPosition)
        || !(R->event_type == KeyPress
             || R->event_type == Expose
             || R->event_type == NoExpose
             || R->event_type == SelectionNotify
             || R->event_type == ButtonRelease || R->event_type == FocusIn)
        || !rxvt_IMisRunning(aR))
        return;

    rxvt_setPosition(aR_ & spot);

    preedit_attr = XVaCreateNestedList(0, XNSpotLocation, &spot, NULL);
    XSetICValues(R->Input_Context, XNPreeditAttributes, preedit_attr, NULL);
    XFree(preedit_attr);
}

/* EXTPROTO */
void
rxvt_setTermFontSet(pR_ int idx)
{
    char           *string;
    long            length;
    int             success = 0;

    if (idx < 0 || idx >= MAX_NFONTS)
        return;
}

/* INTPROTO */
void
rxvt_setPreeditArea(pR_ XRectangle * preedit_rect, XRectangle * status_rect,
                    XRectangle * needed_rect)
{
    int             mbh, vtx = 0;

    if (scrollbar_visible(R) && !(R->Options & Opt_scrollBar_right))
        vtx = scrollbar_TotalWidth();
    mbh = menubar_visible(R) ? menuBar_TotalHeight() : 0;
    mbh -= R->TermWin.lineSpace;

    preedit_rect->x = needed_rect->width + vtx;
    preedit_rect->y = Height2Pixel(R->TermWin.nrow - 1) + mbh;

    preedit_rect->width = Width2Pixel(R->TermWin.ncol + 1) - needed_rect->width
        + vtx;
    preedit_rect->height = Height2Pixel(1);

    status_rect->x = vtx;
    status_rect->y = Height2Pixel(R->TermWin.nrow - 1) + mbh;

    status_rect->width = needed_rect->width ? needed_rect->width
        : Width2Pixel(R->TermWin.ncol + 1);
    status_rect->height = Height2Pixel(1);
}

/* ARGSUSED */
/* INTPROTO */
void
rxvt_IMDestroyCallback(XIM xim __attribute__ ((unused)), XPointer client_data
                       __attribute__ ((unused)), XPointer call_data
                       __attribute__ ((unused)))
{
    dR;

    R->Input_Context = NULL;
/* To avoid Segmentation Fault in C locale: Solaris only? */
    if (STRCMP(R->locale, "C"))
        XRegisterIMInstantiateCallback(R->Xdisplay, NULL, NULL, NULL,
                                       rxvt_IMInstantiateCallback, NULL);
}

/*
 * X manual pages and include files don't match on some systems:
 * some think this is an XIDProc and others an XIMProc so we can't
 * use the first argument - need to update this to be nice for
 * both types via some sort of configure detection
 */
/* ARGSUSED */
/* EXTPROTO */
void
rxvt_IMInstantiateCallback(Display * unused
                           __attribute__ ((unused)), XPointer client_data
                           __attribute__ ((unused)), XPointer call_data
                           __attribute__ ((unused)))
{
    dR;
    int             i, found, had_im;
    const char     *p;
    char          **s;
    char            buf[IMBUFSIZ];

    D_MAIN((stderr, "rxvt_IMInstantiateCallback()"));
    if (R->Input_Context)
        return;

    found = had_im = 0;
    p = R->rs[Rs_inputMethod];
    if (p && *p) {
        had_im = 1;
        s = rxvt_splitcommastring(p);
        for (i = 0; s[i]; i++) {
            if (*s[i]) {
                STRCPY(buf, "@im=");
                STRNCAT(buf, s[i], IMBUFSIZ - 5);
                if ((p = XSetLocaleModifiers(buf)) != NULL && *p
                    && (rxvt_IM_get_IC(aR) == True)) {
                    found = 1;
                    break;
                }
            }
        }
        for (i = 0; s[i]; i++)
            free(s[i]);
        free(s);
    }
    if (found)
        return;

/* try with XMODIFIERS env. var. */
    if ((p = XSetLocaleModifiers("")) != NULL && *p) {
        rxvt_IM_get_IC(aR);
        return;
    }

/* try with no modifiers base IF the user didn't specify an IM */
    if (!had_im && (p = XSetLocaleModifiers("@im=none")) != NULL && *p
        && rxvt_IM_get_IC(aR) == True)
        return;
}

/*
 * Try to open a XIM with the current modifiers, then see if we can
 * open a suitable preedit type
 */
/* INTPROTO */
Bool
rxvt_IM_get_IC(pR)
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
    XIMCallback     ximcallback;

    D_MAIN((stderr, "rxvt_IM_get_IC()"));
    xim = XOpenIM(R->Xdisplay, NULL, NULL, NULL);
    if (xim == NULL)
        return False;

    xim_styles = NULL;
    if (XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL)
        || !xim_styles || !xim_styles->count_styles) {
        XCloseIM(xim);
        return False;
    }

    p = R->rs[Rs_preeditType] ? R->rs[Rs_preeditType]
        : "OverTheSpot,OffTheSpot,Root";
    s = rxvt_splitcommastring(p);
    for (i = found = 0; !found && s[i]; i++) {
        if (!STRCMP(s[i], "OverTheSpot"))
            R->input_style = (XIMPreeditPosition | XIMStatusNothing);
        else if (!STRCMP(s[i], "OffTheSpot"))
            R->input_style = (XIMPreeditArea | XIMStatusArea);
        else if (!STRCMP(s[i], "Root"))
            R->input_style = (XIMPreeditNothing | XIMStatusNothing);

        for (j = 0; j < xim_styles->count_styles; j++)
            if (R->input_style == xim_styles->supported_styles[j]) {
                found = 1;
                break;
            }
    }
    for (i = 0; s[i]; i++)
        free(s[i]);
    free(s);
    XFree(xim_styles);

    if (!found) {
        XCloseIM(xim);
        return False;
    }

    ximcallback.callback = rxvt_IMDestroyCallback;

/* XXX: not sure why we need this (as well as IC one below) */
    XSetIMValues(xim, XNDestroyCallback, &ximcallback, NULL);

    preedit_attr = status_attr = NULL;

    if (R->input_style & XIMPreeditPosition) {
        rxvt_setSize(aR_ & rect);
        rxvt_setPosition(aR_ & spot);
        rxvt_setColor(aR_ & fg, &bg);

        preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
                                           XNSpotLocation, &spot,
                                           XNForeground, fg, XNBackground, bg,
                                       //XNFontSet, R->TermWin.fontset,
                                           NULL);
    } else if (R->input_style & XIMPreeditArea) {
        rxvt_setColor(aR_ & fg, &bg);

    /*
     * The necessary width of preedit area is unknown
     * until create input context.
     */
        needed_rect.width = 0;

        rxvt_setPreeditArea(aR_ & rect, &status_rect, &needed_rect);

        preedit_attr = XVaCreateNestedList(0, XNArea, &rect,
                                           XNForeground, fg, XNBackground, bg,
                                       //XNFontSet, R->TermWin.fontset,
                                           NULL);
        status_attr = XVaCreateNestedList(0, XNArea, &status_rect,
                                          XNForeground, fg, XNBackground, bg,
                                      //XNFontSet, R->TermWin.fontset,
                                          NULL);
    }
    R->Input_Context = XCreateIC(xim, XNInputStyle, R->input_style,
                                 XNClientWindow, R->TermWin.parent[0],
                                 XNFocusWindow, R->TermWin.parent[0],
                                 XNDestroyCallback, &ximcallback,
                                 preedit_attr ? XNPreeditAttributes : NULL,
                                 preedit_attr,
                                 status_attr ? XNStatusAttributes : NULL,
                                 status_attr, NULL);
    if (preedit_attr)
        XFree(preedit_attr);
    if (status_attr)
        XFree(status_attr);
    if (R->Input_Context == NULL) {
        rxvt_print_error("failed to create input context");
        XCloseIM(xim);
        return False;
    }
    if (R->input_style & XIMPreeditArea)
        rxvt_IMSetStatusPosition(aR);
    D_MAIN((stderr, "rxvt_IM_get_IC() - successful connection"));
    return True;
}

/* EXTPROTO */
void
rxvt_IMSetStatusPosition(pR)
{
    XRectangle      preedit_rect, status_rect, *needed_rect;
    XVaNestedList   preedit_attr, status_attr;

    if (R->Input_Context == NULL
        || !R->TermWin.focus || !(R->input_style & XIMPreeditArea)
        || !rxvt_IMisRunning(aR))
        return;

/* Getting the necessary width of preedit area */
    status_attr = XVaCreateNestedList(0, XNAreaNeeded, &needed_rect, NULL);
    XGetICValues(R->Input_Context, XNStatusAttributes, status_attr, NULL);
    XFree(status_attr);

    rxvt_setPreeditArea(aR_ & preedit_rect, &status_rect, needed_rect);

    preedit_attr = XVaCreateNestedList(0, XNArea, &preedit_rect, NULL);
    status_attr = XVaCreateNestedList(0, XNArea, &status_rect, NULL);

    XSetICValues(R->Input_Context,
                 XNPreeditAttributes, preedit_attr,
                 XNStatusAttributes, status_attr, NULL);

    XFree(preedit_attr);
    XFree(status_attr);
}
#endif                          /* USE_XIM */

/*----------------------------------------------------------------------*/
rxvt_t          rxvt_current_term;

/*----------------------- end-of-file (C source) -----------------------*/
