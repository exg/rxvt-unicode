/*
 * File:	feature.h
 *
 * Compile-time configuration.
 *-----------------------------------------------------------------------
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
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
 *
 *----------------------------------------------------------------------*/
#ifndef FEATURE_H
#define FEATURE_H

#ifndef X11USRLIBDIR
# define X11USRLIBDIR		"/usr/lib"
#endif
#ifndef X11LIBDIR
# define X11LIBDIR		X11USRLIBDIR "/X11"
#endif
#ifndef XAPPLOADDIR
# define XAPPLOADDIR		X11LIBDIR "/app-defaults"
# define XAPPLOADDIRLOCALE	X11LIBDIR "/%s/app-defaults"
#endif

/*-----------------------SCREEN OPTIONS AND COLOURS---------------------*/

/*
 * NOTE:
 *
 * Some of these configuration options have not been tested within the new
 * rxvt-unicode framework. Changing them should work, might have no effect,
 * destroy your disks or have any other effects. You may freely try (and
 * report bugs, too!), but don't _expect_ them to work.
 */

/*
 * The cursor blink interval, in seconds.
 */
#define CURSOR_BLINK_INTERVAL 0.5

/*
 * The text blink interval, in seconds.
 */
#define TEXT_BLINK_INTERVAL CURSOR_BLINK_INTERVAL

/*
 * Avoid enabling the colour cursor (-cr, cursorColor, cursorColor2)
 */
/* #define NO_CURSORCOLOR */

/*
 * Make colours match xterm colours instead of `traditional' rxvt colours
 */
#define XTERM_COLORS

/*
 * Disable separate colours for bold, underline and reverse video
 */
/* #define NO_BOLD_UNDERLINE_REVERSE */

/*
 * Define maximum possible columns and rows
 */
#define MAX_COLS	10000
#define MAX_ROWS	10000

/*
 * Define maximum possible savelines
 */
#define MAX_SAVELINES   10000000

/*
 * Define to remove support for XCopyArea () support.  XCopyArea () is useful
 * for scrolling on non-local X displays
 */
#define NO_SLOW_LINK_SUPPORT

/*
 * Allow 80/132 mode switching on startup
 */
/* #define ALLOW_132_MODE */

/*---------------------------------KEYS---------------------------------*/

/*
 * Enable the keysym resource which allows you to define strings associated
 * with various KeySyms (0xFF00 - 0xFFFF).
 * Required by perl.
 */
#if ENABLE_FRILLS || ENABLE_PERL
# define KEYSYM_RESOURCE
#endif

/*
 * Modifier/s to use to allow up/down arrows and Prior/Next keys
 * to scroll single or page-fulls
 */
#define SCROLL_ON_SHIFT
/* #define SCROLL_ON_CTRL */
/* #define SCROLL_ON_META */

/*
 * Allow scrolling with modifier+Up/Down keys, in addition
 * to modifier+Prior/Next? (modifier is controlled with
 * SCROLL_ON_* defines above.).
 * Also for modifier+Home/End keys to move to top/bottom
 */
/* #define SCROLL_ON_UPDOWN_KEYS */
/* #define SCROLL_ON_HOMEEND_KEYS */

/*
 * Allow unshifted Next/Prior keys to scroll forward/back
 * (in addition to shift+Next/shift+Prior)       --pjh
 */
/* #define UNSHIFTED_SCROLLKEYS */

/* (Hops) Set to choose a number of lines of context between pages
 *      (rather than a proportion (1/5) of savedlines buffer)
 *      when paging the savedlines with SHIFT-{Prior,Next} keys.
 */
#define PAGING_CONTEXT_LINES 1

/*
 * Have either Ctrl+Tab or Mod4+Tab emit \e\t
 * Useful when window manager grabs Alt+Tab   -- mg
 */
/* #define CTRL_TAB_MAKES_META */
/* #define MOD4_TAB_MAKES_META */

/*--------------------------------MOUSE---------------------------------*/
/*
 * Disable sending escape sequences (up, down, page up/down)
 * from the scrollbar when XTerm mouse reporting is enabled
 */
/* #define NO_SCROLLBAR_REPORT */

/*
 * Have mouse reporting include double-click info for button1
 */
/* #define MOUSE_REPORT_DOUBLECLICK */

/*
 * Set delay between multiple click events [default: 500 milliseconds]
 */
/* #define MULTICLICK_TIME 500 */

/*
 * Time factor to slow down a `jumpy' mouse.  Motion isn't recognised until
 * this long after the last mouse click [default: 50 milliseconds]
 */
#define MOUSE_THRESHOLD		50

/*
 * Set delay periods for continuous scrolling with scrollbar buttons
 */
/* #define SCROLLBAR_INITIAL_DELAY 0.33 */
/* #define SCROLLBAR_CONTINUOUS_DELAY 0.05 */

/*
 * The speed of selection scrolling is proportional to the distance
 * the mouse pointer is out of the text window.  This is the max
 * number of lines to scroll at a time.
 */
#define SELECTION_SCROLL_MAX_LINES 8

/*
 * The number of lines (measured in character's heights, not pixels)
 * the pointer must be out of the window for each increase in the
 * number of lines scrolled.
 */
#define SELECTION_SCROLL_LINE_SPEEDUP 3

/*--------------------------------MISC----------------------------------*/
/*
 * Only log in wtmp and lastlog files when we're a login shell (-ls option)
 */
#define LOG_ONLY_ON_LOGIN

/*--------------------------------BELL----------------------------------*/
/*
 * Disable all bell indications
 */
