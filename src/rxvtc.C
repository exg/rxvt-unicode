#include "rxvtdaemon.h"

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
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

  sockaddr_un sa;

  sa.sun_family = AF_UNIX;
  strcpy (sa.sun_path, rxvt_connection::unix_sockname ());

  if (connect (fd, (sockaddr *)&sa, sizeof (sa)))
    {
      perror ("unable to bind listening socket");
      exit (EXIT_FAILURE);
    }
}

int
main(int argc, const char *const *argv)
{
  client c;
  char buf[PATH_MAX];

  c.send ("NEW");
  c.send ("DISPLAY"), c.send (getenv ("DISPLAY"));
  // instead of getcwd we could opendir(".") and pass the fd for fchdir *g*
  c.send ("CWD"), c.send (getcwd (buf, sizeof (buf)));

  for (int i = 0; i < argc; i++)
    c.send ("ARG"), c.send (argv[i]);

  c.send ("END");
}

