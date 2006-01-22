// This file is part of libptytty. Do not make local modifications.
// http://software.schmorp.de/pkg/libptytty

/*----------------------------------------------------------------------*
 * File:	ptytty.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004-2006 Marc Lehmann <pcg@goof.com>
 * Copyright (c) 2006      Emanuele Giaquinta <e.giaquinta@glauco.it>
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

#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#if defined(HAVE_DEV_PTMX) && defined(HAVE_SYS_STROPTS_H)
# include <sys/stropts.h>      /* for I_PUSH */
#endif
#ifdef HAVE_ISASTREAM
# include <stropts.h>
#endif
#if defined(HAVE_PTY_H)
# include <pty.h>
#elif defined(HAVE_LIBUTIL_H)
# include <libutil.h>
#elif defined(HAVE_UTIL_H)
# include <util.h>
#endif
#ifdef TTY_GID_SUPPORT
#include <grp.h>
#endif

#include <cstdio>

/////////////////////////////////////////////////////////////////////////////

/* ------------------------------------------------------------------------- *
 *                  GET PSEUDO TELETYPE - MASTER AND SLAVE                   *
 * ------------------------------------------------------------------------- */
/*
 * Returns pty file descriptor, or -1 on failure 
 * If successful, ttydev is set to the name of the slave device.
 * fd_tty _may_ also be set to an open fd to the slave device
 */
#if defined(UNIX98_PTY)
static int
get_pty (int *fd_tty, char **ttydev)
{
  int pfd;

# if defined(HAVE_GETPT)
  pfd = getpt();
# elif defined(HAVE_POSIX_OPENPT)
  pfd = posix_openpt (O_RDWR);
# else
  pfd = open (CLONE_DEVICE, O_RDWR | O_NOCTTY, 0);
# endif
  if (pfd >= 0)
    {
      if (grantpt (pfd) == 0	/* change slave permissions */
          && unlockpt (pfd) == 0)
        {	/* slave now unlocked */
          *ttydev = strdup (ptsname (pfd));	/* get slave's name */
          return pfd;
        }

      close (pfd);
    }

  return -1;
}
#elif defined(HAVE_OPENPTY)
static int
get_pty (int *fd_tty, char **ttydev)
{
  int pfd;
  int res;
  char tty_name[32];
  
  res = openpty (&pfd, fd_tty, tty_name, NULL, NULL);
  if (res != -1)
    {
      *ttydev = strdup (tty_name);
      return pfd;
    }

  return -1;
}
#elif defined(HAVE__GETPTY)
static int
get_pty (int *fd_tty, char **ttydev)
{
  int pfd;

  *ttydev = _getpty (&pfd, O_RDWR | O_NONBLOCK | O_NOCTTY, 0622, 0);
  if (*ttydev != NULL)
    return pfd;

  return -1;
}
#elif defined(HAVE_DEV_PTC)
static int
get_pty (int *fd_tty, char **ttydev)
{
  int pfd;

  if ((pfd = open ("/dev/ptc", O_RDWR | O_NOCTTY, 0)) >= 0)
    {
      *ttydev = strdup (ttyname (pfd));
      return pfd;
    }

  return -1;
}
#elif defined(HAVE_DEV_CLONE)
static int
get_pty (int *fd_tty, char **ttydev)
{
  int pfd;

  if ((pfd = open ("/dev/ptym/clone", O_RDWR | O_NOCTTY, 0)) >= 0)
    {
      *ttydev = strdup (ptsname (pfd));
      return pfd;
    }

  return -1;
}
#else
/* Based on the code in openssh/openbsd-compat/bsd-openpty.c */
static int
get_pty (int *fd_tty, char **ttydev)
{
  int pfd;
  int i;
  char pty_name[32];
  char tty_name[32];
  const char *majors = "pqrstuvwxyzabcde";
  const char *minors = "0123456789abcdef";
  for (i = 0; i < 256; i++)
    {
      snprintf(pty_name, 32, "/dev/pty%c%c", majors[i / 16], minors[i % 16]);
      snprintf(tty_name, 32, "/dev/tty%c%c", majors[i / 16], minors[i % 16]);
      if ((pfd = open (pty_name, O_RDWR | O_NOCTTY, 0)) == -1)
        {
	  snprintf(pty_name, 32, "/dev/ptyp%d", i);
	  snprintf(tty_name, 32, "/dev/ttyp%d", i);
	  if ((pfd = open (pty_name, O_RDWR | O_NOCTTY, 0)) == -1)
	    continue;
        }
      if (access (tty_name, R_OK | W_OK) == 0)
        {
	  *ttydev = strdup (tty_name);
	  return pfd;
        }

      close (pfd);
    }
}
#endif

