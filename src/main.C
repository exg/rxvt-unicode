/*----------------------------------------------------------------------*
 * File:        main.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1992      John Bovey, University of Kent at Canterbury <jdb@ukc.ac.uk>
 *                              - original version
 * Copyright (c) 1994      Robert Nation <nation@rocket.sanders.lockheed.com>
 *                              - extensive modifications
 * Copyright (c) 1995      Garrett D'Amore <garrett@netcom.com>
 * Copyright (c) 1997      mj olesen <olesen@me.QueensU.CA>
 *                              - extensive modifications
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 *                              - extensive modifications
 * Copyright (c) 2003-2010 Marc Lehmann <schmorp@schmorp.de>
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
 *---------------------------------------------------------------------*/

#include "../config.h"          /* NECESSARY */
#include "rxvt.h"               /* NECESSARY */
#include "init.h"
#include "keyboard.h"
#include "rxvtperl.h"

#include <limits>

#include <assert.h>
#include <signal.h>
#include <string.h>

#include <termios.h>

#ifdef HAVE_XSETLOCALE
# define X_LOCALE
# include <X11/Xlocale.h>
#else
# include <locale.h>
#endif

struct termios rxvt_term::def_tio;
vector<rxvt_term *> rxvt_term::termlist;

// used to tell global functions which terminal instance is "active"
rxvt_t rxvt_current_term;

static char curlocale[128], savelocale[128];

bool
rxvt_set_locale (const char *locale) NOTHROW
{
  int size = strlen (locale) + 1;

  if (size > sizeof (curlocale))
    rxvt_fatal ("locale string too long, aborting.\n");

  if (!locale || !memcmp (locale, curlocale, size))
    return false;

  memcpy (curlocale, locale, size);
  setlocale (LC_CTYPE, curlocale);
  return true;
}

void
rxvt_push_locale (const char *locale) NOTHROW
{
  strcpy (savelocale, curlocale);
  rxvt_set_locale (locale);
}

void
rxvt_pop_locale () NOTHROW
{
  rxvt_set_locale (savelocale);
}

#if ENABLE_COMBINING
class rxvt_composite_vec rxvt_composite;

text_t rxvt_composite_vec::compose (unicode_t c1, unicode_t c2)
{
  compose_char *cc;

  // break compose chains, as stupid readline really likes to duplicate
  // composing characters for some reason, near the end of a line.
  cc = (*this)[c1];
  while (cc)
    {
      if (cc->c2 == c2) return c1;
      cc = (*this)[cc->c1];
    }

  // check to see whether this combination already exists otherwise
  for (cc = v.begin (); cc < v.end (); cc++)
    if (cc->c1 == c1 && cc->c2 == c2)
      return COMPOSE_LO + (cc - v.begin ());

  // allocate a new combination
  if (v.size () == COMPOSE_HI - COMPOSE_LO + 1)
    {
      static int seen;

      if (!seen++)
        rxvt_warn ("too many unrepresentable composite characters, try --enable-unicode3\n");

      return REPLACEMENT_CHAR;
    }

  v.push_back (compose_char (c1, c2));

  return v.size () - 1 + COMPOSE_LO;
}

int rxvt_composite_vec::expand (unicode_t c, wchar_t *r)
{
  compose_char *cc = (*this)[c];

  if (!cc)
    {
      if (r) *r = c;
      return 1;
    }

  int len = expand (cc->c1, r);

  if (r) r += len;

  if (cc->c2 != NOCHAR)
    {
      len++;
      if (r) *r++ = cc->c2;
    }

  return len;

}
#endif

rxvt_term::rxvt_term ()
{
#if HAVE_BG_PIXMAP
  update_background_ev.set<rxvt_term, &rxvt_term::update_background_cb> (this);
#endif
#ifdef CURSOR_BLINK
  cursor_blink_ev.set     <rxvt_term, &rxvt_term::cursor_blink_cb> (this); cursor_blink_ev.set (0., CURSOR_BLINK_INTERVAL);
#endif
#ifdef TEXT_BLINK
  text_blink_ev.set       <rxvt_term, &rxvt_term::text_blink_cb>   (this); text_blink_ev.set (0., TEXT_BLINK_INTERVAL);
#endif
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
  cont_scroll_ev.set      <rxvt_term, &rxvt_term::cont_scroll_cb>  (this);
#endif
#ifdef SELECTION_SCROLLING
  sel_scroll_ev.set       <rxvt_term, &rxvt_term::sel_scroll_cb>   (this);
#endif
#if defined(MOUSE_WHEEL) && defined(MOUSE_SLIP_WHEELING)
  slip_wheel_ev.set       <rxvt_term, &rxvt_term::slip_wheel_cb>   (this);
#endif
#if ENABLE_TRANSPARENCY || ENABLE_PERL
  rootwin_ev.set          <rxvt_term, &rxvt_term::rootwin_cb> (this),
#endif
  scrollbar_ev.set        <rxvt_term, &rxvt_term::x_cb>       (this),
#ifdef USE_XIM
  im_ev.set               <rxvt_term, &rxvt_term::im_cb>      (this),
#endif
#ifdef POINTER_BLANK
  pointer_ev.set          <rxvt_term, &rxvt_term::pointer_cb> (this);
#endif
#ifndef NO_BELL
  bell_ev.set             <rxvt_term, &rxvt_term::bell_cb>    (this);
#endif
  child_ev.set            <rxvt_term, &rxvt_term::child_cb>   (this);
  flush_ev.set            <rxvt_term, &rxvt_term::flush_cb>   (this);
  destroy_ev.set          <rxvt_term, &rxvt_term::destroy_cb> (this);
  pty_ev.set              <rxvt_term, &rxvt_term::pty_cb>     (this);
  termwin_ev.set          <rxvt_term, &rxvt_term::x_cb>       (this);
  vt_ev.set               <rxvt_term, &rxvt_term::x_cb>       (this);

  cmdbuf_ptr = cmdbuf_endp = cmdbuf_base;

  termlist.push_back (this);

#ifdef KEYSYM_RESOURCE
  keyboard = new keyboard_manager;
#endif
}

