/*--------------------------------*-C-*---------------------------------*
 * File:        init.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *                              - original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *                              - extensive modifications
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 *                              - extensive modifications
 * Copyright (c) 1999      D J Hawkey Jr <hawkeyd@visi.com>
 *                              - QNX support
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
/*
 * Initialisation routines.
 */

#include "../config.h"          /* NECESSARY */
#include "rxvt.h"               /* NECESSARY */
#include "init.h"

#include <signal.h>

const char *const def_colorName[] =
  {
    COLOR_FOREGROUND,
    COLOR_BACKGROUND,
    /* low-intensity colors */
    "Black",                    /* 0: black             (#000000) */
#ifndef NO_BRIGHTCOLOR
    "Red3",                     /* 1: red               (#CD0000) */
    "Green3",                   /* 2: green             (#00CD00) */
    "Yellow3",                  /* 3: yellow            (#CDCD00) */
    "Blue3",                    /* 4: blue              (#0000CD) */
    "Magenta3",                 /* 5: magenta           (#CD00CD) */
    "Cyan3",                    /* 6: cyan              (#00CDCD) */
# ifdef XTERM_COLORS
    "Grey90",                   /* 7: white             (#E5E5E5) */
# else
    "AntiqueWhite",             /* 7: white             (#FAEBD7) */
# endif
    /* high-intensity colors */
# ifdef XTERM_COLORS
    "Grey30",                   /* 8: bright black      (#4D4D4D) */
# else
    "Grey25",                   /* 8: bright black      (#404040) */
# endif
#endif                          /* NO_BRIGHTCOLOR */
    "Red",                      /* 1/9: bright red      (#FF0000) */
    "Green",                    /* 2/10: bright green   (#00FF00) */
    "Yellow",                   /* 3/11: bright yellow  (#FFFF00) */
    "Blue",                     /* 4/12: bright blue    (#0000FF) */
    "Magenta",                  /* 5/13: bright magenta (#FF00FF) */
    "Cyan",                     /* 6/14: bright cyan    (#00FFFF) */
    "White",                    /* 7/15: bright white   (#FFFFFF) */
#ifdef TTY_256COLOR
    "rgb:00/00/00",             /* default 16-255 color table     */
    "rgb:00/00/2a",
    "rgb:00/00/55",
    "rgb:00/00/7f",
    "rgb:00/00/aa",
    "rgb:00/00/d4",
    "rgb:00/2a/00",
    "rgb:00/2a/2a",
    "rgb:00/2a/55",
    "rgb:00/2a/7f",
    "rgb:00/2a/aa",
    "rgb:00/2a/d4",
    "rgb:00/55/00",
    "rgb:00/55/2a",
    "rgb:00/55/55",
    "rgb:00/55/7f",
    "rgb:00/55/aa",
    "rgb:00/55/d4",
    "rgb:00/7f/00",
    "rgb:00/7f/2a",
    "rgb:00/7f/55",
    "rgb:00/7f/7f",
    "rgb:00/7f/aa",
    "rgb:00/7f/d4",
    "rgb:00/aa/00",
    "rgb:00/aa/2a",
    "rgb:00/aa/55",
    "rgb:00/aa/7f",
    "rgb:00/aa/aa",
    "rgb:00/aa/d4",
    "rgb:00/d4/00",
    "rgb:00/d4/2a",
    "rgb:00/d4/55",
    "rgb:00/d4/7f",
    "rgb:00/d4/aa",
    "rgb:00/d4/d4",
    "rgb:2a/00/00",
    "rgb:2a/00/2a",
    "rgb:2a/00/55",
    "rgb:2a/00/7f",
    "rgb:2a/00/aa",
    "rgb:2a/00/d4",
    "rgb:2a/2a/00",
    "rgb:2a/2a/2a",
    "rgb:2a/2a/55",
    "rgb:2a/2a/7f",
    "rgb:2a/2a/aa",
    "rgb:2a/2a/d4",
    "rgb:2a/55/00",
    "rgb:2a/55/2a",
    "rgb:2a/55/55",
    "rgb:2a/55/7f",
    "rgb:2a/55/aa",
    "rgb:2a/55/d4",
    "rgb:2a/7f/00",
    "rgb:2a/7f/2a",
    "rgb:2a/7f/55",
    "rgb:2a/7f/7f",
    "rgb:2a/7f/aa",
    "rgb:2a/7f/d4",
    "rgb:2a/aa/00",
    "rgb:2a/aa/2a",
    "rgb:2a/aa/55",
    "rgb:2a/aa/7f",
    "rgb:2a/aa/aa",
    "rgb:2a/aa/d4",
    "rgb:2a/d4/00",
    "rgb:2a/d4/2a",
    "rgb:2a/d4/55",
    "rgb:2a/d4/7f",
    "rgb:2a/d4/aa",
    "rgb:2a/d4/d4",
    "rgb:55/00/00",
    "rgb:55/00/2a",
    "rgb:55/00/55",
    "rgb:55/00/7f",
    "rgb:55/00/aa",
    "rgb:55/00/d4",
    "rgb:55/2a/00",
    "rgb:55/2a/2a",
    "rgb:55/2a/55",
    "rgb:55/2a/7f",
    "rgb:55/2a/aa",
    "rgb:55/2a/d4",
    "rgb:55/55/00",
    "rgb:55/55/2a",
    "rgb:55/55/55",
    "rgb:55/55/7f",
    "rgb:55/55/aa",
    "rgb:55/55/d4",
    "rgb:55/7f/00",
    "rgb:55/7f/2a",
    "rgb:55/7f/55",
    "rgb:55/7f/7f",
    "rgb:55/7f/aa",
    "rgb:55/7f/d4",
    "rgb:55/aa/00",
    "rgb:55/aa/2a",
    "rgb:55/aa/55",
    "rgb:55/aa/7f",
    "rgb:55/aa/aa",
    "rgb:55/aa/d4",
    "rgb:55/d4/00",
    "rgb:55/d4/2a",
    "rgb:55/d4/55",
    "rgb:55/d4/7f",
    "rgb:55/d4/aa",
    "rgb:55/d4/d4",
    "rgb:7f/00/00",
    "rgb:7f/00/2a",
    "rgb:7f/00/55",
    "rgb:7f/00/7f",
    "rgb:7f/00/aa",
    "rgb:7f/00/d4",
    "rgb:7f/2a/00",
    "rgb:7f/2a/2a",
    "rgb:7f/2a/55",
    "rgb:7f/2a/7f",
    "rgb:7f/2a/aa",
    "rgb:7f/2a/d4",
    "rgb:7f/55/00",
    "rgb:7f/55/2a",
    "rgb:7f/55/55",
    "rgb:7f/55/7f",
    "rgb:7f/55/aa",
    "rgb:7f/55/d4",
    "rgb:7f/7f/00",
    "rgb:7f/7f/2a",
    "rgb:7f/7f/55",
    "rgb:7f/7f/7f",
    "rgb:7f/7f/aa",
    "rgb:7f/7f/d4",
    "rgb:7f/aa/00",
    "rgb:7f/aa/2a",
    "rgb:7f/aa/55",
    "rgb:7f/aa/7f",
    "rgb:7f/aa/aa",
    "rgb:7f/aa/d4",
    "rgb:7f/d4/00",
    "rgb:7f/d4/2a",
    "rgb:7f/d4/55",
    "rgb:7f/d4/7f",
    "rgb:7f/d4/aa",
    "rgb:7f/d4/d4",
    "rgb:aa/00/00",
    "rgb:aa/00/2a",
    "rgb:aa/00/55",
    "rgb:aa/00/7f",
    "rgb:aa/00/aa",
    "rgb:aa/00/d4",
    "rgb:aa/2a/00",
    "rgb:aa/2a/2a",
    "rgb:aa/2a/55",
    "rgb:aa/2a/7f",
    "rgb:aa/2a/aa",
    "rgb:aa/2a/d4",
    "rgb:aa/55/00",
    "rgb:aa/55/2a",
    "rgb:aa/55/55",
    "rgb:aa/55/7f",
    "rgb:aa/55/aa",
    "rgb:aa/55/d4",
    "rgb:aa/7f/00",
    "rgb:aa/7f/2a",
    "rgb:aa/7f/55",
    "rgb:aa/7f/7f",
    "rgb:aa/7f/aa",
    "rgb:aa/7f/d4",
    "rgb:aa/aa/00",
    "rgb:aa/aa/2a",
    "rgb:aa/aa/55",
    "rgb:aa/aa/7f",
    "rgb:aa/aa/aa",
    "rgb:aa/aa/d4",
    "rgb:aa/d4/00",
    "rgb:aa/d4/2a",
    "rgb:aa/d4/55",
    "rgb:aa/d4/7f",
    "rgb:aa/d4/aa",
    "rgb:aa/d4/d4",
    "rgb:d4/00/00",
    "rgb:d4/00/2a",
    "rgb:d4/00/55",
    "rgb:d4/00/7f",
    "rgb:d4/00/aa",
    "rgb:d4/00/d4",
    "rgb:d4/2a/00",
    "rgb:d4/2a/2a",
    "rgb:d4/2a/55",
    "rgb:d4/2a/7f",
    "rgb:d4/2a/aa",
    "rgb:d4/2a/d4",
    "rgb:d4/55/00",
    "rgb:d4/55/2a",
    "rgb:d4/55/55",
    "rgb:d4/55/7f",
    "rgb:d4/55/aa",
    "rgb:d4/55/d4",
    "rgb:d4/7f/00",
    "rgb:d4/7f/2a",
    "rgb:d4/7f/55",
    "rgb:d4/7f/7f",
    "rgb:d4/7f/aa",
    "rgb:d4/7f/d4",
    "rgb:d4/aa/00",
    "rgb:d4/aa/2a",
    "rgb:d4/aa/55",
    "rgb:d4/aa/7f",
    "rgb:d4/aa/aa",
    "rgb:d4/aa/d4",
    "rgb:d4/d4/00",
    "rgb:d4/d4/2a",
    "rgb:d4/d4/55",
    "rgb:d4/d4/7f",
    "rgb:d4/d4/aa",
    "rgb:d4/d4/d4",
    "rgb:08/08/08",
    "rgb:12/12/12",
    "rgb:1c/1c/1c",
    "rgb:26/26/26",
    "rgb:30/30/30",
    "rgb:3a/3a/3a",
    "rgb:44/44/44",
    "rgb:4e/4e/4e",
    "rgb:58/58/58",
    "rgb:62/62/62",
    "rgb:6c/6c/6c",
    "rgb:76/76/76",
    "rgb:80/80/80",
    "rgb:8a/8a/8a",
    "rgb:94/94/94",
    "rgb:9e/9e/9e",
    "rgb:a8/a8/a8",
    "rgb:b2/b2/b2",
    "rgb:bc/bc/bc",
    "rgb:c6/c6/c6",
    "rgb:d0/d0/d0",
    "rgb:da/da/da",
    "rgb:e4/e4/e4",
    "rgb:ee/ee/ee",
#endif
#ifndef NO_CURSORCOLOR
    COLOR_CURSOR_BACKGROUND,
    COLOR_CURSOR_FOREGROUND,
#endif                          /* ! NO_CURSORCOLOR */
    NULL,                       /* Color_pointer                  */
    NULL,                       /* Color_border                   */
#ifndef NO_BOLD_UNDERLINE_REVERSE
    NULL,                       /* Color_BD                       */
    NULL,                       /* Color_UL                       */
    NULL,                       /* Color_RV                       */
#endif                          /* ! NO_BOLD_UNDERLINE_REVERSE */
#ifdef OPTION_HC
    NULL,
#endif
#ifdef KEEP_SCROLLCOLOR
    COLOR_SCROLLBAR,
    COLOR_SCROLLTROUGH,
#endif                          /* KEEP_SCROLLCOLOR */

  };

