/*--------------------------------*-C-*---------------------------------*;
 * File:	defaultfont.C
 *----------------------------------------------------------------------*
 * Copyright (c) 2003-2004 Marc Lehmann <pcg@goof.com>
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

#define DISPLAY  r->display->display
#define TGC      r->TermWin.gc

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
  { CS_JIS0208_1990_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0208*-0"  },
  { CS_JIS0212_1990_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0212*-0"  },
#endif

#if ENCODING_CN || ENCODING_CN_EXT
# if XFT
  { CS_BIG5_EXT,       "xft:AR PL Mingti2L Big5"                   },
  { CS_BIG5_EXT,       "xft:AR PL KaitiM Big5"                     },
  { CS_GB2312_1980_0,  "xft:AR PL KaitiM GB"                       },
  { CS_GB2312_1980_0,  "xft:AR PL SungtiL GB"                      },
# endif
  { CS_BIG5,            "-*-*-*-*-*-*-*-*-*-*-c-*-big5-0"          },
  { CS_BIG5_PLUS,       "-*-*-*-*-*-*-*-*-*-*-c-*-big5p-0"         },
  { CS_BIG5_EXT,        "-*-*-*-*-*-*-*-*-*-*-c-*-big5.eten-0"     },
  { CS_CNS11643_1992_1, "-*-*-*-*-*-*-*-*-*-*-c-*-gb2312*-0"       },
  { CS_CNS11643_1992_1, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-1"     },
  { CS_CNS11643_1992_2, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-2"     },
  { CS_CNS11643_1992_3, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-3"     },
  { CS_CNS11643_1992_4, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-4"     },
  { CS_CNS11643_1992_5, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-5"     },
  { CS_CNS11643_1992_6, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-6"     },
  { CS_CNS11643_1992_7, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-7"     },
  { CS_CNS11643_1992_F, "-*-*-*-*-*-*-*-*-*-*-c-*-cns11643*-f"     },
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

#if UNICODE_3 && XFT
  { CS_UNICODE,      "xft:Code2001"                                }, // contains many plane-1 characters
#endif

  { CS_UNKNOWN, 0 }
};

/////////////////////////////////////////////////////////////////////////////

#if XFT
rxvt_drawable::~rxvt_drawable ()
{
  if (xftdrawable)
    XftDrawDestroy (xftdrawable);
}

rxvt_drawable::operator XftDraw *()
{
  if (!xftdrawable)
    xftdrawable = XftDrawCreate (display->display, drawable, display->visual, display->cmap);

  return xftdrawable;
}
#endif

/////////////////////////////////////////////////////////////////////////////

static void *enc_buf;
static uint32_t enc_len;

static inline void *
get_enc_buf (uint32_t len)
{
  if (len > enc_len)
    {
      free (enc_buf);
      enc_buf = malloc (len);
    }

  return enc_buf;
}

static const char *
enc_char (const text_t *text, uint32_t len, codeset cs, bool &zero)
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
enc_xchar2b (const text_t *text, uint32_t len, codeset cs, bool &zero)
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
rxvt_font::clear_rect (rxvt_drawable &d, int x, int y, int w, int h, int color)
{
  if (color == Color_bg)
    XClearArea (d.display->display, d, x, y, w, h, FALSE);
  else if (color >= 0)
    {
#if XFT
      XftDrawRect (d, &r->PixColors[color].c, x, y, w, h);
#else
      XSetForeground (d.display->display, TGC, r->PixColors[color]);
      XFillRectangle (d.display->display, d, TGC, x, y, w, h);
#endif
    }
}

#include "table/linedraw.h"

struct rxvt_font_default : rxvt_font {

  rxvt_fontprop properties ()
  {
    rxvt_fontprop p;

    p.width = p.height = 1;
    p.weight = rxvt_fontprop::medium;
    p.slant = rxvt_fontprop::roman;

    return p;
  }

  bool load (const rxvt_fontprop &prop)
  {
    width = 1; height = 1;
    ascent = 1; descent = 0;

    set_name (strdup ("built-in pseudofont"));

    return true;
  }

  bool has_codepoint (unicode_t unicode)
  {
    if (unicode <= 0x001f)
      return true;

    if (unicode <= 0x007f)
      return false;

    if (unicode <= 0x009f)
      return true;

    if (unicode >= 0x2500 && unicode <= 0x259f)
      return true;

    if (IS_COMPOSE (unicode))
      return true;

    switch (unicode)
      {
        case ZERO_WIDTH_CHAR:
          return true;
      }

    return false;
  }

  void draw (rxvt_drawable &d, int x, int y,
             const text_t *text, int len,
             int fg, int bg);
};

void
rxvt_font_default::draw (rxvt_drawable &d, int x, int y,
                         const text_t *text, int len,
                         int fg, int bg)
{
  clear_rect (d, x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

  XSetForeground (d.display->display, TGC, r->PixColors[fg]);

  while (len--)
    {
#if ENABLE_COMBINING
      compose_char *cc;
#endif
      text_t t = *text++;

      if (0x2500 <= t && t <= 0x259f)
        {
          uint16_t offs = linedraw_offs[t - 0x2500];
          uint32_t *a = linedraw_command + (offs >> 4);
          uint32_t *b = a + (offs & 15);

          int W = r->TermWin.fwidth;
          int H = r->TermWin.fheight;

          int x_[16];
          int y_[16];

          for (int i = 0; i <= 8; i++)
            {
              x_[i] = x + ((W-1) * i + (i*7/8)) / 8;
              y_[i] = y + ((H-1) * i + (i*7/8)) / 8;
            }

          x_[10] = x + (W - 1) / 2; x_[9] = x_[10] - 1; x_[11] = x_[10] + 1;
          y_[10] = y + (H - 1) / 2; y_[9] = y_[10] - 1; y_[11] = y_[10] + 1;

          XGCValues gcv;

          gcv.cap_style = CapButt;
          gcv.line_width = 0;
          XChangeGC (d.display->display, TGC, GCLineWidth | GCCapStyle, &gcv);

          while (a < b)
            {
              uint32_t command = *a++;

              int op = (command >> 24) & 255;
              int a  = (command >> 20) & 15;
              int b  = (command >> 16) & 15;
              int x1 = x_[(command >> 12) & 15];
              int y1 = y_[(command >>  8) & 15];
              int x2 = x_[(command >>  4) & 15];
              int y2 = y_[(command >>  0) & 15];

              switch (op)
                {
                  case 0: // line
                    XDrawLine (d.display->display, d, TGC, x1, y1, x2, y2);
                    break;

                  case 1: // rectangle, possibly stippled
                    if (a)
                      {
                        static char bm[] = { 0,0 , 3,1 , 1,2 , 1,0 };
 
                        gcv.fill_style = FillStippled;
                        gcv.stipple = XCreateBitmapFromData (d.display->display, d, bm + a * 2, 2, 2);
                        gcv.ts_x_origin = x;
                        gcv.ts_y_origin = y;

                        XChangeGC (d.display->display, TGC,
                                   GCFillStyle | GCStipple | GCTileStipXOrigin | GCTileStipYOrigin,
                                   &gcv);
                      }

                    XFillRectangle (d.display->display, d, TGC, x1, y1, x2 - x1 + 1, y2 - y1 + 1);

                    if (a)
                      {
                        XFreePixmap (d.display->display, gcv.stipple);
                        gcv.stipple = 0;
                        gcv.fill_style = FillSolid;
                        XChangeGC (d.display->display, TGC, GCFillStyle, &gcv);
                      }
                    break;
                  case 2: // arc
                    XDrawArc (d.display->display, d, TGC,
                              x1 - W/2, y1 - H/2, W-1, H-1,
                              (a - 1) * 90*64, (b - 1) * 90*64);
                    break;
                }
            }
        }
#if ENABLE_COMBINING
      else if (IS_COMPOSE (t) && (cc = rxvt_composite[t]))
        {
          rxvt_font *f1 = (*fs)[fs->find_font (cc->c1)];
          f1->draw (d, x, y, &(t = cc->c1), 1, fg, bg);
          if (cc->c2 != NOCHAR)
            {
              // prefer font of first character, for no good reasons
              rxvt_font *f2 = f1->has_codepoint (cc->c2)
                              ? f1
                              : (*fs)[fs->find_font (cc->c2)];

              f2->draw (d, x, y, &(t = cc->c2), 1, fg, -1);
            }
        }
#endif
      else
        switch (t)
          {
            case ZERO_WIDTH_CHAR:
              break;

            default:
              int w = 0;
              while (len > 0 && *text == NOCHAR)
                {
                  ++text;
                  --len;
                  w += r->TermWin.fwidth;
                }

              XDrawRectangle (d.display->display, d, TGC, x + 2, y + 2,
                              w + r->TermWin.fwidth - 4, r->TermWin.fheight - 4);
              x += w;
          }

      x += r->TermWin.fwidth;
    }
}

/////////////////////////////////////////////////////////////////////////////

struct rxvt_font_x11 : rxvt_font {
  rxvt_font_x11 () { f = 0; }

  void clear ();

  rxvt_fontprop properties ();

  bool load (const rxvt_fontprop &prop);

  bool has_codepoint (unicode_t unicode);

  void draw (rxvt_drawable &d, int x, int y,
             const text_t *text, int len,
             int fg, int bg);

  XFontStruct *f;
  codeset cs;
  bool enc2b, encm;

  char *get_property (XFontStruct *f, const char *property, const char *repl) const;
  bool set_properties (rxvt_fontprop &p, int height, const char *weight, const char *slant, int avgwidth);
  bool set_properties (rxvt_fontprop &p, XFontStruct *f);
  bool set_properties (rxvt_fontprop &p, const char *name);
};

char *
rxvt_font_x11::get_property (XFontStruct *f, const char *property, const char *repl) const
{
  unsigned long value;

  if (XGetFontProperty (f, XInternAtom (DISPLAY, property, 0), &value))
    return XGetAtomName (DISPLAY, value);
  else
    return rxvt_strdup (repl);
}

rxvt_fontprop
rxvt_font_x11::properties ()
{
  rxvt_fontprop p;
  set_properties (p, f);
  return p;
}

bool
rxvt_font_x11::set_properties (rxvt_fontprop &p, int height, const char *weight, const char *slant, int avgwidth)
{
  p.width = avgwidth ? (avgwidth + 1) / 10 : (height + 1) / 2;
  p.height = height;
  p.weight = *weight == 'B' || *weight == 'b' ? rxvt_fontprop::bold : rxvt_fontprop::medium;
  p.slant  = *slant == 'r' || *slant == 'R' ? rxvt_fontprop::roman : rxvt_fontprop::italic;

  return true;
}

bool
rxvt_font_x11::set_properties (rxvt_fontprop &p, XFontStruct *f)
{
  unsigned long height;
  if (!XGetFontProperty (f, XInternAtom (DISPLAY, "PIXEL_SIZE", 0), &height))
    return false;

  unsigned long avgwidth;
  if (!XGetFontProperty (f, XInternAtom (DISPLAY, "AVERAGE_WIDTH", 0), &avgwidth))
    avgwidth = 0;

  char *weight = get_property (f, "WEIGHT_NAME", "medium");
  char *slant  = get_property (f, "SLANT", "r");

  set_properties (p, height, weight, slant, avgwidth);

  free (weight);
  free (slant);

  return true;
}

bool
rxvt_font_x11::set_properties (rxvt_fontprop &p, const char *name)
{
  int slashes = 0;
  const char *comp[13];

  for (const char *c = name; *c; c++)
    if (*c == '-')
      {
        comp[slashes++] = c + 1;
        if (slashes >= 13)
          break;
      }

  /* can we short-circuit the costly XLoadQueryFont? */
  if (slashes >= 13
      && (*comp[ 6] >= '1' && *comp[ 6] <= '9')
      && (*comp[11] >= '0' && *comp[11] <= '9'))
    return set_properties (p, atoi (comp[6]), comp[2], comp[3], atoi (comp[11]));

  XFontStruct *f = XLoadQueryFont (DISPLAY, name);

  if (f)
    {
      // the font should really exist now. if not, we have a problem
      // (e.g. if the user did xset fp rehash just when we were searching fonts).
      // in that case, just return garbage.
      bool ret = set_properties (p, f);
      XFreeFont (DISPLAY, f);
      return ret;
    }
  else
    return false;
}

