#ifndef RXVT_VEC_H
#define RXVT_VEC_H

template<typename T> static inline T min (T a, long b) { return a < b ? a : b; }
template<typename T> static inline T max (T a, long b) { return a > b ? a : b; }

#include <cstring>
#include "simplevec.h"

#if 0
template<typename T>
struct rxvt_vec : simplevec<void *> {
  typedef T *iterator;

  void push_back (T d) { simplevec<void *>::push_back ((void *)d); }
  T pop_back () { return (T*)simplevec<void *>::pop_back (); }
  void erase (int i) { erase (begin () + i); }
  void erase (iterator i) { simplevec<void *>::erase ((void **)i); }
  iterator begin () const { return (iterator)simplevec<void *>::begin (); }
  iterator end () const { return (iterator)simplevec<void *>::end (); }
  T &operator [](int i) { return *(T *)(&((*(simplevec<void *> *)this)[i])); }
  const T &operator [](int i) const { return *(const T *)(&((*(const simplevec<void *> *)this)[i])); }
};
#endif

template <typename I, typename T>
I find(I first, I last, const T& value)
{
  while (first != last && *first != value)
    ++first;

  return first;
}

#endif
