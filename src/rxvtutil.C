#include <cstdlib>
#include <cstring>
#include <inttypes.h>

#include "rxvtutil.h"

class byteorder byteorder;

byteorder::byteorder ()
{
  union {
    uint32_t u;
    uint8_t b[4];
  } w;

  w.b[0] = 0x11;
  w.b[1] = 0x22;
  w.b[2] = 0x33;
  w.b[3] = 0x44;

  e = w.u;
}