const char *const xa_names[NUM_XA] =
  {
    "TEXT",
    "COMPOUND_TEXT",
    "UTF8_STRING",
    "MULTIPLE",
    "TARGETS",
    "TIMESTAMP",
    "VT_SELECTION",
    "INCR",
    "WM_DELETE_WINDOW",
#ifdef TRANSPARENT
    "_XROOTPMAP_ID",
#endif
#ifdef OFFIX_DND
    "DndProtocol",
    "DndSelection",
#endif
    "CLIPBOARD"
  };

bool
rxvt_term::init_vars ()
{
  PixColors = new rxvt_color [TOTAL_COLORS];
  if (PixColors == NULL)
    return false;

#if defined(XPM_BACKGROUND) || defined(TRANSPARENT)
  TermWin.pixmap = None;
#endif
#ifdef UTMP_SUPPORT
  next_utmp_action = SAVE;
#endif
#ifndef NO_SETOWNER_TTYDEV
  next_tty_action = SAVE;
#endif

  MEvent.time = CurrentTime;
  MEvent.button = AnyButton;
  Options = DEFAULT_OPTIONS;
  want_refresh = 1;
  cmd_pid = -1;
  cmd_fd = tty_fd = -1;
  PrivateModes = SavedModes = PrivMode_Default;
  TermWin.focus = 0;
  TermWin.ncol = 80;
  TermWin.nrow = 24;
  TermWin.int_bwidth = INTERNALBORDERWIDTH;
  TermWin.ext_bwidth = EXTERNALBORDERWIDTH;
  TermWin.lineSpace = LINESPACE;
  TermWin.saveLines = SAVELINES;
  numPixColors = TOTAL_COLORS;

#ifndef NO_NEW_SELECTION
  selection_style = NEW_SELECT;
#else
  selection_style = OLD_SELECT;
#endif

#ifndef NO_BRIGHTCOLOR
  colorfgbg = DEFAULT_RSTYLE;
#endif

  refresh_limit = 1;
  refresh_type = SLOW_REFRESH;
  prev_nrow = prev_ncol = 0;

  oldcursor.row = oldcursor.col = -1;
#ifdef XPM_BACKGROUND
  /*  bgPixmap.w = bgPixmap.h = 0; */
  bgPixmap.x = bgPixmap.y = 50;
  bgPixmap.pixmap = None;
#endif

  last_bot = last_state = -1;

#ifdef MENUBAR
  menu_readonly = 1;
# if ! (MENUBAR_MAX > 1)
  CurrentBar = & (BarList);
# endif                         /* (MENUBAR_MAX > 1) */
#endif

  return true;
}