// clean up the most important stuff, do *not* call x or free mem etc.
// for use before an emergency exit
void
rxvt_term::emergency_cleanup ()
{
  if (cmd_pid)
    kill (-cmd_pid, SIGHUP);

  pty_ev.stop ();
  delete pty; pty = 0;
}

rxvt_term::~rxvt_term ()
{
  termlist.erase (find (termlist.begin (), termlist.end(), this));

  emergency_cleanup ();

#if ENABLE_STYLES
  for (int i = RS_styleCount; --i; )
    if (fontset[i] != fontset[0])
      delete fontset[i];
#endif
  delete fontset[0];

#ifdef HAVE_BG_PIXMAP
  bg_destroy ();
#endif

  if (display)
    {
      selection_clear ();
      selection_clear (true);

#ifdef USE_XIM
      im_destroy ();
#endif
      scrollBar.destroy ();
      if (gc)   XFreeGC (dpy, gc);

      delete drawable;
      // destroy all windows
      if (parent)
        XDestroyWindow (dpy, parent);

      for (int i = 0; i < TOTAL_COLORS; i++)
        if (ISSET_PIXCOLOR (i))
          {
            pix_colors_focused   [i].free (this);
#if OFF_FOCUS_FADING
            pix_colors_unfocused [i].free (this);
#endif
          }

      clear ();

      display->flush (); /* ideally .put should do this */
      displays.put (display);
    }

  scr_release ();

  /* clear all resources */
  for (int i = 0; i < allocated.size (); i++)
    free (allocated [i]);

  free (selection.text);
  free (selection.clip_text);
  free (locale);
  free (v_buffer);

  delete selection_req;

  delete envv;
  delete argv;

#ifdef KEYSYM_RESOURCE
  delete keyboard;
#endif
#ifndef NO_RESOURCES
  XrmDestroyDatabase (option_db);
#endif
}

// child has exited, usually destroys
void
rxvt_term::child_cb (ev::child &w, int status)
{
  HOOK_INVOKE ((this, HOOK_CHILD_EXIT, DT_INT, status, DT_END));

  cmd_pid = 0;

  if (!option (Opt_hold))
    destroy ();
}

void
rxvt_term::destroy ()
{
  if (destroy_ev.is_active ())
    return;

  HOOK_INVOKE ((this, HOOK_DESTROY, DT_END));

#if ENABLE_OVERLAY
  scr_overlay_off ();
#endif

  if (display)
    {
#if USE_XIM
      im_ev.stop (display);
#endif
      scrollbar_ev.stop (display);
#if ENABLE_TRANSPARENCY || ENABLE_PERL
      rootwin_ev.stop (display);
#endif
      termwin_ev.stop (display);
      vt_ev.stop (display);
    }

  flush_ev.stop ();
  pty_ev.stop ();
#ifdef CURSOR_BLINK
  cursor_blink_ev.stop ();
#endif
#ifdef TEXT_BLINK
  text_blink_ev.stop ();
#endif
#ifndef NO_SCROLLBAR_BUTTON_CONTINUAL_SCROLLING
  cont_scroll_ev.stop ();
#endif
#ifdef SELECTION_SCROLLING
  sel_scroll_ev.stop ();
#endif
#ifdef POINTER_BLANK
  pointer_ev.stop ();
#endif

  destroy_ev.start ();
}

void
rxvt_term::destroy_cb (ev::idle &w, int revents)
{
  make_current ();

  delete this;
}

void
rxvt_term::set_option (uint8_t opt, bool set) NOTHROW
{
  if (!opt)
    return;

  uint8_t mask = 1 << (opt & 7);
  uint8_t &val = options [opt >> 3];

  val = val & ~mask | (set ? mask : 0);
}

/*----------------------------------------------------------------------*/
/*
 * Exit gracefully, clearing the utmp entry and restoring tty attributes
 * TODO: if debugging, this should free up any known resources if we can
 */
static XErrorHandler old_xerror_handler;

static void
rxvt_emergency_cleanup ()
{
  for (rxvt_term **t = rxvt_term::termlist.begin (); t < rxvt_term::termlist.end (); t++)
    (*t)->emergency_cleanup ();
}

#if !ENABLE_MINIMAL
static void
print_x_error (Display *dpy, XErrorEvent *event)
{
    char buffer[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    const char mtype[] = "XlibMessage";
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    rxvt_warn ("An X Error occurred, trying to continue after report.\n");
    rxvt_warn ("%s:  %s\n", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", mesg, BUFSIZ);
    rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->request_code);
    sprintf(number, "%d", event->request_code);
    XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
    rxvt_warn ("(which is %s)\n", buffer);
    if (event->request_code >= 128) {
        XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code %d",
                              mesg, BUFSIZ);
        rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->minor_code);
    }
    if ((event->error_code == BadWindow) ||
               (event->error_code == BadPixmap) ||
               (event->error_code == BadCursor) ||
               (event->error_code == BadFont) ||
               (event->error_code == BadDrawable) ||
               (event->error_code == BadColor) ||
               (event->error_code == BadGC) ||
               (event->error_code == BadIDChoice) ||
               (event->error_code == BadValue) ||
               (event->error_code == BadAtom)) {
        if (event->error_code == BadValue)
            XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x",
                                  mesg, BUFSIZ);
        else if (event->error_code == BadAtom)
            XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
                                  mesg, BUFSIZ);
        else
            XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
                                  mesg, BUFSIZ);
        rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->resourceid);
    }
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d",
                          mesg, BUFSIZ);
    rxvt_warn (strncat (mesg, "\n", BUFSIZ), event->serial);
}
#endif

