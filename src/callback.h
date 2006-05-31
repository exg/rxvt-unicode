// THIS IS A GENERATED FILE: RUN callback.pl to regenerate it
// THIS IS A GENERATED FILE: callback.pl is part of the GVPE
// THIS IS A GENERATED FILE: distribution.

/*
    callback.h -- C++ callback mechanism
    Copyright (C) 2003-2006 Marc Lehmann <pcg@goof.com>
 
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

#define CALLBACK_H_VERSION 2

template<class signature>
struct callback_funtype_trait;

template<int arity, class signature>
struct callback_get_impl;

template<class R>
class callback0
{
  struct object { };

  typedef R (object::*ptr_type)();

  void *obj;
  R (object::*meth)();

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)()) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)()) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)()>(meth)))
          ();
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback0 (O1 *object, R (O2::*method)())
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)()>(method);
      prxy = &p;
    }

  R call() const
    {
      return prxy->call (obj, meth);
    }

  R operator ()() const
    {
      return call ();
    }
};

template<class R>
struct callback_funtype_trait0
{
  static const int arity = 0;
  typedef R type (void);
  typedef R result_type;
  
};

template<class R>
struct callback_funtype_trait<R ()> : callback_funtype_trait0<R>
{
};

template<class signature>
struct callback_get_impl<0, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback0<typename T::result_type> type;
};
   
template<class R, class A1>
class callback1
{
  struct object { };

  typedef R (object::*ptr_type)(A1);

  void *obj;
  R (object::*meth)(A1);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1), A1 a1) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1), A1 a1) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1)>(meth)))
          (a1);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback1 (O1 *object, R (O2::*method)(A1))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1)>(method);
      prxy = &p;
    }

  R call(A1 a1) const
    {
      return prxy->call (obj, meth, a1);
    }

  R operator ()(A1 a1) const
    {
      return call (a1);
    }
};

template<class R, class A1>
struct callback_funtype_trait1
{
  static const int arity = 1;
  typedef R type (A1);
  typedef R result_type;
  typedef A1 arg1_type;
};

template<class R, class A1>
struct callback_funtype_trait<R (A1)> : callback_funtype_trait1<R, A1>
{
};

template<class signature>
struct callback_get_impl<1, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback1<typename T::result_type, typename T::arg1_type> type;
};
   
template<class R, class A1, class A2>
class callback2
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2);

  void *obj;
  R (object::*meth)(A1, A2);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2), A1 a1, A2 a2) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2), A1 a1, A2 a2) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2)>(meth)))
          (a1, a2);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback2 (O1 *object, R (O2::*method)(A1, A2))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2) const
    {
      return prxy->call (obj, meth, a1, a2);
    }

  R operator ()(A1 a1, A2 a2) const
    {
      return call (a1, a2);
    }
};

template<class R, class A1, class A2>
struct callback_funtype_trait2
{
  static const int arity = 2;
  typedef R type (A1, A2);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type;
};

template<class R, class A1, class A2>
struct callback_funtype_trait<R (A1, A2)> : callback_funtype_trait2<R, A1, A2>
{
};

template<class signature>
struct callback_get_impl<2, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback2<typename T::result_type, typename T::arg1_type, typename T::arg2_type> type;
};
   
template<class R, class A1, class A2, class A3>
class callback3
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3);

  void *obj;
  R (object::*meth)(A1, A2, A3);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3), A1 a1, A2 a2, A3 a3) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3), A1 a1, A2 a2, A3 a3) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3)>(meth)))
          (a1, a2, a3);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback3 (O1 *object, R (O2::*method)(A1, A2, A3))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3) const
    {
      return prxy->call (obj, meth, a1, a2, a3);
    }

  R operator ()(A1 a1, A2 a2, A3 a3) const
    {
      return call (a1, a2, a3);
    }
};

template<class R, class A1, class A2, class A3>
struct callback_funtype_trait3
{
  static const int arity = 3;
  typedef R type (A1, A2, A3);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type;
};

template<class R, class A1, class A2, class A3>
struct callback_funtype_trait<R (A1, A2, A3)> : callback_funtype_trait3<R, A1, A2, A3>
{
};

template<class signature>
struct callback_get_impl<3, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback3<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type> type;
};
   
template<class R, class A1, class A2, class A3, class A4>
class callback4
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3, A4);

  void *obj;
  R (object::*meth)(A1, A2, A3, A4);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4)>(meth)))
          (a1, a2, a3, a4);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback4 (O1 *object, R (O2::*method)(A1, A2, A3, A4))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3, A4)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3, A4 a4) const
    {
      return prxy->call (obj, meth, a1, a2, a3, a4);
    }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4) const
    {
      return call (a1, a2, a3, a4);
    }
};

template<class R, class A1, class A2, class A3, class A4>
struct callback_funtype_trait4
{
  static const int arity = 4;
  typedef R type (A1, A2, A3, A4);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type; typedef A4 arg4_type;
};

template<class R, class A1, class A2, class A3, class A4>
struct callback_funtype_trait<R (A1, A2, A3, A4)> : callback_funtype_trait4<R, A1, A2, A3, A4>
{
};

template<class signature>
struct callback_get_impl<4, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback4<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type, typename T::arg4_type> type;
};
   
template<class R, class A1, class A2, class A3, class A4, class A5>
class callback5
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3, A4, A5);

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5)>(meth)))
          (a1, a2, a3, a4, a5);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback5 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3, A4, A5)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
    {
      return prxy->call (obj, meth, a1, a2, a3, a4, a5);
    }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) const
    {
      return call (a1, a2, a3, a4, a5);
    }
};

template<class R, class A1, class A2, class A3, class A4, class A5>
struct callback_funtype_trait5
{
  static const int arity = 5;
  typedef R type (A1, A2, A3, A4, A5);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type; typedef A4 arg4_type; typedef A5 arg5_type;
};

template<class R, class A1, class A2, class A3, class A4, class A5>
struct callback_funtype_trait<R (A1, A2, A3, A4, A5)> : callback_funtype_trait5<R, A1, A2, A3, A4, A5>
{
};

template<class signature>
struct callback_get_impl<5, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback5<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type, typename T::arg4_type, typename T::arg5_type> type;
};
   
template<class R, class A1, class A2, class A3, class A4, class A5, class A6>
class callback6
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3, A4, A5, A6);

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5, A6);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5, A6)>(meth)))
          (a1, a2, a3, a4, a5, a6);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback6 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5, A6))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3, A4, A5, A6)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
    {
      return prxy->call (obj, meth, a1, a2, a3, a4, a5, a6);
    }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) const
    {
      return call (a1, a2, a3, a4, a5, a6);
    }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6>
struct callback_funtype_trait6
{
  static const int arity = 6;
  typedef R type (A1, A2, A3, A4, A5, A6);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type; typedef A4 arg4_type; typedef A5 arg5_type; typedef A6 arg6_type;
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6>
struct callback_funtype_trait<R (A1, A2, A3, A4, A5, A6)> : callback_funtype_trait6<R, A1, A2, A3, A4, A5, A6>
{
};

template<class signature>
struct callback_get_impl<6, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback6<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type, typename T::arg4_type, typename T::arg5_type, typename T::arg6_type> type;
};
   
template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
class callback7
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3, A4, A5, A6, A7);

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5, A6, A7);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5, A6, A7)>(meth)))
          (a1, a2, a3, a4, a5, a6, a7);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback7 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5, A6, A7))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3, A4, A5, A6, A7)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
    {
      return prxy->call (obj, meth, a1, a2, a3, a4, a5, a6, a7);
    }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) const
    {
      return call (a1, a2, a3, a4, a5, a6, a7);
    }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct callback_funtype_trait7
{
  static const int arity = 7;
  typedef R type (A1, A2, A3, A4, A5, A6, A7);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type; typedef A4 arg4_type; typedef A5 arg5_type; typedef A6 arg6_type; typedef A7 arg7_type;
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
struct callback_funtype_trait<R (A1, A2, A3, A4, A5, A6, A7)> : callback_funtype_trait7<R, A1, A2, A3, A4, A5, A6, A7>
{
};

template<class signature>
struct callback_get_impl<7, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback7<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type, typename T::arg4_type, typename T::arg5_type, typename T::arg6_type, typename T::arg7_type> type;
};
   
template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
class callback8
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3, A4, A5, A6, A7, A8);

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5, A6, A7, A8)>(meth)))
          (a1, a2, a3, a4, a5, a6, a7, a8);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback8 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5, A6, A7, A8))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3, A4, A5, A6, A7, A8)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
    {
      return prxy->call (obj, meth, a1, a2, a3, a4, a5, a6, a7, a8);
    }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8) const
    {
      return call (a1, a2, a3, a4, a5, a6, a7, a8);
    }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct callback_funtype_trait8
{
  static const int arity = 8;
  typedef R type (A1, A2, A3, A4, A5, A6, A7, A8);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type; typedef A4 arg4_type; typedef A5 arg5_type; typedef A6 arg6_type; typedef A7 arg7_type; typedef A8 arg8_type;
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
struct callback_funtype_trait<R (A1, A2, A3, A4, A5, A6, A7, A8)> : callback_funtype_trait8<R, A1, A2, A3, A4, A5, A6, A7, A8>
{
};

template<class signature>
struct callback_get_impl<8, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback8<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type, typename T::arg4_type, typename T::arg5_type, typename T::arg6_type, typename T::arg7_type, typename T::arg8_type> type;
};
   
template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
class callback9
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3, A4, A5, A6, A7, A8, A9);

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8, A9);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8, A9), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8, A9), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9)>(meth)))
          (a1, a2, a3, a4, a5, a6, a7, a8, a9);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback9 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const
    {
      return prxy->call (obj, meth, a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9) const
    {
      return call (a1, a2, a3, a4, a5, a6, a7, a8, a9);
    }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct callback_funtype_trait9
{
  static const int arity = 9;
  typedef R type (A1, A2, A3, A4, A5, A6, A7, A8, A9);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type; typedef A4 arg4_type; typedef A5 arg5_type; typedef A6 arg6_type; typedef A7 arg7_type; typedef A8 arg8_type; typedef A9 arg9_type;
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
struct callback_funtype_trait<R (A1, A2, A3, A4, A5, A6, A7, A8, A9)> : callback_funtype_trait9<R, A1, A2, A3, A4, A5, A6, A7, A8, A9>
{
};

template<class signature>
struct callback_get_impl<9, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback9<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type, typename T::arg4_type, typename T::arg5_type, typename T::arg6_type, typename T::arg7_type, typename T::arg8_type, typename T::arg9_type> type;
};
   
template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
class callback10
{
  struct object { };

  typedef R (object::*ptr_type)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>(meth)))
          (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  explicit callback10 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10))
    {
      static proxy<O1,O2> p;
      obj  = reinterpret_cast<void *>(object);
      meth = reinterpret_cast<R (object::*)(A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)>(method);
      prxy = &p;
    }

  R call(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const
    {
      return prxy->call (obj, meth, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }

  R operator ()(A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7, A8 a8, A9 a9, A10 a10) const
    {
      return call (a1, a2, a3, a4, a5, a6, a7, a8, a9, a10);
    }
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct callback_funtype_trait10
{
  static const int arity = 10;
  typedef R type (A1, A2, A3, A4, A5, A6, A7, A8, A9, A10);
  typedef R result_type;
  typedef A1 arg1_type; typedef A2 arg2_type; typedef A3 arg3_type; typedef A4 arg4_type; typedef A5 arg5_type; typedef A6 arg6_type; typedef A7 arg7_type; typedef A8 arg8_type; typedef A9 arg9_type; typedef A10 arg10_type;
};

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
struct callback_funtype_trait<R (A1, A2, A3, A4, A5, A6, A7, A8, A9, A10)> : callback_funtype_trait10<R, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>
{
};

template<class signature>
struct callback_get_impl<10, signature>
{
  typedef callback_funtype_trait<signature> T;
  typedef callback10<typename T::result_type, typename T::arg1_type, typename T::arg2_type, typename T::arg3_type, typename T::arg4_type, typename T::arg5_type, typename T::arg6_type, typename T::arg7_type, typename T::arg8_type, typename T::arg9_type, typename T::arg10_type> type;
};
   

template<class signature>
struct callback : callback_get_impl<callback_funtype_trait<signature>::arity, signature>::type
{
  typedef typename callback_get_impl<callback_funtype_trait<signature>::arity, signature>::type base_type;

  template<class O, class M>
  explicit callback (O object, M method)
    : base_type (object, method)
    {
    }
};

#endif
