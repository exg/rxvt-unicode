dnl#
dnl# Process this file with autoconf to produce a configure script.
dnl#

AC_INIT
AC_PREREQ(2.71)
AC_CONFIG_SRCDIR([src/feature.h])
AC_CONFIG_HEADERS(config.h:config.h.in)

dnl RXVT version
VERSION=`sed -n -e 's/^.* VERSION.*"\(.*\)"$/\1/p' ${srcdir}/src/version.h`
DATE=`sed -n -e 's/^.* DATE.*"\(.*\)"$/\1/p' ${srcdir}/src/version.h`
AC_SUBST(VERSION)dnl
AC_SUBST(DATE)dnl

echo ""
echo "configuring for rxvt $VERSION"
echo ""

orig_CXXFLAGS="$CXXFLAGS"

dnl# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL

AC_USE_SYSTEM_EXTENSIONS
AC_LANG(C++)

LINKER="$CXX"
dnl check whether we can link with gcc -lsupc++
if test x$GCC = xyes && test x$GXX = xyes; then
   dnl FreeBSD (at least up to 5.3) has a broken GCC, workaround it
   AC_MSG_CHECKING([for working libsupc++])
   save_CXX="$CXX"
   save_LIBS="$LIBS"
   CXX="$CC"
   LIBS="$LIBS -lsupc++"
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([struct test { }; void f() try { throw new test; } catch (...) { throw; }],[])],
      [AC_MSG_RESULT(ok)
       LINKER="$CC"],
      [
         AC_MSG_RESULT([no, making everything bigger and slower])
         LIBS="$save_LIBS"
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
    CXXFLAGS="-g -O3 -fno-rtti -fvisibility-inlines-hidden -fno-threadsafe-statics -fno-enforce-eh-specs"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])], [], [CXXFLAGS="-g -O3"])
  else
    CXXFLAGS="-O"
  fi
  CFLAGS="$CXXFLAGS"
fi

dnl# FreeBSD needs to link libxpg4
AC_SEARCH_LIBS(setlocale,     xpg4)

dnl# solaris needs to link libnsl and socket
AC_SEARCH_LIBS(gethostbyname, nsl)
AC_SEARCH_LIBS(socket,        socket)

support_frills=yes
support_inheritpixmap=yes
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
support_xim=yes
support_pixbuf=yes
support_startup_notification=yes
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

AC_ARG_ENABLE(everything,
  [AS_HELP_STRING([--enable-everything],
     [enable standard non-multichoice features
      NOTE: this option is order dependent])],
  [
    if test x$enableval = xno; then
       support_frills=no
       support_inheritpixmap=no
       support_fading=no
       support_keepscrolling=no
       support_selectionscrolling=no
       support_mousewheel=no
       support_mouseslipwheel=no
       support_text_blink=no
       support_pointer_blank=no
       support_scroll_rxvt=no
       support_scroll_next=no
       support_scroll_xterm=no
       support_xim=no
       support_pixbuf=no
       support_startup_notification=no
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
       support_xim=yes
       support_pixbuf=yes
       support_startup_notification=yes
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

support_assertions=no
AC_ARG_ENABLE(assert,
  [AS_HELP_STRING([--enable-assert],
     [enable assertions])],
  [if test x$enableval = xyes; then
    support_assertions=yes
  fi])
if test x$support_assertions = xno; then
  AC_DEFINE(NDEBUG, 1, Disable assertions (good for debugging))
fi

support_warnings=no
AC_ARG_ENABLE(warnings,
  [AS_HELP_STRING([--enable-warnings],
     [turn on g++ warnings])],
  [if test x$enableval = xyes; then
    support_warnings=yes
  fi])
if test x$GXX = xyes; then
  if test $support_warnings = yes; then
    save_CXXFLAGS="$CXXFLAGS"
    for warning in \
      -Wall \
      -Wno-parentheses \
      -Wno-reorder \
      -Wno-sign-compare \
      -Wno-unused-value \
      ; do
      CXXFLAGS="$CXXFLAGS $warning"
    done
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([],[])], [], [CXXFLAGS="$save_CXXFLAGS"])
  else
    CXXFLAGS="$CXXFLAGS -w"
  fi