void
rxvt_term::init_secondary ()
{
  int i;
#ifdef TTY_GID_SUPPORT
  struct group *gr = getgrnam ("tty");

  if (gr)
    {           /* change group ownership of tty to "tty" */
      ttymode = S_IRUSR | S_IWUSR | S_IWGRP;
      ttygid = gr->gr_gid;
    }
  else
#endif                          /* TTY_GID_SUPPORT */

    {
      ttymode = S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH;
      ttygid = getgid ();
    }

  /*
   * Close all unused file descriptors
   * We don't want them, we don't need them.
   */
  if ((i = open ("/dev/null", O_RDONLY)) < 0)
    {
      /* TODO: BOO HISS */
      dup2 (STDERR_FILENO, STDIN_FILENO);
    }
  else if (i > STDIN_FILENO)
    {
      dup2 (i, STDIN_FILENO);
      close (i);
    }

  dup2 (STDERR_FILENO, STDOUT_FILENO);

#if 0 // schmorp sayz closing filies is murder
  for (i = STDERR_FILENO + 1; i < num_fds; i++)
    {
#ifdef __sgi                    /* Alex Coventry says we need 4 & 7 too */
      if (i == 4 || i == 7)
        continue;
#endif
      close (i);
    }
#endif
}

/*----------------------------------------------------------------------*/
const char **
rxvt_term::init_resources (int argc, const char *const *argv)
{
  int i, r_argc;
  char *val;
  const char **cmd_argv, **r_argv;

  /*
   * Look for -exec option.  Find => split and make cmd_argv[] of command args
   */
  for (r_argc = 0; r_argc < argc; r_argc++)
    if (!STRCMP (argv[r_argc], "-e") || !STRCMP (argv[r_argc], "-exec"))
      break;

  r_argv = (const char **)rxvt_malloc (sizeof (char *) * (r_argc + 1));

  for (i = 0; i < r_argc; i++)
    r_argv[i] = (const char *)argv[i];

  r_argv[i] = NULL;

  if (r_argc == argc)
    cmd_argv = NULL;
  else
    {
      cmd_argv = (const char **)rxvt_malloc (sizeof (char *) * (argc - r_argc));

      for (i = 0; i < argc - r_argc - 1; i++)
        cmd_argv[i] = (const char *)argv[i + r_argc + 1];

      cmd_argv[i] = NULL;
    }

  /* clear all resources */
  for (i = 0; i < NUM_RESOURCES;)
    rs[i++] = NULL;

  rs[Rs_name] = rxvt_r_basename (argv[0]);

  /*
   * Open display, get options/resources and create the window
   */
  if ((rs[Rs_display_name] = getenv ("DISPLAY")) == NULL)
    rs[Rs_display_name] = ":0";

  get_options (r_argc, r_argv);
  free (r_argv);

#ifdef LOCAL_X_IS_UNIX
  if (rs[Rs_display_name][0] == ':')
    {
      val = rxvt_malloc (5 + STRLEN (rs[Rs_display_name]));
      STRCPY (val, "unix");
      STRCAT (val, rs[Rs_display_name]);
      display = displays.get (val);
      free (val);
    }
#endif

  if (!display
      && ! (display = displays.get (rs[Rs_display_name])))
    {
      rxvt_print_error ("can't open display %s", rs[Rs_display_name]);
      exit (EXIT_FAILURE);
    }

  extract_resources (display->display, rs[Rs_name]);

  /*
   * set any defaults not already set
   */
  if (cmd_argv && cmd_argv[0])
    {
      if (!rs[Rs_title])
        rs[Rs_title] = rxvt_r_basename (cmd_argv[0]);
      if (!rs[Rs_iconName])
        rs[Rs_iconName] = rs[Rs_title];
    }
  else
    {
      if (!rs[Rs_title])
        rs[Rs_title] = rs[Rs_name];
      if (!rs[Rs_iconName])
        rs[Rs_iconName] = rs[Rs_name];
    }

  if (rs[Rs_saveLines] && (i = atoi (rs[Rs_saveLines])) >= 0)
    TermWin.saveLines = BOUND_POSITIVE_INT16 (i);

#ifndef NO_FRILLS
  if (rs[Rs_int_bwidth] && (i = atoi (rs[Rs_int_bwidth])) >= 0)
    TermWin.int_bwidth = min (i, 100);    /* arbitrary limit */
  if (rs[Rs_ext_bwidth] && (i = atoi (rs[Rs_ext_bwidth])) >= 0)
    TermWin.ext_bwidth = min (i, 100);    /* arbitrary limit */
#endif

#ifndef NO_LINESPACE
  if (rs[Rs_lineSpace] && (i = atoi (rs[Rs_lineSpace])) >= 0)
    TermWin.lineSpace = min (i, 100);     /* arbitrary limit */
#endif

#ifdef POINTER_BLANK
  if (rs[Rs_pointerBlankDelay] && (i = atoi (rs[Rs_pointerBlankDelay])) >= 0)
    pointerBlankDelay = i;
  else
    pointerBlankDelay = 2;
#endif

  /* no point having a scrollbar without having any scrollback! */
  if (!TermWin.saveLines)
    Options &= ~Opt_scrollBar;

#ifdef PRINTPIPE
  if (!rs[Rs_print_pipe])
    rs[Rs_print_pipe] = PRINTPIPE;
#endif

  if (!rs[Rs_cutchars])
    rs[Rs_cutchars] = CUTCHARS;

#ifndef NO_BACKSPACE_KEY
  if (!rs[Rs_backspace_key])
# ifdef DEFAULT_BACKSPACE
    key_backspace = DEFAULT_BACKSPACE;
# else
    key_backspace = "DEC";       /* can toggle between \010 or \177 */
# endif
  else
    {
      val = STRDUP (rs[Rs_backspace_key]);
      rxvt_Str_trim (val);
      rxvt_Str_escaped (val);
      key_backspace = val;
    }
#endif

#ifndef NO_DELETE_KEY
  if (!rs[Rs_delete_key])
# ifdef DEFAULT_DELETE
    key_delete = DEFAULT_DELETE;
# else
    key_delete = "\033[3~";
# endif
  else
    {
      val = STRDUP (rs[Rs_delete_key]);
      rxvt_Str_trim (val);
      rxvt_Str_escaped (val);
      key_delete = val;
    }
#endif
  if (rs[Rs_answerbackstring])
    {
      rxvt_Str_trim ((char *)rs[Rs_answerbackstring]);
      rxvt_Str_escaped ((char *)rs[Rs_answerbackstring]);
    }

  if (rs[Rs_selectstyle])
    {
      if (STRNCASECMP (rs[Rs_selectstyle], "oldword", 7) == 0)
        selection_style = OLD_WORD_SELECT;
#ifndef NO_OLD_SELECTION

      else if (STRNCASECMP (rs[Rs_selectstyle], "old", 3) == 0)
        selection_style = OLD_SELECT;
#endif

    }

#ifdef HAVE_SCROLLBARS
  setup_scrollbar (rs[Rs_scrollBar_align], rs[Rs_scrollstyle],
                   rs[Rs_scrollBar_thickness]);
#endif

#ifdef XTERM_REVERSE_VIDEO
  /* this is how xterm implements reverseVideo */
  if (Options & Opt_reverseVideo)
    {
      if (!rs[Rs_color + Color_fg])
        rs[Rs_color + Color_fg] = def_colorName[Color_bg];
      if (!rs[Rs_color + Color_bg])
        rs[Rs_color + Color_bg] = def_colorName[Color_fg];
    }
#endif

  for (i = 0; i < NRS_COLORS; i++)
    if (!rs[Rs_color + i])
      rs[Rs_color + i] = def_colorName[i];

#ifndef XTERM_REVERSE_VIDEO
  /* this is how we implement reverseVideo */
  if (Options & Opt_reverseVideo)
    SWAP_IT (rs[Rs_color + Color_fg], rs[Rs_color + Color_bg], const char *);
#endif

  /* convenient aliases for setting fg/bg to colors */
  color_aliases (Color_fg);
  color_aliases (Color_bg);
#ifndef NO_CURSORCOLOR
  color_aliases (Color_cursor);
  color_aliases (Color_cursor2);
#endif                          /* NO_CURSORCOLOR */
  color_aliases (Color_pointer);
  color_aliases (Color_border);
#ifndef NO_BOLD_UNDERLINE_REVERSE
  color_aliases (Color_BD);
  color_aliases (Color_UL);
  color_aliases (Color_RV);
#endif                          /* ! NO_BOLD_UNDERLINE_REVERSE */

  return cmd_argv;
}

