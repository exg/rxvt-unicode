/*--------------------------------*-C-*---------------------------------*
 * File:	ptytty.c
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
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

#include "../config.h"		/* NECESSARY */

/*
 * Try to be self-contained except for the above autoconfig'd defines
 */

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#if defined(HAVE_STRING_H)
# include <string.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#if defined(PTYS_ARE_PTMX) && !defined(__CYGWIN32__)
# include <sys/stropts.h>      /* for I_PUSH */
#endif

#ifdef DEBUG_TTY
# define D_TTY(x)		fprintf x ; fputc('\n', stderr) ; fflush(stderr)
#else
# define D_TTY(x)
#endif

/* ------------------------------------------------------------------------- *
 *                  GET PSEUDO TELETYPE - MASTER AND SLAVE                   *
 * ------------------------------------------------------------------------- */
/*
 * Returns pty file descriptor, or -1 on failure 
 * If successful, ttydev is set to the name of the slave device.
 * fd_tty _may_ also be set to an open fd to the slave device
 */
/* EXTPROTO */
int
rxvt_get_pty (int *fd_tty, const char **ttydev)
{
  int pfd;

#ifdef PTYS_ARE_OPENPTY

  char tty_name[sizeof "/dev/pts/????\0"];

  if (openpty (&pfd, fd_tty, tty_name, NULL, NULL) != -1)
    {
      *ttydev = strdup (tty_name);
      return pfd;
    }
#endif

#ifdef PTYS_ARE__GETPTY
  *ttydev = _getpty (&pfd, O_RDWR | O_NONBLOCK | O_NOCTTY, 0622, 0);
  if (*ttydev != NULL)
    return pfd;
#endif

#ifdef PTYS_ARE_GETPTY

  char           *ptydev;

  while ((ptydev = getpty ()) != NULL)
    if ((pfd = open (ptydev, O_RDWR | O_NOCTTY, 0)) >= 0)
      {
        *ttydev = ptydev;
        return pfd;
      }
#endif

#if defined(HAVE_GRANTPT) && defined(HAVE_UNLOCKPT)
# if defined(PTYS_ARE_GETPT) || defined(PTYS_ARE_PTMX)

  {
#  ifdef PTYS_ARE_GETPT
    pfd = getpt ();
#  else

    pfd = open ("/dev/ptmx", O_RDWR | O_NOCTTY, 0);
#  endif

    if (pfd >= 0)
      {
        if (grantpt (pfd) == 0	/* change slave permissions */
            && unlockpt (pfd) == 0)
          {	/* slave now unlocked */
            *ttydev = ptsname (pfd);	/* get slave's name */
            return pfd;
          }
        close (pfd);
      }
  }
# endif
#endif

#ifdef PTYS_ARE_PTC
  if ((pfd = open ("/dev/ptc", O_RDWR | O_NOCTTY, 0)) >= 0)
    {
      *ttydev = ttyname (pfd);
      return pfd;
    }
#endif

#ifdef PTYS_ARE_CLONE
  if ((pfd = open ("/dev/ptym/clone", O_RDWR | O_NOCTTY, 0)) >= 0)
    {
      *ttydev = ptsname (pfd);
      return pfd;
    }
#endif

#ifdef PTYS_ARE_NUMERIC
  {
    int             idx;
    char           *c1, *c2;
    char            pty_name[] = "/dev/ptyp???";
    char            tty_name[] = "/dev/ttyp???";

    c1 = & (pty_name[sizeof (pty_name) - 4]);
    c2 = & (tty_name[sizeof (tty_name) - 4]);
    for (idx = 0; idx < 256; idx++)
      {
        sprintf (c1, "%d", idx);
        sprintf (c2, "%d", idx);
        if (access (tty_name, F_OK) < 0)
          {
            idx = 256;
            break;
          }
        if ((pfd = open (pty_name, O_RDWR | O_NOCTTY, 0)) >= 0)
          {
            if (access (tty_name, R_OK | W_OK) == 0)
              {
                *ttydev = strdup (tty_name);
                return pfd;
              }
            close (pfd);
          }
      }
  }
#endif
#ifdef PTYS_ARE_SEARCHED
  {
    const char     *c1, *c2;
    char            pty_name[] = "/dev/pty??";
    char            tty_name[] = "/dev/tty??";

# ifndef PTYCHAR1
#  define PTYCHAR1	"pqrstuvwxyz"
# endif
# ifndef PTYCHAR2
#  define PTYCHAR2	"0123456789abcdef"
# endif

    for (c1 = PTYCHAR1; *c1; c1++)
      {
        pty_name[ (sizeof (pty_name) - 3)] =
          tty_name[ (sizeof (pty_name) - 3)] = *c1;
        for (c2 = PTYCHAR2; *c2; c2++)
          {
            pty_name[ (sizeof (pty_name) - 2)] =
              tty_name[ (sizeof (pty_name) - 2)] = *c2;
            if ((pfd = open (pty_name, O_RDWR | O_NOCTTY, 0)) >= 0)
              {
                if (access (tty_name, R_OK | W_OK) == 0)
                  {
                    *ttydev = strdup (tty_name);
                    return pfd;
                  }
                close (pfd);
              }
          }
      }
  }
#endif
  return -1;
}

