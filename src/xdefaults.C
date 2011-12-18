/*----------------------------------------------------------------------*
 * File:	xdefaults.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *				- original version
 * Copyright (c) 1997,1998 mj olesen <olesen@me.queensu.ca>
 * Copyright (c) 2003-2006 Marc Lehmann <schmorp@schmorp.de>
 * Copyright (c) 2007      Emanuele Giaquinta <e.giaquinta@glauco.it>
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

#include "../config.h"
#include "rxvt.h"
#include "version.h"

#ifdef KEYSYM_RESOURCE
# include "keyboard.h"
#endif

/* place holders used for parsing command-line options */
#define Optflag_Reverse              1
#define Optflag_Boolean              2
#define Optflag_Switch               4
#define Optflag_Info                 8

/* monolithic option/resource structure: */
/*
 * `string' options MUST have a usage argument
 * `switch' and `boolean' options have no argument
 * if there's no desc (ription), it won't appear in rxvt_usage ()
 */

/* INFO () - descriptive information only */
#define INFO(opt, arg, desc)					\
    {0, Optflag_Info, -1, NULL, (opt), (arg), (desc)}

#define RINFO(kw, arg)						\
    {0, Optflag_Info, -1, (kw), NULL, (arg), NULL}

/* STRG () - command-line option, with/without resource */
#define STRG(rsp, kw, opt, arg, desc)				\
    {0, 0, (rsp), (kw), (opt), (arg), (desc)}

/* RSTRG () - resource/long-option */
#define RSTRG(rsp, kw, arg)					\
    {0, 0, (rsp), (kw), NULL, (arg), NULL}

/* BOOL () - regular boolean `-/+' flag */
#define BOOL(rsp, kw, opt, option, flag, desc)			\
    { (option), (Optflag_Boolean | (flag)), (rsp), (kw), (opt), NULL, (desc)}

/* SWCH () - `-' flag */
#define SWCH(opt, option, flag, desc)				\
    { (option), (Optflag_Switch | (flag)), -1, NULL, (opt), NULL, (desc)}

/* convenient macros */
#define optList_isString(i)						\
    (optList[i].flag == 0)
#define optList_isBool(i)						\
    (optList[i].flag & Optflag_Boolean)
#define optList_isReverse(i)						\
    (optList[i].flag & Optflag_Reverse)
#define optList_isInfo(i)						\
    (optList[i].flag & Optflag_Info)

static const struct
  {
    const uint8_t   index;      /* Option index */
    const uint8_t   flag;	/* Option flag */
    const int16_t   doff;	/* resource value index or -1 */
    const char     *kw;		/* keyword */
    const char     *opt;	/* option */
    const char     *arg;	/* argument */
    const char     *desc;	/* description */
  }
