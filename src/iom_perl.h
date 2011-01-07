#define IOM_CLASS "urxvt"
typedef int IOM_CHAINED;

static SV *
iom_new_ref (HV *hv, const char *klass)
{
  return sv_bless (newRV ((SV *)hv), gv_stashpv (klass, 1));
}

/////////////////////////////////////////////////////////////////////////////

#define SvWATCHER(sv) (perl_watcher *)SvPTR (sv, IOM_CLASS "::watcher")

struct perl_watcher
{
  SV *cbsv;
  HV *self;

  perl_watcher ()
  : cbsv (0)
  {
  }

  ~perl_watcher ()
  {
    SvREFCNT_dec (cbsv);
  }

  void cb (SV *cb)
  {
    SvREFCNT_dec (cbsv);
    cbsv = newSVsv (cb);
  }

  void invoke (const char *type, SV *self, int arg = -1);
};

void
perl_watcher::invoke (const char *type, SV *self, int arg)
{
  dSP;

  ENTER;
  SAVETMPS;

  PUSHMARK (SP);

  XPUSHs (sv_2mortal (self));

  if (arg >= 0)
    XPUSHs (sv_2mortal (newSViv (arg)));

  PUTBACK;
  call_sv (cbsv, G_VOID | G_EVAL | G_DISCARD);
  SPAGAIN;

  PUTBACK;
  FREETMPS;
  LEAVE;

  if (SvTRUE (ERRSV))
    rxvt_warn ("%s callback evaluation error: %s", type, SvPV_nolen (ERRSV));
}

#define newSVtimer(timer) iom_new_ref ((timer)->self, IOM_CLASS "::timer")
#define SvTIMER(sv) (timer *)(perl_watcher *)SvPTR ((sv), IOM_CLASS "::timer")

struct timer : perl_watcher, ev::timer
{
  timer ()
  {
    set<timer, &timer::execute> (this);
  }

  void execute (ev::timer &w, int revents)
  {
    invoke (IOM_CLASS "::timer", newSVtimer (this));
  }
};

#define newSViow(iow) iom_new_ref ((iow)->self, IOM_CLASS "::iow")
#define SvIOW(sv) (iow *)(perl_watcher *)SvPTR ((sv), IOM_CLASS "::iow")

struct iow : perl_watcher, ev::io
{
  iow ()
  {
    set<iow, &iow::execute> (this);
  }

  void execute (ev::io &w, int revents)
  {
    invoke (IOM_CLASS "::iow", newSViow (this), revents);
  }
};

#define newSViw(iw) iom_new_ref ((iw)->self, IOM_CLASS "::iw")
#define SvIW(sv) (iw *)(perl_watcher *)SvPTR ((sv), IOM_CLASS "::iw")

struct iw : perl_watcher, ev::idle
{
  iw ()
  {
    set<iw, &iw::execute> (this);
  }

  void execute (ev::idle &w, int revents)
  {
    invoke (IOM_CLASS "::iw", newSViw (this));
  }
};

#define newSVpw(pw) iom_new_ref ((pw)->self, IOM_CLASS "::pw")
#define SvPW(sv) (pw *)(perl_watcher *)SvPTR ((sv), IOM_CLASS "::pw")

struct pw : perl_watcher, ev::child
{
  pw ()
  {
    set<pw, &pw::execute> (this);
  }

  void execute (ev::child &w, int revents)
  {
    invoke (IOM_CLASS "::pw", newSVpw (this), w.rstatus);
  }
};

