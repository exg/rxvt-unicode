dnl#
dnl# Process this file with autoconf to produce a configure script.
dnl#

AC_INIT
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([src/feature.h])
AC_CONFIG_HEADER(config.h:config.h.in)

AC_CANONICAL_HOST

dnl RXVT version
changequote(, )dnl
VERSION=`sed -n -e 's/^.*[ \t]VERSION.*"\(.*\)"$/\1/p' ${srcdir}/src/version.h`
DATE=`sed -n -e 's/^.*[ \t]DATE.*"\(.*\)"$/\1/p' ${srcdir}/src/version.h`
dnl LIBVERSION=`sed -n -e 's/^.*[ \t]LIBVERSION.*"\(.*\)"$/\1/p' ${srcdir}/src/version.h`
changequote([, ])dnl
AC_SUBST(VERSION)dnl
AC_SUBST(DATE)dnl
AC_SUBST(LSMDATE)dnl
AC_SUBST(LIBVERSION)dnl
echo ""
echo "configuring for rxvt $VERSION"
echo ""

orig_CXXFLAGS="$CXXFLAGS"

dnl# Checks for programs.
dnl AC_MAKE_SET

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL

AC_AIX
AC_ISC_POSIX
AC_LANG(C++)

dnl check wether we can link with gcc -lsupc++
if test x$GCC = xyes && test x$GXX = xyes; then
   dnl FreeBSD (at least up to 5.3) has a broken GCC, workaround it
   AC_MSG_CHECKING([for working libsupc++])
   save_CXX="$CXX"
   save_LIBS="$LIBS"
   CXX="$CC"
   LIBS="$LIBS -lsupc++"
   LINKER="$CC"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([struct test { }; void f() try { throw new test; } catch (...) { throw; }],[])],
      [AC_MSG_RESULT(ok)],
      [
         AC_MSG_RESULT([no, making everything bigger and slower])
         LIBS="$save_LIBS"
         LINKER="$save_CXX"
      ]
   )
   CXX="$save_CXX"
fi
AC_SUBST(LINKER,[$LINKER])

dnl# --------------------------------------------------------------------------
dnl# Supply default CXXFLAGS, if not specified by `CXXFLAGS=flags ./configure'
dnl#
if test -z "$orig_CXXFLAGS"; then
  if test x$GCC = xyes && test "x$GXX" = xyes; then
     CXXFLAGS="-g -O3 -fno-threadsafe-statics -fno-enforce-eh-specs"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])], [], [CXXFLAGS="-g -O3"])
  else
    CXXFLAGS="-O"
  fi
  CFLAGS="$CXXFLAGS"
fi

AC_DEFINE(_GNU_SOURCE, 1, Use all glibc features.)

case $host in
   *-*-solaris* )
      AC_DEFINE(_XOPEN_SOURCE_EXTENDED, 1, Needed to get declarations for msg_control and msg_controllen on Solaris)
      AC_DEFINE(_XOPEN_SOURCE,          2, Needed to get declarations for msg_control and msg_controllen on Solaris)
      AC_DEFINE(__EXTENSIONS__,         1, Needed to get declarations for msg_control and msg_controllen on Solaris)
      ;;
esac

dnl if test x$GXX = xyes; then
dnl    CXXFLAGS="$CXXFLAGS" dnl I once had -fno-exceptions, but I am using them now...
dnl fi

dnl# FreeBSD needs to link libxpg4
AC_CHECK_FUNC(setlocale,     [], [AC_CHECK_LIB(xpg4,   setlocale,     [LIBS="$LIBS -lxpg4"])])

dnl# solaris needs to link libnsl and socket
AC_CHECK_FUNC(gethostbyname, [], [AC_CHECK_LIB(nsl,    gethostbyname, [LIBS="$LIBS -lnsl"])])
AC_CHECK_FUNC(socket,        [], [AC_CHECK_LIB(socket, socket,        [LIBS="$LIBS -lsocket"])])

dnl AC_ENABLE_SHARED(no)dnl#	libtool
dnl AC_ENABLE_STATIC(yes)dnl#	libtool
dnl AC_PROG_LIBTOOL()dnl#		libtool

support_frills=yes
support_inheritpixmap=yes
support_tinting=yes
support_fading=yes
support_keepscrolling=yes
support_selectionscrolling=yes
support_mousewheel=yes
support_mouseslipwheel=yes
support_text_blink=yes
support_pointer_blank=yes
support_scroll_rxvt=yes
support_scroll_next=yes
support_scroll_xterm=yes
support_scroll_plain=yes
support_xim=yes
support_xpm=yes
support_afterimage=yes
support_afterstep=yes
support_xft=yes
support_unicode3=no
support_combining=yes
support_8bitctrls=no
support_iso14755=yes
support_styles=yes
support_perl=yes
codesets=all

dnl# --------------------------------------------------------------------------
dnl# CHECKING COMMAND LINE OPTIONS
dnl# --------------------------------------------------------------------------

if test "x$enable_shared" = xyes; then
    AC_DEFINE(LIBRXVT, 1, Build shared library version - specify via configure only)
    INSTALL_LIBRXVT=yes
fi
AC_SUBST(INSTALL_LIBRXVT)