fi

support_256_color=no
AC_ARG_ENABLE(256-color,
  [AS_HELP_STRING([--enable-256-color],
     [enable 256-color support])],
  [if test x$enableval = xyes; then
    support_256_color=yes
  fi])
if test x$support_256_color = xyes; then
  AC_DEFINE(USE_256_COLORS, 1, Define if you want 256-color support)
fi

AC_ARG_ENABLE(unicode3,
  [AS_HELP_STRING([--enable-unicode3],
     [use 21 instead of 16 bits to represent unicode characters])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_unicode3=$enableval
  fi])

AC_ARG_ENABLE(combining,
  [AS_HELP_STRING([--enable-combining],
     [enable composition of base and combining characters])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_combining=$enableval
  fi])

AC_ARG_ENABLE(xft,
  [AS_HELP_STRING([--enable-xft],
     [enable xft support on systems that have it])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_xft=$enableval
  fi])

AC_ARG_ENABLE(font-styles,
  [AS_HELP_STRING([--enable-font-styles],
     [enable bold and italic support])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_styles=$enableval
  fi])

AC_ARG_ENABLE(pixbuf,
  [AS_HELP_STRING([--enable-pixbuf],
     [enable integration with gdk-pixbuf for background images])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_pixbuf=$enableval
  fi])

AC_ARG_ENABLE(startup-notification,
  [AS_HELP_STRING([--enable-startup-notification],
     [enable freedesktop startup notification support])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_startup_notification=$enableval
  fi])

AC_ARG_ENABLE(transparency,
  [AS_HELP_STRING([--enable-transparency],
     [enable transparent backgrounds])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_inheritpixmap=$enableval
  fi])

AC_ARG_ENABLE(fading,
  [AS_HELP_STRING([--enable-fading],
     [enable colors fading when off focus])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_fading=$enableval
  fi])

AC_ARG_ENABLE(rxvt-scroll,
  [AS_HELP_STRING([--enable-rxvt-scroll],
     [enable rxvt style scrollbar])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_scroll_rxvt=$enableval
  fi])

AC_ARG_ENABLE(next-scroll,
  [AS_HELP_STRING([--enable-next-scroll],
     [enable NeXT style scrollbar])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_scroll_next=$enableval
  fi])

AC_ARG_ENABLE(xterm-scroll,
  [AS_HELP_STRING([--enable-xterm-scroll],
     [enable Xterm style scrollbar])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_scroll_xterm=$enableval
  fi])

AC_ARG_ENABLE(perl,
  [AS_HELP_STRING([--enable-perl],
     [enable embedded perl interpreter])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_perl=$enableval
  fi])

AC_ARG_WITH(codesets,
  [AS_HELP_STRING([--with-codesets=CS,...],
     [compile in additional codesets (jp,jp_ext,kr,zh,zh_ext,all)])],
  [codesets="$withval"])

AC_ARG_ENABLE(xim,
  [AS_HELP_STRING([--enable-xim],
     [XIM (X Input Method) protocol support])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_xim=$enableval
  fi])

