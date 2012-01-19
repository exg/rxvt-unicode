/*----------------------------------------------------------------------*
 * File:        init.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *                              - original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *                              - extensive modifications
 * Copyright (c) 1996      Chuck Blake <cblake@BBN.COM>
 * Copyright (c) 1997      mj olesen <olesen@me.queensu.ca>
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 *                              - extensive modifications
 * Copyright (c) 2003-2008 Marc Lehmann <schmorp@schmorp.de>
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
#include "keyboard.h"

#include <limits>

#include <signal.h>

#include <fcntl.h>

#ifdef HAVE_XSETLOCALE
# define X_LOCALE
# include <X11/Xlocale.h>
#else
# include <locale.h>
#endif

#ifdef HAVE_NL_LANGINFO
# include <langinfo.h>
#endif

#ifdef HAVE_STARTUP_NOTIFICATION
# define SN_API_NOT_YET_FROZEN
# include <libsn/sn-launchee.h>
#endif

#ifdef DISPLAY_IS_IP
/* On Solaris link with -lsocket and -lnsl */
#include <sys/types.h>
#include <sys/socket.h>

/* these next two are probably only on Sun (not Solaris) */
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#ifdef HAVE_SYS_BYTEORDER_H
#include <sys/byteorder.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>

static char *
rxvt_network_display (const char *display)
{
  char            buffer[1024], *rval = NULL;
  struct ifconf   ifc;
  struct ifreq   *ifr;
  int             i, skfd;

  if (display[0] != ':' && strncmp (display, "unix:", 5))
    return (char *) display;		/* nothing to do */

  ifc.ifc_len = sizeof (buffer);	/* Get names of all ifaces */
  ifc.ifc_buf = buffer;

  if ((skfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("socket");
      return NULL;
    }

  if (ioctl (skfd, SIOCGIFCONF, &ifc) < 0)
    {
      perror ("SIOCGIFCONF");
      close (skfd);
      return NULL;
    }

  for (i = 0, ifr = ifc.ifc_req;
       i < (ifc.ifc_len / sizeof (struct ifreq));
       i++, ifr++)
    {
      struct ifreq ifr2;

      strcpy (ifr2.ifr_name, ifr->ifr_name);

      if (ioctl (skfd, SIOCGIFADDR, &ifr2) >= 0)
        {
          unsigned long addr;
          struct sockaddr_in *p_addr;

          p_addr = (struct sockaddr_in *) &ifr2.ifr_addr;
          addr = htonl ((unsigned long)p_addr->sin_addr.s_addr);

          /*
           * not "0.0.0.0" or "127.0.0.1" - so format the address
           */
          if (addr && addr != 0x7F000001)
            {
              char *colon = strchr (display, ':');

              if (colon == NULL)
                colon = ":0.0";

              rval = rxvt_malloc (strlen (colon) + 16);
              sprintf (rval, "%d.%d.%d.%d%s",
                      (int) ((addr >> 030) & 0xFF),
                      (int) ((addr >> 020) & 0xFF),
                      (int) ((addr >> 010) & 0xFF),
                      (int) (addr & 0xFF), colon);
              break;
            }
        }
    }

  close (skfd);

  return rval;
}
#endif

static const char *const def_colorName[] =
  {
    COLOR_FOREGROUND,
    COLOR_BACKGROUND,
    /* low-intensity colors */
    "rgb:00/00/00",             // 0: black             (Black)
    "rgb:cd/00/00",             // 1: red               (Red3)
    "rgb:00/cd/00",             // 2: green             (Green3)
    "rgb:cd/cd/00",             // 3: yellow            (Yellow3)
    "rgb:00/00/cd",             // 4: blue              (Blue3)
    "rgb:cd/00/cd",             // 5: magenta           (Magenta3)
    "rgb:00/cd/cd",             // 6: cyan              (Cyan3)
# ifdef XTERM_COLORS
    "rgb:e5/e5/e5",             // 7: white             (Grey90)
# else
    "rgb:fa/eb/d7",             // 7: white             (AntiqueWhite)
# endif
    /* high-intensity colors */
# ifdef XTERM_COLORS
    "rgb:4d/4d/4d",             // 8: bright black      (Grey30)
# else
    "rgb:40/40/40",             // 8: bright black      (Grey25)
# endif
    "rgb:ff/00/00",             // 1/9: bright red      (Reed)
    "rgb:00/ff/00",             // 2/10: bright green   (Green)
    "rgb:ff/ff/00",             // 3/11: bright yellow  (Yellow)
    "rgb:00/00/ff",             // 4/12: bright blue    (Blue)
    "rgb:ff/00/ff",             // 5/13: bright magenta (Magenta)
    "rgb:00/ff/ff",             // 6/14: bright cyan    (Cyan)
    "rgb:ff/ff/ff",             // 7/15: bright white   (White)

#if !USE_256_COLORS
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
#else
    // 256 xterm colours
    "rgb:00/00/00",
    "rgb:00/00/5f",
    "rgb:00/00/87",
    "rgb:00/00/af",
    "rgb:00/00/d7",
    "rgb:00/00/ff",
    "rgb:00/5f/00",
    "rgb:00/5f/5f",
    "rgb:00/5f/87",
    "rgb:00/5f/af",
    "rgb:00/5f/d7",
    "rgb:00/5f/ff",
    "rgb:00/87/00",
    "rgb:00/87/5f",
    "rgb:00/87/87",
    "rgb:00/87/af",
    "rgb:00/87/d7",
    "rgb:00/87/ff",
    "rgb:00/af/00",
    "rgb:00/af/5f",
    "rgb:00/af/87",
    "rgb:00/af/af",
    "rgb:00/af/d7",
    "rgb:00/af/ff",
    "rgb:00/d7/00",
    "rgb:00/d7/5f",
    "rgb:00/d7/87",
    "rgb:00/d7/af",
    "rgb:00/d7/d7",
    "rgb:00/d7/ff",
    "rgb:00/ff/00",
    "rgb:00/ff/5f",
    "rgb:00/ff/87",
    "rgb:00/ff/af",
    "rgb:00/ff/d7",
    "rgb:00/ff/ff",
    "rgb:5f/00/00",
    "rgb:5f/00/5f",
    "rgb:5f/00/87",
    "rgb:5f/00/af",
    "rgb:5f/00/d7",
    "rgb:5f/00/ff",
    "rgb:5f/5f/00",
    "rgb:5f/5f/5f",
    "rgb:5f/5f/87",
    "rgb:5f/5f/af",
    "rgb:5f/5f/d7",
    "rgb:5f/5f/ff",
    "rgb:5f/87/00",
    "rgb:5f/87/5f",
    "rgb:5f/87/87",
    "rgb:5f/87/af",
    "rgb:5f/87/d7",
    "rgb:5f/87/ff",
    "rgb:5f/af/00",
    "rgb:5f/af/5f",
    "rgb:5f/af/87",
    "rgb:5f/af/af",
    "rgb:5f/af/d7",
    "rgb:5f/af/ff",
    "rgb:5f/d7/00",
    "rgb:5f/d7/5f",
    "rgb:5f/d7/87",
    "rgb:5f/d7/af",
    "rgb:5f/d7/d7",
    "rgb:5f/d7/ff",
    "rgb:5f/ff/00",
    "rgb:5f/ff/5f",
    "rgb:5f/ff/87",
    "rgb:5f/ff/af",
    "rgb:5f/ff/d7",
    "rgb:5f/ff/ff",
    "rgb:87/00/00",
    "rgb:87/00/5f",
    "rgb:87/00/87",
    "rgb:87/00/af",
    "rgb:87/00/d7",
    "rgb:87/00/ff",
    "rgb:87/5f/00",
    "rgb:87/5f/5f",
    "rgb:87/5f/87",
    "rgb:87/5f/af",
    "rgb:87/5f/d7",
    "rgb:87/5f/ff",
    "rgb:87/87/00",
    "rgb:87/87/5f",
    "rgb:87/87/87",
    "rgb:87/87/af",
    "rgb:87/87/d7",
    "rgb:87/87/ff",
    "rgb:87/af/00",
    "rgb:87/af/5f",
    "rgb:87/af/87",
    "rgb:87/af/af",
    "rgb:87/af/d7",
    "rgb:87/af/ff",
    "rgb:87/d7/00",
    "rgb:87/d7/5f",
    "rgb:87/d7/87",
    "rgb:87/d7/af",
    "rgb:87/d7/d7",
    "rgb:87/d7/ff",
    "rgb:87/ff/00",
    "rgb:87/ff/5f",
    "rgb:87/ff/87",
    "rgb:87/ff/af",
    "rgb:87/ff/d7",
    "rgb:87/ff/ff",
    "rgb:af/00/00",
    "rgb:af/00/5f",
    "rgb:af/00/87",
    "rgb:af/00/af",
    "rgb:af/00/d7",
    "rgb:af/00/ff",
    "rgb:af/5f/00",
    "rgb:af/5f/5f",
    "rgb:af/5f/87",
    "rgb:af/5f/af",
    "rgb:af/5f/d7",
    "rgb:af/5f/ff",
    "rgb:af/87/00",
    "rgb:af/87/5f",
    "rgb:af/87/87",
    "rgb:af/87/af",
    "rgb:af/87/d7",
    "rgb:af/87/ff",
    "rgb:af/af/00",
    "rgb:af/af/5f",
    "rgb:af/af/87",
    "rgb:af/af/af",
    "rgb:af/af/d7",
    "rgb:af/af/ff",
    "rgb:af/d7/00",
    "rgb:af/d7/5f",
    "rgb:af/d7/87",
    "rgb:af/d7/af",
    "rgb:af/d7/d7",
    "rgb:af/d7/ff",
    "rgb:af/ff/00",
    "rgb:af/ff/5f",
    "rgb:af/ff/87",
    "rgb:af/ff/af",
    "rgb:af/ff/d7",
    "rgb:af/ff/ff",
    "rgb:d7/00/00",
    "rgb:d7/00/5f",
    "rgb:d7/00/87",
    "rgb:d7/00/af",
    "rgb:d7/00/d7",
    "rgb:d7/00/ff",
    "rgb:d7/5f/00",
    "rgb:d7/5f/5f",
    "rgb:d7/5f/87",
    "rgb:d7/5f/af",
    "rgb:d7/5f/d7",
    "rgb:d7/5f/ff",
    "rgb:d7/87/00",
    "rgb:d7/87/5f",
    "rgb:d7/87/87",
    "rgb:d7/87/af",
    "rgb:d7/87/d7",
    "rgb:d7/87/ff",
    "rgb:d7/af/00",
    "rgb:d7/af/5f",
    "rgb:d7/af/87",
    "rgb:d7/af/af",
    "rgb:d7/af/d7",
    "rgb:d7/af/ff",
    "rgb:d7/d7/00",
    "rgb:d7/d7/5f",
    "rgb:d7/d7/87",
    "rgb:d7/d7/af",
    "rgb:d7/d7/d7",
    "rgb:d7/d7/ff",
    "rgb:d7/ff/00",
    "rgb:d7/ff/5f",
    "rgb:d7/ff/87",
    "rgb:d7/ff/af",
    "rgb:d7/ff/d7",
    "rgb:d7/ff/ff",
    "rgb:ff/00/00",
    "rgb:ff/00/5f",
    "rgb:ff/00/87",
    "rgb:ff/00/af",
    "rgb:ff/00/d7",
    "rgb:ff/00/ff",
    "rgb:ff/5f/00",
    "rgb:ff/5f/5f",
    "rgb:ff/5f/87",
    "rgb:ff/5f/af",
    "rgb:ff/5f/d7",
    "rgb:ff/5f/ff",
    "rgb:ff/87/00",
    "rgb:ff/87/5f",
    "rgb:ff/87/87",
    "rgb:ff/87/af",
    "rgb:ff/87/d7",
    "rgb:ff/87/ff",
    "rgb:ff/af/00",
    "rgb:ff/af/5f",
    "rgb:ff/af/87",
    "rgb:ff/af/af",
    "rgb:ff/af/d7",
    "rgb:ff/af/ff",
    "rgb:ff/d7/00",
    "rgb:ff/d7/5f",
    "rgb:ff/d7/87",
    "rgb:ff/d7/af",
    "rgb:ff/d7/d7",
    "rgb:ff/d7/ff",
    "rgb:ff/ff/00",
    "rgb:ff/ff/5f",
    "rgb:ff/ff/87",
    "rgb:ff/ff/af",
    "rgb:ff/ff/d7",
    "rgb:ff/ff/ff",
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
#endif /* ! NO_CURSORCOLOR */
    NULL,                       /* Color_pointer_fg               */
    NULL,                       /* Color_pointer_bg               */
    NULL,                       /* Color_border                   */
#ifndef NO_BOLD_UNDERLINE_REVERSE
    NULL,                       /* Color_BD                       */
    NULL,                       /* Color_IT                       */
    NULL,                       /* Color_UL                       */
    NULL,                       /* Color_RV                       */
#endif /* ! NO_BOLD_UNDERLINE_REVERSE */
#if ENABLE_FRILLS
    NULL,			// Color_underline
#endif
#ifdef OPTION_HC
    NULL,
    NULL,
#endif
    COLOR_SCROLLBAR,
#ifdef RXVT_SCROLLBAR
    COLOR_SCROLLTROUGH,
#endif
#if ENABLE_TRANSPARENCY
    NULL,
#endif
#if OFF_FOCUS_FADING
    "rgb:00/00/00",
#endif
  };

void
rxvt_term::init_vars ()
{
  pix_colors = pix_colors_focused;

  MEvent.time = CurrentTime;
  MEvent.button = AnyButton;
  want_refresh = 1;
  priv_modes = SavedModes = PrivMode_Default;
  ncol = 80;
  nrow = 24;
  int_bwidth = INTERNALBORDERWIDTH;
  ext_bwidth = EXTERNALBORDERWIDTH;
  lineSpace = LINESPACE;
  letterSpace = LETTERSPACE;
  saveLines = SAVELINES;

  refresh_type = SLOW_REFRESH;

  oldcursor.row = oldcursor.col = -1;

  set_option (Opt_scrollBar);
  set_option (Opt_scrollTtyOutput);
  set_option (Opt_jumpScroll);
  set_option (Opt_skipScroll);
  set_option (Opt_secondaryScreen);
  set_option (Opt_secondaryScroll);
  set_option (Opt_pastableTabs);
  set_option (Opt_intensityStyles);
  set_option (Opt_iso14755);
  set_option (Opt_iso14755_52);
  set_option (Opt_buffered);
}

/*----------------------------------------------------------------------*/
const char **
rxvt_term::init_resources (int argc, const char *const *argv)
{
  int i;
  const char **cmd_argv;

  rs[Rs_name] = rxvt_basename (argv[0]);

  /*
   * Open display, get options/resources and create the window
   */

  if ((rs[Rs_display_name] = getenv ("DISPLAY")) == NULL)
    rs[Rs_display_name] = ":0";

  cmd_argv = get_options (argc, argv);

  if (!(display = displays.get (rs[Rs_display_name])))
    rxvt_fatal ("can't open display %s, aborting.\n", rs[Rs_display_name]);

  // using a local pointer decreases code size a lot
  xa = display->xa;

  set (display);
  extract_resources ();

#if XFT
  if (rs[Rs_depth])
    select_visual (strtol (rs[Rs_depth], 0, 0));
#endif

  for (int i = NUM_RESOURCES; i--; )
    if (rs [i] == resval_undef)
      rs [i] = 0;

#if ENABLE_PERL
  if (!rs[Rs_perl_ext_1])
    rs[Rs_perl_ext_1] = "default";

  if ((rs[Rs_perl_ext_1] && *rs[Rs_perl_ext_1])
      || (rs[Rs_perl_ext_2] && *rs[Rs_perl_ext_2])
      || (rs[Rs_perl_eval] && *rs[Rs_perl_eval]))
    {
      rxvt_perl.init (this);
      HOOK_INVOKE ((this, HOOK_INIT, DT_END));
    }
#endif

  // must be called after initialising the perl interpreter as it
  // may invoke the `on_register_command' hook
  extract_keysym_resources ();

  /*
   * set any defaults not already set
   */
  if (cmd_argv && cmd_argv[0])
    {
      if (!rs[Rs_title])
        rs[Rs_title] = rxvt_basename (cmd_argv[0]);

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

  if (rs[Rs_letterSpace])
    letterSpace = atoi (rs[Rs_letterSpace]);
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

  if (!rs[Rs_cutchars])
    rs[Rs_cutchars] = CUTCHARS;

#ifndef NO_BACKSPACE_KEY
  if (!rs[Rs_backspace_key])
# ifdef DEFAULT_BACKSPACE
    rs[Rs_backspace_key] = DEFAULT_BACKSPACE;
# else
    rs[Rs_backspace_key] = "DEC";       /* can toggle between \010 or \177 */
# endif
#endif

#ifndef NO_DELETE_KEY
  if (!rs[Rs_delete_key])
# ifdef DEFAULT_DELETE
    rs[Rs_delete_key] = DEFAULT_DELETE;
# else
    rs[Rs_delete_key] = "\033[3~";
# endif
#endif

    scrollBar.setup (this);

#ifdef XTERM_REVERSE_VIDEO
  /* this is how xterm implements reverseVideo */
  if (option (Opt_reverseVideo))
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
  if (option (Opt_reverseVideo))
    ::swap (rs[Rs_color + Color_fg], rs[Rs_color + Color_bg]);
#endif

  /* convenient aliases for setting fg/bg to colors */
  color_aliases (Color_fg);
  color_aliases (Color_bg);
#ifndef NO_CURSORCOLOR
  color_aliases (Color_cursor);
  color_aliases (Color_cursor2);
#endif /* NO_CURSORCOLOR */
  color_aliases (Color_pointer_fg);
  color_aliases (Color_pointer_bg);
  color_aliases (Color_border);
#ifndef NO_BOLD_UNDERLINE_REVERSE
  color_aliases (Color_BD);
  color_aliases (Color_UL);
  color_aliases (Color_RV);
#endif /* ! NO_BOLD_UNDERLINE_REVERSE */

  if (!rs[Rs_color + Color_border])
    rs[Rs_color + Color_border] = rs[Rs_color + Color_bg];

  return cmd_argv;
}

/*----------------------------------------------------------------------*/
void
rxvt_term::init (stringvec *argv, stringvec *envv)
{
  argv->push_back (0);
  envv->push_back (0);

  this->argv = argv;
  this->envv = envv;

  init2 (argv->size () - 1, argv->begin ());
}

void
rxvt_term::init (int argc, const char *const *argv, const char *const *envv)
{
#if ENABLE_PERL
  // perl might want to access the stringvecs later, so we need to copy them
  stringvec *args = new stringvec;
  for (int i = 0; i < argc; i++)
    args->push_back (strdup (argv [i]));

  stringvec *envs = new stringvec;
  for (const char *const *var = envv; *var; var++)
    envs->push_back (strdup (*var));

  init (args, envs);
#else
  init2 (argc, argv);
#endif
}

void
rxvt_term::init2 (int argc, const char *const *argv)
{
  SET_R (this);
  set_locale ("");
  set_environ (envv); // a few things in X do not call setlocale :(

  init_vars ();

  const char **cmd_argv = init_resources (argc, argv);

#ifdef KEYSYM_RESOURCE
  keyboard->register_done ();
#endif

  if (const char *path = rs[Rs_chdir])
    if (*path) // ignored if empty
      {
        if (*path != '/')
          rxvt_fatal ("specified shell working directory must start with a slash, aborting.\n");

        if (chdir (path))
          rxvt_fatal ("unable to change into specified shell working directory, aborting.\n");
      }

  if (option (Opt_scrollBar))
    scrollBar.state = SB_STATE_IDLE;    /* set existence for size calculations */

  pty = ptytty::create ();

#ifdef HAVE_AFTERIMAGE
  set_application_name ((char *)rs[Rs_name]);
  set_output_threshold (OUTPUT_LEVEL_WARNING);
#endif

  // must be called before create_windows, because the latter may call set_icon
#ifdef HAVE_PIXBUF
  g_type_init ();
#endif

  create_windows (argc, argv);

  init_xlocale ();

  scr_poweron (); // initialize screen

#if 0
  XSynchronize (dpy, True);
#endif

  if (option (Opt_scrollBar))
    scrollBar.resize ();      /* create and map scrollbar */
#ifdef HAVE_BG_PIXMAP
  {
    bg_init ();

#ifdef ENABLE_TRANSPARENCY
    if (option (Opt_transparent))
      {
        bg_set_transparent ();

        if (rs [Rs_blurradius])
          bg_set_blur (rs [Rs_blurradius]);

        if (ISSET_PIXCOLOR (Color_tint))
          bg_set_tint (pix_colors_focused [Color_tint]);

        if (rs [Rs_shade])
          bg_set_shade (rs [Rs_shade]);

        bg_set_root_pixmap ();
        XSelectInput (dpy, display->root, PropertyChangeMask);
        rootwin_ev.start (display, display->root);
      }
#endif

#ifdef BG_IMAGE_FROM_FILE
    if (rs[Rs_backgroundPixmap])
      if (bg_set_file (rs[Rs_backgroundPixmap]))
        if (!bg_window_position_sensitive ())
          update_background ();
#endif
  }
#endif

#if ENABLE_PERL
  rootwin_ev.start (display, display->root);
#endif

  init_done = 1;

  init_command (cmd_argv);

  if (pty->pty >= 0)
    pty_ev.start (pty->pty, ev::READ);

  HOOK_INVOKE ((this, HOOK_START, DT_END));

#if ENABLE_XEMBED
  if (rs[Rs_embed])
    {
      long info[2] = { 0, XEMBED_MAPPED };

      XChangeProperty (dpy, parent, xa[XA_XEMBED_INFO], xa[XA_XEMBED_INFO],
                       32, PropModeReplace, (unsigned char *)&info, 2);
    }
#endif

#if HAVE_STARTUP_NOTIFICATION
  SnDisplay *snDisplay;
  SnLauncheeContext *snContext;

  snDisplay = sn_display_new (dpy, NULL, NULL);
  snContext = sn_launchee_context_new_from_environment (snDisplay, DefaultScreen (dpy));

  /* Tell the window manager that this window is part of the startup context */
  if (snContext)
    sn_launchee_context_setup_window (snContext, parent);
#endif

  XMapWindow (dpy, vt);
  XMapWindow (dpy, parent);

#if HAVE_STARTUP_NOTIFICATION
  if (snContext)
    {
      /* Mark the startup process as complete */
      sn_launchee_context_complete (snContext);

      sn_launchee_context_unref (snContext);
    }

  sn_display_unref (snDisplay);
#endif

  refresh_check ();
}

/*----------------------------------------------------------------------*/
void
rxvt_term::init_env ()
{
  char *val;
  char *env_display;
  char *env_windowid;
  char *env_colorfgbg;
  char *env_term;

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
#endif /* DISPLAY_IS_IP */
    val = XDisplayString (dpy);

  if (rs[Rs_display_name] == NULL)
    rs[Rs_display_name] = val;   /* use broken `:0' value */

  env_display = (char *)rxvt_malloc (strlen (val) + 9);

  sprintf (env_display, "DISPLAY=%s", val);

  env_windowid = (char *)rxvt_malloc (21);
  sprintf (env_windowid, "WINDOWID=%lu", (unsigned long)parent);

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

  env_colorfgbg = get_colorfgbg ();
  putenv (env_colorfgbg);

#ifdef RXVT_TERMINFO
  putenv ("TERMINFO=" RXVT_TERMINFO);
#endif

  if (depth <= 2)
    putenv ("COLORTERM=" COLORTERMENV "-mono");
  else
    putenv ("COLORTERM=" COLORTERMENVFULL);

  if (rs[Rs_term_name] != NULL)
    {
      env_term = (char *)rxvt_malloc (strlen (rs[Rs_term_name]) + 6);
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
#endif /* HAVE_UNSETENV */
}

/*----------------------------------------------------------------------*/
void
rxvt_term::set_locale (const char *locale)
{
  set_environ (envv);

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


  this->locale = strdup (this->locale);
  SET_LOCALE (this->locale);
  mbstate.reset ();

#if HAVE_NL_LANGINFO
  char *codeset = nl_langinfo (CODESET);
  // /^UTF.?8/i
  enc_utf8 = (codeset[0] == 'U' || codeset[0] == 'u')
          && (codeset[1] == 'T' || codeset[1] == 't')
          && (codeset[2] == 'F' || codeset[2] == 'f')
          && (codeset[3] == '8' || codeset[4] == '8');
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
    rxvt_warn ("setting locale failed, continuing without locale support.\n");
  else
    {
      set_string_property (xa[XA_WM_LOCALE_NAME], locale);

      if (!XSupportsLocale ())
        {
          rxvt_warn ("the locale is not supported by Xlib, continuing without locale support.\n");
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
  meta_char = option (Opt_meta8) ? 0x80 : C0_ESC;
#endif

  get_ourmods ();

  if (!option (Opt_scrollTtyOutput))
    priv_modes |= PrivMode_TtyOutputInh;
  if (option (Opt_scrollTtyKeypress))
    priv_modes |= PrivMode_Keypress;
  if (!option (Opt_jumpScroll))
    priv_modes |= PrivMode_smoothScroll;

#ifndef NO_BACKSPACE_KEY
  if (strcmp (rs[Rs_backspace_key], "DEC") == 0)
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
rxvt_term::get_colours ()
{
  int i;

#ifdef OFF_FOCUS_FADING
  pix_colors = pix_colors_focused;
#endif

  for (i = 0; i < (depth <= 2 ? 2 : NRS_COLORS); i++)
    {
      const char *name = rs[Rs_color + i];

      if (!name)
        continue;

      rxvt_color xcol;

      if (!set_color (xcol, name))
        {
#ifndef XTERM_REVERSE_VIDEO
          if (i < 2 && option (Opt_reverseVideo))
            name = def_colorName [1 - i];
          else
#endif
            name = def_colorName [i];

          if (!name)
            continue;

          if (!set_color (xcol, name))
            {
              switch (i)
                {
                  case Color_fg:
                  case Color_bg:
                    rxvt_warn ("unable to get foreground/background colour, continuing.\n");
                    name = "";
                    break;
#ifndef NO_CURSORCOLOR
                  case Color_cursor2:
#endif
                  case Color_pointer_fg:
                    name = rs[Rs_color + Color_fg];
                    xcol.set (this, name);
                    break;
                  default:
                    name = rs[Rs_color + Color_bg];
                    xcol.set (this, name);
                    break;
                }
            }
        }

      pix_colors[i] = xcol;
      rs[Rs_color + i] = name;
    }

  if (depth <= 2)
    {
      if (!rs[Rs_color + Color_pointer_fg]) alias_color (Color_pointer_fg, Color_fg);
      if (!rs[Rs_color + Color_pointer_bg]) alias_color (Color_pointer_bg, Color_bg);
      if (!rs[Rs_color + Color_border]    ) alias_color (Color_border,     Color_fg);
    }

  /*
   * get scrollBar shadow colors
   *
   * The calculations of topShadow/bottomShadow values are adapted
   * from the fvwm window manager.
   */
#ifdef RXVT_SCROLLBAR
  if (depth <= 2)
    {
      /* Monochrome */
      alias_color (Color_scroll,       Color_fg);
      alias_color (Color_topShadow,    Color_bg);
      alias_color (Color_bottomShadow, Color_bg);
    }
  else
    {
      pix_colors [Color_scroll].fade (this, 50, pix_colors [Color_bottomShadow]);

      rgba cscroll;
      pix_colors [Color_scroll].get (cscroll);

      /* topShadowColor */
      if (!pix_colors[Color_topShadow].set (this,
                       rgba (
                         min ((int)rgba::MAX_CC, max (cscroll.r / 5, cscroll.r) * 7 / 5),
                         min ((int)rgba::MAX_CC, max (cscroll.g / 5, cscroll.g) * 7 / 5),
                         min ((int)rgba::MAX_CC, max (cscroll.b / 5, cscroll.b) * 7 / 5),
                         cscroll.a)
                       ))
        alias_color (Color_topShadow, Color_White);
    }
#endif

#ifdef OFF_FOCUS_FADING
  for (i = 0; i < (depth <= 2 ? 2 : NRS_COLORS); i++)
    update_fade_color (i);
#endif
}

/*----------------------------------------------------------------------*/
/* color aliases, fg/bg bright-bold */
void
rxvt_term::color_aliases (int idx)
{
  if (rs[Rs_color + idx] && isdigit (*rs[Rs_color + idx]))
    {
      int i = atoi (rs[Rs_color + idx]);

      if (i >= 8 && i <= 15)
        /* bright colors */
        rs[Rs_color + idx] = rs[Rs_color + minBrightCOLOR + i - 8];
      else if (i >= 0 && i <= 7)
        /* normal colors */
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

  map = XGetModifierMapping (dpy);
  kc = map->modifiermap;

  for (i = 1; i < 6; i++)
    {
      k = (i + 2) * map->max_keypermod;       /* skip shift/lock/control */

      for (j = map->max_keypermod; j--; k++)
        {
          if (kc[k] == 0)
            break;

          switch (XKeycodeToKeysym (dpy, kc[k], 0))
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

void
rxvt_term::set_icon (const char *file)
{
#ifdef HAVE_AFTERIMAGE
  init_asv ();

  ASImage *im = file2ASImage (file, 0xFFFFFFFF, SCREEN_GAMMA, 0, NULL);
  if (!im)
    {
      rxvt_warn ("Loading image icon failed, continuing without.\n");
      return;
    }

  unsigned int w = im->width;
  unsigned int h = im->height;

  if (!IN_RANGE_INC (w, 1, 16383) || !IN_RANGE_INC (h, 1, 16383))
    {
      rxvt_warn ("Icon image too big, continuing without.\n");
      destroy_asimage (&im);
      return;
    }

  ASImage *result = scale_asimage (asv, im,
                                   w, h, ASA_ARGB32,
                                   100, ASIMAGE_QUALITY_DEFAULT);
  destroy_asimage (&im);

  if (!result)
    {
      rxvt_warn ("Icon image transformation to ARGB failed, continuing without.\n");
      return;
    }

  long *buffer = (long *)malloc ((2 + w * h) * sizeof (long));
  if (buffer)
    {
      ARGB32 *asbuf = result->alt.argb32;
      buffer [0] = w;
      buffer [1] = h;

      for (unsigned int i = 0; i < w * h; ++i)
        buffer [i + 2] = asbuf [i];

      XChangeProperty (dpy, parent, xa[XA_NET_WM_ICON], XA_CARDINAL, 32,
                       PropModeReplace, (const unsigned char *) buffer, 2 + w * h);
      free (buffer);
    }
  else
    rxvt_warn ("Memory allocation for icon hint failed, continuing without.\n");

  destroy_asimage (&result);
#endif

#ifdef HAVE_PIXBUF
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (file, NULL);
  if (!pixbuf)
    {
      rxvt_warn ("Loading image icon failed, continuing without.\n");
      return;
    }

  unsigned int w = gdk_pixbuf_get_width (pixbuf);
  unsigned int h = gdk_pixbuf_get_height (pixbuf);

  if (!IN_RANGE_INC (w, 1, 16383) || !IN_RANGE_INC (h, 1, 16383))
    {
      rxvt_warn ("Icon image too big, continuing without.\n");
      g_object_unref (pixbuf);
      return;
    }

  if (long *buffer = (long *)malloc ((2 + w * h) * sizeof (long)))
    {
      int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
      unsigned char *row = gdk_pixbuf_get_pixels (pixbuf);
      int channels = gdk_pixbuf_get_n_channels (pixbuf);

      buffer [0] = w;
      buffer [1] = h;
      for (int i = 0; i < h; i++)
        {
          for (int j = 0; j < w; j++)
            {
              unsigned char *pixel = row + j * channels;
              long value;

              if (channels == 4)
                value = pixel[3];
              else
                value = (unsigned char)0x00ff;

              value = (value << 8) + pixel[0];
              value = (value << 8) + pixel[1];
              value = (value << 8) + pixel[2];
              buffer[(i * w + j) + 2] = value;
            }

          row += rowstride;
        }

      XChangeProperty (dpy, parent, xa[XA_NET_WM_ICON], XA_CARDINAL, 32,
                       PropModeReplace, (const unsigned char *) buffer, 2 + w * h);
      free (buffer);
    }
  else
    rxvt_warn ("Memory allocation for icon hint failed, continuing without.\n");

  g_object_unref (pixbuf);
#endif
}

/*----------------------------------------------------------------------*/
/* Open and map the window */
void
rxvt_term::create_windows (int argc, const char *const *argv)
{
  XClassHint classHint;
  XWMHints wmHint;
#if ENABLE_FRILLS
  MWMHints mwmhints = { };
#endif
  XGCValues gcvalue;
  XSetWindowAttributes attributes;
  Window top, parent;

  dLocal (Display *, dpy);

  /* grab colors before netscape does */
  get_colours ();

  if (!set_fonts ())
    rxvt_fatal ("unable to load base fontset, please specify a valid one using -fn, aborting.\n");

  parent = display->root;

  attributes.override_redirect = !!option (Opt_override_redirect);

#if ENABLE_FRILLS
  if (option (Opt_borderLess))
    {
      if (XInternAtom (dpy, "_MOTIF_WM_INFO", True) == None)
        {
          // rxvt_warn("Window Manager does not support MWM hints.  Bypassing window manager control for borderless window.\n");
          attributes.override_redirect = true;
        }
      else
        {
          mwmhints.flags = MWM_HINTS_DECORATIONS;
        }
    }
#endif

#if ENABLE_XEMBED
  if (rs[Rs_embed])
    {
      XWindowAttributes wattr;

      parent = strtol (rs[Rs_embed], 0, 0);

      if (!XGetWindowAttributes (dpy, parent, &wattr))
        rxvt_fatal ("invalid window-id specified with -embed, aborting.\n");

      window_calc (wattr.width, wattr.height);
    }
#endif

  window_calc (0, 0);

  /* sub-window placement & size in rxvt_term::resize_all_windows () */
  attributes.background_pixel = pix_colors_focused [Color_border];
  attributes.border_pixel     = pix_colors_focused [Color_border];
  attributes.colormap         = cmap;

  top = XCreateWindow (dpy, parent,
                       szHint.x, szHint.y,
                       szHint.width, szHint.height,
                       ext_bwidth,
                       depth, InputOutput, visual,
                       CWColormap | CWBackPixel | CWBorderPixel | CWOverrideRedirect,
                       &attributes);

  this->parent = top;

  set_title     (rs [Rs_title]);
  set_icon_name (rs [Rs_iconName]);

  classHint.res_name  = (char *)rs[Rs_name];
  classHint.res_class = (char *)RESCLASS;

  wmHint.flags         = InputHint | StateHint | WindowGroupHint;
  wmHint.input         = True;
  wmHint.initial_state = option (Opt_iconic) ? IconicState : NormalState;
  wmHint.window_group  = top;

  XmbSetWMProperties (dpy, top, NULL, NULL, (char **)argv, argc,
                      &szHint, &wmHint, &classHint);
#if ENABLE_EWMH
  /*
   * set up icon hint
   * rs [Rs_iconfile] is path to icon
   */

  if (rs [Rs_iconfile])
    {
      set_icon (rs [Rs_iconfile]);
    }
#endif

#if ENABLE_FRILLS
  if (mwmhints.flags)
    XChangeProperty (dpy, top, xa[XA_MOTIF_WM_HINTS], xa[XA_MOTIF_WM_HINTS], 32,
                     PropModeReplace, (unsigned char *)&mwmhints, PROP_MWM_HINTS_ELEMENTS);
#endif

  Atom protocols[] = {
    xa[XA_WM_DELETE_WINDOW],
#if ENABLE_EWMH
    xa[XA_NET_WM_PING],
#endif
  };

  XSetWMProtocols (dpy, top, protocols, ecb_array_length (protocols));

#if ENABLE_FRILLS
  if (rs[Rs_transient_for])
    XSetTransientForHint (dpy, top, (Window)strtol (rs[Rs_transient_for], 0, 0));
#endif

#if ENABLE_EWMH
  long pid = getpid ();

  XChangeProperty (dpy, top,
                   xa[XA_NET_WM_PID], XA_CARDINAL, 32,
                   PropModeReplace, (unsigned char *)&pid, 1);

  // _NET_WM_WINDOW_TYPE is NORMAL, which is the default
#endif

  XSelectInput (dpy, top,
                KeyPressMask
#if (MOUSE_WHEEL && MOUSE_SLIP_WHEELING) || ENABLE_FRILLS || ISO_14755
                | KeyReleaseMask
#endif
                | FocusChangeMask | VisibilityChangeMask
                | ExposureMask | StructureNotifyMask);

  termwin_ev.start (display, top);

  /* vt cursor: Black-on-White is standard, but this is more popular */
  TermWin_cursor = XCreateFontCursor (dpy, XC_xterm);

  /* the vt window */
  vt = XCreateSimpleWindow (dpy, top,
                            window_vt_x, window_vt_y,
                            vt_width, vt_height,
                            0,
                            pix_colors_focused[Color_fg],
                            pix_colors_focused[Color_bg]);

  attributes.bit_gravity = NorthWestGravity;
  XChangeWindowAttributes (dpy, vt, CWBitGravity, &attributes);

  vt_emask = ExposureMask | ButtonPressMask | ButtonReleaseMask | PropertyChangeMask;

  if (option (Opt_pointerBlank))
    vt_emask |= PointerMotionMask;
  else
    vt_emask |= Button1MotionMask | Button3MotionMask;

  vt_select_input ();

  vt_ev.start (display, vt);

  /* graphics context for the vt window */
  gcvalue.foreground         = pix_colors[Color_fg];
  gcvalue.background         = pix_colors[Color_bg];
  gcvalue.graphics_exposures = 0;

  gc = XCreateGC (dpy, vt,
                  GCForeground | GCBackground | GCGraphicsExposures,
                  &gcvalue);

  drawable = new rxvt_drawable (this, vt);

#ifdef OFF_FOCUS_FADING
  // initially we are in unfocused state
  if (rs[Rs_fade])
    pix_colors = pix_colors_unfocused;
#endif

  pointer_unblank ();
  scr_recolour ();
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
      pty->pty = atoi (rs[Rs_pty_fd]);

      if (pty->pty >= 0)
        {
          if (getfd_hook)
            pty->pty = (*getfd_hook) (pty->pty);

          if (pty->pty < 0)
            rxvt_fatal ("unusable pty-fd filehandle, aborting.\n");
        }
    }
  else
#endif
    if (!pty->get ())
      rxvt_fatal ("can't initialize pseudo-tty, aborting.\n");

  fcntl (pty->pty, F_SETFL, O_NONBLOCK);

  struct termios tio = def_tio;

#ifndef NO_BACKSPACE_KEY
  if (rs[Rs_backspace_key][0] && !rs[Rs_backspace_key][1])
    tio.c_cc[VERASE] = rs[Rs_backspace_key][0];
  else if (strcmp (rs[Rs_backspace_key], "DEC") == 0)
    tio.c_cc[VERASE] = '\177';            /* the initial state anyway */
#endif

  /* init terminal attributes */
  cfsetospeed (&tio, BAUDRATE);
  cfsetispeed (&tio, BAUDRATE);
  tcsetattr (pty->tty, TCSANOW, &tio);
  pty->set_utf8_mode (enc_utf8);

  /* set initial window size */
  tt_winch ();

#if ENABLE_FRILLS
  if (rs[Rs_pty_fd])
    return;
#endif

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

        if (!pty->make_controlling_tty ())
          fprintf (stderr, "%s: could not obtain control of tty.", RESNAME);
        else
          {
            /* Reopen stdin, stdout and stderr over the tty file descriptor */
            dup2 (pty->tty, STDIN_FILENO);
            dup2 (pty->tty, STDOUT_FILENO);
            dup2 (pty->tty, STDERR_FILENO);

            // close all our file handles that we do no longer need
            for (rxvt_term **t = termlist.begin (); t < termlist.end (); t++)
              {
                if ((*t)->pty->pty > 2) close ((*t)->pty->pty);
                if ((*t)->pty->tty > 2) close ((*t)->pty->tty);
              }

            run_child (argv);
            fprintf (stderr, "%s: unable to exec child.", RESNAME);
          }

        _exit (EXIT_FAILURE);

      default:
        if (!option (Opt_utmpInhibit))
          {
#ifdef LOG_ONLY_ON_LOGIN
            bool login_shell = option (Opt_loginShell);
#else
            bool login_shell = true;
#endif
            pty->login (cmd_pid, login_shell, rs[Rs_display_name]);
          }

        pty->close_tty ();

        child_ev.start (cmd_pid);

        HOOK_INVOKE ((this, HOOK_CHILD_START, DT_INT, cmd_pid, DT_END));
        break;
    }
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

  if (option (Opt_console))
    {
      /* be virtual console, fail silently */
#ifdef TIOCCONS
      unsigned int on = 1;

      ioctl (STDIN_FILENO, TIOCCONS, &on);
#elif defined (SRIOCSREDIR)
      int fd;

      fd = open (CONSOLE, O_WRONLY, 0);
      if (fd >= 0)
        {
          ioctl (fd, SRIOCSREDIR, STDIN_FILENO);
          close (fd);
        }
#endif /* SRIOCSREDIR */
    }

  /* reset signals and spin off the command interpreter */
  signal (SIGINT,  SIG_DFL);
  signal (SIGQUIT, SIG_DFL);
  signal (SIGCHLD, SIG_DFL);
  signal (SIGHUP,  SIG_DFL);
  signal (SIGPIPE, SIG_DFL);
  /*
   * mimic login's behavior by disabling the job control signals
   * a shell that wants them can turn them back on
   */
#ifdef SIGTSTP
  signal (SIGTSTP, SIG_IGN);
  signal (SIGTTIN, SIG_IGN);
  signal (SIGTTOU, SIG_IGN);
#endif /* SIGTSTP */

  /* command interpreter path */
  if (argv)
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

      argv0 = rxvt_basename (shell);

      if (option (Opt_loginShell))
        {
          login = (char *)rxvt_malloc (strlen (argv0) + 2);

          login[0] = '-';
          strcpy (&login[1], argv0);
          argv0 = login;
        }

      execlp (shell, argv0, (char *)0);
      /* no error message: STDERR is closed! */
    }

  return -1;
}

/*----------------------- end-of-file (C source) -----------------------*/
