/*----------------------------------------------------------------------*
 * File:	rxvtutil.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2004-2006 Marc Lehmann <pcg@goof.com>
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

#include <cstdlib>
#include <cstring>
#include <inttypes.h>

#include "rxvtutil.h"

class byteorder byteorder;

unsigned int byteorder::e;

byteorder::byteorder ()
{
  union {
    uint32_t u;
    uint8_t b[4];
  } w;

  w.b[0] = 0x11;
  w.b[1] = 0x22;
  w.b[2] = 0x33;
  w.b[3] = 0x44;

  e = w.u;
}

#if !HAVE_GCC_BUILTINS
int rxvt_ctz (unsigned int x) CONST
{
  int r = 0;

  x &= -x; // this isolates the lowest bit

  if (x & 0xaaaaaaaa) r +=  1;
  if (x & 0xcccccccc) r +=  2;
  if (x & 0xf0f0f0f0) r +=  4;
  if (x & 0xff00ff00) r +=  8;
  if (x & 0xffff0000) r += 16;

  return r;
}

int rxvt_popcount (unsigned int x) CONST
{
  x -=  (x >> 1) & 0x55555555;
  x  = ((x >> 2) & 0x33333333) + (x & 0x33333333);
  x  = ((x >> 4) + x) & 0x0f0f0f0f;
  x *= 0x01010101;

  return x >> 24;
}
#endif

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
      temp_buf = realloc (temp_buf, len);
      temp_len = len;
    }

  return temp_buf;
}