int
rxvt_xerror_handler (Display *display, XErrorEvent *event)
{
  if (GET_R->allowedxerror == -1)
    GET_R->allowedxerror = event->error_code;
  else
    {
      // GET_R is most likely not the terminal which caused the error,
      // so just output the error and continue
#if ENABLE_MINIMAL
      old_xerror_handler (display, event);
#else
      print_x_error (display, event);
#endif
    }

  return 0;
}

int
rxvt_xioerror_handler (Display *display)
{
  rxvt_warn ("X connection to '%s' broken, unable to recover, exiting.\n",
             DisplayString (display));
  rxvt_emergency_cleanup ();
  _exit (EXIT_FAILURE);
}

static struct sig_handlers
{
  ev::sig sw_term, sw_int;

  /*
   * Catch a fatal signal and tidy up before quitting
   */
  void sig_term (ev::sig &w, int revents);

  sig_handlers ()
  {
    sw_term.set<sig_handlers, &sig_handlers::sig_term> (this);
    sw_int .set<sig_handlers, &sig_handlers::sig_term> (this);
  }
} sig_handlers;

void
sig_handlers::sig_term (ev::sig &w, int revents)
{
  rxvt_emergency_cleanup ();
  w.stop ();
  kill (getpid (), w.signum);
}

static void
rxvt_get_ttymode (struct termios *tio)
{
  if (tcgetattr (STDIN_FILENO, tio) < 0)
    memset (tio, 0, sizeof (struct termios));

  for (int i = 0; i < NCCS; i++)
    tio->c_cc[i] = VDISABLE;

  tio->c_cc[VINTR] = CINTR;
  tio->c_cc[VQUIT] = CQUIT;
  tio->c_cc[VERASE] = CERASE;
#ifdef VERASE2
  tio->c_cc[VERASE2] = CERASE2;
#endif
  tio->c_cc[VKILL] = CKILL;
  tio->c_cc[VEOF] = CEOF;
  tio->c_cc[VSTART] = CSTART;
  tio->c_cc[VSTOP] = CSTOP;
  tio->c_cc[VSUSP] = CSUSP;
# ifdef VDSUSP
  tio->c_cc[VDSUSP] = CDSUSP;
# endif
# ifdef VREPRINT
  tio->c_cc[VREPRINT] = CRPRNT;
# endif
# ifdef VDISCRD
  tio->c_cc[VDISCRD] = CFLUSH;
# endif
# ifdef VWERSE
  tio->c_cc[VWERSE] = CWERASE;
# endif
# ifdef VLNEXT
  tio->c_cc[VLNEXT] = CLNEXT;
# endif
# ifdef VSTATUS
  tio->c_cc[VSTATUS] = CSTATUS;
# endif

# if VMIN != VEOF
  tio->c_cc[VMIN] = 1;
# endif
# if VTIME != VEOL
  tio->c_cc[VTIME] = 0;
# endif

  /* input modes */
  tio->c_iflag = (BRKINT | IGNPAR | ICRNL
# ifdef IMAXBEL
                  | IMAXBEL
# endif
                  | IXON);

  /* output modes */
  tio->c_oflag = (OPOST | ONLCR);

  /* control modes */
  tio->c_cflag = (CS8 | CREAD);

  /* local modes */
  tio->c_lflag = (ISIG | ICANON | IEXTEN | ECHO
# if defined (ECHOCTL) && defined (ECHOKE)
                  | ECHOCTL | ECHOKE
# endif
                  | ECHOE | ECHOK);
}

char **rxvt_environ; // startup environment

void
rxvt_init ()
{
  assert (("fontMask must not overlap other RS masks",
           0 == (RS_fontMask & (RS_Sel | RS_baseattrMask | RS_customMask | RS_bgMask | RS_fgMask))));

  rxvt_get_ttymode (&rxvt_term::def_tio);

  // get rid of stdin/stdout as we don't need them, to free resources
  dup2 (STDERR_FILENO, STDIN_FILENO);
  dup2 (STDERR_FILENO, STDOUT_FILENO);

  if (!ev_default_loop ())
    rxvt_fatal ("cannot initialise libev (bad value for LIBEV_METHODS?)\n");

  rxvt_environ = environ;

  signal (SIGHUP,  SIG_IGN);
  signal (SIGPIPE, SIG_IGN);

  sig_handlers.sw_term.start (SIGTERM); ev_unref ();
  sig_handlers.sw_int.start  (SIGINT);  ev_unref ();

  old_xerror_handler = XSetErrorHandler ((XErrorHandler) rxvt_xerror_handler);
  // TODO: handle this with exceptions and tolerate the memory loss
  XSetIOErrorHandler (rxvt_xioerror_handler);

  XrmInitialize ();
}

/*----------------------------------------------------------------------*/
/*
 * window size/position calculations for XSizeHint and other storage.
 * if width/height are non-zero then override calculated width/height
 */
