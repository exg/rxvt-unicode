#ifndef LOGGING_H_
#define LOGGING_H_

#ifdef UTMP_SUPPORT
# if ! defined(HAVE_STRUCT_UTMPX) && ! defined(HAVE_STRUCT_UTMP)
#  error cannot build with utmp support - no utmp or utmpx struct found
# endif

# ifdef HAVE_LASTLOG_H
#  include <lastlog.h>
# endif
# include <pwd.h>

# ifdef RXVT_UTMP_SYSV
#  ifndef USER_PROCESS
#   define USER_PROCESS		7
#  endif
#  ifndef DEAD_PROCESS
#   define DEAD_PROCESS		8
#  endif
# endif

# ifdef __QNX__
#  include <sys/utsname.h>
#  define ut_name		ut_user
# endif

#endif
#endif /* _LOGGING_H_ */
