/*----------------------------------------------------------------------*
 * File:	salloc.C
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

#include "salloc.h"

#define SALLOC_BLOCK 65536 // size of basic block to allocate

rxvt_salloc::rxvt_salloc (unsigned int size)
{
  this->size = size < sizeof (chain) ? sizeof (chain) : size;
  firstline = 0;
  firstblock = 0;
  firstfree = SALLOC_BLOCK;
}

rxvt_salloc::~rxvt_salloc ()
{
  while (firstblock)
    {
      chain *next = firstblock->next;
      ::free (firstblock);
      firstblock = next;
    }
}

void *
rxvt_salloc::alloc ()
{
  void *r;

  if (firstline)
    {
      r = (void *)firstline;
      firstline = firstline->next;
    }
  else
    {
      if (firstfree + size > SALLOC_BLOCK)
        {
          chain *next = (chain *)rxvt_malloc ((SALLOC_BLOCK - sizeof (chain)) / size * size + sizeof (chain));
          next->next = firstblock;
          firstblock = next;
          firstfree = sizeof (chain);
        }

      r = (void *) ((char *)firstblock + firstfree);

      firstfree += size;
    }

  return r;
}

void *
rxvt_salloc::alloc (void *data, unsigned int datalen)
{
  void *s = alloc ();

  if (datalen < size)
    {
      memcpy (s, data, datalen);
      memset ((unsigned char *)s + datalen, 0, size - datalen); // not strictly required for screen.C
    }
  else
    memcpy (s, data, size);

  return s;
}

void
rxvt_salloc::free (void *data)
{
  if (!data)
    return;

  chain *line = (chain *)data;
  line->next = firstline;
  firstline = line;
}