AC_ARG_ENABLE(backspace-key,
  [AS_HELP_STRING([--disable-backspace-key],
     [disable handling of the backspace key])],
  [if test x$enableval = xno; then
    AC_DEFINE(NO_BACKSPACE_KEY, 1, Define if you don't want support for the backspace key)
  fi])

AC_ARG_ENABLE(delete-key,
  [AS_HELP_STRING([--disable-delete-key],
     [disable handling of the delete key])],
  [if test x$enableval = xno; then
    AC_DEFINE(NO_DELETE_KEY, 1, Define if you don't want support for the (non-keypad) delete key)
  fi])

support_resources=yes
AC_ARG_ENABLE(resources,
  [AS_HELP_STRING([--disable-resources],
     [disable all resource checking])],
  [if test x$enableval = xno; then
    support_resources=no
  fi])

AC_ARG_ENABLE(8bitctrls,
  [AS_HELP_STRING([--enable-8bitctrls],
     [enable 8 bit control sequences (not recommended)])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_8bitctrls=$enableval
  fi])

RESFALLBACK=Rxvt
AC_ARG_ENABLE(fallback,
  [AS_HELP_STRING([--enable-fallback@<:@=CLASS@:>@],
     [fall back on CLASS resources in addition to URxvt ones (default: Rxvt)])],
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
  [AS_HELP_STRING([--with-res-name=NAME],
     [set the default resource name (default: urxvt)])],
  [RESNAME="$withval"])
AC_DEFINE_UNQUOTED(RESNAME,"$RESNAME", [Default resource name])

RESCLASS=URxvt
AC_ARG_WITH(res-class,
  [AS_HELP_STRING([--with-res-class=CLASS],
     [set the resource class (default: URxvt)])],
  [RESCLASS="$withval"])
AC_DEFINE_UNQUOTED(RESCLASS,"$RESCLASS", [Resource class])

RXVTNAME=urxvt
AC_ARG_WITH(name,
  [AS_HELP_STRING([--with-name=NAME],
     [set the basename for the installed binaries (default: urxvt)])],
  [RXVTNAME="$withval"])
AC_DEFINE_UNQUOTED(RXVTNAME,"$RXVTNAME", [Binary base name])

AC_SUBST(RXVTNAME)

AC_ARG_ENABLE(swapscreen,
  [AS_HELP_STRING([--disable-swapscreen],
     [disable swap screen support])],
  [if test x$enableval = xno; then
    AC_DEFINE(NO_SECONDARY_SCREEN, 1, Disable the secondary screen. Many programs use the secondary screen as their workplace)
  fi])

AC_ARG_ENABLE(iso14755,
  [AS_HELP_STRING([--enable-iso14755],
     [enable support for extended ISO 14755 modes])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_iso14755=$enableval
  fi])

AC_ARG_ENABLE(frills,
  [AS_HELP_STRING([--enable-frills],
     [enable support for rarely used features])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_frills=$enableval
  fi])

AC_ARG_ENABLE(keepscrolling,
  [AS_HELP_STRING([--enable-keepscrolling],
     [enable continual scrolling on scrollbar arrow press])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_keepscrolling=$enableval
  fi])

AC_ARG_ENABLE(selectionscrolling,
  [AS_HELP_STRING([--enable-selectionscrolling],
     [enable scrolling during selections])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_selectionscrolling=$enableval
  fi])

AC_ARG_ENABLE(mousewheel,
  [AS_HELP_STRING([--enable-mousewheel],
     [enable scrolling via mouse wheel or buttons 4 & 5])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_mousewheel=$enableval
  fi])

AC_ARG_ENABLE(slipwheeling,
  [AS_HELP_STRING([--enable-slipwheeling],
     [enable slip wheel scrolling (requires --enable-mousewheel)])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_mouseslipwheel=$enableval
  fi])

AC_ARG_ENABLE(smart-resize,
  [AS_HELP_STRING([--enable-smart-resize],
     [enable smart growth/shrink behaviour])],
  [if test x$enableval = xyes; then
    AC_DEFINE(SMART_RESIZE, 1, Define to use "smart" resize behavior)
  fi])

AC_ARG_ENABLE(text-blink,
  [AS_HELP_STRING([--enable-text-blink],
     [enable blinking text])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_text_blink=$enableval
  fi])

AC_ARG_ENABLE(pointer-blank,
  [AS_HELP_STRING([--enable-pointer-blank],
     [enable pointer blanking when typing or inactive])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_pointer_blank=$enableval
  fi])

AC_ARG_WITH(term,
  [AS_HELP_STRING([--with-term=VALUE],
     [set the value of the TERM environment variable (default: rxvt)])],
  [if test x$withval != x; then
    AC_DEFINE_UNQUOTED(TERMENV, "$withval",Set TERM to the value given by configure) term="$withval"
  fi])

AC_ARG_WITH(terminfo,
  [AS_HELP_STRING([--with-terminfo=VALUE],
     [set the value of the TERMINFO environment variable])],
  [if test x$withval != x; then
    AC_DEFINE_UNQUOTED(RXVT_TERMINFO, "$withval", Set TERMINFO value to the value given by configure) terminfo="$withval"
  fi])

if test x$support_resources = xno; then
  if test x$support_frills = xyes || test x$support_perl = xyes; then
    AC_MSG_ERROR([--disable-resources requires --disable-frills --disable-perl])
  fi

  AC_DEFINE(NO_RESOURCES, 1, Define if you don't want any resources read)
fi

dnl# --------------------------------------------------------------------------

LIBEV_M4_AVOID_LIBRT=1
m4_include([libev/libev.m4])

dnl# --------------------------------------------------------------------------

AC_PATH_PROG(TIC, tic, :)

AC_PATH_XTRA

AC_PATH_TOOL(PKG_CONFIG, pkg-config, no)

RXVT_CHECK_MODULES([PTYTTY], [libptytty], [:], [
  AC_MSG_ERROR([unable to find libptytty])
])

AC_SUBST(PTYTTY_CFLAGS)
AC_SUBST(PTYTTY_LIBS)

image_lib=none

PIXBUF_CFLAGS=
PIXBUF_LIBS=

if test x$support_pixbuf = xyes; then
  RXVT_CHECK_MODULES([PIXBUF], [gdk-pixbuf-2.0], [
    image_lib=gdk-pixbuf
    AC_DEFINE(HAVE_PIXBUF, 1, Define if you want to use gdk-pixbuf for image processing)
  ], [:])
fi

AC_SUBST(PIXBUF_CFLAGS)
AC_SUBST(PIXBUF_LIBS)

STARTUP_NOTIFICATION_CFLAGS=
STARTUP_NOTIFICATION_LIBS=

if test x$support_startup_notification = xyes; then
  RXVT_CHECK_MODULES([STARTUP_NOTIFICATION], [libstartup-notification-1.0], [
    AC_DEFINE(HAVE_STARTUP_NOTIFICATION, 1, Define if freedesktop startup notifications should be supported)
  ], [:])
fi

AC_SUBST(STARTUP_NOTIFICATION_CFLAGS)
AC_SUBST(STARTUP_NOTIFICATION_LIBS)

if test x$support_frills = xyes; then
  RXVT_CHECK_MODULES([XMU], [xmu], [
    X_LIBS="$XMU_LIBS $X_LIBS"
    CPPFLAGS="$CPPFLAGS $XMU_CFLAGS"
    AC_DEFINE(HAVE_XMU, 1, Define to enable Xmu support)
  ], [:])
fi

dnl# --------------------------------------------------------------------------
dnl# CHECKING FOR HEADER FILES
dnl# --------------------------------------------------------------------------
AC_CHECK_HEADERS( \
	sys/byteorder.h \
	sys/ioctl.h \
	sys/sockio.h \
	sys/strredir.h \
	stdint.h \
	wchar.h \
)

AC_CACHE_CHECK([for XLIB_ILLEGAL_ACCESS], rxvt_cv_xlib_illegal_access,
[AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([
#define XLIB_ILLEGAL_ACCESS
#include <X11/Xlib.h>
   ],[
      Display *dpy;
      dpy->xdefaults = (char *)0;
   ])],
   [rxvt_cv_xlib_illegal_access=yes],[rxvt_cv_xlib_illegal_access=no]
)])
if test x$rxvt_cv_xlib_illegal_access = xyes; then
   AC_DEFINE(XLIB_ILLEGAL_ACCESS, 1, Define ONLY iff Xlib.h supports it)
fi

dnl# --------------------------------------------------------------------------
dnl# CHECKING FOR MISSING TYPEDEFS
dnl# --------------------------------------------------------------------------
dnl# Missing typedefs and replacements
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_UID_T
AC_TYPE_INT16_T
AC_TYPE_UINT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT32_T

dnl# --------------------------------------------------------------------------
dnl# CHECKING FOR LIBRARY FUNCTIONS
dnl# --------------------------------------------------------------------------
AC_CHECK_FUNCS(unsetenv)

dnl# --------------------------------------------------------------------------

dnl# --------------------------------------------------------------------------
dnl# --------------------------------------------------------------------------

dnl# this is a really hack test for some basic Xlocale stuff
save_LIBS=$LIBS
save_CXXFLAGS=$CXXFLAGS
CXXFLAGS="$CXXFLAGS $X_CFLAGS"
LIBS="$LIBS $X_LIBS $X_EXTRA_LIBS -lX11"
if test x$support_xim = xyes; then
  AC_CACHE_CHECK(for working Xlocale, rxvt_cv_func_xlocale,
  [AC_RUN_IFELSE([AC_LANG_SOURCE([[
  #include <X11/Xlib.h>
  #include <stdlib.h>
  int main() {
    char *p;
    if ((p = XSetLocaleModifiers("@im=none")) != NULL && *p)
      exit (XSupportsLocale() ? 0 : 1);
    else
      exit (1);
  }
  ]])],[rxvt_cv_func_xlocale=yes],[rxvt_cv_func_xlocale=no],[:])])
  if test x$rxvt_cv_func_xlocale = xyes; then
    AC_DEFINE(USE_XIM, 1, Define if you want to have XIM (X Input Method) protocol support - required for multibyte characters input)
    AC_CACHE_CHECK(for broken XIM callback, rxvt_cv_broken_ximcb,
    [AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
    #include <X11/Xlib.h>

    void im_destroy_cb (XIC unused1, XPointer client_data, XPointer unused3);

    void f() {
      XIMCallback cb;
      cb.callback = im_destroy_cb;
    }
    ]])],rxvt_cv_broken_ximcb=yes,rxvt_cv_broken_ximcb=no)])

    if test x$rxvt_cv_broken_ximcb = xyes; then
       AC_DEFINE(XIMCB_PROTO_BROKEN, 1, Define if your XIMCallback specifies XIC as first type.)
    fi
  fi
