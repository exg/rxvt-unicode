/*--------------------------------*-C-*---------------------------------*
 * File:	xdefaults.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *				- original version
 * Copyright (c) 1997,1998 mj olesen <olesen@me.queensu.ca>
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
/*----------------------------------------------------------------------*
 * get resources from ~/.Xdefaults or ~/.Xresources with the memory-saving
 * default or with XGetDefault() (#define USE_XGETDEFAULT)
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "version.h"

/* #define DEBUG_RESOURCES */

static const char *const xnames[2] = { ".Xdefaults", ".Xresources" };

/*{{{ monolithic option/resource structure: */
/*
 * `string' options MUST have a usage argument
 * `switch' and `boolean' options have no argument
 * if there's no desc (ription), it won't appear in rxvt_usage ()
 */

/* INFO () - descriptive information only */
#define INFO(opt, arg, desc)					\
    {0, -1, NULL, (opt), (arg), (desc)}

/* STRG () - command-line option, with/without resource */
#define STRG(rsp, kw, opt, arg, desc)				\
    {0, (rsp), (kw), (opt), (arg), (desc)}

/* RSTRG () - resource/long-option */
#define RSTRG(rsp, kw, arg)					\
    {0, (rsp), (kw), NULL, (arg), NULL}

/* BOOL () - regular boolean `-/+' flag */
#define BOOL(rsp, kw, opt, flag, desc)				\
    { (Opt_Boolean| (flag)), (rsp), (kw), (opt), NULL, (desc)}

/* SWCH () - `-' flag */
#define SWCH(opt, flag, desc)					\
    { (flag), -1, NULL, (opt), NULL, (desc)}

/* convenient macros */
#define optList_strlen(i)						\
    (optList[i].flag ? 0 : (optList[i].arg ? strlen (optList[i].arg) : 1))
#define optList_isBool(i)						\
    (optList[i].flag & Opt_Boolean)
#define optList_isReverse(i)						\
    (optList[i].flag & Opt_Reverse)
#define optList_size()							\
    (sizeof (optList) / sizeof (optList[0]))