AC_ARG_ENABLE(everything,
  [  --enable-everything     enable standard non-multichoice features
                     NOTE: this option is order dependent
                     NOTE: automatically enabled with --enable-shared],
  [
    if test x$enableval = xno; then
       support_frills=no
       support_inheritpixmap=no
       support_tinting=no
       support_fading=no
       support_keepscrolling=no
       support_selectionscrolling=no
       support_lastlog=no
       support_mousewheel=no
       support_mouseslipwheel=no
       support_text_blink=no
       support_pointer_blank=no
       support_scroll_rxvt=no
       support_scroll_next=no
       support_scroll_xterm=no
       support_scroll_plain=no
       support_utmp=no
       support_wtmp=no
       support_xim=no
       support_xpm=no
       support_afterimage=no
       support_afterstep=no
       support_xft=no
       support_unicode3=no
       support_combining=no
       support_8bitctrls=no
       support_iso14755=no
       support_styles=no
       support_perl=no
       codesets=
    fi
    if test x$enableval = xyes; then
       support_frills=yes
       support_inheritpixmap=yes
       support_tinting=yes
       support_fading=yes
       support_keepscrolling=yes
       support_selectionscrolling=yes
       support_lastlog=yes
       support_mousewheel=yes
       support_mouseslipwheel=yes
       support_text_blink=yes
       support_pointer_blank=yes
       support_scroll_rxvt=yes
       support_scroll_next=yes
       support_scroll_xterm=yes
       support_scroll_plain=yes
       support_utmp=yes
       support_wtmp=yes
       support_xim=yes
       support_xpm=yes
       support_afterimage=yes
       support_afterstep=yes
       support_xft=yes
       support_unicode3=yes
       support_combining=yes
       #support_8bitctrls=yes
       support_iso14755=yes
       support_styles=yes
       support_perl=yes
       codesets=all
    fi
  ])

AC_ARG_ENABLE(unicode3,
  [  --enable-unicode3       use 21 instead of 16 bits to represent unicode characters],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_unicode3=$enableval
  fi])

AC_ARG_ENABLE(combining,
  [  --enable-combining      enable composition of base and combining characters],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_combining=$enableval
  fi])

AC_ARG_ENABLE(xft,
  [  --enable-xft            enable xft support on systems that have it],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_xft=$enableval
  fi])

AC_ARG_ENABLE(font-styles,
  [  --enable-font-styles    enable bold and italic support],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_styles=$enableval
  fi])

AC_ARG_ENABLE(xpm-background,
  [  --enable-xpm-background enable background pixmaps],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_xpm=$enableval
  fi])

AC_ARG_ENABLE(afterimage,
  [  --enable-afterimage enable integration with libAfterImage for background images],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_afterimage=$enableval
  fi])

AC_ARG_WITH(afterimage_config,
  [  --with-afterimage-config=DIR  use libAfterImage config script in DIR],
  [if test "x$withval" != x; then
    afterimage_config=$withval/afterimage-config
  fi])

AC_ARG_ENABLE(afterstep,
  [  --enable-afterstep enable integration with AfterStep window manager],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_afterstep=$enableval
  fi])

AC_ARG_ENABLE(transparency,
  [  --enable-transparency   enable transparent backgrounds],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_inheritpixmap=$enableval
  fi])

AC_ARG_ENABLE(tinting,
  [  --enable-tinting        enable tinting of transparent bg],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_tinting=$enableval
  fi])

AC_ARG_ENABLE(fading,
  [  --enable-fading         enable colors fading when off focus],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_fading=$enableval
  fi])

AC_ARG_ENABLE(rxvt-scroll,
  [  --enable-rxvt-scroll    enable rxvt style scrollbar],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_scroll_rxvt=$enableval
  fi])

AC_ARG_ENABLE(next-scroll,
  [  --enable-next-scroll    enable NeXT style scrollbar],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_scroll_next=$enableval
  fi])

AC_ARG_ENABLE(xterm-scroll,
  [  --enable-xterm-scroll   enable Xterm style scrollbar],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_scroll_xterm=$enableval
  fi])

AC_ARG_ENABLE(perl,
  [  --enable-perl           enable embedded perl interpreter],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_perl=$enableval
  fi])

AC_ARG_ENABLE(plain-scroll,
  [  --enable-plain-scroll   enable plain style scrollbar],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_scroll_plain=$enableval
  fi])

AC_ARG_WITH(codesets,
  [  --with-codesets=NAME,.. compile in additional codesets (jp,jp_ext,kr,zh,zh_ext,all)],
  [codesets="$withval"])

AC_ARG_ENABLE(xim,
  [  --enable-xim            XIM (X Input Method) protocol support],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_xim=$enableval
  fi])