/* #define NO_BELL */

/*
 * Disable automatic de-iconify when a bell is received
 */
/* #define NO_MAPALERT */

/*
 * Have mapAlert behaviour selectable with mapAlert resource
 */
#define MAPALERT_OPTION

/*-----------------------------SCROLL BAR-------------------------------*/
/*
 * Choose the rxvt style scrollbar width
 * - should be an even number [default: 10]
 */
/* #define SB_WIDTH_RXVT 10 */

/*
 * Minimum and maximum widths of the scrollbar (all styles)
 */
#define SB_WIDTH_MINIMUM 	5
#define SB_WIDTH_MAXIMUM 	100

/*
 * rxvt scrollbar shadow width in pixels, must be 1 or 2
 */
#define SHADOW_WIDTH 1

/*
 * clicking above or below the scrollbar slider (all styles minus
 * xterm) will scroll by (height - 1) rather than (height / 4).
 */
#define RXVT_SCROLL_FULL 1

/*
 * (Hops) draw an internal border line on inside edge of the rxvt scrollbar
 */
/* #define SB_BORDER */

/*---------------------------MULTILINGUAL-------------------------------*/
/*
 * Allow run-time selection of Meta (Alt) to set the 8th bit on
 */
#define META8_OPTION

/*---------------------------DISPLAY OPTIONS----------------------------*/
/*
 * Force local connection to be socket (or other local) communication
 */
/* #define LOCAL_X_IS_UNIX */

/*
 * Have DISPLAY environment variable & "\E[7n" transmit display with IP number
 */
/* #define DISPLAY_IS_IP */

/*
 * Change what ESC Z transmits instead of the default "\E[?1;2c"
 */
/* #define ESCZ_ANSWER	"\033[?1;2C" */

/*
 * Allow foreground/background colour to be changed with xterm
 * operating system commands.
 */
#define XTERM_COLOR_CHANGE

/*
 * Remove secondary screen's independent cursor position, a la xterm
 */
/* #define NO_SECONDARY_SCREEN_CURSOR */

/*
 * Provide termcap/terminfo bw support (wrap backwards on cub1)
 */
#define TERMCAP_HAS_BW 1

/*
 * The duration of the visual bell flash in s. The default of 20ms
 * corresponds to the delay given in the terminfo flash code.
 */
#define VISUAL_BELL_DURATION .020

/*--------------------------------OTHER---------------------------------*/

/*
 * Enable the linux yield/usleep hack, which can dramatically improve
 * performance by working around the linux kernel tty ratelimit bug.
 * Unfortunately, it seems screen is negatively affected by this on some
 * machines, so it is disabled by default. Use freebsd or any other kernel
 * that doesn't suffer form this bug and it will be fast either way.
 *
 * See command.C for details.
 */
#if __linux__
# define LINUX_YIELD_HACK 0
#endif

/* DEFAULT RESOURCES VALUES */

/*
 * Define default colours for certain items.  If you have a low colour
 * display, then consider using colours which are already pre-allocated:
 *
 *   Black		(#000000)
 *   Red3		(#CD0000)
 *   Green3		(#00CD00)
 *   Yellow3		(#CDCD00)
 *   Blue3		(#0000CD)
 *   Magenta3		(#CD00CD)
 *   Cyan3		(#00CDCD)
 *   AntiqueWhite	(#FAEBD7)
 *   Grey25		(#404040)
 *   Red		(#FF0000)
 *   Green		(#00FF00)
 *   Yellow		(#FFFF00)
 *   Blue		(#0000FF)
 *   Magenta		(#FF00FF)
 *   Cyan		(#00FFFF)
 *   White		(#FFFFFF)
 */
/* These colours MUST be defined */
#define COLOR_FOREGROUND	"rgb:00/00/00"
#define COLOR_BACKGROUND	"rgb:ff/ff/ff"
#define COLOR_SCROLLBAR		"rgb:b2/b2/b2"	/* scrollColor match Netscape */
#define COLOR_SCROLLTROUGH	"rgb:96/96/96"

/*
 * The cursor colours are special.  Be very careful about setting these:
 * foreground/background colours may be modified by command line or resources
 * prior to this allocation.  Also, they are not valid if NO_CURSORCOLOR is
 * defined
 */
#define COLOR_CURSOR_FOREGROUND	NULL	/* if NULL, use background colour */
#define COLOR_CURSOR_BACKGROUND	NULL	/* if NULL, use foreground colour */

/*
 * Printer pipe which will be used for emulation of attached vt100 printer
 */
#define PRINTPIPE	"lpr"

/*
 * Define defaults for backspace and delete keys - unless they have been
 * configured out with --disable-backspace-key / --disable-delete-key
 */
#define DEFAULT_BACKSPACE	"DEC"		/* SPECIAL */
#define DEFAULT_DELETE		"\033[3~"

/*
 * Default separating chars for multiple-click selection
 * Space and tab are separate separating characters and are not settable
 */
#define CUTCHARS	"\"&'()*,;<=>?@[\\]^`{|}"

/*
 * Width of the term internal border
 */
#define INTERNALBORDERWIDTH	2

/*
 * Width of the term external border
 */
#define EXTERNALBORDERWIDTH	0

/*
 * Default number of extra dots between lines
 */
#define LINESPACE	0

/*
 * Default number of extra dots between columns
 */
#define LETTERSPACE	0

/*
 * Default number of lines in the scrollback buffer
 */
#define SAVELINES	1000

#endif