fi

AC_CACHE_CHECK(for working X setlocale, rxvt_cv_func_xsetlocale,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#define X_LOCALE 1
#include <X11/Xlocale.h>]], [[setlocale(LC_CTYPE, "");]])],[rxvt_cv_func_xsetlocale=yes],[rxvt_cv_func_xsetlocale=no])])
if test x$rxvt_cv_func_xsetlocale = xyes; then
  AC_DEFINE(HAVE_XSETLOCALE, 1, Define if setlocale (defined to Xsetlocale) works)
fi
LIBS=$save_LIBS
CXXFLAGS=$save_CXXFLAGS

AC_CACHE_CHECK(for working nl_langinfo, rxvt_cv_func_nl_langinfo,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <langinfo.h>]], [[nl_langinfo(CODESET);]])],[rxvt_cv_func_nl_langinfo=yes],[rxvt_cv_func_nl_langinfo=no])])
if test x$rxvt_cv_func_nl_langinfo = xyes; then
  AC_DEFINE(HAVE_NL_LANGINFO, 1, Define if nl_langinfo(CODESET) works)
fi

dnl# --------------------------------------------------------------------------
dnl# now add and remove other stuff
dnl# --------------------------------------------------------------------------
support_image=no
if test x$support_inheritpixmap = xyes || test x$support_pixbuf = xyes; then
  support_image=yes
