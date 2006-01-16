/*--------------------------------*-C-*---------------------------------*
 * File:        init.C
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
 * Copyright (c) 2003-2006 Marc Lehmann <pcg@goof.com>
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
#include "rxvtutil.h"
#include "init.h"

#include <limits>

#include <csignal>

const char *const def_colorName[] =
  {
    COLOR_FOREGROUND,
    COLOR_BACKGROUND,
    /* low-intensity colors */
    "Black",                    /* 0: black             (#000000) */
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
    "Red",                      /* 1/9: bright red      (#FF0000) */
    "Green",                    /* 2/10: bright green   (#00FF00) */
    "Yellow",                   /* 3/11: bright yellow  (#FFFF00) */
    "Blue",                     /* 4/12: bright blue    (#0000FF) */
    "Magenta",                  /* 5/13: bright magenta (#FF00FF) */
    "Cyan",                     /* 6/14: bright cyan    (#00FFFF) */
    "White",                    /* 7/15: bright white   (#FFFFFF) */

    // 88 xterm colours
    "rgb:00/00/00",
    "rgb:00/00/8b",
    "rgb:00/00/cd",
    "rgb:00/00/ff",
    "rgb:00/8b/00",
    "rgb:00/8b/8b",
    "rgb:00/8b/cd",
    "rgb:00/8b/ff",
    "rgb:00/cd/00",
    "rgb:00/cd/8b",
    "rgb:00/cd/cd",
    "rgb:00/cd/ff",
    "rgb:00/ff/00",
    "rgb:00/ff/8b",
    "rgb:00/ff/cd",
    "rgb:00/ff/ff",
    "rgb:8b/00/00",
    "rgb:8b/00/8b",
    "rgb:8b/00/cd",
    "rgb:8b/00/ff",
    "rgb:8b/8b/00",
    "rgb:8b/8b/8b",
    "rgb:8b/8b/cd",
    "rgb:8b/8b/ff",
    "rgb:8b/cd/00",
    "rgb:8b/cd/8b",
    "rgb:8b/cd/cd",
    "rgb:8b/cd/ff",
    "rgb:8b/ff/00",
    "rgb:8b/ff/8b",
    "rgb:8b/ff/cd",
    "rgb:8b/ff/ff",
    "rgb:cd/00/00",
    "rgb:cd/00/8b",
    "rgb:cd/00/cd",
    "rgb:cd/00/ff",
    "rgb:cd/8b/00",
    "rgb:cd/8b/8b",
    "rgb:cd/8b/cd",
    "rgb:cd/8b/ff",
    "rgb:cd/cd/00",
    "rgb:cd/cd/8b",
    "rgb:cd/cd/cd",
    "rgb:cd/cd/ff",
    "rgb:cd/ff/00",
    "rgb:cd/ff/8b",
    "rgb:cd/ff/cd",
    "rgb:cd/ff/ff",
    "rgb:ff/00/00",
    "rgb:ff/00/8b",
    "rgb:ff/00/cd",
    "rgb:ff/00/ff",
    "rgb:ff/8b/00",
    "rgb:ff/8b/8b",
    "rgb:ff/8b/cd",
    "rgb:ff/8b/ff",
    "rgb:ff/cd/00",
    "rgb:ff/cd/8b",
    "rgb:ff/cd/cd",
    "rgb:ff/cd/ff",
    "rgb:ff/ff/00",
    "rgb:ff/ff/8b",
    "rgb:ff/ff/cd",
    "rgb:ff/ff/ff",
    "rgb:2e/2e/2e",
    "rgb:5c/5c/5c",
    "rgb:73/73/73",
    "rgb:8b/8b/8b",
    "rgb:a2/a2/a2",
    "rgb:b9/b9/b9",
    "rgb:d0/d0/d0",
    "rgb:e7/e7/e7",

#ifndef NO_CURSORCOLOR
    COLOR_CURSOR_BACKGROUND,
    COLOR_CURSOR_FOREGROUND,
#endif                          /* ! NO_CURSORCOLOR */
    NULL,                       /* Color_pointer_fg               */
    NULL,                       /* Color_pointer_bg               */
    NULL,                       /* Color_border                   */
