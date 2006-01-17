dnl# -*- sh -*-
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
AC_PROG_AWK

AC_LANG(C)

dnl# --------------------------------------------------------------------------
dnl# Supply default CFLAGS, if not specified by `CFLAGS=flags ./configure'
dnl#
if test -z "$orig_CXXFLAGS"; then
  if test x$GCC = xyes && test "x$GXX" = xyes; then
    CXXFLAGS="-g -O3"
  else
    CXXFLAGS="-O"
  fi
  CFLAGS="$CXXFLAGS"
fi

LINKER="$CXX"
if test x$GCC = xyes && test x$GXX = xyes; then
   AC_CHECK_LIB(supc++, main, [
      LINKER="$CC"
      LIBS="$LIBS -lsupc++"
   ])
fi
AC_SUBST(LINKER,[$LINKER])

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

AC_LANG(C++)

dnl# system hacks
AC_AIX
AC_ISC_POSIX

dnl# FreeBSD needs to link libxpg4
AC_CHECK_FUNC(setlocale,     [], [AC_CHECK_LIB(xpg4,   setlocale,     [LIBS="$LIBS -lxpg4"])])

dnl# solaris needs to link libnsl and socket
AC_CHECK_FUNC(gethostbyname, [], [AC_CHECK_LIB(nsl,    gethostbyname, [LIBS="$LIBS -lnsl"])])
AC_CHECK_FUNC(socket,        [], [AC_CHECK_LIB(socket, socket,        [LIBS="$LIBS -lsocket"])])

dnl# more solaris
AC_CHECK_FUNC(sched_yield,   [], [AC_CHECK_LIB(rt,     sched_yield,   [LIBS="$LIBS -lrt"])])

dnl# sched_yield to improve terminal efficiency
AC_MSG_CHECKING(for sched_yield support)
AC_TRY_LINK([#include <sched.h>],[sched_yield ()],
            [AC_MSG_RESULT(yes)
             AC_DEFINE(HAVE_SCHED_YIELD, 1, Define if you have sched-h and sched_yield)],
            [AC_MSG_RESULT(no)])

dnl AC_ENABLE_SHARED(no)dnl#	libtool
dnl AC_ENABLE_STATIC(yes)dnl#	libtool
dnl AC_PROG_LIBTOOL()dnl#		libtool

MALLOC_TYPE=S

support_frills=yes
support_inheritpixmap=yes
support_tinting=yes
support_fading=yes
support_keepscrolling=yes
support_selectionscrolling=yes
support_mousewheel=yes
support_mouseslipwheel=yes
support_utmp=yes
support_wtmp=yes
support_lastlog=yes
support_text_blink=yes
support_pointer_blank=yes
support_scroll_rxvt=yes
support_scroll_next=yes
support_scroll_xterm=yes
support_scroll_plain=yes
support_xim=yes
support_xpm=yes
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
       support_24bit=no
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
       support_24bit=yes
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

AC_ARG_ENABLE(utmp,
  [  --enable-utmp           enable utmp (utmpx) support],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_utmp=$enableval
  fi])

AC_ARG_ENABLE(wtmp,
  [  --enable-wtmp           enable wtmp (wtmpx) support (requires --enable-utmp)],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_wtmp=$enableval
  fi])

AC_ARG_ENABLE(lastlog,
  [  --enable-lastlog        enable lastlog support (requires --enable-utmp)],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_lastlog=$enableval
  fi])

AC_ARG_ENABLE(xpm-background,
  [  --enable-xpm-background enable XPM background pixmaps],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_xpm=$enableval
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

dnl AC_ARG_ENABLE(half-shadow,
dnl   [  --enable-half-shadow    use half width/height shadow on rxvt scrollbar],
dnl   [if test x$enableval = xyes; then
dnl     AC_DEFINE(HALFSHADOW, 1, Define if you want the depth of scrollbars and menus to be less)
dnl   fi])

AC_ARG_WITH(codesets,
  [  --with-codesets=NAME,.. compile in additional codesets (jp,jp_ext,kr,zh,zh_ext,all)],
  [codesets="$withval"])

AC_ARG_ENABLE(xim,
  [  --enable-xim            XIM (X Input Method) protocol support],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_xim=$enableval
  fi])

