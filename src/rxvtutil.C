#include <cstdlib>
#include <cstring>
#include <inttypes.h>

#include "rxvtutil.h"

class byteorder byteorder;

unsigned int byteorder::e;

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

void *
zero_initialized::operator new (size_t s)
{
  void *p = malloc (s);

  memset (p, 0, s);
  return p;
}

void
zero_initialized::operator delete (void *p, size_t s)
{
  free (p);
}



