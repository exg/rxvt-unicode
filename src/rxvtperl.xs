/*----------------------------------------------------------------------*
 * File:	rxvtperl.xs
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2006 Marc Lehmann <pcg@goof.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

#define line_t perl_line_t
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>
#undef line_t

#include "../config.h"

#include <cstddef>
#include <cstdarg>

#include "rxvt.h"
#include "iom.h"
#include "rxvtutil.h"
#include "rxvtperl.h"

#include "perlxsi.c"

#if defined(HAVE_SCROLLBARS) || defined(MENUBAR)
# define GRAB_CURSOR THIS->leftptr_cursor
#else
# define GRAB_CURSOR None
#endif

#undef LINENO
#define LINENO(n) MOD (THIS->term_start + int(n), THIS->total_rows)
#undef ROW
#define ROW(n) THIS->row_buf [LINENO (n)]

/////////////////////////////////////////////////////////////////////////////

static SV *
taint (SV *sv)
{
  SvTAINT (sv);
  return sv;
}

static SV *
taint_if (SV *sv, SV *src)
{
  if (SvTAINTED (src))
    SvTAINT (sv);

  return sv;
}

static wchar_t *
sv2wcs (SV *sv)
{
  STRLEN len;
  char *str = SvPVutf8 (sv, len);
  return rxvt_utf8towcs (str, len);
}

static SV *
wcs2sv (wchar_t *wstr, int len = -1)
{
  char *str = rxvt_wcstoutf8 (wstr, len);

  SV *sv = newSVpv (str, 0);
  SvUTF8_on (sv);
  free (str);

  return sv;
}

static SV *
new_ref (HV *hv, const char *klass)
{
  return sv_bless (newRV ((SV *)hv), gv_stashpv (klass, 1));
}

//TODO: use magic
static SV *
newSVptr (void *ptr, const char *klass)
{
  HV *hv = newHV ();
  sv_magic ((SV *)hv, 0, PERL_MAGIC_ext, (char *)ptr, 0);
  return sv_bless (newRV_noinc ((SV *)hv), gv_stashpv (klass, 1));
}

static void
clearSVptr (SV *sv)
{
  if (SvROK (sv))
    sv = SvRV (sv);

  hv_clear ((HV *)sv);
  sv_unmagic (sv, PERL_MAGIC_ext);
}

static long
SvPTR (SV *sv, const char *klass)
{
  if (!sv_derived_from (sv, klass))
    croak ("object of type %s expected", klass);

  MAGIC *mg = mg_find (SvRV (sv), PERL_MAGIC_ext);

  if (!mg)
    croak ("perl code used %s object, but C++ object is already destroyed, caught", klass);

  return (long)mg->mg_ptr;
}

#define newSVterm(term) SvREFCNT_inc ((SV *)term->perl.self)
#define SvTERM(sv) (rxvt_term *)SvPTR (sv, "urxvt::term")

/////////////////////////////////////////////////////////////////////////////

struct perl_watcher
{
  SV *cbsv;
  HV *self;

  perl_watcher ()
  : cbsv (newSV (0))
  {
  }

  ~perl_watcher ()
  {
    SvREFCNT_dec (cbsv);
  }

  void cb (SV *cb)
  {
    sv_setsv (cbsv, cb);
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

#define newSVtimer(timer) new_ref (timer->self, "urxvt::timer")
#define SvTIMER(sv) (timer *)SvPTR (sv, "urxvt::timer")

struct timer : time_watcher, perl_watcher
{
  tstamp interval;

  timer ()
  : time_watcher (this, &timer::execute)
  {
  }

  void execute (time_watcher &w)
  {
    if (interval)
      start (at + interval);

    invoke ("urxvt::timer", newSVtimer (this));
  }
};

#define newSViow(iow) new_ref (iow->self, "urxvt::iow")
#define SvIOW(sv) (iow *)SvPTR (sv, "urxvt::iow")

struct iow : io_watcher, perl_watcher
{
  iow ()
  : io_watcher (this, &iow::execute)
  {
  }

  void execute (io_watcher &w, short revents)
  {
    invoke ("urxvt::iow", newSViow (this), revents);
  }
};

/////////////////////////////////////////////////////////////////////////////

#define SvOVERLAY(sv) (overlay *)SvPTR (sv, "urxvt::overlay")

struct overlay {
  HV *self;
  rxvt_term *THIS;
  int x, y, w, h;
  int border;
  text_t **text;
  rend_t **rend;

  overlay (rxvt_term *THIS, int x_, int y_, int w_, int h_, rend_t rstyle, int border);
  ~overlay ();

  void show ();
  void hide ();

  void swap ();

  void set (int x, int y, SV *str, SV *rend);
};

overlay::overlay (rxvt_term *THIS, int x_, int y_, int w_, int h_, rend_t rstyle, int border)
: THIS(THIS), x(x_), y(y_), w(w_), h(h_), border(border == 2)
{
  if (border == 2)
    {
      w += 2;
      h += 2;
    }

  text = new text_t *[h];
  rend = new rend_t *[h];

  for (int y = 0; y < h; y++)
    {
      text_t *tp = text[y] = new text_t[w];
      rend_t *rp = rend[y] = new rend_t[w];

      text_t t0, t1, t2;
      rend_t r = rstyle;

      if (border == 2)
        {
          if (y == 0)
            t0 = 0x2554, t1 = 0x2550, t2 = 0x2557;
          else if (y < h - 1)
            t0 = 0x2551, t1 = 0x0020, t2 = 0x2551;
          else
            t0 = 0x255a, t1 = 0x2550, t2 = 0x255d;

          *tp++ = t0;
          *rp++ = r;

          for (int x = w - 2; x-- > 0; )
            {
              *tp++ = t1;
              *rp++ = r;
            }

          *tp = t2;
          *rp = r;
        }
      else
        for (int x = w; x-- > 0; )
          {
            *tp++ = 0x0020;
            *rp++ = r;
          }
    }

  show ();
  THIS->want_refresh = 1;
}

overlay::~overlay ()
{
  hide ();

  for (int y = h; y--; )
    {
      delete [] text[y];
      delete [] rend[y];
    }

  delete [] text;
  delete [] rend;

  THIS->want_refresh = 1;
}

void
overlay::show ()
{
  char key[33]; sprintf (key, "%32lx", (long)this);

  HV *hv = (HV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)THIS->perl.self), "_overlay", 8, 0));
  hv_store (hv, key, 32, newSViv ((long)this), 0);
}

void
overlay::hide ()
{
  SV **ovs = hv_fetch ((HV *)SvRV ((SV *)THIS->perl.self), "_overlay", 8, 0);

  if (ovs)
    {
      char key[33]; sprintf (key, "%32lx", (long)this);

      HV *hv = (HV *)SvRV (*ovs);
      hv_delete (hv, key, 32, G_DISCARD);
    }
}

void overlay::swap ()
{
  int ov_x = max (0, min (MOD (x, THIS->ncol), THIS->ncol - w));
  int ov_y = max (0, min (MOD (y, THIS->nrow), THIS->nrow - h));

  int ov_w = min (w, THIS->ncol - ov_x);
  int ov_h = min (h, THIS->nrow - ov_y);

  for (int y = ov_h; y--; )
    {
      text_t *t1 = text [y];
      rend_t *r1 = rend [y];

      text_t *t2 = ROW(y + ov_y - THIS->view_start).t + ov_x;
      rend_t *r2 = ROW(y + ov_y - THIS->view_start).r + ov_x;

      for (int x = ov_w; x--; )
        {
          text_t t = *t1; *t1++ = *t2; *t2++ = t;
          rend_t r = *r1; *r1++ = *r2; *r2++ = SET_FONT (r, THIS->fontset [GET_STYLE (r)]->find_font (t));
        }
    }

}

void overlay::set (int x, int y, SV *text, SV *rend)
{
  x += border;
  y += border;

  if (!IN_RANGE_EXC (y, 0, h - border))
    return;

  wchar_t *wtext = sv2wcs (text);

  for (int col = min (wcslen (wtext), w - x - border); col--; )
    this->text [y][x + col] = wtext [col];

  free (wtext);

  if (rend)
    {
      if (!SvROK (rend) || SvTYPE (SvRV (rend)) != SVt_PVAV)
        croak ("rend must be arrayref");

      AV *av = (AV *)SvRV (rend);

      for (int col = min (av_len (av) + 1, w - x - border); col--; )
        this->rend [y][x + col] = SvIV (*av_fetch (av, col, 1));
    }

  THIS->want_refresh = 1;
}


/////////////////////////////////////////////////////////////////////////////

struct rxvt_perl_interp rxvt_perl;

static PerlInterpreter *perl;

rxvt_perl_interp::rxvt_perl_interp ()
{
}

rxvt_perl_interp::~rxvt_perl_interp ()
{
  if (perl)
    {
      perl_destruct (perl);
      perl_free (perl);
    }
}

void
rxvt_perl_interp::init ()
{
  if (!perl)
    {
      char *argv[] = {
        "",
        "-T",
        "-edo '" LIBDIR "/urxvt.pm' or ($@ and die $@) or exit 1",
      };

      perl = perl_alloc ();
      perl_construct (perl);

      if (perl_parse (perl, xs_init, 3, argv, (char **)NULL)
          || perl_run (perl))
        {
          rxvt_warn ("unable to initialize perl-interpreter, continuing without.\n");

          perl_destruct (perl);
          perl_free (perl);
          perl = 0;
        }
    }
}

static void
ungrab (rxvt_term *THIS)
{
  if (THIS->perl.grabtime)
    {
      XUngrabKeyboard (THIS->display->display, THIS->perl.grabtime);
      XUngrabPointer  (THIS->display->display, THIS->perl.grabtime);
      THIS->perl.grabtime = 0;
    }
}

bool
rxvt_perl_interp::invoke (rxvt_term *term, hook_type htype, ...)
{
  if (!perl)
    return false;

  if (htype == HOOK_INIT) // first hook ever called
    {
      term->perl.self = (void *)newSVptr ((void *)term, "urxvt::term");
      hv_store ((HV *)SvRV ((SV *)term->perl.self), "_overlay", 8, newRV_noinc ((SV *)newHV ()), 0);
    }
  else if (!term->perl.self)
    return false; // perl not initialized for this instance
  else if (htype == HOOK_DESTROY)
    {
      // handled later
    }
  else if (htype == HOOK_REFRESH_BEGIN || htype == HOOK_REFRESH_END)
    {
      HV *hv = (HV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)term->perl.self), "_overlay", 8, 0));

      if (HvKEYS (hv))
        {
          hv_iterinit (hv);

          while (HE *he = hv_iternext (hv))
            ((overlay *)SvIV (hv_iterval (hv, he)))->swap ();
        }
    }
  else if (!should_invoke [htype])
    return false;

  dSP;
  va_list ap;

  va_start (ap, htype);

  ENTER;
  SAVETMPS;

  PUSHMARK (SP);

  XPUSHs (sv_2mortal (newSVterm (term)));
  XPUSHs (sv_2mortal (newSViv (htype)));

  for (;;) {
    data_type dt = (data_type)va_arg (ap, int);

    switch (dt)
      {
        case DT_INT:
          XPUSHs (sv_2mortal (newSViv (va_arg (ap, int))));
          break;

        case DT_LONG:
          XPUSHs (sv_2mortal (newSViv (va_arg (ap, long))));
          break;

        case DT_STR:
          XPUSHs (taint (sv_2mortal (newSVpv (va_arg (ap, char *), 0))));
          break;

        case DT_STR_LEN:
          {
            char *str = va_arg (ap, char *);
            int len = va_arg (ap, int);

            XPUSHs (taint (sv_2mortal (newSVpvn (str, len))));
          }
          break;

        case DT_WCS_LEN:
          {
            wchar_t *wstr = va_arg (ap, wchar_t *);
            int wlen = va_arg (ap, int);

            XPUSHs (taint (sv_2mortal (wcs2sv (wstr, wlen))));
          }
         break;

        case DT_XEVENT:
          {
            XEvent *xe = va_arg (ap, XEvent *);
            HV *hv = newHV ();

#           define set(name, sv) hv_store (hv, # name,  sizeof (# name) - 1, sv, 0)
#           define setiv(name, val) hv_store (hv, # name,  sizeof (# name) - 1, newSViv (val), 0)
#           undef set

            setiv (type,       xe->type);
            setiv (send_event, xe->xany.send_event);
            setiv (serial,     xe->xany.serial);

            switch (xe->type)
              {
                case KeyPress:
                case KeyRelease:
                case ButtonPress:
                case ButtonRelease:
                case MotionNotify:
                  setiv (time,   xe->xmotion.time);
                  setiv (x,      xe->xmotion.x);
                  setiv (y,      xe->xmotion.y);
                  setiv (row,    xe->xmotion.y / term->fheight);
                  setiv (col,    xe->xmotion.x / term->fwidth);
                  setiv (x_root, xe->xmotion.x_root);
                  setiv (y_root, xe->xmotion.y_root);
                  setiv (state,  xe->xmotion.state);
                  break;
              }

            switch (xe->type)
              {
                case KeyPress:
                case KeyRelease:
                  setiv (keycode, xe->xkey.keycode);
                  break;

                case ButtonPress:
                case ButtonRelease:
                  setiv (button,  xe->xbutton.button);
                  break;

                case MotionNotify:
                  setiv (is_hint, xe->xmotion.is_hint);
                  break;
              }

            XPUSHs (sv_2mortal (newRV_noinc ((SV *)hv)));
          }
          break;

        case DT_END:
          {
            va_end (ap);

            PUTBACK;
            int count = call_pv ("urxvt::invoke", G_ARRAY | G_EVAL);
            SPAGAIN;

            if (count)
              {
                SV *status = POPs;
                count = SvTRUE (status);
              }

            PUTBACK;
            FREETMPS;
            LEAVE;

            if (SvTRUE (ERRSV))
              {
                rxvt_warn ("perl hook %d evaluation error: %s", htype, SvPV_nolen (ERRSV));
                ungrab (term); // better lose the grab than the session
              }

            if (htype == HOOK_DESTROY)
              {
                clearSVptr ((SV *)term->perl.self);
                SvREFCNT_dec ((SV *)term->perl.self);
              }

            return count;
          }

        default:
          rxvt_fatal ("FATAL: unable to pass data type %d\n", dt);
      }
  }
}

/////////////////////////////////////////////////////////////////////////////

MODULE = urxvt             PACKAGE = urxvt

PROTOTYPES: ENABLE

BOOT:
{
  sv_setsv (get_sv ("urxvt::LIBDIR", 1), newSVpvn (LIBDIR, sizeof (LIBDIR) - 1));

  AV *hookname = get_av ("urxvt::HOOKNAME", 1);
# define def(sym) av_store (hookname, HOOK_ ## sym, newSVpv (# sym, 0));
# include "hookinc.h"
# undef def

  HV *option = get_hv ("urxvt::OPTION", 1);
# define def(name,val) hv_store (option, # name, sizeof (# name) - 1, newSVuv (Opt_ ## name), 0);
# define nodef(name)
# include "optinc.h"
# undef nodef
# undef def

  HV *stash = gv_stashpv ("urxvt", 1);
# define export_const_iv(name) newCONSTSUB (stash, # name, newSViv (name));
  export_const_iv (DEFAULT_RSTYLE);
  export_const_iv (OVERLAY_RSTYLE);
  export_const_iv (RS_Bold);
  export_const_iv (RS_Italic);
  export_const_iv (RS_Blink);
  export_const_iv (RS_RVid);
  export_const_iv (RS_Uline);

  export_const_iv (CurrentTime);
  export_const_iv (ShiftMask);
  export_const_iv (LockMask);
  export_const_iv (ControlMask);
  export_const_iv (Mod1Mask);
  export_const_iv (Mod2Mask);
  export_const_iv (Mod3Mask);
  export_const_iv (Mod4Mask);
  export_const_iv (Mod5Mask);
  export_const_iv (Button1Mask);
  export_const_iv (Button2Mask);
  export_const_iv (Button3Mask);
  export_const_iv (Button4Mask);
  export_const_iv (Button5Mask);
  export_const_iv (AnyModifier);
}

SV *
new (...)
	CODE:
{
	stringvec *argv = new stringvec;
        bool success;

        for (int i = 0; i < items ;i++)
          argv->push_back (strdup (SvPVbyte_nolen (ST (i))));

        rxvt_term *term = new rxvt_term;

        term->argv = argv;

        try
          {
            if (!term->init (argv->size (), argv->begin ()))
              term = 0;
          }
        catch (const class rxvt_failure_exception &e)
          {
            term->destroy ();
            croak ("exception caught while initializing new terminal instance");
          }

        RETVAL = term && term->perl.self ? newSVterm (term) : &PL_sv_undef;
}
	OUTPUT:
        RETVAL

void
set_should_invoke (int htype, int value)
	CODE:
        rxvt_perl.should_invoke [htype] = value;

void
warn (const char *msg)
	CODE:
        rxvt_warn ("%s", msg);

void
fatal (const char *msg)
	CODE:
        rxvt_fatal ("%s", msg);

SV *
untaint (SV *sv)
	CODE:
        RETVAL = newSVsv (sv);
        SvTAINTED_off (RETVAL);
        OUTPUT:
        RETVAL

NV
NOW ()
	CODE:
        RETVAL = NOW;
        OUTPUT:
        RETVAL

int
GET_BASEFG (int rend)
	CODE:
        RETVAL = GET_BASEFG (rend);
	OUTPUT:
        RETVAL

int
GET_BASEBG (int rend)
	CODE:
        RETVAL = GET_BASEBG (rend);
	OUTPUT:
        RETVAL

int
SET_FGCOLOR (int rend, int new_color)
	CODE:
        RETVAL = SET_FGCOLOR (rend, new_color);
	OUTPUT:
        RETVAL

int
SET_BGCOLOR (int rend, int new_color)
	CODE:
        RETVAL = SET_BGCOLOR (rend, new_color);
	OUTPUT:
        RETVAL

int
GET_CUSTOM (int rend)
	CODE:
        RETVAL = (rend && RS_customMask) >> RS_customShift;
	OUTPUT:
        RETVAL

int
SET_CUSTOM (int rend, int new_value)
	CODE:
{
        if (!IN_RANGE_EXC (new_value, 0, RS_customCount))
          croak ("custom value out of range, must be 0..%d", RS_customCount - 1);

        RETVAL = (rend & ~RS_customMask)
               | ((new_value << RS_customShift) & RS_customMask);
}
	OUTPUT:
        RETVAL

MODULE = urxvt             PACKAGE = urxvt::term

void
rxvt_term::destroy ()

void
rxvt_term::grab_button (int button, U32 modifiers)
	CODE:
	XGrabButton (THIS->display->display, button, modifiers, THIS->vt, 1,
                     ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                     GrabModeSync, GrabModeSync, None, GRAB_CURSOR);

bool
rxvt_term::grab (U32 eventtime, int sync = 0)
	CODE:
{
        int mode = sync ? GrabModeSync : GrabModeAsync;

        THIS->perl.grabtime = 0;

        if (!XGrabPointer (THIS->display->display, THIS->vt, 0,
                           ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                           mode, mode, None, GRAB_CURSOR, eventtime))
          if (!XGrabKeyboard (THIS->display->display, THIS->vt, 0, mode, mode, eventtime))
            THIS->perl.grabtime = eventtime;
          else
            XUngrabPointer (THIS->display->display, eventtime);

        RETVAL = !!THIS->perl.grabtime;
}
	OUTPUT:
        RETVAL

void
rxvt_term::allow_events_async ()
	CODE:
        XAllowEvents (THIS->display->display, AsyncBoth,      THIS->perl.grabtime);

void
rxvt_term::allow_events_sync ()
	CODE:
        XAllowEvents (THIS->display->display, SyncBoth,       THIS->perl.grabtime);

void
rxvt_term::allow_events_replay ()
	CODE:
        XAllowEvents (THIS->display->display, ReplayPointer,  THIS->perl.grabtime);
        XAllowEvents (THIS->display->display, ReplayKeyboard, THIS->perl.grabtime);

void
rxvt_term::ungrab ()
	CODE:
        ungrab (THIS);

int
rxvt_term::strwidth (SV *str)
	CODE:
{
        wchar_t *wstr = sv2wcs (str);

	rxvt_push_locale (THIS->locale);
        RETVAL = wcswidth (wstr, wcslen (wstr));
        rxvt_pop_locale ();

        free (wstr);
}
	OUTPUT:
        RETVAL

SV *
rxvt_term::locale_encode (SV *str)
	CODE:
{
        wchar_t *wstr = sv2wcs (str);

	rxvt_push_locale (THIS->locale);
        char *mbstr = rxvt_wcstombs (wstr);
        rxvt_pop_locale ();

        free (wstr);

        RETVAL = taint_if (newSVpv (mbstr, 0), str);
        free (mbstr);
}
	OUTPUT:
        RETVAL

SV *
rxvt_term::locale_decode (SV *octets)
	CODE:
{
	STRLEN len;
        char *data = SvPVbyte (octets, len);

	rxvt_push_locale (THIS->locale);
        wchar_t *wstr = rxvt_mbstowcs (data, len);
        rxvt_pop_locale ();

        RETVAL = taint_if (wcs2sv (wstr), octets);
        free (wstr);
}
	OUTPUT:
        RETVAL

#define TERM_OFFSET(sym) offsetof (TermWin_t, sym)

#define TERM_OFFSET_width      TERM_OFFSET(width)
#define TERM_OFFSET_height     TERM_OFFSET(height)
#define TERM_OFFSET_fwidth     TERM_OFFSET(fwidth)
#define TERM_OFFSET_fheight    TERM_OFFSET(fheight)
#define TERM_OFFSET_fbase      TERM_OFFSET(fbase)
#define TERM_OFFSET_nrow       TERM_OFFSET(nrow)
#define TERM_OFFSET_ncol       TERM_OFFSET(ncol)
#define TERM_OFFSET_focus      TERM_OFFSET(focus)
#define TERM_OFFSET_mapped     TERM_OFFSET(mapped)
#define TERM_OFFSET_saveLines  TERM_OFFSET(saveLines)
#define TERM_OFFSET_total_rows TERM_OFFSET(total_rows)
#define TERM_OFFSET_nsaved     TERM_OFFSET(nsaved)

int
rxvt_term::width ()
	ALIAS:
           width      = TERM_OFFSET_width
           height     = TERM_OFFSET_height
           fwidth     = TERM_OFFSET_fwidth
           fheight    = TERM_OFFSET_fheight
           fbase      = TERM_OFFSET_fbase
           nrow       = TERM_OFFSET_nrow
           ncol       = TERM_OFFSET_ncol
           focus      = TERM_OFFSET_focus
           mapped     = TERM_OFFSET_mapped
           saveLines  = TERM_OFFSET_saveLines
           total_rows = TERM_OFFSET_total_rows
           nsaved     = TERM_OFFSET_nsaved
	CODE:
        RETVAL = *(int *)((char *)THIS + ix);
        OUTPUT:
        RETVAL

unsigned int
rxvt_term::ModLevel3Mask ()
	ALIAS:
           ModLevel3Mask  = 0
           ModMetaMask    = 1
           ModNumLockMask = 2
	CODE:
        switch (ix)
          {
           case 0: RETVAL = THIS->ModLevel3Mask;  break;
           case 1: RETVAL = THIS->ModMetaMask;    break;
           case 2: RETVAL = THIS->ModNumLockMask; break;
          }
        OUTPUT:
        RETVAL

U32
rxvt_term::parent ()
	CODE:
        RETVAL = (U32)THIS->parent [0];
        OUTPUT:
        RETVAL

U32
rxvt_term::vt ()
	CODE:
        RETVAL = (U32)THIS->vt;
        OUTPUT:
        RETVAL

U32
rxvt_term::rstyle (U32 new_rstyle = THIS->rstyle)
	CODE:
{
        RETVAL = THIS->rstyle;
        THIS->rstyle = new_rstyle;
}
        OUTPUT:
	RETVAL

int
rxvt_term::view_start (int newval = -1)
	CODE:
{
        RETVAL = THIS->view_start;

        if (newval >= 0)
          {
            THIS->view_start = min (newval, THIS->nsaved);
            THIS->scr_changeview (RETVAL);
          }
}
        OUTPUT:
	RETVAL

void
rxvt_term::want_refresh ()
	CODE:
        THIS->want_refresh = 1;

void
rxvt_term::ROW_t (int row_number, SV *new_text = 0, int start_col = 0, int start_ofs = 0, int max_len = MAX_COLS)
	PPCODE:
{
        if (!IN_RANGE_EXC (row_number, -THIS->nsaved, THIS->nrow))
          XSRETURN_EMPTY;

        line_t &l = ROW(row_number);

        if (GIMME_V != G_VOID)
          {
            wchar_t *wstr = new wchar_t [THIS->ncol];

            for (int col = 0; col < THIS->ncol; col++)
              wstr [col] = l.t [col];

            XPUSHs (taint (sv_2mortal (wcs2sv (wstr, THIS->ncol))));

            delete [] wstr;
          }

        if (new_text)
          {
            wchar_t *wstr = sv2wcs (new_text);

            int len = min (wcslen (wstr) - start_ofs, max_len);

            if (!IN_RANGE_INC (start_col, 0, THIS->ncol - len))
              {
                free (wstr);
                croak ("new_text extends beyond horizontal margins");
              }

            for (int col = start_col; col < start_col + len; col++)
              {
                l.t [col] = wstr [start_ofs + col - start_col];
                l.r [col] = SET_FONT (l.r [col], THIS->fontset [GET_STYLE (l.r [col])]->find_font (l.t [col]));
              }

            free (wstr);
          }
}

void
rxvt_term::ROW_r (int row_number, SV *new_rend = 0, int start_col = 0, int start_ofs = 0, int max_len = MAX_COLS)
	PPCODE:
{
        if (!IN_RANGE_EXC (row_number, -THIS->nsaved, THIS->nrow))
          XSRETURN_EMPTY;

        line_t &l = ROW(row_number);

        if (GIMME_V != G_VOID)
          {
            AV *av = newAV ();

            av_extend (av, THIS->ncol - 1);
            for (int col = 0; col < THIS->ncol; col++)
              av_store (av, col, newSViv (l.r [col]));

            XPUSHs (sv_2mortal (newRV_noinc ((SV *)av)));
          }

        if (new_rend)
          {
            if (!SvROK (new_rend) || SvTYPE (SvRV (new_rend)) != SVt_PVAV)
              croak ("new_rend must be arrayref");

            AV *av = (AV *)SvRV (new_rend);
            int len = min (av_len (av) + 1 - start_ofs, max_len);

            if (!IN_RANGE_INC (start_col, 0, THIS->ncol - len))
              croak ("new_rend array extends beyond horizontal margins");

            for (int col = start_col; col < start_col + len; col++)
              {
                rend_t r = SvIV (*av_fetch (av, start_ofs + col - start_col, 1)) & ~RS_fontMask;

                l.r [col] = SET_FONT (r, THIS->fontset [GET_STYLE (r)]->find_font (l.t [col]));
              }
          }
}

int
rxvt_term::ROW_l (int row_number, int new_length = -1)
	CODE:
{
        if (!IN_RANGE_EXC (row_number, -THIS->nsaved, THIS->nrow))
          XSRETURN_EMPTY;

        line_t &l = ROW(row_number);
        RETVAL = l.l;

        if (new_length >= 0)
          l.l = new_length;
}
        OUTPUT:
        RETVAL

bool
rxvt_term::ROW_is_longer (int row_number, int new_is_longer = -1)
	CODE:
{
        if (!IN_RANGE_EXC (row_number, -THIS->nsaved, THIS->nrow))
          XSRETURN_EMPTY;

        line_t &l = ROW(row_number);
        RETVAL = l.is_longer ();

        if (new_is_longer >= 0)
          l.is_longer (new_is_longer);
}
        OUTPUT:
        RETVAL

SV *
rxvt_term::special_encode (SV *string)
	CODE:
{
        wchar_t *wstr = sv2wcs (string);
        int wlen = wcslen (wstr);
        wchar_t *rstr = new wchar_t [wlen]; // cannot become longer

	rxvt_push_locale (THIS->locale);

        wchar_t *r = rstr;
        for (wchar_t *s = wstr; *s; s++)
          if (wcwidth (*s) == 0)
            {
              if (r == rstr)
                croak ("leading combining character unencodable");

              unicode_t n = rxvt_compose (r[-1], *s);
              if (n == NOCHAR)
                n = rxvt_composite.compose (r[-1], *s);

              r[-1] = n;
            }
#if !UNICODE_3
          else if (*s >= 0x10000)
            *r++ = rxvt_composite.compose (*s);
#endif
          else
            *r++ = *s;

	rxvt_pop_locale ();

        RETVAL = taint_if (wcs2sv (rstr, r - rstr), string);

        delete [] rstr;
}
	OUTPUT:
        RETVAL

SV *
rxvt_term::special_decode (SV *text)
	CODE:
{
        wchar_t *wstr = sv2wcs (text);
        int wlen = wcslen (wstr);
        int dlen = 0;

        // find length
        for (wchar_t *s = wstr; *s; s++)
          if (*s == NOCHAR)
            ;
          else if (IS_COMPOSE (*s))
            dlen += rxvt_composite.expand (*s, 0);
          else
            dlen++;

        wchar_t *rstr = new wchar_t [dlen];

        // decode
        wchar_t *r = rstr;
        for (wchar_t *s = wstr; *s; s++)
          if (*s == NOCHAR)
            ;
          else if (IS_COMPOSE (*s))
            r += rxvt_composite.expand (*s, r);
          else
            *r++ = *s;

        RETVAL = taint_if (wcs2sv (rstr, r - rstr), text);

        delete [] rstr;
}
	OUTPUT:
        RETVAL

void
rxvt_term::_resource (char *name, int index, SV *newval = 0)
	PPCODE:
{
	struct resval { const char *name; int value; } rslist [] = {
#	  define def(name) { # name, Rs_ ## name },
#	  define reserve(name,count)
#	  include "rsinc.h"
#	  undef def
#	  undef reserve
        };

        struct resval *rs = rslist + sizeof (rslist) / sizeof (rslist [0]);

        do {
          if (rs-- == rslist)
            croak ("no such resource '%s', requested", name);
        } while (strcmp (name, rs->name));

        index += rs->value;

        if (!IN_RANGE_EXC (index, 0, NUM_RESOURCES))
          croak ("requested out-of-bound resource %s+%d,", name, index - rs->value);

        if (GIMME_V != G_VOID)
          XPUSHs (THIS->rs [index] ? sv_2mortal (taint (newSVpv (THIS->rs [index], 0))) : &PL_sv_undef);

        if (newval)
          {
            if (SvOK (newval))
              {
                char *str = strdup (SvPVbyte_nolen (newval));
                THIS->rs [index] = str;
                THIS->allocated.push_back (str);
              }
            else
              THIS->rs [index] = 0;
          }
}

bool
rxvt_term::option (U32 optval, int set = -1)
	CODE:
{
	RETVAL = THIS->options & optval;

        if (set >= 0)
          {
            if (set)
              THIS->options |= optval;
            else
              THIS->options &= ~optval;

            switch (optval)
              {
                case Opt_skipBuiltinGlyphs:
                  THIS->set_fonts ();
                  THIS->scr_remap_chars ();
                  THIS->scr_touch (true);
                  THIS->want_refresh = 1;
                  break;

                case Opt_cursorUnderline:
                  THIS->want_refresh = 1;
                  break;

#                  case Opt_scrollBar_floating:
#                  case Opt_scrollBar_right:
#                    THIS->resize_all_windows (THIS->width, THIS->height, 1);
#                    break;
              }
          }
}
        OUTPUT:
        RETVAL

void
rxvt_term::screen_cur (...)
	PROTOTYPE: $;$$
        ALIAS:
           screen_cur     = 0
           selection_beg  = 1
           selection_end  = 2
           selection_mark = 3
        PPCODE:
{
        row_col_t &rc = ix == 0 ? THIS->screen.cur
                      : ix == 1 ? THIS->selection.beg
                      : ix == 2 ? THIS->selection.end
                      :           THIS->selection.mark;

        if (GIMME_V != G_VOID)
          {
            EXTEND (SP, 2);
            PUSHs (sv_2mortal (newSViv (rc.row)));
            PUSHs (sv_2mortal (newSViv (rc.col)));
          }

        if (items == 3)
          {
            rc.row = clamp (SvIV (ST (1)), -THIS->nsaved, THIS->nrow - 1);
            rc.col = clamp (SvIV (ST (2)), 0, THIS->ncol - 1);

            if (ix)
              THIS->want_refresh = 1;
          }
}

int
rxvt_term::selection_grab (U32 eventtime)

void
rxvt_term::selection (SV *newtext = 0)
        PPCODE:
{
        if (GIMME_V != G_VOID)
          XPUSHs (THIS->selection.text
                  ? taint (sv_2mortal (wcs2sv (THIS->selection.text, THIS->selection.len)))
                  : &PL_sv_undef);

        if (newtext)
          {
            free (THIS->selection.text);

            THIS->selection.text = sv2wcs (newtext);
            THIS->selection.len = wcslen (THIS->selection.text);
          }
}

void
rxvt_term::scr_add_lines (SV *string)
	CODE:
{
        wchar_t *wstr = sv2wcs (string);
        THIS->scr_add_lines (wstr, wcslen (wstr));
        free (wstr);
}

void
rxvt_term::tt_write (SV *octets)
        INIT:
          STRLEN len;
          char *str = SvPVbyte (octets, len);
	C_ARGS:
          str, len

void
rxvt_term::cmd_parse (SV *octets)
	CODE:
{
	STRLEN len;
        char *str = SvPVbyte (octets, len);

        char *old_cmdbuf_ptr  = THIS->cmdbuf_ptr;
        char *old_cmdbuf_endp = THIS->cmdbuf_endp;

        THIS->cmdbuf_ptr  = str;
        THIS->cmdbuf_endp = str + len;

	rxvt_push_locale (THIS->locale);
        THIS->cmd_parse ();
	rxvt_pop_locale ();

        THIS->cmdbuf_ptr  = old_cmdbuf_ptr;
        THIS->cmdbuf_endp = old_cmdbuf_endp;
}

SV *
rxvt_term::overlay (int x, int y, int w, int h, int rstyle = OVERLAY_RSTYLE, int border = 2)
	CODE:
{
        overlay *o = new overlay (THIS, x, y, w, h, rstyle, border);
        RETVAL = newSVptr ((void *)o, "urxvt::overlay");
        o->self = (HV *)SvRV (RETVAL);
}
	OUTPUT:
        RETVAL

MODULE = urxvt             PACKAGE = urxvt::overlay

void
overlay::set (int x, int y, SV *text, SV *rend = 0)

void
overlay::show ()

void
overlay::hide ()

void
overlay::DESTROY ()

MODULE = urxvt             PACKAGE = urxvt::timer

SV *
timer::new ()
	CODE:
        timer *w =  new timer;
        w->start (NOW);
        RETVAL = newSVptr ((void *)w, "urxvt::timer");
        w->self = (HV *)SvRV (RETVAL);
        OUTPUT:
        RETVAL

timer *
timer::cb (SV *cb)
	CODE:
        THIS->cb (cb);
        RETVAL = THIS;
        OUTPUT:
        RETVAL

NV
timer::at ()
	CODE:
        RETVAL = THIS->at;
        OUTPUT:
        RETVAL

timer *
timer::interval (NV interval)
	CODE:
        THIS->interval = interval;
        RETVAL = THIS;
        OUTPUT:
        RETVAL

timer *
timer::set (NV tstamp)
	CODE:
        THIS->set (tstamp);
        RETVAL = THIS;
        OUTPUT:
        RETVAL

timer *
timer::start (NV tstamp = THIS->at)
        CODE:
        THIS->start (tstamp);
        RETVAL = THIS;
        OUTPUT:
        RETVAL

timer *
timer::stop ()
	CODE:
        THIS->stop ();
        RETVAL = THIS;
        OUTPUT:
        RETVAL

void
timer::DESTROY ()

MODULE = urxvt             PACKAGE = urxvt::iow

SV *
iow::new ()
	CODE:
        iow *w =  new iow;
        RETVAL = newSVptr ((void *)w, "urxvt::iow");
        w->self = (HV *)SvRV (RETVAL);
        OUTPUT:
        RETVAL

iow *
iow::cb (SV *cb)
	CODE:
        THIS->cb (cb);
        RETVAL = THIS;
        OUTPUT:
        RETVAL

iow *
iow::fd (int fd)
	CODE:
        THIS->fd = fd;
        RETVAL = THIS;
        OUTPUT:
        RETVAL

iow *
iow::events (short events)
	CODE:
        THIS->events = events;
        RETVAL = THIS;
        OUTPUT:
        RETVAL

iow *
iow::start ()
        CODE:
        THIS->start ();
        RETVAL = THIS;
        OUTPUT:
        RETVAL

iow *
iow::stop ()
	CODE:
        THIS->stop ();
        RETVAL = THIS;
        OUTPUT:
        RETVAL

void
iow::DESTROY ()