static const struct
  {
    const unsigned long flag;	/* Option flag */
    const int       doff;	/* data offset */
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
              SWCH ("C", Opt_console, "intercept console messages"),
              SWCH ("iconic", Opt_iconic, "start iconic"),
              SWCH ("ic", Opt_iconic, NULL),	/* short form */
              BOOL (Rs_reverseVideo, "reverseVideo", "rv", Opt_reverseVideo, "reverse video"),
              BOOL (Rs_loginShell, "loginShell", "ls", Opt_loginShell, "login shell"),
              BOOL (Rs_jumpScroll, "jumpScroll", "j", Opt_jumpScroll, "jump scrolling"),
#ifdef HAVE_SCROLLBARS
              BOOL (Rs_scrollBar, "scrollBar", "sb", Opt_scrollBar, "scrollbar"),
              BOOL (Rs_scrollBar_right, "scrollBar_right", "sr", Opt_scrollBar_right, "scrollbar right"),
              BOOL (Rs_scrollBar_floating, "scrollBar_floating", "st", Opt_scrollBar_floating, "scrollbar without a trough"),
#endif
              BOOL (Rs_scrollTtyOutput, "scrollTtyOutput", NULL, Opt_scrollTtyOutput, NULL),
              BOOL (Rs_scrollTtyOutput, NULL, "si", Opt_Reverse | Opt_scrollTtyOutput, "scroll-on-tty-output inhibit"),
              BOOL (Rs_scrollTtyKeypress, "scrollTtyKeypress", "sk", Opt_scrollTtyKeypress, "scroll-on-keypress"),
              BOOL (Rs_scrollWithBuffer, "scrollWithBuffer", "sw", Opt_scrollWithBuffer, "scroll-with-buffer"),
#ifdef TRANSPARENT
              BOOL (Rs_transparent, "inheritPixmap", "ip", Opt_transparent, "inherit parent pixmap"),
              BOOL (Rs_transparent_all, "inheritPixmapforce", "ipf", Opt_transparent_all, "forcefully inherit root pixmap"),
              SWCH ("tr", Opt_transparent, NULL),
#if TINTING
              STRG (Rs_color + Color_tint, "tintColor", "tint", "color", "tint color"),
#endif
#endif
#ifdef OFF_FOCUS_FADING
              STRG (Rs_fade, "fading", "fade", "%", "make colors x% darker when rxvt-unicode is losing focus."),
#endif
#ifdef TINTING
              STRG (Rs_shade, "shading", "sh", "%", "shade background by x% when tinting."),
#endif
              BOOL (Rs_utmpInhibit, "utmpInhibit", "ut", Opt_utmpInhibit, "utmp inhibit"),
#ifndef NO_BELL
              BOOL (Rs_visualBell, "visualBell", "vb", Opt_visualBell, "visual bell"),
# if ! defined(NO_MAPALERT) && defined(MAPALERT_OPTION)
              BOOL (Rs_mapAlert, "mapAlert", NULL, Opt_mapAlert, NULL),
# endif
#endif
#ifdef META8_OPTION
              BOOL (Rs_meta8, "meta8", NULL, Opt_meta8, NULL),
#endif
#ifdef MOUSE_WHEEL
              BOOL (Rs_mouseWheelScrollPage, "mouseWheelScrollPage", NULL, Opt_mouseWheelScrollPage, NULL),
#endif
#if ENABLE_FRILLS
              BOOL (Rs_tripleclickwords, "tripleclickwords", "tcw", Opt_tripleclickwords, "triple click word selection"),
              BOOL (Rs_insecure, "insecure", "insecure", Opt_insecure, "enable possibly insecure escape sequences"),
#endif
#ifdef CURSOR_BLINK
              BOOL (Rs_cursorBlink, "cursorBlink", "bc", Opt_cursorBlink, "blinking cursor"),
#endif
#ifdef POINTER_BLANK
              BOOL (Rs_pointerBlank, "pointerBlank", "pb", Opt_pointerBlank, "switch off pointer after delay"),
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
#ifndef NO_BRIGHTCOLOR
              RSTRG (Rs_color + minBrightCOLOR + 0, "color8", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 1, "color9", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 2, "color10", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 3, "color11", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 4, "color12", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 5, "color13", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 6, "color14", "color"),
              RSTRG (Rs_color + minBrightCOLOR + 7, "color15", "color"),
#endif				/* NO_BRIGHTCOLOR */
#ifndef NO_BOLD_UNDERLINE_REVERSE
              RSTRG (Rs_color + Color_BD, "colorBD", "color"),
              RSTRG (Rs_color + Color_IT, "colorIT", "color"),
              RSTRG (Rs_color + Color_UL, "colorUL", "color"),
              RSTRG (Rs_color + Color_RV, "colorRV", "color"),
#endif				/* ! NO_BOLD_UNDERLINE_REVERSE */
#ifdef KEEP_SCROLLCOLOR
              RSTRG (Rs_color + Color_scroll, "scrollColor", "color"),
              RSTRG (Rs_color + Color_trough, "troughColor", "color"),
#endif				/* KEEP_SCROLLCOLOR */
#ifdef OPTION_HC
              STRG (Rs_color + Color_HC, "highlightColor", "hc", "color", "highlight color"),
#endif
#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
              RSTRG (Rs_path, "path", "search path"),
#endif				/* defined (XPM_BACKGROUND) || (MENUBAR_MAX) */
#ifdef XPM_BACKGROUND
              STRG (Rs_backgroundPixmap, "backgroundPixmap", "pixmap", "file[;geom]", "background pixmap"),
#endif				/* XPM_BACKGROUND */
#if (MENUBAR_MAX)
              RSTRG (Rs_menu, "menu", "name[;tag]"),
#endif
              /* fonts: command-line option = resource name */
              STRG (Rs_font, "font", "fn", "fontname", "normal text font"),
#if ENABLE_STYLES
              STRG (Rs_boldFont, "boldFont", "fb", "fontname", "bold font"),
              STRG (Rs_italicFont, "italicFont", "fi", "fontname", "italic font"),
              STRG (Rs_boldItalicFont, "boldItalicFont", "fbi", "fontname", "bold italic font"),
#endif
#ifdef USE_XIM
              STRG (Rs_inputMethod, "inputMethod", "im", "name", "name of input method"),
              STRG (Rs_preeditType, "preeditType", "pt", "style", "input style: style = OverTheSpot|OffTheSpot|Root"),
#if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
              STRG (Rs_imLocale, "imLocale", "imlocale", "string", "locale to use for input method"),
#endif
#endif				/* USE_XIM */
              STRG (Rs_name, NULL, "name", "string", "client instance, icon, and title strings"),
              STRG (Rs_title, "title", "title", "string", "title name for window"),
              STRG (Rs_title, NULL, "T", NULL, NULL),	/* short form */
              STRG (Rs_iconName, "iconName", "n", "string", "icon name for window"),
#ifndef NO_CURSORCOLOR
              STRG (Rs_color + Color_cursor, "cursorColor", "cr", "color", "cursor color"),
              /* command-line option = resource name */
              RSTRG (Rs_color + Color_cursor2, "cursorColor2", "color"),
#endif				/* NO_CURSORCOLOR */
              STRG (Rs_color + Color_pointer_fg, "pointerColor", "pr", "color", "pointer color"),
              STRG (Rs_color + Color_pointer_bg, "pointerColor2", "pr2", "color", "pointer bg color"),
              STRG (Rs_color + Color_border, "borderColor", "bd", "color", "border color"),
              STRG (Rs_saveLines, "saveLines", "sl", "number", "number of scrolled lines to save"),
#if ENABLE_FRILLS
              STRG (Rs_ext_bwidth, "externalBorder", "w", "number", "external border in pixels"),
              STRG (Rs_ext_bwidth, NULL, "bw", NULL, NULL),
              STRG (Rs_ext_bwidth, NULL, "borderwidth", NULL, NULL),
              STRG (Rs_int_bwidth, "internalBorder", "b", "number", "internal border in pixels"),
              BOOL (Rs_borderLess, "borderLess", "bl", Opt_borderLess, "borderless window"),
#endif
#ifndef NO_LINESPACE
              STRG (Rs_lineSpace, "lineSpace", "lsp", "number", "number of extra pixels between rows"),
#endif
              STRG (Rs_scrollBar_thickness, "thickness", "sbt", "number", "scrollbar thickness/width in pixels"),
#ifdef POINTER_BLANK
              RSTRG (Rs_pointerBlankDelay, "pointerBlankDelay", "number"),
#endif
#ifndef NO_BACKSPACE_KEY
              RSTRG (Rs_backspace_key, "backspacekey", "string"),
#endif
#ifndef NO_DELETE_KEY
              RSTRG (Rs_delete_key, "deletekey", "string"),
#endif
              RSTRG (Rs_selectstyle, "selectstyle", "mode"),
              RSTRG (Rs_scrollstyle, "scrollstyle", "mode"),
#ifdef HAVE_SCROLLBARS
              RSTRG (Rs_scrollBar_align, "scrollBar_align", "mode"),
#endif
#ifdef PRINTPIPE
              RSTRG (Rs_print_pipe, "print-pipe", "string"),
#endif
              STRG (Rs_modifier, "modifier", "mod", "modifier", "meta modifier = alt|meta|hyper|super|mod1|...|mod5"),
              INFO ("xrm", "string", "X resource"),
#ifdef CUTCHAR_RESOURCE
              RSTRG (Rs_cutchars, "cutchars", "string"),
#endif				/* CUTCHAR_RESOURCE */
              RSTRG (Rs_answerbackstring, "answerbackString", "string"),
#ifndef NO_SECONDARY_SCREEN
              BOOL (Rs_secondaryScreen, "secondaryScreen", "ssc", Opt_secondaryScreen, "enable secondary screen"),
              BOOL (Rs_secondaryScroll, "secondaryScroll", "ssr", Opt_secondaryScroll, "enable secondary screen scroll"),
#endif
              INFO ("e", "command arg ...", "command to execute")
            };

