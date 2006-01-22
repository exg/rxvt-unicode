/*--------------------------------*-C-*---------------------------------*
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

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "fdpass.h"

#ifndef CMSG_LEN // CMSG_SPACe && CMSG_LEN are rfc2292 extensions to unix
# define CMSG_LEN(len) (sizeof (cmsghdr) + len)
#endif

int
ptytty_send_fd (int socket, int fd)
{
  msghdr msg;
  iovec iov;
  char buf [CMSG_LEN (sizeof (int))];
  char data = 0;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_name       = 0;
  msg.msg_namelen    = 0;
  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = (void *)buf;
  msg.msg_controllen = sizeof buf;

  cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;
  cmsg->cmsg_len   = CMSG_LEN (sizeof (int));

  *(int *)CMSG_DATA (cmsg) = fd;

  msg.msg_controllen = cmsg->cmsg_len;

  return sendmsg (socket, &msg, 0);
}

int
ptytty_recv_fd (int socket)
{
  msghdr msg;
  iovec iov;
  char buf [CMSG_LEN (sizeof (int))];  /* ancillary data buffer */
  char data = 1;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_name       = 0;
  msg.msg_namelen    = 0;
  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = buf;
  msg.msg_controllen = sizeof buf;

  cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;
  cmsg->cmsg_len   = CMSG_LEN (sizeof (int));

  msg.msg_controllen = cmsg->cmsg_len;

  if (recvmsg (socket, &msg, 0) <= 0
      || data               != 0
      || msg.msg_controllen < CMSG_LEN (sizeof (int))
      || cmsg->cmsg_level   != SOL_SOCKET
      || cmsg->cmsg_type    != SCM_RIGHTS
      || cmsg->cmsg_len     < CMSG_LEN (sizeof (int)))
    return -1;

  return *(int *)CMSG_DATA (cmsg);
}