void
rxvt_term::window_calc (unsigned int newwidth, unsigned int newheight)
{
  short recalc_x, recalc_y;
  int x, y, flags;
  unsigned int w, h;
  unsigned int max_width, max_height;

  szHint.flags = PMinSize | PResizeInc | PBaseSize | PWinGravity;
  szHint.win_gravity = NorthWestGravity;
  /* szHint.min_aspect.x = szHint.min_aspect.y = 1; */

  recalc_x = recalc_y = 0;
  flags = 0;

  if (!parsed_geometry)
    {
      parsed_geometry = 1;

      if (rs[Rs_geometry])
        flags = XParseGeometry (rs[Rs_geometry], &x, &y, &w, &h);

      if (flags & WidthValue)
        {
          if (!w)
            rxvt_fatal ("illegal window geometry (width and height must be non-zero), aborting.\n");

          ncol = clamp (w, 1, std::numeric_limits<int16_t>::max ());
          szHint.flags |= USSize;
        }

      if (flags & HeightValue)
        {
          if (!h)
            rxvt_fatal ("illegal window geometry (width and height must be non-zero), aborting.\n");

          nrow = clamp (h, 1, std::numeric_limits<int16_t>::max ());
          szHint.flags |= USSize;
        }

      if (flags & XValue)
        {
          szHint.x = x;
          szHint.flags |= USPosition;

          if (flags & XNegative)
            {
              recalc_x = 1;
              szHint.win_gravity = NorthEastGravity;
            }
        }

      if (flags & YValue)
        {
          szHint.y = y;
          szHint.flags |= USPosition;

          if (flags & YNegative)
            {
              recalc_y = 1;

              if (szHint.win_gravity == NorthEastGravity)
                szHint.win_gravity = SouthEastGravity;
              else
                szHint.win_gravity = SouthWestGravity;
            }
        }
    }

  /* TODO: BOUNDS */
  width = ncol * fwidth;
  height = nrow * fheight;
  max_width = MAX_COLS * fwidth;
  max_height = MAX_ROWS * fheight;

  szHint.base_width = szHint.base_height = 2 * int_bwidth;

  window_vt_x = window_vt_y = int_bwidth;

  if (scrollBar.state)
    {
      int sb_w = scrollBar.total_width ();
      szHint.base_width += sb_w;

      if (!option (Opt_scrollBar_right))
        window_vt_x += sb_w;
    }

  szHint.width_inc  = fwidth;
  szHint.height_inc = fheight;
  szHint.min_width  = szHint.base_width + szHint.width_inc;
  szHint.min_height = szHint.base_height + szHint.height_inc;

  if (newwidth && newwidth - szHint.base_width < max_width)
    {
      szHint.width = newwidth;
      width = newwidth - szHint.base_width;
    }
  else
    {
      min_it (width, max_width);
      szHint.width = szHint.base_width + width;
    }

  if (newheight && newheight - szHint.base_height < max_height)
    {
      szHint.height = newheight;
      height = newheight - szHint.base_height;
    }
  else
    {
      min_it (height, max_height);
      szHint.height = szHint.base_height + height;
    }

  if (recalc_x)
    szHint.x += DisplayWidth  (dpy, display->screen) - szHint.width  - 2 * ext_bwidth;

  if (recalc_y)
    szHint.y += DisplayHeight (dpy, display->screen) - szHint.height - 2 * ext_bwidth;

  ncol = width  / fwidth;
  nrow = height / fheight;

  // When the size of the vt window is not a multiple of the cell
  // size, i.e., when the wm does not honour our size hints, there are
  // extra areas not covered by the terminal screen. Such gaps, when a
  // bg pixmap is set, would have to be cleared manually to properly
  // refresh the background. We take the simpler route and shrink the
  // vt window so as to avoid creating gaps.
  vt_width  = ncol * fwidth;
  vt_height = nrow * fheight;
}

/*----------------------------------------------------------------------*/
/*
 * Tell the teletype handler what size the window is.
 * Called after a window size change.
 */
void
rxvt_term::tt_winch ()
{
  if (pty->pty < 0)
    return;

  struct winsize ws;

  ws.ws_col = ncol;
  ws.ws_row = nrow;
  ws.ws_xpixel = vt_width;
  ws.ws_ypixel = vt_height;
  ioctl (pty->pty, TIOCSWINSZ, &ws);

#if 0
  // TIOCSWINSZ is supposed to do this automatically and correctly
  if (cmd_pid)               /* force through to the command */
    kill (-cmd_pid, SIGWINCH);
#endif
}

/*----------------------------------------------------------------------*/
/* load and set the various fonts */
bool
rxvt_term::set_fonts ()
{
  rxvt_fontset *fs = new rxvt_fontset (this);
  rxvt_fontprop prop;

  if (!fs
      || !fs->populate (rs[Rs_font] ? rs[Rs_font] : "fixed")
      || !fs->realize_font (1))
    {
      delete fs;
      return false;
    }

#if ENABLE_STYLES
  for (int i = RS_styleCount; --i; )
    if (fontset[i] != fontset[0])
      delete fontset[i];
#endif

  delete fontset[0];
  fontset[0] = fs;

  prop = (*fs)[rxvt_fontset::firstFont]->properties ();
  prop.height += lineSpace;
  prop.width += letterSpace;

  fs->set_prop (prop, false);

  fwidth  = prop.width;
  fheight = prop.height;
  fbase   = prop.ascent;

  for (int style = 1; style < 4; style++)
    {
#if ENABLE_STYLES
      const char *res = rs[Rs_font + style];

      if (res && !*res)
        fontset[style] = fontset[0];
      else
        {
          fontset[style] = fs = new rxvt_fontset (this);
          rxvt_fontprop prop2 = prop;

          if (res)
            {
              fs->populate (res);
              fs->set_prop (prop2, false);
            }
          else
            {
              fs->populate (fontset[0]->fontdesc);

              if (SET_STYLE (0, style) & RS_Bold)   prop2.weight = rxvt_fontprop::bold;
              if (SET_STYLE (0, style) & RS_Italic) prop2.slant  = rxvt_fontprop::italic;

              fs->set_prop (prop2, true);
            }

        }
#else
      fontset[style] = fontset[0];
#endif
    }

  if (parent)
    {
      resize_all_windows (0, 0, 0);
      scr_remap_chars ();
      scr_touch (true);
    }

  return true;
}

void
rxvt_term::set_string_property (Atom prop, const char *str, int len)
{
  XChangeProperty (dpy, parent,
                   prop, XA_STRING, 8, PropModeReplace,
                   (const unsigned char *)str, len >= 0 ? len : strlen (str));
}

void
rxvt_term::set_mbstring_property (Atom prop, const char *str, int len)
{
  XTextProperty ct;

  if (XmbTextListToTextProperty (dpy, (char **)&str, 1, XStdICCTextStyle, &ct) >= 0)
    {
      XSetTextProperty (dpy, parent, &ct, prop);
      XFree (ct.value);
    }
}

void
rxvt_term::set_utf8_property (Atom prop, const char *str, int len)
{
  wchar_t *ws = rxvt_mbstowcs (str, len);
  char *s = rxvt_wcstoutf8 (ws);

  XChangeProperty (dpy, parent,
                   prop, xa[XA_UTF8_STRING], 8, PropModeReplace,
                   (const unsigned char *)s, strlen (s));

  free (s);
  free (ws);
}

