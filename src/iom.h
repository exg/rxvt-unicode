/*
    iom.h -- generic I/O multiplexor
    Copyright (C) 2003 Marc Lehmann <pcg@goof.com>
 
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
 
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc. 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef VPE_IOM_H__
#define VPE_IOM_H__

#include <cassert>

#include "rxvtvec.h"
#include "callback.h"

typedef double tstamp;

extern tstamp NOW;

struct io_watcher;
struct time_watcher;

class io_manager {
  simplevec<io_watcher *> iow;
  simplevec<time_watcher *> tw; // actually a heap

  void idle_cb (time_watcher &w); time_watcher *idle;
public:
  // register a watcher
  void reg (io_watcher *w);
  void unreg (io_watcher *w);
  void reg (time_watcher *w);
  void unreg (time_watcher *w);
  
  void loop ();

  io_manager ();
  ~io_manager ();
};

extern io_manager iom; // a singleton, together with it's construction/destruction problems.

enum { EVENT_READ = 1, EVENT_WRITE = 2 };

struct io_watcher : callback2<void, io_watcher &, short> {
  int fd;
  short events;

  template<class O1, class O2>
  io_watcher (O1 *object, void (O2::*method)(io_watcher &, short))
    : callback2<void, io_watcher &, short>(object,method)
    { }

  ~io_watcher ();

  void set(int fd_, short events_) { fd = fd_; events = events_; }

  void set(short events_) { set (fd, events_); }
  void start (int fd_, short events_) { set (fd_, events_); iom.reg (this); }
  void stop () { iom.unreg (this); }
};

#define TSTAMP_CANCEL -1.

struct time_watcher : callback1<void, time_watcher &> {
  tstamp at;

  template<class O1, class O2>
  time_watcher (O1 *object, void (O2::*method)(time_watcher &))
    : callback1<void, time_watcher &>(object,method)
    { }

  ~time_watcher ();

  void trigger ();

  void set (tstamp when) { at = when; }
  void operator ()() { trigger (); }
  void start () { iom.reg (this); }
  void start (tstamp when) { set (when); iom.reg (this); }
  void stop () { iom.unreg (this); }

  void reset (tstamp when = TSTAMP_CANCEL)
    {
      stop ();
      at = when;
    }
};

#endif

