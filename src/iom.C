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

#include "iom.h"

#include <cstdio>
#include <cstdlib>
#include <cerrno>

#include <sys/time.h>

#include <assert.h>

#if 1 // older unices need these includes for select (2)
# include <unistd.h>
# include <sys/types.h>
# include <time.h>
#endif

// for IOM_SIG
#if IOM_SIG
# include <signal.h>
# include <fcntl.h>
#endif

// if the BSDs would at least be marginally POSIX-compatible.. *sigh*
// until that happens, sys/select.h must come last
#include <sys/select.h>

// TSTAMP_MAX must still fit into a positive struct timeval
#define TSTAMP_MAX (double)(1UL<<31)

#define TIMEVAL timeval
#define TV_FRAC tv_usec
#define TV_MULT 1000000L

#if IOM_IO
static io_manager_vec<io_watcher>    iow;
#endif
#if IOM_CHECK
static io_manager_vec<check_watcher> cw;
#endif
#if IOM_TIME
static io_manager_vec<time_watcher>  tw;
#endif
#if IOM_IDLE
static io_manager_vec<idle_watcher>  iw;
#endif
#if IOM_SIG
static int sigpipe[2]; // signal signalling pipe
static sigset_t sigs;
struct sig_vec : io_manager_vec<sig_watcher> {
  int pending;
  sig_vec ()
    : pending (false)
    { }
};
static vector<sig_vec *> sw;
#endif

// this is a dummy time watcher to ensure that the first
// time watcher is _always_ valid, this gets rid of a lot
// of null-pointer-checks
// (must come _before_ iom is being defined)
static struct tw0 : time_watcher
  {
    void cb (time_watcher &w)
    {
      // should never get called
      // reached end-of-time, or tstamp has a bogus definition,
      // or compiler initialisation order broken, or something else :)
      abort ();
    }

    tw0 ()
      : time_watcher (this, &tw0::cb)
      { }
  } tw0;

tstamp NOW;

#if IOM_TIME
inline void set_now (void)
{
  struct timeval tv;

  gettimeofday (&tv, 0);
  NOW = (tstamp)tv.tv_sec + (tstamp)tv.tv_usec / 1000000.;
}
#endif

static bool iom_valid;

// used for initialisation only
static struct init {
  init ()
  {
#if IOM_SIG
    sigemptyset (&sigs);

    if (pipe (sigpipe))
      {
        perror ("io_manager: unable to create signal pipe, aborting.");
        abort ();
      }

    fcntl (sigpipe[0], F_SETFL, O_NONBLOCK);
    fcntl (sigpipe[1], F_SETFL, O_NONBLOCK);
#endif

    iom_valid = true;

#if IOM_TIME
    set_now ();

    tw0.start (TSTAMP_MAX);
#endif
  }

  static void required ();
} init;

void
init::required ()
{
  if (!iom_valid)
    {
      write (2, "io_manager: early registration attempt, aborting.\n",
                sizeof ("io_manager: early registration attempt, aborting.\n") - 1);
      abort ();
    }
}

template<class watcher>
void io_manager::reg (watcher &w, io_manager_vec<watcher> &queue)
{
  init::required ();

  if (!w.active)
    {
#if IOM_CHECK
      queue.activity = true;
#endif
      queue.push_back (&w);
      w.active = queue.size ();
    }
}

template<class watcher>
void io_manager::unreg (watcher &w, io_manager_vec<watcher> &queue)
{
  if (!iom_valid)
    return;

  if (w.active)
    {
      queue [w.active - 1] = 0;
      w.active = 0;
    }
}

#if IOM_TIME
void time_watcher::trigger ()
{
  call (*this);
  io_manager::reg (*this);
}

void io_manager::reg   (time_watcher &w)  { io_manager::reg   (w, tw); }
void io_manager::unreg (time_watcher &w)  { io_manager::unreg (w, tw); }
#endif

#if IOM_IO
void io_manager::reg   (io_watcher &w)    { io_manager::reg   (w, iow); }
void io_manager::unreg (io_watcher &w)    { io_manager::unreg (w, iow); }
#endif

#if IOM_CHECK
void io_manager::reg   (check_watcher &w) { io_manager::reg   (w, cw); }
void io_manager::unreg (check_watcher &w) { io_manager::unreg (w, cw); }
#endif

#if IOM_IDLE
void io_manager::reg   (idle_watcher &w)  { io_manager::reg   (w, iw); }
void io_manager::unreg (idle_watcher &w)  { io_manager::unreg (w, iw); }
#endif

