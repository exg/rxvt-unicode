#ifndef RXVT_SALLOC_H_
#define RXVT_SALLOC_H_

#include <cstdlib>

// small blocks allocator

struct rxvt_salloc {
  struct chain {
    struct chain *next;
  };

  chain *firstblock;
  chain *firstline;
  int firstfree;
  int size;

  rxvt_salloc (int size);
  ~rxvt_salloc ();

  void *alloc ();
  void free (void *data);
};

#endif
