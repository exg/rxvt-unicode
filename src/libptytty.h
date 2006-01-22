// This file is part of libptytty. Do not make local modifications.
// http://software.schmorp.de/pkg/libptytty

#ifndef LIBPTYTTY_H_ /* public libptytty header file */
#define LIBPTYTTY_H_

struct ptytty {
  int pty; // pty file descriptor; connected to rxvt
  int tty; // tty file descriptor; connected to child

  virtual ~ptytty ()
  {
  }

  virtual bool get () = 0;
  virtual void login (int cmd_pid, bool login_shell, const char *hostname) = 0;

  void close_tty ();
  bool make_controlling_tty ();
  void set_utf8_mode (bool on);

  static void init ();
  static ptytty *create (); // create a new pty object

  static void drop_privileges ();
  static void use_helper ();

  static bool send_fd (int socket, int fd);
  static int recv_fd (int socket);

protected:

  ptytty ()
  : pty(-1), tty(-1)
  {
  }
};

#endif

