/*
 * $Id: defaultfont.h,v 1.4 2003/11/25 11:52:42 pcg Exp $
 */

#ifndef _DEFAULTFONT_H_
#define _DEFAULTFONT_H_

#ifdef HAVE_XSETLOCALE
# define X_LOCALE
# include <X11/Xlocale.h>
#else
# ifdef HAVE_SETLOCALE
#  include <locale.h>
# endif
#endif				/* HAVE_XLOCALE */

#ifdef HAVE_NL_LANGINFO
# include <langinfo.h>
#endif

#include "rxvtlib.h"
#include "feature.h"
#include "encoding.h"
#include "rxvtvec.h"

struct rxvt_font {
  // managed by the fontset
#if EXPLICIT_CONTEXT
  rxvt_t rxvt_term;
  void set_term (pR) { this->rxvt_term = R; }
#else
  void set_term (pR) { }
#endif
  char *name;
  codeset cs;
  bool loaded;

  // managed by the font object
  bool prop; // wether this is a proportional font or has other funny characteristics
  int ascent, descent,
      width, height;

  void set_name (char *name)
  {
    if (this->name) free (this->name); // let the compiler optimize
    this->name = name;
  }

  rxvt_font () { name = 0; }
  ~rxvt_font () { free (name); clear (); };

  void clear_rect (int x, int y, int w, int h, int color);

  virtual void clear () { };

  virtual bool load (int maxheight) = 0;
  virtual bool has_codepoint (uint32_t unicode) = 0;

  virtual void draw (int x, int y,
                     const text_t *text, int len,
                     int fg, int bg) = 0;
};

//#define FONT_REF(obj) (obj)->refcnt++
//#define FONT_UNREF(obj) if (!--(obj)->refcnt) delete (obj)
#define FONT_UNREF(f) delete f

struct rxvt_fallback_font;

struct rxvt_fontset {
  rxvt_fontset (pR);
  ~rxvt_fontset ();

  rxvt_font *new_font (const char *name, codeset cs);

  void populate (const char *desc);
  int find_font (uint32_t unicode);

  rxvt_font *operator [](int id) const
  {
    return fonts[id];
  }

  rxvt_font *base_font () const
  {
    return fonts[base_id];
  }

private:
#ifdef EXPLICIT_CONTEXT
  rxvt_t rxvt_term;
#endif
  simplevec<rxvt_font *> fonts;
  const rxvt_fallback_font *fallback;

  int height;
  int base_id;

  bool realize_font (int i);
  void add_fonts (const char *desc);
  void clear ();
};

#endif /* _DEFAULTFONT_H_ */

