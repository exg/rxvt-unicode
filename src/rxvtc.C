/*----------------------------------------------------------------------*
 * File:	rxvtc.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2003-2006 Marc Lehmann <schmorp@schmorp.de>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "rxvtdaemon.h"
#include "libptytty.h"

#define STATUS_SUCCESS           0
#define STATUS_FAILURE           1
#define STATUS_CONNECTION_FAILED 2

struct client : rxvt_connection
{
  client ();
};

client::client ()
{
  sockaddr_un sa;
  char *sockname = rxvt_connection::unix_sockname ();

  if (strlen (sockname) >= sizeof (sa.sun_path))
    {
      fputs ("socket name too long, aborting.\n", stderr);
      exit (STATUS_FAILURE);
    }

  if ((fd = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      perror ("unable to create communications socket");
      exit (STATUS_FAILURE);
    }

  sa.sun_family = AF_UNIX;
  strcpy (sa.sun_path, sockname);
  free (sockname);

  if (connect (fd, (sockaddr *)&sa, sizeof (sa)))
    {
      perror ("unable to connect to the rxvt-unicode daemon");
      exit (STATUS_CONNECTION_FAILED);
    }
}

extern char **environ;

int
main (int argc, const char *const *argv)
{
  // instead of getcwd we could opendir (".") and pass the fd for fchdir *g*
  char cwd[PATH_MAX];

  if (!getcwd (cwd, sizeof (cwd)))
    {
      perror ("unable to determine current working directory");
      exit (STATUS_FAILURE);
    }

  client c;

  {
    sigset_t ss;

    sigemptyset (&ss);
    sigaddset (&ss, SIGHUP);
    sigaddset (&ss, SIGPIPE);
    sigprocmask (SIG_BLOCK, &ss, 0);
  }

  c.send ("NEW");

  for (char **var = environ; *var; var++)
    c.send ("ENV"), c.send (*var);

  const char *base = strrchr (argv[0], '/');
  base = base ? base + 1 : argv[0];
  c.send ("ARG"), c.send (strcmp (base, RXVTNAME "c") ? base : RXVTNAME);

  c.send ("ARG"), c.send ("-cd");
  c.send ("ARG"), c.send (cwd);

  for (int i = 1; i < argc; i++)
    c.send ("ARG"), c.send (argv[i]);

  c.send ("END");

  auto_str tok;
  int cint;

  for (;;)
    if (!c.recv (tok))
      {
        fprintf (stderr, "protocol error: unexpected eof from server.\n");
        break;
      }
    else if (!strcmp (tok, "MSG") && c.recv (tok))
      fprintf (stderr, "%s", (const char *)tok);
    else if (!strcmp (tok, "GETFD") && c.recv (cint))
      {
        if (!ptytty::send_fd (c.fd, cint))
          {
            fprintf (stderr, "unable to send fd %d: ", cint); perror (0);
            exit (STATUS_FAILURE);
          }
      }
    else if (!strcmp (tok, "END"))
      {
        int success;

        if (c.recv (success))
          exit (success ? STATUS_SUCCESS : STATUS_FAILURE);
      }
    else
      {
        fprintf (stderr, "protocol error: received unsupported token '%s'.\n", (const char *)tok);
        break;
      }

  return STATUS_FAILURE;
}