AC_ARG_ENABLE(ttygid,
  [  --enable-ttygid         enable tty setting to group named "tty"],
  [if test x$enableval = xyes; then
    AC_DEFINE(TTY_GID_SUPPORT, 1, Define to change gid of ttys to group tty)
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

AC_ARG_ENABLE(24bit,
  [  --enable-24bit          enable support for using 24bit visuals if available],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_24bit=$enableval
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

AC_ARG_ENABLE(dmalloc,
  [  --enable-dmalloc        enable Gray Watson's malloc - for debugging use],
  [if test x$enableval = xyes; then
    MALLOC_TYPE=G
    DEBUG=-DDEBUG_MALLOC
    DLIB="-L/usr/local/lib -ldmalloc"
    DINCLUDE=-I/usr/local/include
  fi])

AC_ARG_ENABLE(dlmalloc,
  [  --enable-dlmalloc       enable Doug Lea's malloc - for production use
                     NOTE: enable only one malloc package],
  [if test x$enableval = xyes; then
    MALLOC_TYPE=D
    DEBUG=
    DLIB="-L/usr/local/lib -ldlmalloc"
    DINCLUDE=
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

AC_DEFINE(PROTOTYPES, 1, Define if you need function prototypes)

AC_PATH_PROG(MV, mv, mv)
AC_PATH_PROG(RM, rm, rm)
AC_PATH_PROG(CP, cp, cp)
AC_PATH_PROG(LN, ln, ln)
AC_PATH_PROG(SED, sed, sed)
AC_PATH_PROG(ECHO, echo, echo)
AC_PATH_PROG(CMP, cmp, cmp)
AC_PATH_PROG(TBL, tbl)
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

if test x$support_xpm = xyes; then
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
	grp.h \
	libc.h \
	lastlog.h \
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
        sys/stropts.h \
	sys/time.h \
	utmp.h \
	utmpx.h \
        stdint.h \
        pty.h \
        util.h \
        libutil.h \
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
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
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
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
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
AC_DEFUN(RXVT_CHECK_SIZE,
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
	revoke \
	unsetenv \
	setutent \
	seteuid \
	setreuid \
	_getpty \
	getpt \
	posix_openpt \
	grantpt \
	unlockpt \
	isastream \
	on_exit \
	nanosleep \
	updwtmp \
	updwtmpx \
	ttyslot \
)

dnl# --------------------------------------------------------------------------
dnl# DO ALL UTMP AND WTMP CHECKING
dnl# --------------------------------------------------------------------------
dnl# check for host field in utmp structure

dnl# --------------------------------------------
AC_CHECK_HEADER(utmp.h,
[AC_CACHE_CHECK([for struct utmp], rxvt_cv_struct_utmp,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <stdint.h>
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

AC_CACHE_CHECK(for unix-compliant filehandle passing ability, rxvt_can_pass_fds,
[AC_TRY_LINK([
#include <sys/types.h>
#include <sys/socket.h>
],[
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
],[rxvt_can_pass_fds=yes],[rxvt_can_pass_fds=no])])
if test x$rxvt_can_pass_fds = xyes; then
   AC_DEFINE(HAVE_UNIX_FDPASS, 1, Define if sys/socket.h defines the necessary macros/functions for file handle passing)
else
   AC_MSG_ERROR([urxvt requires unix-compliant filehandle passing ability])
fi

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

if test x$ac_cv_func_getpt = xno -a x$ac_cv_func_posix_openpt = xno; then
  AC_CHECK_FUNC(openpty, [], [AC_CHECK_LIB(util, openpty, [LIBS="$LIBS -lutil"])])
fi

dnl# if we don't guess right then it's up to the user
AC_CACHE_CHECK(for pty/tty type, rxvt_cv_ptys,
[if test x$ac_cv_func_openpty = xyes -o x$ac_cv_lib_util_openpty = xyes; then
    rxvt_cv_ptys=OPENPTY
 else if test x$ac_cv_func_getpt = xyes; then
    rxvt_cv_ptys=GLIBC
 else if test x$ac_cv_func_posix_openpt = xyes; then
    rxvt_cv_ptys=POSIX
 else if test x$ac_cv_func__getpty = xyes; then
    rxvt_cv_ptys=SGI4
 else if test -c /dev/ttyp20; then
    rxvt_cv_ptys=SCO
 else if test -c /dev/ptym/clone; then
    rxvt_cv_ptys=HPUX
 else if test -c /dev/ptc -a -c /dev/pts; then
    rxvt_cv_ptys=PTC
 else if test -c /dev/ptc -a -d /dev/pts; then
    rxvt_cv_ptys=PTC
 else if test -c /dev/ptmx -a -c /dev/pts/0; then
    rxvt_cv_ptys=STREAMS
 else if test x$ac_cv_func_grantpt = xyes && test x$ac_cv_func_unlockpt = xyes; then
dnl# catch CYGWIN
    rxvt_cv_ptys=STREAMS
 else
    rxvt_cv_ptys=BSD
fi
fi
fi
fi
fi
fi
fi
fi
fi
fi
])

if test x$rxvt_cv_ptys = xGLIBC; then
  AC_DEFINE(PTYS_ARE_GETPT, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xPOSIX; then
  AC_DEFINE(PTYS_ARE_POSIX, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xOPENPTY; then
  AC_DEFINE(PTYS_ARE_OPENPTY, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xSCO; then
  AC_DEFINE(PTYS_ARE_NUMERIC, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xSTREAMS; then
  AC_DEFINE(PTYS_ARE_PTMX, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xPTC; then
  AC_DEFINE(PTYS_ARE_PTC, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xSGI4; then
  AC_DEFINE(PTYS_ARE__GETPTY, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xHPUX; then
  AC_DEFINE(PTYS_ARE_CLONE, 1, Define for this pty type)
fi
if test x$rxvt_cv_ptys = xBSD -o x$rxvt_cv_ptys = xHPUX -o x$rxvt_cv_ptys = xPOSIX; then
  AC_DEFINE(PTYS_ARE_SEARCHED, 1, Define for this pty type)
fi


AS_MESSAGE(checking for pty ranges...)
ptys=`echo /dev/pty??`
pch1=`echo $ptys | tr ' ' '\012' | sed -e 's/^.*\(.\).$/\1/g' | sort -u | tr -d '\012'`
pch2=`echo $ptys | tr ' ' '\012' | sed -e 's/^.*\(.\)$/\1/g' | sort -u | tr -d '\012'`
if test x$pch1 != x; then
  AC_DEFINE_UNQUOTED(PTYCHAR1, "$pch1", Define for first char in devptyXX)
fi
if test x$pch2 != x; then
  AC_DEFINE_UNQUOTED(PTYCHAR2, "$pch2", Define for second char in devptyXX)
fi

dnl# --------------------------------------------------------------------------
dnl# now add and remove other stuff
dnl# --------------------------------------------------------------------------
if test x$support_xft = xyes; then
  LIBS="$LIBS `pkg-config xft --libs`"
  CPPFLAGS="$CPPFLAGS `pkg-config xft --cflags`"

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
if test x$support_24bit = xyes; then
  AC_DEFINE(PREFER_24BIT, 1, Define to use a 24 bit visual if the screen has 24 bit mode, even if the default is 8 bit)
fi
if test x$support_mousewheel = xyes; then
  AC_DEFINE(MOUSE_WHEEL, 1, Define to use wheel events (button4 and button5) to scroll)
fi
if test x$support_mouseslipwheel = xyes; then
  AC_DEFINE(MOUSE_SLIP_WHEELING, 1, Define to have CTRL cause wheel events to accelerate scrolling. Release CTRL to halt scrolling)
fi
if test x$support_utmp = xyes; then
  AC_DEFINE(UTMP_SUPPORT, 1, Define if you want to have utmp/utmpx support)
fi
if test x$support_wtmp = xyes; then
  AC_DEFINE(WTMP_SUPPORT, 1, Define if you want to have wtmp support when utmp/utmpx is enabled)
fi
if test x$support_lastlog = xyes; then
  AC_DEFINE(LASTLOG_SUPPORT, 1, Define if you want to have lastlog support when utmp/utmpx is enabled)
fi
if test x$support_xim = xyes -o x$multichar_set = xyes; then
  if test x$rxvt_cv_func_xlocale = xyes; then
    AC_DEFINE(USE_XIM, 1, Define if you want to have XIM (X Input Method) protocol support - required for multibyte characters input)
  fi
fi
if test x$support_xpm = xyes; then
  AC_DEFINE(XPM_BACKGROUND, 1, Define if you want to have sexy-looking background pixmaps. Needs libXpm)
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

dnl> AC_CHECK_FUNCS(gettimeofday putenv select socket)

CFLAGS=${CFLAGS--O}
LDFLAGS=${LDFLAGS--O}

CPPFLAGS="$CPPFLAGS"
AC_SUBST(DEBUG)
AC_SUBST(DINCLUDE)
AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(X_CFLAGS)

dnl# Attack the libs
if test x$rxvt_cv_rpath = xyes -o x$rxvt_cv_R = xyes; then
  DLIB=`echo $DLIB | sed "$R_TRANSLATE"`
  LIBS=`echo $LIBS | sed "$R_TRANSLATE"`
  X_LIBS=`echo $X_LIBS | sed "$R_TRANSLATE"`
  X_EXTRA_LIBS=`echo $X_EXTRA_LIBS | sed "$R_TRANSLATE"`
  XPM_LIBS=`echo $XPM_LIBS | sed "$R_TRANSLATE"`
fi
AC_SUBST(DLIB)
AC_SUBST(LIBS)
AC_SUBST(X_LIBS)
AC_SUBST(X_EXTRA_LIBS)

AC_ARG_PROGRAM

dnl# test for "sun" or "__sun__" before include sys_ioctl

dnl# revert HAVE_BLAH_H into a "#include <blah.h>"
AC_DEFUN(RXVT_DEFINE_TO_INCLUDE, dnl
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
  Compiler flags:             $CFLAGS
  Linker:                     $LINKER"

if test "$MALLOC_TYPE" = S; then
  echo "  malloc support:             system default"
fi
if test "$MALLOC_TYPE" = G; then
  echo "  malloc support:             Gray Watson's dmalloc"
fi
if test "$MALLOC_TYPE" = D; then
  echo "  malloc support:             Doug Lea's malloc"
fi

if test x$support_xpm = xyes; then
  echo "  Xpm library:                $XPM_LIBS"
fi

echo "
The following are set in config.h
"
echo "  pty/tty type:               "$rxvt_cv_ptys
if test x$support_utmp != xyes; then
  echo "  utmp support:               disabled"
  echo
else
  echo "  utmp support:               enabled
  utmp file:                  $rxvt_cv_path_utmp
  utmpx file:                 $rxvt_cv_path_utmpx
  wtmp file:                  $rxvt_cv_path_wtmp
  wtmpx file:                 $rxvt_cv_path_wtmpx
  lastlog file:               $rxvt_cv_path_lastlog
  ttys/ttytab file:           $rxvt_cv_path_ttytab
  embedded perl:              $support_perl"
  echo
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
if test x$rxvt_cv_ptys = xUNKNOWN; then
  echo ".----------------------------------------------------------------."
  echo ". WARNING: could not determine pty/tty type.  Do not build until ."
  echo ".          the appropriate PTYS_ARE_* is defined in config.h     ."
  echo ".----------------------------------------------------------------."
fi
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

echo "  *** Optionally check src/feature.h for further options ***"
echo

echo ".----------------------------------------------------------------."
echo ". NOTE: some features are disabled by default, try               ."
echo ". configure --help to get an idea of the optional features,      ."
echo ". or read ./README.configure                                     ."
echo ".                                                                ."
echo ". The script ./reconf gives a reasonable baseline, try it if     ."
echo ". you do not want to read documentation and still want some      ."
echo ". useful features.                                               ."
echo ".----------------------------------------------------------------."
echo

