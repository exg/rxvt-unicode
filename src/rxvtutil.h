#ifndef RXVT_UTIL_H
#define RXVT_UTIL_H

#include <stdlib.h>
#include <string.h>
#include "ecb.h"

// increases code size unless -fno-enforce-eh-specs
#if __GNUC__
# define NOTHROW
# define THROW(x)
#else
# define NOTHROW  throw()
# define THROW(x) throw x
#endif

// various utility functions
template<typename T, typename U> static inline T    min    (T  a, U b) { return a < (T)b ? a : (T)b; }
template<typename T, typename U> static inline void min_it (T &a, U b) {    a = a < (T)b ? a : (T)b; }
template<typename T, typename U> static inline T    max    (T  a, U b) { return a > (T)b ? a : (T)b; }
template<typename T, typename U> static inline void max_it (T &a, U b) {    a = a > (T)b ? a : (T)b; }

template<typename T, typename U, typename V> static inline T    clamp    (T  v, U a, V b) { return v < (T)a ? a : v >(T)b ? b : v; }
template<typename T, typename U, typename V> static inline void clamp_it (T &v, U a, V b) {    v = v < (T)a ? a : v >(T)b ? b : v; }

template<typename T, typename U> static inline void swap (T& a, U& b) { T t=a; a=(T)b; b=(U)t; }

template<typename T> static inline T squared_diff (T a, T b) { return (a-b)*(a-b); }

// linear interpolation
template<typename T, typename U, typename P>
static inline
T lerp (T a, U b, P p)
{
  return (long(a) * long(100 - p) + long(b) * long(p) + 50) / 100;
}