/*----------------------------------------------------------------------*/
/*
 * Returns tty file descriptor, or -1 on failure 
 */
/* EXTPROTO */
int
rxvt_get_tty (const char *ttydev)
{
  return open (ttydev, O_RDWR | O_NOCTTY, 0);
}

/*----------------------------------------------------------------------*/
/*
 * Make our tty a controlling tty so that /dev/tty points to us
 */
/* EXTPROTO */
int
rxvt_control_tty (int fd_tty, const char *ttydev)
{
#ifndef __QNX__
  int fd;

  D_TTY ((stderr, "rxvt_control_tty (): pid: %d, tty fd: %d, dev: %s", getpid (), fd_tty, ttydev));
  /* ---------------------------------------- */
# ifdef HAVE_SETSID
  setsid ();
# endif
# if defined(HAVE_SETPGID)
  setpgid (0, 0);
# elif defined(HAVE_SETPGRP)
  setpgrp (0, 0);
# endif
  /* ---------------------------------------- */
# ifdef TIOCNOTTY

  fd = open ("/dev/tty", O_RDWR | O_NOCTTY);
  D_TTY ((stderr, "rxvt_control_tty (): Voiding tty associations: previous=%s", fd < 0 ? "no" : "yes"));
  if (fd >= 0)
    {
      ioctl (fd, TIOCNOTTY, NULL);	/* void tty associations */
      close (fd);
    }
# endif
  /* ---------------------------------------- */
  fd = open ("/dev/tty", O_RDWR | O_NOCTTY);
  D_TTY ((stderr, "rxvt_control_tty (): /dev/tty has controlling tty? %s", fd < 0 ? "no (good)" : "yes (bad)"));
  if (fd >= 0)
    close (fd);		/* ouch: still have controlling tty */
  /* ---------------------------------------- */
#if defined(PTYS_ARE_PTMX) && defined(I_PUSH)
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
      D_TTY ((stderr, "rxvt_control_tty (): Pushing STREAMS modules"));
      ioctl (fd_tty, I_PUSH, "ptem");
      ioctl (fd_tty, I_PUSH, "ldterm");
      ioctl (fd_tty, I_PUSH, "ttcompat");
    }
#endif
  /* ---------------------------------------- */
# if defined(TIOCSCTTY)
  fd = ioctl (fd_tty, TIOCSCTTY, NULL);
  D_TTY ((stderr, "rxvt_control_tty (): ioctl (..,TIOCSCTTY): %d", fd));
# elif defined(TIOCSETCTTY)
  fd = ioctl (fd_tty, TIOCSETCTTY, NULL);
  D_TTY ((stderr, "rxvt_control_tty (): ioctl (..,TIOCSETCTTY): %d", fd));
# else
  fd = open (ttydev, O_RDWR);
  D_TTY ((stderr, "rxvt_control_tty (): tty open%s", fd < 0 ? " failure" : "ed OK"));
  if (fd >= 0)
    close (fd);
# endif
  /* ---------------------------------------- */
  fd = open ("/dev/tty", O_WRONLY);
  D_TTY ((stderr, "rxvt_control_tty (): do we have controlling tty now: %s", fd < 0 ? "no (fatal)" : "yes (good)"));
  if (fd < 0)
    return -1;		/* fatal */
  close (fd);
  /* ---------------------------------------- */
  D_TTY ((stderr, "rxvt_control_tty (): tcgetpgrp (): %d  getpgrp (): %d", tcgetpgrp (fd_tty), getpgrp ()));
  /* ---------------------------------------- */
#endif				/* ! __QNX__ */

  return 0;
}
/*----------------------- end-of-file (C source) -----------------------*/
