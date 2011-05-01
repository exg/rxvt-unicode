#ifndef RXVT_DAEMON_H
#define RXVT_DAEMON_H

#include <limits.h>

#ifndef PATH_MAX
# define PATH_MAX 16384
#endif

#include "rxvtutil.h"

struct rxvt_connection
{
  int fd;

  static char *unix_sockname ();

  void send (const char *data, int len);
  void send (const char *data);
  void send (int data);

  bool recv (auto_str &data, int *len = 0);
  bool recv (int &data);
};

#endif

