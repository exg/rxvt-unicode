#ifndef DEFAULTFONT_H_
#define DEFAULTFONT_H_

#include <X11/Xlib.h>
#if XFT
# include <X11/Xft/Xft.h>
#endif

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
#include "rxvtstl.h"

struct rxvt_fontprop {
  enum {
    medium = 100, bold = 200,
    roman  = 0, italic = 100,
  };
  int width, height;
  int weight, slant;
};

struct rxvt_drawable {
  rxvt_display *display;
  Drawable drawable;
#if XFT
  XftDraw *xftdrawable;
  operator XftDraw *();
#endif

  rxvt_drawable (rxvt_display *display, Drawable drawable)
  : display(display),
#if XFT
    xftdrawable(0),
#endif
    drawable(drawable)
  { }

#if XFT
  ~rxvt_drawable ();
#endif

  operator Drawable() { return drawable; }
};

struct rxvt_font {
  struct rxvt_fontset *fs;
  // managed by the fontset
  rxvt_t r;
  void set_term (rxvt_t r) { this->r = r; }

  char *name;
  codeset cs;
  bool loaded; // wether we tried loading it before (not wether it's loaded)

  // managed by the font object
  bool slow; // wether this is a proportional font or has other funny characteristics
  int ascent, descent,
      width, height;

  void set_name (char *name)
  {
    if (this->name) free (this->name); // let the compiler optimize
    this->name = name;
  }

  rxvt_font () { name = 0; }
  ~rxvt_font () { free (name); };

  void clear_rect (rxvt_drawable &d, int x, int y, int w, int h, int color);

  virtual void clear () { };

  virtual rxvt_fontprop properties () = 0;

  virtual bool load (const rxvt_fontprop &prop) = 0;
  virtual bool has_codepoint (uint32_t unicode) = 0;

  virtual void draw (rxvt_drawable &d,
                     int x, int y,
                     const text_t *text, int len,
                     int fg, int bg) = 0;
};

#define FONT_UNREF(f) do { (f)->clear (); delete (f); } while (0)

struct rxvt_fallback_font;

struct rxvt_fontset {
  rxvt_fontset (rxvt_t r);
  ~rxvt_fontset ();

  rxvt_font *new_font (const char *name, codeset cs);

  bool populate (const char *desc);
  int find_font (uint32_t unicode);

  rxvt_font *operator [] (int id) const
  {
    return fonts[id];
  }

  rxvt_font *base_font () const
  {
    return fonts[base_id];
  }

private:
  rxvt_t r;
  simplevec<rxvt_font *> fonts;
  const rxvt_fallback_font *fallback;

  rxvt_fontprop base_prop;
  int base_id;

  bool realize_font (int i);
  void add_fonts (const char *desc);
  void clear ();
};

#endif /* _DEFAULTFONT_H_ */

