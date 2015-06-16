m4_include([libptytty/ptytty.m4])

dnl maybe import pkg.m4 and use PKG_CHECK_MODULES in place of this macro
AC_DEFUN([RXVT_CHECK_MODULES],
[
  AC_MSG_CHECKING(for $2)
  if test $PKG_CONFIG != no && $PKG_CONFIG --exists $2; then
    $1[]_CFLAGS="`$PKG_CONFIG --cflags $2`"
    $1[]_LIBS="`$PKG_CONFIG --libs $2`"

    AC_MSG_RESULT(ok)
    $3
  else
    AC_MSG_RESULT(no)
    $4
  fi
])