AC_ARG_ENABLE(backspace-key,
  [  --disable-backspace-key disable handling of the backspace key],
  [if test x$enableval = xno; then
    AC_DEFINE(NO_BACKSPACE_KEY, 1, Define if you don't want support for the backspace key)
  fi])

AC_ARG_ENABLE(delete-key,
  [  --disable-delete-key    disable handling of the delete key],
  [if test x$enableval = xno; then
    AC_DEFINE(NO_DELETE_KEY, 1, Define if you don't want support for the (non-keypad) delete key)
  fi])

AC_ARG_ENABLE(resources,
  [  --disable-resources     disable all resource checking],
  [if test x$enableval = xno; then
    AC_DEFINE(NO_RESOURCES, 1, Define if you don't want any resources read)
  fi])

AC_ARG_ENABLE(8bitctrls,
  [  --enable-8bitctrls      enable 8 bit control sequences (not recommended)],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_8bitctrls=$enableval
  fi])

RESFALLBACK=Rxvt
AC_ARG_ENABLE(fallback,
  [  --enable-fallback(=CLASS) fall back on CLASS resources in addition to URxvt ones (default: Rxvt)],
  [
   test x$enableval = xyes && enableval=Rxvt
   test x$enableval = xno  && enableval=
   RESFALLBACK="$enableval"
  ])

if test x$RESFALLBACK != x; then
   AC_DEFINE_UNQUOTED(RESFALLBACK, "$RESFALLBACK", [Fallback resource class])
fi

RESNAME=urxvt
AC_ARG_WITH(res-name,
  [  --with-res-name=NAME    use this as default resource name (default: urxvt)],
  [RESNAME="$withval"])
AC_DEFINE_UNQUOTED(RESNAME,"$RESNAME", [Default resource name])

RESCLASS=URxvt
AC_ARG_WITH(res-class,
  [  --with-res-class=CLASS  use this as the resource class (default: URxvt)],
  [RESCLASS="$withval"])
AC_DEFINE_UNQUOTED(RESCLASS,"$RESCLASS", [Resource class])

RXVTNAME=urxvt
AC_ARG_WITH(name,
  [  --with-name=NAME        set the basename for the installed binaries (default: urxvt)],
  [RXVTNAME="$withval"])
AC_DEFINE_UNQUOTED(RXVTNAME,"$RXVTNAME", [Binary base name])

RXVTNAME=`echo "$RXVTNAME"|sed "$program_transform_name"`
AC_SUBST(RXVTNAME)

AC_ARG_ENABLE(swapscreen,
  [  --disable-swapscreen    disable swap screen support],
  [if test x$enableval = xno; then
    AC_DEFINE(NO_SECONDARY_SCREEN, 1, Disable the secondary screen. Many programs use the secondary screen as their workplace)
  fi])

AC_ARG_ENABLE(iso14755,
  [  --enable-iso14755       enable support for extended ISO 14755 modes],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_iso14755=$enableval
  fi])

AC_ARG_ENABLE(frills,
  [  --enable-frills         enable support for rarely used features],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_frills=$enableval
  fi])

AC_ARG_ENABLE(keepscrolling,
  [  --enable-keepscrolling  enable continual scrolling on scrollbar arrow press],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_keepscrolling=$enableval
  fi])

AC_ARG_ENABLE(selectionscrolling,
  [  --enable-selectionscrolling  enable scrolling during selections],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_selectionscrolling=$enableval
  fi])

AC_ARG_ENABLE(mousewheel,
  [  --enable-mousewheel     enable scrolling via mouse wheel or buttons 4 & 5],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_mousewheel=$enableval
  fi])

AC_ARG_ENABLE(slipwheeling,
  [  --enable-slipwheeling   enable slip wheel scrolling     (requires previous)],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_mouseslipwheel=$enableval
  fi])

AC_ARG_ENABLE(smart-resize,
  [  --enable-smart-resize   enable smart growth/shrink behaviour],
  [if test x$enableval = xyes; then
    AC_DEFINE(SMART_RESIZE, 1, Define to use "smart" resize behavior)
  fi])

AC_ARG_ENABLE(text-blink,
  [  --enable-text-blink     enable blinking text],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_text_blink=$enableval
  fi])

AC_ARG_ENABLE(pointer-blank,
  [  --enable-pointer-blank  enable pointer blank when typing or inactive pointer],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_pointer_blank=$enableval
  fi])

AC_ARG_WITH(term,
  [  --with-term=NAME        set the terminal to NAME (default: rxvt)],
  [if test x$withval != x; then
    AC_DEFINE_UNQUOTED(TERMENV, "$withval",Set TERM to the value given by configure) term="$withval"
  fi])

AC_ARG_WITH(terminfo,
  [  --with-terminfo=PATH    set the path to the terminfo tree to PATH],
  [if test x$withval != x; then
    AC_DEFINE_UNQUOTED(RXVT_TERMINFO, "$withval", Set TERMINFO value to the value given by configure) terminfo="$withval"
  fi])

dnl# --------------------------------------------------------------------------

AC_PATH_PROG(MV, mv, mv)
AC_PATH_PROG(CP, cp, cp)
AC_PATH_PROG(LN, ln, ln)
AC_PATH_PROG(SED, sed, sed)
AC_PATH_PROG(ECHO, echo, echo)
AC_PATH_PROG(CMP, cmp, cmp)
AC_PATH_PROG(TIC, tic)

dnl# need a neat way to detect SVR4 or its features
dnl# in src/command.c we use these functions:
dnl# grantpt(), unlockpt(), ptsname(), which are defined in <sys/ptms.h>
dnl# - but are these also defined for other systems?

dnl# hack to find if this is SVR4 -- who knows?
dnl## AC_MSG_CHECKING(for SVR4)
dnl## AC_EGREP_CPP(yes,
dnl## [#if defined (SVR4) || defined (_SVR4) || defined (__svr4__)
dnl##   yes;
dnl## #endif
dnl## ], [AC_MSG_RESULT(yes); AC_DEFINE(PERHAPS_SVR4)], AC_MSG_RESULT(perhaps not?))