/*----------------------------------------------------------------------*/
void
rxvt_term::init_env ()
{
  int i;
  unsigned int u;
  char *val;

#ifdef DISPLAY_IS_IP
  /* Fixup display_name for export over pty to any interested terminal
   * clients via "ESC[7n" (e.g. shells).  Note we use the pure IP number
   * (for the first non-loopback interface) that we get from
   * rxvt_network_display ().  This is more "name-resolution-portable", if you
   * will, and probably allows for faster x-client startup if your name
   * server is beyond a slow link or overloaded at client startup.  Of
   * course that only helps the shell's child processes, not us.
   *
   * Giving out the display_name also affords a potential security hole
   */
  val = rxvt_network_display (rs[Rs_display_name]);
  rs[Rs_display_name] = (const char *)val;

  if (val == NULL)
#endif                          /* DISPLAY_IS_IP */
    val = XDisplayString (display->display);

  if (rs[Rs_display_name] == NULL)
    rs[Rs_display_name] = val;   /* use broken `:0' value */

  i = STRLEN (val);
  env_display = (char *)rxvt_malloc ((i + 9) * sizeof (char));

  sprintf (env_display, "DISPLAY=%s", val);

  /* avoiding the math library:
   * i = (int) (ceil (log10 ((unsigned int)TermWin.parent[0]))) */
  for (i = 0, u = (unsigned int)TermWin.parent[0]; u; u /= 10, i++)
    ;
  MAX_IT (i, 1);
  env_windowid = (char *)rxvt_malloc ((i + 10) * sizeof (char));

  sprintf (env_windowid, "WINDOWID=%u",
           (unsigned int)TermWin.parent[0]);

  /* add entries to the environment:
   * @ DISPLAY:   in case we started with -display
   * @ WINDOWID:  X window id number of the window
   * @ COLORTERM: terminal sub-name and also indicates its color
   * @ TERM:      terminal name
   * @ TERMINFO:  path to terminfo directory
   * @ COLORFGBG: fg;bg color codes
   */
  putenv (env_display);
  putenv (env_windowid);
  if (env_colorfgbg)
    putenv (env_colorfgbg);

#ifdef RXVT_TERMINFO
  putenv ("TERMINFO=" RXVT_TERMINFO);
#endif

  if (display->depth <= 2)
    putenv ("COLORTERM=" COLORTERMENV "-mono");
  else
    putenv ("COLORTERM=" COLORTERMENVFULL);

  if (rs[Rs_term_name] != NULL)
    {
      env_term = (char *)rxvt_malloc ((STRLEN (rs[Rs_term_name]) + 6) * sizeof (char));
      sprintf (env_term, "TERM=%s", rs[Rs_term_name]);
      putenv (env_term);
    }
  else
    putenv ("TERM=" TERMENV);

#ifdef HAVE_UNSETENV
  /* avoid passing old settings and confusing term size */
  unsetenv ("LINES");
  unsetenv ("COLUMNS");
  unsetenv ("TERMCAP");        /* terminfo should be okay */
#endif                          /* HAVE_UNSETENV */
}

/*----------------------------------------------------------------------*/
/*
 * This is more or less stolen straight from XFree86 xterm.
 * This should support all European type languages.
 */
void
rxvt_term::set_locale (const char *locale)
{
#if HAVE_XSETLOCALE || HAVE_SETLOCALE
  free (this->locale);
  this->locale = rxvt_strdup (setlocale (LC_CTYPE, locale));
  SET_LOCALE (this->locale);
  mbstate.reset ();
#endif
#if HAVE_NL_LANGINFO
  free (codeset);
  codeset = strdup (nl_langinfo (CODESET));
  enc_utf8 = !STRCASECMP (codeset, "UTF-8")
             || !STRCASECMP (codeset, "UTF8");
#else
  enc_utf8 = 1;
#endif
}

void
rxvt_term::init_xlocale ()
{
#ifdef USE_XIM
  if (!locale)
    rxvt_print_error ("Setting locale failed.");
  else
    {
      Atom wmlocale;

      wmlocale = XInternAtom (display->display, "WM_LOCALE_NAME", False);
      XChangeProperty (display->display, TermWin.parent[0], wmlocale,
                       XA_STRING, 8, PropModeReplace,
                       (unsigned char *)locale, STRLEN (locale));

      if (!XSupportsLocale ())
        {
          rxvt_print_error ("The locale is not supported by Xlib");
          return;
        }

      im_ev.start (display);

      /* see if we can connect already */
      im_cb ();
    }
#endif
}

/*----------------------------------------------------------------------*/
void
rxvt_term::init_command (const char *const *argv)
{
  /*
   * Initialize the command connection.
   * This should be called after the X server connection is established.
   */
  int i;

  for (i = 0; i < NUM_XA; i++)
    xa[i] = XInternAtom (display->display, xa_names[i], False);

  /* Enable delete window protocol */
  XSetWMProtocols (display->display, TermWin.parent[0],
                   & (xa[XA_WMDELETEWINDOW]), 1);

#ifdef USING_W11LIB
  /* enable W11 callbacks */
  W11AddEventHandler (display->display, rxvt_W11_process_x_event);
#endif

#ifdef META8_OPTION
  meta_char = (Options & Opt_meta8 ? 0x80 : C0_ESC);
#endif

  get_ourmods ();

  if (! (Options & Opt_scrollTtyOutput))
    PrivateModes |= PrivMode_TtyOutputInh;
  if (Options & Opt_scrollTtyKeypress)
    PrivateModes |= PrivMode_Keypress;
  if (! (Options & Opt_jumpScroll))
    PrivateModes |= PrivMode_smoothScroll;

#ifndef NO_BACKSPACE_KEY
  if (STRCMP (key_backspace, "DEC") == 0)
    PrivateModes |= PrivMode_HaveBackSpace;
#endif

  /* add value for scrollBar */
  if (scrollbar_visible ())
    {
      PrivateModes |= PrivMode_scrollBar;
      SavedModes |= PrivMode_scrollBar;
    }
  if (menubar_visible ())
    {
      PrivateModes |= PrivMode_menuBar;
      SavedModes |= PrivMode_menuBar;
    }

#ifdef CURSOR_BLINK
  if (Options & Opt_cursorBlink)
    (void)gettimeofday (&lastcursorchange, NULL);
#endif

  if ((cmd_fd = run_command (argv)) < 0)
    {
      rxvt_print_error ("aborting");
      exit (EXIT_FAILURE);
    }
}

