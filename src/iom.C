/*
    iom.C -- generic I/O multiplexor
    Copyright (C) 2003, 2004 Marc Lehmann <pcg@goof.com>
 
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

#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include <sys/select.h>
#include <sys/time.h>

#include "iom.h"

// TSTAMP_MAX must still fit into a positive struct timeval
#define TSTAMP_MAX (double)(1UL<<31)

tstamp NOW;
static bool iom_valid;
io_manager iom;

template<class watcher>
void io_manager::reg (watcher *w, io_manager_vec<watcher> &queue)
{
  if (!iom_valid)
    abort ();

  if (!w->active)
    {
      queue.push_back (w);
      w->active = queue.size ();
    }
}

template<class watcher>
void io_manager::unreg (watcher *w, io_manager_vec<watcher> &queue)
{
  if (!iom_valid)
    return;

  if (w->active)
    {
      queue [w->active - 1] = 0;
      w->active = 0;
    }
}

#if IOM_TIME
void time_watcher::trigger ()
{
  call (*this);

  iom.reg (this);
}

void io_manager::reg (time_watcher *w) { reg (w, tw); }
void io_manager::unreg (time_watcher *w) { unreg (w, tw); }
#endif

#if IOM_IO
void io_manager::reg (io_watcher *w) { reg (w, iow); } 
void io_manager::unreg (io_watcher *w) { unreg (w, iow); }
#endif

#if IOM_CHECK
void io_manager::reg (check_watcher *w) { reg (w, cw); }
void io_manager::unreg (check_watcher *w) { unreg (w, cw); }
#endif

#if IOM_IDLE
void io_manager::reg (idle_watcher *w) { reg (w, iw); }
void io_manager::unreg (idle_watcher *w) { unreg (w, iw); }
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
      struct timeval tval;

#if IOM_IDLE
      if (iw.size ())
        {
          tval.tv_sec  = 0;
          tval.tv_usec = 0;
          to = &tval;
        }
      else
#endif
        {
#if IOM_TIME
          time_watcher *next;

          for (;;)
            {
              next = tw[0]; // the first time-watcher must exist at ALL times

              for (int i = tw.size (); i--; )
                if (!tw[i])
                  tw.erase_unordered (i);
                else if (tw[i]->at < next->at)
                  next = tw[i];

              if (next->at > NOW)
                {
                  if (next != tw[0])
                    {
                      double diff = next->at - NOW;
                      tval.tv_sec  = (int)diff;
                      tval.tv_usec = (int)((diff - tval.tv_sec) * 1000000);
                      to = &tval;
                    }
                  break;
                }
              else
                {
                  unreg (next);
                  next->call (*next);
                }
            }
#endif
        }

#if IOM_CHECK
      for (int i = cw.size (); i--; )
        if (!cw[i])
          cw.erase_unordered (i);
        else
          cw[i]->call (*cw[i]);
#endif

#if IOM_IO
      fd_set rfd, wfd, efd;

      FD_ZERO (&rfd);
      FD_ZERO (&wfd);

      int fds = 0;

      for (io_manager_vec<io_watcher>::iterator i = iow.end (); i-- > iow.begin (); )
        if (*i)
          {
            if ((*i)->events & EVENT_READ ) FD_SET ((*i)->fd, &rfd);
            if ((*i)->events & EVENT_WRITE) FD_SET ((*i)->fd, &wfd);

            if ((*i)->fd >= fds) fds = (*i)->fd + 1;
          }

      if (!to && !fds) //TODO: also check idle_watchers and check_watchers
        break; // no events

      fds = select (fds, &rfd, &wfd, &efd, to);
# if IOM_TIME
      set_now ();
# endif

      if (fds > 0)
        for (int i = iow.size (); i--; )
          if (!iow[i])
            iow.erase_unordered (i);
          else
            {
              short revents = iow[i]->events;

              if (!FD_ISSET (iow[i]->fd, &rfd)) revents &= ~EVENT_READ;
              if (!FD_ISSET (iow[i]->fd, &wfd)) revents &= ~EVENT_WRITE;

              if (revents)
                iow[i]->call (*iow[i], revents);
            }
      else if (fds < 0 && errno != EINTR)
        {
          perror ("Error while waiting for I/O or time event");
          abort ();
        }
#if IOM_IDLE
      else
        for (int i = iw.size (); i--; )
          if (!iw[i])
            iw.erase_unordered (i);
          else
            iw[i]->call (*iw[i]);
#endif

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

// this is a dummy time watcher to ensure that the first
// time watcher is _always_ valid, this gets rid of a lot
// of null-pointer-checks
static struct tw0 : time_watcher {
  void cb (time_watcher &w)
  {
    // should never get called
    // reached end-of-time, or tstamp has a bogus definition :)
    abort ();
  }

  tw0()
  : time_watcher (this, &tw0::cb)
  { }
} tw0;

io_manager::io_manager ()
{
  iom_valid = true;

#if IOM_TIME
  set_now ();

  tw0.start (TSTAMP_MAX);
#endif
}

io_manager::~io_manager ()
{
  iom_valid = false;
}