#ifndef NO_BOLD_UNDERLINE_REVERSE
    NULL,                       /* Color_BD                       */
    NULL,                       /* Color_IT                       */
    NULL,                       /* Color_UL                       */
    NULL,                       /* Color_RV                       */
#endif                          /* ! NO_BOLD_UNDERLINE_REVERSE */
#if ENABLE_FRILLS
    NULL,			// Color_underline
#endif
#ifdef OPTION_HC
    NULL,
#endif
#ifdef KEEP_SCROLLCOLOR
    COLOR_SCROLLBAR,
    COLOR_SCROLLTROUGH,
#endif                          /* KEEP_SCROLLCOLOR */
#if TINTING
    NULL,
#endif
#if OFF_FOCUS_FADING
    "black",
#endif
  };

const char *const xa_names[] =
  {
    "TEXT",
    "COMPOUND_TEXT",
    "UTF8_STRING",
    "MULTIPLE",
    "TARGETS",
    "TIMESTAMP",
    "VT_SELECTION",
    "INCR",
    "WM_PROTOCOLS",
    "WM_DELETE_WINDOW",
    "CLIPBOARD",
#if ENABLE_FRILLS
    "_MOTIF_WM_HINTS",
#endif
#if ENABLE_EWMH
    "_NET_WM_PID",
    "_NET_WM_NAME",
    "_NET_WM_ICON_NAME",
    "_NET_WM_PING",
#endif
#if USE_XIM
    "WM_LOCALE_NAME",
#endif
#ifdef TRANSPARENT
    "_XROOTPMAP_ID",
    "ESETROOT_PMAP_ID",
#endif
#ifdef OFFIX_DND
    "DndProtocol",
    "DndSelection",
#endif
#if ENABLE_XEMBED
    "_XEMBED",
    "_XEMBED_INFO",
#endif
  };

bool
rxvt_term::init_vars ()
{
  pix_colors_focused = new rxvt_color [TOTAL_COLORS];
#ifdef OFF_FOCUS_FADING
  pix_colors_unfocused = new rxvt_color [TOTAL_COLORS];
#endif
  pix_colors = pix_colors_focused;

  if (pix_colors == NULL)
    return false;

#if defined(XPM_BACKGROUND) || defined(TRANSPARENT)
  pixmap = None;
#endif

  MEvent.time = CurrentTime;
  MEvent.button = AnyButton;
  options = DEFAULT_OPTIONS;
  want_refresh = 1;
  priv_modes = SavedModes = PrivMode_Default;
  focus = 0;
  ncol = 80;
  nrow = 24;
  int_bwidth = INTERNALBORDERWIDTH;
  ext_bwidth = EXTERNALBORDERWIDTH;
  lineSpace = LINESPACE;
  saveLines = SAVELINES;
  numpix_colors = TOTAL_COLORS;

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

  return true;
}

