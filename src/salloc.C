#include "salloc.h"

#define SALLOC_BLOCK 65536 // size of basic block to allocate

rxvt_salloc::rxvt_salloc (int size)
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

      r = (void *)((char *)firstblock + firstfree);

      firstfree += size;
    }

  return r;
}

void
rxvt_salloc::free (void *data)
{
  chain *line = (chain *)data;
  line->next = firstline;
  firstline = line;
}