/*----------------------------------------------------------------------*/
/*----------------------------------------------------------------------*/
/* xterm sequences - title, iconName, color (exptl) */
void
rxvt_term::set_title (const char *str)
{
  set_mbstring_property (XA_WM_NAME, str);
#if ENABLE_EWMH
  set_utf8_property (xa[XA_NET_WM_NAME], str);
#endif
}

void
rxvt_term::set_icon_name (const char *str)
{
  set_mbstring_property (XA_WM_ICON_NAME, str);
#if ENABLE_EWMH
  set_utf8_property (xa[XA_NET_WM_ICON_NAME], str);
#endif
}

void
rxvt_term::set_window_color (int idx, const char *color)
{
#ifdef XTERM_COLOR_CHANGE
  rxvt_color xcol;

  if (color == NULL || *color == '\0')
    return;

  color = strdup (color);
  allocated.push_back ((void *)color);
  rs[Rs_color + idx] = color;

  /* handle color aliases */
  if (isdigit (*color))
    {
      int i = atoi (color);

      if (i >= 8 && i <= 15)
        {
          /* bright colors */
          pix_colors_focused[idx] = pix_colors_focused[minBrightCOLOR + i - 8];
          goto done;
        }

      if (i >= 0 && i <= 7)
        {
          /* normal colors */
          pix_colors_focused[idx] = pix_colors_focused[minCOLOR + i];
          goto done;
        }
    }

  set_color (xcol, color);

  /*
   * FIXME: should free colors here, but no idea how to do it so instead,
   * so just keep gobbling up the colormap
   */

  pix_colors_focused[idx] = xcol;

done:
  /*TODO: handle Color_BD, scrollbar background, etc. */

  update_fade_color (idx);
  recolour_cursor ();
  scr_recolour ();
#endif /* XTERM_COLOR_CHANGE */
}

void
rxvt_term::recolour_cursor ()
{
  XColor fg, bg;

  (ISSET_PIXCOLOR (Color_pointer_fg)
     ? pix_colors_focused[Color_pointer_fg]
     : pix_colors_focused[Color_fg]).get (fg);

  (ISSET_PIXCOLOR (Color_pointer_bg)
     ? pix_colors_focused[Color_pointer_bg]
     : pix_colors_focused[Color_bg]).get (bg);

  XRecolorCursor (dpy, TermWin_cursor, &fg, &bg);
}

/*----------------------------------------------------------------------*/
/*
 * find if fg/bg matches any of the normal (low-intensity) colors
 */
char *
rxvt_term::get_colorfgbg ()
{
  unsigned int i;
  const char *xpmb = "";
  char fstr[] = "default";
  char bstr[] = "default";
  char *env_colorfgbg;

  for (i = Color_Black; i <= Color_White; i++)
    if (pix_colors[Color_fg] == pix_colors[i])
      {
        sprintf (fstr, "%d", i - Color_Black);
        break;
      }

  for (i = Color_Black; i <= Color_White; i++)
    if (pix_colors[Color_bg] == pix_colors[i])
      {
        sprintf (bstr, "%d", i - Color_Black);
#ifdef BG_IMAGE_FROM_FILE
        xpmb = "default;";
#endif
        break;
      }

  env_colorfgbg = (char *)rxvt_malloc (sizeof ("COLORFGBG=default;default;bg"));
  sprintf (env_colorfgbg, "COLORFGBG=%s;%s%s", fstr, xpmb, bstr);
  return env_colorfgbg;
}

/*----------------------------------------------------------------------*/

bool
rxvt_term::set_color (rxvt_color &color, const char *name)
{
  if (color.set (this, name))
    return true;

  rxvt_warn ("can't get colour '%s', continuing without.\n", name);
  return false;
}

void
rxvt_term::alias_color (int dst, int src)
{
  pix_colors[dst].set (this, rs[Rs_color + dst] = rs[Rs_color + src]);
}

/* -------------------------------------------------------------------- *
 * -                         WINDOW RESIZING                          - *
 * -------------------------------------------------------------------- */
void
rxvt_term::resize_all_windows (unsigned int newwidth, unsigned int newheight, int ignoreparent)
{
  int fix_screen;
  int old_width  = szHint.width;
  int old_height = szHint.height;

  window_calc (newwidth, newheight);

  bool set_hint = !HOOK_INVOKE ((this, HOOK_RESIZE_ALL_WINDOWS, DT_INT, newwidth, DT_INT, newheight, DT_END));

  // to avoid races between us and the wm, we clear the incremental size hints around the xresizewindow
  if (set_hint)
    {
      szHint.flags &= ~(PBaseSize | PResizeInc);
      XSetWMNormalHints (dpy, parent, &szHint);
      szHint.flags |= PBaseSize | PResizeInc;
    }

  if (!ignoreparent)
    {
#ifdef SMART_RESIZE
      /*
       * resize by Marius Gedminas <marius.gedminas@uosis.mif.vu.lt>
       * reposition window on resize depending on placement on screen
       */
      int x, y, x1, y1;
      int dx, dy;
      unsigned int unused_w1, unused_h1, unused_b1, unused_d1;
      Window unused_cr;

      XTranslateCoordinates (dpy, parent, display->root,
                             0, 0, &x, &y, &unused_cr);
      XGetGeometry (dpy, parent, &unused_cr, &x1, &y1,
                    &unused_w1, &unused_h1, &unused_b1, &unused_d1);
      /*
       * if display->root isn't the parent window, a WM will probably have offset
       * our position for handles and decorations.  Counter it
       */
      if (x1 != x || y1 != y)
        {
          x -= x1;
          y -= y1;
        }

      x1 = (DisplayWidth  (dpy, display->screen) - old_width ) / 2;
      y1 = (DisplayHeight (dpy, display->screen) - old_height) / 2;
      dx = old_width  - szHint.width;
      dy = old_height - szHint.height;

      /* Check position of the center of the window */
      if (x < x1)             /* left half */
        dx = 0;
      else if (x == x1)       /* exact center */
        dx /= 2;
      if (y < y1)             /* top half */
        dy = 0;
      else if (y == y1)       /* exact center */
        dy /= 2;

      XMoveResizeWindow (dpy, parent, x + dx, y + dy,
                         szHint.width, szHint.height);
#else
      XResizeWindow (dpy, parent, szHint.width, szHint.height);
#endif
    }

  if (set_hint)
    XSetWMNormalHints (dpy, parent, &szHint);

  fix_screen = ncol != prev_ncol || nrow != prev_nrow;

  if (fix_screen || newwidth != old_width || newheight != old_height)
    {
      if (scrollBar.state)
        scrollBar.resize ();

      XMoveResizeWindow (dpy, vt,
                         window_vt_x, window_vt_y,
                         vt_width, vt_height);

#ifdef HAVE_BG_PIXMAP
      if (bg_window_size_sensitive ())
        update_background ();
#endif
    }

  if (fix_screen || old_height == 0)
    scr_reset ();

#ifdef USE_XIM
  im_set_position ();
#endif
}

