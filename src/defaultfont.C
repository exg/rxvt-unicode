/*--------------------------------*-C-*---------------------------------*;
 * File:	defaultfont.C
 *----------------------------------------------------------------------*
 * Copyright (c) 2003      Marc Lehmann rxvt@plan9.de>
 *				- original version.
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *---------------------------------------------------------------------*/

#include "../config.h"
#include "rxvt.h"
#include "defaultfont.h"

#include <cstdlib>

#define DISPLAY  r->Xdisplay
#define DRAWABLE r->TermWin.vt
#define GC       r->TermWin.gc

const struct rxvt_fallback_font {
  codeset cs;
  const char *name;
} fallback_fonts[] = {
  { CS_ISO8859_1,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-1"  },
  { CS_ISO8859_15,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-15" },
  { CS_ISO8859_15,   "-*-*-*-r-*--*-*-*-*-c-*-fcd8859-15" },

#if ENCODING_EU
  // cyrillic
  { CS_KOI8_R,        "-*-*-*-r-*--*-*-*-*-c-*-koi8-r"    },
  { CS_KOI8_U,        "-*-*-*-r-*--*-*-*-*-c-*-koi8-u"    },

  { CS_ISO8859_2,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-2"  },
  { CS_ISO8859_3,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-3"  },
  { CS_ISO8859_4,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-4"  },
  { CS_ISO8859_5,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-5"  },
  { CS_ISO8859_6,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-6"  },
  { CS_ISO8859_7,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-7"  },
  { CS_ISO8859_8,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-8"  },
  { CS_ISO8859_9,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-9"  },
  { CS_ISO8859_10,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-10" },
  { CS_ISO8859_11,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-11" },
  { CS_ISO8859_13,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-13" },
  { CS_ISO8859_14,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-14" },
  { CS_ISO8859_16,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-16" },
#endif

  // japanese
#if ENCODING_JP || ENCODING_JP_EXT
# if XFT
  // prefer xft for complex scripts
  { CS_UNICODE,        "xft:Kochi Gothic:antialias=false"          },
# endif
  { CS_JIS0201_1976_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0201*-0"  },
  { CS_JIS0208_1983_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0208*-0"  },
  { CS_JIS0212_1990_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0212*-0"  },
#endif

#if ENCODING_CN || ENCODING_CN_EXT
# if XFT
  { CS_BIG5_EXT,       "xft:AR PL Mingti2L Big5"                   },
  { CS_BIG5_EXT,       "xft:AR PL KaitiM Big5"                     },
  { CS_GB2312_1980_0,  "xft:AR PL KaitiM GB"                       },
  { CS_GB2312_1980_0,  "xft:AR PL SungtiL GB"                      },
# endif
  { CS_CNS11643_1992_1, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-1" },
  { CS_CNS11643_1992_2, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-2" },
  { CS_CNS11643_1992_3, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-3" },
  { CS_CNS11643_1992_4, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-4" },
  { CS_CNS11643_1992_5, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-5" },
  { CS_CNS11643_1992_6, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-6" },
  { CS_CNS11643_1992_7, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-7" },
  { CS_CNS11643_1992_F, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643.1992-f" },
#endif

#if XFT
  { CS_UNICODE,      "xft:Andale Mono"                             },
  { CS_UNICODE,      "xft:Arial Unicode MS"                        },
#endif
  { CS_UNICODE,      "-*-lucidatypewriter-*-*-*-*-*-*-*-*-m-*-iso10646-1" },
  { CS_UNICODE,      "xft:FreeMono"                                },
  { CS_UNICODE,      "-*-unifont-*-*-*-*-*-*-*-*-c-*-iso10646-1"   },
  { CS_UNICODE,      "-*-*-*-r-*-*-*-*-*-*-c-*-iso10646-1"         },
  { CS_UNICODE,      "-*-*-*-r-*-*-*-*-*-*-m-*-iso10646-1"         },

  { CS_UNKNOWN, 0 }
};

/////////////////////////////////////////////////////////////////////////////

static void *enc_buf;
static uint32_t enc_len;

static inline void *
get_enc_buf (int len)
{
  if (len > enc_len)
    {
      free (enc_buf);
      enc_buf = malloc (len);
    }

  return enc_buf;
}

static const char *
enc_char (const text_t *text, int len, codeset cs, bool &zero)
{
  uint8_t *buf = (uint8_t *)get_enc_buf (len);

  while (len--)
    {
      uint32_t c = FROM_UNICODE (cs, *text++);

      if (c == NOCHAR)
        {
          c = 0;
          zero = true;
        }

      *buf++ = c;
    }

  return (const char *)enc_buf;
}

static const XChar2b *
enc_xchar2b (const text_t *text, int len, codeset cs, bool &zero)
{
  XChar2b *buf = (XChar2b *)get_enc_buf (len * sizeof (XChar2b));

  while (len--)
    {
      uint32_t c = FROM_UNICODE (cs, *text++);

      if (c == NOCHAR)
        {
          c = 0;
          zero = true;
        }

      buf->byte1 = c >> 8;
      buf->byte2 = c;
      buf++;
    }

  return (XChar2b *)enc_buf;
}

/////////////////////////////////////////////////////////////////////////////

void
rxvt_font::clear_rect (int x, int y, int w, int h, int color)
{
  if (color == Color_bg)
    XClearArea (DISPLAY, DRAWABLE, x, y, w, h, FALSE);
  else if (color >= 0)
    {
      XSetForeground (DISPLAY, GC, r->PixColors[color]);
      XFillRectangle (DISPLAY, DRAWABLE, GC, x, y, w, h);
    }
}

static const char *linedraw_cmds[128] = {
  "1hH", "2hH", "1vV", "2vV",
  0, 0, 0, 0,
  0, 0, 0, 0,
  "1HV", "2H1V", "1H2V", "2HV",

  // 2510
  "1hV", "2h1V", "1h2V", "2hV",
  "1Hv", "2H1v", "1H2v", "2Hv",
  "1hv", "2h1v", "1h2v", "2hv",
  "1HvV", "2H1vV", "1HV2v", "1Hv2V",

  // 2520
  "1H2vV", "2Hv1V", "2HV1v", "2HvV",
  "1hvV", "2h1vV", "1hV2v", "1hv2V",
  "1h2vV", "2hv1V", "1v2hV", "2hvV",
  "1hHV", "2h1HV", "2H1hV", "2hH1V",

  // 2530
  "1hH2V", "2hV1H", "1h2HV", "2hHV",
  "1hHv", "1vH2h", "1hv2H", "1v2hH",
  "1hH2v", "1H2hv", "1h2Hv", "2hHv",
  "1hHvV", "1vVH2h", "1hvV2H", "1vV2hH",

  // 2540
  "1hHV2v", "1hHv2V", "1hH2vV", "1HV2hv",
  "1hV2Hv", "1Hv2hV", "1hv2HV", "1V2hHv",
  "1v2hHV", "1H2hvV", "1h2HvV", "2hHvV",
  0, 0, 0, 0,

  // 2550
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,

  // 2560
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,
  0, 0, 0, 0,

  // 2570
  0, "1a", "1b", "1ab",
  "1h", "1v", "1H", "1V",
  "2h", "2v", "2H", "2V",
  "1h2H", "1v2V", "1H2h", "1V2v"

  // to be done
};

struct rxvt_font_default : rxvt_font {
  rxvt_fontprop properties ()
  {
    rxvt_fontprop p;

    p.height = 1;
    p.weight = rxvt_fontprop::medium;
    p.slant = rxvt_fontprop::roman;

    return p;
  }

  bool load (const rxvt_fontprop &prop)
  {
    width = 1; height = 1;
    ascent = 1; descent = 0;

    return true;
  }

  bool has_codepoint (uint32_t unicode)
  {
    if (unicode <= 0x001f)
      return true;
    if (unicode >= 0x0080 && unicode <= 0x009f)
      return true;

    if (unicode >= 0x2500 && unicode <= 0x257f
        && linedraw_cmds[unicode - 0x2500])
      return true;

    switch (unicode)
      {
        case ZERO_WIDTH_CHAR:
          return true;
      }

    return false;
  }

  void draw (int x, int y,
             const text_t *text, int len,
             int fg, int bg);
};

void
rxvt_font_default::draw (int x, int y,
                         const text_t *text, int len,
                         int fg, int bg)
{
  clear_rect (x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

  XSetForeground (DISPLAY, GC, r->PixColors[fg]);

  while (len--)
    {
      text_t t = *text++;

      if (t >= 0x2500 & t <= 0x2580 && linedraw_cmds[t - 0x2500])
        {
          const char *p = linedraw_cmds[t - 0x2500];

          int x0 = x, x1 = x + r->TermWin.fwidth  / 2, x2 = x + r->TermWin.fwidth ;
          int y0 = y, y1 = y + r->TermWin.fheight / 2, y2 = y + r->TermWin.fheight;

          XGCValues gcv;

          while (*p)
            {
              switch (*p++)
                {
                  case '1':
                    gcv.line_width = 0;
                    XChangeGC (DISPLAY, GC, GCLineWidth, &gcv);
                    break;

                  case '2':
                    gcv.line_width = 2;
                    XChangeGC (DISPLAY, GC, GCLineWidth, &gcv);
                    break;

                  case 'h': XDrawLine (DISPLAY, DRAWABLE, GC, x0, y1, x1, y1); break;
                  case 'H': XDrawLine (DISPLAY, DRAWABLE, GC, x1, y1, x2, y1); break;
                  case 'v': XDrawLine (DISPLAY, DRAWABLE, GC, x1, y0, x1, y1); break;
                  case 'V': XDrawLine (DISPLAY, DRAWABLE, GC, x1, y1, x1, y2); break;
                  case 'a': XDrawLine (DISPLAY, DRAWABLE, GC, x0, y2, x2, y0); break;
                  case 'b': XDrawLine (DISPLAY, DRAWABLE, GC, x0, y0, x2, y2); break;
                }
            }

          gcv.line_width = 0;
          XChangeGC (DISPLAY, GC, GCLineWidth, &gcv);
        }
      else
        switch (*text++)
          {
            case NOCHAR:
            case ZERO_WIDTH_CHAR:
              break;
            default:
              XDrawRectangle (DISPLAY, DRAWABLE, GC, x + 2, y + 2, r->TermWin.fwidth - 5, r->TermWin.fheight - 5);
          }

      x += r->TermWin.fwidth;
    }
}

/////////////////////////////////////////////////////////////////////////////

struct rxvt_font_x11 : rxvt_font {
  rxvt_font_x11 () { f = 0; }

  void clear ();

  rxvt_fontprop properties ();
  rxvt_fontprop properties (XFontStruct *f);

  bool load (const rxvt_fontprop &prop);

  bool has_codepoint (uint32_t unicode);

  void draw (int x, int y,
             const text_t *text, int len,
             int fg, int bg);

  XFontStruct *f;
  codeset cs;
  bool enc2b, encm;

  const char *get_property (XFontStruct *f, const char *property, const char *repl) const;
};

const char *
rxvt_font_x11::get_property (XFontStruct *f, const char *property, const char *repl) const
{
  unsigned long value;

  if (XGetFontProperty (f, XInternAtom (DISPLAY, property, 0), &value))
    return XGetAtomName (DISPLAY, value);
  else
    return repl;
}

rxvt_fontprop
rxvt_font_x11::properties ()
{
  return properties (f);
}

rxvt_fontprop
rxvt_font_x11::properties (XFontStruct *f)
{
  rxvt_fontprop p;

  const char *weight = get_property (f, "WEIGHT_NAME", "medium");
  const char *slant  = get_property (f, "SLANT", "r");

  p.height = height;
  p.weight = *weight == 'B' || *weight == 'b' ? rxvt_fontprop::bold : rxvt_fontprop::medium;
  p.slant = *slant == 'r' || *slant == 'R' ? rxvt_fontprop::roman : rxvt_fontprop::italic;

  return p;
}

bool
rxvt_font_x11::load (const rxvt_fontprop &prop)
{
  clear ();

  char **list;
  int count;
  XFontStruct *info;
  list = XListFontsWithInfo (DISPLAY, name, 128, &count, &info);

  if (!list)
    return false;

  int bestdiff = 0x7fffffff;
  XFontStruct *best = 0;
  for (int i = 0; i < count; i++)
    {
      XFontStruct *f = info + i;

      if (f->ascent + f->descent <= prop.height) // weed out too large fonts
        {
          rxvt_fontprop p = properties (f);
          int diff = (prop.height - f->ascent + f->descent) * 32
                   + abs (prop.weight - p.weight)
                   + abs (prop.slant  - p.slant );

          if (!best // compare against best found so far
              || diff < bestdiff)
            {
              best = f;
              bestdiff = diff;
            }
        }
    }

  if (!best)
    return false;

  set_name (strdup (list[best - info]));

  XFreeFontInfo (list, info, count);

  f = XLoadQueryFont (DISPLAY, name);

  if (!f)
    return false;

  unsigned long value;

  const char *registry = get_property (f, "CHARSET_REGISTRY", 0);
  const char *encoding = get_property (f, "CHARSET_ENCODING", 0);

  if (registry && encoding)
    {
      char charset[64];
      snprintf (charset, 64, "%s-%s", registry, encoding);

      cs = codeset_from_name (charset);
    }
  else
    {
      const char *charset = get_property (f, "FONT", 0);

      if (!charset)
        charset = name;

      int count = 13;
      while (*charset)
        if (*charset++ == '-' && !--count)
          break;

      cs = codeset_from_name (charset);
    }

  if (cs == CS_UNICODE)
    cs = CS_UNICODE_16; // X11 can have a max. of 65536 chars per font

  encm = f->min_byte1 != 0 || f->max_byte1 != 0;
  enc2b = encm || f->max_char_or_byte2 > 255;

  ascent = f->ascent;
  descent = f->descent;
  height = ascent + descent;

  slow = false;

  if (f->min_bounds.width == f->max_bounds.width)
    width = f->min_bounds.width;
  else if (f->per_char == NULL)
    width = f->max_bounds.width;
  else
    {
      slow = true;

      int N = f->max_char_or_byte2 - f->min_char_or_byte2;

      if (encm)
        N += (f->max_byte1 - f->min_byte1)
             * (f->max_char_or_byte2 - f->min_char_or_byte2 + 1);
       
      while (N)
        {
          if (f->per_char[N].width > width)
            width = f->per_char[N].width;

          --N;
        }
    }

  if (cs == CS_UNKNOWN)
    {
      fprintf (stderr, "unable to deduce codeset, ignoring font '%s'\n", name);

      clear ();

      return false;
    }

  return true;
}

void
rxvt_font_x11::clear ()
{
  if (f)
    {
      XFreeFont (DISPLAY, f);
      f = 0;
    }
}

bool
rxvt_font_x11::has_codepoint (uint32_t unicode)
{
  uint32_t ch = FROM_UNICODE (cs, unicode);

  if (ch == NOCHAR)
    return false;

  /* check wether the character exists in _this_ font. horrible. */
  XCharStruct *xcs;

  if (encm)
    {
      int byte1 = ch >> 8;
      int byte2 = ch & 255;

      if (byte1 < f->min_byte1 || byte1 > f->max_byte1
          || byte2 < f->min_char_or_byte2 || byte2 > f->max_char_or_byte2)
        return false;

      if (!f->per_char)
        return true;

      int D = f->max_char_or_byte2 - f->min_char_or_byte2 + 1;
      int N = (byte1 - f->min_byte1) * D + byte2 - f->min_char_or_byte2;

      xcs = f->per_char + N;
    }
  else
    {
      if (ch < f->min_char_or_byte2 || ch > f->max_char_or_byte2)
        return false;

      if (!f->per_char)
        return true;

      xcs = f->per_char + (ch - f->min_char_or_byte2);
    }

  if (xcs->lbearing == 0 && xcs->rbearing == 0 && xcs->width == 0
      && xcs->ascent == 0 && xcs->descent == 0)
    return false;

  return true;
}

void
rxvt_font_x11::draw (int x, int y,
                     const text_t *text, int len,
                     int fg, int bg)
{
  // this looks like a mess /.
  // and it is a mess /.
  // yet we are trying to be perfect /.
  // but the result still isn't perfect /.

  bool slow = this->slow
              || width != r->TermWin.fwidth
              || height != r->TermWin.fheight;

  int base = r->TermWin.fbase;

  XGCValues v;
  v.foreground = r->PixColors[fg];
  v.background = r->PixColors[bg];
  v.font = f->fid;

  if (enc2b)
    {
      const XChar2b *xc = enc_xchar2b (text, len, cs, slow);

      if (bg == Color_bg && !slow)
        {
          XChangeGC (DISPLAY, GC, GCForeground | GCBackground | GCFont, &v);
          XDrawImageString16 (DISPLAY, DRAWABLE, GC, x, y + base, xc, len);
        }
      else
        {
          clear_rect (x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

          XChangeGC (DISPLAY, GC, GCForeground | GCFont, &v);
          
          if (slow)
            {
              do
                {
                  if (xc->byte1 || xc->byte2)
                    XDrawString16 (DISPLAY, DRAWABLE, GC, x, y + base, xc, 1);

                  x += r->TermWin.fwidth;
                  xc++; len--;
                }
              while (len);
            }
          else
            XDrawString16 (DISPLAY, DRAWABLE, GC, x, y + base, xc, len);
        }
    }
  else
    {
      const char *xc = enc_char (text, len, cs, slow);

      if (bg == Color_bg && !slow)
        {
          XChangeGC (DISPLAY, GC, GCForeground | GCBackground | GCFont, &v);
          XDrawImageString (DISPLAY, DRAWABLE, GC, x, y + base, xc, len);
        }
      else
        {
          clear_rect (x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

          XChangeGC (DISPLAY, GC, GCForeground | GCFont, &v);
          
          if (slow)
            {
              do
                {
                  if (*xc)
                    XDrawString (DISPLAY, DRAWABLE, GC, x, y + base, xc, 1);

                  x += r->TermWin.fwidth;
                  xc++; len--;
                }
              while (len);
            }
          else
            XDrawString (DISPLAY, DRAWABLE, GC, x, y + base, xc, len);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

#if XFT
#if 0
#define UNIBITS 21
//#define SWATHBITS (UNIBITS / 2 + 3) // minimum size for "full" tables
#define SWATHBITS 8
#endif

struct rxvt_font_xft : rxvt_font {
#if 0
  enum {
    SWATHCOUNT = 1 << (21 - UNIBITS),
    SWATHSIZE  = 1 << (SWATHBITS - 5)
  };
  typedef uint32_t swath[SWATHSIZE];

  swath *cvr[SWATHCOUNT];
#endif

#if 0
  void gen_coverage_swath (unsigned int page);

  bool has_char (uint32_t ch)
    {
      unsigned int page = ch >> SWATHBITS;
      unsigned int idx  = ch & ((1 << SWATHBITS) - 1);

      if (page >= SWATHCOUNT)
        return false;

      if (!cvr[page]) gen_coverage_swath (page);

      return cvr[page][idx >> 5] & (1 << (idx & 31));
    }
#endif
  rxvt_font_xft () { f = 0; d = 0; }

  void clear ();

  rxvt_fontprop properties ();

  bool load (const rxvt_fontprop &prop);

  void draw (int x, int y,
             const text_t *text, int len,
             int fg, int bg);

  bool has_codepoint (uint32_t unicode);

protected:
  XftFont *f;
  XftDraw *d;

#if 0
  virtual void populate_coverage_swath (uint32_t lo, uint32_t hi) = 0;
  void set_swath (uint32_t ch)
    {
      cvr[ch >> SWATHBITS] |= 1 << (ch & ((1 << SWATHBITS) - 1));
    }
#endif
};

void
rxvt_font_xft::clear ()
{
  if (f)
    {
      XftFontClose (DISPLAY, f);
      f = 0;
    }

  if (d)
    {
      XftDrawDestroy (d);
      d = 0;
    }

#if 0
  for (int i = 0; i < SWATHCOUNT; i++)
    delete cvr[i];
#endif
}

rxvt_fontprop
rxvt_font_xft::properties ()
{
  rxvt_fontprop p;

  FT_Face face = XftLockFace (f);

  p.height = height;
  p.weight = face->style_flags & FT_STYLE_FLAG_BOLD ? rxvt_fontprop::bold : rxvt_fontprop::medium;
  p.slant = face->style_flags & FT_STYLE_FLAG_ITALIC ? rxvt_fontprop::italic : rxvt_fontprop::roman;

  XftUnlockFace (f);

  return p;
}

bool
rxvt_font_xft::load (const rxvt_fontprop &prop)
{
#if 0
  for (int i = 0; i < SWATHCOUNT; i++)
    cvr[i] = 0;
#endif

  clear ();

  FcPattern *p = FcNameParse ((FcChar8 *) name);

  if (!p)
    return false;

  FcValue v;

  if (FcPatternGet (p, FC_WEIGHT, 0, &v) != FcResultMatch)
    FcPatternAddInteger (p, FC_WEIGHT, prop.weight);

  if (FcPatternGet (p, FC_SLANT, 0, &v) != FcResultMatch)
    FcPatternAddInteger (p, FC_SLANT, prop.slant);

  //FcPatternAddBool (p, FC_MINSPACE, 1);

  XftResult result;
  FcPattern *match = XftFontMatch (DISPLAY, DefaultScreen (DISPLAY), p, &result);

  FcPatternDestroy (p);

  if (!match)
    return false;

  f = XftFontOpenPattern (DISPLAY, match);

  if (!f)
    {
      FcPatternDestroy (match);
      return false;
    }

  FT_Face face = XftLockFace (f);

  slow = !FT_IS_FIXED_WIDTH (face);

  int ftheight = 0;

  for (;;)
    {
      XGlyphInfo g1, g2;
      FcChar8 c;

      c = 'i'; XftTextExtents8 (DISPLAY, f, &c, 1, &g1);
      c = 'W'; XftTextExtents8 (DISPLAY, f, &c, 1, &g2);

      if (g1.xOff != g2.xOff) // don't simply trust the font
        slow = true;

      width = g2.xOff;
      ascent = (face->size->metrics.ascender + 63) >> 6;
      descent = (-face->size->metrics.descender + 63) >> 6;
      height = ascent + descent;

      if (height <= prop.height || !prop.height)
        break;

      if (ftheight)
        {
          // take smaller steps near the end
          if (height > prop.height + 1) ftheight++;
          if (height > prop.height + 2) ftheight++;
          if (height > prop.height + 3) ftheight++;

          FT_Set_Pixel_Sizes (face, 0, ftheight -= height - prop.height);
        }
      else
        FT_Set_Pixel_Sizes (face, 0, ftheight = prop.height);
    }

  XftUnlockFace (f);

  return true;
}

#if 0
void rxvt_font::gen_coverage_swath (unsigned int page)
{
  cvr[page] = new swath;

  for (int i = 0; i < SWATHSIZE; i++)
    cvr[page][i] = 0;

  populate_coverage_swath (cvr[page], page << SWATHBITS, ((page + 1) << SWATHBITS) - 1);
}
#endif

bool
rxvt_font_xft::has_codepoint (uint32_t unicode)
{
  return XftCharExists (DISPLAY, f, unicode);
}

void
rxvt_font_xft::draw (int x, int y,
                     const text_t *text, int len,
                     int fg, int bg)
{
  if (!d)
    {
      dR;
      d = XftDrawCreate (DISPLAY, DRAWABLE, XVISUAL, XCMAP);
    }

  if (bg >= 0 && bg != Color_bg)
    XftDrawRect (d, &r->PixColors[bg].c, x, y, r->TermWin.fwidth * len, r->TermWin.fheight);
  else
    clear_rect (x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

  if (!slow && width == r->TermWin.fwidth)
    {
      if (sizeof (text_t) == sizeof (FcChar16))
        XftDrawString16 (d, &r->PixColors[fg].c, f, x, y + r->TermWin.fbase, (const FcChar16 *)text, len);
      else
        XftDrawString32 (d, &r->PixColors[fg].c, f, x, y + r->TermWin.fbase, (const FcChar32 *)text, len);
    }
  else
    {
      while (len)
        {
          if (*text != NOCHAR && *text != ' ')
            {
              if (sizeof (text_t) == sizeof (FcChar16))
                XftDrawString16 (d, &r->PixColors[fg].c, f, x, y + r->TermWin.fbase, (const FcChar16 *)text, 1);
              else
                XftDrawString32 (d, &r->PixColors[fg].c, f, x, y + r->TermWin.fbase, (const FcChar32 *)text, 1);
            }

          x += r->TermWin.fwidth;
          text++;
          len--;
        }
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////

rxvt_fontset::rxvt_fontset (rxvt_t r)
#ifdef EXPLICIT_CONTEXT
: r(r)
#endif
{
  clear ();
}

rxvt_fontset::~rxvt_fontset ()
{
  clear ();
}

void
rxvt_fontset::clear ()
{
  for (rxvt_font **i = fonts.begin (); i != fonts.end(); i++)
    FONT_UNREF (*i);

  fonts.clear ();
  base_id = 0;
  base_prop.height = 0x7fffffff;
  base_prop.weight = rxvt_fontprop::medium;
  base_prop.slant = rxvt_fontprop::roman;

  fallback = fallback_fonts;
}

rxvt_font *
rxvt_fontset::new_font (const char *name, codeset cs)
{
  rxvt_font *f;
  
  if (!name || !*name)
    {
      name = "";
      f = new rxvt_font_default;
    }
#if XFT
  else if (!strncmp (name, "xft:", 4))
    {
      name += 4;
      f = new rxvt_font_xft;
    }
#endif
  else if (!strncmp (name, "x:", 2))
    {
      name += 2;
      f = new rxvt_font_x11;
    }
  else
    f = new rxvt_font_x11;

  f->set_term (r);
  f->set_name (strdup (name));

  f->cs = cs;
  f->loaded = false;

  return f;
}

/////////////////////////////////////////////////////////////////////////////

void
rxvt_fontset::add_fonts (const char *desc)
{
  if (desc)
    {
      char buf[512];
      const char *end;

      do
        {
          while (*desc <= ' ') desc++;

          if (*desc == '[')
            {
              fprintf (stderr, "extra font parameters not yet supported, skipping.\n");

              const char *extra = desc++;

              desc = strchr (desc, ']');

              if (!desc)
                {
                  fprintf (stderr, "ERROR: opening '[' without closing ']' in font specification.\n");
                  break;
                }

              desc++;
              while (*desc <= ' ') desc++;
            }

          end = strchr (desc, ',');
          if (!end)
            end = desc + strlen (desc);

          if (end - desc < 511)
            {
              strncpy (buf, desc, end - desc);
              buf[end - desc] = 0;

              fonts.push_back (new_font (buf, CS_UNICODE));
            }

          desc = end + 1;
        }
      while (*end);
    }
}

bool
rxvt_fontset::realize_font (int i)
{
  if (fonts[i]->loaded)
    return true;

  fonts[i]->loaded = true;

  if (!fonts[i]->load (base_prop))
    {
      fonts[i]->cs = CS_UNKNOWN;
      return false;
    }

  return true;
}

void
rxvt_fontset::populate (const char *desc)
{
  clear ();

  fonts.push_back (new_font (0, CS_UNICODE));
  realize_font (0);

  add_fonts (desc);

  if (!base_id)
    base_id = 1;

  // we currently need a base-font, no matter what
  if (fonts.size () <= base_id || !realize_font (base_id))
    {
      add_fonts ("fixed");
      base_id = fonts.size () - 1;
    }

  if (fonts.size () <= base_id || !realize_font (base_id))
    {
      fprintf (stderr, "unable to load a base font, please provide one using -fn fontname\n");
      exit (1);
    }

  base_prop = fonts[base_id]->properties ();
}

int
rxvt_fontset::find_font (uint32_t unicode)
{
  for (int i = 0; i < fonts.size (); i++)
    {
      rxvt_font *f = fonts[i];

      if (!f->loaded)
        {
          if (FROM_UNICODE (f->cs, unicode) == NOCHAR)
            goto next_font;

          if (!realize_font (i))
            goto next_font;
        }

      if (f->cs != CS_UNKNOWN && f->has_codepoint (unicode))
        return i;

    next_font:
      if (i == fonts.size () - 1 && fallback->name)
        {
          fonts.push_back (new_font (fallback->name, fallback->cs));
          fallback++;
          i = 0;
        }
    }

  return 0; /* we must return SOME font */
}



