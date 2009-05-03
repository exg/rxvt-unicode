// This file is part of libptytty. Do not make local modifications.
// http://software.schmorp.de/pkg/libptytty

/*----------------------------------------------------------------------*
 * File:	logging.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey <jdb@ukc.ac.uk>
 *				- original version
 * Copyright (c) 1993      lipka
 * Copyright (c) 1993      Brian Stempien <stempien@cs.wmich.edu>
 * Copyright (c) 1995      Raul Garcia Garcia <rgg@tid.es>
 * Copyright (c) 1995      Piet W. Plomp <piet@idefix.icce.rug.nl>
 * Copyright (c) 1997      Raul Garcia Garcia <rgg@tid.es>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 * 				- extensive modifications
 * Copyright (c) 1999      D J Hawkey Jr <hawkeyd@visi.com>
 *				- lastlog support
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
 *----------------------------------------------------------------------*/

#include "../config.h"

#include "ptytty.h"

#if UTMP_SUPPORT

#include <cstdio>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

/*
 * BSD style utmp entry
 *      ut_line, ut_name, ut_host, ut_time
 * SYSV style utmp (and utmpx) entry
 *      ut_user, ut_id, ut_line, ut_pid, ut_type, ut_exit, ut_time
 */

/* ------------------------------------------------------------------------- */
/*
 * Write a BSD style utmp entry
 */
#if defined(HAVE_STRUCT_UTMP) && !defined(HAVE_UTMP_PID)
static int
write_bsd_utmp (int utmp_pos, struct utmp *wu)
{
  int             fd;

  if (utmp_pos <= 0 || (fd = open (UTMP_FILE, O_WRONLY)) == -1)
    return 0;

  if (lseek (fd, (off_t) (utmp_pos * sizeof (struct utmp)), SEEK_SET) != -1)
    write (fd, wu, sizeof (struct utmp));
  close (fd);
  return 1;
}
#endif

/* ------------------------------------------------------------------------- */
/*
 * Update a BSD style wtmp entry
 */
#if defined(WTMP_SUPPORT) && !defined(HAVE_UPDWTMP) && defined(HAVE_STRUCT_UTMP)
static void
update_wtmp (const char *fname, const struct utmp *putmp)
{
  int             fd, gotlock, retry;
  struct flock    lck;	/* fcntl locking scheme */
  struct stat     sbuf;

  if ((fd = open (fname, O_WRONLY | O_APPEND, 0)) < 0)
    return;

  lck.l_whence = SEEK_END;	/* start lock at current eof */
  lck.l_len = 0;		/* end at ``largest possible eof'' */
  lck.l_start = 0;
  lck.l_type = F_WRLCK;	/* we want a write lock */

  /* attempt lock with F_SETLK; F_SETLKW would cause a deadlock! */
  for (retry = 10, gotlock = 0; retry--;)
    if (fcntl (fd, F_SETLK, &lck) != -1)
      {
        gotlock = 1;
        break;
      }
    else if (errno != EAGAIN && errno != EACCES)
      break;
  if (!gotlock)
    {
      /* give it up */
      close (fd);
      return;
    }
  if (fstat (fd, &sbuf) == 0)
    if (write (fd, putmp, sizeof (struct utmp)) != sizeof (struct utmp))
      ftruncate (fd, sbuf.st_size);	/* remove bad writes */

  lck.l_type = F_UNLCK;	/* unlocking the file */
  fcntl (fd, F_SETLK, &lck);
  close (fd);
}
#endif