AC_PATH_XTRA

dnl# the only reasonable way to find libXpm is do-it-yourself
dnl# only check if we want xpm-background

LIBAFTERIMAGE_CFLAGS=
AFTERIMAGE_LIBS=
have_afterimage=

if test x$support_afterimage = xyes; then
  support_afterimage=no

  if test "x$afterimage_config" = "x" ; then
    AC_PATH_PROG(afterimage_config, afterimage-config, no)
  fi

  AC_MSG_CHECKING(for libAfterImage)
  if $afterimage_config --version >/dev/null 2>&1 ; then
    LIBAFTERIMAGE_CFLAGS=`$afterimage_config --cflags`
    AFTERIMAGE_LIBS=`$afterimage_config --libs`
    if test "x$AFTERIMAGE_LIBS" != "x"; then
      support_afterimage=yes
    fi
  fi

  if test "x$support_afterimage" = "xyes"; then
    AC_MSG_RESULT($AFTERIMAGE_LIBS)
  else
    AC_MSG_RESULT(no)
  fi
fi

if test x$support_afterimage != xyes -a x$support_xpm = xyes ; then
  VT_FIND_LIBXPM
  if test x$no_xpm = xyes; then
    support_xpm=needsmanualspecification
  fi
fi

dnl# --------------------------------------------------------------------------
dnl# CHECKING FOR HEADER FILES
dnl# --------------------------------------------------------------------------
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS( \
	assert.h \
	fcntl.h \
	stdarg.h \
	stdlib.h \
	string.h \
	termios.h \
	unistd.h \
	sys/byteorder.h \
	sys/ioctl.h \
	sys/select.h \
	sys/sockio.h \
	sys/strredir.h \
	sys/time.h \
	stdint.h \
	wchar.h \
	cwchar \
	clocale \
)

AC_HEADER_TIME

dnl# check to allow both <termios.h> and <sys/ioctl.h>
AC_CACHE_CHECK(whether termios.h and sys/ioctl.h may both be included, rxvt_cv_header_sysioctl,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdio.h>
#include <sys/ioctl.h>
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif]], [[int a = ECHO;]])],[rxvt_cv_header_sysioctl=yes],[rxvt_cv_header_sysioctl=no])])

dnl# ELF systems may want to store paths for dynamic libraries.
dnl# Lets see if the compiler can accept "-Rpath" or "-Wl,-Rpath"
dnl# At least one version of SunOS wants "-R path" but it's not checked yet.
if test -n "$GCC"; then
  LDARG="-Wl,"
else
  LDARG=""
fi
changequote(, )dnl
R_TRANSLATE='s/-L\([^ ]*\)/-L\1 '$LDARG'-rpath '$LDARG'\1/g'
changequote([, ])dnl

ac_save_CFLAGS=$CFLAGS
ac_save_LIBS=$LIBS
CFLAGS="$CFLAGS $X_CFLAGS"
LIBS=`echo "$LIBS $X_LIBS $X_EXTRA_LIBS -lX11" | sed "$R_TRANSLATE"`
AC_CACHE_CHECK([for -rpath dynamic library path recording], rxvt_cv_rpath,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <X11/Xlib.h>
#include <stdlib.h>
main()
{
    exit(0);
    (void) XOpenDisplay("foobar");
}]])],[rxvt_cv_rpath=yes],[rxvt_cv_rpath=no],[dnl
  AC_MSG_WARN(You may need to check the LIBS line)])])
if test x$rxvt_cv_rpath != xyes; then
  changequote(, )dnl
  R_TRANSLATE='s/-L\([^ ]*\)/-L\1 '$LDARG'-R\1/g'
  changequote([, ])dnl
  LIBS=`echo "$ac_save_LIBS $X_LIBS $X_EXTRA_LIBS -lX11" | sed "$R_TRANSLATE"`
  AC_CACHE_CHECK([for -R dynamic library path recording], rxvt_cv_R,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <X11/Xlib.h>
#include <stdlib.h>
main()
{
    exit(0);
    (void) XOpenDisplay("foobar");
}]])],[rxvt_cv_R=yes],[rxvt_cv_R=no],[rxvt_cv_R=no])])
  if test x$rxvt_cv_R != xyes; then
    LIBS="$ac_save_LIBS $X_LIBS $X_EXTRA_LIBS -lX11"
  fi
fi

AC_CACHE_CHECK([for XPointer], rxvt_cv_xpointer,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <X11/Xlib.h>]], [[XPointer dummy;]])],[rxvt_cv_xpointer=yes],[rxvt_cv_xpointer=no])])
if test x$rxvt_cv_xpointer = xyes; then
  AC_DEFINE(HAVE_XPOINTER, 1, Define if you have XPointer typedef)
fi
LIBS=$ac_save_LIBS
CFLAGS=$ac_save_CFLAGS

AC_CACHE_CHECK([for XLIB_ILLEGAL_ACCESS], rxvt_xlib_illegal_access,
[AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([
#define XLIB_ILLEGAL_ACCESS
#include <X11/Xlib.h>
   ],[
      Display *dpy;
      dpy->xdefaults = (char *)0;
   ])],
   [rxvt_xlib_illegal_access=yes],[rxvt_xlib_illegal_access=no]
)])
if test x$rxvt_xlib_illegal_access = xyes; then
   AC_DEFINE(XLIB_ILLEGAL_ACCESS, 1, Define ONLY iff Xlib.h supports it)
