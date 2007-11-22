#define EV_USE_POLL 0
#define EV_MULTIPLICITY 0
#define EV_PERIODICS 0

#ifndef __NetBSD__
// disable kqueue on anything but netbsd, as its broken
// on freebsd, openbsd and darwin
# define EV_USE_KQUEUE 0
#endif

#include "ev++.h"
