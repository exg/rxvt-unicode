#ifndef ENCODING_H
#define ENCODING_H

#include <stdint.h>

enum codeset {
  CS_UNKNOWN = 0,
  CS_SPECIAL,
  CS_US_ASCII,

  CS_ISO8859_1,
  CS_ISO8859_2,
  CS_ISO8859_3,
  CS_ISO8859_4,
  CS_ISO8859_5,
  CS_ISO8859_6,
  CS_ISO8859_7,
  CS_ISO8859_8,
  CS_ISO8859_9,
  CS_ISO8859_10,
  CS_ISO8859_11,
  CS_ISO8859_13,
  CS_ISO8859_14,
  CS_ISO8859_15,
  CS_ISO8859_16,

  CS_KOI8_R,
  CS_KOI8_U,
  CS_JIS0201_1976_0,
  CS_JIS0208_1983_0,
  CS_JIS0212_1990_0,

  CS_JIS0213_1,
  CS_JIS0213_2,

  CS_KSC5601_1987_0,

  CS_GB2312_1980_0,

  CS_CNS11643_1992_1,
  CS_CNS11643_1992_2,
  CS_CNS11643_1992_3,
  CS_CNS11643_1992_4,
  CS_CNS11643_1992_5,
  CS_CNS11643_1992_6,
  CS_CNS11643_1992_7,
  CS_CNS11643_1992_F,

  CS_BIG5_EXT,
  CS_BIG5_PLUS,

  CS_VISCII,

  CS_UNICODE_16, /* 16-bit subset of unicode, for X11 */
  CS_UNICODE,

  NUM_CODESETS
};

codeset codeset_from_name (const char *name);

enum {
  ZERO_WIDTH_CHAR = 0x200b,
  NOCHAR = 65535, // must be invalid in ANY codeset (!)
};

struct rxvt_codeset_conv {
  virtual uint32_t from_unicode (uint32_t unicode) const { return unicode; }
  virtual uint32_t to_unicode (uint32_t enc) const { return enc; }
};

extern const rxvt_codeset_conv *rxvt_codeset[NUM_CODESETS];

#define FROM_UNICODE(cs,code) rxvt_codeset[cs]->from_unicode (code)
#define TO_UNICODE(cs,code)   rxvt_codeset[cs]->to_unicode   (code)

#endif

