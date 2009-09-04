#define EV_MINIMAL 2
#define EV_USE_POLL 0
#define EV_USE_INOTIFY 0
#define EV_USE_EVENTFD 0
#define EV_USE_SIGNALFD 0
#define EV_MULTIPLICITY 0
#define EV_PERIODIC_ENABLE 0
#define EV_STAT_ENABLE 0
#define EV_FORK_ENABLE 0
#define EV_ASYNC_ENABLE 0
#define EV_EMBED_ENABLE 0
#define EV_CONFIG_H <config.h>
#define EV_MINPRI 0
#define EV_MAXPRI 0
#define EV_USE_STDEXCEPT 0

#include <config.h>

#if !ENABLE_FRILLS
# define NDEBUG
#endif

#include "ev++.h"
