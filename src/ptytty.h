#ifndef PTYTTY_H
#define PTYTTY_H

#include "rxvt.h"

enum rxvt_privaction { IGNORE = 'i', SAVE = 's', RESTORE = 'r' };

struct rxvt_ptytty {
#ifndef RESET_TTY_TO_COMMON_DEFAULTS
  struct stat savestat; /* original status of our tty */
#endif
  void privileges (rxvt_privaction action);
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
};

#endif

