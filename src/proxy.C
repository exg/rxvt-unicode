// This file is part of libptytty. Do not make local modifications.
// http://software.schmorp.de/pkg/libptytty

/*----------------------------------------------------------------------*
 * File:	proxy.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2006      Marc Lehmann <pcg@goof.com>
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
 *---------------------------------------------------------------------*/

#include "../config.h"

#include "ptytty.h"

#include <csignal>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// helper/proxy support

#if PTYTTY_HELPER

static int sock_fd = -1, lock_fd = -1;
static int helper_pid, owner_pid;

struct command
{
  enum { get, login, destroy } type;

  ptytty *id;

  bool login_shell;
  int cmd_pid;
  char hostname[512]; // arbitrary, but should be plenty
};

struct ptytty_proxy : ptytty
{
  ptytty *id;

  ptytty_proxy ()
  : id(0)
  {
  }

  ~ptytty_proxy ();

  bool get ();
  void login (int cmd_pid, bool login_shell, const char *hostname);
};

#if PTYTTY_REENTRANT
# define NEED_TOKEN do { char ch; read  (lock_fd, &ch, 1); } while (0)
# define GIVE_TOKEN do { char ch; write (lock_fd, &ch, 1); } while (0)
#else
# define NEED_TOKEN (void)0
# define GIVE_TOKEN (void)0
#endif

bool
ptytty_proxy::get ()
{
  NEED_TOKEN;

  command cmd;

  cmd.type = command::get;

  write (sock_fd, &cmd, sizeof (cmd));

  if (read (sock_fd, &id, sizeof (id)) != sizeof (id))
    ptytty_fatal ("protocol error while creating pty using helper process, aborting.\n");

  if (!id)
    {
      GIVE_TOKEN;
      return false;
    }

  if ((pty = recv_fd (sock_fd)) < 0
      || (tty = recv_fd (sock_fd)) < 0)
    ptytty_fatal ("protocol error while reading pty/tty fds from helper process, aborting.\n");

  GIVE_TOKEN;
  return true;
}

void
ptytty_proxy::login (int cmd_pid, bool login_shell, const char *hostname)
{
  NEED_TOKEN;

  command cmd;

  cmd.type = command::login;
  cmd.id = id;
  cmd.cmd_pid = cmd_pid;
  cmd.login_shell = login_shell;
  strncpy (cmd.hostname, hostname, sizeof (cmd.hostname));

  write (sock_fd, &cmd, sizeof (cmd));

  GIVE_TOKEN;
}

ptytty_proxy::~ptytty_proxy ()
{
  if (id)
    {
      close_tty ();

      if (pty >= 0)
        close (pty);

      NEED_TOKEN;

      command cmd;

      cmd.type = command::destroy;
      cmd.id = id;

      write (sock_fd, &cmd, sizeof (cmd));

      GIVE_TOKEN;
    }
}

static
void serve ()
{
  command cmd;
  vector<ptytty *> ptys;

  for (;;)
    {
      GIVE_TOKEN;

      if (read (sock_fd, &cmd, sizeof (command)) != sizeof (command))
        break;

      if (cmd.type == command::get)
        {
          // -> id ptyfd ttyfd
          cmd.id = new ptytty_unix;

          if (cmd.id->get ())
            {
              write (sock_fd, &cmd.id, sizeof (cmd.id));
              ptys.push_back (cmd.id);

              ptytty::send_fd (sock_fd, cmd.id->pty);
              ptytty::send_fd (sock_fd, cmd.id->tty);
            }
          else
            {
              delete cmd.id;
              cmd.id = 0;
              write (sock_fd, &cmd.id, sizeof (cmd.id));
            }
        }
      else if (cmd.type == command::login)
        {
#if UTMP_SUPPORT
          if (find (ptys.begin (), ptys.end (), cmd.id) != ptys.end ())
            {
              cmd.hostname[sizeof (cmd.hostname) - 1] = 0;
              cmd.id->login (cmd.cmd_pid, cmd.login_shell, cmd.hostname);
            }
#endif
        }
      else if (cmd.type == command::destroy)
        {
          vector<ptytty *>::iterator pty = find (ptys.begin (), ptys.end (), cmd.id);

          if (pty != ptys.end ())
            {
              delete *pty;
              ptys.erase (pty);
            }
        }
      else
        break;

      NEED_TOKEN;
    }

  // destroy all ptys
  for (vector<ptytty *>::iterator i = ptys.end (); i-- > ptys.begin (); )
    delete *i;
}

