// THIS IS A GENERATED FILE: RUN callback.pl to regenerate it
// THIS IS A GENERATED FILE: callback.pl is part of the GVPE
// THIS IS A GENERATED FILE: distribution.

/*
    callback.h -- C++ callback mechanism
    Copyright (C) 2003-2005 Marc Lehmann <pcg@goof.com>
 
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

template<class R>
class callback0 {
  struct object { };

  void *obj;
  R (object::*meth)();

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)()) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)())
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)()>(meth)))
          ();
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback0 (O1 *object, R (O2::*method)())
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

template<class R, class A1>
class callback1 {
  struct object { };

  void *obj;
  R (object::*meth)(A1);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1), A1 a1) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1), A1 a1)
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1)>(meth)))
          (a1);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback1 (O1 *object, R (O2::*method)(A1))
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

template<class R, class A1, class A2>
class callback2 {
  struct object { };

  void *obj;
  R (object::*meth)(A1, A2);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2), A1 a1, A2 a2) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2), A1 a1, A2 a2)
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2)>(meth)))
          (a1, a2);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback2 (O1 *object, R (O2::*method)(A1, A2))
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

template<class R, class A1, class A2, class A3>
class callback3 {
  struct object { };

  void *obj;
  R (object::*meth)(A1, A2, A3);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3), A1 a1, A2 a2, A3 a3) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3), A1 a1, A2 a2, A3 a3)
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3)>(meth)))
          (a1, a2, a3);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback3 (O1 *object, R (O2::*method)(A1, A2, A3))
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

template<class R, class A1, class A2, class A3, class A4>
class callback4 {
  struct object { };

  void *obj;
  R (object::*meth)(A1, A2, A3, A4);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4)
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4)>(meth)))
          (a1, a2, a3, a4);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback4 (O1 *object, R (O2::*method)(A1, A2, A3, A4))
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

template<class R, class A1, class A2, class A3, class A4, class A5>
class callback5 {
  struct object { };

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5)>(meth)))
          (a1, a2, a3, a4, a5);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback5 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5))
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

template<class R, class A1, class A2, class A3, class A4, class A5, class A6>
class callback6 {
  struct object { };

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5, A6);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6)
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5, A6)>(meth)))
          (a1, a2, a3, a4, a5, a6);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback6 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5, A6))
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

template<class R, class A1, class A2, class A3, class A4, class A5, class A6, class A7>
class callback7 {
  struct object { };

  void *obj;
  R (object::*meth)(A1, A2, A3, A4, A5, A6, A7);

  /* a proxy is a kind of recipe on how to call a specific class method	*/
  struct proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) = 0;
  };
  template<class O1, class O2>
  struct proxy : proxy_base {
    virtual R call (void *obj, R (object::*meth)(A1, A2, A3, A4, A5, A6, A7), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7)
      {
        return (R)((reinterpret_cast<O1 *>(obj)) ->* (reinterpret_cast<R (O2::*)(A1, A2, A3, A4, A5, A6, A7)>(meth)))
          (a1, a2, a3, a4, a5, a6, a7);
      }
  };

  proxy_base *prxy;

public:
  template<class O1, class O2>
  callback7 (O1 *object, R (O2::*method)(A1, A2, A3, A4, A5, A6, A7))
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

#endif
