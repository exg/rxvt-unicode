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

template<class watcher>
void io_manager::reg (watcher *w, simplevec<watcher *> &queue)
{
  if (find (queue.begin (), queue.end (), w) == queue.end ())
    queue.push_back (w);
}

template<class watcher>
void io_manager::unreg (watcher *w, simplevec<watcher *> &queue)
{
  queue.erase (find (queue.begin (), queue.end (), w));
}

#if IOM_IO
io_watcher::~io_watcher ()
{
  if (iom_valid)
    iom.unreg (this);
}

void io_manager::reg (io_watcher *w)
{
  reg (w, iow);
}

void io_manager::unreg (io_watcher *w)
{
  unreg (w, iow);
}

#endif

#if IOM_TIME
void time_watcher::trigger ()
{
  call (*this);

  iom.reg (this);
}

time_watcher::~time_watcher ()
{
  if (iom_valid)
    iom.unreg (this);

  at = TSTAMP_CANCEL;
}

void io_manager::reg (time_watcher *w)
{
  reg (w, tw);
}

void io_manager::unreg (time_watcher *w)
{
  unreg (w, tw);
}
#endif

#if IOM_CHECK
check_watcher::~check_watcher ()
{
  if (iom_valid)
    iom.unreg (this);
}

void io_manager::reg (check_watcher *w)
{
  reg (w, cw);
}

void io_manager::unreg (check_watcher *w)
{
  unreg (w, cw);
}
#endif

#if IOM_TIME
inline void set_now (void)
{
  struct timeval tv;

  gettimeofday (&tv, 0);

  NOW = (tstamp)tv.tv_sec + (tstamp)tv.tv_usec / 1000000;
#endif
}

void io_manager::loop ()
{
#if IOM_TIME
  set_now ();
#endif

  for (;;)
    {
      struct timeval *to = 0;

#if IOM_TIME
      struct timeval tval;
      time_watcher *w;

      for (;tw.size ();)
        {
          w = tw[0];

          for (time_watcher **i = tw.begin (); i < tw.end (); ++i)
            if ((*i)->at < w->at)
              w = *i;

          if (w->at > NOW)
            {
              double diff = w->at - NOW;
              tval.tv_sec  = (int)diff;
              tval.tv_usec = (int)((diff - tval.tv_sec) * 1000000);
              to = &tval;
              break;
            }
          else if (w->at >= 0)
            w->call (*w);
          else
            unreg (w);
        }
#endif

#if IOM_CHECK
      for (int i = 0; i < cw.size (); ++i)
        cw[i]->call (*cw[i]);
#endif

#if IOM_IO
      fd_set rfd, wfd;

      FD_ZERO (&rfd);
      FD_ZERO (&wfd);

      int fds = 0;

      for (io_watcher **w = iow.begin (); w < iow.end (); ++w)
        {
          if ((*w)->events & EVENT_READ ) FD_SET ((*w)->fd, &rfd);
          if ((*w)->events & EVENT_WRITE) FD_SET ((*w)->fd, &wfd);

          if ((*w)->fd >= fds) fds = (*w)->fd + 1;
        }

      if (!to && !fds)
        break; // no events

      fds = select (fds, &rfd, &wfd, 0, to);
# if IOM_TIME
      set_now ();
# endif

      if (fds > 0)
        for (int i = 0; i < iow.size (); ++i)
          {
            io_watcher *w = iow[i];

            short revents = w->events;

            if (!FD_ISSET (w->fd, &rfd)) revents &= ~EVENT_READ;
            if (!FD_ISSET (w->fd, &wfd)) revents &= ~EVENT_WRITE;

            if (revents)
              w->call (*w, revents);
          }
#elif IOM_TIME
      if (!to)
        break;

      select (0, 0, 0, 0, &to);
      set_now ();
#else
      break;
#endif
    }
}

io_manager::io_manager ()
{
#if IOM_TIME
  set_now ();
#endif

  iom_valid = true;
}

io_manager::~io_manager ()
{
  iom_valid = false;
}