optList[] = {
              STRG (Rs_display_name, NULL, "d", NULL, NULL),	/* short form */
              STRG (Rs_display_name, NULL, "display", "string", "X server to contact"),
              STRG (Rs_term_name, "termName", "tn", "string", "value of the TERM environment variable"),
              STRG (Rs_geometry, NULL, "g", NULL, NULL),	/* short form */
              STRG (Rs_geometry, "geometry", "geometry", "geometry", "size (in characters) and position"),
              SWCH ("C", Opt_console, 0, "intercept console messages"),
              SWCH ("iconic", Opt_iconic, 0, "start iconic"),
              SWCH ("ic", Opt_iconic, 0, NULL),	/* short form */
              STRG (Rs_chdir, "chdir", "cd", "string", "start shell in this directory"),
              BOOL (Rs_reverseVideo, "reverseVideo", "rv", Opt_reverseVideo, 0, "reverse video"),
              BOOL (Rs_loginShell, "loginShell", "ls", Opt_loginShell, 0, "login shell"),
              BOOL (Rs_jumpScroll, "jumpScroll", "j", Opt_jumpScroll, 0, "jump scrolling"),
              BOOL (Rs_skipScroll, "skipScroll", "ss", Opt_skipScroll, 0, "skip scrolling"),
              BOOL (Rs_pastableTabs, "pastableTabs", "ptab", Opt_pastableTabs, 0, "tab characters are pastable"),
              RSTRG (Rs_scrollstyle, "scrollstyle", "mode"),
              BOOL (Rs_scrollBar, "scrollBar", "sb", Opt_scrollBar, 0, "scrollbar"),
              BOOL (Rs_scrollBar_right, "scrollBar_right", "sr", Opt_scrollBar_right, 0, "scrollbar right"),
              BOOL (Rs_scrollBar_floating, "scrollBar_floating", "st", Opt_scrollBar_floating, 0, "scrollbar without a trough"),
              RSTRG (Rs_scrollBar_align, "scrollBar_align", "mode"),
              STRG (Rs_scrollBar_thickness, "thickness", "sbt", "number", "scrollbar thickness/width in pixels"),
              BOOL (Rs_scrollTtyOutput, "scrollTtyOutput", NULL, Opt_scrollTtyOutput, 0, NULL),
              BOOL (Rs_scrollTtyOutput, NULL, "si",  Opt_scrollTtyOutput, Optflag_Reverse, "scroll-on-tty-output inhibit"),
              BOOL (Rs_scrollTtyKeypress, "scrollTtyKeypress", "sk", Opt_scrollTtyKeypress, 0, "scroll-on-keypress"),
              BOOL (Rs_scrollWithBuffer, "scrollWithBuffer", "sw", Opt_scrollWithBuffer, 0, "scroll-with-buffer"),
#if ENABLE_TRANSPARENCY
              BOOL (Rs_transparent, "inheritPixmap", "ip", Opt_transparent, 0, "inherit parent pixmap"),
              BOOL (Rs_transparent, "transparent", "tr", Opt_transparent, 0, "inherit parent pixmap"),
              STRG (Rs_color + Color_tint, "tintColor", "tint", "color", "tint color"),
              STRG (Rs_shade, "shading", "sh", "number", "shade background by number %."),
              STRG (Rs_blurradius, "blurRadius", "blr", "HxV", "gaussian blur radii to apply to the root background"),
#endif
#if OFF_FOCUS_FADING
              STRG (Rs_fade, "fading", "fade", "number", "fade colors by number % when losing focus"),
              STRG (Rs_color + Color_fade, "fadeColor", "fadecolor", "color", "target color for off-focus fading"),
#endif
              BOOL (Rs_utmpInhibit, "utmpInhibit", "ut", Opt_utmpInhibit, 0, "utmp inhibit"),
#ifndef NO_BELL
# if ENABLE_FRILLS
              BOOL (Rs_urgentOnBell, "urgentOnBell", NULL, Opt_urgentOnBell, 0, NULL),
# endif
              BOOL (Rs_visualBell, "visualBell", "vb", Opt_visualBell, 0, "visual bell"),
# if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
              BOOL (Rs_mapAlert, "mapAlert", NULL, Opt_mapAlert, 0, NULL),
# endif
#endif
#ifdef META8_OPTION
              BOOL (Rs_meta8, "meta8", NULL, Opt_meta8, 0, NULL),
#endif
#ifdef MOUSE_WHEEL
              BOOL (Rs_mouseWheelScrollPage, "mouseWheelScrollPage", NULL, Opt_mouseWheelScrollPage, 0, NULL),
#endif
#if ENABLE_FRILLS
              BOOL (Rs_tripleclickwords, "tripleclickwords", "tcw", Opt_tripleclickwords, 0, "triple click word selection"),
              BOOL (Rs_insecure, "insecure", "insecure", Opt_insecure, 0, "enable possibly insecure escape sequences"),
              BOOL (Rs_cursorUnderline, "cursorUnderline", "uc", Opt_cursorUnderline, 0, "underline cursor"),
#endif
#if CURSOR_BLINK
              BOOL (Rs_cursorBlink, "cursorBlink", "bc", Opt_cursorBlink, 0, "blinking cursor"),
#endif
#ifdef POINTER_BLANK
              BOOL (Rs_pointerBlank, "pointerBlank", "pb", Opt_pointerBlank, 0, "switch off pointer after delay"),
#endif
              STRG (Rs_color + Color_bg, "background", "bg", "color", "background color"),
              STRG (Rs_color + Color_fg, "foreground", "fg", "color", "foreground color"),
              RSTRG (Rs_color + minCOLOR + 0, "color0", "color"),
              RSTRG (Rs_color + minCOLOR + 1, "color1", "color"),
              RSTRG (Rs_color + minCOLOR + 2, "color2", "color"),
              RSTRG (Rs_color + minCOLOR + 3, "color3", "color"),
              RSTRG (Rs_color + minCOLOR + 4, "color4", "color"),
              RSTRG (Rs_color + minCOLOR + 5, "color5", "color"),
              RSTRG (Rs_color + minCOLOR + 6, "color6", "color"),
              RSTRG (Rs_color + minCOLOR + 7, "color7", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 0, "color8", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 1, "color9", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 2, "color10", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 3, "color11", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 4, "color12", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 5, "color13", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 6, "color14", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 7, "color15", "color"),
#ifndef NO_BOLD_UNDERLINE_REVERSE
              RSTRG (Rs_color + Color_BD, "colorBD", "color"),
              RSTRG (Rs_color + Color_IT, "colorIT", "color"),
              RSTRG (Rs_color + Color_UL, "colorUL", "color"),
              RSTRG (Rs_color + Color_RV, "colorRV", "color"),
#endif /* ! NO_BOLD_UNDERLINE_REVERSE */
#if ENABLE_FRILLS
              RSTRG (Rs_color + Color_underline, "underlineColor", "color"),
#endif
              RSTRG (Rs_color + Color_scroll, "scrollColor", "color"),
#ifdef RXVT_SCROLLBAR
              RSTRG (Rs_color + Color_trough, "troughColor", "color"),
#endif
#ifdef OPTION_HC
              STRG (Rs_color + Color_HC, "highlightColor", "hc", "color", "highlight color"),
              RSTRG (Rs_color + Color_HTC, "highlightTextColor", "color"),
#endif
#ifndef NO_CURSORCOLOR
              STRG (Rs_color + Color_cursor, "cursorColor", "cr", "color", "cursor color"),
              /* command-line option = resource name */
              RSTRG (Rs_color + Color_cursor2, "cursorColor2", "color"),
#endif /* NO_CURSORCOLOR */
              STRG (Rs_color + Color_pointer_fg, "pointerColor", "pr", "color", "pointer color"),
              STRG (Rs_color + Color_pointer_bg, "pointerColor2", "pr2", "color", "pointer bg color"),
              STRG (Rs_color + Color_border, "borderColor", "bd", "color", "border color"),
#ifdef BG_IMAGE_FROM_FILE
              RSTRG (Rs_path, "path", "search path"),
              STRG (Rs_backgroundPixmap, "backgroundPixmap", "pixmap", "file[;geom]", "background pixmap"),
# if ENABLE_EWMH
              STRG (Rs_iconfile, "iconFile", "icon", "file", "path to application icon image"),
# endif
#endif
              /* fonts: command-line option = resource name */
              STRG (Rs_font, "font", "fn", "fontname", "normal text font"),
#if ENABLE_STYLES
              STRG (Rs_boldFont, "boldFont", "fb", "fontname", "bold font"),
              STRG (Rs_italicFont, "italicFont", "fi", "fontname", "italic font"),
              STRG (Rs_boldItalicFont, "boldItalicFont", "fbi", "fontname", "bold italic font"),
              BOOL (Rs_intensityStyles, "intensityStyles", "is", Opt_intensityStyles, 0, "font styles imply intensity changes"),
#endif
#ifdef USE_XIM
              STRG (Rs_inputMethod, "inputMethod", "im", "name", "name of input method"),
              STRG (Rs_preeditType, "preeditType", "pt", "style", "input style: style = OverTheSpot|OffTheSpot|Root"),
              STRG (Rs_imLocale, "imLocale", "imlocale", "string", "locale to use for input method"),
              STRG (Rs_imFont, "imFont", "imfont", "fontname", "fontset for styles OverTheSpot and OffTheSpot"),
#endif /* USE_XIM */
              STRG (Rs_name, NULL, "name", "string", "client instance, icon, and title strings"),
              STRG (Rs_title, "title", "title", "string", "title name for window"),
              STRG (Rs_title, NULL, "T", NULL, NULL),	/* short form */
              STRG (Rs_iconName, "iconName", "n", "string", "icon name for window"),
              STRG (Rs_saveLines, "saveLines", "sl", "number", "number of scrolled lines to save"),
#if ENABLE_XEMBED
              STRG (Rs_embed, NULL, "embed", "windowid", "window id to embed terminal in"),
#endif
#if XFT
              STRG (Rs_depth, "depth", "depth", "number", "depth of visual to request"),
              BOOL (Rs_buffered, "buffered", NULL, Opt_buffered, 0, NULL),
#endif
#if ENABLE_FRILLS
              RSTRG (Rs_transient_for, "transient-for", "windowid"),
              BOOL (Rs_override_redirect, "override-redirect", "override-redirect", Opt_override_redirect, 0, "set override-redirect on the terminal window"),
              STRG (Rs_pty_fd, NULL, "pty-fd", "fileno", "file descriptor of pty to use"),
              BOOL (Rs_hold, "hold", "hold", Opt_hold, 0, "retain window after shell exit"),
              STRG (Rs_ext_bwidth, "externalBorder", "w", "number", "external border in pixels"),
              STRG (Rs_ext_bwidth, NULL, "bw", NULL, NULL),
              STRG (Rs_ext_bwidth, NULL, "borderwidth", NULL, NULL),
              STRG (Rs_int_bwidth, "internalBorder", "b", "number", "internal border in pixels"),
              BOOL (Rs_borderLess, "borderLess", "bl", Opt_borderLess, 0, "borderless window"),
              STRG (Rs_lineSpace, "lineSpace", "lsp", "number", "number of extra pixels between rows"),
              STRG (Rs_letterSpace, "letterSpace", "letsp", "number", "letter spacing adjustment"),
#endif
#ifdef BUILTIN_GLYPHS
              BOOL (Rs_skipBuiltinGlyphs, "skipBuiltinGlyphs", "sbg", Opt_skipBuiltinGlyphs, 0, "do not use internal glyphs"),
#endif
#ifdef POINTER_BLANK
              RSTRG (Rs_pointerBlankDelay, "pointerBlankDelay", "number"),
#endif
#ifndef NO_BACKSPACE_KEY
              RSTRG (Rs_backspace_key, "backspacekey", "string"),
#endif
#ifndef NO_DELETE_KEY
              RSTRG (Rs_delete_key, "deletekey", "string"),
#endif
#ifdef PRINTPIPE
              RSTRG (Rs_print_pipe, "print-pipe", "string"),
#endif
              STRG (Rs_modifier, "modifier", "mod", "modifier", "meta modifier = alt|meta|hyper|super|mod1|...|mod5"),
              RSTRG (Rs_cutchars, "cutchars", "string"),
              RSTRG (Rs_answerbackstring, "answerbackString", "string"),
#ifndef NO_SECONDARY_SCREEN
              BOOL (Rs_secondaryScreen, "secondaryScreen", "ssc", Opt_secondaryScreen, 0, "enable secondary screen"),
              BOOL (Rs_secondaryScroll, "secondaryScroll", "ssr", Opt_secondaryScroll, 0, "enable secondary screen scroll"),
#endif
#if ENABLE_PERL
              RSTRG (Rs_perl_lib, "perl-lib", "string"), //, "colon-separated directories with extension scripts"),TODO
              RSTRG (Rs_perl_eval, "perl-eval", "perl-eval"), // "string", "code to be evaluated after all extensions have been loaded"),TODO
              RSTRG (Rs_perl_ext_1, "perl-ext-common", "string"), //, "colon-separated list of perl extensions to enable"),TODO
              STRG (Rs_perl_ext_2, "perl-ext", "pe", "string", "colon-separated list of perl extensions to enable for this instance"),
#endif
#if ISO_14755
              BOOL (Rs_iso14755, "iso14755", NULL, Opt_iso14755, 0, NULL),
              BOOL (Rs_iso14755_52, "iso14755_52", NULL, Opt_iso14755_52, 0, NULL),
#endif
#ifdef HAVE_AFTERIMAGE
              STRG (Rs_blendtype, "blendType", "blt", "string", "background image blending type - alpha, tint, etc..."),
#endif
#ifndef NO_RESOURCES
              RINFO ("xrm", "string"),
#endif
#ifdef KEYSYM_RESOURCE
              RINFO ("keysym.sym", "keysym"),
#endif
              INFO ("e", "command arg ...", "command to execute")
            };