/*
 * Set the width/height of the vt window in characters.  Units are pixels.
 * good for toggling 80/132 columns
 */
void
rxvt_term::set_widthheight (unsigned int newwidth, unsigned int newheight)
{
  XWindowAttributes wattr;

  if (newwidth == 0 || newheight == 0)
    {
      XGetWindowAttributes (dpy, display->root, &wattr);

      if (newwidth == 0)
        newwidth = wattr.width - szHint.base_width;
      if (newheight == 0)
        newheight = wattr.height - szHint.base_height;
    }

  if (newwidth != vt_width || newheight != vt_height)
    {
      newwidth += szHint.base_width;
      newheight += szHint.base_height;
      resize_all_windows (newwidth, newheight, 0);
    }
}

/* -------------------------------------------------------------------- *
 * -                      X INPUT METHOD ROUTINES                     - *
 * -------------------------------------------------------------------- */
#ifdef USE_XIM

void
rxvt_term::im_set_color (unsigned long &fg, unsigned long &bg)
{
  fg = pix_colors [Color_fg];
  bg = pix_colors [Color_bg];
}

void
rxvt_term::im_set_size (XRectangle &size)
{
  // the int_bwidth terms make no sense to me
  size.x      = int_bwidth;
  size.y      = int_bwidth;
  size.width  = Width2Pixel (ncol) + int_bwidth;
  size.height = Height2Pixel (nrow) + int_bwidth;
}

void
rxvt_term::im_set_preedit_area (XRectangle &preedit_rect,
                                XRectangle &status_rect,
                                const XRectangle &needed_rect)
{
  preedit_rect.x      = needed_rect.width;
  preedit_rect.y      = 0;
  preedit_rect.width  = Width2Pixel (ncol) - needed_rect.width + 1;
  preedit_rect.height = fheight;

  status_rect.x       = 0;
  status_rect.y       = 0;
  status_rect.width   = needed_rect.width ? needed_rect.width : Width2Pixel (ncol) + 1;
  status_rect.height  = fheight;
}

/* Checking whether input method is running. */
bool
rxvt_term::im_is_running ()
{
  Atom atom;
  Window win;
  char server[IMBUFSIZ];

  /* get current locale modifier */
  if (char *p = XSetLocaleModifiers (0))
    {
      strcpy (server, "@server=");
      strncat (server, p + 4, IMBUFSIZ - 9); /* skip "@im=" */

      if (p = strchr (server + 1, '@'))      /* first one only */
        *p = '\0';

      atom = XInternAtom (dpy, server, False);
      win = XGetSelectionOwner (dpy, atom);

      if (win != None)
        return true;
    }

  return false;
}

void
rxvt_term::im_send_spot ()
{
  XPoint nspot;
  XVaNestedList preedit_attr;

  if (!Input_Context
      || !focus
      || !(input_style & (XIMPreeditPosition | XIMPreeditCallbacks)))
    return;

  im_set_position (nspot);

  if (nspot.x == spot.x && nspot.y == spot.y)
    return;

  spot = nspot;

  preedit_attr = XVaCreateNestedList (0, XNSpotLocation, &spot, NULL);
  XSetICValues (Input_Context, XNPreeditAttributes, preedit_attr, NULL);
  XFree (preedit_attr);
}

void
rxvt_term::im_destroy ()
{
  if (input_method)
    {
      if (Input_Context && input_method->xim)
        XDestroyIC (Input_Context);

      display->put_xim (input_method);
      input_method = 0;
    }

  Input_Context = 0;
}

#ifdef ENABLE_XIM_ONTHESPOT

static void
xim_preedit_start (XIC ic, XPointer client_data, XPointer call_data)
{
  ((rxvt_term *)client_data)->make_current ();
  HOOK_INVOKE (((rxvt_term *)client_data, HOOK_XIM_PREEDIT_START, DT_END));
}

static void
xim_preedit_done (XIC ic, XPointer client_data, XPointer call_data)
{
  ((rxvt_term *)client_data)->make_current ();
  HOOK_INVOKE (((rxvt_term *)client_data, HOOK_XIM_PREEDIT_DONE, DT_END));
}