#undef INFO
#undef STRG
#undef RSTRG
#undef SWCH
#undef BOOL
/*}}} */

static const char releasestring[] = "rxvt-unicode (" RXVTNAME ") v" VERSION " - released: " DATE "\n";
static const char optionsstring[] = "options: "
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
#if defined(XPM_BACKGROUND)
                                    "XPM,"
#endif
#if defined(TRANSPARENT)
                                    "transparent,"
#endif
#if TINTING
                                    "tint,"
#endif
#if defined(UTMP_SUPPORT)
                                    "utmp,"
#endif
#if defined(MENUBAR)
                                    "menubar,"
#endif
#if defined(USE_XIM)
                                    "XIM,"
#endif
                                    "scrollbars="
#if !defined(HAVE_SCROLLBARS)
                                    "NONE"
#else
# if defined(PLAIN_SCROLLBAR)
                                    "plain"
#  if defined(RXVT_SCROLLBAR) || defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR)
                                    "+"
#  endif
# endif
# if defined(RXVT_SCROLLBAR)
                                    "rxvt"
#  if defined(NEXT_SCROLLBAR) || defined(XTERM_SCROLLBAR)
                                    "+"
#  endif
# endif
# if defined(NEXT_SCROLLBAR)
                                    "NeXT"
#  if defined(XTERM_SCROLLBAR)
                                    "+"
