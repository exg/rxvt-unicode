#include "rxvtlib.h"
#include "rxvtdaemon.h"
#include "iom.h"

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <cerrno>

extern char **environ;

struct server : rxvt_connection {
  void read_cb (io_watcher &w, short revents); io_watcher read_ev;

  server (int fd)
  : read_ev (this, &server::read_cb)
  {
    this->fd = fd;
    read_ev.start (fd, EVENT_READ);
  }

  void err (const char *format = 0, ...);
};

struct unix_listener {
  int fd;

  void accept_cb (io_watcher &w, short revents); io_watcher accept_ev;

  unix_listener (const char *sockname);
};

unix_listener::unix_listener (const char *sockname)
: accept_ev (this, &unix_listener::accept_cb)
{
  if ((fd = socket (PF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
      perror ("unable to create listening socket");
      exit (EXIT_FAILURE);
    }

  sockaddr_un sa;

  sa.sun_family = AF_UNIX;
  strcpy (sa.sun_path, sockname);

  unlink (rxvt_connection::unix_sockname ());

  mode_t omask = umask (0077);

  if (bind (fd, (sockaddr *)&sa, sizeof (sa)))
    {
      perror ("unable to bind listening socket");
      exit (EXIT_FAILURE);
    }

  umask (omask);

  if (listen (fd, 5))
    {
      perror ("unable to listen on socket");
      exit (EXIT_FAILURE);
    }

  accept_ev.start (fd, EVENT_READ);
}

void unix_listener::accept_cb (io_watcher &w, short revents)
{
  int fd2 = accept (fd, 0, 0);

  if (fd2 >= 0)
    new server (fd2);
}

void server::err (const char *format, ...)
{
  if (format)
    {
      char err[1024];

      va_list ap;
      va_start (ap, format);
      vsnprintf (err, 1024, format, ap);
      va_end (ap);

      send ("ERR"), send (err);
    }

  close (fd);
  delete this;
}

void server::read_cb (io_watcher &w, short revents)
{
  auto_str tok;

  if (recv (tok))
    {
      if (!strcmp (tok, "NEW"))
        {
          stringvec argv;
          stringvec envv;
            
          for (;;)
            {
              if (!recv (tok))
                return err ();

              if (!strcmp (tok, "END"))
                break;
              else if (!strcmp (tok, "ENV") && recv (tok))
                envv.push_back (tok.get ());
              else if (!strcmp (tok, "CWD") && recv (tok))
                {
                  if (chdir (tok))
                    err ("unable to change to working directory to '%s': %s",
                         (char *)tok, strerror (errno));
                }
              else if (!strcmp (tok, "ARG") && recv (tok))
                argv.push_back (tok.get ());
              else
                return err ("protocol error: unexpected NEW token");
            }

          envv.push_back (0);

          {
            char **old_environ = environ;
            environ = envv.begin ();

            rxvt_init (argv.size (), argv.begin ());

            environ = old_environ;
            envv.clear (); // can't yet save the env 'cause rxvt modifies it :(
          }
        }
      else
        return err ("protocol error: request '%s' unsupported.", (char *)tok);
    }
  else
    return err ();
}

int
main(int argc, const char *const *argv)
{
  rxvt_init_signals ();

  char *sockname = rxvt_connection::unix_sockname ();
  unix_listener l (sockname);
  printf ("rxvtd listening on %s.\n", sockname);
  free (sockname);

  iom.loop ();

#if 0
  if (rxvt_init(argc, argv) == NULL)
      return EXIT_FAILURE;

  dR;
  rxvt_main_loop(aR);	/* main processing loop */
#endif
  return EXIT_SUCCESS;
}
