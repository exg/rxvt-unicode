// THIS IS A GENERATED FILE: RUN callback.pl to regenerate it
// THIS IS A GENERATED FILE: callback.pl is part of the GVPE
// THIS IS A GENERATED FILE: distribution.

/*
    callback.h -- C++ callback mechanism
    Copyright (C) 2003-2007 Marc Lehmann <schmorp@schmorp.de>
 
    This file is part of GVPE.

    GVPE is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with gvpe; if not, write to the Free Software
    Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef CALLBACK_H__
#define CALLBACK_H__

#define CALLBACK_H_VERSION 3

template<typename signature>
struct callback;

template<class R>
struct callback<R ()>
{
  typedef R (*ptr_type)(void *self);

  template<class K, R (K::*method)()>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call () const
  {
    return func (self);
  }

  R operator ()() const
  {
    return call ();
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)()>
  static R thunk (void *self)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) ();
  }
};

template<class R, class A1>
struct callback<R (A1)>
{
  typedef R (*ptr_type)(void *self, A1);

  template<class K, R (K::*method)(A1)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1) const
  {
    return func (self, a1);
  }

  R operator ()(A1 a1) const
  {
    return call (a1);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1)>
  static R thunk (void *self, A1 a1)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1);
  }
};

template<class R, class A1, class A2>
struct callback<R (A1, A2)>
{
  typedef R (*ptr_type)(void *self, A1, A2);

  template<class K, R (K::*method)(A1, A2)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2) const
  {
    return func (self, a1, a2);
  }

  R operator ()(A1 a1, A2 a2) const
  {
    return call (a1, a2);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2)>
  static R thunk (void *self, A1 a1, A2 a2)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2);
  }
};

template<class R, class A1, class A2, class A3>
struct callback<R (A1, A2, A3)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3);

  template<class K, R (K::*method)(A1, A2, A3)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3) const
  {
    return func (self, a1, a2, a3);
  }

  R operator ()(A1 a1, A2 a2, A3 a3) const
  {
    return call (a1, a2, a3);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3);
  }
};

template<class R, class A1, class A2, class A3, class A4>
struct callback<R (A1, A2, A3, A4)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3, A4);

  template<class K, R (K::*method)(A1, A2, A3, A4)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3, A4 a4) const
  {
    return func (self, a1, a2, a3, a4);
  }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4) const
  {
    return call (a1, a2, a3, a4);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3, A4)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3, A4 a4)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3, a4);
  }
};

template<class R, class A1, class A2, class A3, class A4, class A5>
struct callback<R (A1, A2, A3, A4, A5)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3, A4, A5);

  template<class K, R (K::*method)(A1, A2, A3, A4, A5)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
  {
    return func (self, a1, a2, a3, a4, a5);
  }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
  {
    return call (a1, a2, a3, a4, a5);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3, A4, A5)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3, a4, a5);
  }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6>
struct callback<R (A1, A2, A3, A4, A5, A6)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3, A4, A5, A6);

  template<class K, R (K::*method)(A1, A2, A3, A4, A5, A6)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
  {
    return func (self, a1, a2, a3, a4, a5, a6);
  }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
  {
    return call (a1, a2, a3, a4, a5, a6);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3, A4, A5, A6)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3, a4, a5, a6);
  }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct callback<R (A1, A2, A3, A4, A5, A6, A7)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3, A4, A5, A6, A7);

  template<class K, R (K::*method)(A1, A2, A3, A4, A5, A6, A7)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
  {
    return func (self, a1, a2, a3, a4, a5, a6, a7);
  }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
  {
    return call (a1, a2, a3, a4, a5, a6, a7);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3, A4, A5, A6, A7)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3, a4, a5, a6, a7);
  }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct callback<R (A1, A2, A3, A4, A5, A6, A7, A8)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3, A4, A5, A6, A7, A8);

  template<class K, R (K::*method)(A1, A2, A3, A4, A5, A6, A7, A8)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
  {
    return func (self, a1, a2, a3, a4, a5, a6, a7, a8);
  }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
  {
    return call (a1, a2, a3, a4, a5, a6, a7, a8);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3, A4, A5, A6, A7, A8)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3, a4, a5, a6, a7, a8);
  }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct callback<R (A1, A2, A3, A4, A5, A6, A7, A8, A9)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3, A4, A5, A6, A7, A8, A9);

  template<class K, R (K::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const
  {
    return func (self, a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const
  {
    return call (a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3, a4, a5, a6, a7, a8, a9);
  }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct callback<R (A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>
{
  typedef R (*ptr_type)(void *self, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

  template<class K, R (K::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>
  void set (K *object)
  {
    self = object;
    func = thunk<K, method>;
  }

  R call (A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const
  {
    return func (self, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const
  {
    return call (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }

private:

  void *self;
  ptr_type func;

  template<class klass, R (klass::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>
  static R thunk (void *self, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10)
  {
    klass *obj = static_cast<klass *>(self);
    return (obj->*method) (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
  }
};


#endif