/*----------------------------------------------------------------------*/
void
rxvt_term::Get_Colours ()
{
  int i;

  for (i = 0; i < (XDEPTH <= 2 ? 2 : NRS_COLORS); i++)
    {
      rxvt_color xcol;

      if (!rs[Rs_color + i])
        continue;

      if (!rXParseAllocColor (&xcol, rs[Rs_color + i]))
        {
#ifndef XTERM_REVERSE_VIDEO
          if (i < 2 && (Options & Opt_reverseVideo))
            rs[Rs_color + i] = def_colorName[!i];
          else
#endif
            rs[Rs_color + i] = def_colorName[i];

          if (!rs[Rs_color + i])
            continue;

          if (!rXParseAllocColor (&xcol, rs[Rs_color + i]))
            {
              switch (i)
                {
                  case Color_fg:
                  case Color_bg:
                    /* fatal: need bg/fg color */
                    rxvt_print_error ("aborting");
                    exit (EXIT_FAILURE);
                    /* NOTREACHED */
                    break;
#ifndef NO_CURSORCOLOR
                  case Color_cursor2:
                    xcol = PixColors[Color_fg];
                    break;
#endif                          /* ! NO_CURSORCOLOR */
                  case Color_pointer:
                    xcol = PixColors[Color_fg];
                    break;
                  default:
                    xcol = PixColors[Color_bg];      /* None */
                    break;
                }
            }
        }

      PixColors[i] = xcol;
      SET_PIXCOLOR (i);
    }

  if (XDEPTH <= 2 || !rs[Rs_color + Color_pointer])
    PixColors[Color_pointer] = PixColors[Color_fg];
  if (XDEPTH <= 2 || !rs[Rs_color + Color_border])
    PixColors[Color_border] = PixColors[Color_fg];

  /*
   * get scrollBar/menuBar shadow colors
   *
   * The calculations of topShadow/bottomShadow values are adapted
   * from the fvwm window manager.
   */
#ifdef KEEP_SCROLLCOLOR

  if (XDEPTH <= 2)
    {  /* Monochrome */
      PixColors[Color_scroll] = PixColors[Color_fg];
      PixColors[Color_topShadow] = PixColors[Color_bg];
      PixColors[Color_bottomShadow] = PixColors[Color_bg];
    }
  else
    {
      rxvt_color          xcol[3];
      /* xcol[0] == white
       * xcol[1] == top shadow
       * xcol[2] == bot shadow */

      xcol[1] = PixColors[Color_scroll];
# ifdef PREFER_24BIT
      xcol[0].set (display, 65535, 65535, 65535);
      /*        XFreeColors (display->display, XCMAP, & (xcol[0].pixel), 1, ~0); */
# else
      xcol[0].set (display, WhitePixel (display->display, display->screen));
# endif

      unsigned short pr1, pg1, pb1, pr0, pg0, pb0;

      xcol[0].get (display, pr0, pg0, pb0);
      xcol[1].get (display, pr1, pg1, pb1);

      /* bottomShadowColor */
      if (!xcol[2].set (display, pr1 / 2, pg1 / 2, pb1 / 2))
        xcol[2] = PixColors[Color_Black];

      PixColors[Color_bottomShadow] = xcol[2];

      /* topShadowColor */
      if (!xcol[1].set (display,
                        min (pr0, max (pr0 / 5, pr1) * 7 / 5),
                        min (pg0, max (pg0 / 5, pg1) * 7 / 5),
                        min (pb0, max (pb0 / 5, pb1) * 7 / 5)))
        xcol[1] = PixColors[Color_White];

      PixColors[Color_topShadow] = xcol[1];
    }
#endif                          /* KEEP_SCROLLCOLOR */
}

/*----------------------------------------------------------------------*/
/* color aliases, fg/bg bright-bold */
void
rxvt_term::color_aliases (int idx)
{
  if (rs[Rs_color + idx] && isdigit (* (rs[Rs_color + idx])))
    {
      int i = atoi (rs[Rs_color + idx]);

      if (i >= 8 && i <= 15)
        {        /* bright colors */
          i -= 8;
#ifndef NO_BRIGHTCOLOR
          rs[Rs_color + idx] = rs[Rs_color + minBrightCOLOR + i];
          return;
#endif

        }

      if (i >= 0 && i <= 7)   /* normal colors */
        rs[Rs_color + idx] = rs[Rs_color + minCOLOR + i];
    }
}

/*----------------------------------------------------------------------*/
/*
 * Probe the modifier keymap to get the Meta (Alt) and Num_Lock settings
 * Use resource ``modifier'' to override the Meta modifier
 */
void
rxvt_term::get_ourmods ()
{
  int             i, j, k;
  int             requestedmeta, realmeta, realalt;
  const char     *cm, *rsmod;
  XModifierKeymap *map;
  KeyCode        *kc;
  const unsigned int modmasks[] =
    {
      Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };

  requestedmeta = realmeta = realalt = 0;
  rsmod = rs[Rs_modifier];
  if (rsmod
      && STRCASECMP (rsmod, "mod1") >= 0 && STRCASECMP (rsmod, "mod5") <= 0)
    requestedmeta = rsmod[3] - '0';

  map = XGetModifierMapping (display->display);
  kc = map->modifiermap;
  for (i = 1; i < 6; i++)
    {
      k = (i + 2) * map->max_keypermod;       /* skip shift/lock/control */
      for (j = map->max_keypermod; j--; k++)
        {
          if (kc[k] == 0)
            break;
          switch (XKeycodeToKeysym (display->display, kc[k], 0))
            {
              case XK_Num_Lock:
                ModNumLockMask = modmasks[i - 1];
                /* FALLTHROUGH */
              default:
                continue;       /* for (;;) */
              case XK_Meta_L:
              case XK_Meta_R:
                cm = "meta";
                realmeta = i;
                break;
              case XK_Alt_L:
              case XK_Alt_R:
                cm = "alt";
                realalt = i;
                break;
              case XK_Super_L:
              case XK_Super_R:
                cm = "super";
                break;
              case XK_Hyper_L:
              case XK_Hyper_R:
                cm = "hyper";
                break;
            }
          if (rsmod && STRNCASECMP (rsmod, cm, STRLEN (cm)) == 0)
            requestedmeta = i;
        }
    }
  XFreeModifiermap (map);
  i = (requestedmeta ? requestedmeta
       : (realmeta ? realmeta
          : (realalt ? realalt : 0)));
  if (i)
    ModMetaMask = modmasks[i - 1];
}