template <typename I, typename T>
I find (I first, I last, const T& value)
{
  while (first != last && *first != value)
    ++first;

  return first;
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

/* simplevec taken (and heavily modified), from:
 *
 *  MICO --- a free CORBA implementation
 *  Copyright (C) 1997-98 Kay Roemer & Arno Puder
 */
template<class T>
struct simplevec
{
    typedef T* iterator;
    typedef const T* const_iterator;
    typedef unsigned long size_type;

private:
    size_type _last, _size;
    T *_buf;

public:
    const_iterator begin () const
    {
        return &_buf[0];
    }
    iterator begin ()
    {
        return &_buf[0];
    }
    const_iterator end () const
    {
        return &_buf[_last];
    }
    iterator end ()
    {
        return &_buf[_last];
    }
    size_type capacity () const
    {
        return _size;
    }
    size_type size () const
    {
        return _last;
    }

private:
    static T *alloc (size_type n)
    {
        return (T *)::operator new ((size_t) (n * sizeof (T)));
    }
    static void dealloc (T *buf)
    {
        if (buf)
            ::operator delete (buf);
    }

    void reserve (iterator where, size_type n)
    {
        if (_last + n <= _size) {
            memmove (where+n, where, (end ()-where)*sizeof (T));
        } else {
            size_type sz = _last+n;
            sz = (_size == 0) ? max (sz, 5) : max (sz, 2*_size);
            T *nbuf = alloc (sz);
            if (_buf) {
                memcpy (nbuf, begin (), (where-begin ())*sizeof (T));
                memcpy (nbuf + (where-begin ()) + n, where,
                        (end ()-where)*sizeof (T));
                dealloc (_buf);
            }
            _buf = nbuf;
            _size = sz;
        }
    }

public:
    void reserve (size_type sz)
    {
        if (_size < sz) {
            sz = (_size == 0) ? max (sz, 5) : max (sz, 2*_size);
            T *nbuf = alloc (sz);
            if (_buf) {
                memcpy (nbuf, begin (), size ()*sizeof (T));
                dealloc (_buf);
            }
            _buf = nbuf;
            _size = sz;
        }
    }
    simplevec ()
    : _last(0), _size(0), _buf(0)
    {
    }
    simplevec (size_type n, const T& t = T ())
    : _last(0), _size(0), _buf(0)
    {
        insert (begin (), n, t);
    }
    simplevec (const_iterator first, const_iterator last)
    : _last(0), _size(0), _buf(0)
    {
        insert (begin (), first, last);
    }
    simplevec (const simplevec<T> &v)
    : _last(0), _size(0), _buf(0)
    {
        reserve (v._last);
        memcpy (_buf, v.begin (), v.size ()*sizeof (T));
        _last = v._last;
    }
    simplevec<T> &operator= (const simplevec<T> &v)
    {
        if (this != &v) {
            _last = 0;
            reserve (v._last);
            memcpy (_buf, v.begin (), v.size ()*sizeof (T));
            _last = v._last;
        }
        return *this;
    }
    ~simplevec ()
    {
        dealloc (_buf);
    }
    const T &front () const
    {
        //ministl_assert (size () > 0);
        return _buf[0];
    }
    T &front ()
    {
        //ministl_assert (size () > 0);
        return _buf[0];
    }
    const T &back () const
    {
        //ministl_assert (size () > 0);
        return _buf[_last-1];
    }
    T &back ()
    {
        //ministl_assert (size () > 0);
        return _buf[_last-1];
    }
    bool empty () const
    {
        return _last == 0;
    }
    void clear ()
    {
        _last = 0;
    }
    void push_back (const T &t)
    {
        reserve (_last+1);
        *end () = t;
        ++_last;
    }
    void push_back (T &t)
    {
        reserve (_last+1);
        *end () = t;
        ++_last;
    }
    void pop_back ()
    {
        //ministl_assert (size () > 0);
        --_last;
    }
    const T &operator[] (size_type idx) const
    {
        //ministl_assert (idx < size ());
        return _buf[idx];
    }
    T &operator[] (size_type idx)
    {
        //ministl_assert (idx < size ());
        return _buf[idx];
    }
    iterator insert (iterator pos, const T &t)
    {
        //ministl_assert (pos <= end ());
        long at = pos - begin ();
        reserve (pos, 1);
        pos = begin ()+at;
        *pos = t;
        ++_last;
        return pos;
    }
    iterator insert (iterator pos, const_iterator first, const_iterator last)
    {
        //ministl_assert (pos <= end ());
        long n = last - first;
        long at = pos - begin ();
        if (n > 0) {
            reserve (pos, n);
            pos = begin ()+at;
            memcpy (pos, first, (last-first)*sizeof (T));
            _last += n;
        }
        return pos;
    }
    iterator insert (iterator pos, size_type n, const T &t)
    {
        //ministl_assert (pos <= end ());
        long at = pos - begin ();
        if (n > 0) {
            reserve (pos, n);
            pos = begin ()+at;
            for (int i = 0; i < n; ++i)
                pos[i] = t;
            _last += n;
        }
        return pos;
    }
    void erase (iterator first, iterator last)
    {
        if (last != first) {
            memmove (first, last, (end () - last) * sizeof (T));
            _last -= last - first;
        }
    }
    void erase (iterator pos)
    {
        if (pos != end ()) {
            memmove (pos, pos+1, (end () - (pos+1)) * sizeof (T));
            --_last;
        }
    }
    void swap (simplevec<T> &t)
    {
        ::swap(_last, t._last);
        ::swap(_size, t._size);
        ::swap(_buf, t._buf);
    }
};

template<class T>
bool operator== (const simplevec<T> &v1, const simplevec<T> &v2)
{
    if (v1.size () != v2.size ())
        return false;
    return !v1.size () || !memcmp (&v1[0], &v2[0], v1.size ()*sizeof (T));
}

template<class T>
bool operator< (const simplevec<T> &v1, const simplevec<T> &v2)
{
    unsigned long minlast = min (v1.size (), v2.size ());
    for (unsigned long i = 0; i < minlast; ++i) {
        if (v1[i] < v2[i])
            return true;
        if (v2[i] < v1[i])
            return false;
    }
    return v1.size () < v2.size ();
}


template<typename T>
struct vector : simplevec<T>
{
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

template<typename T>
struct auto_ptr
{
  T *p;

  auto_ptr () : p (0) { }
  auto_ptr (T *a) : p (a) { }

  auto_ptr (auto_ptr<T> &a)
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

  // void because it makes sense in our context
  void operator = (T *a)
  {
    delete p;
    p = a;
  }

  void operator = (auto_ptr &a)
  {
    *this = a.p;
    a.p = 0;
  }

  template<typename A>
  void operator = (auto_ptr<A> &a)
  {
    *this = a.p;
    a.p = 0;
  }

  operator T * () const { return p; }

  T *operator -> () const { return p; }
  T &operator * () const { return *p; }

  T *get ()
  {
    T *r = p;
    p = 0;
    return r;
  }
};

typedef auto_ptr<char> auto_str;

#endif

