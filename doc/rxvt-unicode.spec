Name: rxvt-unicode
Version: 4.2
Release: 1
Summary: rxvt-unicode is an unicode version of rxvt
License: GPL
Group: Terminals
Url: http://software.schmorp.de/
Buildroot: /var/tmp/%{name}-root

Source: http://dist.schmorp.de/%name/%name-%version.tar.bz2

# Automatically added by buildreq on Mon Oct 11 2004
BuildRequires: fontconfig-devel freetype2-devel gcc-c++ glib2 groff-base hostinfo libstdc++-devel pkgconfig xorg-x11-devel xorg-x11-libs xorg-x11-locales termutils-devel

%description
rxvt-unicode is a clone of the well known terminal emulator rxvt, modified to
store text in Unicode (either UCS-2 or UCS-4) and to use locale-correct input
and output. It also supports mixing multiple fonts at the same time, including
Xft fonts.

%prep
%setup -q

%build
%configure --enable-xft --enable-font-styles --enable-utmp --enable-wtmp \
  --enable-lastlog --enable-transparency --enable-tinting --enable-fading \
  --enable-menubar --enable-rxvt-scroll --enable-xterm-scroll \
  --enable-plain-scroll --enable-half-shadow --enable-xgetdefault \
  --enable-24bit --enable-keepscrolling --enable-selectionscrolling \
  --enable-mousewheel --enable-slipwheeling --enable-smart-resize \
  --enable-pointer-blank --enable-xpm-background --enable-next-scroll \
  --enable-xim --enable-linespace --with-save-lines=2000 --enable-resources
make

%install
make install DESTDIR=$RPM_BUILD_ROOT
tic -o $RPM_BUILD_ROOT/%_datadir/terminfo doc/etc/rxvt-unicode.terminfo 

%files
%doc README.FAQ INSTALL doc/README.menu doc/README.xvt doc/etc doc/menu
%{_bindir}/*
%{_mandir}/man1/*
%{_mandir}/man7/*
%{_datadir}/terminfo/r/*


%changelog
* Mon Dec 01 2004 vherva@babbage 4.2-1
- 4.2

* Mon Nov 27 2004 vherva@babbage 4.1-1
- 4.1