#  endif
# endif
# if defined(XTERM_SCROLLBAR)
                                    "xterm"
# endif
#endif
                                    ","
#if defined(NO_BACKSPACE_KEY)
                                    "no_backspace,"
#endif
#if defined(NO_DELETE_KEY)
                                    "no_delete,"
#endif
#if EIGHT_BIT_CONTROLS
                                    "8bitctrls,"
#endif
#if !defined(NO_STRINGS)
                                    "strings,"
#endif
#if defined(ENABLE_FRILLS)
                                    "frills,"
#endif
#if !defined(NO_LINESPACE)
                                    "linespace,"
#endif
#if defined(PREFER_24BIT)
                                    "24bit,"
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
                                    "NoResources"
#else
# if defined(USE_XGETDEFAULT)
                                    "XGetDefaults"
# else
                                    ".Xdefaults"
# endif
#endif
                                    "\nUsage: ";		/* Usage */

#define INDENT 18

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

        for (col = 1, i = 0; i < optList_size (); i++)
          if (optList[i].desc != NULL)
            {
              int             len = 0;

              if (!optList_isBool (i))
                {
                  len = optList_strlen (i);
                  if (len > 0)
                    len++;	/* account for space */
                }
#ifdef DEBUG_STRICT
              assert (optList[i].opt != NULL);
#endif
              len += 4 + strlen (optList[i].opt) + (optList_isBool (i) ? 2: 0);
              col += len;
              if (col > 79)
                {	/* assume regular width */
                  rxvt_log ("\n");
                  col = 1 + len;
                }

              rxvt_log (" [-%s%s", (optList_isBool (i) ? "/+" : ""), optList[i].opt);
              if (optList_strlen (i))
                rxvt_log (" %s]", optList[i].arg);
              else
                rxvt_log ("]");
            }
        break;

      case 1:			/* full command-line listing */
        rxvt_log (" [options] [-e command args]\n\nwhere options include:\n");

        for (i = 0; i < optList_size (); i++)
          if (optList[i].desc != NULL)
            {
#ifdef DEBUG_STRICT
              assert (optList[i].opt != NULL);
#endif
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

        for (i = 0; i < optList_size (); i++)
          if (optList[i].kw != NULL)
            rxvt_log ("  %s: %*s%s\n",
                    optList[i].kw,
                    (INDENT - strlen (optList[i].kw)), "", /* XXX */
                    (optList_isBool (i) ? "boolean" : optList[i].arg));
#ifdef KEYSYM_RESOURCE
        rxvt_log ("  " "keysym.sym" ": %*s%s\n",
                (INDENT - sizeof ("keysym.sym") + 1), "", /* XXX */
                "keysym");
#endif
        rxvt_log ("\n  -help to list options");
        break;
    }

  rxvt_log ("\n\n");
  rxvt_exit_failure ();
  /* NOTREACHED */
}

/*}}} */

