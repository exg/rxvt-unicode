/* this file lives in libptytty/src/estl.h, only edit it there and copy it here */
#ifndef ESTL_H_
#define ESTL_H_

#include <stdlib.h>
#include <string.h>

#include "ecb.h"

template<typename T, typename U> static inline T min (T a, U b) { return a < (T)b ? a : (T)b; }
template<typename T, typename U> static inline T max (T a, U b) { return a > (T)b ? a : (T)b; }

template<typename T, typename U> static inline void swap (T& a, U& b) { T t = a; a = (T)b; b = (U)t; }

template <typename I, typename T>
I find (I first, I last, const T& value)
{
  while (first != last && *first != value)
    ++first;

  return first;
}

#include <new>

#if ECB_CPP11
  #include <type_traits>
#endif

namespace estl
{
#if ESTL_LARGE_MEMORY_MODEL
  // should use size_t/ssize_t, but that's not portable enough for us
  typedef unsigned long size_type;
  typedef          long difference_type;
#else
  typedef uint32_t size_type;
  typedef  int32_t difference_type;
#endif

  template<typename T>
  struct scoped_ptr
  {
    T *p;

    scoped_ptr ()     : p (0) { }

    explicit
    scoped_ptr (T *a) : p (a) { }

    ~scoped_ptr ()
    {
      delete p;
    }

    void reset (T *a)
    {
      delete p;
      p = a;
    }

    T *operator ->() const { return p; }
    T &operator *() const { return *p; }

    operator T *()  { return p; }
    T *get () const { return p; }

  private:
    scoped_ptr (const scoped_ptr &);
    scoped_ptr &operator =(const scoped_ptr &);
  };

  template<typename T>
  struct scoped_array
  {
    T *p;

    scoped_array ()     : p (0) { }

    explicit
    scoped_array (T *a) : p (a) { }

    ~scoped_array ()
    {
      delete [] p;
    }

    void reset (T *a)
    {
      delete [] p;
      p = a;
    }

    operator T *()  { return p; }
    T *get () const { return p; }

  private:
    scoped_array (const scoped_array &);
    scoped_array &operator =(const scoped_array &);
  };
}

// original version taken from MICO, but this has been completely rewritten
// known limitations w.r.t. std::vector
// - many methods missing
// - no error checking, no exceptions thrown (e.g. at())
// - size_type is 32bit even on 64 bit hosts, so limited to 2**31 elements
// - no allocator support
// - we don't really care about const correctness, but we try
// - we don't care about namespaces and stupid macros the user might define
// - no bool specialisation
template<class T>
struct simplevec
{
  typedef estl::size_type size_type;

  typedef       T  value_type;
  typedef       T *iterator;
  typedef const T *const_iterator;
  typedef       T *pointer;
  typedef const T *const_pointer;
  typedef       T &reference;
  typedef const T &const_reference;
  // missing: allocator_type
  // missing: reverse iterator

private:
  size_type sze, res;
  T *buf;

  // we shamelessly optimise for "simple" types. everything
  // "not simple enough" will use the slow path.
  static bool is_simple_enough ()
  {
    #if ECB_CPP11
      return std::is_trivially_assignable<T, T>::value
          && std::is_trivially_constructible<T>::value
          && std::is_trivially_copyable<T>::value
          && std::is_trivially_destructible<T>::value;
    #elif ECB_GCC_VERSION(4,4) || ECB_CLANG_VERSION(2,8)
      return __has_trivial_assign (T)
          && __has_trivial_constructor (T)
          && __has_trivial_copy (T)
          && __has_trivial_destructor (T);
    #else
      return 0;
    #endif
  }

  static void construct (iterator a, size_type n = 1)
  {
    if (!is_simple_enough ())
      while (n--)
        new (a++) T ();
  }

  static void destruct (iterator a, size_type n = 1)
  {
    if (!is_simple_enough ())
      while (n--)
        (*a++).~T ();
  }

  template<class I>
  static void cop_new (iterator a, I b) { new (a) T (*b); }
  template<class I>
  static void cop_set (iterator a, I b) {     *a  =  *b ; }

  // MUST copy forwards
  template<class I>
  static void copy (iterator dst, I        src, size_type n, void (*op)(iterator,        I))
  {
    while (n--)
      op (dst++, src++);
  }

  static void copy (iterator dst, iterator src, size_type n, void (*op)(iterator, iterator))
  {
    if (is_simple_enough ())
      memcpy (dst, src, sizeof (T) * n);
    else
      copy<iterator> (dst, src, n, op);
  }

  static T *alloc (size_type n) ecb_cold
  {
    return (T *)::operator new ((size_t) (sizeof (T) * n));
  }

  void dealloc () ecb_cold
  {
    destruct (buf, sze);
    ::operator delete (buf);
  }

  size_type good_size (size_type n) ecb_cold
  {
    return n ? 2UL << ecb_ld32 (n) : 5;
  }

