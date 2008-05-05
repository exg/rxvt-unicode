dnl this file is part of libptytty, do not make local modifications
dnl http://software.schmorp.de/pkg/libptytty

AC_DEFUN([PTY_CHECK],
[
AC_CHECK_HEADERS( \
  pty.h \
  util.h \
  libutil.h \
  sys/ioctl.h \
  sys/stropts.h \
  stropts.h \
)

AC_CHECK_FUNCS( \
  revoke \
  _getpty \
  getpt \
  posix_openpt \
  isastream \
  setuid \
  seteuid \
  setreuid \
  setresuid \
)

have_clone=no

AC_MSG_CHECKING(for /dev/ptym/clone)
if test -e /dev/ptym/clone; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_DEV_CLONE, 1, [Define to 1 if you have /dev/ptym/clone])
  AC_DEFINE(CLONE_DEVICE, "/dev/ptym/clone", [clone device filename])
  have_clone=yes
else
  AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(for /dev/ptc)
if test -e /dev/ptc; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(HAVE_DEV_PTC, 1, [Define to 1 if you have /dev/ptc])
  AC_DEFINE(CLONE_DEVICE, "/dev/ptc", [clone device filename])
  have_clone=yes
else
  AC_MSG_RESULT(no)
fi

case $host in
  *-*-cygwin*)
    have_clone=yes
    AC_DEFINE(CLONE_DEVICE, "/dev/ptmx", [clone device filename])
    ;;
  *)
    AC_MSG_CHECKING(for /dev/ptmx)
    if test -e /dev/ptmx; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_DEV_PTMX, 1, [Define to 1 if you have /dev/ptmx])
      AC_DEFINE(CLONE_DEVICE, "/dev/ptmx", [clone device filename])
      have_clone=yes
    else
      AC_MSG_RESULT(no)
    fi
    ;;
esac

if test x$ac_cv_func_getpt = xyes -o x$ac_cv_func_posix_openpt = xyes -o x$have_clone = xyes; then
  AC_MSG_CHECKING(for UNIX98 ptys)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>]],
              [[grantpt(0);unlockpt(0);ptsname(0);]])],
              [unix98_pty=yes
               AC_DEFINE(UNIX98_PTY, 1, "")
               AC_MSG_RESULT(yes)],
              [AC_MSG_RESULT(no)])
fi

if test -z "$unix98_pty"; then
  AC_CHECK_FUNCS(openpty, [], [AC_CHECK_LIB(util, openpty, [AC_DEFINE(HAVE_OPENPTY) LIBS="$LIBS -lutil"])])
fi
])

AC_DEFUN([UTMP_CHECK],
[
support_utmp=yes
support_wtmp=yes
support_lastlog=yes

AC_ARG_ENABLE(utmp,
  [AS_HELP_STRING([--enable-utmp],[enable utmp (utmpx) support])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_utmp=$enableval
  fi])

AC_ARG_ENABLE(wtmp,
  [AS_HELP_STRING([--enable-wtmp],[enable wtmp (wtmpx) support (requires --enable-utmp)])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_wtmp=$enableval
  fi])

AC_ARG_ENABLE(lastlog,
  [AS_HELP_STRING([--enable-lastlog],[enable lastlog support (requires --enable-utmp)])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_lastlog=$enableval
  fi])

if test x$support_utmp = xyes; then
  AC_DEFINE(UTMP_SUPPORT, 1, Define if you want to have utmp/utmpx support)
fi
if test x$support_wtmp = xyes; then
  AC_DEFINE(WTMP_SUPPORT, 1, Define if you want to have wtmp support when utmp/utmpx is enabled)
fi
if test x$support_lastlog = xyes; then
  AC_DEFINE(LASTLOG_SUPPORT, 1, Define if you want to have lastlog support when utmp/utmpx is enabled)
fi

AC_CHECK_FUNCS( \
	updwtmp \
	updwtmpx \
	updlastlogx \
)

AC_CHECK_HEADERS(lastlog.h)

dnl# --------------------------------------------------------------------------
dnl# DO ALL UTMP AND WTMP CHECKING
dnl# --------------------------------------------------------------------------
dnl# check for host field in utmp structure

dnl# --------------------------------------------
AC_CHECK_HEADERS(utmp.h,
[AC_CACHE_CHECK([for struct utmp], struct_utmp,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut;]])],[struct_utmp=yes],[struct_utmp=no])])
if test x$struct_utmp = xyes; then
  AC_DEFINE(HAVE_STRUCT_UTMP, 1, Define if utmp.h has struct utmp)