#undef INFO
#undef RINFO
#undef STRG
#undef RSTRG
#undef SWCH
#undef BOOL

static const char releasestring[] = "rxvt-unicode (" RXVTNAME ") v" VERSION " - released: " DATE "\n";
static const char optionsstring[] = "options: "
#if ENABLE_PERL
                                    "perl,"
#endif
#if XFT
                                    "xft,"
#endif
#if ENABLE_STYLES
                                    "styles,"
#endif
#if ENABLE_COMBINING
                                    "combining,"
#endif
#if TEXT_BLINK
                                    "blink,"
#endif
#if ISO_14755
                                    "iso14755,"
#endif
#if UNICODE_3
                                    "unicode3,"
#endif
                                    "encodings=eu+vn"
#if ENCODING_JP
                                    "+jp"
#endif
#if ENCODING_JP_EXT
                                    "+jp-ext"
#endif
#if ENCODING_KR
                                    "+kr"
#endif
#if ENCODING_ZH
                                    "+zh"
#endif
#if ENCODING_ZH_EXT
                                    "+zh-ext"
#endif
                                    ","
#if OFF_FOCUS_FADING
                                    "fade,"
#endif
#if defined(ENABLE_TRANSPARENCY)
                                    "transparent,"
                                    "tint,"
#endif
#if HAVE_AFTERIMAGE
                                    "afterimage,"