fi

AC_C_CONST
AC_C_INLINE

dnl> AC_HEADER_STDC	 dnl# skip this test, Sun always fails anyhow.

dnl# --------------------------------------------------------------------------
dnl# CHECKING FOR MISSING TYPEDEFS
dnl# --------------------------------------------------------------------------
dnl# Missing typedefs and replacements
AC_TYPE_MODE_T
dnl> AC_CHECK_TYPE(umode_t, int)
dnl> AC_CHECK_TYPE(off_t, long)
AC_TYPE_PID_T
AC_TYPE_UID_T

AC_CHECK_SIZEOF(short, 2)
AC_CHECK_SIZEOF(int, 4)
dnl AC_CHECK_SIZEOF(long, 4)
AC_CHECK_SIZEOF(long long, 8)
AC_CHECK_SIZEOF(int *, 4)

dnl# see usage below
AC_DEFUN([RXVT_CHECK_SIZE],
  [AC_CACHE_CHECK([for $2], $1,
    [AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <stdint.h>]], [[$2 dummy;]])],[$1=yes],[dnl
if test "$ac_cv_sizeof_char" -ge $3; then
  $1="$4 char"
else
  if test "$ac_cv_sizeof_short" -ge $3; then
    $1="$4 short"
  else
    if test "$ac_cv_sizeof_int" -ge $3; then
      $1="$4 int"
    else
      if test "$ac_cv_sizeof_long" -ge $3; then
        $1="$4 long"
      else
        if test "$ac_cv_sizeof_long_long" -ge $3; then
          $1="$4 long long"
        else
          $1="$4 $5" # we _must_ have a (possibly wrong) default
        fi
      fi
    fi
  fi
fi])])]
if test x"$$1" != xyes; then
  $6="typedef $$1 $2;"
else
  if test x"$4" = x; then
    $6="/* typedef $5 $2; */"
  else
    $6="/* typedef $4 $5 $2; */"
  fi
fi dnl
)
dnl#
dnl# Look for types the system may know about anyway.
dnl#
RXVT_CHECK_SIZE(rxvt_cv_int16_t, int16_t, 2, , short, rxvt_int16_typedef)
AC_SUBST(rxvt_int16_typedef)
RXVT_CHECK_SIZE(rxvt_cv_uint16_t, uint16_t, 2, unsigned, short, rxvt_uint16_typedef)
AC_SUBST(rxvt_uint16_typedef)
RXVT_CHECK_SIZE(rxvt_cv_int32_t, int32_t, 4, , int, rxvt_int32_typedef)
AC_SUBST(rxvt_int32_typedef)
RXVT_CHECK_SIZE(rxvt_cv_uint32_t, uint32_t, 4, unsigned, int, rxvt_uint32_typedef)
AC_SUBST(rxvt_uint32_typedef)
dnl RXVT_CHECK_SIZE(rxvt_cv_int64_t, int64_t, 8, , long long, rxvt_int64_typedef)
dnl AC_SUBST(rxvt_int64_typedef)
dnl RXVT_CHECK_SIZE(rxvt_cv_uint64_t, uint64_t, 8, unsigned, long long, rxvt_uint64_typedef)
dnl AC_SUBST(rxvt_uint64_typedef)
dnl#
dnl# Now look for another we use
dnl#
if test "$ac_cv_sizeof_int_p" -eq 8; then
  rxvt_intp_define="#define intp_t int64_t"
  rxvt_u_intp_define="#define u_intp_t u_int64_t"
else
  if test "$ac_cv_sizeof_int_p" -eq 4; then
    rxvt_intp_define="#define intp_t int32_t"
    rxvt_u_intp_define="#define u_intp_t u_int32_t"
  else
    if test "$ac_cv_sizeof_int_p" -eq 2; then
      rxvt_intp_define="#define intp_t int16_t"
      rxvt_u_intp_define="#define u_intp_t u_int16_t"
    else
      rxvt_intp_define="#error set intp_t"
      rxvt_u_intp_define="#error set u_intp_t"
    fi
  fi
fi
AC_SUBST(rxvt_intp_define)
AC_SUBST(rxvt_u_intp_define)

dnl# --------------------------------------------------------------------------
dnl# CHECKING FOR LIBRARY FUNCTIONS
dnl# --------------------------------------------------------------------------
AC_TYPE_SIGNAL
dnl> AC_FUNC_VPRINTF

AC_CHECK_FUNCS( \
	unsetenv \
	setutent \
	on_exit \
)

UTMP_CHECK

dnl# --------------------------------------------------------------------------

dnl# find ttys/ttytab
AC_CACHE_CHECK(where ttys/ttytab is located, rxvt_cv_path_ttytab,
[for ttys_file in   dnl
  /etc/ttys /etc/ttytab;
do
  if test -f "$ttys_file" ; then
    rxvt_cv_path_ttytab=$ttys_file
    break
  fi
done
])
if test x$rxvt_cv_path_ttytab != x; then
  AC_DEFINE_UNQUOTED(TTYTAB_FILENAME, "$rxvt_cv_path_ttytab", Define location of ttys/ttytab)
fi

dnl# --------------------------------------------------------------------------
dnl# --------------------------------------------------------------------------

