#include "../config.h"

#include "encoding.h"

#include <cstdlib>
#include <cstring>

const struct n2cs {
  const char *name;
  codeset cs;
} n2cs[] = {
  /* first one found is the normalized one */
  { "ISO88591",		CS_ISO8859_1	},
  { "ISO8859PRIMARY",	CS_ISO8859_1	}, // some stupid fonts use this (hi tigert)
  { "ISO88592",		CS_ISO8859_2	},
  { "ISO88593",		CS_ISO8859_3	},
  { "ISO88594",		CS_ISO8859_4	},
  { "ISO88595",		CS_ISO8859_5	},
  { "ISO88596",		CS_ISO8859_6	},
  { "ISO88597",		CS_ISO8859_7	},
  { "ISO88598",		CS_ISO8859_8	},
  { "ISO88599",		CS_ISO8859_9	},
  { "ISO885910",	CS_ISO8859_10	},
  { "ISO885911",	CS_ISO8859_11	},
  { "ISO885913",	CS_ISO8859_13	},
  { "ISO885914",	CS_ISO8859_14	},
  { "ISO885915",	CS_ISO8859_15	},
  { "FCD885915",	CS_ISO8859_15	},
  { "ISO885916",	CS_ISO8859_16	},

  { "ISO106461",	CS_UNICODE	},
  { "UNICODE",		CS_UNICODE	},
  { "UTF8",		CS_UNICODE	},

  { "ASCII",		CS_US_ASCII	},
  { "USASCII",		CS_US_ASCII	},
  { "ANSIX341968",	CS_US_ASCII	},

  { "KOI8R",		CS_KOI8_R	},
  { "GOST19768741",     CS_KOI8_R       },
  { "KOI8U",		CS_KOI8_U	},

  { "KSC560119870",	CS_KSC5601_1987_0 },
  { "KSX100119970",	CS_KSC5601_1987_0 },
  { "KSX100119980",	CS_KSC5601_1987_0 }, // adds johab

  { "GB231219800",	CS_GB2312_1980_0 },

  { "VISCII",		CS_VISCII	},
  { "VISCII111",	CS_VISCII	},
  { "TIS62025291",	CS_VISCII	}, /* close enough */

  { "JISX020119760",	CS_JIS0201_1976_0 },
  { "JISX020819830",	CS_JIS0208_1983_0 },
  { "JISX020819900",	CS_JIS0208_1983_0 }, /* ehrm. */
  { "JISX021219900",	CS_JIS0212_1990_0 },

  { 0,       		CS_UNKNOWN      }
};

static const char *
normalize_name (const char *name)
{
  static char res[16];
  char *r;

  for (r = res; *name && r < res + 15; name++)
    if ((*name >= '0' && *name <= '9')
        || (*name >= 'A' && *name <= 'Z'))
      *r++ = *name;
    else if (*name >= 'a' && *name <= 'z')
      *r++ = *name - ('a' - 'A');

  *r = 0;

  return res;
}

codeset
codeset_from_name (const char *name)
{
  if (!name)
    return CS_UNKNOWN;

  name = normalize_name (name);

  const struct n2cs *i = n2cs;

  do {
    if (!strcmp (name, i->name))
      return i->cs;
  } while ((++i)->name);

  return CS_UNKNOWN;
}

struct rxvt_codeset_conv_unknown : rxvt_codeset_conv {
  uint32_t to_unicode (uint32_t enc) const { return NOCHAR; }
  uint32_t from_unicode (uint32_t unicode) const { return NOCHAR; }
} rxvt_codeset_conv_unknown;

struct rxvt_codeset_conv_us_ascii : rxvt_codeset_conv {
  uint32_t from_unicode (uint32_t unicode) const { return unicode <= 127 ? unicode : NOCHAR; }
} rxvt_codeset_conv_us_ascii;

struct rxvt_codeset_conv_unicode : rxvt_codeset_conv {
  /* transparent */
} rxvt_codeset_conv_unicode;

struct rxvt_codeset_conv_unicode_16 : rxvt_codeset_conv {
  uint32_t to_unicode (uint32_t enc) const { return enc; }
  uint32_t from_unicode (uint32_t unicode) const { return unicode <= 65535 ? unicode : NOCHAR; }
} rxvt_codeset_conv_unicode_16;