/*----------------------------------------------------------------------*/
/*
 * Returns tty file descriptor, or -1 on failure 
 */
static int
get_tty (char *ttydev)
{
  return open (ttydev, O_RDWR | O_NOCTTY, 0);
}

/*----------------------------------------------------------------------*/
/*
 * Make our tty a controlling tty so that /dev/tty points to us
 */
static int
control_tty (int fd_tty)
{
  setsid ();

#if defined(HAVE_DEV_PTMX) && defined(I_PUSH)
  /*
   * Push STREAMS modules:
   *    ptem: pseudo-terminal hardware emulation module.
   *    ldterm: standard terminal line discipline.
   *    ttcompat: V7, 4BSD and XENIX STREAMS compatibility module.
   *
   * After we push the STREAMS modules, the first open () on the slave side
   * (i.e. the next section between the dashes giving us "tty opened OK")
   * should make the "ptem" (or "ldterm" depending upon either which OS
   * version or which set of manual pages you have) module give us a
   * controlling terminal.  We must already have close ()d the master side
   * fd in this child process before we push STREAMS modules on because the
   * documentation is really unclear about whether it is any close () on
   * the master side or the last close () - i.e. a proper STREAMS dismantling
   * close () - on the master side which causes a hang up to be sent
   * through - Geoff Wing
   */
# ifdef HAVE_ISASTREAM
  if (isastream (fd_tty) == 1)
# endif
    {
      ioctl (fd_tty, I_PUSH, "ptem");
      ioctl (fd_tty, I_PUSH, "ldterm");
      ioctl (fd_tty, I_PUSH, "ttcompat");
    }
#endif

  ioctl (fd_tty, TIOCSCTTY, NULL);

  int fd = open ("/dev/tty", O_WRONLY);
  if (fd < 0)
    return -1; /* fatal */

  close (fd);

  return 0;
}

void
ptytty::close_tty ()
{
  if (tty < 0)
    return;

  close (tty);
  tty = -1;
}

bool
ptytty::make_controlling_tty ()
{
  return control_tty (tty) >= 0;
}

void
ptytty::set_utf8_mode (bool on)
{
#ifdef IUTF8
  if (pty < 0)
    return;

  struct termios tio;

  if (tcgetattr (pty, &tio) != -1)
    {
      tcflag_t new_cflag = tio.c_iflag;

      if (on)
        new_cflag |= IUTF8;
      else
        new_cflag &= ~IUTF8;

      if (new_cflag != tio.c_iflag)
        {
          tio.c_iflag = new_cflag;
          tcsetattr (pty, TCSANOW, &tio);
        }
    }
#endif
}

static struct ttyconf {
  gid_t gid;
  mode_t mode;

  ttyconf ()
    {
#ifdef TTY_GID_SUPPORT
      struct group *gr = getgrnam ("tty");

      if (gr)
        {           /* change group ownership of tty to "tty" */
          mode = S_IRUSR | S_IWUSR | S_IWGRP;
          gid = gr->gr_gid;
        }
      else
#endif                          /* TTY_GID_SUPPORT */
        {
          mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH;
          gid = 0;
        }
    }
} ttyconf;

ptytty_unix::ptytty_unix ()
{
  name = 0;
#if UTMP_SUPPORT
  cmd_pid = 0;
#endif
}