#endif
#if HAVE_PIXBUF
                                    "pixbuf,"
#endif
#if defined(USE_XIM)
                                    "XIM,"
#endif
#if defined(NO_BACKSPACE_KEY)
                                    "no_backspace,"
#endif
#if defined(NO_DELETE_KEY)
                                    "no_delete,"
#endif
#if EIGHT_BIT_CONTROLS
                                    "8bitctrls,"
#endif
#if defined(ENABLE_FRILLS)
                                    "frills,"
#endif
#if defined(SELECTION_SCROLLING)
                                    "selectionscrolling,"
#endif
#if MOUSE_WHEEL
                                    "wheel,"
#endif
#if MOUSE_SLIP_WHEELING
                                    "slipwheel,"
#endif
#if defined(SMART_RESIZE)
                                    "smart-resize,"
#endif
#if defined(CURSOR_BLINK)
                                    "cursorBlink,"
#endif
#if defined(POINTER_BLANK)
                                    "pointerBlank,"
#endif
#if defined(NO_RESOURCES)
                                    "NoResources,"
#endif
                                    "scrollbars=plain"
#if defined(RXVT_SCROLLBAR)
                                    "+rxvt"
#endif
#if defined(NEXT_SCROLLBAR)
                                    "+NeXT"
#endif
#if defined(XTERM_SCROLLBAR)
                                    "+xterm"