dnl# this is a really hack test for some basic Xlocale stuff
ac_save_LIBS=$LIBS
ac_save_CFLAGS=$CFLAGS
CFLAGS="$CFLAGS $X_CFLAGS"
LIBS="$LIBS $X_LIBS $X_EXTRA_LIBS -lX11"
if test x$rxvt_cv_rpath = xyes -o x$rxvt_cv_R = xyes; then
  LIBS=`echo $LIBS | sed "$R_TRANSLATE"`
fi
AC_CACHE_CHECK(for working Xlocale, rxvt_cv_func_xlocale,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <X11/Xlib.h>
#include <stdlib.h>
main() {
char *p;
if ((p = XSetLocaleModifiers("@im=none")) != NULL && *p)
exit (XSupportsLocale() ? 0 : 1);
else
exit (1);}]])],[dnl
  rxvt_cv_func_xlocale=yes],[rxvt_cv_func_xlocale=no],[dnl
  AC_MSG_WARN(Define NO_XLOCALE in config.h manually)])])
if test x$rxvt_cv_func_xlocale = xyes; then
  AC_DEFINE(HAVE_XLOCALE, 1, Define if Xlocale support works)
fi

AC_CACHE_CHECK(for working X setlocale, rxvt_cv_func_xsetlocale,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#define X_LOCALE 1
#include <X11/Xlocale.h>]], [[setlocale(LC_CTYPE, "");]])],[rxvt_cv_func_xsetlocale=yes],[rxvt_cv_func_xsetlocale=no])])
if test x$rxvt_cv_func_xsetlocale = xyes; then
  AC_DEFINE(HAVE_XSETLOCALE, 1, Define if setlocale (defined to Xsetlocale) works)
fi
LIBS=$ac_save_LIBS
CFLAGS=$ac_save_CFLAGS

AC_CACHE_CHECK(for working plain setlocale, rxvt_cv_func_setlocale,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <clocale>]], [[setlocale(LC_CTYPE, "");]])],[rxvt_cv_func_setlocale=yes],[rxvt_cv_func_setlocale=no])])
if test x$rxvt_cv_func_setlocale = xyes; then
  AC_DEFINE(HAVE_SETLOCALE, 1, Define if plain old setlocale works)
fi

AC_CACHE_CHECK(for working nl_langinfo, rxvt_cv_func_nl_langinfo,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <langinfo.h>]], [[nl_langinfo(CODESET);]])],[rxvt_cv_func_nl_langinfo=yes],[rxvt_cv_func_nl_langinfo=no])])
if test x$rxvt_cv_func_nl_langinfo = xyes; then
  AC_DEFINE(HAVE_NL_LANGINFO, 1, Define if nl_langinfo(CODESET) works)
fi

SCM_RIGHTS_CHECK

AC_CACHE_CHECK(for broken XIM callback, rxvt_broken_ximcb,
[AC_COMPILE_IFELSE([
#include <X11/Xlib.h>

void im_destroy_cb (XIC unused1, XPointer client_data, XPointer unused3);

void f() {
  XIMCallback cb;
  cb.callback = im_destroy_cb;
}
],rxvt_broken_ximcb=yes,rxvt_broken_ximcb=no)])

if test x$rxvt_broken_ximcb = xyes; then
   AC_DEFINE(XIMCB_PROTO_BROKEN, 1, Define if your XIMCallback specifies XIC as first type.)
fi

PTY_CHECK

TTY_GROUP_CHECK

dnl# --------------------------------------------------------------------------
dnl# now add and remove other stuff
dnl# --------------------------------------------------------------------------
if test x$support_xft = xyes; then
  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  if test $PKG_CONFIG != no && $PKG_CONFIG --exists xft; then
    LIBS="$LIBS `$PKG_CONFIG xft --libs`"
    CPPFLAGS="$CPPFLAGS `$PKG_CONFIG xft --cflags`"
  else
    AC_PATH_PROG(XFT_CONFIG, xft-config, no)
    if test $XFT_CONFIG != no; then
      LIBS="$LIBS `$XFT_CONFIG --libs`"
      CPPFLAGS="$CPPFLAGS `$XFT_CONFIG --cflags`"
    fi
  fi

  AC_CHECK_HEADERS(X11/Xft/Xft.h,,[support_xft=no])
  AC_CHECK_LIB(Xft,XftDrawString32,,[support_xft=no])

  if test x$support_xft = xyes; then
    AC_DEFINE(XFT, 1, Define to enable xft support)
  fi
fi
if test x$support_styles = xyes; then
  AC_DEFINE(ENABLE_STYLES, 1, Define if you want bold and italic support)
fi
if test x$support_iso14755 = xyes; then
  AC_DEFINE(ISO_14755, 1, Define if you want ISO 14755 extended support)
fi
if test x$support_8bitctrls = xyes; then
  AC_DEFINE(EIGHT_BIT_CONTROLS, 1, Define if you want 8 bit control sequences)
fi
if test x$support_fading = xyes; then
  AC_DEFINE(OFF_FOCUS_FADING, 1, Define if you want faded colors when focus is lost)
fi
if test x$support_tinting = xyes; then
  AC_DEFINE(TINTING, 1, Define if you want your background to be tinted)
fi
if test x$support_inheritpixmap = xyes; then
  AC_DEFINE(TRANSPARENT, 1, Define if you want your background to use the parent window background)
