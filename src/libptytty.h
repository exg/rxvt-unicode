// This file is part of libptytty. Do not make local modifications.
// http://software.schmorp.de/pkg/libptytty

#ifndef LIBPTYTTY_H_ /* public libptytty header file */
#define LIBPTYTTY_H_

#ifdef __cplusplus

// c++ api

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

#else

// c api

typedef void *PTYTTY;

int ptytty_pty (PTYTTY ptytty);
int ptytty_tty (PTYTTY ptytty);
void ptytty_delete (PTYTTY ptytty);
int ptytty_get (PTYTTY ptytty);
void ptytty_login (PTYTTY ptytty, int cmd_pid, bool login_shell, const char *hostname);

void ptytty_close_tty (PTYTTY ptytty);
int ptytty_make_controlling_tty (PTYTTY ptytty);
void ptytty_set_utf8_mode (PTYTTY ptytty, int on);

void ptytty_init ();
PTYTTY ptytty_create ();

void ptytty_drop_privileges ();
void ptytty_use_helper ();

#endif

#endif

