/*
 * command.h
 */

#ifndef COMMAND_H_
#define COMMAND_H_

// STRING_MAX __MUST__ not be larger than what CBUFSIZ can hold.
#define STRING_MAX	2048	/* max string size for process_{dcs,osc}_seq () */

#define ESC_ARGS	32	/* max # of args for esc sequences */

#ifndef MULTICLICK_TIME
# define MULTICLICK_TIME	500
#endif
#ifndef SCROLLBAR_INITIAL_DELAY
# define SCROLLBAR_INITIAL_DELAY	0.33
#endif
#ifndef SCROLLBAR_CONTINUOUS_DELAY
# define SCROLLBAR_CONTINUOUS_DELAY	0.05
#endif

#ifdef SCROLL_ON_SHIFT
# define SCROLL_SHIFTKEY (shft)
# define NOSCROLL_SHIFTKEY 0
#else
# define SCROLL_SHIFTKEY 0
# define NOSCROLL_SHIFTKEY (shft)
#endif
#ifdef SCROLL_ON_CTRL
# define SCROLL_CTRLKEY  (ctrl)
# define NOSCROLL_CTRLKEY 0
#else
# define SCROLL_CTRLKEY 0
# define NOSCROLL_CTRLKEY (ctrl)
#endif
#ifdef SCROLL_ON_META
# define SCROLL_METAKEY  (meta)
# define NOSCROLL_METAKEY 0
#else
# define SCROLL_METAKEY 0
# define NOSCROLL_METAKEY (meta)
#endif
#define IS_SCROLL_MOD  ((SCROLL_SHIFTKEY || SCROLL_CTRLKEY || SCROLL_METAKEY) \
              && (!NOSCROLL_SHIFTKEY && !NOSCROLL_CTRLKEY && !NOSCROLL_METAKEY))


/*
 * ESC-Z processing:
 *
 * By stealing a sequence to which other xterms respond, and sending the
 * same number of characters, but having a distinguishable sequence,
 * we can avoid having a timeout (when not under an rxvt) for every login
 * shell to auto-set its DISPLAY.
 *
 * This particular sequence is even explicitly stated as obsolete since
 * about 1985, so only very old software is likely to be confused, a
 * confusion which can likely be remedied through termcap or TERM. Frankly,
 * I doubt anyone will even notice.  We provide a #ifdef just in case they
 * don't care about auto-display setting.  Just in case the ancient
 * software in question is broken enough to be case insensitive to the 'c'
 * character in the answerback string, we make the distinguishing
 * characteristic be capitalization of that character. The length of the
 * two strings should be the same so that identical read (2) calls may be
 * used.
 */
#define VT100_ANS	"\033[?1;2c"	/* vt100 answerback */
#ifndef ESCZ_ANSWER
# define ESCZ_ANSWER	VT100_ANS	/* obsolete ANSI ESC[c */
#endif

#endif /* _COMMAND_H_ */