fi
if test x$support_keepscrolling = xno; then
  AC_DEFINE(NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING, 1, Define for continual scrolling when you keep the scrollbar button pressed)
fi
if test x$support_selectionscrolling = xyes; then
  AC_DEFINE(SELECTION_SCROLLING, 1, Define to allow scrolling when the selection moves to the top or bottom of the screen)
fi
if test x$support_frills = xyes; then
  AC_DEFINE(ENABLE_FRILLS, 1, Define if you want handling for rarely used but handy features)
fi
if test x$support_mousewheel = xyes; then
  AC_DEFINE(MOUSE_WHEEL, 1, Define to use wheel events (button4 and button5) to scroll)
fi
if test x$support_mouseslipwheel = xyes; then
  AC_DEFINE(MOUSE_SLIP_WHEELING, 1, Define to have CTRL cause wheel events to accelerate scrolling. Release CTRL to halt scrolling)
fi
if test x$support_xim = xyes -o x$multichar_set = xyes; then
  if test x$rxvt_cv_func_xlocale = xyes; then
    AC_DEFINE(USE_XIM, 1, Define if you want to have XIM (X Input Method) protocol support - required for multibyte characters input)
  fi
fi
if test x$support_xpm = xyes -o x$support_afterimage = xyes ; then
  AC_DEFINE(XPM_BACKGROUND, 1, Define if you want to have sexy-looking background pixmaps. Needs libXpm or libAfterImage)
fi

if test x$support_afterimage = xyes; then
  AC_DEFINE(HAVE_AFTERIMAGE, 1, Define if you want to use libAfterImage for image processing)
fi

if test x$support_afterstep = xyes; then
  AC_DEFINE(AFTERSTEP_INTEGRATION, 1, Define if you want to integrate with AfterStep window manager)
fi

if test x$support_scroll_rxvt = xyes; then
  AC_DEFINE(RXVT_SCROLLBAR, 1, Support Rxvt original style scrollbars)
  scrolltypes="rxvt"
fi
if test x$support_scroll_next = xyes; then
  AC_DEFINE(NEXT_SCROLLBAR, 1, Support NeXT style scrollbars)
  scrolltypes="$scrolltypes next"
fi
if test x$support_scroll_xterm = xyes; then
  AC_DEFINE(XTERM_SCROLLBAR, 1, Support Xterm style scrollbars)
  scrolltypes="$scrolltypes xterm"
fi
if test x$support_scroll_plain = xyes; then
  AC_DEFINE(PLAIN_SCROLLBAR, 1, Support plain style scrollbars)
  scrolltypes="$scrolltypes plain"
fi
if test x$support_pointer_blank = xyes; then
  AC_DEFINE(POINTER_BLANK, 1, Define if you want hide the pointer while typing)
fi
if test x$support_text_blink = xyes; then
  AC_DEFINE(TEXT_BLINK, 1, Define if you want blinking text support)
fi
if test x$support_unicode3 = xyes; then
  AC_DEFINE(UNICODE_3, 1, Define if you want to represent unicode characters outside plane 0)
fi
if test x$support_combining = xyes; then
  AC_DEFINE(ENABLE_COMBINING, 1, Define if you want to automatically compose combining characters)
fi
if test x$codesets = xall; then
  codesets=jp,jp-ext,kr,zh,zh-ext
fi
AC_DEFINE(ENCODING_EU, 1, Define if you want european extended codesets)
AC_DEFINE(ENCODING_VN, 1, Define if you want vietnamese codesets)
for codeset in `echo $codesets | tr "[a-z,\\-]" "[A-Z _]"`; do
  case "$codeset" in
    JP     ) AC_DEFINE(ENCODING_JP,     1, Define if you want japanese codesets) ;;
    JP_EXT ) AC_DEFINE(ENCODING_JP_EXT, 1, Define if you want extended japanese codesets) ;;
    KR     ) AC_DEFINE(ENCODING_KR,     1, Define if you want korean codesets) ;;
    ZH     ) AC_DEFINE(ENCODING_ZH,     1, Define if you want chinese codesets) ;;
    ZH_EXT ) AC_DEFINE(ENCODING_ZH_EXT, 1, Define if you want extended chinese codesets) ;;
  esac
done

IF_PERL=\#
if test x$support_perl = xyes; then
  AC_PATH_PROG(PERL, perl5)
  AC_PATH_PROG(PERL, perl)

  AC_MSG_CHECKING(for $PERL suitability)
  if $PERL -MExtUtils::Embed -e "use v5.8" >/dev/null 2>/dev/null; then

     save_CXXFLAGS="$CXXFLAGS"
     save_LIBS="$LIBS"
     CXXFLAGS="$CXXFLAGS `$PERL -MExtUtils::Embed -e ccopts`"
     LIBS="$LIBS `$PERL -MExtUtils::Embed -e ldopts`"
     AC_TRY_LINK([
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>
],[
     PerlInterpreter *perl = perl_alloc ();
],[rxvt_perl_link=yes],[rxvt_perl_link=no])
     CXXFLAGS="$save_CXXFLAGS"
     LIBS="$save_LIBS"

     if test x$rxvt_perl_link = xyes; then
        AC_MSG_RESULT(ok)
        AC_DEFINE(ENABLE_PERL, 1, Define if you can embed a perl interpreter)
        IF_PERL=
        PERL_O=rxvtperl.o
        PERLFLAGS="`$PERL -MExtUtils::Embed -e ccopts`"
        PERLLIB="`$PERL -MExtUtils::Embed -e ldopts`"
        PERLPRIVLIBEXP="`$PERL -MConfig -e 'print $Config{privlibexp}'`"
     else
        AC_MSG_ERROR([no, unable to link])
     fi
  else
     AC_MSG_ERROR([no working perl found, or perl not version >= 5.8])
  fi
