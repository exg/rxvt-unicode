#ifndef LOGGING_H_
#define LOGGING_H_

#ifdef UTMP_SUPPORT
# if !defined(RXVT_UTMPX_FILE) || !defined(HAVE_STRUCT_UTMPX)
#  undef HAVE_UTMPX_H
#  undef HAVE_STRUCT_UTMPX
# endif
# if !defined(RXVT_UTMP_FILE) || !defined(HAVE_STRUCT_UTMP)
#  undef HAVE_UTMP_H
#  undef HAVE_STRUCT_UTMP
# endif

# ifdef HAVE_UTMPX_H
#  include <utmpx.h>
# endif
# ifdef HAVE_UTMP_H
#  include <utmp.h>
# endif

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

struct rxvt_session
{
  int utmp_pos;
  int cmd_pid;
  bool login_shell;

#ifdef HAVE_STRUCT_UTMP
  struct utmp ut;
#endif
#ifdef HAVE_STRUCT_UTMPX
  struct utmpx utx;
#endif
#if (defined(HAVE_STRUCT_UTMP) && defined(HAVE_UTMP_PID)) || defined(HAVE_STRUCT_UTMPX)
  char ut_id[5];
#endif

  void login (const char *pty, int cmd_pid, bool login_shell, const char *hostname);
  void logout ();
};

#endif

#endif /* _LOGGING_H_ */
