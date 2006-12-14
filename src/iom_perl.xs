#############################################################################
# IOM_CLASS constants
#############################################################################

BOOT:
{
  HV *stash     = gv_stashpv ("IOM_CLASS", 1);
  SV *baseclass = newSVpv ("IOM_CLASS::watcher", 0);

  static const struct {
    const char *name;
    IV iv;
  } *civ, const_iv[] = {
#   define const_iv(name) { # name, (IV)name }
    const_iv (EVENT_NONE),
    const_iv (EVENT_READ),
    const_iv (EVENT_WRITE),
  };

  for (civ = const_iv + sizeof (const_iv) / sizeof (const_iv [0]); civ-- > const_iv; )
    newCONSTSUB (stash, (char *)civ->name, newSViv (civ->iv));

  /* slightly dirty to put the same scalar into all those arrays, but */
  /* we do not expect users to modify them anyways */
  av_push (get_av ("IOM_CLASS" "::timer::ISA", 1), SvREFCNT_inc (baseclass));
  av_push (get_av ("IOM_CLASS"   "::iow::ISA", 1), SvREFCNT_inc (baseclass));
  av_push (get_av ("IOM_CLASS"    "::pw::ISA", 1), SvREFCNT_inc (baseclass));
  av_push (get_av ("IOM_CLASS"    "::iw::ISA", 1), SvREFCNT_inc (baseclass));

  SvREFCNT_dec (baseclass);
}

#############################################################################
# IOM_CLASS::watcher
#############################################################################

MODULE = IOM_MODULE             PACKAGE = IOM_CLASS::watcher

CHAINED
perl_watcher::cb (SV *cb)
	CODE:
        THIS->cb (cb);
        OUTPUT:
        RETVAL

#############################################################################
# IOM_CLASS::timer
#############################################################################

MODULE = IOM_MODULE             PACKAGE = IOM_CLASS::timer

SV *
timer::new ()
	CODE:
        timer *w =  new timer;
        w->start (NOW);
        RETVAL = newSVptr ((void *)(perl_watcher *)w, "IOM_CLASS::timer");
        w->self = (HV *)SvRV (RETVAL);
        OUTPUT:
        RETVAL

NV
timer::at ()
	CODE:
        RETVAL = THIS->at;
        OUTPUT:
        RETVAL

CHAINED
timer::interval (NV interval)
	CODE:
        THIS->interval = interval;
        OUTPUT:
        RETVAL

CHAINED
timer::set (NV tstamp)
	CODE:
        THIS->set (tstamp);
        OUTPUT:
        RETVAL

CHAINED
timer::start (NV tstamp = THIS->at)
	CODE:
        THIS->start (tstamp);
        OUTPUT:
        RETVAL

CHAINED
timer::after (NV delay)
	CODE:
        THIS->start (NOW + delay);
        OUTPUT:
        RETVAL

CHAINED
timer::stop ()
	CODE:
        THIS->stop ();
        OUTPUT:
        RETVAL

void
timer::DESTROY ()

#############################################################################
# IOM_CLASS::iow
#############################################################################

MODULE = IOM_MODULE             PACKAGE = IOM_CLASS::iow

SV *
iow::new ()
	CODE:
        iow *w =  new iow;
        RETVAL = newSVptr ((void *)(perl_watcher *)w, "IOM_CLASS::iow");
        w->self = (HV *)SvRV (RETVAL);
        OUTPUT:
        RETVAL

CHAINED
iow::fd (int fd)
	CODE:
        THIS->fd = fd;
        OUTPUT:
        RETVAL

CHAINED
iow::events (short events)
	CODE:
        THIS->events = events;
        OUTPUT:
        RETVAL

CHAINED
iow::start ()
	CODE:
        THIS->start ();
        OUTPUT:
        RETVAL

CHAINED
iow::stop ()
	CODE:
        THIS->stop ();
        OUTPUT:
        RETVAL

void
iow::DESTROY ()

#############################################################################
# IOM_CLASS::iw
#############################################################################

MODULE = IOM_MODULE             PACKAGE = IOM_CLASS::iw

SV *
iw::new ()
	CODE:
        iw *w =  new iw;
        RETVAL = newSVptr ((void *)(perl_watcher *)w, "IOM_CLASS::iw");
        w->self = (HV *)SvRV (RETVAL);
        OUTPUT:
        RETVAL

CHAINED
iw::start ()
	CODE:
        THIS->start ();
        OUTPUT:
        RETVAL

CHAINED
iw::stop ()
	CODE:
        THIS->stop ();
        OUTPUT:
        RETVAL

void
iw::DESTROY ()

#############################################################################
# IOM_CLASS::pw
#############################################################################

MODULE = IOM_MODULE             PACKAGE = IOM_CLASS::pw

SV *
pw::new ()
	CODE:
        pw *w =  new pw;
        RETVAL = newSVptr ((void *)(perl_watcher *)w, "IOM_CLASS::pw");
        w->self = (HV *)SvRV (RETVAL);
        OUTPUT:
        RETVAL

CHAINED
pw::start (int pid)
	CODE:
        THIS->start (pid);
        OUTPUT:
        RETVAL

CHAINED
pw::stop ()
	CODE:
        THIS->stop ();
        OUTPUT:
        RETVAL

void
pw::DESTROY ()


