#ifndef RXVT_DAEMON_H
#define RXVT_DAEMON_H

struct rxvt_connection {
  int fd;

  static const char *unix_sockname ();

  typedef char[4] token;

  void send (const char *data, int len);
  void send (const char *data);
  void send (int data);

  bool recv (char *&data, int *len = 0);
  bool recv (token &data);
  bool recv (int &data);
};

#endif

