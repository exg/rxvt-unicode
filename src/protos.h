/* Include prototypes for all files */
#include "command.extpro"

#include "defaultfont.extpro"

#include "init.extpro"

#ifdef UTMP_SUPPORT
# include "logging.extpro"
#endif

#include "main.extpro"

#ifdef MENUBAR
# include "menubar.extpro"
#endif

#include "misc.extpro"

#ifdef DISPLAY_IS_IP
# include "netdisp.extpro"
#endif

#include "ptytty.extpro"

#if !defined(NO_STRINGS) && !defined(HAVE_STRING_H)
# include "strings.extpro"
#endif

#include "screen.extpro"

#include "scrollbar.extpro"
#ifdef RXVT_SCROLLBAR
# include "scrollbar-rxvt.extpro"
#endif
#ifdef NEXT_SCROLLBAR
# include "scrollbar-next.extpro"
#endif
#ifdef XTERM_SCROLLBAR
# include "scrollbar-xterm.extpro"
#endif

#include "xdefaults.extpro"

#ifdef XPM_BACKGROUND
# include "xpm.extpro"
#endif