ptytty_unix::~ptytty_unix ()
{
#if UTMP_SUPPORT
  logout ();
#endif
  put ();
}

void
ptytty_unix::put ()
{
  chmod (name, RESTORE_TTY_MODE);
  chown (name, 0, ttyconf.gid);

  close_tty ();

  if (pty >= 0)
    close (pty);

  free (name);

  pty = tty = -1;
  name = 0;
}

bool
ptytty_unix::get ()
{
  /* get master (pty) */
  if ((pty = get_pty (&tty, &name)) < 0)
    return false;

  fcntl (pty, F_SETFL, O_NONBLOCK);

  /* get slave (tty) */
  if (tty < 0)
    {
#ifndef NO_SETOWNER_TTYDEV
      chown (name, getuid (), ttyconf.gid);      /* fail silently */
      chmod (name, ttyconf.mode);
# ifdef HAVE_REVOKE
      revoke (name);
# endif
#endif

      if ((tty = get_tty (name)) < 0)
        {
          put ();
          return false;
        }
    }

  return true;
}

#if PTYTTY_HELPER

static int sock_fd = -1;
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

bool
ptytty_proxy::get ()
{
  command cmd;

  cmd.type = command::get;

  write (sock_fd, &cmd, sizeof (cmd));

  if (read (sock_fd, &id, sizeof (id)) != sizeof (id))
    ptytty_fatal ("protocol error while creating pty using helper process, aborting.\n");

  if (!id)
    return false;

  if ((pty = recv_fd (sock_fd)) < 0
      || (tty = recv_fd (sock_fd)) < 0)
    ptytty_fatal ("protocol error while reading pty/tty fds from helper process, aborting.\n");

  return true;
}

void
ptytty_proxy::login (int cmd_pid, bool login_shell, const char *hostname)
{
  command cmd;

  cmd.type = command::login;
  cmd.id = id;
  cmd.cmd_pid = cmd_pid;
  cmd.login_shell = login_shell;
  strncpy (cmd.hostname, hostname, sizeof (cmd.hostname));

  write (sock_fd, &cmd, sizeof (cmd));
}

ptytty_proxy::~ptytty_proxy ()
{
  if (id)
    {
      command cmd;

      cmd.type = command::destroy;
      cmd.id = id;

      write (sock_fd, &cmd, sizeof (cmd));
    }
}

static
void serve ()
{
  command cmd;
  vector<ptytty *> ptys;

  while (read (sock_fd, &cmd, sizeof (command)) == sizeof (command))
    {
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
          if (find (ptys.begin (), ptys.end (), cmd.id))
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
    }

  // destroy all ptys
  for (vector<ptytty *>::iterator i = ptys.end (); i-- > ptys.begin (); )
    delete *i;
}

void
ptytty::use_helper ()
{
  int pid = getpid ();

  if (sock_fd >= 0 && pid == owner_pid)
    return;

  owner_pid = pid;

  int sv[2];

  if (socketpair (AF_UNIX, SOCK_STREAM, 0, sv))
    ptytty_fatal ("could not create socket to communicate with pty/sessiondb helper, aborting.\n");

  helper_pid = fork ();

  if (helper_pid < 0)
    ptytty_fatal ("could not create pty/sessiondb helper process, aborting.\n");

  if (helper_pid)
    {
      // client, process
      sock_fd = sv[0];
      close (sv[1]);
      fcntl (sock_fd, F_SETFD, FD_CLOEXEC);
    }
  else
    {
      // server, pty-helper
      sock_fd = sv[1];

      chdir ("/");

      for (int fd = 0; fd < 1023; fd++)
        if (fd != sock_fd)
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
  if (helper_pid && getpid () == owner_pid)
    // use helper process
    return new ptytty_proxy;
  else
#endif
    return new ptytty_unix;
}

void
ptytty::init ()
{
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
      ptytty_warn ("running setuid/setgid without pty helper compiled in, continuing unprivileged.\n");
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
#endif

  if (uid != geteuid ()
      || gid != getegid ())
    ptytty_fatal ("unable to drop privileges, aborting.\n");
}

