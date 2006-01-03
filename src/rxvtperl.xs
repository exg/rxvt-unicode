/*----------------------------------------------------------------------*
 * File:	rxvtperl.xs
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2005 Marc Lehmann <pcg@goof.com>
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

#include <cstdarg>

#include "rxvt.h"
#include "iom.h"
#include "rxvtutil.h"
#include "rxvtperl.h"

#include "perlxsi.c"

#undef LINENO
#define LINENO(n) MOD (THIS->term_start + int(n), THIS->total_rows)
#undef ROW
#define ROW(n) THIS->row_buf [LINENO (n)]

/////////////////////////////////////////////////////////////////////////////

static wchar_t *
sv2wcs (SV *sv)
{
  STRLEN len;
  char *str = SvPVutf8 (sv, len);
  return rxvt_utf8towcs (str, len);
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
  hv_store (hv, "_ptr", 4, newSViv ((long)ptr), 0);
  return sv_bless (newRV_noinc ((SV *)hv), gv_stashpv (klass, 1));
}

static long
SvPTR (SV *sv, const char *klass)
{
  if (!sv_derived_from (sv, klass))
    croak ("object of type %s expected", klass);

  IV iv = SvIV (*hv_fetch ((HV *)SvRV (sv), "_ptr", 4, 1));

  if (!iv)
    croak ("perl code used %s object, but C++ object is already destroyed, caught", klass);

  return (long)iv;
}

#define newSVterm(term) SvREFCNT_inc ((SV *)term->self)
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

  THIS->want_refresh = 1;
}

overlay::~overlay ()
{
  for (int y = h; y--; )
    {
      delete [] text[y];
      delete [] rend[y];
    }

  delete [] text;
  delete [] rend;

  THIS->want_refresh = 1;
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
        "-edo '" LIBDIR "/urxvt.pm' or ($@ and die $@) or exit 1",
      };

      perl = perl_alloc ();
      perl_construct (perl);

      if (perl_parse (perl, xs_init, 2, argv, (char **)NULL)
          || perl_run (perl))
        {
          rxvt_warn ("unable to initialize perl-interpreter, continuing without.\n");

          perl_destruct (perl);
          perl_free (perl);
          perl = 0;
        }
    }
}

bool
rxvt_perl_interp::invoke (rxvt_term *term, hook_type htype, ...)
{
  if (!perl)
    return false;
  
  if (htype == HOOK_INIT) // first hook ever called
    {
      term->self = (void *)newSVptr ((void *)term, "urxvt::term");
      hv_store ((HV *)SvRV ((SV *)term->self), "_overlay", 8, newRV_noinc ((SV *)newHV ()), 0);
    }
  else if (!term->self)
    return false; // perl not initialized for this instance
  else if (htype == HOOK_DESTROY)
    {
      // handled later
    }
  else if (htype == HOOK_REFRESH_BEGIN || htype == HOOK_REFRESH_END)
    {
      HV *hv = (HV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)term->self), "_overlay", 8, 0));

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

        case DT_STRING:
          XPUSHs (sv_2mortal (newSVpv (va_arg (ap, char *), 0)));
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
              rxvt_warn ("perl hook %d evaluation error: %s", htype, SvPV_nolen (ERRSV));

            if (htype == HOOK_DESTROY)
              {
                // TODO: clear magic
                hv_clear ((HV *)SvRV ((SV *)term->self));
                SvREFCNT_dec ((SV *)term->self);
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
# define set_hookname(sym) av_store (hookname, PP_CONCAT(HOOK_, sym), newSVpv (PP_STRINGIFY(sym), 0))
# define export_const(name) newCONSTSUB (gv_stashpv ("urxvt", 1), #name, newSViv (name));
  AV *hookname = get_av ("urxvt::HOOKNAME", 1);
  set_hookname (INIT);
  set_hookname (RESET);
  set_hookname (START);
  set_hookname (DESTROY);
  set_hookname (SEL_BEGIN);
  set_hookname (SEL_EXTEND);
  set_hookname (SEL_MAKE);
  set_hookname (SEL_GRAB);
  set_hookname (FOCUS_IN);
  set_hookname (FOCUS_OUT);
  set_hookname (VIEW_CHANGE);
  set_hookname (SCROLL_BACK);
  set_hookname (TTY_ACTIVITY);
  set_hookname (REFRESH_BEGIN);
  set_hookname (REFRESH_END);
  set_hookname (KEYBOARD_COMMAND);

  export_const (DEFAULT_RSTYLE);
  export_const (OVERLAY_RSTYLE);
  export_const (RS_Bold);
  export_const (RS_Italic);
  export_const (RS_Blink);
  export_const (RS_RVid);
  export_const (RS_Uline);

  sv_setpv (get_sv ("urxvt::LIBDIR", 1), LIBDIR);
}

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

        RETVAL = newSVpv (mbstr, 0);
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

        char *str = rxvt_wcstoutf8 (wstr);
        free (wstr);

        RETVAL = newSVpv (str, 0);
        SvUTF8_on (RETVAL);
        free (str);
}
	OUTPUT:
        RETVAL

int
rxvt_term::nsaved ()
	CODE:
        RETVAL = THIS->nsaved;
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

int
rxvt_term::nrow ()
	CODE:
        RETVAL = THIS->nrow;
        OUTPUT:
        RETVAL

int
rxvt_term::ncol ()
	CODE:
        RETVAL = THIS->ncol;
        OUTPUT:
        RETVAL

void
rxvt_term::want_refresh ()
	CODE:
        THIS->want_refresh = 1;

void
rxvt_term::ROW_t (int row_number, SV *new_text = 0, int start_col = 0)
	PPCODE:
{
        if (!IN_RANGE_EXC (row_number, -THIS->nsaved, THIS->nrow))
          croak ("row_number number of out range");

        line_t &l = ROW(row_number);

        if (GIMME_V != G_VOID)
          {
            wchar_t *wstr = new wchar_t [THIS->ncol];

            for (int col = 0; col <THIS->ncol; col++)
              wstr [col] = l.t [col];

            char *str = rxvt_wcstoutf8 (wstr, THIS->ncol);
            free (wstr);

            SV *sv = newSVpv (str, 0);
            SvUTF8_on (sv);
            XPUSHs (sv_2mortal (sv));
            free (str);
          }

        if (new_text)
          {
            wchar_t *wstr = sv2wcs (new_text);

            int len = wcslen (wstr);

            if (!IN_RANGE_INC (start_col, 0, THIS->ncol - len))
              {
                free (wstr);
                croak ("new_text extends beyond horizontal margins");
              }

            for (int col = start_col; col < start_col + len; col++)
              {
                l.t [col] = wstr [col - start_col];
                l.r [col] = SET_FONT (l.r [col], THIS->fontset [GET_STYLE (l.r [col])]->find_font (l.t [col]));
              }

            free (wstr);
          }
}

void
rxvt_term::ROW_r (int row_number, SV *new_rend = 0, int start_col = 0)
	PPCODE:
{
        if (!IN_RANGE_EXC (row_number, -THIS->nsaved, THIS->nrow))
          croak ("row_number number of out range");

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
            int len = av_len (av) + 1;

            if (!IN_RANGE_INC (start_col, 0, THIS->ncol - len))
              croak ("new_rend array extends beyond horizontal margins");

            for (int col = start_col; col < start_col + len; col++)
              {
                rend_t r = SvIV (*av_fetch (av, col - start_col, 1)) & ~RS_fontMask;

                l.r [col] = SET_FONT (r, THIS->fontset [GET_STYLE (r)]->find_font (l.t [col]));
              }
          }
}

int
rxvt_term::ROW_l (int row_number, int new_length = -2)
	CODE:
{
        if (!IN_RANGE_EXC (row_number, -THIS->nsaved, THIS->nrow))
          croak ("row_number number of out range");

        line_t &l = ROW(row_number);
        RETVAL = l.l;

        if (new_length >= -1)
          l.l = new_length;
}
        OUTPUT:
        RETVAL

SV *
rxvt_term::special_encode (SV *str)
	CODE:
        abort ();//TODO

SV *
rxvt_term::special_decode (SV *str)
	CODE:
        abort ();//TODO

void
rxvt_term::_resource (char *name, int index, SV *newval = 0)
	PPCODE:
{
	struct resval { const char *name; int value; } rslist [] = {
#	  define Rs_def(name) { # name, Rs_ ## name },
#	  define Rs_reserve(name,count)
#	  include "rsinc.h"
#	  undef Rs_def
#	  undef Rs_reserve
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
          XPUSHs (THIS->rs [index] ? sv_2mortal (newSVpv (THIS->rs [index], 0)) : &PL_sv_undef);

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

void
rxvt_term::selection_mark (...)
	PROTOTYPE: $;$$
        ALIAS:
           selection_beg = 1
           selection_end = 2
        PPCODE:
{
        row_col_t &sel = ix == 1 ? THIS->selection.beg
                       : ix == 2 ? THIS->selection.end
                       :           THIS->selection.mark;

        if (GIMME_V != G_VOID)
          {
            EXTEND (SP, 2);
            PUSHs (sv_2mortal (newSViv (sel.row)));
            PUSHs (sv_2mortal (newSViv (sel.col)));
          }

        if (items == 3)
          {
            sel.row = clamp (SvIV (ST (1)), -THIS->nsaved, THIS->nrow - 1);
            sel.col = clamp (SvIV (ST (2)), 0, THIS->ncol - 1);

            if (ix)
              THIS->want_refresh = 1;
          }
}

int
rxvt_term::selection_grab (int eventtime = CurrentTime)

void
rxvt_term::selection (SV *newtext = 0)
        PPCODE:
{
        if (GIMME_V != G_VOID)
          {
            char *sel = rxvt_wcstoutf8 (THIS->selection.text, THIS->selection.len);
            SV *sv = newSVpv (sel, 0);
            SvUTF8_on (sv);
            free (sel);
            XPUSHs (sv_2mortal (sv));
          }

        if (newtext)
          {
            free (THIS->selection.text);

            THIS->selection.text = sv2wcs (newtext);
            THIS->selection.len = wcslen (THIS->selection.text);
          }
}
        
void
rxvt_term::tt_write (SV *octets)
        INIT:
          STRLEN len;
          char *str = SvPVbyte (octets, len);
	C_ARGS:
          (unsigned char *)str, len

SV *
rxvt_term::overlay (int x, int y, int w, int h, int rstyle = OVERLAY_RSTYLE, int border = 2)
	CODE:
{
        overlay *o = new overlay (THIS, x, y, w, h, rstyle, border);
        RETVAL = newSVptr ((void *)o, "urxvt::overlay");
        o->self = (HV *)SvRV (RETVAL);

        HV *hv = (HV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)THIS->self), "_overlay", 8, 0));
        char key[33]; sprintf (key, "%32lx", (long)o);
        hv_store (hv, key, 32, newSViv ((long)o), 0);
}
	OUTPUT:
        RETVAL

MODULE = urxvt             PACKAGE = urxvt::overlay

void
overlay::set (int x, int y, SV *text, SV *rend = 0)

void
overlay::DESTROY ()
	CODE:
{
        SV **ovs = hv_fetch ((HV *)SvRV ((SV *)THIS->THIS->self), "_overlay", 8, 0);
        if (ovs)
          {
            HV *hv = (HV *)SvRV (*ovs);
            char key[33]; sprintf (key, "%32lx", (long)THIS);
            hv_delete (hv, key, 32, G_DISCARD);
          }

        delete THIS;
}

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