void
rxvt_term::init_secondary ()
{
  int i;

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
    if (!strcmp (argv[r_argc], "-e") || !strcmp (argv[r_argc], "-exec"))
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

  rs[Rs_name] = rxvt_r_basename (argv[0]);

  /*
   * Open display, get options/resources and create the window
   */

  if ((rs[Rs_display_name] = getenv ("DISPLAY")) == NULL)
    rs[Rs_display_name] = ":0";

  get_options (r_argc, r_argv);

  if (!(display = displays.get (rs[Rs_display_name])))
    rxvt_fatal ("can't open display %s, aborting.\n", rs[Rs_display_name]);

  extract_resources ();

  free (r_argv);

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
    saveLines = min (i, MAX_SAVELINES);

#if ENABLE_FRILLS
  if (rs[Rs_int_bwidth] && (i = atoi (rs[Rs_int_bwidth])) >= 0)
    int_bwidth = min (i, std::numeric_limits<int16_t>::max ());

  if (rs[Rs_ext_bwidth] && (i = atoi (rs[Rs_ext_bwidth])) >= 0)
    ext_bwidth = min (i, std::numeric_limits<int16_t>::max ());

  if (rs[Rs_lineSpace] && (i = atoi (rs[Rs_lineSpace])) >= 0)
    lineSpace = min (i, std::numeric_limits<int16_t>::max ());
#endif

#ifdef POINTER_BLANK
  if (rs[Rs_pointerBlankDelay] && (i = atoi (rs[Rs_pointerBlankDelay])) >= 0)
    pointerBlankDelay = i;
  else
    pointerBlankDelay = 2;
#endif

  /* no point having a scrollbar without having any scrollback! */
  if (!saveLines)
    set_option (Opt_scrollBar, 0);

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
      val = strdup (rs[Rs_backspace_key]);
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
      val = strdup (rs[Rs_delete_key]);
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

#ifdef HAVE_SCROLLBARS
  setup_scrollbar (rs[Rs_scrollBar_align], rs[Rs_scrollstyle], rs[Rs_scrollBar_thickness]);
#endif

#ifdef XTERM_REVERSE_VIDEO
  /* this is how xterm implements reverseVideo */
  if (OPTION (Opt_reverseVideo))
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
  if (OPTION (Opt_reverseVideo))
    ::swap (rs[Rs_color + Color_fg], rs[Rs_color + Color_bg]);
#endif

  /* convenient aliases for setting fg/bg to colors */
  color_aliases (Color_fg);
  color_aliases (Color_bg);
#ifndef NO_CURSORCOLOR
  color_aliases (Color_cursor);
  color_aliases (Color_cursor2);
#endif                          /* NO_CURSORCOLOR */
  color_aliases (Color_pointer_fg);
  color_aliases (Color_pointer_bg);
  color_aliases (Color_border);
#ifndef NO_BOLD_UNDERLINE_REVERSE
  color_aliases (Color_BD);
  color_aliases (Color_UL);
  color_aliases (Color_RV);
#endif                          /* ! NO_BOLD_UNDERLINE_REVERSE */

  if (!rs[Rs_color + Color_border])
    rs[Rs_color + Color_border] = rs[Rs_color + Color_bg];

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

  i = strlen (val);
  env_display = (char *)rxvt_malloc ((i + 9) * sizeof (char));

  sprintf (env_display, "DISPLAY=%s", val);

  /* avoiding the math library:
   * i = (int) (ceil (log10 ((unsigned int)parent[0]))) */
  for (i = 0, u = (unsigned int)parent[0]; u; u /= 10, i++)
    ;
  max_it (i, 1);
  env_windowid = (char *)rxvt_malloc ((i + 10) * sizeof (char));

  sprintf (env_windowid, "WINDOWID=%u",
           (unsigned int)parent[0]);

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
      env_term = (char *)rxvt_malloc ((strlen (rs[Rs_term_name]) + 6) * sizeof (char));
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
  set_environ (envv);

#if HAVE_XSETLOCALE || HAVE_SETLOCALE
  free (this->locale);
  this->locale = setlocale (LC_CTYPE, locale);

  if (!this->locale)
    {
      if (*locale)
        {
          rxvt_warn ("unable to set locale \"%s\", using C locale instead.\n", locale);
          setlocale (LC_CTYPE, "C");
        }
      else
        rxvt_warn ("default locale unavailable, check LC_* and LANG variables. Continuing.\n");

      this->locale = "C";
    }


  this->locale = rxvt_strdup (this->locale);
  SET_LOCALE (this->locale);
  mbstate.reset ();
#endif

#if HAVE_NL_LANGINFO
  char *codeset = strdup (nl_langinfo (CODESET));
  // /^UTF.?8/i
  enc_utf8 = (codeset[0] == 'U' || codeset[0] == 'u')
          && (codeset[1] == 'T' || codeset[1] == 't')
          && (codeset[2] == 'F' || codeset[2] == 'f')
          && (codeset[3] == '8' || codeset[4] == '8');
  free (codeset);
#else
  enc_utf8 = 0;
#endif
}