fi
]

AC_CACHE_CHECK(for ut_host in utmp struct, struct_utmp_host,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut; ut.ut_host;]])],[struct_utmp_host=yes],[struct_utmp_host=no])])
if test x$struct_utmp_host = xyes; then
  AC_DEFINE(HAVE_UTMP_HOST, 1, Define if struct utmp contains ut_host)
fi

AC_CACHE_CHECK(for ut_pid in utmp struct, struct_utmp_pid,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut; ut.ut_pid;]])],[struct_utmp_pid=yes],[struct_utmp_pid=no])])
if test x$struct_utmp_pid = xyes; then
  AC_DEFINE(HAVE_UTMP_PID, 1, Define if struct utmp contains ut_pid)
fi
) dnl# AC_CHECK_HEADERS(utmp.h

dnl# --------------------------------------------

AC_CHECK_HEADERS(utmpx.h,
[AC_CACHE_CHECK([for struct utmpx], struct_utmpx,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx ut;]])],[struct_utmpx=yes],[struct_utmpx=no])])
if test x$struct_utmpx = xyes; then
  AC_DEFINE(HAVE_STRUCT_UTMPX, 1, Define if utmpx.h has struct utmpx)
fi
]

AC_CACHE_CHECK(for host in utmpx struct, struct_utmpx_host,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx utx; utx.ut_host;]])],[struct_utmpx_host=yes],[struct_utmpx_host=no])])
if test x$struct_utmpx_host = xyes; then
  AC_DEFINE(HAVE_UTMPX_HOST, 1, Define if struct utmpx contains ut_host)
fi

AC_CACHE_CHECK(for session in utmpx struct, struct_utmpx_session,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx utx; utx.ut_session;]])],[struct_utmpx_session=yes],[struct_utmpx_session=no])])
if test x$struct_utmpx_session = xyes; then
  AC_DEFINE(HAVE_UTMPX_SESSION, 1, Define if struct utmpx contains ut_session)
fi
) dnl# AC_CHECK_HEADERS(utmpx.h

dnl# --------------------------------------------------------------------------
dnl# check for struct lastlog
AC_CACHE_CHECK(for struct lastlog, struct_lastlog,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
]], [[struct lastlog ll;]])],[struct_lastlog=yes],[struct_lastlog=no])])
if test x$struct_lastlog = xyes; then
  AC_DEFINE(HAVE_STRUCT_LASTLOG, 1, Define if utmp.h or lastlog.h has struct lastlog)
fi

dnl# check for struct lastlogx
AC_CACHE_CHECK(for struct lastlogx, struct_lastlogx,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
]], [[struct lastlogx ll;]])],[struct_lastlogx=yes],[struct_lastlogx=no])])
if test x$struct_lastlogx = xyes; then
  AC_DEFINE(HAVE_STRUCT_LASTLOGX, 1, Define if utmpx.h or lastlog.h has struct lastlogx)
fi

dnl# --------------------------------------------------------------------------
dnl# FIND FILES
dnl# --------------------------------------------------------------------------

dnl# find utmp
AC_CACHE_CHECK(where utmp is located, path_utmp,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <utmp.h>
#include <errno.h>
main()
{
    char **u, *utmplist[] = {
    "/var/run/utmp", "/var/adm/utmp", "/etc/utmp", "/usr/etc/utmp", "/usr/adm/utmp", NULL };
    FILE *a, *f=fopen("conftestval", "w");
    if (!f) exit(1);
#ifdef UTMP_FILE
    fprintf(f, "%s\n", UTMP_FILE);
    exit(0);
#endif
#ifdef _PATH_UTMP
    fprintf(f, "%s\n", _PATH_UTMP);
    exit(0);
#endif
    for (u = utmplist; *u; u++) {
	if ((a = fopen(*u, "r")) != NULL || errno == EACCES) {
	    fprintf(f, "%s\n", *u);
	    exit(0);
	}
    }
    exit(0);
}]])],[path_utmp=`cat conftestval`],[path_utmp=],[dnl
  AC_MSG_WARN(Define UTMP_FILE in config.h manually)])])