#endif
                                    "\nUsage: ";		/* Usage */

#define INDENT 18

const char rxvt_term::resval_undef [] = "<undef>";
const char rxvt_term::resval_on []    = "on";
const char rxvt_term::resval_off []   = "off";

/*{{{ usage: */
/*----------------------------------------------------------------------*/
static void
rxvt_usage (int type)
{
  unsigned int i, col;

  rxvt_log ("%s%s%s", releasestring, optionsstring, RESNAME);

  switch (type)
    {
      case 0:			/* brief listing */
        rxvt_log (" [-help] [--help]\n");

        for (col = 1, i = 0; i < ecb_array_length (optList); i++)
          if (optList[i].desc != NULL)
            {
              int len = 0;

              if (optList[i].arg)
                len = strlen (optList[i].arg) + 1;
              assert (optList[i].opt != NULL);
              len += 4 + strlen (optList[i].opt) + (optList_isBool (i) ? 2 : 0);
              col += len;
              if (col > 79)
                {
                  /* assume regular width */
                  rxvt_log ("\n");
                  col = 1 + len;
                }

              rxvt_log (" [-%s%s", (optList_isBool (i) ? "/+" : ""), optList[i].opt);
              if (optList[i].arg)
                rxvt_log (" %s]", optList[i].arg);
              else
                rxvt_log ("]");
            }
        break;

      case 1:			/* full command-line listing */
        rxvt_log (" [options] [-e command args]\n\nwhere options include:\n");

        for (i = 0; i < ecb_array_length (optList); i++)
          if (optList[i].desc != NULL)
            {
              assert (optList[i].opt != NULL);
              rxvt_log ("  %s%s %-*s%s%s\n",
                         (optList_isBool (i) ? "-/+" : "-"), optList[i].opt,
                         (INDENT - strlen (optList[i].opt)
                          + (optList_isBool (i) ? 0 : 2)),
                         (optList[i].arg ? optList[i].arg : ""),
                         (optList_isBool (i) ? "turn on/off " : ""),
                         optList[i].desc);
            }
        rxvt_log ("\n  --help to list long-options");
        break;

      case 2:			/* full resource listing */
        rxvt_log (" [options] [-e command args]\n\n"
                   "where resources (long-options) include:\n");

        for (i = 0; i < ecb_array_length (optList); i++)
          if (optList[i].kw != NULL)
            rxvt_log ("  %s: %*s%s\n",
                    optList[i].kw,
                    (INDENT - strlen (optList[i].kw)), "", /* XXX */
                    (optList_isBool (i) ? "boolean" : optList[i].arg));
        rxvt_log ("\n  -help to list options");
        break;
    }

  rxvt_log ("\n\n");
  rxvt_exit_failure ();
}

