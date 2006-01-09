/*
    iom.h -- generic I/O multiplexer
    Copyright (C) 2003, 2004 Marc Lehmann <gvpe@schmorp.de>
 
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

#ifndef IOM_H__
#define IOM_H__

// required:
// - a vector template like simplevec or stl's vector
// - defines for all watcher types required in your app
// edit iom_conf.h as appropriate.
#include "iom_conf.h"

#include "callback.h"

#ifndef IOM_IO
# define IOM_IO 0
#endif
#ifndef IOM_TIME
# define IOM_TIME 0
#endif
#ifndef IOM_CHECK
# define IOM_CHECK 0
#endif
#ifndef IOM_IDLE
# define IOM_IDLE 0
#endif
#ifndef IOM_SIG
# define IOM_SIG 0
#endif

typedef double tstamp;
extern tstamp NOW;

// TSTAMP_MAX must still fit into a positive struct timeval
#define TSTAMP_MAX (double)(1UL<<31)

struct watcher;
#if IOM_IO
struct io_watcher;
#endif
#if IOM_TIME
struct time_watcher;
#endif
#if IOM_CHECK
struct check_watcher;
#endif
#if IOM_IDLE
struct idle_watcher;
#endif
#if IOM_SIG
struct sig_watcher;
#endif

template<class watcher>
struct io_manager_vec : vector<watcher *> {
  void erase_unordered (unsigned int pos)
  {
    watcher *w = (*this)[this->size () - 1];
    this->pop_back ();

    if (!this->empty ())
      if (((*this)[pos] = w)) // '=' is correct!
        w->active = pos + 1;
  }
};

// only used as a namespace, and for initialisation purposes
class io_manager {
  template<class watcher>
  static void reg (watcher &w, io_manager_vec<watcher> &queue);

  template<class watcher>
  static void unreg (watcher &w, io_manager_vec<watcher> &queue);

public:
#if IOM_TIME
  // fetch time only
  static tstamp now ();

  // set NOW
  static void set_now ();
#endif

  // register a watcher
#if IOM_IO
  static void reg (io_watcher    &w); static void unreg (io_watcher    &w);
#endif
#if IOM_TIME
  static void reg (time_watcher  &w); static void unreg (time_watcher  &w);
#endif
#if IOM_CHECK
  static void reg (check_watcher &w); static void unreg (check_watcher &w);
#endif
#if IOM_IDLE
  static void reg (idle_watcher  &w); static void unreg (idle_watcher  &w);
#endif
#if IOM_SIG
  static void reg (sig_watcher   &w); static void unreg (sig_watcher   &w);
#endif
  
  static void loop ();
};

struct watcher {
  int active; /* 0 == inactive, else index into respective vector */

  watcher () : active (0) { }
};

#if IOM_IO
enum { EVENT_UNDEF = -1, EVENT_NONE = 0, EVENT_READ = 1, EVENT_WRITE = 2 };

struct io_watcher : watcher, callback2<void, io_watcher &, short> {
  int fd;
  short events;

  void set (int fd_, short events_) { fd = fd_; events = events_; }

  void set (short events_) { set (fd, events_); }
  void start () { io_manager::reg (*this); }
  void start (int fd_, short events_) { set (fd_, events_); io_manager::reg (*this); }
  void stop () { io_manager::unreg (*this); }

  template<class O1, class O2>
  io_watcher (O1 *object, void (O2::*method) (io_watcher &, short))
  : callback2<void, io_watcher &, short> (object,method)
  { }
  ~io_watcher () { stop (); }
};
#endif

#if IOM_TIME
struct time_watcher : watcher, callback1<void, time_watcher &> {
  tstamp at;

  void trigger ();

  void set (tstamp when) { at = when; }
  void operator () () { trigger (); }
  void start () { io_manager::reg (*this); }
  void start (tstamp when) { set (when); io_manager::reg (*this); }
  void stop () { io_manager::unreg (*this); }

  template<class O1, class O2>
  time_watcher (O1 *object, void (O2::*method) (time_watcher &))
  : callback1<void, time_watcher &> (object,method), at (0)
  { }
  ~time_watcher () { stop (); }
};
#endif

#if IOM_CHECK
// run before checking for new events
struct check_watcher : watcher, callback1<void, check_watcher &> {
  void start () { io_manager::reg (*this); }
  void stop () { io_manager::unreg (*this); }

  template<class O1, class O2>
  check_watcher (O1 *object, void (O2::*method) (check_watcher &))
  : callback1<void, check_watcher &> (object,method)
  { }
  ~check_watcher () { stop (); }
};
#endif

#if IOM_IDLE
// run after checking for any i/o, but before waiting
struct idle_watcher : watcher, callback1<void, idle_watcher &> {
  void start () { io_manager::reg (*this); }
  void stop () { io_manager::unreg (*this); }

  template<class O1, class O2>
  idle_watcher (O1 *object, void (O2::*method) (idle_watcher &))
    : callback1<void, idle_watcher &> (object,method)
    { }
  ~idle_watcher () { stop (); }
};
#endif

#if IOM_SIG
struct sig_watcher : watcher, callback1<void, sig_watcher &> {
  int signum;

  void start (int signum);
  void stop () { io_manager::unreg (*this); }

  template<class O1, class O2>
  sig_watcher (O1 *object, void (O2::*method) (sig_watcher &))
  : callback1<void, sig_watcher &> (object,method), signum (-1)
  { }
  ~sig_watcher () { stop (); }
};
#endif

#endif

