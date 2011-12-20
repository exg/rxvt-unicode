#include <config.h>
#include "rxvt.h"

#define PTYTTY_NO_LIBCPP 1

#define PTYTTY_REENTRANT 0

#define PTYTTY_FATAL rxvt_fatal
#define PTYTTY_WARN  rxvt_warn

#include "logging.C"
#include "proxy.C"
#include "ptytty.C"