void
rxvt_term::init_xlocale ()
{
  set_environ (envv);

#ifdef USE_XIM
  if (!locale)
    rxvt_warn ("setting locale failed, working without locale support.\n");
  else
    {
      set_string_property (xa[XA_WM_LOCALE_NAME], locale);

      if (!XSupportsLocale ())
        {
          rxvt_warn ("the locale is not supported by Xlib, working without locale support.\n");
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

#ifdef META8_OPTION
  meta_char = OPTION (Opt_meta8) ? 0x80 : C0_ESC;
#endif

  get_ourmods ();

  if (!OPTION (Opt_scrollTtyOutput))
    priv_modes |= PrivMode_TtyOutputInh;
  if (OPTION (Opt_scrollTtyKeypress))
    priv_modes |= PrivMode_Keypress;
  if (!OPTION (Opt_jumpScroll))
    priv_modes |= PrivMode_smoothScroll;

#ifndef NO_BACKSPACE_KEY
  if (strcmp (key_backspace, "DEC") == 0)
    priv_modes |= PrivMode_HaveBackSpace;
#endif

  /* add value for scrollBar */
  if (scrollBar.state)
    {
      priv_modes |= PrivMode_scrollBar;
      SavedModes |= PrivMode_scrollBar;
    }

  run_command (argv);
}

/*----------------------------------------------------------------------*/
void
rxvt_term::Get_Colours ()
{
  int i;

#ifdef OFF_FOCUS_FADING
  pix_colors = pix_colors_focused;
#endif
  
  for (i = 0; i < (display->depth <= 2 ? 2 : NRS_COLORS); i++)
    {
      rxvt_color xcol;

      if (!rs[Rs_color + i])
        continue;

      if (!rXParseAllocColor (&xcol, rs[Rs_color + i]))
        {
#ifndef XTERM_REVERSE_VIDEO
          if (i < 2 && OPTION (Opt_reverseVideo))
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
                    rxvt_fatal ("unable to get foreground/background colour, aborting.\n");
                    /* NOTREACHED */
                    break;
#ifndef NO_CURSORCOLOR
                  case Color_cursor2:
                    xcol = pix_colors[Color_fg];
                    break;
#endif                          /* ! NO_CURSORCOLOR */
                  case Color_pointer_fg:
                    xcol = pix_colors[Color_fg];
                    break;
                  default:
                    xcol = pix_colors[Color_bg];      /* None */
                    break;
                }
            }
        }

      pix_colors[i] = xcol;
      SET_PIXCOLOR (i);
    }

#ifdef OFF_FOCUS_FADING
  if (rs[Rs_fade])
    for (i = 0; i < (display->depth <= 2 ? 2 : NRS_COLORS); i++)
      pix_colors_unfocused[i] = pix_colors_focused[i].fade (display, atoi (rs[Rs_fade]), pix_colors[Color_fade]);
#endif

  if (display->depth <= 2)
    {
      if (!rs[Rs_color + Color_pointer_fg]) pix_colors[Color_pointer_fg] = pix_colors[Color_fg];
      if (!rs[Rs_color + Color_pointer_bg]) pix_colors[Color_pointer_bg] = pix_colors[Color_bg];
      if (!rs[Rs_color + Color_border]    ) pix_colors[Color_border]     = pix_colors[Color_fg];
    }

  /*
   * get scrollBar shadow colors
   *
   * The calculations of topShadow/bottomShadow values are adapted
   * from the fvwm window manager.
   */
#ifdef KEEP_SCROLLCOLOR

  if (display->depth <= 2)
    {
      /* Monochrome */
      pix_colors[Color_scroll]       = pix_colors[Color_fg];
      pix_colors[Color_topShadow]    = pix_colors[Color_bg];
      pix_colors[Color_bottomShadow] = pix_colors[Color_bg];
    }
  else
    {
      rxvt_color xcol[2];
      /* xcol[0] == white
       * xcol[1] == top shadow
       * xcol[2] == bot shadow */

      xcol[1] = pix_colors[Color_scroll];
      xcol[0].set (display, 65535, 65535, 65535);

      unsigned short pr1, pg1, pb1, pr0, pg0, pb0;

      xcol[0].get (display, pr0, pg0, pb0);
      xcol[1].get (display, pr1, pg1, pb1);

      pix_colors[Color_bottomShadow] = xcol[1].fade (display, 50);

      /* topShadowColor */
      if (!xcol[1].set (display,
                        min (pr0, max (pr0 / 5, pr1) * 7 / 5),
                        min (pg0, max (pg0 / 5, pg1) * 7 / 5),
                        min (pb0, max (pb0 / 5, pb1) * 7 / 5)))
        xcol[1] = pix_colors[Color_White];

      pix_colors[Color_topShadow] = xcol[1];
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
          rs[Rs_color + idx] = rs[Rs_color + minBrightCOLOR + i];
          return;
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
  int i, j, k;
  int requestedmeta, realmeta, realalt;
  const char *cm, *rsmod;
  XModifierKeymap *map;
  KeyCode *kc;
  const unsigned int modmasks[] =
    {
      Mod1Mask, Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };

  requestedmeta = realmeta = realalt = 0;
  rsmod = rs[Rs_modifier];

  if (rsmod
      && strcasecmp (rsmod, "mod1") >= 0 && strcasecmp (rsmod, "mod5") <= 0)
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
                continue;

              case XK_ISO_Level3_Shift:
                ModLevel3Mask = modmasks[i - 1];
                continue;

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

              default:
                continue;
            }

          if (rsmod && strncasecmp (rsmod, cm, strlen (cm)) == 0)
            requestedmeta = i;
        }
    }

  XFreeModifiermap (map);

  i = requestedmeta ? requestedmeta
    : realmeta      ? realmeta
    : realalt       ? realalt
    : 0;

  if (i)
    ModMetaMask = modmasks[i - 1];
}

