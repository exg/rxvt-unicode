/*--------------------------------*-C-*---------------------------------*
 * File:	ptytty.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004-2006 Marc Lehmann <pcg@goof.com>
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
#include "rxvt.h"

# include <cstdlib>
# include <cstring>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#if defined(PTYS_ARE_PTMX) && defined(HAVE_SYS_STROPTS_H)
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

#include <cstdio>
#include <grp.h>

/* ------------------------------------------------------------------------- *
 *                  GET PSEUDO TELETYPE - MASTER AND SLAVE                   *
 * ------------------------------------------------------------------------- */
/*
 * Returns pty file descriptor, or -1 on failure 
 * If successful, ttydev is set to the name of the slave device.
 * fd_tty _may_ also be set to an open fd to the slave device
 */
static int
get_pty (int *fd_tty, char **ttydev)
{
  int pfd;

#ifdef PTYS_ARE_OPENPTY
  char tty_name[sizeof "/dev/pts/????\0"];

  rxvt_privileges(RESTORE);
  int res = openpty (&pfd, fd_tty, tty_name, NULL, NULL);
  rxvt_privileges(IGNORE);

  if (res != -1)
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
            *ttydev = strdup (ptsname (pfd));	/* get slave's name */
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
      *ttydev = strdup (ttyname (pfd));
      return pfd;
    }
#endif

#ifdef PTYS_ARE_CLONE
  if ((pfd = open ("/dev/ptym/clone", O_RDWR | O_NOCTTY, 0)) >= 0)
    {
      *ttydev = strdup (ptsname (pfd));
      return pfd;
    }
#endif

#ifdef PTYS_ARE_NUMERIC
  {
    int idx;
    char *c1, *c2;
    char pty_name[] = "/dev/ptyp???";
    char tty_name[] = "/dev/ttyp???";

    c1 = &(pty_name[sizeof (pty_name) - 4]);
    c2 = &(tty_name[sizeof (tty_name) - 4]);
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
    const char *c1, *c2;
    char pty_name[] = "/dev/pty??";
    char tty_name[] = "/dev/tty??";

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
control_tty (int fd_tty, const char *ttydev)
{
#ifndef __QNX__
  int fd;

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
  if (fd >= 0)
    {
      ioctl (fd, TIOCNOTTY, NULL);	/* void tty associations */
      close (fd);
    }
# endif

  /* ---------------------------------------- */
  fd = open ("/dev/tty", O_RDWR | O_NOCTTY);
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
      ioctl (fd_tty, I_PUSH, "ptem");
      ioctl (fd_tty, I_PUSH, "ldterm");
      ioctl (fd_tty, I_PUSH, "ttcompat");
    }
#endif
  /* ---------------------------------------- */
# if defined(TIOCSCTTY)
  fd = ioctl (fd_tty, TIOCSCTTY, NULL);
# elif defined(TIOCSETCTTY)
  fd = ioctl (fd_tty, TIOCSETCTTY, NULL);
# else
  fd = open (ttydev, O_RDWR);
  if (fd >= 0)
    close (fd);
# endif
  /* ---------------------------------------- */
  fd = open ("/dev/tty", O_WRONLY);
  if (fd < 0)
    return -1;		/* fatal */
  close (fd);
  /* ---------------------------------------- */
#endif				/* ! __QNX__ */

  return 0;
}

#ifndef NO_SETOWNER_TTYDEV
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
          gid = getgid ();
        }
    }
} ttyconf;

void
rxvt_ptytty::privileges (rxvt_privaction action)
{
  if (!name || !*name)
    return;

  rxvt_privileges (RESTORE);

  if (action == SAVE)
    {
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
      /* store original tty status for restoration rxvt_clean_exit () -- rgg 04/12/95 */
      if (lstat (name, &savestat) < 0)       /* you lose out */
        ;
      else
# endif
        {
          saved = true;
          chown (name, getuid (), ttyconf.gid);      /* fail silently */
          chmod (name, ttyconf.mode);
# ifdef HAVE_REVOKE
          revoke (name);
# endif
        }
    }
  else
    {                    /* action == RESTORE */
# ifndef RESET_TTY_TO_COMMON_DEFAULTS
      if (saved)
        {
          chmod (name, savestat.st_mode);
          chown (name, savestat.st_uid, savestat.st_gid);
        }
# else
      chmod (name, (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH));
      chown (name, 0, 0);
# endif

    }

  rxvt_privileges (IGNORE);
}
#endif

rxvt_ptytty::rxvt_ptytty ()
{
  pty = tty = -1;
  name = 0;
#ifndef NO_SETOWNER_TTYDEV
  saved = false;
#endif
}

rxvt_ptytty::~rxvt_ptytty ()
{
  put ();
}

void

rxvt_ptytty::close_tty ()
{
  if (tty < 0)
    return;

  close (tty);
  tty = -1;
}

void
rxvt_ptytty::put ()
{
#ifndef NO_SETOWNER_TTYDEV
  privileges (RESTORE);
#endif

  if (pty >= 0) close (pty);
  close_tty ();
  free (name);

  pty = tty = -1;
  name = 0;
}

bool
rxvt_ptytty::make_controlling_tty ()
{
  return control_tty (tty, name) >= 0;
}

bool
rxvt_ptytty::get ()
{
  /* get master (pty) */
  if ((pty = get_pty (&tty, &name)) < 0)
    return false;

  fcntl (pty, F_SETFL, O_NONBLOCK);

  /* get slave (tty) */
  if (tty < 0)
    {
#ifndef NO_SETOWNER_TTYDEV
      privileges (SAVE);
#endif

      if ((tty = get_tty (name)) < 0)
        {
          put ();
          return false;
        }
    }

  return true;
}

void
rxvt_ptytty::set_utf8_mode (bool on)
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

/*----------------------- end-of-file (C source) -----------------------*/