static void
xim_preedit_draw (XIC ic, XPointer client_data, XIMPreeditDrawCallbackStruct *call_data)
{
  rxvt_term *term = (rxvt_term *)client_data;
  XIMText *text = call_data->text;

  term->make_current ();

  if (text)
    {
      wchar_t *str;

      if (!text->encoding_is_wchar && text->string.multi_byte)
        {
          // of course, X makes it ugly again
          if (term->rs[Rs_imLocale])
            SET_LOCALE (term->rs[Rs_imLocale]);

          str = rxvt_temp_buf<wchar_t> (text->length + 1);
          mbstowcs (str, text->string.multi_byte, text->length + 1);

          if (term->rs[Rs_imLocale])
            SET_LOCALE (term->locale);
        }
      else
        str = text->string.wide_char;

      HOOK_INVOKE ((term, HOOK_XIM_PREEDIT_DRAW,
                    DT_INT, call_data->caret,
                    DT_INT, call_data->chg_first,
                    DT_INT, call_data->chg_length,
                    DT_LCS_LEN, (void *)text->feedback, text->feedback ? (int)text->length : 0,
                    DT_WCS_LEN, str, str ? (int)text->length : 0,
                    DT_END));
    }
  else
    HOOK_INVOKE ((term, HOOK_XIM_PREEDIT_DRAW,
                  DT_INT, call_data->caret,
                  DT_INT, call_data->chg_first,
                  DT_INT, call_data->chg_length,
                  DT_END));
}

#if 0
static void
xim_preedit_caret (XIC ic, XPointer client_data, XIMPreeditCaretCallbackStruct *call_data)
{
  ((rxvt_term *)client_data)->make_current ();
  HOOK_INVOKE (((rxvt_term *)client_data, HOOK_XIM_PREEDIT_CARET,
                DT_INT, call_data->position,
                DT_INT, call_data->direction,
                DT_INT, call_data->style,
                DT_END));
}
#endif

#endif

/*
 * Try to open a XIM with the current modifiers, then see if we can
 * open a suitable preedit type
 */
bool
rxvt_term::im_get_ic (const char *modifiers)
{
  int i, j, found;
  XIM xim;
  XPoint spot;
  XRectangle rect, status_rect, needed_rect;
  unsigned long fg, bg;
  const char *p;
  char **s;
  XIMStyles *xim_styles;

  set_environ (envv);

  if (! ((p = XSetLocaleModifiers (modifiers)) && *p))
    return false;

  input_method = display->get_xim (locale, modifiers);
  if (input_method == NULL)
    return false;

  xim = input_method->xim;
  spot.x = spot.y = -1;

  xim_styles = NULL;
  if (XGetIMValues (xim, XNQueryInputStyle, &xim_styles, NULL)
      || !xim_styles || !xim_styles->count_styles)
    {
      im_destroy ();
      return false;
    }

  const char *pet[] = { rs[Rs_preeditType], "OverTheSpot,OffTheSpot,Root,None" };

  for (int pi = 0; pi < 2; pi++)
    {
      p = pet[pi];

      if (!p)
        continue;

      s = rxvt_strsplit (',', p);

      for (i = found = 0; !found && s[i]; i++)
        {
          if (!strcmp (s[i], "OverTheSpot"))
            input_style = XIMPreeditPosition | XIMStatusNothing;
          else if (!strcmp (s[i], "OffTheSpot"))
            input_style = XIMPreeditArea | XIMStatusArea;
          else if (!strcmp (s[i], "Root"))
            input_style = XIMPreeditNothing | XIMStatusNothing;
          else if (!strcmp (s[i], "None"))
            input_style = XIMPreeditNone | XIMStatusNone;
#ifdef ENABLE_XIM_ONTHESPOT
          else if (SHOULD_INVOKE (HOOK_XIM_PREEDIT_START) && !strcmp (s[i], "OnTheSpot"))
            input_style = XIMPreeditCallbacks | XIMStatusNothing;
#endif
          else
            input_style = XIMPreeditNothing | XIMStatusNothing;

          for (j = 0; j < xim_styles->count_styles; j++)
            if (input_style == xim_styles->supported_styles[j])
              {
                rxvt_free_strsplit (s);

                found = 1;
                goto foundpet;
              }

        }

      rxvt_free_strsplit (s);
    }

foundpet:

  XFree (xim_styles);

  if (!found)
    {
      im_destroy ();
      return false;
    }

  XFontSet fs = 0;
  XVaNestedList preedit_attr = 0, status_attr = 0;

  if (input_style & (XIMPreeditPosition | XIMPreeditArea))
    {
      // fake us a font-set, please
      char **missing_charset_list;
      int missing_charset_count;
      char *def_string;
      char pat[512];

      sprintf (pat,
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "-*-*-*-R-*-*-%d-*-*-*-*-*-*,"
               "*",
               fheight,
               fheight + 1, fheight - 1,
               fheight - 2, fheight + 2);

      fs = XCreateFontSet (dpy, rs[Rs_imFont] ? rs[Rs_imFont] : pat,
                           &missing_charset_list, &missing_charset_count, &def_string);

      if (missing_charset_list)
        XFreeStringList (missing_charset_list);

      if (!fs)
        {
          input_style &= ~(XIMPreeditPosition | XIMPreeditArea);
          rxvt_warn ("unable to create fontset for input method, try \"-pt Root\". Continuing.\n");
        }
    }

  if (input_style & XIMPreeditPosition)
    {
      im_set_size (rect);
      im_set_position (spot);
      im_set_color (fg, bg);

      preedit_attr = XVaCreateNestedList (0,
                                          XNForeground, fg,
                                          XNBackground, bg,
                                          XNArea, &rect,
                                          XNSpotLocation, &spot,
                                          XNFontSet, fs,
                                          NULL);
    }
  else if (input_style & XIMPreeditArea)
    {
      im_set_color (fg, bg);

      /*
       * The necessary width of preedit area is unknown
       * until create input context.
       */
      needed_rect.width = 0;
      im_set_preedit_area (rect, status_rect, needed_rect);

      preedit_attr = XVaCreateNestedList (0,
                                          XNForeground, fg,
                                          XNBackground, bg,
                                          XNArea, &rect,
                                          XNFontSet, fs,
                                          NULL);
      status_attr = XVaCreateNestedList (0,
                                         XNForeground, fg,
                                         XNBackground, bg,
                                         XNArea, &status_rect,
                                         XNFontSet, fs,
                                         NULL);
    }
#if ENABLE_XIM_ONTHESPOT
  else if (input_style & XIMPreeditCallbacks)
    {
      XIMCallback xcb[4];

      im_set_position (spot);

      xcb[0].client_data = (XPointer)this; xcb[0].callback = (XIMProc)xim_preedit_start;
      xcb[1].client_data = (XPointer)this; xcb[1].callback = (XIMProc)xim_preedit_done;
      xcb[2].client_data = (XPointer)this; xcb[2].callback = (XIMProc)xim_preedit_draw;
# if 0
      xcb[3].client_data = (XPointer)this; xcb[3].callback = (XIMProc)xim_preedit_caret;
# endif

      preedit_attr = XVaCreateNestedList (0,
                                          XNSpotLocation, &spot,
                                          XNPreeditStartCallback, &xcb[0],
                                          XNPreeditDoneCallback , &xcb[1],
                                          XNPreeditDrawCallback , &xcb[2],
# if 0
                                          XNPreeditCaretCallback, &xcb[3],
# endif
                                          NULL);
    }
#endif

  Input_Context = XCreateIC (xim,
                             XNInputStyle, input_style,
                             XNClientWindow, vt,
                             XNFocusWindow, parent,
                             preedit_attr ? XNPreeditAttributes : NULL,
                             preedit_attr,
                             status_attr ? XNStatusAttributes : NULL,
                             status_attr, NULL);

  if (preedit_attr) XFree (preedit_attr);
  if (status_attr)  XFree (status_attr);
  if (fs)           XFreeFontSet (dpy, fs);

  if (Input_Context == NULL)
    {
      rxvt_warn ("failed to create input context, continuing without XIM.\n");
      im_destroy ();
      return false;
    }

#if 0
  // unfortunately, only the focus window is used by XIM, hard to fix
  if (!XGetICValues (Input_Context, XNFilterEvents, &vt_emask_xim, NULL))
    vt_select_input ();
#endif

  im_set_position ();

  return true;
}

