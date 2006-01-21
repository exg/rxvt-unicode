dnl> test to find the hard-to-find libXpm
dnl> mostly copied from AC_PATH_X & AC_PATH_DIRECT, but explictly set

AC_DEFUN([VT_FIND_LIBXPM],
[
AC_REQUIRE_CPP()

# Initialize some more variables set by options.
# The variables have the same names as the options, with
# dashes changed to underlines.

# If we find XPM, set shell vars xpm_includes and xpm_libraries to the
# paths, otherwise set no_xpm=yes.
# Uses ac_ vars as temps to allow command line to override cache and checks.
AC_MSG_CHECKING(for libXpm)

AC_ARG_WITH(xpm_includes,
  [  --with-xpm-includes=DIR use XPM includes in DIR],
  xpm_includes="$withval", xpm_includes=NO)
AC_ARG_WITH(xpm_library,
  [  --with-xpm-library=DIR  use XPM library in DIR],
  xpm_libraries="$withval", xpm_libraries=NO)

# --without-xpm overrides everything else, but does not touch the cache.
AC_ARG_WITH(xpm,
  [  --with-xpm              use XPM])
if test "$with_xpm" = no; then
  have_xpm=disabled
else
  AC_CACHE_VAL(ac_cv_have_xpm, [
  vt_xpm_include_X11=no
  if test -n "$xpm_includes"; then
    vt_xpm_includes=$xpm_includes
  else
    vt_xpm_includes=NO
  fi
  if test -n "$xpm_libraries"; then
    vt_xpm_libraries=$xpm_libraries
  else
    vt_xpm_libraries=NO
  fi

  VT_XPM_DIRECT

  if test "$vt_xpm_includes" = NO -o "$vt_xpm_libraries" = NO; then
    ac_cv_have_xpm="have_xpm=no"
  else
    ac_cv_have_xpm="have_xpm=yes \
        vt_xpm_includes=$vt_xpm_includes vt_xpm_libraries=$vt_xpm_libraries \
	vt_xpm_include_X11=$vt_xpm_include_X11"
  fi])dnl
  eval "$ac_cv_have_xpm"
fi

if test "$have_xpm" != yes; then
  AC_MSG_RESULT($have_xpm)
  no_xpm=yes
else
  if test "$xpm_includes" != NO; then
    if test "$xpm_includes" = "$vt_xpm_includes"; then
      if test -r "$xpm_includes/X11/xpm.h"; then
	vt_xpm_include_X11=yes
      fi
    else
      vt_xpm_include_X11=no
      if test -z "$xpm_includes"; then
	AC_TRY_CPP([#include <X11/xpm.h>],
	vt_xpm_include_X11=yes)
      else
        if test -r "$xpm_includes/X11/xpm.h"; then
	  vt_xpm_include_X11=yes
        fi
      fi
    fi
    vt_xpm_includes=$xpm_includes
  fi
  if test "x$xpm_libraries" != xNO; then
    vt_xpm_libraries=$xpm_libraries
  fi
  # Update the cache value to reflect the command line values.
  ac_cv_have_xpm="have_xpm=yes \
	vt_xpm_includes=$vt_xpm_includes vt_xpm_libraries=$vt_xpm_libraries \
	vt_xpm_include_X11=$vt_xpm_include_X11"
  eval "$ac_cv_have_xpm"
  AC_MSG_RESULT([-I$vt_xpm_includes, -L$vt_xpm_libraries])
  if test -n "$vt_xpm_includes"; then
    XPM_CPPFLAGS="-DHAVE_LIBXPM"
  fi
  if test -n "$vt_xpm_includes"; then
    XPM_CFLAGS="-I$vt_xpm_includes"
  fi
  XPM_LIBS="-lXpm"
  if test -n "$vt_xpm_libraries"; then
    XPM_LIBS="-L$vt_xpm_libraries $XPM_LIBS"
  fi
  if test "x$vt_xpm_include_X11" = xyes; then
    AC_DEFINE(XPM_INC_X11, 1, Define if you include <X11/xpm.h> on a normal include path (be careful))
  fi
fi

AC_SUBST(XPM_CPPFLAGS)
AC_SUBST(XPM_CFLAGS)
AC_SUBST(XPM_LIBS)
])

dnl Internal subroutine of VT_FIND_LIBXPM
dnl Set vt_xpm_include and vt_xpm_libr
# -------------- find xpm.h and Xpm.a/Xpm.so/Xpm.sl
AC_DEFUN([VT_XPM_DIRECT],
[if test "$vt_xpm_includes" = NO; then
  # Guess where to find xpm.h

ac_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $X_CFLAGS"

  # First, try using that file with no special directory specified.
AC_TRY_CPP([#include <X11/xpm.h>],
[# We can compile using X headers with no special include directory.
vt_xpm_includes=
vt_xpm_include_X11=yes],
[CPPFLAGS="$ac_save_CPPFLAGS"
# Look for the header file in a standard set of common directories.
  for ac_dir in               \
    /usr/X11/include          \
    /usr/X11R6/include        \
    /usr/X11R5/include        \
    /usr/X11R4/include        \
                              \
    /usr/include/X11          \
    /usr/include/X11R6        \
    /usr/include/X11R5        \
    /usr/include/X11R4        \
                              \
    /usr/local/X11/include    \
    /usr/local/X11R6/include  \
    /usr/local/X11R5/include  \
    /usr/local/X11R4/include  \
                              \
    /usr/local/include/X11    \
    /usr/local/include/X11R6  \
    /usr/local/include/X11R5  \
    /usr/local/include/X11R4  \
                              \
    /usr/X386/include         \
    /usr/x386/include         \
    /usr/XFree86/include/X11  \
                              \
    /usr/include              \
    /usr/local/include        \
    /usr/unsupported/include  \
    /usr/athena/include       \
    /usr/local/x11r5/include  \
    /usr/lpp/Xamples/include  \
                              \
    /usr/openwin/include      \
    /usr/openwin/share/include \
    ; \
  do
    if test -r "$ac_dir/X11/xpm.h"; then
      vt_xpm_includes="$ac_dir"
      vt_xpm_include_X11=yes
      break
    else
      if test -r "$ac_dir/xpm.h"; then
        vt_xpm_includes=$ac_dir
        break
      fi
    fi
  done])
fi

if test "$vt_xpm_libraries" = NO; then
  # Check for the libraries.

  # See if we find them without any special options.
  # Don't add to $LIBS permanently.
  ac_save_LIBS="$LIBS"
  LIBS="$LIBS $X_LIBS -lXpm -lX11"
AC_TRY_LINK(, [XpmReadFileToPixmap()],
[LIBS="$ac_save_LIBS"
# We can link libXpm with no special library path.
vt_xpm_libraries=],
[LIBS="$ac_save_LIBS"
# First see if replacing the include by lib works.
for ac_dir in \
    `echo "$vt_xpm_includes" | sed 's,include/X11,lib,;s,include,lib,'` \
    /usr/X11/lib          \
    /usr/X11R6/lib        \
    /usr/X11R5/lib        \
    /usr/X11R4/lib        \
                          \
    /usr/lib/X11          \
    /usr/lib/X11R6        \
    /usr/lib/X11R5        \
    /usr/lib/X11R4        \
                          \
    /usr/local/X11/lib    \
    /usr/local/X11R6/lib  \
    /usr/local/X11R5/lib  \
    /usr/local/X11R4/lib  \
                          \
    /usr/local/lib/X11    \
    /usr/local/lib/X11R6  \
    /usr/local/lib/X11R5  \
    /usr/local/lib/X11R4  \
                          \
    /usr/X386/lib         \
    /usr/x386/lib         \
    /usr/XFree86/lib/X11  \
                          \
    /usr/lib              \
    /usr/local/lib        \
    /usr/unsupported/lib  \
    /usr/athena/lib       \
    /usr/local/x11r5/lib  \
    /usr/lpp/Xamples/lib  \
                          \
    /usr/openwin/lib      \
    /usr/openwin/share/lib \
    ; \
do
dnl XXX Shouldn't this really use AC_TRY_LINK to be portable & robust??
  for ac_extension in a so sl; do
    if test -r $ac_dir/libXpm.$ac_extension; then
      vt_xpm_libraries=$ac_dir
      break 2
    fi
  done
done])
fi
])

AC_DEFUN([PTY_CHECK],
[
AC_CHECK_HEADERS( \
  pty.h \
  util.h \
  libutil.h \
  sys/ioctl.h \
  sys/stropts.h \
)

AC_CHECK_FUNCS( \
  revoke \
  _getpty \
  getpt \
  posix_openpt \
  isastream \
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
  AC_TRY_LINK([#include <stdlib.h>],
              [grantpt(0);unlockpt(0);ptsname(0);],
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
AC_CHECK_FUNCS( \
	ttyslot \
	updwtmp \
	updwtmpx \
)

AC_CHECK_HEADERS( \
	utmp.h \
	utmpx.h \
	lastlog.h \
)

dnl# --------------------------------------------------------------------------
dnl# DO ALL UTMP AND WTMP CHECKING
dnl# --------------------------------------------------------------------------
dnl# check for host field in utmp structure

dnl# --------------------------------------------
AC_CHECK_HEADER(utmp.h,
[AC_CACHE_CHECK([for struct utmp], rxvt_cv_struct_utmp,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut;]])],[rxvt_cv_struct_utmp=yes],[rxvt_cv_struct_utmp=no])])
if test x$rxvt_cv_struct_utmp = xyes; then
  AC_DEFINE(HAVE_STRUCT_UTMP, 1, Define if utmp.h has struct utmp)
fi
]

AC_CACHE_CHECK(for ut_host in utmp struct, rxvt_cv_struct_utmp_host,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut; ut.ut_host;]])],[rxvt_cv_struct_utmp_host=yes],[rxvt_cv_struct_utmp_host=no])])
if test x$rxvt_cv_struct_utmp_host = xyes; then
  AC_DEFINE(HAVE_UTMP_HOST, 1, Define if struct utmp contains ut_host)
