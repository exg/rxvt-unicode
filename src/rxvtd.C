#include "rxvtlib.h"
#include "rxvtdaemon.h"
#include "iom.h"

#include <cstdio>
#include <cstdlib>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

struct server : rxvt_connection {
  void read_cb (io_watcher &w, short revents); io_watcher read_ev;

  server (int fd)
  : read_ev (this, &server::read_cb)
  {
    this->fd = fd;
    read_ev.start (fd, EVENT_READ);
  }

  void err ();
};

struct listener {
  int fd;

  void accept_cb (io_watcher &w, short revents); io_watcher accept_ev;

  listener ();
};

listener::listener ()
: accept_ev (this, &listener::accept_cb)
{
  if ((fd = socket (PF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
      perror ("unable to create listening socket");
      exit (EXIT_FAILURE);
    }

  sockaddr_un sa;

  sa.sun_family = AF_UNIX;
  strcpy (sa.sun_path, rxvt_connection::unix_sockname ());

  unlink (rxvt_connection::unix_sockname ());

  if (bind (fd, (sockaddr *)&sa, sizeof (sa)))
    {
      perror ("unable to bind listening socket");
      exit (EXIT_FAILURE);
    }

  if (listen (fd, 5))
    {
      perror ("unable to listen on socket");
      exit (EXIT_FAILURE);
    }

  accept_ev.start (fd, EVENT_READ);
}

void listener::accept_cb (io_watcher &w, short revents)
{
  int fd2 = accept (fd, 0, 0);

  if (fd2 >= 0)
    new server (fd2);
}

void server::err ()
{
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
          auto_str display, cwd;
          simplevec<auto_str> argv;
            
          for (;;)
            {
              if (!recv (tok))
                return err ();

              if (!strcmp (tok, "END"))
                break;
              else if (!strcmp (tok, "DISPLAY") && recv (display))
                ;
              else if (!strcmp (tok, "CWD") && recv (cwd))
                ;
              else if (!strcmp (tok, "ARG") && recv (tok))
                argv.push_back (tok);
              else
                return err ();
            }

          // TODO: no setenv, please
          setenv ("DISPLAY", display.get (), 1);

          rxvt_init (argv.size (), reinterpret_cast<char **>(argv.begin ()));
          dR;
          rxvt_main_loop (aR);
        }
      else
        return err ();
    }
  else
    return err ();
}

int
main(int argc, const char *const *argv)
{
  listener l;
  iom.loop ();

#if 0
  if (rxvt_init(argc, argv) == NULL)
      return EXIT_FAILURE;

  dR;
  rxvt_main_loop(aR);	/* main processing loop */
#endif
  return EXIT_SUCCESS;
}