/*----------------------------------------------------------------------*/
/* rxvt_Create_Windows () - Open and map the window */
void
rxvt_term::create_windows (int argc, const char *const *argv)
{
  XClassHint      classHint;
  XWMHints        wmHint;
  XGCValues       gcvalue;
  long            vt_emask;

  XWindowAttributes gattr;

  if (Options & Opt_transparent)
    {
      XGetWindowAttributes (display->display, RootWindow (display->display, display->screen), &gattr);
      display->depth = gattr.depth; // doh //TODO, per-term not per-display?
    }

  /* grab colors before netscape does */
  Get_Colours ();

  if (!change_font (rs[Rs_font]))
    {
      fprintf (stderr, "unable to load a base font, please provide one using -fn fontname\n");
      destroy ();
      return;
    }

  window_calc (0, 0);
  old_width = szHint.width;
  old_height = szHint.height;

  /* parent window - reverse video so we can see placement errors
   * sub-window placement & size in rxvt_resize_subwindows ()
   */

#ifdef PREFER_24BIT
  XSetWindowAttributes attributes;

  attributes.background_pixel = PixColors[Color_fg];
  attributes.border_pixel = PixColors[Color_border];
  attributes.colormap = display->cmap;
  TermWin.parent[0] = XCreateWindow (display->display, DefaultRootWindow (display->display),
                                     szHint.x, szHint.y,
                                     szHint.width, szHint.height,
                                     TermWin.ext_bwidth,
                                     display->depth, InputOutput,
                                     display->visual,
                                     CWBackPixel | CWBorderPixel | CWColormap, &attributes);
#else
  TermWin.parent[0] = XCreateSimpleWindow (display->display, DefaultRootWindow (display->display),
                      szHint.x, szHint.y,
                      szHint.width,
                      szHint.height,
                      TermWin.ext_bwidth,
                      PixColors[Color_border],
                      PixColors[Color_fg]);
#endif

  xterm_seq (XTerm_title, rs[Rs_title], CHAR_ST);
  xterm_seq (XTerm_iconName, rs[Rs_iconName], CHAR_ST);

  classHint.res_name = (char *)rs[Rs_name];
  classHint.res_class = (char *)RESCLASS;

  wmHint.flags = (InputHint | StateHint | WindowGroupHint);
  wmHint.input = True;
  wmHint.initial_state = (Options & Opt_iconic ? IconicState
                          : NormalState);
  wmHint.window_group = TermWin.parent[0];

  XSetWMProperties (display->display, TermWin.parent[0], NULL, NULL,
                   (char **)argv, argc, &szHint, &wmHint, &classHint);

  XSelectInput (display->display, TermWin.parent[0],
               KeyPressMask
#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
               | KeyReleaseMask
#endif
               | FocusChangeMask | VisibilityChangeMask
               | StructureNotifyMask);
  termwin_ev.start (display, TermWin.parent[0]);

  /* vt cursor: Black-on-White is standard, but this is more popular */
  TermWin_cursor = XCreateFontCursor (display->display, XC_xterm);

#if defined(HAVE_SCROLLBARS) || defined(MENUBAR)
  /* cursor (menuBar/scrollBar): Black-on-White */
  leftptr_cursor = XCreateFontCursor (display->display, XC_left_ptr);
#endif

#ifdef POINTER_BLANK
  {
    XColor blackcolour;
    blackcolour.red   = 0;
    blackcolour.green = 0;
    blackcolour.blue  = 0;
    Font f = XLoadFont (display->display, "fixed");
    blank_cursor = XCreateGlyphCursor (display->display, f, f, ' ', ' ',
                                       &blackcolour, &blackcolour);
    XUnloadFont (display->display, f);
  }
#endif

  /* the vt window */
  TermWin.vt = XCreateSimpleWindow (display->display, TermWin.parent[0],
                                   window_vt_x, window_vt_y,
                                   TermWin_TotalWidth (),
                                   TermWin_TotalHeight (),
                                   0,
                                   PixColors[Color_fg],
                                   PixColors[Color_bg]);

#ifdef DEBUG_X
  XStoreName (display->display, TermWin.vt, "vt window");
#endif

  vt_emask = (ExposureMask | ButtonPressMask | ButtonReleaseMask
              | PropertyChangeMask);

#ifdef POINTER_BLANK
  pointer_unblank ();

  if ((Options & Opt_pointerBlank))
    vt_emask |= PointerMotionMask;
  else
#endif
    vt_emask |= (Button1MotionMask | Button3MotionMask);

  XSelectInput (display->display, TermWin.vt, vt_emask);
  vt_ev.start (display, TermWin.vt);

#if defined(MENUBAR) && (MENUBAR_MAX > 1)
  if (menuBar_height ())
    {
      menuBar.win = XCreateSimpleWindow (display->display, TermWin.parent[0],
                                         window_vt_x, 0,
                                         TermWin_TotalWidth (),
                                         menuBar_TotalHeight (),
                                         0,
                                         PixColors[Color_fg],
                                         PixColors[Color_scroll]);

#ifdef DEBUG_X
      XStoreName (display->display, menuBar.win, "menubar");
#endif

      menuBar.drawable = new rxvt_drawable (display, menuBar.win);

      XDefineCursor (display->display, menuBar.win,
                     XCreateFontCursor (display->display, XC_left_ptr));

      XSelectInput (display->display, menuBar.win,
                   (ExposureMask | ButtonPressMask | ButtonReleaseMask
                    | Button1MotionMask));
      menubar_ev.start (display, menuBar.win);
    }
#endif

#ifdef XPM_BACKGROUND
  if (rs[Rs_backgroundPixmap] != NULL
      && ! (Options & Opt_transparent))
    {
      const char     *p = rs[Rs_backgroundPixmap];

      if ((p = STRCHR (p, ';')) != NULL)
        {
          p++;
          scale_pixmap (p);
        }
      set_bgPixmap (rs[Rs_backgroundPixmap]);
      scr_touch (True);
    }
#endif

  /* graphics context for the vt window */
  gcvalue.foreground = PixColors[Color_fg];
  gcvalue.background = PixColors[Color_bg];
  gcvalue.graphics_exposures = 1;
  TermWin.gc = XCreateGC (display->display, TermWin.vt,
                         GCForeground | GCBackground
                         | GCGraphicsExposures, &gcvalue);

  TermWin.drawable = new rxvt_drawable (display, TermWin.vt);

#if defined(MENUBAR) || defined(RXVT_SCROLLBAR)
  gcvalue.foreground = PixColors[Color_topShadow];
  topShadowGC = XCreateGC (display->display, TermWin.vt, GCForeground, &gcvalue);
  gcvalue.foreground = PixColors[Color_bottomShadow];
  botShadowGC = XCreateGC (display->display, TermWin.vt, GCForeground, &gcvalue);
  gcvalue.foreground = PixColors[ (XDEPTH <= 2 ? Color_fg : Color_scroll)];
  scrollbarGC = XCreateGC (display->display, TermWin.vt, GCForeground, &gcvalue);
#endif
}

/*----------------------------------------------------------------------*/
/*
 * Run the command in a subprocess and return a file descriptor for the
 * master end of the pseudo-teletype pair with the command talking to
 * the slave.
 */
