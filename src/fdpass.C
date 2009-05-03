// This file is part of libptytty. Do not make local modifications.
// http://software.schmorp.de/pkg/libptytty

/*----------------------------------------------------------------------*
 * File:	fdpass.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2006 Marc Lehmann <pcg@goof.com>
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

#include <cstddef> // needed by broken bsds for NULL used in sys/uio.h
#include <cstdlib>

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "libptytty.h"

// CMSG_SPACE & CMSG_LEN are rfc2292 extensions to unix
#ifndef CMSG_SPACE
# define CMSG_SPACE(len) (sizeof (cmsghdr) + len)
#endif

#ifndef CMSG_LEN
# define CMSG_LEN(len) (sizeof (cmsghdr) + len)
#endif

bool
ptytty::send_fd (int socket, int fd)
{
  void *buf = malloc (CMSG_SPACE (sizeof (int)));

  if (!buf)
    return 0;

  msghdr msg;
  iovec iov;
  cmsghdr *cmsg;
  char data = 0;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_name       = 0;
  msg.msg_namelen    = 0;
  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = buf;
  msg.msg_controllen = CMSG_SPACE (sizeof (int));

  cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;
  cmsg->cmsg_len   = CMSG_LEN (sizeof (int));

  *(int *)CMSG_DATA (cmsg) = fd;

  ssize_t result = sendmsg (socket, &msg, 0);

  free (buf);

  return result >= 0;
}

int
ptytty::recv_fd (int socket)
{
  void *buf = malloc (CMSG_SPACE (sizeof (int)));

  if (!buf)
    return -1;

  msghdr msg;
  iovec iov;
  char data = 1;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_name       = 0;
  msg.msg_namelen    = 0;
  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = buf;
  msg.msg_controllen = CMSG_SPACE (sizeof (int));

  int fd = -1;

  if (recvmsg (socket, &msg, 0) >  0
      && data                   == 0
      && msg.msg_controllen     >= CMSG_SPACE (sizeof (int)))
    {
      cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);

      if (cmsg->cmsg_level   == SOL_SOCKET
          && cmsg->cmsg_type == SCM_RIGHTS
          && cmsg->cmsg_len  >= CMSG_LEN (sizeof (int)))
        fd = *(int *)CMSG_DATA (cmsg);
    }

  free (buf);

  return fd;
}