fi

AC_CACHE_CHECK(for ut_pid in utmp struct, rxvt_cv_struct_utmp_pid,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut; ut.ut_pid;]])],[rxvt_cv_struct_utmp_pid=yes],[rxvt_cv_struct_utmp_pid=no])])
if test x$rxvt_cv_struct_utmp_pid = xyes; then
  AC_DEFINE(HAVE_UTMP_PID, 1, Define if struct utmp contains ut_pid)
fi
) dnl# AC_CHECK_HEADER(utmp.h

dnl# --------------------------------------------

AC_CHECK_HEADER(utmpx.h,
[AC_CACHE_CHECK([for struct utmpx], rxvt_cv_struct_utmpx,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx ut;]])],[rxvt_cv_struct_utmpx=yes],[rxvt_cv_struct_utmpx=no])])
if test x$rxvt_cv_struct_utmpx = xyes; then
  AC_DEFINE(HAVE_STRUCT_UTMPX, 1, Define if utmpx.h has struct utmpx)
fi
]

AC_CACHE_CHECK(for host in utmpx struct, rxvt_cv_struct_utmpx_host,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx utx; utx.ut_host;]])],[rxvt_cv_struct_utmpx_host=yes],[rxvt_cv_struct_utmpx_host=no])])
if test x$rxvt_cv_struct_utmpx_host = xyes; then
  AC_DEFINE(HAVE_UTMPX_HOST, 1, Define if struct utmpx contains ut_host)
