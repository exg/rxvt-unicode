/*--------------------------------*-C-*---------------------------------*
 * File:	logging.c
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
/*----------------------------------------------------------------------*
 * Public:
 *	extern void cleanutent (void);
 *	extern void makeutent (const char * pty, const char * hostname);
 *
 * Private:
 *	rxvt_update_wtmp ();
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#include "logging.h"
#ifdef UTMP_SUPPORT

/*
 * BSD style utmp entry
 *      ut_line, ut_name, ut_host, ut_time
 * SYSV style utmp (and utmpx) entry
 *      ut_user, ut_id, ut_line, ut_pid, ut_type, ut_exit, ut_time
 */

/* ------------------------------------------------------------------------- */

/*
 * make and write utmp and wtmp entries
 */
void
rxvt_term::makeutent (const char *pty, const char *hostname)
{
#ifdef HAVE_STRUCT_UTMP
  struct utmp    *ut = & (this->ut);
#endif
#ifdef HAVE_STRUCT_UTMPX
  struct utmpx   *utx = & (this->utx);
#endif
#ifdef HAVE_UTMP_PID
  int             i;
#endif
  char            ut_id[5];
  struct passwd  *pwent = getpwuid (getuid ());

  if (!STRNCMP (pty, "/dev/", 5))
    pty += 5;		/* skip /dev/ prefix */

  if (!STRNCMP (pty, "pty", 3) || !STRNCMP (pty, "tty", 3))
    {
      STRNCPY (ut_id, (pty + 3), sizeof (ut_id));
    }
#ifdef HAVE_UTMP_PID
  else if (sscanf (pty, "pts/%d", &i) == 1)
    sprintf (ut_id, "vt%02x", (i & 0xff));	/* sysv naming */
#endif
  else if (STRNCMP (pty, "pty", 3) && STRNCMP (pty, "tty", 3))
    {
      rxvt_print_error ("can't parse tty name \"%s\"", pty);
      return;
    }

#ifdef HAVE_STRUCT_UTMP
  MEMSET (ut, 0, sizeof (struct utmp));
# ifdef HAVE_UTMP_PID
  setutent ();
  STRNCPY (ut->ut_id, ut_id, sizeof (ut->ut_id));
  ut->ut_type = DEAD_PROCESS;
  getutid (ut);		/* position to entry in utmp file */
  STRNCPY (ut_id, ut_id, sizeof (ut_id));
# endif
#endif

#ifdef HAVE_STRUCT_UTMPX
  MEMSET (utx, 0, sizeof (struct utmpx));
  setutxent ();
  STRNCPY (utx->ut_id, ut_id, sizeof (utx->ut_id));
  utx->ut_type = DEAD_PROCESS;
  getutxid (utx);		/* position to entry in utmp file */
  STRNCPY (ut_id, ut_id, sizeof (ut_id));
#endif

#ifdef HAVE_STRUCT_UTMP
  STRNCPY (ut->ut_line, pty, sizeof (ut->ut_line));
  ut->ut_time = time (NULL);
# ifdef HAVE_UTMP_PID
  STRNCPY (ut->ut_user, (pwent && pwent->pw_name) ? pwent->pw_name : "?",
          sizeof (ut->ut_user));
  STRNCPY (ut->ut_id, ut_id, sizeof (ut->ut_id));
  ut->ut_time = time (NULL);
  ut->ut_pid = cmd_pid;
#  ifdef HAVE_UTMP_HOST
  STRNCPY (ut->ut_host, hostname, sizeof (ut->ut_host));
#  endif
  ut->ut_type = USER_PROCESS;
  pututline (ut);
  endutent ();			/* close the file */
  utmp_pos = 0;
# else
  STRNCPY (ut->ut_name, (pwent && pwent->pw_name) ? pwent->pw_name : "?",
          sizeof (ut->ut_name));
#  ifdef HAVE_UTMP_HOST
  STRNCPY (ut->ut_host, hostname, sizeof (ut->ut_host));
#  endif
# endif
#endif

#ifdef HAVE_STRUCT_UTMPX
  STRNCPY (utx->ut_line, pty, sizeof (utx->ut_line));
  STRNCPY (utx->ut_user, (pwent && pwent->pw_name) ? pwent->pw_name : "?",
          sizeof (utx->ut_user));
  STRNCPY (utx->ut_id, ut_id, sizeof (utx->ut_id));
  utx->ut_session = getsid (0);
  utx->ut_tv.tv_sec = time (NULL);
  utx->ut_tv.tv_usec = 0;
  utx->ut_pid = cmd_pid;
# ifdef HAVE_UTMPX_HOST
  STRNCPY (utx->ut_host, hostname, sizeof (utx->ut_host));
#  if 0
  {
    char           *colon;

    if ((colon = STRRCHR (ut->ut_host, ':')) != NULL)
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
    int             i;
# ifdef HAVE_TTYSLOT
    i = ttyslot ();
    if (rxvt_write_bsd_utmp (i, ut))
      utmp_pos = i;
# else
    FILE           *fd0;

    if ((fd0 = fopen (TTYTAB_FILENAME, "r")) != NULL)
      {
        char            buf[256], name[256];

        buf[sizeof (buf) - 1] = '\0';
        for (i = 1; (fgets (buf, sizeof (buf) - 1, fd0) != NULL);)
          {
            if (*buf == '#' || sscanf (buf, "%s", name) != 1)
              continue;
            if (!STRCMP (ut->ut_line, name))
              {
                if (!rxvt_write_bsd_utmp (i, ut))
                  i = 0;
                utmp_pos = i;
                fclose (fd0);
                break;
              }
            i++;
          }
        fclose (fd0);
      }
# endif

  }
#endif

#ifdef WTMP_SUPPORT
# ifdef WTMP_ONLY_ON_LOGIN
  if (Options & Opt_loginShell)
# endif
    {
# ifdef HAVE_STRUCT_UTMP
#  ifdef HAVE_UPDWTMP
      updwtmp (RXVT_WTMP_FILE, ut);
#  else
      rxvt_update_wtmp (RXVT_WTMP_FILE, ut);
#  endif
# endif
# ifdef HAVE_STRUCT_UTMPX
      updwtmpx (RXVT_WTMPX_FILE, utx);
# endif
    }
#endif
#if defined(LASTLOG_SUPPORT) && defined(RXVT_LASTLOG_FILE)
  if (Options & Opt_loginShell)
    rxvt_update_lastlog (RXVT_LASTLOG_FILE, pty, hostname);
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * remove utmp and wtmp entries
 */
void
rxvt_term::cleanutent ()
{
#ifdef HAVE_STRUCT_UTMP
  struct utmp    *ut = & (this->ut);
#endif
#ifdef HAVE_STRUCT_UTMPX
  struct utmpx   *tmputx, *utx = & (this->utx);
#endif

#ifdef HAVE_STRUCT_UTMP
# ifdef HAVE_UTMP_PID
  MEMSET (ut, 0, sizeof (struct utmp));
  setutent ();
  STRNCPY (ut->ut_id, ut_id, sizeof (ut->ut_id));
  ut->ut_type = USER_PROCESS;
  {
    struct utmp    *tmput = getutid (ut);

    if (tmput)		/* position to entry in utmp file */
      ut = tmput;
  }
  ut->ut_type = DEAD_PROCESS;
# else
  MEMSET (ut->ut_name, 0, sizeof (ut->ut_name));
#  ifdef HAVE_UTMP_HOST
  MEMSET (ut->ut_host, 0, sizeof (ut->ut_host));
#  endif
# endif
  ut->ut_time = time (NULL);
#endif

#ifdef HAVE_STRUCT_UTMPX
  MEMSET (utx, 0, sizeof (struct utmpx));
  setutxent ();
  STRNCPY (utx->ut_id, ut_id, sizeof (utx->ut_id));
  utx->ut_type = USER_PROCESS;
  if ((tmputx = getutxid (utx)))	/* position to entry in utmp file */
    utx = tmputx;
  utx->ut_type = DEAD_PROCESS;
  utx->ut_session = getsid (0);
  utx->ut_tv.tv_sec = time (NULL);
  utx->ut_tv.tv_usec = 0;
#endif

  /*
   * Write ending wtmp entry
   */
#ifdef WTMP_SUPPORT
# ifdef WTMP_ONLY_ON_LOGIN
  if (Options & Opt_loginShell)
# endif
    {
# ifdef HAVE_STRUCT_UTMP
#  ifdef HAVE_UPDWTMP
      updwtmp (RXVT_WTMP_FILE, ut);
#  else
      rxvt_update_wtmp (RXVT_WTMP_FILE, ut);
#  endif
# endif
# ifdef HAVE_STRUCT_UTMPX
      updwtmpx (RXVT_WTMPX_FILE, utx);
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
  MEMSET (ut, 0, sizeof (struct utmp));
  rxvt_write_bsd_utmp (utmp_pos, ut);
# endif
#endif
#ifdef HAVE_STRUCT_UTMPX
  if (utx->ut_pid == cmd_pid)
    pututxline (utx);
  endutxent ();
#endif
}

/* ------------------------------------------------------------------------- */
/*
 * Write a BSD style utmp entry
 */
#ifdef HAVE_UTMP_H
/* INTPROTO */
int
rxvt_write_bsd_utmp (int utmp_pos, struct utmp *wu)
{
  int             fd;

  if (utmp_pos <= 0 || (fd = open (RXVT_UTMP_FILE, O_WRONLY)) == -1)
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
#if defined(WTMP_SUPPORT) && !defined(HAVE_UPDWTMP)
/* INTPROTO */
void
rxvt_update_wtmp (const char *fname, const struct utmp *putmp)
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
    {		/* give it up */
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
/* INTPROTO */
void
rxvt_update_lastlog (const char *fname, const char *pty, const char *host)
{
# ifdef HAVE_STRUCT_LASTLOGX
  struct lastlogx llx;
# endif
# ifdef HAVE_STRUCT_LASTLOG
  int             fd;
  struct lastlog  ll;
#  ifdef LASTLOG_IS_DIR
  char            lastlogfile[256];
#  endif
  struct passwd  *pwent;
# endif

# ifdef HAVE_STRUCT_LASTLOGX
  MEMSET (&llx, 0, sizeof (llx));
  llx.ll_tv.tv_sec = time (NULL);
  llx.ll_tv.tv_usec = 0;
  STRNCPY (llx.ll_line, pty, sizeof (llx.ll_line));
  STRNCPY (llx.ll_host, host, sizeof (llx.ll_host));
  updlastlogx (RXVT_LASTLOGX_FILE, getuid (), &llx);
# endif

# ifdef HAVE_STRUCT_LASTLOG
  pwent = getpwuid (getuid ());
  if (!pwent)
    {
      rxvt_print_error ("no entry in password file");
      return;
    }
  MEMSET (&ll, 0, sizeof (ll));
  ll.ll_time = time (NULL);
  STRNCPY (ll.ll_line, pty, sizeof (ll.ll_line));
  STRNCPY (ll.ll_host, host, sizeof (ll.ll_host));
#  ifdef LASTLOG_IS_DIR
  sprintf (lastlogfile, "%.*s/%.*s",
          sizeof (lastlogfile) - sizeof (pwent->pw_name) - 2, fname,
          sizeof (pwent->pw_name),
          (!pwent->pw_name || pwent->pw_name[0] == '\0') ? "unknown"
          : pwent->pw_name);
  if ((fd = open (lastlogfile, O_WRONLY | O_CREAT, 0644)) >= 0)
    {
      write (fd, &ll, sizeof (ll));
      close (fd);
    }
#  else
  if ((fd = open (fname, O_RDWR)) != -1)
    {
      if (lseek (fd, (off_t) ((long)pwent->pw_uid * sizeof (ll)),
                SEEK_SET) != -1)
        write (fd, &ll, sizeof (ll));
      close (fd);
    }
#  endif			/* LASTLOG_IS_DIR */
# endif				/* HAVE_STRUCT_LASTLOG */
}
#endif				/* LASTLOG_SUPPORT */
/* ------------------------------------------------------------------------- */

#endif				/* UTMP_SUPPORT */
