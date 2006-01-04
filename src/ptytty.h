#ifndef PTYTTY_H
#define PTYTTY_H

#include "feature.h"

#if defined(HAVE_GRANTPT) && defined(HAVE_UNLOCKPT)
# if defined(PTYS_ARE_GETPT) || defined(PTYS_ARE_PTMX)
#  define NO_SETOWNER_TTYDEV 1
# endif
#endif
#if defined(__CYGWIN32__)
# define NO_SETOWNER_TTYDEV 1
#endif

enum rxvt_privaction { IGNORE = 'i', SAVE = 's', RESTORE = 'r' };

struct rxvt_ptytty {
#ifndef RESET_TTY_TO_COMMON_DEFAULTS
  struct stat savestat; /* original status of our tty */
#endif
#ifndef NO_SETOWNER_TTYDEV
  void privileges (rxvt_privaction action);
  bool saved;
#endif
public:
  int pty; // pty file descriptor; connected to rxvt
  int tty; // tty file descriptor; connected to child
  char *name;

  rxvt_ptytty ();
  ~rxvt_ptytty ();

  bool get ();
  void put ();

  void close_tty ();

  bool make_controlling_tty ();
  void set_utf8_mode (bool on);
};

#endif