#if IOM_SIG
static void
sighandler (int signum)
{
  sw [signum - 1]->pending = true;

  // we use a pipe for signal notifications, as most current
  // OSes (Linux...) do not implement pselect correctly. ugh.
  char ch = signum; // actual content not used
  write (sigpipe[1], &ch, 1);
}

void io_manager::reg (sig_watcher &w)
{
  assert (0 < w.signum);

  sw.reserve (w.signum);

  while (sw.size () < w.signum) // pathetic
    sw.push_back (0);

  sig_vec *&sv = sw[w.signum - 1];

  if (!sv)
    {
      sv = new sig_vec;

      sigaddset (&sigs, w.signum);
      sigprocmask (SIG_BLOCK, &sigs, NULL);

      struct sigaction sa;
      sa.sa_handler = sighandler;
      sigfillset (&sa.sa_mask);
      sa.sa_flags = SA_RESTART;

      if (sigaction (w.signum, &sa, 0))
        {
          perror ("io_manager: error while installing signal handler, ignoring.");
          abort ();
        }

    }

  io_manager::reg (w, *sv);
}

void io_manager::unreg (sig_watcher &w)
{
  if (!w.active)
    return;

  assert (0 < w.signum && w.signum <= sw.size ());
  
  io_manager::unreg (w, *sw[w.signum - 1]);
}

void sig_watcher::start (int signum)
{
  stop ();
  this->signum = signum;
  io_manager::reg (*this);
}
#endif

void io_manager::loop ()
{
  init::required ();

#if IOM_TIME
  set_now ();
#endif

  for (;;)
    {
      struct TIMEVAL *to = 0;
      struct TIMEVAL tval;

#if IOM_IDLE
      if (iw.size ())
        {
          tval.tv_sec  = 0;
          tval.TV_FRAC = 0;
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
                      tval.TV_FRAC = (int) ((diff - tval.tv_sec) * TV_MULT);
                      to = &tval;
                    }
                  break;
                }
              else
                {
                  unreg (*next);
                  next->call (*next);
                }
            }
#endif
        }

#if IOM_CHECK
      tw.activity = false;

      for (int i = cw.size (); i--; )
        if (!cw[i])
          cw.erase_unordered (i);
        else
          cw[i]->call (*cw[i]);

      if (tw.activity)
        {
          tval.tv_sec  = 0;
          tval.TV_FRAC = 0;
          to = &tval;
        }
#endif

#if IOM_IO || IOM_SIG
      fd_set rfd, wfd;

      FD_ZERO (&rfd);
      FD_ZERO (&wfd);

      int fds = 0;

# if IOM_IO
      for (io_manager_vec<io_watcher>::iterator i = iow.end (); i-- > iow.begin (); )
        if (*i)
          {
            if ((*i)->events & EVENT_READ ) FD_SET ((*i)->fd, &rfd);
            if ((*i)->events & EVENT_WRITE) FD_SET ((*i)->fd, &wfd);

            if ((*i)->fd >= fds) fds = (*i)->fd + 1;
          }
# endif

      if (!to && !fds) //TODO: also check idle_watchers and check_watchers
        break; // no events

# if IOM_SIG
      FD_SET (sigpipe[0], &rfd);
      if (sigpipe[0] >= fds) fds = sigpipe[0] + 1;
# endif

# if IOM_SIG
      // there is no race, as we use a pipe for signals, so select
      // will return if a signal is caught.
      sigprocmask (SIG_UNBLOCK, &sigs, NULL);
# endif
      fds = select (fds, &rfd, &wfd, NULL, to);
# if IOM_SIG
      sigprocmask (SIG_BLOCK, &sigs, NULL);
# endif

# if IOM_TIME
      set_now ();
# endif

      if (fds > 0)
        {
# if IOM_SIG
          if (FD_ISSET (sigpipe[0], &rfd))
            {
              char ch;

              while (read (sigpipe[0], &ch, 1) > 0)
                ;

              for (sig_vec **svp = sw.end (); svp-- > sw.begin (); )
                if (*svp && (*svp)->pending)
                  {
                    sig_vec &sv = **svp;
                    for (int i = sv.size (); i--; )
                      if (!sv[i])
                        sv.erase_unordered (i);
                      else
                        sv[i]->call (*sv[i]);

                    sv.pending = false;
                  }
            }
# endif

# if IOM_IO
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
#endif
        }
      else if (fds < 0 && errno != EINTR)
        {
          perror ("io_manager: fatal error while waiting for I/O or time event, aborting.");
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

