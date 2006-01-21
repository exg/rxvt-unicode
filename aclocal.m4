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