/*}}} */

/*{{{ get command-line options before getting resources */
const char **
rxvt_term::get_options (int argc, const char *const *argv)
{
  int i, bad_option = 0;

  for (i = 1; i < argc; i++)
    {
      unsigned int entry, longopt = 0;
      const char *opt;
      int flag;

      opt = argv[i];

      if (*opt == '-')
        {
          flag = 1;

          if (*++opt == '-')
            longopt = *opt++;	/* long option */
        }
      else if (*opt == '+')
        {
          flag = 0;

          if (*++opt == '+')
            longopt = *opt++;	/* long option */
        }
      else
        {
          bad_option = 1;
          rxvt_warn ("\"%s\": malformed option.\n", opt);
          continue;
        }

      if (!strcmp (opt, "help"))
        rxvt_usage (longopt ? 2 : 1);

      if (!strcmp (opt, "h"))
        rxvt_usage (0);

      /* feature: always try to match long-options */
      for (entry = 0; entry < ecb_array_length (optList); entry++)
        if ((optList[entry].kw && !strcmp (opt, optList[entry].kw))
            || (!longopt
                && optList[entry].opt && !strcmp (opt, optList[entry].opt)))
          break;

      if (entry < ecb_array_length (optList)
          && !optList_isInfo (entry))
        {
          if (optList_isReverse (entry))
            flag = !flag;

          if (optList_isString (entry))
            {
              /*
               * special cases are handled in init_resources () to allow
               * X resources to set these values before we settle for
               * default values
               */

              if (optList[entry].doff != -1)
                {
                  if (flag && i+1 == argc)
                    rxvt_fatal ("option '%s' requires an argument, aborting.\n", argv [i]);

                  rs[optList[entry].doff] = flag ? argv[++i] : resval_undef;
                }
            }
          else
            {
              /* boolean value */
              set_option (optList[entry].index, flag);

              if (optList[entry].doff != -1)
                rs[optList[entry].doff] = flag ? resval_on : resval_off;
            }
        }
#ifndef NO_RESOURCES
      else if (!strcmp (opt, "xrm"))
        {
          if (i+1 < argc)
            XrmPutLineResource (&option_db, argv[++i]);
        }
#endif
#ifdef KEYSYM_RESOURCE
      else if (!strncmp (opt, "keysym.", sizeof ("keysym.") - 1))
        {
          if (i+1 < argc)
            {
              char *res = rxvt_temp_buf<char> (strlen (opt) + strlen (argv[++i]) + 6);
              sprintf (res, "*.%s: %s\n", opt, argv[i]);
              XrmPutLineResource (&option_db, res);
            }
        }
#endif
      else if (!strcmp (opt, "e"))
        {
          if (i+1 == argc)
            rxvt_fatal ("option '-e' requires an argument, aborting.\n");

          return (const char **)argv + i + 1;
        }
      else
        {
          bad_option = 1;
          rxvt_warn ("\"%s\": unknown or malformed option.\n", opt);
        }
    }

  if (bad_option)
    rxvt_usage (0);

  return 0;
}

