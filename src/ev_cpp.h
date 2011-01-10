#define EV_COMPAT3 0
#define EV_USE_SELECT 1
#define EV_USE_EPOLL 1
#define EV_USE_KQUEUE 1
#define EV_USE_PORT 1
#define EV_PREPARE_ENABLE 1
#define EV_IDLE_ENABLE 1
#define EV_SIGNAL_ENABLE 1
#define EV_CHILD_ENABLE 1
#define EV_USE_STDEXCEPT 0
#define EV_CONFIG_H <config.h>

#include <config.h>

#if ENABLE_FRILLS
# define EV_FEATURES 1+2
#else
# define EV_FEATURES 0
# define NDEBUG
#endif

#include "ev++.h"