void
ptytty::use_helper ()
{
#ifndef PTYTTY_NO_PID_CHECK
  int pid = getpid ();
#endif

  if (sock_fd >= 0
#ifndef PTYTTY_NO_PID_CHECK
      && pid == owner_pid
#endif
      )
    return;

#ifndef PTYTTY_NO_PID_CHECK
  owner_pid = pid;
#endif

  int sv[2];

  if (socketpair (AF_UNIX, SOCK_STREAM, 0, sv))
    ptytty_fatal ("could not create socket to communicate with pty/sessiondb helper, aborting.\n");

#ifdef PTYTTY_REENTRANT
  int lv[2];

  if (socketpair (AF_UNIX, SOCK_STREAM, 0, lv))
    ptytty_fatal ("could not create socket to communicate with pty/sessiondb helper, aborting.\n");
#endif

  helper_pid = fork ();

  if (helper_pid < 0)
    ptytty_fatal ("could not create pty/sessiondb helper process, aborting.\n");

  if (helper_pid)
    {
      // client, process
      sock_fd = sv[0];
      close (sv[1]);
      fcntl (sock_fd, F_SETFD, FD_CLOEXEC);
#ifdef PTYTTY_REENTRANT
      lock_fd = lv[0];
      close (lv[1]);
      fcntl (lock_fd, F_SETFD, FD_CLOEXEC);
#endif
    }
  else
    {
      // server, pty-helper
      sock_fd = sv[1];
#ifdef PTYTTY_REENTRANT
      lock_fd = lv[1];
#endif

      chdir ("/");

      signal (SIGHUP,  SIG_IGN);
      signal (SIGTERM, SIG_IGN);
      signal (SIGINT,  SIG_IGN);
      signal (SIGPIPE, SIG_IGN);

      for (int fd = 0; fd < 1023; fd++)
        if (fd != sock_fd && fd != lock_fd)
          close (fd);

      serve ();
      _exit (EXIT_SUCCESS);
    }
}

#endif

ptytty *
ptytty::create ()
{
#if PTYTTY_HELPER
  if (helper_pid
# ifndef PTYTTY_NO_PID_CHECK
      && getpid () == owner_pid
# endif
      )
    // use helper process
    return new ptytty_proxy;
  else
#endif
    return new ptytty_unix;
}

void
ptytty::sanitise_stdfd ()
{
  // sanitise stdin/stdout/stderr to point to *something*.
  for (int fd = 0; fd <= 2; ++fd)
    if (fcntl (fd, F_GETFL) < 0 && errno == EBADF)
      {
        int fd2 = open ("/dev/tty", fd ? O_WRONLY : O_RDONLY);

        if (fd2 < 0)
          fd2 = open ("/dev/null", fd ? O_WRONLY : O_RDONLY);

        if (fd2 != fd)
          abort ();
      }
}

void
ptytty::init ()
{
  sanitise_stdfd ();

  uid_t uid = getuid ();
  gid_t gid = getgid ();

  // before doing anything else, check for setuid/setgid operation,
  // start the helper process and drop privileges
  if (uid != geteuid ()
      || gid != getegid ())
    {
#if PTYTTY_HELPER
      use_helper ();
#else
      ptytty_warn ("running setuid/setgid without pty helper compiled in, continuing unprivileged.\n", 0);
#endif

      drop_privileges ();
    }
}

void
ptytty::drop_privileges ()
{
  uid_t uid = getuid ();
  gid_t gid = getgid ();

  // drop privileges
#if HAVE_SETRESUID
  setresgid (gid, gid, gid);
  setresuid (uid, uid, uid);
#elif HAVE_SETREUID
  setregid (gid, gid);
  setreuid (uid, uid);
#elif HAVE_SETUID
  setgid (gid);
  setuid (uid);
#else
# error no way to drop privileges, configure failed?
#endif

  if (uid != geteuid ()
      || gid != getegid ())
    ptytty_fatal ("unable to drop privileges, aborting.\n");
}