/* block character set, must conform to the dec special pseudofont in defaultfont.C */
struct rxvt_codeset_conv_special : rxvt_codeset_conv {
  uint32_t to_unicode (uint32_t enc) const {
    return enc;
  }

  uint32_t from_unicode (uint32_t unicode) const {
    return unicode;
  }
} rxvt_codeset_conv_special;

#define ENCODING_DEFAULT

#include "table/iso8859_1.h"
#include "table/iso8859_15.h"

//#define ENCODING_EU

#include "table/iso8859_2.h"
#include "table/iso8859_3.h"
#include "table/iso8859_4.h"
#include "table/iso8859_5.h"
#include "table/iso8859_6.h"
#include "table/iso8859_7.h"
#include "table/iso8859_8.h"
#include "table/iso8859_9.h"
#include "table/iso8859_10.h"
#include "table/iso8859_11.h"
#include "table/iso8859_13.h"
#include "table/iso8859_14.h"
#include "table/iso8859_16.h"

#include "table/koi8_r.h"
#include "table/koi8_u.h"

//#define ENCODING_KR

#include "table/ksc5601_1987_0.h"

//#define ENCODING_CN

#include "table/gb2312_1980_0.h"

//#define ENCODING_CN_EXT

#include "table/cns11643_1992_1.h"
#include "table/cns11643_1992_2.h"
#include "table/cns11643_1992_3.h"
#include "table/cns11643_1992_4.h"
#include "table/cns11643_1992_5.h"
#include "table/cns11643_1992_6.h"
#include "table/cns11643_1992_7.h"
#include "table/cns11643_1992_f.h"
#include "table/big5_ext.h"
#include "table/big5_plus.h"

//#define ENCODING_VN

#include "table/viscii.h"

//#define ENCODING_JP

#include "table/jis0201_1976_0.h"
#include "table/jis0208_1983_0.h"
#include "table/jis0212_1990_0.h"

//#define ENCODING_JP_EXT

#include "table/jis0213_1.h"
#include "table/jis0213_2.h"

const rxvt_codeset_conv *rxvt_codeset[NUM_CODESETS] = {
  &rxvt_codeset_conv_unknown,
  &rxvt_codeset_conv_special,

  &rxvt_codeset_conv_us_ascii,

  &rxvt_codeset_conv_iso8859_1,
  &rxvt_codeset_conv_iso8859_2,
  &rxvt_codeset_conv_iso8859_3,
  &rxvt_codeset_conv_iso8859_4,
  &rxvt_codeset_conv_iso8859_5,
  &rxvt_codeset_conv_iso8859_6,
  &rxvt_codeset_conv_iso8859_7,
  &rxvt_codeset_conv_iso8859_8,
  &rxvt_codeset_conv_iso8859_9,
  &rxvt_codeset_conv_iso8859_10,
  &rxvt_codeset_conv_iso8859_11,
  &rxvt_codeset_conv_iso8859_13,
  &rxvt_codeset_conv_iso8859_14,
  &rxvt_codeset_conv_iso8859_15,
  &rxvt_codeset_conv_iso8859_16,

  &rxvt_codeset_conv_koi8_r,
  &rxvt_codeset_conv_koi8_u,

  &rxvt_codeset_conv_jis0201_1976_0,
  &rxvt_codeset_conv_jis0208_1983_0,
  &rxvt_codeset_conv_jis0212_1990_0,

  &rxvt_codeset_conv_jis0213_1,
  &rxvt_codeset_conv_jis0213_2,

  &rxvt_codeset_conv_ksc5601_1987_0,

  &rxvt_codeset_conv_gb2312_1980_0,

  &rxvt_codeset_conv_cns11643_1992_1,
  &rxvt_codeset_conv_cns11643_1992_2,
  &rxvt_codeset_conv_cns11643_1992_3,
  &rxvt_codeset_conv_cns11643_1992_4,
  &rxvt_codeset_conv_cns11643_1992_5,
  &rxvt_codeset_conv_cns11643_1992_6,
  &rxvt_codeset_conv_cns11643_1992_7,
  &rxvt_codeset_conv_cns11643_1992_f,
  &rxvt_codeset_conv_big5_ext,
  &rxvt_codeset_conv_big5_plus,

  &rxvt_codeset_conv_viscii,

  &rxvt_codeset_conv_unicode_16,
  &rxvt_codeset_conv_unicode
};

