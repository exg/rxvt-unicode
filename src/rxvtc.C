/*--------------------------------*-C-*---------------------------------*
 * File:	rxvtc.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2003-2004 Marc Lehmann <pcg@goof.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

#include "../config.h"
#include "rxvtdaemon.h"

#include "rxvt.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>

struct client : rxvt_connection {
  client ();
};

client::client ()
{
  if ((fd = socket (PF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      perror ("unable to create communications socket");
      exit (EXIT_FAILURE);
    }

  char *sockname = rxvt_connection::unix_sockname ();
  sockaddr_un sa;
  sa.sun_family = AF_UNIX;
  strcpy (sa.sun_path, sockname);
  free (sockname);

  if (connect (fd, (sockaddr *)&sa, sizeof (sa)))
    {
      perror ("unable to connect to the rxvt-unicode daemon");
      exit (EXIT_FAILURE);
    }
}

extern char **environ;

int
main (int argc, const char *const *argv)
{
  client c;
  char buf[PATH_MAX];

  {
    sigset_t ss;

    sigemptyset (&ss);
    sigaddset (&ss, SIGHUP);
    sigprocmask (SIG_BLOCK, &ss, 0);
  }

  c.send ("NEW");
  // instead of getcwd we could opendir (".") and pass the fd for fchdir *g*
  c.send ("CWD"), c.send (getcwd (buf, sizeof (buf)));

  for (char **var = environ; *environ; environ++)
    c.send ("ENV"), c.send (*environ);

  const char *base = strrchr (argv[0], '/');
  base = base ? base + 1 : argv[0];
  c.send ("ARG"), c.send (strcmp (base, RXVTNAME "c") ? base : RXVTNAME);

  for (int i = 1; i < argc; i++)
    c.send ("ARG"), c.send (argv[i]);

  c.send ("END");

  auto_str tok;

  for (;;)
    if (!c.recv (tok))
      {
        fprintf (stderr, "protocol error: unexpected eof from server.\n");
        break;
      }
    else if (!strcmp (tok, "MSG") && c.recv (tok))
      fprintf (stderr, "%s", (const char *)tok);
    else if (!strcmp (tok, "END"))
      {
        int success;
        if (c.recv (success))
          exit (success ? EXIT_SUCCESS : EXIT_FAILURE);
      }
    else
      {
        fprintf (stderr, "protocol error: received illegal token '%s'.\n", (const char *)tok);
        break;
      }

  return EXIT_FAILURE;
}