/*----------------------------------------------------------------------*/
/* rxvt_Create_Windows () - Open and map the window */
void
rxvt_term::create_windows (int argc, const char *const *argv)
{
  XClassHint classHint;
  XWMHints wmHint;
#if ENABLE_FRILLS
  Atom prop = None;
  MWMHints mwmhints;
#endif
  XGCValues gcvalue;
  XSetWindowAttributes attributes;
  XWindowAttributes gattr;
  Window top, parent;
  dDisp;

#ifdef USING_W11LIB
  /* enable W11 callbacks */
  W11AddEventHandler (disp, rxvt_W11_process_x_event);
#endif

  assert (sizeof (xa_names) / sizeof (char *) == NUM_XA);
  XInternAtoms (disp, (char **)xa_names, NUM_XA, False, xa);

  if (OPTION (Opt_transparent))
    {
      XGetWindowAttributes (disp, RootWindow (disp, display->screen), &gattr);
      display->depth = gattr.depth; // doh //TODO, per-term not per-display?
    }

#if ENABLE_FRILLS
  if (OPTION (Opt_borderLess))
    {
      prop = XInternAtom(disp, "_MOTIF_WM_INFO", True);

      if (prop == None)
        {
          /*     print_warning("Window Manager does not support MWM hints.  Bypassing window manager control for borderless window.\n");*/
#ifdef PREFER_24BIT
          attributes.override_redirect = TRUE;
#endif
          mwmhints.flags = 0;
        }
      else
        {
          mwmhints.flags = MWM_HINTS_DECORATIONS;
          mwmhints.decorations = 0;
        }
    }
  else
    mwmhints.flags = 0;
#endif

  /* grab colors before netscape does */
  Get_Colours ();

  if (!set_fonts ())
    rxvt_fatal ("unable to load base fontset, please specify a valid one using -fn, aborting.\n");

  parent = DefaultRootWindow (disp);

#if ENABLE_XEMBED
  if (rs[Rs_embed])
    {
      XWindowAttributes wattr;

      parent = strtol (rs[Rs_embed], 0, 0);

      if (!XGetWindowAttributes (disp, parent, &wattr))
        rxvt_fatal ("invalid window-id specified with -embed, aborting.\n");

      window_calc (wattr.width, wattr.height);
    }
#endif

  window_calc (0, 0);

  /* sub-window placement & size in rxvt_resize_subwindows () */
#ifdef PREFER_24BIT
  attributes.background_pixel = pix_colors_focused[Color_border];
  attributes.border_pixel = pix_colors_focused[Color_border];
  attributes.colormap = display->cmap;
  top = XCreateWindow (disp, parent,
                       szHint.x, szHint.y,
                       szHint.width, szHint.height,
                       ext_bwidth,
                       display->depth, InputOutput,
                       display->visual,
                       CWColormap | CWBackPixel | CWBorderPixel, &attributes);
#else
  top = XCreateSimpleWindow (disp, parent,
                             szHint.x, szHint.y,
                             szHint.width, szHint.height,
                             ext_bwidth,
                             pix_colors_focused[Color_border],
                             pix_colors_focused[Color_border]);
#endif

  this->parent[0] = top;

  old_width = szHint.width;
  old_height = szHint.height;

  process_xterm_seq (XTerm_title, rs[Rs_title], CHAR_ST);
  process_xterm_seq (XTerm_iconName, rs[Rs_iconName], CHAR_ST);

  classHint.res_name = (char *)rs[Rs_name];
  classHint.res_class = (char *)RESCLASS;

  wmHint.flags = InputHint | StateHint | WindowGroupHint;
  wmHint.input = True;
  wmHint.initial_state = OPTION (Opt_iconic) ? IconicState : NormalState;
  wmHint.window_group = top;

  XmbSetWMProperties (disp, top, NULL, NULL, (char **)argv, argc,
                      &szHint, &wmHint, &classHint);

  Atom protocols[] = {
    xa[XA_WM_DELETE_WINDOW],
#if ENABLE_EWMH
    xa[XA_NET_WM_PING],
#endif
  };

  XSetWMProtocols (disp, top, protocols, sizeof (protocols) / sizeof (protocols[0]));

#if ENABLE_FRILLS
  if (rs[Rs_transient_for])
    XSetTransientForHint (disp, top, (Window)strtol (rs[Rs_transient_for], 0, 0));
#endif

#if ENABLE_EWMH
  long pid = getpid ();

  XChangeProperty (disp, top,
                   xa[XA_NET_WM_PID], XA_CARDINAL, 32,
                   PropModeReplace, (unsigned char *)&pid, 1);

  // _NET_WM_WINDOW_TYPE is NORMAL, which is the default
#endif

  XSelectInput (disp, top,
                KeyPressMask
#if (MOUSE_WHEEL && MOUSE_SLIP_WHEELING) || ENABLE_FRILLS || ISO_14755
                | KeyReleaseMask
#endif
                | FocusChangeMask | VisibilityChangeMask
                | ExposureMask | StructureNotifyMask);

  termwin_ev.start (display, top);

#if ENABLE_FRILLS
  if (mwmhints.flags)
    XChangeProperty (disp, top, xa[XA_MOTIF_WM_HINTS], xa[XA_MOTIF_WM_HINTS], 32,
                     PropModeReplace, (unsigned char *)&mwmhints, PROP_MWM_HINTS_ELEMENTS);
#endif

  /* vt cursor: Black-on-White is standard, but this is more popular */
  TermWin_cursor = XCreateFontCursor (disp, XC_xterm);

#ifdef HAVE_SCROLLBARS
  /* cursor scrollBar: Black-on-White */
  leftptr_cursor = XCreateFontCursor (disp, XC_left_ptr);
#endif

  /* the vt window */
  vt = XCreateSimpleWindow (disp, top,
                                    window_vt_x,
                                    window_vt_y,
                                    width,
                                    height,
                                    0,
                                    pix_colors_focused[Color_fg],
                                    pix_colors_focused[Color_bg]);
#ifdef DEBUG_X
  XStoreName (disp, vt, "vt window");
#endif

  attributes.bit_gravity = NorthWestGravity;
  XChangeWindowAttributes (disp, vt, CWBitGravity, &attributes);

  vt_emask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PropertyChangeMask;

  if (OPTION (Opt_pointerBlank))
    vt_emask |= PointerMotionMask;
  else
    vt_emask |= Button1MotionMask | Button3MotionMask;

  vt_select_input ();

  vt_ev.start (display, vt);

#ifdef XPM_BACKGROUND
  if (rs[Rs_backgroundPixmap] != NULL
      && ! OPTION (Opt_transparent))
    {
      const char *p = rs[Rs_backgroundPixmap];

      if ((p = strchr (p, ';')) != NULL)
        {
          p++;
          scale_pixmap (p);
        }

      set_bgPixmap (rs[Rs_backgroundPixmap]);
      scr_touch (true);
    }
#endif

  /* graphics context for the vt window */
  gcvalue.foreground = pix_colors[Color_fg];
  gcvalue.background = pix_colors[Color_bg];
  gcvalue.graphics_exposures = 1;
  gc = XCreateGC (disp, vt,
                          GCForeground | GCBackground | GCGraphicsExposures,
                          &gcvalue);

  drawable = new rxvt_drawable (display, vt);

#ifdef RXVT_SCROLLBAR
  gcvalue.foreground = pix_colors[Color_topShadow];
  topShadowGC = XCreateGC (disp, vt, GCForeground, &gcvalue);
  gcvalue.foreground = pix_colors[Color_bottomShadow];
  botShadowGC = XCreateGC (disp, vt, GCForeground, &gcvalue);
  gcvalue.foreground = pix_colors[ (display->depth <= 2 ? Color_fg : Color_scroll)];
  scrollbarGC = XCreateGC (disp, vt, GCForeground, &gcvalue);
#endif

#ifdef OFF_FOCUS_FADING
  // initially we are in unfocused state
  if (rs[Rs_fade])
    pix_colors = pix_colors_unfocused;
#endif

  pointer_unblank ();
  scr_recolour ();

#if ENABLE_XEMBED
  if (rs[Rs_embed])
    {
      long info[2] = { 0, XEMBED_MAPPED };

      XChangeProperty (disp, parent, xa[XA_XEMBED_INFO], xa[XA_XEMBED_INFO],
                       32, PropModeReplace, (unsigned char *)&info, 2);
    }
#endif
}