void
rxvt_term::im_cb ()
{
  int i;
  const char *p;
  char **s;
  char buf[IMBUFSIZ];

  make_current ();

  im_destroy ();

  if (Input_Context)
    return;

  if (rs[Rs_imLocale])
    SET_LOCALE (rs[Rs_imLocale]);

  p = rs[Rs_inputMethod];
  if (p && *p)
    {
      bool found = false;

      s = rxvt_strsplit (',', p);

      for (i = 0; s[i]; i++)
        {
          if (*s[i])
            {
              strcpy (buf, "@im=");
              strncat (buf, s[i], IMBUFSIZ - 5);
              if (im_get_ic (buf))
                {
                  found = true;
                  break;
                }
            }
        }

      rxvt_free_strsplit (s);

      if (found)
        goto done;
    }

  /* try with XMODIFIERS env. var. */
  if (im_get_ic (""))
    goto done;

  /* try with no modifiers base IF the user didn't specify an IM */
  if (im_get_ic ("@im=none"))
    goto done;

done:
  if (rs[Rs_imLocale])
    SET_LOCALE (locale);
}

void
rxvt_term::im_set_position ()
{
  XRectangle preedit_rect, status_rect, *needed_rect;
  XVaNestedList preedit_attr, status_attr;

  if (!Input_Context
      || !focus
      || !(input_style & (XIMPreeditArea | XIMPreeditPosition))
      || !im_is_running ())
    return;

  if (input_style & XIMPreeditPosition)
    {
      im_set_size (preedit_rect);
      preedit_attr = XVaCreateNestedList (0, XNArea, &preedit_rect, NULL);

      XSetICValues (Input_Context,
                    XNPreeditAttributes, preedit_attr, NULL);
    }
  else
    {
      /* Getting the necessary width of preedit area */
      status_attr = XVaCreateNestedList (0, XNAreaNeeded, &needed_rect, NULL);
      XGetICValues (Input_Context, XNStatusAttributes, status_attr, NULL);
      XFree (status_attr);

      im_set_preedit_area (preedit_rect, status_rect, *needed_rect);
      XFree (needed_rect);

      preedit_attr = XVaCreateNestedList (0, XNArea, &preedit_rect, NULL);
      status_attr  = XVaCreateNestedList (0, XNArea, &status_rect,  NULL);

      XSetICValues (Input_Context,
                    XNPreeditAttributes, preedit_attr,
                    XNStatusAttributes,  status_attr,
                    NULL);

      XFree (status_attr);
    }

   XFree (preedit_attr);
}
#endif /* USE_XIM */

void
rxvt_term::get_window_origin (int &x, int &y)
{
  Window cr;
  XTranslateCoordinates (dpy, parent, display->root, 0, 0, &x, &y, &cr);
}

Pixmap
rxvt_term::get_pixmap_property (Atom property)
{
  Pixmap pixmap = None;

  int aformat;
  unsigned long nitems, bytes_after;
  Atom atype;
  unsigned char *prop;
  int result = XGetWindowProperty (dpy, display->root, property,
                                   0L, 1L, False, XA_PIXMAP, &atype, &aformat,
                                   &nitems, &bytes_after, &prop);
  if (result == Success)
    {
      if (atype == XA_PIXMAP)
        pixmap = *(Pixmap *)prop;
      XFree (prop);
    }

  return pixmap;
}

#ifdef HAVE_BG_PIXMAP

void
rxvt_term::update_background ()
{
  if (update_background_ev.is_active ())
    return;

  bg_invalidate ();

  if (!mapped)
    return;

  ev_tstamp to_wait = 0.5 - (ev::now () - bg_valid_since);

  if (to_wait <= 0.)
    bg_render ();
  else
    update_background_ev.start (to_wait);
}

void
rxvt_term::update_background_cb (ev::timer &w, int revents)
{
  make_current ();

  update_background_ev.stop ();
  bg_render ();
  refresh_check ();
}

#endif /* HAVE_BG_PIXMAP */

/*----------------------- end-of-file (C source) -----------------------*/