int
rxvt_term::run_command (const char *const *argv)
{
  int cfd, er;

  /* get master (pty) */
  if ((cfd = rxvt_get_pty (& (tty_fd), & (ttydev))) < 0)
    {
      rxvt_print_error ("can't open pseudo-tty");
      return -1;
    }

  fcntl (cfd, F_SETFL, O_NONBLOCK);

  /* get slave (tty) */
  if (tty_fd < 0)
    {
#ifndef NO_SETOWNER_TTYDEV
      privileged_ttydev (SAVE);
#endif

      if ((tty_fd = rxvt_get_tty (ttydev)) < 0)
        {
          close (cfd);
          rxvt_print_error ("can't open slave tty %s", ttydev);
          return -1;
        }
    }
#ifndef NO_BACKSPACE_KEY
  if (key_backspace[0] && !key_backspace[1])
    er = key_backspace[0];
  else if (STRCMP (key_backspace, "DEC") == 0)
    er = '\177';            /* the initial state anyway */
  else
#endif

    er = -1;

  rxvt_get_ttymode (& (tio), er);

#ifndef __QNX__
  /* spin off the command interpreter */
  switch (cmd_pid = fork ())
    {
      case -1:
        rxvt_print_error ("can't fork");
        return -1;
      case 0:
        close (cfd);             /* only keep tty_fd and STDERR open */

        init_env ();

        if (rxvt_control_tty (tty_fd, ttydev) < 0)
          rxvt_print_error ("could not obtain control of tty");
        else
          {
            /* Reopen stdin, stdout and stderr over the tty file descriptor */
            dup2 (tty_fd, STDIN_FILENO);
            dup2 (tty_fd, STDOUT_FILENO);
            dup2 (tty_fd, STDERR_FILENO);

            if (tty_fd > 2)
              close (tty_fd);

            run_child (argv);
          }
        exit (EXIT_FAILURE);
        /* NOTREACHED */
      default:
        {
#if defined(HAVE_STRUCT_UTMP) && defined(HAVE_TTYSLOT)
          int fdstdin;

          fdstdin = dup (STDIN_FILENO);
          dup2 (tty_fd, STDIN_FILENO);
#endif

#ifdef UTMP_SUPPORT
          privileged_utmp (SAVE);
#endif

#if defined(HAVE_STRUCT_UTMP) && defined(HAVE_TTYSLOT)

          dup2 (fdstdin, STDIN_FILENO);
          close (fdstdin);
#endif

        }
        close (tty_fd);       /* keep STDERR_FILENO, cmd_fd, display->fd () open */
        break;
    }
#else                           /* __QNX__ uses qnxspawn () */
  fchmod (tty_fd, 0622);
  fcntl (tty_fd, F_SETFD, FD_CLOEXEC);
  fcntl (cfd, F_SETFD, FD_CLOEXEC);

  if (run_child (argv) == -1)
    exit (EXIT_FAILURE);
#endif

  return cfd;
}

/* ------------------------------------------------------------------------- *
 *                          CHILD PROCESS OPERATIONS                         *
 * ------------------------------------------------------------------------- */
/*
 * The only open file descriptor is the slave tty - so no error messages.
 * returns are fatal
 */
