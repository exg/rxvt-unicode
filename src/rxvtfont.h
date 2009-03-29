#ifndef DEFAULTFONT_H_
#define DEFAULTFONT_H_

#include <X11/Xlib.h>
#if XFT
# include <X11/Xft/Xft.h>
#endif

#include <inttypes.h>

#include "feature.h"
#include "encoding.h"
#include "rxvtutil.h"
#include "rxvttoolkit.h"

struct rxvt_term;

struct rxvt_fontprop
{
  enum {
    unset  = -1,
    medium = 100, bold = 200,
    roman  = 0, italic = 100,
  };
  int width, height, ascent;
  int weight, slant;
};

struct rxvt_font
{
  // managed by the fontset
  rxvt_term *term;
  void set_term (rxvt_term *term) { this->term = term; }

  char *name;
  codeset cs;
  bool loaded; // wether we tried loading it before (not wether it's loaded)

  // managed by the font object
  int ascent, descent,
      width, height;

  void set_name (char *name_);

  rxvt_font ();
  virtual ~rxvt_font () { free (name); };

  virtual void clear () { };

  void clear_rect (rxvt_drawable &d, int x, int y, int w, int h, int color) const;

  virtual rxvt_fontprop properties () = 0;

  virtual bool load (const rxvt_fontprop &morph, bool force_prop) = 0;
  virtual bool has_char (uint32_t unicode, const rxvt_fontprop *prop, bool &careful) const = 0;

  virtual void draw (rxvt_drawable &d,
                     int x, int y,
                     const text_t *text, int len,
                     int fg, int bg) = 0;
};

#define FONT_UNREF(f) do { (f)->clear (); delete (f); } while (0)

struct rxvt_fallback_font;

struct rxvt_fontset
{
  char *fontdesc;

  rxvt_fontset (rxvt_term *term);
  ~rxvt_fontset ();

  bool populate (const char *desc);
  void set_prop (const rxvt_fontprop &prop, bool force_prop) { this->prop = prop; this->force_prop = force_prop; }
  int find_font (uint32_t unicode);
  int find_font (const char *name) const;
  bool realize_font (int i);

  // font-id's MUST fit into a signed 16 bit integer, and within 0..255
  rxvt_font *operator [] (int id) const
  {
    return fonts[id & 0x7f];
  }

private:
  rxvt_term *term;
  rxvt_fontprop prop;
  bool force_prop;
  simplevec<rxvt_font *> fonts;
  const rxvt_fallback_font *fallback;

  typedef unsigned char pagemap[256];
  vector<pagemap *> fmap;

  void clear ();
  rxvt_font *new_font (const char *name, codeset cs);
  void add_fonts (const char *desc);
};

#endif /* _DEFAULTFONT_H_ */

