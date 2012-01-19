/*----------------------------------------------------------------------*
 * File:	rxvtutil.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2004-2006 Marc Lehmann <schmorp@schmorp.de>
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
#include <inttypes.h>

#include "rxvtutil.h"

void *
zero_initialized::operator new (size_t s)
{
  void *p = malloc (s);

  memset (p, 0, s);
  return p;
}

void
zero_initialized::operator delete (void *p, size_t s)
{
  free (p);
}

static void *temp_buf;
static uint32_t temp_len;

void *
rxvt_temp_buf (int len)
{
  if (len > temp_len)
    {
      free (temp_buf);
      temp_buf = malloc (len);
      temp_len = len;
    }

  return temp_buf;
}


