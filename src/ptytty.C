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
#include <csignal>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#if defined(HAVE_DEV_PTMX) && defined(HAVE_SYS_STROPTS_H)
# include <sys/stropts.h>      /* for I_PUSH */
#endif
#if defined(HAVE_ISASTREAM) && defined(HAVE_STROPTS_H)
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
          {
            /* slave now unlocked */
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

    res = openpty (&pfd, fd_tty, NULL, NULL, NULL);

    if (res != -1)
      {
        *ttydev = strdup (ttyname (*fd_tty));
        return pfd;
      }

    return -1;
  }

#elif defined(HAVE__GETPTY)

  static int
  get_pty (int *fd_tty, char **ttydev)
  {
    int pfd;
    char *slave;

    slave = _getpty (&pfd, O_RDWR | O_NONBLOCK | O_NOCTTY, 0622, 0);

    if (slave != NULL)
      {
        *ttydev = strdup (slave);
        return pfd;
      }

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

    return -1;
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
  int fd;

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
#if defined(HAVE_ISASTREAM) && defined(HAVE_STROPTS_H)
  if (isastream (fd_tty) == 1)
# endif
    {
      ioctl (fd_tty, I_PUSH, "ptem");
      ioctl (fd_tty, I_PUSH, "ldterm");
      ioctl (fd_tty, I_PUSH, "ttcompat");
    }
#endif

#ifdef TIOCSCTTY
  ioctl (fd_tty, TIOCSCTTY, NULL);
#else
  fd = open (ttyname (fd_tty), O_RDWR);
  if (fd >= 0)
    close (fd);
#endif

  fd = open ("/dev/tty", O_WRONLY);
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
        {
          /* change group ownership of tty to "tty" */
          mode = S_IRUSR | S_IWUSR | S_IWGRP;
          gid = gr->gr_gid;
        }
      else
#endif /* TTY_GID_SUPPORT */
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
  if (name)
    {
      chmod (name, RESTORE_TTY_MODE);
      chown (name, 0, ttyconf.gid);
    }

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