/* ------------------------------------------------------------------------- */
#ifdef LASTLOG_SUPPORT
static void
update_lastlog (const char *fname, const char *pty, const char *host)
{
# if defined(HAVE_STRUCT_LASTLOGX) && defined(HAVE_UPDLASTLOGX)
  struct lastlogx llx;
# endif
# ifdef HAVE_STRUCT_LASTLOG
  int             fd;
  struct lastlog  ll;
  char            lastlogfile[256];
  struct passwd  *pwent;
  struct stat     st;
# endif

# if defined(HAVE_STRUCT_LASTLOGX) && defined(HAVE_UPDLASTLOGX)
  memset (&llx, 0, sizeof (llx));
  llx.ll_tv.tv_sec = time (NULL);
  llx.ll_tv.tv_usec = 0;
  strncpy (llx.ll_line, pty, sizeof (llx.ll_line));
  strncpy (llx.ll_host, host, sizeof (llx.ll_host));
  updlastlogx (LASTLOGX_FILE, getuid (), &llx);
# endif

# ifdef HAVE_STRUCT_LASTLOG
  pwent = getpwuid (getuid ());
  if (!pwent)
    {
      ptytty_warn ("no entry in password file, not updating lastlog.\n", 0);
      return;
    }

  memset (&ll, 0, sizeof (ll));
  ll.ll_time = time (NULL);
  strncpy (ll.ll_line, pty, sizeof (ll.ll_line));
  strncpy (ll.ll_host, host, sizeof (ll.ll_host));
  if (stat (fname, &st) != 0)
    return;
  if (S_ISDIR (st.st_mode))
    {
      sprintf (lastlogfile, "%.*s/%.*s",
               (int)(sizeof (lastlogfile) - sizeof (pwent->pw_name) - 2), fname,
               (int)sizeof (pwent->pw_name),
               (!pwent->pw_name || pwent->pw_name[0] == '\0') ? "unknown"
               : pwent->pw_name);
      if ((fd = open (lastlogfile, O_WRONLY | O_CREAT, 0644)) >= 0)
        {
          write (fd, &ll, sizeof (ll));
          close (fd);
        }
    }
  else if (S_ISREG (st.st_mode))
    if ((fd = open (fname, O_RDWR)) != -1)
      {
        if (lseek (fd, (off_t) ((long)pwent->pw_uid * sizeof (ll)),
                   SEEK_SET) != -1)
          write (fd, &ll, sizeof (ll));
        close (fd);
      }
# endif /* HAVE_STRUCT_LASTLOG */
}
#endif /* LASTLOG_SUPPORT */

/* ------------------------------------------------------------------------- */

/*
 * make and write utmp and wtmp entries
 */
