#include "rxvtdaemon.h"

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

struct client : rxvt_connection {
  client ();
};

client::client ()
{
  if ((fd = socket (PF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
      perror ("unable to create listening socket");
      exit (EXIT_FAILURE);
    }

  char *sockname = rxvt_connection::unix_sockname ();
  sockaddr_un sa;
  sa.sun_family = AF_UNIX;
  strcpy (sa.sun_path, sockname);
  free (sockname);

  if (connect (fd, (sockaddr *)&sa, sizeof (sa)))
    {
      perror ("unable to bind listening socket");
      exit (EXIT_FAILURE);
    }
}

extern char **environ;

int
main(int argc, const char *const *argv)
{
  client c;
  char buf[PATH_MAX];

  {
    sigset_t ss;

    sigaddset (&ss, SIGHUP);
    sigprocmask (SIG_BLOCK, &ss, 0);
  }

  c.send ("NEW");
  // instead of getcwd we could opendir(".") and pass the fd for fchdir *g*
  c.send ("CWD"), c.send (getcwd (buf, sizeof (buf)));

  for (char **var = environ; *environ; environ++)
    c.send ("ENV"), c.send (*environ);

  for (int i = 0; i < argc; i++)
    c.send ("ARG"), c.send (argv[i]);

  c.send ("END");
}