if test x$path_utmp != x; then
  AC_DEFINE_UNQUOTED(UTMP_FILE, "$path_utmp", Define location of utmp)
fi

dnl# --------------------------------------------------------------------------

dnl# find utmpx - if a utmp file exists at the same location and is more than
dnl# a day newer, then dump the utmpx.  People leave lots of junk around.
AC_CACHE_CHECK(where utmpx is located, path_utmpx,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <utmpx.h>
#include <errno.h>
#include <sys/stat.h>
#ifdef HAVE_STRING_H
#include <string.h>
#endif
main()
{
    char **u, *p, *utmplist[] = {
#ifdef UTMPX_FILE
	UTMPX_FILE,
#endif
#ifdef _PATH_UTMPX
	_PATH_UTMPX,
#endif
    "/var/adm/utmpx", "/etc/utmpx", NULL };
    FILE *a, *f=fopen("conftestval", "w");
    struct stat statu, statux;
    if (!f) exit(1);
    for (u = utmplist; *u; u++) {
	if ((a = fopen(*u, "r")) != NULL || errno == EACCES) {
	    if (stat(*u, &statux) < 0)
		continue;
	    p = strdup(*u);
	    p[strlen(p) - 1] = '\0';
	    if (stat(p, &statu) >= 0
		&& (statu.st_mtime - statux.st_mtime > 86400))
		continue;
	    fprintf(f, "%s\n", *u);
	    exit(0);
	}
    }
    exit(0);
}]])],[path_utmpx=`cat conftestval`],[path_utmpx=],[dnl
  AC_MSG_WARN(Define UTMPX_FILE in config.h manually)])])
if test x$path_utmpx != x; then
  AC_DEFINE_UNQUOTED(UTMPX_FILE, "$path_utmpx", Define location of utmpx)
fi

dnl# --------------------------------------------------------------------------

dnl# find wtmp
AC_CACHE_CHECK(where wtmp is located, path_wtmp,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#include <errno.h>
main()
{
    char **w, *wtmplist[] = {
    "/var/log/wtmp", "/var/adm/wtmp", "/etc/wtmp", "/usr/etc/wtmp", "/usr/adm/wtmp", NULL };
    FILE *a, *f=fopen("conftestval", "w");
    if (!f) exit(1);
#ifdef WTMP_FILE
    fprintf(f, "%s\n", WTMP_FILE);
    exit(0);
#endif
#ifdef _PATH_WTMP
    fprintf(f, "%s\n", _PATH_WTMP);
    exit(0);
#endif
    for (w = wtmplist; *w; w++) {
	if ((a = fopen(*w, "r")) != NULL || errno == EACCES) {
	    fprintf(f, "%s\n", *w);
	    exit(0);
	}
    }
    exit(0);
}]])],[path_wtmp=`cat conftestval`],[path_wtmp=],[dnl
  AC_MSG_WARN(Define WTMP_FILE in config.h manually)])])
if test x$path_wtmp != x; then
  AC_DEFINE_UNQUOTED(WTMP_FILE, "$path_wtmp", Define location of wtmp)
fi
dnl# --------------------------------------------------------------------------

dnl# find wtmpx
AC_CACHE_CHECK(where wtmpx is located, path_wtmpx,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif
#include <errno.h>
main()
{
    char **w, *wtmplist[] = {
    "/var/log/wtmpx", "/var/adm/wtmpx", NULL };
    FILE *a, *f=fopen("conftestval", "w");
    if (!f) exit(1);
#ifdef WTMPX_FILE
    fprintf(f, "%s\n", WTMPX_FILE);
    exit(0);
#endif
#ifdef _PATH_WTMPX
    fprintf(f, "%s\n", _PATH_WTMPX);
    exit(0);
#endif
    for (w = wtmplist; *w; w++) {
	if ((a = fopen(*w, "r")) != NULL || errno == EACCES) {
	    fprintf(f, "%s\n", *w);
	    exit(0);
	}
    }
    exit(0);
}]])],[path_wtmpx=`cat conftestval`],[path_wtmpx=],[dnl
  AC_MSG_WARN(Define WTMPX_FILE in config.h manually)])])