fi
if test x$support_xft = xyes || test x$support_image = xyes; then
  rxvt_have_xrender=no
  RXVT_CHECK_MODULES([XRENDER], [xrender], [
    X_LIBS="$XRENDER_LIBS $X_LIBS"
    CPPFLAGS="$CPPFLAGS $XRENDER_CFLAGS"
    rxvt_have_xrender=yes

    save_LIBS="$LIBS"
    LIBS="$LIBS $X_LIBS"
    AC_CHECK_HEADER(X11/extensions/Xrender.h,,[rxvt_have_xrender=no])
    AC_CHECK_FUNC(XRenderFindStandardFormat,,[rxvt_have_xrender=no])
    LIBS="$save_LIBS"
  ], [:])
fi

if test x$support_xft = xyes && test x$rxvt_have_xrender = xyes; then
  RXVT_CHECK_MODULES([XFT], [fontconfig xft], [
    X_LIBS="$XFT_LIBS $X_LIBS"
    CPPFLAGS="$CPPFLAGS $XFT_CFLAGS"
  ], [
    AC_PATH_PROG(XFT_CONFIG, xft-config, no)
    if test $XFT_CONFIG != no; then
      X_LIBS="`$XFT_CONFIG --libs` $X_LIBS"
      CPPFLAGS="$CPPFLAGS `$XFT_CONFIG --cflags`"
    fi
  ])

  save_LIBS="$LIBS"
  LIBS="$LIBS $X_LIBS"
  AC_CHECK_HEADERS(X11/Xft/Xft.h fontconfig/fontconfig.h,,[support_xft=no])
  AC_CHECK_FUNCS(XftDrawString32 FcPatternGet,,[support_xft=no])
  LIBS="$save_LIBS"

  if test x$support_xft = xyes; then
    AC_DEFINE(XFT, 1, Define to enable xft support)
  fi