/*}}} */

#ifndef NO_RESOURCES
/*----------------------------------------------------------------------*/

# ifdef KEYSYM_RESOURCE
/*
 * Define key from XrmEnumerateDatabase.
 *   quarks will be something like
 *      "rxvt" "keysym" "0xFF01"
 *   value will be a string
 */
static int
rxvt_define_key (XrmDatabase *database ecb_unused,
                 XrmBindingList bindings ecb_unused,
                 XrmQuarkList quarks,
                 XrmRepresentation *type ecb_unused,
                 XrmValue *value,
                 XPointer closure ecb_unused)
{
  int last;

  for (last = 0; quarks[last] != NULLQUARK; last++)	/* look for last quark in list */
    ;

  last--;
  GET_R->parse_keysym (XrmQuarkToString (quarks[last]), (char *)value->addr);//D//TODO
  return False;
}

/*
 * look for something like this (XK_Delete)
 * rxvt*keysym.0xFFFF: "\177"
 */

struct keysym_vocabulary_t
{
  const char    *name;
  unsigned short len;
  unsigned short value;
};
static const keysym_vocabulary_t keysym_vocabulary[] =
{
  { "ISOLevel3", 9, Level3Mask    },
  { "AppKeypad", 9, AppKeypadMask },
  { "Control",   7, ControlMask   },
  { "NumLock",   7, NumLockMask   },
  { "Shift",     5, ShiftMask     },
  { "Meta",      4, MetaMask      },
  { "Lock",      4, LockMask      },
  { "Mod1",      4, Mod1Mask      },
  { "Mod2",      4, Mod2Mask      },
  { "Mod3",      4, Mod3Mask      },
  { "Mod4",      4, Mod4Mask      },
  { "Mod5",      4, Mod5Mask      },
  { "I",         1, Level3Mask    },
  { "K",         1, AppKeypadMask },
  { "C",         1, ControlMask   },
  { "N",         1, NumLockMask   },
  { "S",         1, ShiftMask     },
  { "M",         1, MetaMask      },
  { "A",         1, MetaMask      },
  { "L",         1, LockMask      },
  { "1",         1, Mod1Mask      },
  { "2",         1, Mod2Mask      },
  { "3",         1, Mod3Mask      },
  { "4",         1, Mod4Mask      },
  { "5",         1, Mod5Mask      },
};

int
rxvt_term::parse_keysym (const char *str, const char *arg)
{
  int sym;
  unsigned int state = 0;
  const char *key = strrchr (str, '-');

  if (!key)
    key = str;
  else
    key++;

  // string or key is empty
  if (*arg == '\0' || *key == '\0')
    return -1;

  // parse modifiers
  while (str < key)
    {
      unsigned int i;

      for (i = 0; i < ecb_array_length (keysym_vocabulary); ++i)
        {
          if (strncmp (str, keysym_vocabulary [i].name, keysym_vocabulary [i].len) == 0)
            {
              state |= keysym_vocabulary[i].value;
              str += keysym_vocabulary[i].len;
              break;
            }
        }

      if (i >= ecb_array_length (keysym_vocabulary))
        return -1;

      if (*str == '-')
        ++str;
    }

  // convert keysym name to keysym number
  if ((sym = XStringToKeysym (str)) == None)
    {
      // fallback on hexadecimal parsing
      char *end;
      sym = strtol (str, &end, 16);
      if (*end)
        return -1;
    }

  wchar_t *ws = rxvt_mbstowcs (arg);
  if (!HOOK_INVOKE ((this, HOOK_REGISTER_COMMAND, DT_INT, sym, DT_INT, state, DT_WCS_LEN, ws, wcslen (ws), DT_END)))
    keyboard->register_user_translation (sym, state, ws);
  free (ws);
  return 1;
}