int
rxvt_term::run_child (const char *const *argv)
{
  char *login;

  SET_TTYMODE (STDIN_FILENO, & (tio));       /* init terminal attributes */

  if (Options & Opt_console)
    {     /* be virtual console, fail silently */
#ifdef TIOCCONS
      unsigned int on = 1;

      ioctl (STDIN_FILENO, TIOCCONS, &on);
#elif defined (SRIOCSREDIR)
      int fd;

      fd = open (CONSOLE, O_WRONLY, 0);
      if (fd >= 0)
        {
          if (ioctl (fd, SRIOCSREDIR, NULL) < 0)
            close (fd);
        }
#endif                          /* SRIOCSREDIR */

    }

  /* reset signals and spin off the command interpreter */
  signal (SIGINT,  SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
  signal (SIGCHLD, SIG_DFL);
  /*
   * mimick login's behavior by disabling the job control signals
   * a shell that wants them can turn them back on
   */
#ifdef SIGTSTP
  signal (SIGTSTP, SIG_IGN);
  signal (SIGTTIN, SIG_IGN);
  signal (SIGTTOU, SIG_IGN);
#endif                          /* SIGTSTP */

  /* set window size */
  struct winsize ws;

  ws.ws_col = TermWin.ncol;
  ws.ws_row = TermWin.nrow;
  ws.ws_xpixel = ws.ws_ypixel = 0;
  (void)ioctl (STDIN_FILENO, TIOCSWINSZ, &ws);

  // unblock signals (signals are blocked by iom.C
  sigset_t ss;
  sigemptyset (&ss);
  sigprocmask (SIG_SETMASK, &ss, 0);

#ifndef __QNX__
  /* command interpreter path */
  if (argv != NULL)
    {
# ifdef DEBUG_CMD
      int             i;

      for (i = 0; argv[i]; i++)
        fprintf (stderr, "argv [%d] = \"%s\"\n", i, argv[i]);
# endif

      execvp (argv[0], (char *const *)argv);
      /* no error message: STDERR is closed! */
    }
  else
    {
      const char     *argv0, *shell;

      if ((shell = getenv ("SHELL")) == NULL || *shell == '\0')
        shell = "/bin/sh";

      argv0 = (const char *)rxvt_r_basename (shell);
      if (Options & Opt_loginShell)
        {
          login = (char *)rxvt_malloc ((STRLEN (argv0) + 2) * sizeof (char));

          login[0] = '-';
          STRCPY (&login[1], argv0);
          argv0 = login;
        }
      execlp (shell, argv0, NULL);
      /* no error message: STDERR is closed! */
    }

#else                           /* __QNX__ uses qnxspawn () */

  char            iov_a[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
  char           *command = NULL, fullcommand[_MAX_PATH];
  char          **arg_v, *arg_a[2] = { NULL, NULL };

  if (argv != NULL)
    {
      if (access (argv[0], X_OK) == -1)
        {
          if (STRCHR (argv[0], '/') == NULL)
            {
              searchenv (argv[0], "PATH", fullcommand);
              if (fullcommand[0] != '\0')
                command = fullcommand;
            }
          if (access (command, X_OK) == -1)
            return -1;
        }
      else
        command = argv[0];
      arg_v = argv;
    }
  else
    {
      if ((command = getenv ("SHELL")) == NULL || *command == '\0')
        command = "/bin/sh";

      arg_a[0] = my_basename (command);
      if (Options & Opt_loginShell)
        {
          login = rxvt_malloc ((STRLEN (arg_a[0]) + 2) * sizeof (char));

          login[0] = '-';
          STRCPY (&login[1], arg_a[0]);
          arg_a[0] = login;
        }
      arg_v = arg_a;
    }
  iov_a[0] = iov_a[1] = iov_a[2] = tty_fd;
  cmd_pid = qnx_spawn (0, 0, 0, -1, -1,
                      _SPAWN_SETSID | _SPAWN_TCSETPGRP,
                      command, arg_v, environ, iov_a, 0);
  if (login)
    free (login);

  close (tty_fd);
  return cmd_fd;

#endif
  return -1;
}

/* ------------------------------------------------------------------------- *
 *                            GET TTY CURRENT STATE                          *
 * ------------------------------------------------------------------------- */
/* rxvt_get_ttymode () */
/* EXTPROTO */
void
rxvt_get_ttymode (ttymode_t *tio, int erase)
{
#ifdef HAVE_TERMIOS_H
  /*
   * standard System V termios interface
   */
  if (GET_TERMIOS (STDIN_FILENO, tio) < 0)
    {
      /* return error - use system defaults */
      tio->c_cc[VINTR] = CINTR;
      tio->c_cc[VQUIT] = CQUIT;
      tio->c_cc[VERASE] = CERASE;
      tio->c_cc[VKILL] = CKILL;
      tio->c_cc[VSTART] = CSTART;
      tio->c_cc[VSTOP] = CSTOP;
      tio->c_cc[VSUSP] = CSUSP;
# ifdef VDSUSP

      tio->c_cc[VDSUSP] = CDSUSP;
# endif
# ifdef VREPRINT

      tio->c_cc[VREPRINT] = CRPRNT;
# endif
# ifdef VDISCRD

      tio->c_cc[VDISCRD] = CFLUSH;
# endif
# ifdef VWERSE

      tio->c_cc[VWERSE] = CWERASE;
# endif
# ifdef VLNEXT

      tio->c_cc[VLNEXT] = CLNEXT;
# endif

    }
  tio->c_cc[VEOF] = CEOF;
  tio->c_cc[VEOL] = VDISABLE;
# ifdef VEOL2

  tio->c_cc[VEOL2] = VDISABLE;
# endif
# ifdef VSWTC

  tio->c_cc[VSWTC] = VDISABLE;
# endif
# ifdef VSWTCH

  tio->c_cc[VSWTCH] = VDISABLE;
# endif
# if VMIN != VEOF

  tio->c_cc[VMIN] = 1;
# endif
# if VTIME != VEOL

  tio->c_cc[VTIME] = 0;
# endif

  if (erase != -1)
    tio->c_cc[VERASE] = (char)erase;

  /* input modes */
  tio->c_iflag = (BRKINT | IGNPAR | ICRNL
# ifdef IMAXBEL
                  | IMAXBEL
# endif
                  | IXON);

  /* output modes */
  tio->c_oflag = (OPOST | ONLCR);

  /* control modes */
  tio->c_cflag = (CS8 | CREAD);

  /* line discipline modes */
  tio->c_lflag = (ISIG | ICANON | IEXTEN | ECHO
# if defined (ECHOCTL) && defined (ECHOKE)
                  | ECHOCTL | ECHOKE
# endif
                  | ECHOE | ECHOK);
# else                          /* HAVE_TERMIOS_H */

  /*
  * sgtty interface
  */

  /* get parameters -- gtty */
  if (ioctl (STDIN_FILENO, TIOCGETP, & (tio->sg)) < 0)
    {
      tio->sg.sg_erase = CERASE;      /* ^H */
      tio->sg.sg_kill = CKILL;        /* ^U */
    }
  if (erase != -1)
    tio->sg.sg_erase = (char)erase;

  tio->sg.sg_flags = (CRMOD | ECHO | EVENP | ODDP);

  /* get special characters */
  if (ioctl (STDIN_FILENO, TIOCGETC, & (tio->tc)) < 0)
    {
      tio->tc.t_intrc = CINTR;        /* ^C */
      tio->tc.t_quitc = CQUIT;        /* ^\ */
      tio->tc.t_startc = CSTART;      /* ^Q */
      tio->tc.t_stopc = CSTOP;        /* ^S */
      tio->tc.t_eofc = CEOF;  /* ^D */
      tio->tc.t_brkc = -1;
    }
  /* get local special chars */
  if (ioctl (STDIN_FILENO, TIOCGLTC, & (tio->lc)) < 0)
    {
      tio->lc.t_suspc = CSUSP;        /* ^Z */
      tio->lc.t_dsuspc = CDSUSP;      /* ^Y */
      tio->lc.t_rprntc = CRPRNT;      /* ^R */
      tio->lc.t_flushc = CFLUSH;      /* ^O */
      tio->lc.t_werasc = CWERASE;     /* ^W */
      tio->lc.t_lnextc = CLNEXT;      /* ^V */
    }
  /* get line discipline */
  ioctl (STDIN_FILENO, TIOCGETD, & (tio->line));
# ifdef NTTYDISC

  tio->line = NTTYDISC;
# endif                         /* NTTYDISC */

  tio->local = (LCRTBS | LCRTERA | LCTLECH | LPASS8 | LCRTKIL);
#endif                          /* HAVE_TERMIOS_H */

  /*
   * Debugging
   */
#ifdef DEBUG_TTYMODE
#ifdef HAVE_TERMIOS_H
  /* c_iflag bits */
  fprintf (stderr, "Input flags\n");

  /* cpp token stringize doesn't work on all machines <sigh> */
# define FOO(flag,name)                 \
    if ((tio->c_iflag) & flag)          \
        fprintf (stderr, "%s ", name)

  /* c_iflag bits */
  FOO (IGNBRK, "IGNBRK");
  FOO (BRKINT, "BRKINT");
  FOO (IGNPAR, "IGNPAR");
  FOO (PARMRK, "PARMRK");
  FOO (INPCK, "INPCK");
  FOO (ISTRIP, "ISTRIP");
  FOO (INLCR, "INLCR");
  FOO (IGNCR, "IGNCR");
  FOO (ICRNL, "ICRNL");
  FOO (IXON, "IXON");
  FOO (IXOFF, "IXOFF");
# ifdef IUCLC

  FOO (IUCLC, "IUCLC");
# endif
# ifdef IXANY

  FOO (IXANY, "IXANY");
# endif
# ifdef IMAXBEL

  FOO (IMAXBEL, "IMAXBEL");
# endif

  fprintf (stderr, "\n");

# undef FOO
# define FOO(entry, name)                                       \
    fprintf (stderr, "%-8s = %#04o\n", name, tio->c_cc [entry])

  FOO (VINTR, "VINTR");
  FOO (VQUIT, "VQUIT");
  FOO (VERASE, "VERASE");
  FOO (VKILL, "VKILL");
  FOO (VEOF, "VEOF");
  FOO (VEOL, "VEOL");
# ifdef VEOL2

  FOO (VEOL2, "VEOL2");
# endif
# ifdef VSWTC

  FOO (VSWTC, "VSWTC");
# endif
# ifdef VSWTCH

  FOO (VSWTCH, "VSWTCH");
# endif

  FOO (VSTART, "VSTART");
  FOO (VSTOP, "VSTOP");
  FOO (VSUSP, "VSUSP");
# ifdef VDSUSP

  FOO (VDSUSP, "VDSUSP");
# endif
# ifdef VREPRINT

  FOO (VREPRINT, "VREPRINT");
# endif
# ifdef VDISCRD

  FOO (VDISCRD, "VDISCRD");
# endif
# ifdef VWERSE

  FOO (VWERSE, "VWERSE");
# endif
# ifdef VLNEXT

  FOO (VLNEXT, "VLNEXT");
# endif

  fprintf (stderr, "\n");
# undef FOO
# endif                         /* HAVE_TERMIOS_H */
#endif                          /* DEBUG_TTYMODE */
}

/*----------------------- end-of-file (C source) -----------------------*/
