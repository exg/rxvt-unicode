#ifndef RXVT_SALLOC_H_
#define RXVT_SALLOC_H_

#include <stdlib.h>

// small blocks allocator

struct rxvt_salloc
{
  struct chain {
    struct chain *next;
  };

  chain *firstblock;
  chain *firstline;
  unsigned int firstfree;
  unsigned int size;

  rxvt_salloc (unsigned int size);
  ~rxvt_salloc ();

  void *alloc ();
  void *alloc (void *data, unsigned int datalen);
  void free (void *data);
};

#endif
