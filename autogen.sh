#! /bin/sh

if ! [ -e libev/ev++.h ]; then
   cat <<EOF
**
** libev/ directory is missing
**
** you need a checkout of libev (http://software.schmorp.de/pkg/libev)
** in the top-level build directory.
**
EOF
   exit 1
fi

if autoheader && autoconf; then
	rm -rf autom4te.cache
	echo "Now run ./configure"
fi