fi

if test x$support_image = xyes && test x$rxvt_have_xrender = xyes; then
  AC_MSG_CHECKING(for Render >= 0.11)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <X11/extensions/Xrender.h>
#if RENDER_MAJOR == 0 && RENDER_MINOR <= 10
error
#endif
]])],[],[support_image=no])
  if test x$support_image = xyes; then
    AC_MSG_RESULT(ok)
    LIBS="$LIBS -lm"
    AC_DEFINE(XRENDER, 1, Define to enable xrender support)

    if test x$support_inheritpixmap = xyes; then
      AC_DEFINE(ENABLE_TRANSPARENCY, 1, Define if you want your background to use the parent window background)
    fi
  else
    AC_MSG_RESULT(no)
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

scrolltypes=plain
AC_DEFINE(PLAIN_SCROLLBAR, 1, Support plain style scrollbars)

if test x$support_scroll_rxvt = xyes; then
  AC_DEFINE(RXVT_SCROLLBAR, 1, Support Rxvt original style scrollbars)
  scrolltypes="$scrolltypes rxvt"
fi
if test x$support_scroll_next = xyes; then
  AC_DEFINE(NEXT_SCROLLBAR, 1, Support NeXT style scrollbars)
  scrolltypes="$scrolltypes next"
fi
if test x$support_scroll_xterm = xyes; then
  AC_DEFINE(XTERM_SCROLLBAR, 1, Support Xterm style scrollbars)
  scrolltypes="$scrolltypes xterm"
fi
if test x$support_pointer_blank = xyes; then
  AC_DEFINE(POINTER_BLANK, 1, Define if you want to hide the pointer while typing)
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
  AC_PATH_PROG(PERL, perl5, perl)

  RXVT_CHECK_MODULES([XEXT], [xext], [:], [
    AC_MSG_ERROR([unable to find xext, required for perl])
  ])

  X_LIBS="$XEXT_LIBS $X_LIBS"
  CPPFLAGS="$CPPFLAGS $XEXT_CFLAGS"

  AC_MSG_CHECKING(for $PERL suitability)
  if $PERL -MExtUtils::Embed -e "use v5.8" >/dev/null 2>/dev/null; then

     save_CXXFLAGS="$CXXFLAGS"
     save_LIBS="$LIBS"
     CXXFLAGS="$CXXFLAGS `$PERL -MExtUtils::Embed -e ccopts`"
     LIBS="$LIBS `$PERL -MExtUtils::Embed -e ldopts`"
     AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>
]], [[
     PerlInterpreter *perl = perl_alloc ();
]])],[rxvt_perl_link=yes],[rxvt_perl_link=no])
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


AC_CONFIG_FILES([Makefile \
doc/Makefile \
src/Makefile \
])
AC_OUTPUT

echo "Configuration:

  Rxvt version:               $VERSION : $DATE
  Source code location:       $srcdir
  Install path:               ${prefix}/bin
  Compiler:                   $CXX
  Compiler flags:             $CXXFLAGS
  Linker:                     $LINKER"

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
echo "  image library:              $image_lib"
echo
if test x$support_xim = xyes -a x$rxvt_cv_func_xlocale = xno; then
  echo ".----------------------------------------------------------------."
  echo ". WARNING: --enable-xim was specified however the locale support ."
  echo ".          functions could not be found.                         ."
  echo ".          XIM is now being DISABLED!                            ."
  echo ".----------------------------------------------------------------."
fi

echo "*** Optionally check src/feature.h for further, rarely used options ***"
echo

