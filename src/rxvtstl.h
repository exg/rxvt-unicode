#ifndef RXVT_STL_H
#define RXVT_STL_H

template<typename T> static inline T min (T a, long b) { return a < b ? a : b; }
template<typename T> static inline T max (T a, long b) { return a > b ? a : b; }

#include "simplevec.h"

template<typename T>
struct vector : simplevec<T>
{
};

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

template<typename T>
struct auto_ptr {
  T *p;

  auto_ptr() : p(0) { }
  auto_ptr(T *a) : p(a) { }

  auto_ptr(auto_ptr<T> &a)
  {
    p = a.p;
    a.p = 0;
  }

  template<typename A>
  auto_ptr(auto_ptr<A> &a)
  {
    p = a.p;
    a.p = 0;
  }

  ~auto_ptr()
  {
    delete p;
  }

  // void because it makes sense in our context
  void operator =(T *a)
  {
    delete p;
    p = a;
  }

  void operator =(auto_ptr &a)
  {
    *this = a.p;
    a.p = 0;
  }

  template<typename A>
  void operator =(auto_ptr<A> &a)
  {
    *this = a.p;
    a.p = 0;
  }

  operator T *() const { return p; }

  T *operator ->() const { return p; }
  T &operator *() const { return *p; }

  T *get ()
  {
    T *r = p;
    p = 0;
    return r;
  }
};

typedef auto_ptr<char> auto_str;

struct stringvec : simplevec<char *>
{
  ~stringvec ()
  {
    for (char **c = begin(); c != end(); c++)
      delete [] *c;
  }
};

#endif