if test x$path_wtmpx != x; then
  AC_DEFINE_UNQUOTED(WTMPX_FILE, "$path_wtmpx", Define location of wtmpx)
fi
dnl# --------------------------------------------------------------------------

dnl# find lastlog
AC_CACHE_CHECK(where lastlog is located, path_lastlog,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#elif defined(HAVE_UTMP_H)
#include <utmp.h>
#endif
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
#include <errno.h>
main()
{
    char **w, *lastloglist[] = { "/var/log/lastlog", NULL };
    FILE *a, *f=fopen("conftestval", "w");
    if (!f) exit(1);
#ifdef LASTLOG_FILE
    fprintf(f, "%s\n", LASTLOG_FILE);
    exit(0);
#endif
#ifdef _PATH_LASTLOG
    fprintf(f, "%s\n", _PATH_LASTLOG);
    exit(0);
#endif
    for (w = lastloglist; *w; w++) {
	if ((a = fopen(*w, "r")) != NULL || errno == EACCES) {
	    fprintf(f, "%s\n", *w);
	    exit(0);
	}
    }
    exit(0);
}]])],[path_lastlog=`cat conftestval`],[path_lastlog=],[dnl
  AC_MSG_WARN(Define LASTLOG_FILE in config.h manually)])])
if test x$path_lastlog != x; then
  AC_DEFINE_UNQUOTED(LASTLOG_FILE, "$path_lastlog", Define location of lastlog)
fi
dnl# --------------------------------------------------------------------------

dnl# find lastlogx
AC_CACHE_CHECK(where lastlogx is located, path_lastlogx,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif
#include <errno.h>
main()
{
    char **w, *wtmplist[] = { "/var/log/lastlogx", "/var/adm/lastlogx", NULL };
    FILE *a, *f=fopen("conftestval", "w");
    if (!f) exit(1);
#ifdef LASTLOGX_FILE
    fprintf(f, "%s\n", LASTLOGX_FILE);
    exit(0);
#endif
#ifdef _PATH_LASTLOGX
    fprintf(f, "%s\n", _PATH_LASTLOGX);
    exit(0);
#endif
    for (w = wtmplist; *w; w++) {
	if ((a = fopen(*w, "r")) != NULL || errno == EACCES) {
	    fprintf(f, "%s\n", *w);
	    exit(0);
	}
    }
    exit(0);
}]])],[path_lastlogx=`cat conftestval`],[path_lastlogx=],[dnl
  AC_MSG_WARN(Define LASTLOGX_FILE in config.h manually)])])
if test x$path_lastlogx != x; then
  AC_DEFINE_UNQUOTED(LASTLOGX_FILE, "$path_lastlogx", Define location of lastlogx)
fi
])

AC_DEFUN([SCM_RIGHTS_CHECK],
[
AC_CACHE_CHECK(for unix-compliant filehandle passing ability, can_pass_fds,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstddef> // broken bsds (is that redundant?) need this
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
]], [[
{
  msghdr msg;
  iovec iov;
  char buf [100];
  char data = 0;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = buf;
  msg.msg_controllen = sizeof buf;

  cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;
  cmsg->cmsg_len   = 100;

  *(int *)CMSG_DATA (cmsg) = 5;

  return sendmsg (3, &msg, 0);
}
]])],[can_pass_fds=yes],[can_pass_fds=no])])
if test x$can_pass_fds = xyes; then
   AC_DEFINE(HAVE_UNIX_FDPASS, 1, Define if sys/socket.h defines the necessary macros/functions for file handle passing)
else
   AC_MSG_ERROR([libptytty requires unix-compliant filehandle passing ability])
fi
])

AC_DEFUN([TTY_GROUP_CHECK],
[
AC_CACHE_CHECK([for tty group], tty_group,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>

main()
{
  struct stat st;
  struct group *gr;
  char *tty;
  gr = getgrnam("tty");
  tty = ttyname(0);
  if (gr != 0
      && tty != 0
      && (stat(tty, &st)) == 0
      && st.st_gid == gr->gr_gid)
    return 0;
  else
    return 1;
}]])],[tty_group=yes],[tty_group=no],[tty_group=no])])
if test x$tty_group = xyes; then
  AC_DEFINE(TTY_GID_SUPPORT, 1, "")
fi])