void
ptytty_unix::login (int cmd_pid, bool login_shell, const char *hostname)
{
  const char *pty = name;

  if (!pty || !*pty)
    return;

  this->cmd_pid     = cmd_pid;
  this->login_shell = login_shell;

#ifdef HAVE_STRUCT_UTMP
  struct utmp *ut = &this->ut;
#endif
#ifdef HAVE_STRUCT_UTMPX
  struct utmpx *utx = &this->utx;
#endif
  int i;
  struct passwd *pwent = getpwuid (getuid ());
  const char *name = (pwent && pwent->pw_name) ? pwent->pw_name : "?";

  if (!strncmp (pty, "/dev/", 5))
    pty += 5;		/* skip /dev/ prefix */

#if defined(HAVE_UTMP_PID) || defined(HAVE_STRUCT_UTMPX)
  if (!strncmp (pty, "pty", 3) || !strncmp (pty, "tty", 3))
    strncpy (ut_id, pty + 3, sizeof (ut_id));
  else if (sscanf (pty, "pts/%d", &i) == 1)
    sprintf (ut_id, "vt%02x", (i & 0xff));	/* sysv naming */
  else
    {
      ptytty_warn ("can't parse tty name \"%s\", not adding utmp entry.\n", pty);
      return;
    }
#endif

#ifdef HAVE_STRUCT_UTMP
  memset (ut, 0, sizeof (struct utmp));
# ifdef HAVE_UTMP_PID
  setutent ();
  strncpy (ut->ut_id, ut_id, sizeof (ut->ut_id));
  ut->ut_type = DEAD_PROCESS;
  getutid (ut);		/* position to entry in utmp file */
# endif
#endif

#ifdef HAVE_STRUCT_UTMPX
  memset (utx, 0, sizeof (struct utmpx));
  setutxent ();
  strncpy (utx->ut_id, ut_id, sizeof (utx->ut_id));
  utx->ut_type = DEAD_PROCESS;
  getutxid (utx);		/* position to entry in utmp file */
#endif

#ifdef HAVE_STRUCT_UTMP
  strncpy (ut->ut_line, pty, sizeof (ut->ut_line));
# ifdef HAVE_UTMP_HOST
  strncpy (ut->ut_host, hostname, sizeof (ut->ut_host));
# endif
  ut->ut_time = time (NULL);
# ifdef HAVE_UTMP_PID
  strncpy (ut->ut_user, name, sizeof (ut->ut_user));
  strncpy (ut->ut_id, ut_id, sizeof (ut->ut_id));
  ut->ut_pid = cmd_pid;
  ut->ut_type = USER_PROCESS;
  pututline (ut);
  endutent ();			/* close the file */
  utmp_pos = 0;
# else
  strncpy (ut->ut_name, name, sizeof (ut->ut_name));
# endif
#endif

#ifdef HAVE_STRUCT_UTMPX
  strncpy (utx->ut_line, pty, sizeof (utx->ut_line));
  strncpy (utx->ut_user, name, sizeof (utx->ut_user));
  strncpy (utx->ut_id, ut_id, sizeof (utx->ut_id));
# if HAVE_UTMPX_SESSION
  utx->ut_session = getsid (0);
# endif
  utx->ut_tv.tv_sec = time (NULL);
  utx->ut_tv.tv_usec = 0;
  utx->ut_pid = cmd_pid;
# ifdef HAVE_UTMPX_HOST
  strncpy (utx->ut_host, hostname, sizeof (utx->ut_host));
#  if 0
  {
    char           *colon;

    if ((colon = strrchr (ut->ut_host, ':')) != NULL)
      *colon = '\0';
  }
#  endif
# endif
  utx->ut_type = USER_PROCESS;
  pututxline (utx);
  endutxent ();		/* close the file */
  utmp_pos = 0;
#endif

#if defined(HAVE_STRUCT_UTMP) && !defined(HAVE_UTMP_PID)
  {
# if 1
    int fdstdin = dup (STDIN_FILENO);
    dup2 (tty, STDIN_FILENO);

    i = ttyslot ();
    if (write_bsd_utmp (i, ut))
      utmp_pos = i;

    dup2 (fdstdin, STDIN_FILENO);
    close (fdstdin);
# endif
  }
#endif

#ifdef WTMP_SUPPORT
#ifdef LOG_ONLY_ON_LOGIN
  if (login_shell)
#endif
    {
# ifdef HAVE_STRUCT_UTMP
#  ifdef HAVE_UPDWTMP
      updwtmp (WTMP_FILE, ut);
#  else
      update_wtmp (WTMP_FILE, ut);
#  endif
# endif
# if defined(HAVE_STRUCT_UTMPX) && defined(HAVE_UPDWTMPX)
      updwtmpx (WTMPX_FILE, utx);
# endif
    }
#endif
#if defined(LASTLOG_SUPPORT) && defined(LASTLOG_FILE)
#ifdef LOG_ONLY_ON_LOGIN
  if (login_shell)
#endif
    update_lastlog (LASTLOG_FILE, pty, hostname);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * remove utmp and wtmp entries
 */
void
ptytty_unix::logout ()
{
  if (!cmd_pid)
    return;

#ifdef HAVE_STRUCT_UTMP
  struct utmp *tmput, *ut = &this->ut;
#endif
#ifdef HAVE_STRUCT_UTMPX
  struct utmpx *tmputx, *utx = &this->utx;
#endif

#ifdef HAVE_STRUCT_UTMP
# ifdef HAVE_UTMP_PID
  memset (ut, 0, sizeof (struct utmp));
  setutent ();
  strncpy (ut->ut_id, this->ut_id, sizeof (ut->ut_id));
  ut->ut_type = USER_PROCESS;
  if ((tmput = getutid (ut)))		/* position to entry in utmp file */
    ut = tmput;
  ut->ut_type = DEAD_PROCESS;
# else
  memset (ut->ut_name, 0, sizeof (ut->ut_name));
#  ifdef HAVE_UTMP_HOST
  memset (ut->ut_host, 0, sizeof (ut->ut_host));
#  endif
# endif
  ut->ut_time = time (NULL);
#endif

#ifdef HAVE_STRUCT_UTMPX
  memset (utx, 0, sizeof (struct utmpx));
  setutxent ();
  strncpy (utx->ut_id, this->ut_id, sizeof (utx->ut_id));
  utx->ut_type = USER_PROCESS;
  if ((tmputx = getutxid (utx)))	/* position to entry in utmp file */
    utx = tmputx;
  utx->ut_type = DEAD_PROCESS;
# if HAVE_UTMPX_SESSION
  utx->ut_session = getsid (0);
# endif
  utx->ut_tv.tv_sec = time (NULL);
  utx->ut_tv.tv_usec = 0;
#endif

  /*
   * Write ending wtmp entry
   */
#ifdef WTMP_SUPPORT
#ifdef LOG_ONLY_ON_LOGIN
  if (login_shell)
#endif
    {
# ifdef HAVE_STRUCT_UTMP
#  ifdef HAVE_UPDWTMP
      updwtmp (WTMP_FILE, ut);
#  else
      update_wtmp (WTMP_FILE, ut);
#  endif
# endif
# if defined(HAVE_STRUCT_UTMPX) && defined(HAVE_UPDWTMPX)
      updwtmpx (WTMPX_FILE, utx);
# endif
    }
#endif

  /*
   * Write utmp entry
   */
#ifdef HAVE_STRUCT_UTMP
# ifdef HAVE_UTMP_PID
  if (ut->ut_pid == cmd_pid)
    pututline (ut);
  endutent ();
# else
  memset (ut, 0, sizeof (struct utmp));
  write_bsd_utmp (utmp_pos, ut);
# endif
#endif
#ifdef HAVE_STRUCT_UTMPX
  if (utx->ut_pid == cmd_pid)
    pututxline (utx);
  endutxent ();
#endif

  cmd_pid = 0;
}

#else
void
ptytty_unix::login (int cmd_pid, bool login_shell, const char *hostname)
{
}
#endif