fi

AC_CACHE_CHECK(for session in utmpx struct, rxvt_cv_struct_utmpx_session,
[AC_TRY_COMPILE([#include <sys/types.h>
#include <utmpx.h>],
[struct utmpx utx; utx.ut_session;],
rxvt_cv_struct_utmpx_session=yes, rxvt_cv_struct_utmpx_session=no)])
if test x$rxvt_cv_struct_utmpx_session = xyes; then
  AC_DEFINE(HAVE_UTMPX_SESSION, 1, Define if struct utmpx contains ut_session)
fi
) dnl# AC_CHECK_HEADER(utmpx.h

dnl# --------------------------------------------------------------------------
dnl# check for struct lastlog
AC_CACHE_CHECK(for struct lastlog, rxvt_cv_struct_lastlog,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
]], [[struct lastlog ll;]])],[rxvt_cv_struct_lastlog=yes],[rxvt_cv_struct_lastlog=no])])
if test x$rxvt_cv_struct_lastlog = xyes; then
  AC_DEFINE(HAVE_STRUCT_LASTLOG, 1, Define if utmp.h or lastlog.h has struct lastlog)
fi

dnl# check for struct lastlogx
AC_CACHE_CHECK(for struct lastlogx, rxvt_cv_struct_lastlogx,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
]], [[struct lastlogx ll;]])],[rxvt_cv_struct_lastlogx=yes],[rxvt_cv_struct_lastlogx=no])])
if test x$rxvt_cv_struct_lastlogx = xyes; then
  AC_DEFINE(HAVE_STRUCT_LASTLOGX, 1, Define if utmpx.h or lastlog.h has struct lastlogx)