fi
AC_SUBST(PERLLIB)
AC_SUBST(PERLFLAGS)
AC_SUBST(PERLPRIVLIBEXP)
AC_SUBST(PERL)
AC_SUBST(IF_PERL)
AC_SUBST(PERL_O)

if test x$support_perl = xyes; then
   support_frills=yes
fi

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(X_CFLAGS)

dnl# Attack the libs
if test x$rxvt_cv_rpath = xyes -o x$rxvt_cv_R = xyes; then
  LIBS=`echo $LIBS | sed "$R_TRANSLATE"`
  X_LIBS=`echo $X_LIBS | sed "$R_TRANSLATE"`
  X_EXTRA_LIBS=`echo $X_EXTRA_LIBS | sed "$R_TRANSLATE"`
  XPM_LIBS=`echo $XPM_LIBS | sed "$R_TRANSLATE"`
fi
AC_SUBST(LIBS)
AC_SUBST(X_LIBS)
AC_SUBST(X_EXTRA_LIBS)

AC_SUBST(LIBAFTERIMAGE_CFLAGS)
AC_SUBST(AFTERIMAGE_LIBS)

AC_ARG_PROGRAM

dnl# test for "sun" or "__sun__" before include sys_ioctl

dnl# revert HAVE_BLAH_H into a "#include <blah.h>"
AC_DEFUN([RXVT_DEFINE_TO_INCLUDE], dnl
[if test "$$3" = "$4" -o "$$5" = "$6"; then
  $1="#include <$2>"
else
  $1="/* #include <$2> */"
fi dnl
AC_SUBST($1)])

RXVT_DEFINE_TO_INCLUDE(include_stdint_h, stdint.h, ac_cv_header_stdint_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_stdarg_h, cstdarg, ac_cv_header_stdarg_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_stdlib_h, cstdlib, ac_cv_header_stdlib_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_unistd_h, unistd.h, ac_cv_header_unistd_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_string_h, cstring, ac_cv_header_string_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_fcntl_h, fcntl.h, ac_cv_header_fcntl_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_util_h, util.h, ac_cv_header_util_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_assert_h, assert.h, ac_cv_header_assert_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_sys_ioctl_h, sys/ioctl.h, rxvt_cv_header_sysioctl, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_sys_select_h, sys/select.h, ac_cv_header_sys_select_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_sys_strredir_h, sys/strredir.h, ac_cv_header_sys_strredir_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_sys_time_h, sys/time.h, ac_cv_header_sys_time_h, yes, notset, dontmatch)
RXVT_DEFINE_TO_INCLUDE(include_time_h, time.h, ac_cv_header_sys_time_h, no, ac_cv_header_time, yes)

AC_CONFIG_FILES([Makefile \
doc/Makefile \
src/Makefile \
src/rxvtlib.h \
])
AC_OUTPUT

echo "Configuration:

  Rxvt version:               $VERSION : $DATE
  Source code location:       $srcdir
  Install path:               ${prefix}/bin
  Compiler:                   $CXX
  Compiler flags:             $CXXFLAGS
  Linker:                     $LINKER"

if test x$support_xpm = xyes; then
  echo "  Xpm library:                $XPM_LIBS"
fi

if test x$support_afterimage = xyes; then
  echo "  AfterImage support:         yes"
fi

if test x$support_afterstep = xyes; then
  echo "  AfterStep integration:      yes"
fi

if test x$term != x; then
  echo "  set TERM to:                $term"
fi
if test x$terminfo != x; then
  echo "  set TERMINFO to:            $terminfo"
fi
echo "  default resource name:      $RESNAME"
echo "  resource class:             $RESCLASS"
if test x$RESFALLBACK != x; then
   echo "  resource class fallback:    $RESFALLBACK"
fi
echo
echo "  embedded perl:              $support_perl"
echo
if test x$support_xpm = xneedsmanualspecification; then
  echo ".----------------------------------------------------------------."
  echo ". WARNING: --enable-xpm-background was specified however the     ."
  echo ".          XPM includes files and libraries could not be found.  ."
  echo ".          XPM backgrounds are now being DISABLED!  If you want  ."
  echo ".          to use them you should rerun   configure   with the   ."
  echo ".          appropriate --with-xpm-includes=/path/to/xpm/includes ."
  echo ".          and --with-xpm-library=/path/to/xpm/library lines.    ."
  echo ".----------------------------------------------------------------."
fi
if test x$support_xim = xyes -a x$rxvt_cv_func_xlocale = xno; then
  echo ".----------------------------------------------------------------."
  echo ". WARNING: --enable-xim was specified however the locale support ."
  echo ".          functions could not be found.                         ."
  echo ".          XIM is now being DISABLED!                            ."
  echo ".----------------------------------------------------------------."
fi

echo "*** Optionally check src/feature.h for further, rarely used options ***"
echo

