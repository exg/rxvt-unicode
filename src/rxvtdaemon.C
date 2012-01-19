/*----------------------------------------------------------------------*
 * File:	rxvtdaemon.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2003-2007 Marc Lehmann <schmorp@schmorp.de>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <inttypes.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/utsname.h>

#include "rxvtdaemon.h"

char *rxvt_connection::unix_sockname ()
{
  char name[PATH_MAX];
  char *path = getenv ("RXVT_SOCKET");

  if (!path)
    {
      struct utsname u;
      uname (&u);

      path = getenv ("HOME");
      if (!path)
        path = "/tmp";

      snprintf (name, PATH_MAX, "%s/.urxvt", path);
      mkdir (name, 0777);

      snprintf (name, PATH_MAX, "%s/.urxvt/urxvtd-%s",
                path,
                u.nodename);

      path = name;
    }

  return strdup (path);
}

void rxvt_connection::send (const char *data, int len)
{
  uint8_t s[2];

  if (len > 65535)
    len = 65535;

  s[0] = len >> 8; s[1] = len;

  write (fd, s, 2);
  write (fd, data, len);
}

void rxvt_connection::send (const char *data)
{
  send (data, strlen (data));
}

bool rxvt_connection::recv (auto_str &data, int *len)
{
  uint8_t s[2];
  int l;

  if (read (fd, s, 2) != 2)
    return false;

  l = (s[0] << 8) + s[1];
  if (l > 65535)
    return false;

  if (len)
    *len = l;

  data = new char[l + 1];

  if (read (fd, data, l) != l)
    return false;

  data[l] = 0;

  return true;
}

void rxvt_connection::send (int data)
{
  uint8_t s[4];

  s[0] = data >> 24; s[1] = data >> 16; s[2] = data >> 8; s[3] = data;

  write (fd, s, 4);
}

bool rxvt_connection::recv (int &data)
{
  uint8_t s[4];

  if (read (fd, s, 4) != 4)
    return false;

  data = (((((s[0] << 8) | s[1]) << 8) | s[2]) << 8) | s[3];

  return true;
}



