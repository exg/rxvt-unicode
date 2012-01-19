/*----------------------------------------------------------------------*
 * File:	rxvtfont.C
 *----------------------------------------------------------------------*
 * Copyright (c) 2003-2008 Marc Lehmann <schmorp@schmorp.de>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *---------------------------------------------------------------------*/

#include "../config.h"
#include "rxvt.h"
#include "rxvtutil.h"
#include "rxvtfont.h"

#include <stdlib.h>

#include <inttypes.h>

#if XFT
# include <fontconfig/fontconfig.h>
#endif

#define MAX_OVERLAP_ROMAN  (8 + 2)	// max. character width in 8ths of the base width
#define MAX_OVERLAP_ITALIC (8 + 3)	// max. overlap for italic fonts

#define OVERLAP_OK(w,wcw,prop) ((w) <= (			\
  (prop)->slant >= rxvt_fontprop::italic			\
    ? ((prop)->width * (wcw) * MAX_OVERLAP_ITALIC + 7) >> 3	\
    : ((prop)->width * (wcw) * MAX_OVERLAP_ROMAN  + 7) >> 3	\
  ))

static const struct rxvt_fallback_font {
  codeset cs;
  const char *name;
} fallback_fonts[] = {
  { CS_ISO8859_1,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-1"           },
  { CS_ISO8859_15,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-15"          },
  { CS_ISO8859_15,   "-*-*-*-r-*--*-*-*-*-c-*-fcd8859-15"          },

#if ENCODING_EU
  // cyrillic
  { CS_KOI8_R,        "-*-*-*-r-*--*-*-*-*-c-*-koi8-r"             },
  { CS_KOI8_U,        "-*-*-*-r-*--*-*-*-*-c-*-koi8-u"             },

  { CS_ISO8859_2,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-2"           },
  { CS_ISO8859_3,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-3"           },
  { CS_ISO8859_4,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-4"           },
  { CS_ISO8859_5,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-5"           },
  { CS_ISO8859_6,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-6"           },
  { CS_ISO8859_7,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-7"           },
  { CS_ISO8859_8,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-8"           },
  { CS_ISO8859_9,    "-*-*-*-r-*--*-*-*-*-c-*-iso8859-9"           },
  { CS_ISO8859_10,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-10"          },
  { CS_ISO8859_11,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-11"          },
  { CS_ISO8859_13,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-13"          },
  { CS_ISO8859_14,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-14"          },
  { CS_ISO8859_16,   "-*-*-*-r-*--*-*-*-*-c-*-iso8859-16"          },

# if XFT
  { CS_KOI8_U,       "xft::lang=ru"                                },

  { CS_ISO8859_5,    "xft::lang=ru"                                },
  { CS_ISO8859_6,    "xft::lang=ar"                                },
  { CS_ISO8859_7,    "xft::lang=el"                                },
  { CS_ISO8859_8,    "xft::lang=he"                                },
  { CS_ISO8859_9,    "xft::lang=tr"                                },
  { CS_ISO8859_10,   "xft::lang=se"                                },
  { CS_ISO8859_11,   "xft::lang=th"                                },
# endif
#endif

  // japanese
#if ENCODING_JP || ENCODING_JP_EXT
# if XFT
  // prefer xft for complex scripts
  { CS_JIS0208_1990_0, "xft:Sazanami Mincho:antialias=false"       },
  { CS_JIS0208_1990_0, "xft:Kochi Gothic:antialias=false"          },
  { CS_JIS0208_1990_0, "xft:Mincho:antialias=false"                },
  { CS_JIS0208_1990_0, "xft::lang=ja:antialias=false"              },
# endif
  { CS_JIS0201_1976_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0201*-0"  },
  { CS_JIS0208_1990_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0208*-0"  },
  { CS_JIS0212_1990_0, "-*-mincho-*-r-*--*-*-*-*-c-*-jisx0212*-0"  },
  { CS_JIS0201_1976_0, "-*-*-*-r-*--*-*-*-*-c-*-jisx0201*-0"       },
  { CS_JIS0208_1990_0, "-*-*-*-r-*--*-*-*-*-c-*-jisx0208*-0"       },
  { CS_JIS0212_1990_0, "-*-*-*-r-*--*-*-*-*-c-*-jisx0212*-0"       },
#endif

#if ENCODING_ZH || ENCODING_ZH_EXT
# if XFT
  { CS_GBK_0,          "xft:AR PL KaitiM GB"                       },
  { CS_GBK_0,          "xft:AR PL SungtiL GB"                      },
  { CS_GBK_0,          "xft::lang=zh"                              },
  { CS_BIG5_EXT,       "xft:AR PL Mingti2L Big5"                   },
  { CS_BIG5_EXT,       "xft:AR PL KaitiM Big5"                     },
  { CS_GB2312_1980_0,  "xft:AR PL KaitiM GB"                       },
  { CS_GB2312_1980_0,  "xft:AR PL SungtiL GB"                      },
  { CS_GB2312_1980_0,  "xft::lang=zh"                              },
# endif
  { CS_GBK_0,           "-*-*-*-*-*-*-*-*-*-*-c-*-gbk*-0"          },
  { CS_BIG5,            "-*-*-*-*-*-*-*-*-*-*-c-*-big5-0"          },
  { CS_BIG5_PLUS,       "-*-*-*-*-*-*-*-*-*-*-c-*-big5p-0"         },
  { CS_BIG5_EXT,        "-*-*-*-*-*-*-*-*-*-*-c-*-big5.eten-0"     },
  { CS_GB2312_1980_0,   "-*-*-*-*-*-*-*-*-*-*-c-*-gb2312*-0"       },
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

#if ENCODING_KR
  { CS_KSC5601_1987_0,  "-baekmuk-gulim-*-*-*-*-*-*-*-*-c-*-ksc5601*" },
  { CS_KSC5601_1987_0,  "-*-*-*-*-*-*-*-*-*-*-c-*-ksc5601*"        },
# if XFT
  { CS_KSC5601_1987_0,  "xft:Baekmuk Gulim:antialias=false"        },
  { CS_KSC5601_1987_0,  "xft::lang=ko:antialias=false"             },
# endif
#endif

  //{ CS_UNICODE,      "-*-unifont-*-*-*-*-*-*-*-*-c-*-iso10646-1"   }, // this gem of a font has actual dotted circles within the combining character glyphs.
#if XFT
  { CS_UNICODE,      "xft:Bitstream Vera Sans Mono:antialias=false:autohint=true" },
  { CS_UNICODE,      "xft:Courier New:antialias=false:autohint=true"              },
  { CS_UNICODE,      "xft:Andale Mono:antialias=false:autohint=false"             },
  { CS_UNICODE,      "xft:Arial Unicode MS:antialias=false:autohint=false"        },

  // FreeMono is usually uglier than x fonts, so try after the others
  { CS_UNICODE,      "xft:FreeMono:autohint=true"                  },
#endif

  // generic font fallback, put this last, as many iso10646 fonts have extents
  // specified for all glyphs in the range they cover, but most are simply empty
  //{ CS_UNICODE,      "-*-*-*-r-*-*-*-*-*-*-c-*-iso10646-1"         },
  //{ CS_UNICODE,      "-*-*-*-r-*-*-*-*-*-*-m-*-iso10646-1"         },
  { CS_UNKNOWN, 0 }
};

// these characters are used to guess the font height and width
// pango uses a similar algorithm and doesn't trust the font either.
static uint16_t extent_test_chars[] = {
  '0', '1', '8', 'a', 'd', 'x', 'm', 'y', 'g', 'W', 'X', '\'', '_',
  0x00cd, 0x00d5, 0x0114, 0x0177, 0x0643,	// ÍÕĔŷﻙ
  0x304c, 0x672c,				// が本
};

#define dTermDisplay Display *disp = term->dpy
#define dTermGC      GC gc = term->gc

/////////////////////////////////////////////////////////////////////////////

static const char *
enc_char (const text_t *text, uint32_t len, codeset cs, bool &zero)
{
  uint8_t *buf = rxvt_temp_buf<uint8_t> (len);
  uint8_t *res = buf;

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

  return (const char *)res;
}

static const XChar2b *
enc_xchar2b (const text_t *text, uint32_t len, codeset cs, bool &zero)
{
  XChar2b *buf = rxvt_temp_buf<XChar2b> (len);
  XChar2b *res = buf;

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

  return res;
}

/////////////////////////////////////////////////////////////////////////////

rxvt_font::rxvt_font ()
: name(0), width(rxvt_fontprop::unset), height(rxvt_fontprop::unset)
{
}

void
rxvt_font::set_name (char *name_)
{
  if (name == name_)
    return;

  if (name) free (name); // let the compiler optimize
  name = name_;
}

void
rxvt_font::clear_rect (rxvt_drawable &d, int x, int y, int w, int h, int color) const
{
  dTermDisplay;
  dTermGC;

  if (color == Color_bg || color == Color_transparent)
    XClearArea (disp, d, x, y, w, h, false);
  else if (color >= 0)
    {
#if XFT
      Picture dst;

# ifdef HAVE_BG_PIXMAP
      if (term->bg_pixmap
          && !term->pix_colors[color].is_opaque ()
          && ((dst = XftDrawPicture (d))))
        {
          XClearArea (disp, d, x, y, w, h, false);

          Picture solid_color_pict = XftDrawSrcPicture (d, &term->pix_colors[color].c);
          XRenderComposite (disp, PictOpOver, solid_color_pict, None, dst, 0, 0, 0, 0, x, y, w, h);
        }
      else
# endif
        XftDrawRect (d, &term->pix_colors[color].c, x, y, w, h);

#else
      XSetForeground (disp, gc, term->pix_colors[color]);
      XFillRectangle (disp, d, gc, x, y, w, h);
#endif
    }
}

/////////////////////////////////////////////////////////////////////////////

struct rxvt_font_default : rxvt_font {
  struct rxvt_fontset *fs;

  rxvt_font_default (rxvt_fontset *fs)
  : rxvt_font ()
  {
    this->fs = fs;
  }

  rxvt_fontprop properties ()
  {
    rxvt_fontprop p;

    p.width = p.height = 1;
    p.ascent = rxvt_fontprop::unset;
    p.weight = rxvt_fontprop::medium;
    p.slant = rxvt_fontprop::roman;

    return p;
  }

  bool load (const rxvt_fontprop &prop, bool force_prop)
  {
    width = 1; height = 1;
    ascent = 1; descent = 0;

    set_name (strdup ("built-in support font"));

    return true;
  }

  bool has_char (unicode_t unicode, const rxvt_fontprop *prop, bool &careful) const
  {
    careful = false;

    if (unicode <= 0x001f)
      return true;

    if (unicode <= 0x007f)
      return false;

    if (unicode <= 0x009f)
      return true;

#ifdef BUILTIN_GLYPHS
    if (unicode >= 0x2500 && unicode <= 0x259f &&
        !term->option (Opt_skipBuiltinGlyphs))
      return true;
#endif

    if (IS_COMPOSE (unicode))
      return true;

    switch (unicode)
      {
        case ZERO_WIDTH_CHAR:
        case NOCHAR:
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
  dTermDisplay;
  dTermGC;

  clear_rect (d, x, y, term->fwidth * len, term->fheight, bg);

  XSetForeground (disp, gc, term->pix_colors[fg]);

  while (len)
    {
#if ENABLE_COMBINING
      compose_char *cc;
#endif
      const text_t *tp = text;
      text_t t  = *tp;

      while (++text, --len && *text == NOCHAR)
        ;

      int width = text - tp;
      int fwidth = term->fwidth * width;

#ifdef BUILTIN_GLYPHS
      if (0x2500 <= t && t <= 0x259f)
        {
# include "table/linedraw.h"
          uint16_t offs = linedraw_offs[t - 0x2500];
          uint32_t *a = linedraw_command + (offs >> 4);
          uint32_t *b = a + (offs & 15);

          int W = fwidth;
          int H = term->fheight;

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
          XChangeGC (disp, gc, GCLineWidth | GCCapStyle, &gcv);

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
                    XDrawLine (disp, d, gc, x1, y1, x2, y2);
                    break;

                  case 1: // rectangle, possibly stippled
                    if (a)
                      {
                        static char bm[] = { 0,0 , 3,1 , 1,2 , 1,0 };

                        gcv.fill_style = FillStippled;
                        gcv.stipple = XCreateBitmapFromData (disp, d, bm + a * 2, 2, 2);
                        gcv.ts_x_origin = x;
                        gcv.ts_y_origin = y;

                        XChangeGC (disp, gc,
                                   GCFillStyle | GCStipple | GCTileStipXOrigin | GCTileStipYOrigin,
                                   &gcv);
                      }

                    XFillRectangle (disp, d, gc, x1, y1, x2 - x1 + 1, y2 - y1 + 1);

                    if (a)
                      {
                        XFreePixmap (disp, gcv.stipple);
                        gcv.stipple = 0;
                        gcv.fill_style = FillSolid;
                        XChangeGC (disp, gc, GCFillStyle, &gcv);
                      }
                    break;
                  case 2: // arc
                    XDrawArc (disp, d, gc,
                              x1 - W/2, y1 - H/2, W-1, H-1,
                              (a - 1) * 90*64, (b - 1) * 90*64);
                    break;
                }
            }
        }
#else
      if (0)
        ;
#endif
#if ENABLE_COMBINING
      else if (IS_COMPOSE (t) && (cc = rxvt_composite[t]))
        {
          min_it (width, 2); // we only support wcwidth up to 2

          text_t chrs[2];
          chrs [1] = NOCHAR;

          *chrs = cc->c1;
          rxvt_font *f1 = (*fs)[fs->find_font_idx (cc->c1)];
          f1->draw (d, x, y, chrs, width, fg, bg);

          if (cc->c2 != NOCHAR)
            {
              bool careful;

              // prefer font of first character, for no good reasons
              *chrs = cc->c2;
              rxvt_font *f2 = (f1->has_char (cc->c2, 0, careful) && !careful)
                                ? f1
                                : (*fs)[fs->find_font_idx (cc->c2)];

              f2->draw (d, x, y, chrs, width, fg, Color_none);
            }
        }
#endif
      else
        switch (t)
          {
            case '\t':
            case ZERO_WIDTH_CHAR:
            case NOCHAR:
              break;

            default:
              XDrawRectangle (disp, d, gc, x + 2, y + 2,
                              fwidth - 4, term->fheight - 4);
          }

      x += fwidth;
    }
}

struct rxvt_font_overflow : rxvt_font {
  struct rxvt_fontset *fs;

  rxvt_font_overflow (rxvt_fontset *fs)
  : rxvt_font ()
  {
    this->fs = fs;
  }

  rxvt_fontprop properties ()
  {
    rxvt_fontprop p;

    p.width = p.height = 1;
    p.ascent = rxvt_fontprop::unset;
    p.weight = rxvt_fontprop::medium;
    p.slant = rxvt_fontprop::roman;

    return p;
  }

  bool load (const rxvt_fontprop &prop, bool force_prop)
  {
    width = 1; height = 1;
    ascent = 1; descent = 0;

    set_name (strdup ("built-in rendition overflow font"));

    return true;
  }

  bool has_char (unicode_t unicode, const rxvt_fontprop *prop, bool &careful) const
  {
    return false;
  }

  void draw (rxvt_drawable &d, int x, int y,
             const text_t *text, int len,
             int fg, int bg)
  {
    while (len)
      {
        int fid = fs->find_font_idx (*text);
        int w = 1;
        while (w < len && text[w] == NOCHAR)
          w++;
        (*fs)[fid]->draw (d, x, y, text, w, fg, bg);
        text += w;
        len -= w;
        x += term->fwidth * w;
      }
  }
};

/////////////////////////////////////////////////////////////////////////////

struct rxvt_font_x11 : rxvt_font {
  rxvt_font_x11 () { f = 0; }

  void clear ();

  rxvt_fontprop properties ();

  bool load (const rxvt_fontprop &prop, bool force_prop);

  bool has_char (unicode_t unicode, const rxvt_fontprop *prop, bool &careful) const;

  void draw (rxvt_drawable &d, int x, int y,
             const text_t *text, int len,
             int fg, int bg);

  bool slow; // whether this is a proportional font or has other funny characteristics
  XFontStruct *f;
  bool enc2b, encm;

  char *get_property (XFontStruct *f, Atom property, const char *repl) const;
  bool set_properties (rxvt_fontprop &p, int height, const char *weight, const char *slant, int avgwidth);
  bool set_properties (rxvt_fontprop &p, XFontStruct *f);
  bool set_properties (rxvt_fontprop &p, const char *name);
};

char *
rxvt_font_x11::get_property (XFontStruct *f, Atom property, const char *repl) const
{
  unsigned long value;

  if (XGetFontProperty (f, property, &value))
    return XGetAtomName (term->dpy, value);
  else
    return repl ? strdup (repl) : 0;
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
  p.width  = width != rxvt_fontprop::unset ? width
           : avgwidth                      ? (avgwidth + 1) / 10
                                           : (height + 1) / 2;
  p.height = height;
  p.ascent = rxvt_fontprop::unset;
  p.weight = *weight == 'B' || *weight == 'b' ? rxvt_fontprop::bold : rxvt_fontprop::medium;
  p.slant  = *slant == 'r' || *slant == 'R' ? rxvt_fontprop::roman : rxvt_fontprop::italic;

  return true;
}

bool
rxvt_font_x11::set_properties (rxvt_fontprop &p, XFontStruct *f)
{
  unsigned long height;

#if 0
  if (!XGetFontProperty (f, XInternAtom (term->dpy, "PIXEL_SIZE", 0), &height))
    return false;
#else
  height = f->ascent + f->descent;
#endif

  unsigned long avgwidth;
  if (!XGetFontProperty (f, term->xa [XA_AVERAGE_WIDTH], &avgwidth))
    avgwidth = 0;

  char *weight = get_property (f, term->xa [XA_WEIGHT_NAME], "medium");
  char *slant  = get_property (f, term->xa [XA_SLANT], "r");

  set_properties (p, height, weight, slant, avgwidth);

  free (weight);
  free (slant);

  p.ascent = f->ascent;

  return true;
}

bool
rxvt_font_x11::set_properties (rxvt_fontprop &p, const char *name)
{
  dTermDisplay;
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

  XFontStruct *f = XLoadQueryFont (disp, name);

  if (f)
    {
      // the font should really exist now. if not, we have a problem
      // (e.g. if the user did xset fp rehash just when we were searching fonts).
      // in that case, just return garbage.
      bool ret = set_properties (p, f);
      XFreeFont (disp, f);
      return ret;
    }
  else
    return false;
}

// fix the size of scalable fonts
static bool
replace_field (char **ptr, const char *name, int index, const char old, const char *replace)
{
  int slashes = 0;
  const char *field, *end;

  for (const char *c = name; *c; c++)
    if (*c == '-')
      {
        if (slashes == index)
          field = c + 1;

        if (slashes == index + 1)
          end = c;

        if (++slashes >= 13)
          break;
      }

  if (slashes >= 13 && (!old || *field == old))
    {
      size_t len = field - name;
      *ptr = (char *)malloc (len + strlen (replace) + strlen (end) + 1);
      memcpy (*ptr, name, len);
      strcpy (*ptr + len, replace);
      strcat (*ptr, end);

      return true;
    }
  else
    {
      *ptr = strdup (name);

      return false;
    }
}

bool
rxvt_font_x11::load (const rxvt_fontprop &prop, bool force_prop)
{
  dTermDisplay;

  clear ();

  char field_str[64]; // enough for 128 bits

  // first morph the font if required
  if (force_prop)
    {
      char *fname;

      if (name[0] != '-')
        {
          f = XLoadQueryFont (disp, name);

          if (!f)
            return false;

          char *new_name = get_property (f, XA_FONT, 0);

          if (new_name)
            set_name (new_name);
          else
            rxvt_warn ("font '%s' has no FONT property, continuing without.\n", name);

          XFreeFont (disp, f);
          f = 0;
        }

      if (prop.weight != rxvt_fontprop::unset)
        {
          replace_field (&fname, name, 2, 0,
                         prop.weight < rxvt_fontprop::bold
                           ? "medium" : "bold");
          set_name (fname);
        }

      if (prop.slant != rxvt_fontprop::unset)
        {
          replace_field (&fname, name, 3, 0,
                         prop.slant < rxvt_fontprop::italic
                           ? "r" : "i"); // TODO: handle "o"blique, too
          set_name (fname);
        }
    }

  sprintf (field_str, "%d", prop.height == rxvt_fontprop::unset
                              ? 0 : prop.height);

  struct font_weight {
    char *name;
    int diff;

    void clear ()
    {
      name = 0;
      diff = 0x7fffffff;
    }

    font_weight () { clear (); }
    ~font_weight () { free (name); }
  };

  char **list;
  int count;
  list = XListFonts (disp, name, 4000, &count);

  set_name (0);

  if (!list)
    return false;

  font_weight *fonts = new font_weight[count];

  for (int i = 0; i < count; i++)
    {
      rxvt_fontprop p;
      char *fname;

      int diff = 0;

      if (replace_field (&fname, list[i], 6, '0', field_str))
        diff += 10; // slightly penalize scalable fonts
      else
        {
          free (fname);
          if (replace_field (&fname, list[i], 11, '0', "0"))
            diff += 300; // more heavily penalize what looks like scaled bitmap fonts
        }

      if (!set_properties (p, fname)
          // also weed out too large fonts
          || (prop.height != rxvt_fontprop::unset
              && p.height > prop.height))
        {
          free (fname);
          continue;
        }

      if (prop.height != rxvt_fontprop::unset) diff += (prop.height - p.height) * 128;
      if (prop.weight != rxvt_fontprop::unset) diff += abs (prop.weight - p.weight);
      if (prop.slant  != rxvt_fontprop::unset) diff += abs (prop.slant  - p.slant);
      //if (prop.width  != rxvt_fontprop::unset) diff += abs (prop.width  - p.width);

      fonts[i].name = fname;
      fonts[i].diff = diff;
    }

  XFreeFontNames (list);

  // this loop only iterates when the guessed font-size is too small
  for (;;)
    {
      font_weight *best = fonts;

      for (font_weight *w = fonts + 1; w < fonts + count; w++)
        if (w->diff < best->diff)
          best = w;

      if (!best->name
          || !(f = XLoadQueryFont (disp, best->name)))
        break;

      set_name (best->name);
      best->clear ();

      ascent  = f->ascent;
      descent = f->descent;
      height  = ascent + descent;

      if (prop.height == rxvt_fontprop::unset
          || height <= prop.height)
        break; // font is ready for use

      // PIXEL_SIZE small enough, but real height too large
      clear ();
    }

  delete [] fonts;

  if (!f)
    return false;

  char *registry = get_property (f, term->xa [XA_CHARSET_REGISTRY], 0);
  char *encoding = get_property (f, term->xa [XA_CHARSET_ENCODING], 0);

  cs = CS_UNKNOWN;

  if (registry && encoding)
    {
      char charset[64];
      snprintf (charset, 64, "%s-%s", registry, encoding);

      cs = codeset_from_name (charset);

      if (cs == CS_UNKNOWN)
        rxvt_warn ("%s: cannot deduce encoding from registry/encoding properties \"%s\", ignoring font.\n", name, charset);
    }

  free (registry);
  free (encoding);

  if (cs == CS_UNKNOWN)
    {
      char *value = get_property (f, XA_FONT, 0);
      const char *charset = value;

      if (!charset)
        charset = name;

      int count = 13;
      while (*charset)
        if (*charset++ == '-' && !--count)
          break;

      cs = codeset_from_name (charset);
      if (cs == CS_UNKNOWN)
        rxvt_warn ("%s: cannot deduce encoding from font name property \"%s\", ignoring font.\n", name, charset);

      free (value);
    }

  if (cs == CS_UNKNOWN)
    {
      clear ();
      return false;
    }

  if (cs == CS_UNICODE)
    cs = CS_UNICODE_16; // X11 can have a max. of 65536 chars per font

  encm = f->min_byte1 != 0 || f->max_byte1 != 0;
  enc2b = encm || f->max_char_or_byte2 > 255;

  slow = false;

#if 1 // only used for slow detection, TODO optimize
  if (f->min_bounds.width == f->max_bounds.width || !f->per_char)
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
#endif

  width = 1;

  for (uint16_t *t = extent_test_chars; t < extent_test_chars + ecb_array_length (extent_test_chars); t++)
    {
      if (FROM_UNICODE (cs, *t) == NOCHAR)
        continue;

      // ignore characters we wouldn't use anyways
      bool careful;
      if (!has_char (*t, &prop, careful))
        continue;

      XChar2b ch = { *t >> 8, *t };

      XCharStruct g;
      int dir_ret, asc_ret, des_ret;
      XTextExtents16 (f, &ch, 1, &dir_ret, &asc_ret, &des_ret, &g);

      int wcw = WCWIDTH (*t);
      if (wcw > 0) g.width = (g.width + wcw - 1) / wcw;

      if (width < g.width) width = g.width;
    }

#if 0 // do it per-character
  if (prop && width > prop->width)
    {
      clear ();
      return false;
    }
#endif

  return true;
}

void
rxvt_font_x11::clear ()
{
  if (f)
    {
      XFreeFont (term->dpy, f);
      f = 0;
    }
}

bool
rxvt_font_x11::has_char (unicode_t unicode, const rxvt_fontprop *prop, bool &careful) const
{
  careful = false;

  uint32_t ch = FROM_UNICODE (cs, unicode);

  if (ch == NOCHAR)
    return false;

  /* check whether the character exists in _this_ font. horrible. */
  XCharStruct *xcs;

  if (encm)
    {
      unsigned char byte1 = ch >> 8;
      unsigned char byte2 = ch & 255;

      if (byte1 < f->min_byte1 || byte1 > f->max_byte1
          || byte2 < f->min_char_or_byte2 || byte2 > f->max_char_or_byte2)
        return false;

      if (f->per_char)
        {
          int D = f->max_char_or_byte2 - f->min_char_or_byte2 + 1;
          int N = (byte1 - f->min_byte1) * D + byte2 - f->min_char_or_byte2;

          xcs = f->per_char + N;
        }
      else
        xcs = &f->max_bounds;
    }
  else
    {
      if (ch < f->min_char_or_byte2 || ch > f->max_char_or_byte2)
        return false;

      if (f->per_char)
        xcs = f->per_char + (ch - f->min_char_or_byte2);
      else
        xcs = &f->max_bounds;
    }

  if (xcs->lbearing == 0 && xcs->rbearing == 0 && xcs->width == 0
      && xcs->ascent == 0 && xcs->descent == 0)
    return false;

  if (!prop || prop->width == rxvt_fontprop::unset)
    return true;

  // check whether character overlaps previous/next character
  int w = xcs->rbearing - xcs->lbearing;
  int wcw = max (WCWIDTH (unicode), 1);

  careful = xcs->lbearing < 0 || xcs->rbearing > prop->width * wcw;

  if (careful && !OVERLAP_OK (w, wcw, prop))
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

  dTermDisplay;
  dTermGC;

  bool slow = this->slow
              || width  != term->fwidth
              || height != term->fheight;

  int base = ascent; // sorry, incorrect: term->fbase;

  XGCValues v;
  v.foreground = term->pix_colors[fg];
  v.font = f->fid;

  if (enc2b)
    {
      const XChar2b *xc = enc_xchar2b (text, len, cs, slow);

      if (bg == Color_bg && !slow)
        {
          v.background = term->pix_colors[bg];
          XChangeGC (disp, gc, GCForeground | GCBackground | GCFont, &v);
          XDrawImageString16 (disp, d, gc, x, y + base, xc, len);
        }
      else
        {
          clear_rect (d, x, y, term->fwidth * len, term->fheight, bg);

          XChangeGC (disp, gc, GCForeground | GCFont, &v);

          if (slow)
            {
              do
                {
                  if (xc->byte1 || xc->byte2)
                    XDrawString16 (disp, d, gc, x, y + base, xc, 1);

                  x += term->fwidth;
                  xc++; len--;
                }
              while (len);
            }
          else
            XDrawString16 (disp, d, gc, x, y + base, xc, len);
        }
    }
  else
    {
      const char *xc = enc_char (text, len, cs, slow);

      if (bg == Color_bg && !slow)
        {
          v.background = term->pix_colors[bg];
          XChangeGC (disp, gc, GCForeground | GCBackground | GCFont, &v);
          XDrawImageString (disp, d, gc, x, y + base, xc, len);
        }
      else
        {
          clear_rect (d, x, y, term->fwidth * len, term->fheight, bg);

          XChangeGC (disp, gc, GCForeground | GCFont, &v);

          if (slow)
            {
              do
                {
                  if (*xc)
                    XDrawString (disp, d, gc, x, y + base, xc, 1);

                  x += term->fwidth;
                  xc++; len--;
                }
              while (len);
            }
          else
            XDrawString (disp, d, gc, x, y + base, xc, len);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

#if XFT

struct rxvt_font_xft : rxvt_font {
  rxvt_font_xft () { f = 0; }

  void clear ();

  rxvt_fontprop properties ();

  bool load (const rxvt_fontprop &prop, bool force_prop);

  void draw (rxvt_drawable &d, int x, int y,
             const text_t *text, int len,
             int fg, int bg);

  bool has_char (unicode_t unicode, const rxvt_fontprop *prop, bool &careful) const;

protected:
  XftFont *f;
};

void
rxvt_font_xft::clear ()
{
  if (f)
    {
      XftFontClose (term->dpy, f);
      f = 0;
    }
}

rxvt_fontprop
rxvt_font_xft::properties ()
{
  rxvt_fontprop p;

  FT_Face face = XftLockFace (f);

  p.width  = width;
  p.height = height;
  p.ascent = ascent;
  p.weight = face->style_flags & FT_STYLE_FLAG_BOLD
               ? rxvt_fontprop::bold : rxvt_fontprop::medium;
  p.slant  = face->style_flags & FT_STYLE_FLAG_ITALIC
               ? rxvt_fontprop::italic : rxvt_fontprop::roman;

  XftUnlockFace (f);

  return p;
}

bool
rxvt_font_xft::load (const rxvt_fontprop &prop, bool force_prop)
{
  dTermDisplay;

  clear ();

  FcPattern *p = FcNameParse ((FcChar8 *) name);

  if (!p)
    return false;

  FcValue v;

  if (prop.height != rxvt_fontprop::unset
      && (FcPatternGet (p, FC_PIXEL_SIZE, 0, &v) != FcResultMatch
          && FcPatternGet (p, FC_SIZE, 0, &v) != FcResultMatch))
    FcPatternAddInteger (p, FC_PIXEL_SIZE, prop.height);

  if (prop.weight != rxvt_fontprop::unset
      && (force_prop || FcPatternGet (p, FC_WEIGHT, 0, &v) != FcResultMatch))
    FcPatternAddInteger (p, FC_WEIGHT, prop.weight);

  if (prop.slant != rxvt_fontprop::unset
      && (force_prop || FcPatternGet (p, FC_SLANT, 0, &v) != FcResultMatch))
    FcPatternAddInteger (p, FC_SLANT, prop.slant);

#if 0 // clipping unfortunately destroys our precious double-width-characters
  // clip width, we can't do better, or can we?
  if (FcPatternGet (p, FC_CHAR_WIDTH, 0, &v) != FcResultMatch)
    FcPatternAddInteger (p, FC_CHAR_WIDTH, prop.width);
#endif

  if (FcPatternGet (p, FC_MINSPACE, 0, &v) != FcResultMatch)
    FcPatternAddBool (p, FC_MINSPACE, 1);

  // store generated name so iso14755 view gives better results
  set_name ((char *)FcNameUnparse (p));

  XftResult result;
  FcPattern *match = XftFontMatch (disp, term->display->screen, p, &result);

  FcPatternDestroy (p);

  if (!match)
    return false;

  int ftheight = 0;
  bool success = true;

  for (;;)
    {
      p = FcPatternDuplicate (match);
      f = XftFontOpenPattern (disp, p);

      if (!f)
        {
          FcPatternDestroy (p);
          success = false;
          break;
        }

      FT_Face face = XftLockFace (f);

      ascent  = (face->size->metrics.ascender + 63) >> 6;
      descent = (-face->size->metrics.descender + 63) >> 6;
      height  = max (ascent + descent, (face->size->metrics.height + 63) >> 6);
      width   = 0;

      bool scalable = face->face_flags & FT_FACE_FLAG_SCALABLE;

      XftUnlockFace (f);

      int glheight = height;

      for (uint16_t *t = extent_test_chars; t < extent_test_chars + ecb_array_length (extent_test_chars); t++)
        {
          FcChar16 ch = *t;

          if (cs != CS_UNICODE
              && ch > 0x100
              && FROM_UNICODE (cs, ch) == NOCHAR)
            continue;

          // ignore characters we wouldn't use anyways
          bool careful;
          if (!has_char (*t, &prop, careful))
            continue;

          XGlyphInfo g;
          XftTextExtents16 (disp, f, &ch, 1, &g);

          g.width -= g.x;

          int wcw = WCWIDTH (ch);
          if (wcw > 0) g.width = (g.width + wcw - 1) / wcw;

          if (width    < g.width       ) width    = g.width;
          if (height   < g.height      ) height   = g.height;
          if (glheight < g.height - g.y) glheight = g.height - g.y;
        }

      if (!width)
        {
          rxvt_warn ("unable to calculate font width for '%s', ignoring.\n", name);

          XftFontClose (disp, f);
          f = 0;

          success = false;
          break;
        }

      if (prop.height == rxvt_fontprop::unset
          || (height <= prop.height && glheight <= prop.height)
          || height <= 2
          || !scalable)
        break;

      if (ftheight)
        {
          // take smaller steps near the end
          if (height > prop.height + 1) ftheight++;
          if (height > prop.height + 2) ftheight++;
          if (height > prop.height + 3) ftheight++;

          ftheight -= height - prop.height;
        }
      else
        ftheight = prop.height - 1;

      XftFontClose (disp, f);
      FcPatternDel (match, FC_PIXEL_SIZE);
      FcPatternAddInteger (match, FC_PIXEL_SIZE, ftheight);
    }

  FcPatternDestroy (match);

#if 0 // do it per-character
  if (prop.width != rxvt_fontprop::unset && width > prop.width)
    {
      clear ();
      success = false;
    }
#endif

  return success;
}

bool
rxvt_font_xft::has_char (unicode_t unicode, const rxvt_fontprop *prop, bool &careful) const
{
  careful = false;

  if (!XftCharExists (term->dpy, f, unicode))
    return false;

  if (!prop || prop->width == rxvt_fontprop::unset)
    return true;

  // check character against base font bounding box
  FcChar32 ch = unicode;
  XGlyphInfo g;
  XftTextExtents32 (term->dpy, f, &ch, 1, &g);

  int w = g.width - g.x;
  int wcw = max (WCWIDTH (unicode), 1);

  careful = g.x > 0 || w > prop->width * wcw;

  if (careful && !OVERLAP_OK (w, wcw, prop))
    return false;

  // this weeds out _totally_ broken fonts, or glyphs
  if (!OVERLAP_OK (g.xOff, wcw, prop))
    return false;

  return true;
}

void
rxvt_font_xft::draw (rxvt_drawable &d, int x, int y,
                     const text_t *text, int len,
                     int fg, int bg)
{
  XGlyphInfo extents;
  XftGlyphSpec *enc = rxvt_temp_buf<XftGlyphSpec> (len);
  XftGlyphSpec *ep = enc;

  dTermDisplay;
  dTermGC;

  int w = term->fwidth * len;
  int h = term->fheight;

  bool buffered = bg >= Color_transparent
                  && term->option (Opt_buffered);

  // cut trailing spaces
  while (len && text [len - 1] == ' ')
    len--;

  int x_ = buffered ? 0 : x;
  int y_ = buffered ? 0 : y;

  while (len)
    {
      int cwidth = term->fwidth;
      FcChar32 fc = *text++; len--;

      while (len && *text == NOCHAR)
        text++, len--, cwidth += term->fwidth;

      if (fc != ' ') // skip spaces
        {
          FT_UInt glyph = XftCharIndex (disp, f, fc);
          XftGlyphExtents (disp, f, &glyph, 1, &extents);

          ep->glyph = glyph;
          ep->x = x_ + (cwidth - extents.xOff >> 1);
          ep->y = y_ + ascent;

          if (extents.xOff == 0)
            ep->x = x_ + cwidth;

          ep++;
        }

      x_ += cwidth;
    }

  if (buffered)
    {
      if (ep != enc)
        {
          rxvt_drawable &d2 = d.screen->scratch_drawable (w, h);

#ifdef HAVE_BG_PIXMAP
          Picture dst = 0; // the only assignment is done conditionally in the following if condition

          if (term->bg_pixmap
              && (bg == Color_transparent || bg == Color_bg
                  || (bg >= 0 && !term->pix_colors[bg].is_opaque () && ((dst = XftDrawPicture (d2))))))
            {
              int src_x = x, src_y = y;

              if (term->bg_flags & rxvt_term::BG_IS_TRANSPARENT)
                {
                  src_x += term->window_vt_x;
                  src_y += term->window_vt_y;
                }

              if (term->bg_pmap_width >= src_x + w
                  && term->bg_pmap_height >= src_y + h)
                {
                  XCopyArea (disp, term->bg_pixmap, d2, gc,
                             src_x, src_y, w, h, 0, 0);
                }
              else
                {
                  XGCValues gcv;

                  gcv.fill_style  = FillTiled;
                  gcv.tile        = term->bg_pixmap;
                  gcv.ts_x_origin = -src_x;
                  gcv.ts_y_origin = -src_y;

                  XChangeGC (disp, gc,
                             GCTile | GCTileStipXOrigin | GCTileStipYOrigin | GCFillStyle,
                             &gcv);

                  XFillRectangle (disp, d2, gc, 0, 0, w, h);

                  gcv.fill_style = FillSolid;
                  XChangeGC (disp, gc, GCFillStyle, &gcv);
                }

              if (dst)
                {
                  Picture solid_color_pict = XftDrawSrcPicture (d2, &term->pix_colors[bg].c);

                  // dst can only be set when bg >= 0
                  XRenderComposite (disp, PictOpOver, solid_color_pict, None, dst, 0, 0, 0, 0, 0, 0, w, h);
                }
            }
          else
#endif
            XftDrawRect (d2, &term->pix_colors[bg >= 0 ? bg : Color_bg].c, 0, 0, w, h);

          XftDrawGlyphSpec (d2, &term->pix_colors[fg].c, f, enc, ep - enc);
          XCopyArea (disp, d2, d, gc, 0, 0, w, h, x, y);
        }
      else
        clear_rect (d, x, y, w, h, bg);
    }
  else
    {
      clear_rect (d, x, y, w, h, bg);
      XftDrawGlyphSpec (d, &term->pix_colors[fg].c, f, enc, ep - enc);
    }
}

#endif

/////////////////////////////////////////////////////////////////////////////

rxvt_fontset::rxvt_fontset (rxvt_term *term)
: fontdesc (0), term (term)
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
  prop.width = prop.height = prop.ascent = prop.weight = prop.slant
    = rxvt_fontprop::unset;
  force_prop = false;

  for (rxvt_font **i = fonts.begin (); i != fonts.end (); i++)
    (*i)->unref ();

  for (pagemap **p = fmap.begin (); p != fmap.end (); p++)
    delete *p;

  free (fontdesc); fontdesc = 0;

  fonts.clear ();

  fallback = fallback_fonts;
}

void
rxvt_fontset::prepare_font (rxvt_font *font, codeset cs)
{
  font->set_term (term);

  font->cs = cs;
  font->loaded = false;
}

rxvt_font *
rxvt_fontset::new_font (const char *name, codeset cs)
{
  rxvt_font *f;

  if (!name || !*name)
    {
      name = "";
      f = new rxvt_font_default (this);
    }
#if XFT
  else if (!strncmp (name, "xft:", 4))
    {
      name += 4;
      f = new rxvt_font_xft ();
    }
#endif
  else if (!strncmp (name, "x:", 2))
    {
      name += 2;
      f = new rxvt_font_x11;
    }
  else
    f = new rxvt_font_x11;

  f->set_name (strdup (name));
  prepare_font (f, cs);

  return f;
}

/////////////////////////////////////////////////////////////////////////////

void
rxvt_fontset::push_font (rxvt_font *font)
{
  // the fontCount index is reserved for the overflow font, it is only
  // necessary when we get fontCount or more fonts, as they cannot be
  // represented in the rendition.
  if (fonts.size () == fontCount)
    {
      rxvt_font *f = new rxvt_font_overflow (this);

      prepare_font (f, CS_UNICODE);
      fonts.push_back (f);
    }

  fonts.push_back (font);
}

void
rxvt_fontset::add_fonts (const char *desc)
{
  if (desc)
    {
      char buf[512];
      const char *end;

      do
        {
          while (*desc && *desc <= ' ')
            desc++;

          codeset cs = CS_UNICODE;

          if (*desc == '[')
            {
              char spec[256];
              const char *extra = ++desc; // not yet used

              desc = strchr (desc, ']');

              if (!desc)
                {
                  rxvt_warn ("ERROR: opening '[' without closing ']' in font specification, trying to continue.\n");
                  break;
                }

              memcpy (spec, extra, min (desc - extra, 255));
              spec[min (desc - extra, 255)] = 0;

              if (!strncmp (extra, "codeset=", sizeof ("codeset=") - 1))
                cs = codeset_from_name (spec + sizeof ("codeset=") - 1);
              else
                rxvt_warn ("unknown parameter '%s' in font specification, skipping.\n", spec);

              desc++;
              while (*desc <= ' ') desc++;
            }

          end = strchr (desc, ',');
          if (!end)
            end = desc + strlen (desc);

          if (end - desc < 511)
            {
              memcpy (buf, desc, end - desc);
              buf[end - desc] = 0;

              push_font (new_font (buf, cs));
            }
          else
            rxvt_warn ("fontset element too long (>511 bytes), ignored.\n");

          desc = end + 1;
        }
      while (*end);
    }
}

bool
rxvt_fontset::realize_font (int i)
{
  if (i < 0 || i >= fonts.size ())
    return false;

  if (fonts[i]->loaded)
    return true;

  fonts[i]->loaded = true;

  if (!fonts[i]->load (prop, force_prop))
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

  fontdesc = strdup (desc);

  push_font (new_font (0, CS_UNICODE));
  realize_font (0);

  add_fonts (desc);

  return true;
}

int
rxvt_fontset::find_font (const char *name) const
{
  for (rxvt_font *const *f = fonts.begin (); f < fonts.end (); f++)
    if ((*f)->name && !strcmp ((*f)->name, name))
      return f - fonts.begin ();

  return -1;
}

int
rxvt_fontset::find_font_idx (unicode_t unicode)
{
  if (unicode >= 1<<20)
    return 0;

  unicode_t hi = unicode >> 8;

  if (hi < fmap.size ()
      && fmap[hi]
      && (*fmap[hi])[unicode & 0xff] != 0xff)
    return (*fmap[hi])[unicode & 0xff];

  unsigned int i;

  for (i = 0; i < fonts.size (); i++)
    {
      rxvt_font *f = fonts[i];

      if (!f->loaded)
        {
          if (FROM_UNICODE (f->cs, unicode) == NOCHAR)
            goto next_font;

          if (!realize_font (i))
            goto next_font;

          if (prop.ascent != rxvt_fontprop::unset)
            max_it (f->ascent, prop.ascent);
        }

      if (f->cs == CS_UNKNOWN)
        goto next_font;

      bool careful;
      if (f->has_char (unicode, &prop, careful))
        {
          i = (i << 1) | careful;

          goto found;
        }

    next_font:
      if (i == fonts.size () - 1)
        {
          if (fallback->name)
            {
              // search through the fallback list
              push_font (new_font (fallback->name, fallback->cs));
              fallback++;
            }
          else
            {
              // try to find a new font.
              // only xft currently supported, as there is no
              // way to configure this and xft is easier to hack in,
              // while x11 has more framework in place already.
              // TODO: this is a real resource hog, xft takes ages(?)
#if XFT && USE_SLOW_LOOKUP
              // grab the first xft font that seems suitable
              FcPattern *p = FcPatternCreate ();

              FcCharSet *s = FcCharSetCreate ();
              FcCharSetAddChar (s, unicode);
              FcPatternAddCharSet (p, FC_CHARSET, s);
              // charsets don't help that much, as xft might return
              // a non-matching font even if a better font is available :/

              x x x x TODO prop might have unset contents
              FcPatternAddInteger (p, FC_PIXEL_SIZE, prop.height);
              FcPatternAddInteger (p, FC_WEIGHT, prop.weight);
              FcPatternAddInteger (p, FC_SLANT, prop.slant);
              FcPatternAddBool    (p, FC_MINSPACE, 1);
              //FcPatternAddBool    (p, FC_ANTIALIAS, 1);

              XftResult result;
              FcPattern *match = XftFontMatch (term->dpy, term->display->screen, p, &result);

              FcPatternDestroy (p);

              if (match)
                {
                  FcPatternDel (match, FC_CHARSET);
                  char *font = (char *)FcNameUnparse (match);
                  FcPatternDestroy (match);

                  if (find_font (font) < 0)
                    {
                      char fontname[4096];
                      snprintf (fontname, sizeof (fontname), "xft:%s", font);

                      push_font (new_font (fontname, CS_UNICODE));
                    }

                  free (font);
                }
#endif
            }
        }
    }

  /* we must return SOME font */
  i = 0;

found:
  // found a font, cache it
  if (i < 255)
    {
      while (hi >= fmap.size ())
        fmap.push_back (0);

      if (!fmap[hi])
        {
          fmap[hi] = (pagemap *)new pagemap;
          memset (fmap[hi], 0xff, sizeof (pagemap));
        }

      (*fmap[hi])[unicode & 0xff] = i;
    }

  return i;
}

