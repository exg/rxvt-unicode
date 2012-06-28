#ifndef RXVT_UTIL_H
#define RXVT_UTIL_H

#include <new>
#include <stdlib.h>
#include <string.h>
#include "ecb.h"
#include "estl.h"

#include "emman.h"

// increases code size unless -fno-enforce-eh-specs
#if __GNUC__
# define NOTHROW
# define THROW(x)
#else
# define NOTHROW  throw()
# define THROW(x) throw x
#endif

// various utility functions
template<typename T, typename U> static inline void min_it (T &a, U b) { a = a < (T)b ? a : (T)b; }
template<typename T, typename U> static inline void max_it (T &a, U b) { a = a > (T)b ? a : (T)b; }

template<typename T, typename U, typename V> static inline T    clamp    (T  v, U a, V b) { return v < (T)a ? a : v >(T)b ? b : v; }
template<typename T, typename U, typename V> static inline void clamp_it (T &v, U a, V b) {    v = v < (T)a ? a : v >(T)b ? b : v; }

template<typename T> static inline T squared_diff (T a, T b) { return (a - b) * (a - b); }

// linear interpolation
template<typename T, typename U, typename P>
static inline T
lerp (T a, U b, P p)
{
  return (long(a) * long(100 - p) + long(b) * long(p) + 50) / 100;
}

// return a very temporary (and never deallocated) buffer. keep small.
void *rxvt_temp_buf (int len);

template<typename T>
static inline T *
rxvt_temp_buf (int len)
{
  return (T *)rxvt_temp_buf (len * sizeof (T));
}

// in range including end
#define IN_RANGE_INC(val,beg,end) \
  ((unsigned int)(val) - (unsigned int)(beg) <= (unsigned int)(end) - (unsigned int)(beg))

// in range excluding end
#define IN_RANGE_EXC(val,beg,end) \
  ((unsigned int)(val) - (unsigned int)(beg) <  (unsigned int)(end) - (unsigned int)(beg))

// for m >= -n, ensure remainder lies between 0..n-1
#define MOD(m,n) (((m) + (n)) % (n))

// makes dynamically allocated objects zero-initialised
struct zero_initialized
{
  void *operator new (size_t s);
  void operator delete (void *p, size_t s);
};

struct stringvec : simplevec<char *>
{
  ~stringvec ()
  {
    for (char **c = begin (); c != end (); c++)
      free (*c);
  }
};

#if 0
template<typename T>
struct rxvt_vec : simplevec<void *>
{
  typedef T *iterator;

  void push_back (T d) { simplevec<void *>::push_back ((void *)d); }
  T pop_back () { return (T*)simplevec<void *>::pop_back (); }
  void erase (int i) { erase (begin () + i); }
  void erase (iterator i) { simplevec<void *>::erase ((void **)i); }
  iterator begin () const { return (iterator)simplevec<void *>::begin (); }
  iterator end () const { return (iterator)simplevec<void *>::end (); }
  T &operator [] (int i) { return * (T *) (& ((* (simplevec<void *> *)this)[i])); }
  const T &operator [] (int i) const { return * (const T *) (& ((* (const simplevec<void *> *)this)[i])); }
};
#endif

inline void *
operator new (size_t size)
{
  // TODO: use rxvt_malloc
  return malloc (size);
}

inline void
operator delete (void *p)
{
  free (p);
}

template<typename T>
struct auto_ptr
{
  T *p;

  auto_ptr ()     : p (0) { }

  explicit
  auto_ptr (T *a) : p (a) { }

  auto_ptr (auto_ptr &a)
  {
    p = a.p;
    a.p = 0;
  }

  template<typename A>
  auto_ptr (auto_ptr<A> &a)
  {
    p = a.p;
    a.p = 0;
  }

  ~auto_ptr ()
  {
    delete p;
  }

  void reset (T *a)
  {
    delete p;
    p = a;
  }

  // void because it makes sense in our context
  void operator =(auto_ptr &a)
  {
    reset (a.release ());
  }

  template<typename A>
  void operator =(auto_ptr<A> &a)
  {
    reset (a.release ());
  }

  T *operator ->() const { return p; }
  T &operator *() const { return *p; }

  operator T *()  { return p; }
  T *get () const { return p; }

  T *release()
  {
    T *r = p;
    p = 0;
    return r;
  }
};

typedef auto_ptr<char> auto_str;

#endif

