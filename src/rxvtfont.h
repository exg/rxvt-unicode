#ifndef DEFAULTFONT_H_
#define DEFAULTFONT_H_

#include <X11/Xlib.h>
#if XFT
# include <X11/Xft/Xft.h>
#endif

#include <inttypes.h>

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
  bool loaded; // whether we tried loading it before (not whether it's loaded)

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

  void unref ()
  {
    clear ();
    delete this;
  }
};

struct rxvt_fallback_font;

struct rxvt_fontset
{
  char *fontdesc;

  // must be power-of-two - 1, also has to match RS_fontMask in rxvt.h
#if USE_256_COLORS
  enum { fontCount =   7 }; // 4 extra colors bits, 4 fewer fontcount bits
#else
  enum { fontCount = 127 };
#endif

  // index of first font in set
  enum { firstFont = 2 };

  rxvt_fontset (rxvt_term *term);
  ~rxvt_fontset ();

  bool populate (const char *desc);
  void set_prop (const rxvt_fontprop &prop, bool force_prop) { this->prop = prop; this->force_prop = force_prop; }
  int find_font_idx (uint32_t unicode);
  int find_font (const char *name) const;
  bool realize_font (int i);

  rxvt_font *operator [] (int id) const
  {
    return fonts[id >> 1];
  }

  int
  find_font (unicode_t unicode)
  {
    return min<int> ((fontCount << 1) | 1, find_font_idx (unicode));
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
  void prepare_font (rxvt_font *font, codeset cs);
  void add_fonts (const char *desc);
  void push_font (rxvt_font *font);
};

#endif /* _DEFAULTFONT_H_ */