/* ------------------------------------------------------------------------- *
 *                            GET TTY CURRENT STATE                          *
 * ------------------------------------------------------------------------- */
void
rxvt_get_ttymode (ttymode_t *tio, int erase)
{
#ifdef HAVE_TERMIOS_H
  /*
   * standard System V termios interface
   */
  if (GET_TERMIOS (STDIN_FILENO, tio) < 0)
    {
      // return error - use system defaults,
      // where possible, and zero elsewhere
      memset (tio, 0, sizeof (ttymode_t));

      tio->c_cc[VINTR] = CINTR;
      tio->c_cc[VQUIT] = CQUIT;
      tio->c_cc[VERASE] = CERASE;
#ifdef VERASE2
      tio->c_cc[VERASE2] = CERASE2;
#endif
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

/*----------------------------------------------------------------------*/
/*
 * Run the command in a subprocess and return a file descriptor for the
 * master end of the pseudo-teletype pair with the command talking to
 * the slave.
 */
void
rxvt_term::run_command (const char *const *argv)
{
#if ENABLE_FRILLS
  if (rs[Rs_pty_fd])
    {
      pty.pty = atoi (rs[Rs_pty_fd]);

      if (pty.pty >= 0)
        {
          if (getfd_hook)
            pty.pty = (*getfd_hook) (pty.pty);

          if (pty.pty < 0 || fcntl (pty.pty, F_SETFL, O_NONBLOCK))
            rxvt_fatal ("unusable pty-fd filehandle, aborting.\n");
        }
    }
  else
#endif
    if (!pty.get ())
      rxvt_fatal ("can't initialize pseudo-tty, aborting.\n");

  pty.set_utf8_mode (enc_utf8);

  /* set initial window size */
  tt_winch ();

  int er;

#ifndef NO_BACKSPACE_KEY
  if (key_backspace[0] && !key_backspace[1])
    er = key_backspace[0];
  else if (strcmp (key_backspace, "DEC") == 0)
    er = '\177';            /* the initial state anyway */
  else
#endif
    er = -1;

  rxvt_get_ttymode (&tio, er);

#if ENABLE_FRILLS
  if (rs[Rs_pty_fd])
    return;
#endif

#ifndef __QNX__
  /* spin off the command interpreter */
  switch (cmd_pid = fork ())
    {
      case -1:
        {
          cmd_pid = 0;
          rxvt_fatal ("can't fork, aborting.\n");
        }
      case 0:
        init_env ();

        if (!pty.make_controlling_tty ())
          fprintf (stderr, "%s: could not obtain control of tty.", RESNAME);
        else
          {
            /* Reopen stdin, stdout and stderr over the tty file descriptor */
            dup2 (pty.tty, STDIN_FILENO);
            dup2 (pty.tty, STDOUT_FILENO);
            dup2 (pty.tty, STDERR_FILENO);

            // close all our file handles that we do no longer need
            for (rxvt_term **t = termlist.begin (); t < termlist.end (); t++)
              {
                if ((*t)->pty.pty > 2) close ((*t)->pty.pty);
                if ((*t)->pty.tty > 2) close ((*t)->pty.tty);
              }

            run_child (argv);
            fprintf (stderr, "%s: unable to exec child.", RESNAME);
          }

        _exit (EXIT_FAILURE);

      default:
        {
#if defined(HAVE_STRUCT_UTMP) && defined(HAVE_TTYSLOT)
          int fdstdin;

          fdstdin = dup (STDIN_FILENO);
          dup2 (pty.tty, STDIN_FILENO);
#endif

#ifdef UTMP_SUPPORT
          privileged_utmp (SAVE);
#endif

#if defined(HAVE_STRUCT_UTMP) && defined(HAVE_TTYSLOT)

          dup2 (fdstdin, STDIN_FILENO);
          close (fdstdin);
#endif
        }

        pty.close_tty ();   /* keep STDERR_FILENO, pty.pty, display->fd () open */
        break;
    }
#else                           /* __QNX__ uses qnxspawn () */
  fchmod (pty.tty, 0622);
  fcntl (pty.tty, F_SETFD, FD_CLOEXEC);
  fcntl (pty.pty, F_SETFD, FD_CLOEXEC);

  if (run_child (argv) == -1)
    exit (EXIT_FAILURE);
#endif
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

  SET_TTYMODE (STDIN_FILENO, &tio);       /* init terminal attributes */

  if (OPTION (Opt_console))
    {     /* be virtual console, fail silently */
#ifdef TIOCCONS
      unsigned int on = 1;

      ioctl (STDIN_FILENO, TIOCCONS, &on);
#elif defined (SRIOCSREDIR)
      int fd;

      fd = open (CONSOLE, O_WRONLY, 0);
      if (fd >= 0)
        if (ioctl (fd, SRIOCSREDIR, NULL) < 0)
          close (fd);
#endif                          /* SRIOCSREDIR */
    }

  /* reset signals and spin off the command interpreter */
  signal (SIGINT,  SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
  signal (SIGCHLD, SIG_DFL);
  signal (SIGHUP,  SIG_DFL);
  signal (SIGPIPE, SIG_DFL);
  /*
   * mimick login's behavior by disabling the job control signals
   * a shell that wants them can turn them back on
   */
#ifdef SIGTSTP
  signal (SIGTSTP, SIG_IGN);
  signal (SIGTTIN, SIG_IGN);
  signal (SIGTTOU, SIG_IGN);
#endif                          /* SIGTSTP */

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
      const char *argv0, *shell;

      if ((shell = getenv ("SHELL")) == NULL || *shell == '\0')
        shell = "/bin/sh";

      argv0 = (const char *)rxvt_r_basename (shell);

      if (OPTION (Opt_loginShell))
        {
          login = (char *)rxvt_malloc ((strlen (argv0) + 2) * sizeof (char));

          login[0] = '-';
          strcpy (&login[1], argv0);
          argv0 = login;
        }

      execlp (shell, argv0, NULL);
      /* no error message: STDERR is closed! */
    }

#else                           /* __QNX__ uses qnxspawn () */

  char iov_a[10] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
  char *command = NULL, fullcommand[_MAX_PATH];
  char **arg_v, *arg_a[2] = { NULL, NULL };

  if (argv != NULL)
    {
      if (access (argv[0], X_OK) == -1)
        {
          if (strchr (argv[0], '/') == NULL)
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

      if (OPTION (Opt_loginShell))
        {
          login = rxvt_malloc ((strlen (arg_a[0]) + 2) * sizeof (char));

          login[0] = '-';
          strcpy (&login[1], arg_a[0]);
          arg_a[0] = login;
        }

      arg_v = arg_a;
    }

  iov_a[0] = iov_a[1] = iov_a[2] = pty.tty;
  cmd_pid = qnx_spawn (0, 0, 0, -1, -1,
                       _SPAWN_SETSID | _SPAWN_TCSETPGRP,
                       command, arg_v, environ, iov_a, 0);
  if (login)
    free (login);

  pty.close_tty ();
  return pty.pty;
#endif

  return -1;
}

/*----------------------- end-of-file (C source) -----------------------*/
