#############################################################################
# IOM_CLASS constants
#############################################################################

BOOT:
{
  {
    HV *stash     = gv_stashpv ("IOM_CLASS", 1);
    SV *baseclass = newSVpv ("IOM_CLASS::watcher", 0);

    static const struct {
      const char *name;
      IV iv;
    } *civ, iom_const_iv[] = {
#   define iom_const_iv(name) { # name, (IV)name }
      iom_const_iv (EV_NONE),
      iom_const_iv (EV_READ),
      iom_const_iv (EV_WRITE),
#   undef iom_const
    };

    for (civ = iom_const_iv + sizeof (iom_const_iv) / sizeof (iom_const_iv [0]); civ > iom_const_iv; civ--)
      newCONSTSUB (stash, (char *)civ[-1].name, newSViv (civ[-1].iv));

    /* slightly dirty to put the same scalar into all those arrays, but */
    /* we do not expect users to modify them anyways */
    av_push (get_av ("IOM_CLASS" "::timer::ISA", 1), SvREFCNT_inc (baseclass));
    av_push (get_av ("IOM_CLASS"   "::iow::ISA", 1), SvREFCNT_inc (baseclass));
    av_push (get_av ("IOM_CLASS"    "::pw::ISA", 1), SvREFCNT_inc (baseclass));
    av_push (get_av ("IOM_CLASS"    "::iw::ISA", 1), SvREFCNT_inc (baseclass));

    SvREFCNT_dec (baseclass);
  }
}

#############################################################################
# IOM_CLASS::watcher
#############################################################################

MODULE = IOM_MODULE             PACKAGE = IOM_CLASS::watcher

IOM_CHAINED
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
        w->start (0);
        RETVAL = newSVptr ((void *)(perl_watcher *)w, "IOM_CLASS::timer");
        w->self = (HV *)SvRV (RETVAL);
        OUTPUT:
        RETVAL

# deprecated
NV
timer::at ()
	CODE:
        RETVAL = THIS->remaining () + ev_now ();
        OUTPUT:
        RETVAL

IOM_CHAINED
timer::interval (NV repeat)
	CODE:
        THIS->repeat = repeat;
        OUTPUT:
        RETVAL

IOM_CHAINED
timer::set (NV tstamp, NV repeat = THIS->repeat)
	CODE:
        THIS->set (tstamp, repeat);
        OUTPUT:
        RETVAL

IOM_CHAINED
timer::start (NV tstamp = ev::now (), NV repeat = THIS->repeat)
	CODE:
        THIS->start (tstamp - ev::now (), repeat);
        OUTPUT:
        RETVAL

IOM_CHAINED
timer::after (NV delay, NV repeat = THIS->repeat)
	CODE:
        THIS->start (delay, repeat);
        OUTPUT:
        RETVAL

IOM_CHAINED
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

IOM_CHAINED
iow::fd (int fd)
	CODE:
        THIS->fd = fd;
        OUTPUT:
        RETVAL

IOM_CHAINED
iow::events (short events)
	CODE:
        THIS->events = events;
        OUTPUT:
        RETVAL

IOM_CHAINED
iow::start ()
	CODE:
        THIS->start ();
        OUTPUT:
        RETVAL

IOM_CHAINED
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

IOM_CHAINED
iw::start ()
	CODE:
        THIS->start ();
        OUTPUT:
        RETVAL

IOM_CHAINED
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

IOM_CHAINED
pw::start (int pid)
	CODE:
        THIS->start (pid);
        OUTPUT:
        RETVAL

IOM_CHAINED
pw::stop ()
	CODE:
        THIS->stop ();
        OUTPUT:
        RETVAL

int
pw::rpid ()
	CODE:
        RETVAL = THIS->rpid;
        OUTPUT:
        RETVAL

int
pw::rstatus ()
	CODE:
        RETVAL = THIS->rstatus;
        OUTPUT:
        RETVAL

void
pw::DESTROY ()