fi

dnl# --------------------------------------------------------------------------
dnl# FIND FILES
dnl# --------------------------------------------------------------------------

dnl# find utmp
AC_CACHE_CHECK(where utmp is located, rxvt_cv_path_utmp,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
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
}]])],[rxvt_cv_path_utmp=`cat conftestval`],[rxvt_cv_path_utmp=],[dnl
  AC_MSG_WARN(Define RXVT_UTMP_FILE in config.h manually)])])
if test x$rxvt_cv_path_utmp != x; then
  AC_DEFINE_UNQUOTED(RXVT_UTMP_FILE, "$rxvt_cv_path_utmp", Define location of utmp)
fi

dnl# --------------------------------------------------------------------------

dnl# find utmpx - if a utmp file exists at the same location and is more than
dnl# a day newer, then dump the utmpx.  People leave lots of junk around.
AC_CACHE_CHECK(where utmpx is located, rxvt_cv_path_utmpx,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
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
}]])],[rxvt_cv_path_utmpx=`cat conftestval`],[rxvt_cv_path_utmpx=],[dnl
  AC_MSG_WARN(Define RXVT_UTMPX_FILE in config.h manually)])])
if test x$rxvt_cv_path_utmpx != x; then
  AC_DEFINE_UNQUOTED(RXVT_UTMPX_FILE, "$rxvt_cv_path_utmpx", Define location of utmpx)
fi

dnl# --------------------------------------------------------------------------

dnl# find wtmp
AC_CACHE_CHECK(where wtmp is located, rxvt_cv_path_wtmp,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
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
}]])],[rxvt_cv_path_wtmp=`cat conftestval`],[rxvt_cv_path_wtmp=],[dnl
  AC_MSG_WARN(Define RXVT_WTMP_FILE in config.h manually)])])
if test x$rxvt_cv_path_wtmp != x; then
  AC_DEFINE_UNQUOTED(RXVT_WTMP_FILE, "$rxvt_cv_path_wtmp", Define location of wtmp)
fi
dnl# --------------------------------------------------------------------------

dnl# find wtmpx
AC_CACHE_CHECK(where wtmpx is located, rxvt_cv_path_wtmpx,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
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
}]])],[rxvt_cv_path_wtmpx=`cat conftestval`],[rxvt_cv_path_wtmpx=],[dnl
  AC_MSG_WARN(Define RXVT_WTMPX_FILE in config.h manually)])])
if test x$rxvt_cv_path_wtmpx != x; then
  AC_DEFINE_UNQUOTED(RXVT_WTMPX_FILE, "$rxvt_cv_path_wtmpx", Define location of wtmpx)
fi
dnl# --------------------------------------------------------------------------

dnl# find lastlog
AC_CACHE_CHECK(where lastlog is located, rxvt_cv_path_lastlog,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
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
}]])],[rxvt_cv_path_lastlog=`cat conftestval`],[rxvt_cv_path_lastlog=],[dnl
  AC_MSG_WARN(Define RXVT_LASTLOG_FILE in config.h manually)])])
if test x$rxvt_cv_path_lastlog != x; then
  AC_DEFINE_UNQUOTED(RXVT_LASTLOG_FILE, "$rxvt_cv_path_lastlog", Define location of lastlog)
  if test -d "$rxvt_cv_path_lastlog"; then
    AC_DEFINE(LASTLOG_IS_DIR, 1, Define if lastlog is provided via a directory)
  fi
fi
dnl# --------------------------------------------------------------------------

dnl# find lastlogx
AC_CACHE_CHECK(where lastlogx is located, rxvt_cv_path_lastlogx,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
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
}]])],[rxvt_cv_path_lastlogx=`cat conftestval`],[rxvt_cv_path_lastlogx=],[dnl
  AC_MSG_WARN(Define RXVT_LASTLOGX_FILE in config.h manually)])])
if test x$rxvt_cv_path_lastlogx != x; then
  AC_DEFINE_UNQUOTED(RXVT_LASTLOGX_FILE, "$rxvt_cv_path_lastlogx", Define location of lastlogx)
fi
])

