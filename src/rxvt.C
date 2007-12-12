/*----------------------------------------------------------------------*
 * File:	rxvt.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2003-2006 Marc Lehmann <pcg@goof.com>
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
#include "rxvt.h"

#include <cstdlib>
#include <cstring>

int
main (int argc, const char *const *argv)
try
  {
    rxvt_init ();

    rxvt_term *t = new rxvt_term;

#if ENABLE_PERL
    stringvec *args = new stringvec;
    stringvec *envv = new stringvec;

    for (int i = 0; i < argc; i++)
      args->push_back (strdup (argv [i]));

    for (char **var = environ; *var; var++)
      envv->push_back (strdup (*var));

    envv->push_back (0);

    t->init (args, envv);
#else
    t->init (argc, argv, 0);
#endif

    ev_loop (0);

    return EXIT_SUCCESS;
  }
catch (const class rxvt_failure_exception &e)
  {
    return EXIT_FAILURE;
  }

