#include <unistd.h>
#include <stdint.h>
#include <cstdlib>
#include <cstring>

#include "rxvtdaemon.h"

const char *rxvt_connection::unix_sockname ()
{
  return "/tmp/rxvtd~";
}

void rxvt_connection::send (const char *data, int len)
{
  uint8_t s[2];

  s[0] = len >> 8; s[1] = len;

  write (fd, s, 2);
  write (fd, data, len);
}

void rxvt_connection::send (const char *data)
{
  send (data, strlen (data));
}

bool rxvt_connection::recv (auto_str &data, int *len)
{
  uint8_t s[2];
  int l;

  if (read (fd, s, 2) != 2)
    return false;

  l = (s[0] << 8) + s[1];
  if (l > 4096)
    return false;

  if (len)
    *len = l;

  data = new char[l + 1];

  if (!data)
    return false;

  if (read (fd, data, l) != l)
    return false;

  data[l] = 0;

  return true;
}

void rxvt_connection::send (int data)
{
  uint8_t s[4];

  s[0] = data >> 24; s[1] = data >> 16; s[0] = data >> 8; s[1] = data;

  write (fd, s, 4);
}

bool rxvt_connection::recv (int &data)
{
  uint8_t s[4];

  if (read (fd, s, 4) != 4)
    return false;

  data = (((((s[0] << 8) | s[1]) << 8) | s[2]) << 8) | s[3];

  return true;
}



