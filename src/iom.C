/*
    iom.C -- generic I/O multiplexor
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

#include "../config.h"

#include <cstdio>

#include <sys/select.h>
#include <sys/time.h>

#include "iom.h"

tstamp NOW;
bool iom_valid;
io_manager iom;

void time_watcher::trigger ()
{
  call (*this);

  iom.reg (this);
}

time_watcher::~time_watcher ()
{
  if (iom_valid)
    iom.unreg (this);
}

io_watcher::~io_watcher ()
{
  if (iom_valid)
    iom.unreg (this);
}

void io_manager::reg (io_watcher *w)
{
  if (find (iow.begin (), iow.end (), w) == iow.end ())
    iow.push_back (w);
}

void io_manager::unreg (io_watcher *w)
{
  iow.erase (find (iow.begin (), iow.end (), w));
}

void io_manager::reg (time_watcher *w)
{
  if (find (tw.begin (), tw.end (), w) == tw.end ())
    tw.push_back (w);
}

void io_manager::unreg (time_watcher *w)
{
  tw.erase (find (tw.begin (), tw.end (), w));
}

inline void set_now (void)
{
  struct timeval tv;

  gettimeofday (&tv, 0);

  NOW = (tstamp)tv.tv_sec + (tstamp)tv.tv_usec / 1000000;
}

void io_manager::loop ()
{
  set_now ();

  for (;;)
    {
      time_watcher *w;

      for (;;)
        {
          w = tw[0];

          for (time_watcher **i = tw.begin (); i != tw.end (); ++i)
            if ((*i)->at < w->at)
              w = *i;

          if (w->at > NOW)
            break;

          // call it
          w->call (*w);

          // re-add it if necessary
          if (w->at >= 0)
            reg (w);
        }

      struct timeval to;
      double diff = w->at - NOW;
      to.tv_sec  = (int)diff;
      to.tv_usec = (int)((diff - to.tv_sec) * 1000000);

      fd_set rfd, wfd;

      FD_ZERO (&rfd);
      FD_ZERO (&wfd);

      int fds = 0;

      for (io_watcher **w = iow.begin (); w < iow.end (); ++w)
        {
          if ((*w)->events & EVENT_READ ) FD_SET ((*w)->fd, &rfd);
          if ((*w)->events & EVENT_WRITE) FD_SET ((*w)->fd, &wfd);

          if ((*w)->fd > fds) fds = (*w)->fd;
        }

      fds = select (fds + 1, &rfd, &wfd, 0, &to);

      set_now ();

      if (fds > 0)
        for (io_watcher **w = iow.begin (); w < iow.end (); ++w)
          {
            short revents = (*w)->events;

            if (!FD_ISSET ((*w)->fd, &rfd)) revents &= ~EVENT_READ;
            if (!FD_ISSET ((*w)->fd, &wfd)) revents &= ~EVENT_WRITE;

            if (revents)
              (*w)->call (**w, revents);
          }
    }
}

void io_manager::idle_cb (time_watcher &w)
{
  w.at = NOW + 1000000000;
}

io_manager::io_manager ()
{
  set_now ();

  iom_valid = true;

  idle = new time_watcher (this, &io_manager::idle_cb);
  idle->start (0);
}

io_manager::~io_manager ()
{
  iom_valid = false;
}