// fix the size of scalable fonts
static void
fix_scalable (char *buf, const char *name, const rxvt_fontprop &prop)
{
  int slashes = 0;
  const char *size;

  for (const char *c = name; *c; c++)
    if (*c == '-')
      {
        if (slashes == 6)
          size = c + 1;

        if (++slashes >= 13)
          break;
      }

  if (slashes >= 13 && size[0] == '0')
    {
      strncpy (buf, name, size - name);
      buf += size - name;
      buf += sprintf (buf, "%d", prop.height);
      strcpy (buf, size + 1);
    }
  else
    strcpy (buf, name);
}

bool
rxvt_font_x11::load (const rxvt_fontprop &prop)
{
  clear ();

  char **list;
  int count;
  list = XListFonts (DISPLAY, name, 512, &count);
  set_name (0);

  if (!list)
    return false;

  int bestdiff = 0x7fffffff;
  for (int i = 0; i < count; i++)
    {
      rxvt_fontprop p;
      char fname[1024];
      fix_scalable (fname, list[i], prop);

      if (!set_properties (p, fname))
        continue;

      if (p.height > prop.height) // weed out too large fonts
        continue;

      int diff = (prop.height - p.height) * 32
               + abs (prop.weight - p.weight)
               + abs (prop.slant  - p.slant );

      if (!name // compare against best found so far
          || diff < bestdiff)
        {
          set_name (strdup (fname));
          bestdiff = diff;
        }
    }

  XFreeFontNames (list);

  if (!name)
    return false;

  f = XLoadQueryFont (DISPLAY, name);

  if (!f)
    return false;

  char *registry = get_property (f, "CHARSET_REGISTRY", 0);
  char *encoding = get_property (f, "CHARSET_ENCODING", 0);

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

  free (registry);
  free (encoding);

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
rxvt_font_x11::has_codepoint (unicode_t unicode)
{
  uint32_t ch = FROM_UNICODE (cs, unicode);

  if (ch == NOCHAR)
    return false;

  /* check wether the character exists in _this_ font. horrible. */
  XCharStruct *xcs;

  if (encm)
    {
      unsigned char byte1 = ch >> 8;
      unsigned char byte2 = ch & 255;

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
rxvt_font_x11::draw (rxvt_drawable &d, int x, int y,
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
          XChangeGC (d.display->display, TGC, GCForeground | GCBackground | GCFont, &v);
          XDrawImageString16 (d.display->display, d, TGC, x, y + base, xc, len);
        }
      else
        {
          clear_rect (d, x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

          XChangeGC (d.display->display, TGC, GCForeground | GCFont, &v);
          
          if (slow)
            {
              do
                {
                  if (xc->byte1 || xc->byte2)
                    XDrawString16 (d.display->display, d, TGC, x, y + base, xc, 1);

                  x += r->TermWin.fwidth;
                  xc++; len--;
                }
              while (len);
            }
          else
            XDrawString16 (d.display->display, d, TGC, x, y + base, xc, len);
        }
    }
  else
    {
      const char *xc = enc_char (text, len, cs, slow);

      if (bg == Color_bg && !slow)
        {
          XChangeGC (d.display->display, TGC, GCForeground | GCBackground | GCFont, &v);
          XDrawImageString (d.display->display, d, TGC, x, y + base, xc, len);
        }
      else
        {
          clear_rect (d, x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

          XChangeGC (d.display->display, TGC, GCForeground | GCFont, &v);
          
          if (slow)
            {
              do
                {
                  if (*xc)
                    XDrawString (d.display->display, d, TGC, x, y + base, xc, 1);

                  x += r->TermWin.fwidth;
                  xc++; len--;
                }
              while (len);
            }
          else
            XDrawString (d.display->display, d, TGC, x, y + base, xc, len);
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
  rxvt_font_xft () { f = 0; }

  void clear ();

  rxvt_fontprop properties ();

  bool load (const rxvt_fontprop &prop);

  void draw (rxvt_drawable &d, int x, int y,
             const text_t *text, int len,
             int fg, int bg);

  bool has_codepoint (unicode_t unicode);

protected:
  XftFont *f;
};

void
rxvt_font_xft::clear ()
{
  if (f)
    {
      XftFontClose (DISPLAY, f);
      f = 0;
    }
}

rxvt_fontprop
rxvt_font_xft::properties ()
{
  rxvt_fontprop p;

  FT_Face face = XftLockFace (f);

  p.width = width; p.height = height;
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

#if 0 // clipping unfortunately destroys our precious double-width-characters
  // clip width, we can't do better, or can we?
  if (FcPatternGet (p, FC_CHAR_WIDTH, 0, &v) != FcResultMatch)
    FcPatternAddInteger (p, FC_CHAR_WIDTH, prop.width);
#endif

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

bool
rxvt_font_xft::has_codepoint (unicode_t unicode)
{
  return XftCharExists (DISPLAY, f, unicode);
}

void
rxvt_font_xft::draw (rxvt_drawable &d, int x, int y,
                     const text_t *text, int len,
                     int fg, int bg)
{
  clear_rect (d, x, y, r->TermWin.fwidth * len, r->TermWin.fheight, bg);

  if (!slow && width == r->TermWin.fwidth && 0)
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
              int fwidth = r->TermWin.fwidth;
              if (len >= 2 && text[1] == NOCHAR)
                fwidth *= 2;

              XGlyphInfo extents;
              if (sizeof (text_t) == sizeof (FcChar16))
                {
                  XftTextExtents16 (d.display->display, f, (const FcChar16 *)text, 1, &extents);
                  XftDrawString16 (d, &r->PixColors[fg].c, f, x + extents.x + (fwidth - extents.width) / 2,
                                   y + r->TermWin.fbase, (const FcChar16 *)text, 1);
                }
              else
                {
                  XGlyphInfo extents;
                  XftTextExtents32 (d.display->display, f, (const FcChar32 *)text, 1, &extents);
                  XftDrawString32 (d, &r->PixColors[fg].c, f, x + extents.x + (fwidth - extents.width) / 2,
                                   y + r->TermWin.fbase, (const FcChar32 *)text, 1);
                }
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
: r (r)
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
  for (rxvt_font **i = fonts.begin (); i != fonts.end (); i++)
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

  f->fs = this;
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

              //const char *extra = desc++; // not yet used

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

bool
rxvt_fontset::populate (const char *desc)
{
  clear ();

  fonts.push_back (new_font (0, CS_UNICODE));
  realize_font (0);

  add_fonts (desc);

  if (!base_id)
    base_id = 1;

  // we currently need a base-font, no matter what
  if ((int)fonts.size () <= base_id || !realize_font (base_id))
    {
      puts ("unable to load specified font (s), falling back to 'fixed'\n");
      add_fonts ("fixed");
      base_id = fonts.size () - 1;
    }

  if ((int)fonts.size () <= base_id || !realize_font (base_id))
    return false;

  base_prop = fonts[base_id]->properties ();

  return true;
}

int
rxvt_fontset::find_font (unicode_t unicode)
{
  for (unsigned int i = !!(0x20 <= unicode && unicode <= 0x7f); // skip pseudo-font for ascii
       i < fonts.size ();
       i++)
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