  void ins (iterator where, size_type n)
  {
    size_type pos = where - begin ();

    if (ecb_expect_false (sze + n > res))
      {
        res = good_size (sze + n);

        T *nbuf = alloc (res);
        copy (nbuf, buf, sze, cop_new);
        dealloc ();
        buf = nbuf;
      }

    construct (buf + sze, n);

    iterator src = buf + pos;
    if (is_simple_enough ())
      memmove (src + n, src, sizeof (T) * (sze - pos));
    else
      for (size_type i = sze - pos; i--; )
        cop_set (src + n + i, src + i);

    sze += n;
  }

public:
  size_type capacity () const { return res; }
  size_type size     () const { return sze; }
  bool empty         () const { return size () == 0; }

  size_t max_size () const
  {
    return (~(size_type)0) >> 1;
  }

  const_iterator  begin () const { return &buf [      0]; }
        iterator  begin ()       { return &buf [      0]; }
  const_iterator  end   () const { return &buf [sze    ]; }
        iterator  end   ()       { return &buf [sze    ]; }
  const_reference front () const { return  buf [      0]; }
        reference front ()       { return  buf [      0]; }
  const_reference back  () const { return  buf [sze - 1]; }
        reference back  ()       { return  buf [sze - 1]; }

  void reserve (size_type sz)
  {
    if (ecb_expect_true (sz <= res))
      return;

    sz = good_size (sz);
    T *nbuf = alloc (sz);

    copy (nbuf, begin (), sze, cop_new);
    dealloc ();

    buf  = nbuf;
    res = sz;
  }

  void resize (size_type sz)
  {
    reserve (sz);

    if (is_simple_enough ())
      sze = sz;
    else
      {
        while (sze < sz) construct (buf + sze++);
        while (sze > sz) destruct  (buf + --sze);
      }
  }

  simplevec ()
  : sze(0), res(0), buf(0)
  {
  }

  simplevec (size_type n, const T &t = T ())
  {
    sze = res = n;
    buf = alloc (sze);

    while (n--)
      new (buf + n) T (t);
  }

  simplevec (const_iterator first, const_iterator last)
  {
    sze = res = last - first;
    buf = alloc (sze);
    copy (buf, first, sze, cop_new);
  }

  simplevec (const simplevec<T> &v)
  : sze(0), res(0), buf(0)
  {
    sze = res = v.size ();
    buf = alloc (sze);
    copy (buf, v.begin (), sze, cop_new);
  }

  ~simplevec ()
  {
    dealloc ();
  }

  void swap (simplevec<T> &t)
  {
    ::swap (sze, t.sze);
    ::swap (res, t.res);
    ::swap (buf, t.buf);
  }

  void clear ()
  {
    destruct (buf, sze);
    sze = 0;
  }

  void push_back (const T &t)
  {
    reserve (sze + 1);
    new (buf + sze++) T (t);
  }

  void pop_back ()
  {
    destruct (buf + --sze);
  }

  const_reference operator [](size_type idx) const { return buf[idx]; }
        reference operator [](size_type idx)       { return buf[idx]; }

  const_reference at (size_type idx) const { return buf [idx]; }
        reference at (size_type idx)       { return buf [idx]; }

  void assign (const_iterator first, const_iterator last)
  {
    simplevec<T> v (first, last);
    swap (v);
  }

  void assign (size_type n, const T &t)
  {
    simplevec<T> v (n, t);
    swap (v);
  }

  simplevec<T> &operator= (const simplevec<T> &v)
  {
    assign (v.begin (), v.end ());
    return *this;
  }

  iterator insert (iterator pos, const T &t)
  {
    size_type at = pos - begin ();

    ins (pos, 1);
    buf [at] = t;

    return buf + at;
  }

  iterator insert (iterator pos, const_iterator first, const_iterator last)
  {
    size_type n  = last - first;
    size_type at = pos - begin ();

    ins (pos, n);
    copy (buf + at, first, n, cop_set);

    return buf + at;
  }

  iterator insert (iterator pos, size_type n, const T &t)
  {
    size_type at = pos - begin ();

    ins (pos, n);

    for (iterator i = buf + at; n--; )
      *i++ = t;

    return buf + at;
  }

  iterator erase (iterator first, iterator last)
  {
    size_type n = last - first;
    size_type c = end () - last;

    if (is_simple_enough ())
      memmove (first, last, sizeof (T) * c);
    else
      copy (first, last, c, cop_set);

    sze -= n;
    destruct (buf + sze, n);

    return first;
  }

  iterator erase (iterator pos)
  {
    if (pos != end ())
      erase (pos, pos + 1);

    return pos;
  }
};

template<class T>
bool operator ==(const simplevec<T> &v1, const simplevec<T> &v2)
{
  if (v1.size () != v2.size ()) return false;

  return !v1.size () || !memcmp (&v1[0], &v2[0], v1.size () * sizeof (T));
}

template<class T>
bool operator <(const simplevec<T> &v1, const simplevec<T> &v2)
{
  unsigned long minlast = min (v1.size (), v2.size ());

  for (unsigned long i = 0; i < minlast; ++i)
    {
      if (v1[i] < v2[i]) return true;
      if (v2[i] < v1[i]) return false;
    }
  return v1.size () < v2.size ();
}

template<typename T>
struct vector : simplevec<T>
{
};

#endif