# endif /* KEYSYM_RESOURCE */
#endif /* NO_RESOURCES */

static char *
get_res (XrmDatabase database, const char *program, const char *option)
{
  char resource[512];
  char *type;
  XrmValue result;

  snprintf (resource, sizeof (resource), "%s.%s", program, option);
  XrmGetResource (database, resource, resource, &type, &result);

  return result.addr;
}

const char *
rxvt_term::x_resource (const char *name)
{
  XrmDatabase database = XrmGetDatabase (dpy);

  const char *p = get_res (database, rs[Rs_name], name);
  const char *p0 = get_res (database, "!INVALIDPROGRAMMENAMEDONTMATCH!", name);

  if (p == NULL || (p0 && strcmp (p, p0) == 0))
    {
      p = get_res (database, RESCLASS, name);
#ifdef RESFALLBACK
      if (p == NULL || (p0 && strcmp (p, p0) == 0))
        p = get_res (database, RESFALLBACK, name);
#endif
    }

  if (p == NULL && p0)
    p = p0;

  return p;
}

void
rxvt_term::extract_resources ()
{
#ifndef NO_RESOURCES
  XrmDatabase database = XrmGetDatabase (dpy);
  XrmMergeDatabases (option_db, &database);
  option_db = NULL;
  /*
   * Query resources for options that affect us
   */
  for (int entry = 0; entry < ecb_array_length (optList); entry++)
    {
      int s;
      const char *kw = optList[entry].kw;

      if (kw == NULL || rs[optList[entry].doff] != NULL)
        continue; // previously set

      const char *p = x_resource (kw);

      if (p)
        {
          p = strdup (p);
          allocated.push_back ((void *)p);
          rs[optList[entry].doff] = p;

          if (optList_isBool (entry))
            {
              s = strcasecmp (p, "TRUE") == 0
                  || strcasecmp (p, "YES") == 0
                  || strcasecmp (p, "ON") == 0
                  || strcasecmp (p, "1") == 0;

              if (optList_isReverse (entry))
                s = !s;

              set_option (optList[entry].index, s);
            }
        }
    }
#endif /* NO_RESOURCES */
}

void
rxvt_term::extract_keysym_resources ()
{
#ifndef NO_RESOURCES
  /*
   * [R5 or later]: enumerate the resource database
   */
#  ifdef KEYSYM_RESOURCE
  XrmDatabase database = XrmGetDatabase (dpy);
  XrmName name_prefix[3];
  XrmClass class_prefix[3];

  name_prefix[0] = XrmStringToName (rs[Rs_name]);
  name_prefix[1] = XrmStringToName ("keysym");
  name_prefix[2] = NULLQUARK;
  class_prefix[0] = XrmStringToName (RESCLASS);
  class_prefix[1] = XrmStringToName ("Keysym");
  class_prefix[2] = NULLQUARK;
  /* XXX: Need to check sizeof (rxvt_t) == sizeof (XPointer) */
  XrmEnumerateDatabase (database, name_prefix, class_prefix,
                        XrmEnumOneLevel, rxvt_define_key, NULL);
#   ifdef RESFALLBACK
  name_prefix[0] = class_prefix[0] = XrmStringToName (RESFALLBACK);
  /* XXX: Need to check sizeof (rxvt_t) == sizeof (XPointer) */
  XrmEnumerateDatabase (database, name_prefix, class_prefix,
                        XrmEnumOneLevel, rxvt_define_key, NULL);
#   endif
#  endif

#endif /* NO_RESOURCES */
}

/*----------------------- end-of-file (C source) -----------------------*/