/*{{{ get command-line options before getting resources */
void
rxvt_term::get_options (int argc, const char *const *argv)
{
  int             i, bad_option = 0;
  static const char On[3] = "ON", Off[4] = "OFF";

  for (i = 1; i < argc; i++)
    {
      unsigned int    entry, longopt = 0;
      const char     *flag, *opt;

      opt = argv[i];
#ifdef DEBUG_RESOURCES
      fprintf (stderr, "argv[%d] = %s: ", i, opt);
#endif
      if (*opt == '-')
        {
          flag = On;
          if (*++opt == '-')
            longopt = *opt++;	/* long option */
        }
      else if (*opt == '+')
        {
          flag = Off;
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
      for (entry = 0; entry < optList_size (); entry++)
        if ((optList[entry].kw && !strcmp (opt, optList[entry].kw))
            || (!longopt
                && optList[entry].opt && !strcmp (opt, optList[entry].opt)))
          break;

      if (entry < optList_size ())
        {
          if (optList_isReverse (entry))
            flag = flag == On ? Off : On;

          if (optList_strlen (entry))
            {
              /*
               * special cases are handled in main.c:main () to allow
               * X resources to set these values before we settle for
               * default values
               */

              if (optList[entry].doff != -1)
                rs[optList[entry].doff] = flag == On && argv[i+1]
                                          ? argv[++i] : 0;
            }
          else
            {		/* boolean value */
#ifdef DEBUG_RESOURCES
              fprintf (stderr, "boolean (%s,%s) = %s\n",
                      optList[entry].opt, optList[entry].kw, flag);
#endif
              if (flag == On)
                options |= (optList[entry].flag);
              else
                options &= ~ (optList[entry].flag);

              if (optList[entry].doff != -1)
                rs[optList[entry].doff] = flag;
            }
        }
      else
#ifdef KEYSYM_RESOURCE
        /* if (!strncmp (opt, "keysym.", sizeof ("keysym.") - 1)) */
        if (rxvt_Str_match (opt, "keysym."))
          {
            const char *str = argv[++i];

            if (str != NULL)
              parse_keysym (opt + sizeof ("keysym.") - 1, str);
          }
        else
#endif
          {
            bad_option = 1;
            rxvt_warn ("\"%s\": unknown or malformed option.\n", opt);
          }
    }

  if (bad_option)
    rxvt_usage (0);
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
/* ARGSUSED */
int
rxvt_define_key (XrmDatabase *database __attribute__((unused)),
                 XrmBindingList bindings __attribute__((unused)),
                 XrmQuarkList quarks,
                 XrmRepresentation *type __attribute__((unused)),
                 XrmValue *value,
                 XPointer closure __attribute__((unused)))
{
  int             last;

  for (last = 0; quarks[last] != NULLQUARK; last++)	/* look for last quark in list */
    ;
  last--;
  GET_R->parse_keysym (XrmQuarkToString (quarks[last]), (char *)value->addr);//D//TODO
  return False;
}

/*
 * look for something like this (XK_Delete)
 * rxvt*keysym.0xFFFF: "\177"
 *
 * arg will be
 *      NULL for ~/.Xdefaults and
 *      non-NULL for command-line options (need to allocate)
 */
#define NEWARGLIM	500	/* `reasonable' size */
int
rxvt_term::parse_keysym (const char *str, const char *arg)
{
  int             n, sym;
  char           *key_string, *newarg = NULL;
  char            newargstr[NEWARGLIM];

  if (arg == NULL)
    {
      if ((n = rxvt_Str_match (str, "keysym.")) == 0)
        return 0;
      str += n;		/* skip `keysym.' */
    }
  /* some scanf () have trouble with a 0x prefix */
  if (isdigit (str[0]))
    {
      if (str[0] == '0' && toupper (str[1]) == 'X')
        str += 2;
      if (arg)
        {
          if (sscanf (str, (strchr (str, ':') ? "%x:" : "%x"), &sym) != 1)
            return -1;
        }
      else
        {
          if (sscanf (str, "%x:", &sym) != 1)
            return -1;

          /* cue to ':', it's there since sscanf () worked */
          strncpy (newargstr, strchr (str, ':') + 1, NEWARGLIM - 1);
          newargstr[NEWARGLIM - 1] = '\0';
          newarg = newargstr;
        }
    }
  else
    {
      /*
       * convert keysym name to keysym number
       */
      strncpy (newargstr, str, NEWARGLIM - 1);
      newargstr[NEWARGLIM - 1] = '\0';
      if (arg == NULL)
        {
          if ((newarg = strchr (newargstr, ':')) == NULL)
            return -1;
          *newarg++ = '\0';	/* terminate keysym name */
        }
      if ((sym = XStringToKeysym (newargstr)) == None)
        return -1;
    }

  if (sym < 0xFF00 || sym > 0xFFFF)	/* we only do extended keys */
    return -1;
  sym &= 0xFF;
  if (Keysym_map[sym] != NULL)	/* already set ? */
    return -1;

  if (newarg == NULL)
    {
      strncpy (newargstr, arg, NEWARGLIM - 1);
      newargstr[NEWARGLIM - 1] = '\0';
      newarg = newargstr;
    }
  rxvt_Str_trim (newarg);
  if (*newarg == '\0' || (n = rxvt_Str_escaped (newarg)) == 0)
    return -1;
  MIN_IT (n, 255);
  key_string = (char *)rxvt_malloc ((n + 1) * sizeof (char));

  key_string[0] = n;
  strncpy (key_string + 1, newarg, n);
  Keysym_map[sym] = (unsigned char *)key_string;

  return 1;
}

# endif				/* KEYSYM_RESOURCE */

# ifndef USE_XGETDEFAULT
/*{{{ rxvt_get_xdefaults () */
/*
 * the matching algorithm used for memory-save fake resources
 */
void
rxvt_term::get_xdefaults (FILE *stream, const char *name)
{
  unsigned int len;
  char *str, buffer[256];

  if (stream == NULL)
    return;

  len = strlen (name);
  while ((str = fgets (buffer, sizeof (buffer), stream)) != NULL)
    {
      unsigned int    entry, n;

      while (*str && isspace (*str))
        str++;		/* leading whitespace */

      if ((str[len] != '*' && str[len] != '.')
          || (len && strncmp (str, name, len)))
        continue;
      str += (len + 1);	/* skip `name*' or `name.' */

# ifdef KEYSYM_RESOURCE
      if (!parse_keysym (str, NULL))
# endif				/* KEYSYM_RESOURCE */
        for (entry = 0; entry < optList_size (); entry++)
          {
            const char *kw = optList[entry].kw;

            if (kw == NULL)
              continue;

            n = strlen (kw);
            if (str[n] == ':' && rxvt_Str_match (str, kw))
              {
                /* skip `keyword:' */
                str += n + 1;
                rxvt_Str_trim (str);
                n = strlen (str);
                if (n && rs[optList[entry].doff] == NULL)
                  {
                    /* not already set */
                    int s;
                    char *p = 0;

                    for (int o = 0;;)
                      {
                        p = (char *)rxvt_realloc (p, o + n + 1);
                        memcpy (p + o, str, n);
                        o += n;
                        p[o] = 0;

                        if (o == 0 || p[o - 1] != '\\') // continuation line
                          break;

                        o--; // eat "\"

                        if ((str = fgets (buffer, sizeof (buffer), stream)) == NULL)
                          break;

                        rxvt_Str_trim (str);
                        n = strlen (str);
                      }

                    rs[optList[entry].doff] = p;
                    allocated.push_back (p);

                    if (optList_isBool (entry))
                      {
                        s = strcasecmp (str, "TRUE") == 0
                            || strcasecmp (str, "YES") == 0
                            || strcasecmp (str, "ON") == 0
                            || strcasecmp (str, "1") == 0;

                        if (optList_isReverse (entry))
                          s = !s;

                        if (s)
                          options |= optList[entry].flag;
                        else
                          options &= ~optList[entry].flag;
                      }
                  }

                break;
              }
          }
    }

  rewind (stream);
}

/*}}} */
# endif				/* ! USE_XGETDEFAULT */
#endif				/* NO_RESOURCES */

/*{{{ read the resources files */
/*
 * using XGetDefault () or the hand-rolled replacement
 */
/* ARGSUSED */
void
rxvt_term::extract_resources (Display *display __attribute__ ((unused)), const char *name)
{
#ifndef NO_RESOURCES

# if defined XAPPLOADDIR
#  if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
  /* Compute the path of the possibly available localized Rxvt file */
  char *localepath = NULL;

  if (locale != NULL)
    {	/* XXX: must limit length of string */
      localepath = (char *)rxvt_malloc (256);
      sprintf (localepath, XAPPLOADDIRLOCALE "/" RESCLASS,
              (int) (258 - sizeof (XAPPLOADDIRLOCALE) - sizeof (RESCLASS)),
              locale);	/* 258 = 255 + 4 (-.*s) - 1 (/) */
    }

  {
#  endif
# endif

# ifdef USE_XGETDEFAULT
    /*
     * get resources using the X library function
     */
    int entry;

#  ifdef XrmEnumOneLevel
    int i;
    char *displayResource, *xe;
    XrmName name_prefix[3];
    XrmClass class_prefix[3];
    XrmDatabase database, rdb1;
    char fname[1024];

    XrmInitialize ();
    database = NULL;

    /* Get any Xserver defaults */

    displayResource = XResourceManagerString (display);
    if (displayResource != NULL)
      database = XrmGetStringDatabase (displayResource);

#   ifdef HAVE_EXTRA_XRESOURCE_FILES
    /* Add in ~/.Xdefaults or ~/.Xresources */
    {
      char *ptr;

      if ((ptr = (char *)getenv ("HOME")) == NULL)
        ptr = ".";

      for (i = 0; i < (sizeof (xnames) / sizeof (xnames[0])); i++)
        {
          sprintf (fname, "%-.*s/%s", sizeof (fname) - strlen (xnames[i]) - 2,
                  ptr, xnames[i]);
          if ((rdb1 = XrmGetFileDatabase (fname)))
            {
              XrmMergeDatabases (rdb1, &database);
#    ifndef HAVE_BOTH_XRESOURCE_FILES
              break;
#    endif

            }
        }
    }
#   endif

    /* Add in XENVIRONMENT file */

    if ((xe = (char *)getenv ("XENVIRONMENT")) != NULL
        && (rdb1 = XrmGetFileDatabase (xe)) != NULL)
      XrmMergeDatabases (rdb1, &database);

    /* Add in Rxvt file */
#   if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
    if (localepath == NULL || (rdb1 = XrmGetFileDatabase (localepath)) == NULL)
#   endif
      rdb1 = XrmGetFileDatabase (XAPPLOADDIR "/" RESCLASS);

    if (rdb1 != NULL)
      XrmMergeDatabases (rdb1, &database);

    /* Add in $XAPPLRESDIR/Rxvt only; not bothering with XUSERFILESEARCHPATH */
    if ((xe = (char *)getenv ("XAPPLRESDIR")) != NULL)
      {
        sprintf (fname, "%-.*s/" RESCLASS, sizeof (fname)
                - sizeof (RESCLASS) - 2, xe);
        if ((rdb1 = XrmGetFileDatabase (fname)) != NULL)
          XrmMergeDatabases (rdb1, &database);
      }

    XrmSetDatabase (display, database);
#  endif

    /*
     * Query resources for options that affect us
     */
    for (entry = 0; entry < optList_size (); entry++)
      {
        int             s;
        char           *p, *p0;
        const char     *kw = optList[entry].kw;

        if (kw == NULL || rs[optList[entry].doff] != NULL)
          continue;		/* previously set */

        p = XGetDefault (display, name, kw);
        p0 = XGetDefault (display, "!INVALIDPROGRAMMENAMEDONTMATCH!", kw);
        if (p == NULL || (p0 && strcmp (p, p0) == 0))
          {
            p = XGetDefault (display, RESCLASS, kw);
#ifdef RESFALLBACK
            if (p == NULL || (p0 && strcmp (p, p0) == 0))
              p = XGetDefault (display, RESFALLBACK, kw);
#endif
          }

        if (p == NULL && p0)
          p = p0;

        if (p)
          {
            rs[optList[entry].doff] = p;

            if (optList_isBool (entry))
              {
                s = strcasecmp (p, "TRUE") == 0
                    || strcasecmp (p, "YES") == 0
                    || strcasecmp (p, "ON") == 0
                    || strcasecmp (p, "1") == 0;
                if (optList_isReverse (entry))
                  s = !s;
                if (s)
                  options |= (optList[entry].flag);
                else
                  options &= ~ (optList[entry].flag);
              }
          }
      }

    /*
     * [R5 or later]: enumerate the resource database
     */
#  ifdef XrmEnumOneLevel
#   ifdef KEYSYM_RESOURCE
    name_prefix[0] = XrmStringToName (name);
    name_prefix[1] = XrmStringToName ("keysym");
    name_prefix[2] = NULLQUARK;
    class_prefix[0] = XrmStringToName (RESCLASS);
    class_prefix[1] = XrmStringToName ("Keysym");
    class_prefix[2] = NULLQUARK;
    /* XXX: Need to check sizeof (rxvt_t) == sizeof (XPointer) */
    XrmEnumerateDatabase (XrmGetDatabase (display), name_prefix, class_prefix,
                          XrmEnumOneLevel, rxvt_define_key, NULL);
#    ifdef RESFALLBACK
    name_prefix[0] = XrmStringToName (RESFALLBACK);
    name_prefix[1] = XrmStringToName ("keysym");
    class_prefix[0] = XrmStringToName (RESFALLBACK);
    class_prefix[1] = XrmStringToName ("Keysym");
    /* XXX: Need to check sizeof (rxvt_t) == sizeof (XPointer) */
    XrmEnumerateDatabase (XrmGetDatabase (display), name_prefix, class_prefix,
                          XrmEnumOneLevel, rxvt_define_key, NULL);
#    endif
#   endif
#  endif

# else				/* USE_XGETDEFAULT */
    /* get resources the hard way, but save lots of memory */
    FILE           *fd = NULL;
    char           *home;

    if ((home = getenv ("HOME")) != NULL)
      {
        unsigned int    i, len = strlen (home) + 2;
        char           *f = NULL;

        for (i = 0; i < (sizeof (xnames) / sizeof (xnames[0])); i++)
          {
            f = (char *)rxvt_realloc (f, (len + strlen (xnames[i])) * sizeof (char));

            sprintf (f, "%s/%s", home, xnames[i]);

            if ((fd = fopen (f, "r")) != NULL)
              break;
          }
        free (f);
      }
    /*
    * The normal order to match resources is the following:
    * @ global resources (partial match, ~/.Xdefaults)
    * @ application file resources (XAPPLOADDIR/Rxvt)
    * @ class resources (~/.Xdefaults)
    * @ private resources (~/.Xdefaults)
    *
    * However, for the hand-rolled resources, the matching algorithm
    * checks if a resource string value has already been allocated
    * and won't overwrite it with (in this case) a less specific
    * resource value.
    *
    * This avoids multiple allocation.  Also, when we've called this
    * routine command-line string options have already been applied so we
    * needn't to allocate for those resources.
    *
    * So, search in resources from most to least specific.
    *
    * Also, use a special sub-class so that we can use either or both of
    * "XTerm" and "Rxvt" as class names.
    */

    get_xdefaults (fd, name);
    get_xdefaults (fd, RESCLASS);
#  ifdef RESFALLBACK
    get_xdefaults (fd, RESFALLBACK);
#  endif

#  if defined(XAPPLOADDIR) && defined(USE_XAPPLOADDIR)
    {
      FILE *ad = NULL;

#   if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)
      if (localepath == NULL || (ad = fopen (localepath, "r")) == NULL)
#   endif
        ad = fopen (XAPPLOADDIR "/" RESCLASS, "r");
      if (ad != NULL)
        {
          get_xdefaults (ad, RESCLASS);
          get_xdefaults (ad, "");
          fclose (ad);
        }
    }
#  endif			/* XAPPLOADDIR */

    get_xdefaults (fd, "");	/* partial match */
    if (fd != NULL)
      fclose (fd);
# endif				/* USE_XGETDEFAULT */

# if defined XAPPLOADDIR
#  if defined(HAVE_XSETLOCALE) || defined(HAVE_SETLOCALE)

  }

  /* Free the path of the possibly available localized Rxvt file */
  free (localepath);
#  endif
# endif

#endif				/* NO_RESOURCES */
}

/*}}} */
/*----------------------- end-of-file (C source) -----------------------*/
