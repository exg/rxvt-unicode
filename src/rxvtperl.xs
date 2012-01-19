/*----------------------------------------------------------------------*
 * File:	rxvtperl.xs
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2008,2011 Marc Lehmann <schmorp@schmorp.de>
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
#undef bool // perl defines it's own bool type, except with g++... what a trap

#include "../config.h"

#include <stddef.h>
#include <stdarg.h>

#include "unistd.h"

#include "ev_cpp.h"
#include "rxvt.h"
#include "keyboard.h"
#include "rxvtutil.h"
#include "rxvtperl.h"

#include "perlxsi.c"

#define GRAB_CURSOR THIS->scrollBar.leftptr_cursor

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
wcs2sv (wchar_t *wstr, int len = -1)
{
  char *str = rxvt_wcstoutf8 (wstr, len);

  SV *sv = newSVpv (str, 0);
  SvUTF8_on (sv);
  free (str);

  return sv;
}

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

#define newSVterm(term) SvREFCNT_inc ((SV *)(term)->perl.self)
#define SvTERM(sv) (rxvt_term *)SvPTR ((sv), "urxvt::term")

/////////////////////////////////////////////////////////////////////////////

#define SvOVERLAY(sv) (overlay *)SvPTR (sv, "urxvt::overlay")

class overlay : overlay_base
{
  rxvt_term *THIS;
  AV *overlay_av;
  int border;

public:
  HV *self;

  overlay (rxvt_term *THIS, int x_, int y_, int w_, int h_, rend_t rstyle, int border);
  ~overlay ();

  void show ();
  void hide ();

  void swap ();

  void set (int x, int y, SV *str, SV *rend);
};

overlay::overlay (rxvt_term *THIS, int x_, int y_, int w_, int h_, rend_t rstyle, int border)
: THIS(THIS), border(border == 2), overlay_av (0)
{
  x = x_;
  y = y_;
  w = w_;
  h = h_;

  if (w < 0) w = 0;
  if (h < 0) h = 0;

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
}

void
overlay::show ()
{
  if (overlay_av)
    return;

  overlay_av = (AV *)SvREFCNT_inc (SvRV (
        *hv_fetch ((HV *)SvRV ((SV *)THIS->perl.self), "_overlay", 8, 0)
     ));
  av_push (overlay_av, newSViv ((long)this));

  THIS->want_refresh = 1;
  THIS->refresh_check ();
}

void
overlay::hide ()
{
  if (!overlay_av)
    return;

  int i;

  for (i = AvFILL (overlay_av); i >= 0; i--)
    if (SvIV (*av_fetch (overlay_av, i, 1)) == (long)this)
      break;

  for (; i < AvFILL (overlay_av); i++)
    av_store (overlay_av, i, SvREFCNT_inc (*av_fetch (overlay_av, i + 1, 0)));

  av_pop (overlay_av);

  SvREFCNT_dec (overlay_av);
  overlay_av = 0;

  THIS->want_refresh = 1;
  THIS->refresh_check ();
}

void overlay::swap ()
{
  int ov_x = max (0, min (MOD (x, THIS->ncol), THIS->ncol - w));
  int ov_y = max (0, min (MOD (y, THIS->nrow), THIS->nrow - h));

  int ov_w = min (w, THIS->ncol - ov_x);
  int ov_h = min (h, THIS->nrow - ov_y);

  // hide cursor if it is within the overlay area
  if (IN_RANGE_EXC (THIS->screen.cur.col - ov_x, 0, ov_w)
      && IN_RANGE_EXC (THIS->screen.cur.row - ov_y, 0, ov_h))
    THIS->screen.flags &= ~Screen_VisibleCursor;

  for (int y = ov_h; y--; )
    {
      text_t *t1 = text [y];
      rend_t *r1 = rend [y];

      text_t *t2 = ROW(y + ov_y + THIS->view_start).t + ov_x;
      rend_t *r2 = ROW(y + ov_y + THIS->view_start).r + ov_x;

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

      for (int col = min (AvFILL (av) + 1, w - x - border); col--; )
        this->rend [y][x + col] = SvIV (*av_fetch (av, col, 1));
    }

  THIS->want_refresh = 1;
  THIS->refresh_check ();
}

/////////////////////////////////////////////////////////////////////////////

#include "iom_perl.h"

/////////////////////////////////////////////////////////////////////////////

struct rxvt_perl_interp rxvt_perl;

static PerlInterpreter *perl;

rxvt_perl_interp::~rxvt_perl_interp ()
{
  if (perl)
    {
      perl_destruct (perl);
      perl_free (perl);
      PERL_SYS_TERM ();
    }
}

void
rxvt_perl_interp::init (rxvt_term *term)
{
  if (!perl)
    {
      rxvt_push_locale (""); // perl init destroys current locale

      {
        perl_environ = rxvt_environ;
        localise_env set_environ (perl_environ);

        char *args[] = {
          "",
          "-e"
          "BEGIN {"
          "   urxvt->bootstrap;"
          "   unshift @INC, '" LIBDIR "';"
          "}"
          ""
          "use urxvt;"
        };
        int argc = ecb_array_length (args);
        char **argv = args;

        PERL_SYS_INIT3 (&argc, &argv, &environ);
        perl = perl_alloc ();
        perl_construct (perl);

        if (perl_parse (perl, xs_init, argc, argv, (char **)NULL)
            || perl_run (perl))
          {
            rxvt_warn ("unable to initialize perl-interpreter, continuing without.\n");

            perl_destruct (perl);
            perl_free (perl);
            perl = 0;
          }
      }

      rxvt_pop_locale ();
    }

  if (perl)
    {
      // runs outside of perls ENV
      term->perl.self = (void *)newSVptr ((void *)term, "urxvt::term");
      hv_store ((HV *)SvRV ((SV *)term->perl.self), "_overlay", 8, newRV_noinc ((SV *)newAV ()), 0);
      hv_store ((HV *)SvRV ((SV *)term->perl.self), "_selection", 10, newRV_noinc ((SV *)newAV ()), 0);
    }
}

static void
ungrab (rxvt_term *THIS)
{
  if (THIS->perl.grabtime)
    {
      XUngrabKeyboard (THIS->dpy, THIS->perl.grabtime);
      XUngrabPointer  (THIS->dpy, THIS->perl.grabtime);
      THIS->perl.grabtime = 0;
    }
}

bool
rxvt_perl_interp::invoke (rxvt_term *term, hook_type htype, ...)
{
  if (!perl || !term->perl.self)
    return false;

  localise_env set_environ (perl_environ);

  // pre-handling of some events
  if (htype == HOOK_REFRESH_END)
    {
      AV *av = (AV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)term->perl.self), "_overlay", 8, 0));

      for (int i = 0; i <= AvFILL (av); i++)
        ((overlay *)SvIV (*av_fetch (av, i, 0)))->swap ();
    }
  else if (htype == HOOK_DESTROY)
    {
      AV *av = (AV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)term->perl.self), "_selection", 10, 0));

      for (int i = AvFILL (av); i >= 0; i--)
        {
          rxvt_selection *req = (rxvt_selection *)SvIV (*av_fetch (av, i, 0));
          delete req;
        }
    }

  bool event_consumed;

  if (htype == HOOK_INIT || htype == HOOK_DESTROY // must be called always
      || term->perl.should_invoke [htype])
    {
      dSP;
      va_list ap;

      va_start (ap, htype);

      ENTER;
      SAVETMPS;

      PUSHMARK (SP);

      EXTEND (SP, 2);
      PUSHs (sv_2mortal (newSVterm (term)));
      PUSHs (sv_2mortal (newSViv (htype)));

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
              XPUSHs (sv_2mortal (newSVpv (va_arg (ap, char *), 0)));
              break;

            case DT_STR_LEN:
              {
                char *str = va_arg (ap, char *);
                int len   = va_arg (ap, int);

                XPUSHs (sv_2mortal (newSVpvn (str, len)));
              }
              break;

            case DT_WCS_LEN:
              {
                wchar_t *wstr = va_arg (ap, wchar_t *);
                int wlen      = va_arg (ap, int);

                XPUSHs (sv_2mortal (wcs2sv (wstr, wlen)));
              }
             break;

            case DT_LCS_LEN:
              {
                long *lstr = va_arg (ap, long *);
                int llen   = va_arg (ap, int);

                XPUSHs (sv_2mortal (newSVpvn ((char *)lstr, llen * sizeof (long))));
              }
             break;

            case DT_XEVENT:
              {
                XEvent *xe = va_arg (ap, XEvent *);
                HV *hv = newHV ();

#               define set(name, sv) hv_store (hv, # name,  sizeof (# name) - 1, sv, 0)
#               define setiv(name, val) hv_store (hv, # name,  sizeof (# name) - 1, newSViv (val), 0)
#               define setuv(name, val) hv_store (hv, # name,  sizeof (# name) - 1, newSVuv (val), 0)
#               undef set

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
                      setuv (window,    xe->xmotion.window);
                      setuv (root,      xe->xmotion.root);
                      setuv (subwindow, xe->xmotion.subwindow);
                      setuv (time,      xe->xmotion.time);
                      setiv (x,         xe->xmotion.x);
                      setiv (y,         xe->xmotion.y);
                      setiv (row,       xe->xmotion.y / term->fheight + term->view_start);
                      setiv (col,       xe->xmotion.x / term->fwidth);
                      setiv (x_root,    xe->xmotion.x_root);
                      setiv (y_root,    xe->xmotion.y_root);
                      setuv (state,     xe->xmotion.state);

                      switch (xe->type)
                        {
                          case KeyPress:
                          case KeyRelease:
                            setuv (keycode, xe->xkey.keycode);
                            break;

                          case ButtonPress:
                          case ButtonRelease:
                            setuv (button,  xe->xbutton.button);
                            break;

                          case MotionNotify:
                            setiv (is_hint, xe->xmotion.is_hint);
                            break;
                        }

                      break;

                    case MapNotify:
                    case UnmapNotify:
                    case ConfigureNotify:
                      setuv (event,  xe->xconfigure.event);
                      setuv (window, xe->xconfigure.window);

                      switch (xe->type)
                        {
                          case ConfigureNotify:
                            setiv (x,      xe->xconfigure.x);
                            setiv (y,      xe->xconfigure.y);
                            setiv (width,  xe->xconfigure.width);
                            setiv (height, xe->xconfigure.height);
                            setuv (above,  xe->xconfigure.above);
                            break;
                        }

                      break;

                    case PropertyNotify:
                      setuv (window,       xe->xproperty.window);
                      setuv (atom,         xe->xproperty.atom);
                      setuv (time,         xe->xproperty.time);
                      setiv (state,        xe->xproperty.state);
                      break;

                    case ClientMessage:
                      setuv (window,       xe->xclient.window);
                      setuv (message_type, xe->xclient.message_type);
                      setuv (format,       xe->xclient.format);
                      setuv (l0,           xe->xclient.data.l[0]);
                      setuv (l1,           xe->xclient.data.l[1]);
                      setuv (l2,           xe->xclient.data.l[2]);
                      setuv (l3,           xe->xclient.data.l[3]);
                      setuv (l4,           xe->xclient.data.l[4]);
                      break;
                  }

                XPUSHs (sv_2mortal (newRV_noinc ((SV *)hv)));
              }
              break;

            case DT_END:
              goto call;

            default:
              rxvt_fatal ("FATAL: unable to pass data type %d\n", dt);
          }
      }

    call:
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
          rxvt_warn ("perl hook %d evaluation error: %s", htype, SvPVbyte_nolen (ERRSV));
          ungrab (term); // better lose the grab than the session
        }

      event_consumed = !!count;
    }
  else
    event_consumed = false;

  // post-handling of some events
  if (htype == HOOK_REFRESH_BEGIN)
    {
      AV *av = (AV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)term->perl.self), "_overlay", 8, 0));

      for (int i = AvFILL (av); i >= 0; i--)
        ((overlay *)SvIV (*av_fetch (av, i, 0)))->swap ();
    }
  else if (htype == HOOK_DESTROY)
    {
      clearSVptr ((SV *)term->perl.self);
      SvREFCNT_dec ((SV *)term->perl.self);

      // don't allow further calls
      term->perl.self = 0;
    }

  return event_consumed;
}

void
rxvt_perl_interp::selection_finish (rxvt_selection *sel, char *data, unsigned int len)
{
  localise_env set_environ (perl_environ);

  ENTER;
  SAVETMPS;

  dSP;
  XPUSHs (sv_2mortal (newSVpvn (data, len)));
  call_sv ((SV *)sel->cb_sv, G_VOID | G_DISCARD | G_EVAL);

  if (SvTRUE (ERRSV))
    rxvt_warn ("perl selection callback evaluation error: %s", SvPVbyte_nolen (ERRSV));

  FREETMPS;
  LEAVE;
}

/////////////////////////////////////////////////////////////////////////////

MODULE = urxvt             PACKAGE = urxvt

PROTOTYPES: ENABLE

BOOT:
{
  sv_setsv (get_sv ("urxvt::LIBDIR",   1), newSVpvn (LIBDIR,   sizeof (LIBDIR)   - 1));
  sv_setsv (get_sv ("urxvt::RESNAME",  1), newSVpvn (RESNAME,  sizeof (RESNAME)  - 1));
  sv_setsv (get_sv ("urxvt::RESCLASS", 1), newSVpvn (RESCLASS, sizeof (RESCLASS) - 1));
  sv_setsv (get_sv ("urxvt::RXVTNAME", 1), newSVpvn (RXVTNAME, sizeof (RXVTNAME) - 1));

  AV *hookname = get_av ("urxvt::HOOKNAME", 1);
# define def(sym) av_store (hookname, HOOK_ ## sym, newSVpv (# sym, 0));
# include "hookinc.h"
# undef def

  HV *option = get_hv ("urxvt::OPTION", 1);
# define def(name) hv_store (option, # name, sizeof (# name) - 1, newSVuv (Opt_ ## name), 0);
# define nodef(name)
# include "optinc.h"
# undef nodef
# undef def

  HV *stash = gv_stashpv ("urxvt", 1);
  static const struct {
    const char *name;
    IV iv;
  } *civ, const_iv[] = {
#   define const_iv(name) { # name, (IV)name }
    const_iv (NUM_RESOURCES),
    const_iv (DEFAULT_RSTYLE),
    const_iv (OVERLAY_RSTYLE),
    const_iv (Color_Bits),
    const_iv (RS_bgShift), const_iv (RS_bgMask),
    const_iv (RS_fgShift), const_iv (RS_fgMask),
    const_iv (RS_Careful),
    const_iv (RS_fontCount),
    const_iv (RS_fontShift),
    const_iv (RS_fontMask),
    const_iv (RS_baseattrMask),
    const_iv (RS_attrMask),
    const_iv (RS_redraw),
    const_iv (RS_Sel),
    const_iv (RS_Bold),
    const_iv (RS_Italic),
    const_iv (RS_Blink),
    const_iv (RS_RVid),
    const_iv (RS_Uline),

    const_iv (CurrentTime),
    const_iv (ShiftMask),
    const_iv (LockMask),
    const_iv (ControlMask),
    const_iv (Mod1Mask),
    const_iv (Mod2Mask),
    const_iv (Mod3Mask),
    const_iv (Mod4Mask),
    const_iv (Mod5Mask),
    const_iv (Button1Mask),
    const_iv (Button2Mask),
    const_iv (Button3Mask),
    const_iv (Button4Mask),
    const_iv (Button5Mask),
    const_iv (AnyModifier),

    const_iv (NoSymbol),
    const_iv (GrabModeSync),
    const_iv (GrabModeAsync),

    const_iv (NoEventMask),
    const_iv (KeyPressMask),
    const_iv (KeyReleaseMask),
    const_iv (ButtonPressMask),
    const_iv (ButtonReleaseMask),
    const_iv (EnterWindowMask),
    const_iv (LeaveWindowMask),
    const_iv (PointerMotionMask),
    const_iv (PointerMotionHintMask),
    const_iv (Button1MotionMask),
    const_iv (Button2MotionMask),
    const_iv (Button3MotionMask),
    const_iv (Button4MotionMask),
    const_iv (Button5MotionMask),
    const_iv (ButtonMotionMask),
    const_iv (KeymapStateMask),
    const_iv (ExposureMask),
    const_iv (VisibilityChangeMask),
    const_iv (StructureNotifyMask),
    const_iv (ResizeRedirectMask),
    const_iv (SubstructureNotifyMask),
    const_iv (SubstructureRedirectMask),
    const_iv (FocusChangeMask),
    const_iv (PropertyChangeMask),
    const_iv (ColormapChangeMask),
    const_iv (OwnerGrabButtonMask),

    const_iv (KeyPress),
    const_iv (KeyRelease),
    const_iv (ButtonPress),
    const_iv (ButtonRelease),
    const_iv (MotionNotify),
    const_iv (EnterNotify),
    const_iv (LeaveNotify),
    const_iv (FocusIn),
    const_iv (FocusOut),
    const_iv (KeymapNotify),
    const_iv (Expose),
    const_iv (GraphicsExpose),
    const_iv (NoExpose),
    const_iv (VisibilityNotify),
    const_iv (CreateNotify),
    const_iv (DestroyNotify),
    const_iv (UnmapNotify),
    const_iv (MapNotify),
    const_iv (MapRequest),
    const_iv (ReparentNotify),
    const_iv (ConfigureNotify),
    const_iv (ConfigureRequest),
    const_iv (GravityNotify),
    const_iv (ResizeRequest),
    const_iv (CirculateNotify),
    const_iv (CirculateRequest),
    const_iv (PropertyNotify),
    const_iv (SelectionClear),
    const_iv (SelectionRequest),
    const_iv (SelectionNotify),
    const_iv (ColormapNotify),
    const_iv (ClientMessage),
    const_iv (MappingNotify),
#   if ENABLE_XIM_ONTHESPOT
    const_iv (XIMReverse),
    const_iv (XIMUnderline),
    const_iv (XIMHighlight),
    const_iv (XIMPrimary),
    const_iv (XIMSecondary),
    const_iv (XIMTertiary),
    const_iv (XIMVisibleToForward),
    const_iv (XIMVisibleToBackword),
    const_iv (XIMVisibleToCenter),
#   if 0
    const_iv (XIMForwardChar),
    const_iv (XIMBackwardChar),
    const_iv (XIMForwardWord),
    const_iv (XIMBackwardWord),
    const_iv (XIMCaretUp),
    const_iv (XIMCaretDown),
    const_iv (XIMNextLine),
    const_iv (XIMPreviousLine),
    const_iv (XIMLineStart),
    const_iv (XIMLineEnd),
    const_iv (XIMAbsolutePosition),
    const_iv (XIMDontChange),
#   endif
#   endif
  };

  for (civ = const_iv + ecb_array_length (const_iv); civ > const_iv; civ--)
    newCONSTSUB (stash, (char *)civ[-1].name, newSViv (civ[-1].iv));
}

void
warn (const char *msg)
	CODE:
        rxvt_warn ("%s", msg);

void
fatal (const char *msg)
	CODE:
        rxvt_fatal ("%s", msg);

void
_exit (int status)

NV
NOW ()
	CODE:
        RETVAL = ev::now ();
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
        RETVAL = SET_FGCOLOR (rend, clamp (new_color, 0, TOTAL_COLORS - 1));
	OUTPUT:
        RETVAL

int
SET_BGCOLOR (int rend, int new_color)
	CODE:
        RETVAL = SET_BGCOLOR (rend, clamp (new_color, 0, TOTAL_COLORS - 1));
	OUTPUT:
        RETVAL

int
GET_CUSTOM (int rend)
	CODE:
        RETVAL = (rend & RS_customMask) >> RS_customShift;
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

void
termlist ()
	PPCODE:
{
        EXTEND (SP, rxvt_term::termlist.size ());

        for (rxvt_term **t = rxvt_term::termlist.begin (); t < rxvt_term::termlist.end (); t++)
          if ((*t)->perl.self)
            PUSHs (sv_2mortal (newSVterm (*t)));
}

IV
_new_selection_request (rxvt_term *term, int selnum, Time tm, Window win, Atom prop, SV *cb)
	CODE:
        rxvt_selection *req = new rxvt_selection (term->display, selnum, tm, win, prop, term);
        req->cb_sv = newSVsv (cb);
        AV *av = (AV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)term->perl.self), "_selection", 10, 0));
        av_push (av, newSViv ((IV)req));
        RETVAL = (IV)req;
	OUTPUT:
        RETVAL

void
_delete_selection_request (IV req_)
	CODE:
        rxvt_selection *req = (rxvt_selection *)req_;
        AV *av = (AV *)SvRV (*hv_fetch ((HV *)SvRV ((SV *)req->term->perl.self), "_selection", 10, 0));
        int i;

        for (i = AvFILL (av); i >= 0; i--)
          if (SvIV (*av_fetch (av, i, 1)) == req_)
            break;

        for (; i < AvFILL (av); i++)
          av_store (av, i, SvREFCNT_inc (*av_fetch (av, i + 1, 0)));

        av_pop (av);

        delete req;

MODULE = urxvt             PACKAGE = urxvt::term

SV *
_new (AV *env, AV *arg)
	CODE:
{
        rxvt_term *term = new rxvt_term;

	stringvec *argv = new stringvec;
        for (int i = 0; i <= AvFILL (arg); i++)
          argv->push_back (strdup (SvPVbyte_nolen (*av_fetch (arg, i, 1))));

	stringvec *envv = new stringvec;
        for (int i = AvFILL (env) + 1; i--; )
          envv->push_back (strdup (SvPVbyte_nolen (*av_fetch (env, i, 1))));

        try
          {
            term->init (argv, envv);
          }
        catch (const class rxvt_failure_exception &e)
          {
            term->destroy ();
            croak ("error while initializing new terminal instance");
          }

        RETVAL = term && term->perl.self
                 ? newSVterm (term) : &PL_sv_undef;
}
	OUTPUT:
        RETVAL

void
rxvt_term::destroy ()

void
rxvt_term::set_should_invoke (int htype, int inc)
	CODE:
        THIS->perl.should_invoke [htype] += inc;

int
rxvt_term::grab_button (int button, U32 modifiers, Window window = THIS->vt)
	CODE:
        RETVAL = XGrabButton (THIS->dpy, button, modifiers, window, 1,
                              ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                              GrabModeSync, GrabModeSync, None, GRAB_CURSOR);
	OUTPUT: RETVAL

int
rxvt_term::ungrab_button (int button, U32 modifiers, Window window = THIS->vt)
	CODE:
        RETVAL = XUngrabButton (THIS->dpy, button, modifiers, window);
	OUTPUT: RETVAL

void
rxvt_term::XGrabKey (int keycode, U32 modifiers, Window window = THIS->vt, \
                     int owner_events = 1, int pointer_mode = GrabModeAsync, int keyboard_mode = GrabModeAsync)
	CODE:
        XGrabKey (THIS->dpy, keycode, modifiers, window, owner_events, pointer_mode, keyboard_mode);

void
rxvt_term::XUngrabKey (int keycode, U32 modifiers, Window window = THIS->vt)
	CODE:
	XUngrabKey (THIS->dpy, keycode, modifiers, window);

bool
rxvt_term::grab (Time eventtime, int sync = 0)
	CODE:
{
        int mode = sync ? GrabModeSync : GrabModeAsync;

        THIS->perl.grabtime = 0;

        if (!XGrabPointer (THIS->dpy, THIS->vt, 0,
                           ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask,
                           mode, mode, None, GRAB_CURSOR, eventtime))
          if (!XGrabKeyboard (THIS->dpy, THIS->vt, 0, mode, mode, eventtime))
            THIS->perl.grabtime = eventtime;
          else
            XUngrabPointer (THIS->dpy, eventtime);

        RETVAL = !!THIS->perl.grabtime;
}
	OUTPUT:
        RETVAL

void
rxvt_term::allow_events_async ()
	CODE:
        XAllowEvents (THIS->dpy, AsyncBoth,      THIS->perl.grabtime);

void
rxvt_term::allow_events_sync ()
	CODE:
        XAllowEvents (THIS->dpy, SyncBoth,       THIS->perl.grabtime);

void
rxvt_term::allow_events_replay ()
	CODE:
        XAllowEvents (THIS->dpy, ReplayPointer,  THIS->perl.grabtime);
        XAllowEvents (THIS->dpy, ReplayKeyboard, THIS->perl.grabtime);

void
rxvt_term::ungrab ()
	CODE:
        ungrab (THIS);

int
rxvt_term::XStringToKeysym (char *string)
	CODE:
        RETVAL = XStringToKeysym (string);
	OUTPUT: RETVAL

char *
rxvt_term::XKeysymToString (int sym)
	CODE:
        RETVAL = XKeysymToString (sym);
	OUTPUT: RETVAL

int
rxvt_term::XKeysymToKeycode (int sym)
	CODE:
        RETVAL = XKeysymToKeycode (THIS->dpy, sym);
	OUTPUT: RETVAL

int
rxvt_term::XKeycodeToKeysym (int code, int index)
	CODE:
        RETVAL = XKeycodeToKeysym (THIS->dpy, code, index);
	OUTPUT: RETVAL

int
rxvt_term::strwidth (SV *str)
	CODE:
{
        wchar_t *wstr = sv2wcs (str);

	rxvt_push_locale (THIS->locale);
        RETVAL = 0;
        for (wchar_t *wc = wstr; *wc; wc++)
          {
            int w = WCWIDTH (*wc);

            if (w)
              RETVAL += max (w, 1);
          }
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

        RETVAL = wcs2sv (wstr);
        free (wstr);
}
	OUTPUT:
        RETVAL

#define TERM_OFFSET(sym) offsetof (TermWin_t, sym)

#define TERM_OFFSET_width       TERM_OFFSET(width)
#define TERM_OFFSET_height      TERM_OFFSET(height)
#define TERM_OFFSET_fwidth      TERM_OFFSET(fwidth)
#define TERM_OFFSET_fheight     TERM_OFFSET(fheight)
#define TERM_OFFSET_fbase       TERM_OFFSET(fbase)
#define TERM_OFFSET_nrow        TERM_OFFSET(nrow)
#define TERM_OFFSET_ncol        TERM_OFFSET(ncol)
#define TERM_OFFSET_focus       TERM_OFFSET(focus)
#define TERM_OFFSET_mapped      TERM_OFFSET(mapped)
#define TERM_OFFSET_int_bwidth  TERM_OFFSET(int_bwidth)
#define TERM_OFFSET_ext_bwidth  TERM_OFFSET(ext_bwidth)
#define TERM_OFFSET_lineSpace   TERM_OFFSET(lineSpace)
#define TERM_OFFSET_letterSpace TERM_OFFSET(letterSpace)
#define TERM_OFFSET_saveLines   TERM_OFFSET(saveLines)
#define TERM_OFFSET_total_rows  TERM_OFFSET(total_rows)
#define TERM_OFFSET_top_row     TERM_OFFSET(top_row)

int
rxvt_term::width ()
	ALIAS:
           width       = TERM_OFFSET_width
           height      = TERM_OFFSET_height
           fwidth      = TERM_OFFSET_fwidth
           fheight     = TERM_OFFSET_fheight
           fbase       = TERM_OFFSET_fbase
           nrow        = TERM_OFFSET_nrow
           ncol        = TERM_OFFSET_ncol
           focus       = TERM_OFFSET_focus
           mapped      = TERM_OFFSET_mapped
           int_bwidth  = TERM_OFFSET_int_bwidth
           ext_bwidth  = TERM_OFFSET_ext_bwidth
           lineSpace   = TERM_OFFSET_lineSpace
           letterSpace = TERM_OFFSET_letterSpace
           saveLines   = TERM_OFFSET_saveLines
           total_rows  = TERM_OFFSET_total_rows
           top_row     = TERM_OFFSET_top_row
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
           current_screen = 3
           hidden_cursor  = 4
	CODE:
        switch (ix)
          {
            case 0: RETVAL = THIS->ModLevel3Mask;  break;
            case 1: RETVAL = THIS->ModMetaMask;    break;
            case 2: RETVAL = THIS->ModNumLockMask; break;
            case 3: RETVAL = THIS->current_screen; break;
#ifdef CURSOR_BLINK
            case 4: RETVAL = THIS->hidden_cursor;  break;
#endif
          }
        OUTPUT:
        RETVAL

char *
rxvt_term::display_id ()
	ALIAS:
           display_id = 0
           locale     = 1
	CODE:
        switch (ix)
          {
            case 0: RETVAL = THIS->display->id; break;
            case 1: RETVAL = THIS->locale;      break;
          }
        OUTPUT:
        RETVAL

SV *
rxvt_term::envv ()
	ALIAS:
        argv = 1
	PPCODE:
{
	stringvec *vec = ix ? THIS->argv : THIS->envv;

        EXTEND (SP, vec->size ());

        for (char **i = vec->begin (); i != vec->end (); ++i)
          if (*i)
            PUSHs (sv_2mortal (newSVpv (*i, 0)));
}

int
rxvt_term::pty_ev_events (int events = ev::UNDEF)
	CODE:
        RETVAL = THIS->pty_ev.events;
        if (events != ev::UNDEF)
          THIS->pty_ev.set (events);
	OUTPUT:
        RETVAL

int
rxvt_term::pty_fd ()
	CODE:
        RETVAL = THIS->pty->pty;
	OUTPUT:
        RETVAL

Window
rxvt_term::parent ()
	CODE:
        RETVAL = THIS->parent;
        OUTPUT:
        RETVAL

Window
rxvt_term::vt ()
	CODE:
        RETVAL = THIS->vt;
        OUTPUT:
        RETVAL

void
rxvt_term::vt_emask_add (U32 emask)
	CODE:
        THIS->vt_emask_perl |= emask;
        THIS->vt_select_input ();

U32
rxvt_term::rstyle (U32 new_rstyle = THIS->rstyle)
	CODE:
        RETVAL = THIS->rstyle;
        THIS->rstyle = new_rstyle;
        OUTPUT:
	RETVAL

int
rxvt_term::view_start (int newval = 1)
	PROTOTYPE: $;$
	CODE:
{
        RETVAL = THIS->view_start;

        if (newval <= 0)
          THIS->scr_changeview (max (newval, THIS->top_row));
}
        OUTPUT:
	RETVAL

void
rxvt_term::set_urgency (bool enable)

void
rxvt_term::focus_in ()

void
rxvt_term::focus_out ()

void
rxvt_term::key_press (unsigned int state, unsigned int keycode, Time time = CurrentTime)
	ALIAS:
           key_release = 1
	CODE:
{
        XKeyEvent xkey;

        memset (&xkey, 0, sizeof (xkey));

        xkey.time      = time;
        xkey.state     = state;
        xkey.keycode   = keycode;

        xkey.type      = ix ? KeyRelease : KeyPress;
        xkey.display   = THIS->dpy;
        xkey.window    = THIS->vt;
        xkey.root      = THIS->display->root;
        xkey.subwindow = THIS->vt;

        if (ix)
          THIS->key_release (xkey);
        else
          THIS->key_press (xkey);
}

void
rxvt_term::want_refresh ()
	CODE:
        THIS->want_refresh = 1;
	THIS->refresh_check ();

void
rxvt_term::ROW_t (int row_number, SV *new_text = 0, int start_col = 0, int start_ofs = 0, int max_len = MAX_COLS)
	PPCODE:
{
        if (!IN_RANGE_EXC (row_number, THIS->top_row, THIS->nrow))
          XSRETURN_EMPTY;

        line_t &l = ROW(row_number);

        if (GIMME_V != G_VOID)
          {
            wchar_t *wstr = rxvt_temp_buf<wchar_t> (THIS->ncol);

            for (int col = 0; col < THIS->ncol; col++)
              wstr [col] = l.t [col];

            XPUSHs (sv_2mortal (wcs2sv (wstr, THIS->ncol)));
          }

        if (new_text)
          {
            wchar_t *wstr = sv2wcs (new_text);

            int len = min (wcslen (wstr) - start_ofs, max_len);

            if (start_col < 0 || start_col + len > THIS->ncol)
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
        if (!IN_RANGE_EXC (row_number, THIS->top_row, THIS->nrow))
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
            int len = min (AvFILL (av) + 1 - start_ofs, max_len);

            if (start_col < 0 || start_col + len > THIS->ncol)
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
        if (!IN_RANGE_EXC (row_number, THIS->top_row, THIS->nrow))
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
        if (!IN_RANGE_EXC (row_number, THIS->top_row, THIS->nrow))
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
        wchar_t *rstr = rxvt_temp_buf<wchar_t> (wlen * 2); // cannot become longer

	rxvt_push_locale (THIS->locale);

        wchar_t *r = rstr;
        for (wchar_t *s = wstr; *s; s++)
          {
            int w = WCWIDTH (*s);

            if (w == 0)
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

            // the *2 above only allows wcwidth <= 2
            if (w > 1)
              *r++ = NOCHAR;
          }

	rxvt_pop_locale ();

        RETVAL = wcs2sv (rstr, r - rstr);
}
	OUTPUT:
        RETVAL

SV *
rxvt_term::special_decode (SV *text)
	CODE:
{
        wchar_t *wstr = sv2wcs (text);
        int dlen = 0;

        // find length
        for (wchar_t *s = wstr; *s; s++)
          if (*s == NOCHAR)
            ;
          else if (IS_COMPOSE (*s))
            dlen += rxvt_composite.expand (*s, 0);
          else
            dlen++;

        wchar_t *rstr = rxvt_temp_buf<wchar_t> (dlen);

        // decode
        wchar_t *r = rstr;
        for (wchar_t *s = wstr; *s; s++)
          if (*s == NOCHAR)
            ;
          else if (IS_COMPOSE (*s))
            r += rxvt_composite.expand (*s, r);
          else
            *r++ = *s;

        RETVAL = wcs2sv (rstr, r - rstr);
}
	OUTPUT:
        RETVAL

void
rxvt_term::_resource (char *name, int index, SV *newval = 0)
	PPCODE:
{
	static const struct resval { const char *name; int value; } *rs, rslist [] = {
#	  define def(name) { # name, Rs_ ## name },
#	  define reserve(name,count)
#	  include "rsinc.h"
#	  undef def
#	  undef reserve
        };

        rs = rslist + ecb_array_length (rslist);

        if (*name)
          {
            do {
              if (rs-- == rslist)
                croak ("no such resource '%s', requested", name);
            } while (strcmp (name, rs->name));

            index += rs->value;
          }
        else
          {
            --rs;
            name = "";
          }

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

const char *
rxvt_term::x_resource (const char *name)

bool
rxvt_term::option (U8 optval, int set = -1)
	CODE:
{
	RETVAL = THIS->option (optval);

        if (set >= 0)
          {
            THIS->set_option (optval, set);

            if (THIS->init_done) // avoid doing this before START
              switch (optval)
                {
                  case Opt_skipBuiltinGlyphs:
                    THIS->set_fonts ();
                    THIS->scr_remap_chars ();
                    THIS->scr_touch (true);
                    THIS->want_refresh = 1;
                    THIS->refresh_check ();
                    break;

                  case Opt_cursorUnderline:
                    THIS->want_refresh = 1;
                    THIS->refresh_check ();
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

bool
rxvt_term::parse_keysym (char *keysym, char *str)
	CODE:
        RETVAL = 0 < THIS->parse_keysym (keysym, str);
        THIS->keyboard->register_done ();
	OUTPUT:
        RETVAL

void
rxvt_term::register_command (int keysym, unsigned int state, SV *str)
        CODE:
        wchar_t *wstr = sv2wcs (str);
        THIS->keyboard->register_user_translation (keysym, state, wstr);
        free (wstr);

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

        if (items >= 3)
          {
            rc.row = SvIV (ST (1));
            rc.col = SvIV (ST (2));

            if (ix == 2)
              {
                if (rc.col == 0)
                  {
                    // col == 0 means end of previous line
                    rc.row--;
                    rc.col = THIS->ncol;
                  }
                else if (IN_RANGE_EXC (rc.row, THIS->top_row, THIS->nrow)
                         && rc.col > ROW(rc.row).l)
                  {
                    // col >= length means while line and add newline
                    rc.col = THIS->ncol;
                  }
              }

            clamp_it (rc.col, 0, THIS->ncol);
            clamp_it (rc.row, THIS->top_row, THIS->nrow - 1);

            if (ix)
              {
                THIS->selection.screen = THIS->current_screen;

                THIS->want_refresh = 1;
                THIS->refresh_check ();
              }
          }
}

int
rxvt_term::selection_screen (int screen = -1)
	CODE:
        RETVAL = THIS->selection.screen;
        if (screen >= 0)
          THIS->selection.screen = screen;
        OUTPUT:
        RETVAL

void
rxvt_term::selection_clear (bool clipboard = false)

void
rxvt_term::selection_make (Time eventtime, bool rect = false)
	CODE:
        THIS->selection.op = SELECTION_CONT;
        THIS->selection.rect = rect;
        THIS->selection_make (eventtime);

int
rxvt_term::selection_grab (Time eventtime, bool clipboard = false)

void
rxvt_term::selection (SV *newtext = 0, bool clipboard = false)
        PPCODE:
{
        wchar_t * &text   = clipboard ? THIS->selection.clip_text : THIS->selection.text;
        unsigned int &len = clipboard ? THIS->selection.clip_len  : THIS->selection.len;

        if (GIMME_V != G_VOID)
          XPUSHs (text
                  ? sv_2mortal (wcs2sv (text, len))
                  : &PL_sv_undef);

        if (newtext)
          {
            free (text);

            text = sv2wcs (newtext);
            len = wcslen (text);
          }
}

char
rxvt_term::cur_charset ()
	CODE:
        RETVAL = THIS->charsets [THIS->screen.charset];
	OUTPUT:
        RETVAL

void
rxvt_term::scr_xor_rect (int beg_row, int beg_col, int end_row, int end_col, U32 rstyle1 = RS_RVid, U32 rstyle2 = RS_RVid | RS_Uline)

void
rxvt_term::scr_xor_span (int beg_row, int beg_col, int end_row, int end_col, U32 rstyle = RS_RVid)

void
rxvt_term::scr_bell ()

void
rxvt_term::scr_change_screen (int screen)

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
rxvt_term::tt_paste (SV *octets)
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

#############################################################################
# Various X Utility Functions
#############################################################################

void
rxvt_term::XListProperties (Window window)
	PPCODE:
{
	int count;
	Atom *props = XListProperties (THIS->dpy, window, &count);

        EXTEND (SP, count);
        while (count--)
          PUSHs (newSVuv ((U32)props [count]));

        XFree (props);
}

void
rxvt_term::XGetWindowProperty (Window window, Atom property)
	PPCODE:
{
        Atom type;
        int format;
        unsigned long nitems;
        unsigned long bytes_after;
        unsigned char *prop;

	XGetWindowProperty (THIS->dpy, window, property,
                            0, 1<<24, 0, AnyPropertyType,
                            &type, &format, &nitems, &bytes_after, &prop);

        if (type != None)
          {
            int elemsize = format == 16 ? sizeof (short)
                         : format == 32 ? sizeof (long)
                         :                1;

            EXTEND (SP, 3);
            PUSHs (newSVuv ((U32)type));
            PUSHs (newSViv (format));
            PUSHs (newSVpvn ((char *)prop, nitems * elemsize));
            XFree (prop);
          }
}

void
rxvt_term::XChangeProperty (Window window, Atom property, Atom type, int format, SV *data)
	CODE:
{
	STRLEN len;
        char *data_ = SvPVbyte (data, len);

        int elemsize = format == 16 ? sizeof (short)
                     : format == 32 ? sizeof (long)
                     :                1;

	XChangeProperty (THIS->dpy, window, property,
                         type, format, PropModeReplace,
                         (unsigned char *)data_, len / elemsize);
}

Atom
XInternAtom (rxvt_term *term, char *atom_name, int only_if_exists = FALSE)
	C_ARGS: term->dpy, atom_name, only_if_exists

char *
XGetAtomName (rxvt_term *term, Atom atom)
	C_ARGS: term->dpy, atom
        CLEANUP:
        XFree (RETVAL);

void
XDeleteProperty (rxvt_term *term, Window window, Atom property)
  	C_ARGS: term->dpy, window, property

Window
rxvt_term::DefaultRootWindow ()
	CODE:
        RETVAL = THIS->display->root;
        OUTPUT:
        RETVAL

#if 0

Window
XCreateSimpleWindow (rxvt_term *term, Window parent, int x, int y, unsigned int width, unsigned int height)
	C_ARGS: term->dpy, (Window)parent,
                x, y, width, height, 0,
                term->pix_colors_focused[Color_border],
                term->pix_colors_focused[Color_border]

#endif

void
XReparentWindow (rxvt_term *term, Window window, Window parent, int x = 0, int y = 0)
	C_ARGS: term->dpy, window, parent, x, y

void
XMapWindow (rxvt_term *term, Window window)
	C_ARGS: term->dpy, window

void
XUnmapWindow (rxvt_term *term, Window window)
	C_ARGS: term->dpy, window

void
XMoveResizeWindow (rxvt_term *term, Window window, int x, int y, unsigned int width, unsigned int height)
	C_ARGS: term->dpy, window, x, y, width, height

void
rxvt_term::XChangeInput (Window window, U32 add_events, U32 del_events = 0)
	CODE:
{
	XWindowAttributes attr;
        XGetWindowAttributes (THIS->dpy, window, &attr);
        XSelectInput (THIS->dpy, window, attr.your_event_mask | add_events & ~del_events);
}

void
rxvt_term::XTranslateCoordinates (Window src, Window dst, int x, int y)
	PPCODE:
{
        int dx, dy;
        Window child;

        if (XTranslateCoordinates (THIS->dpy, src, dst, x, y, &dx, &dy, &child))
          {
            EXTEND (SP, 3);
            PUSHs (newSViv (dx));
            PUSHs (newSViv (dy));
            PUSHs (newSVuv (child));
          }
}

#############################################################################
# urxvt::overlay
#############################################################################

MODULE = urxvt             PACKAGE = urxvt::overlay

void
overlay::set (int x, int y, SV *text, SV *rend = 0)

void
overlay::show ()

void
overlay::hide ()

void
overlay::DESTROY ()

INCLUDE: $PERL <iom_perl.xs -pe s/IOM_MODULE/urxvt/g,s/IOM_CLASS/urxvt/g |

