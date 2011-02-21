/*----------------------------------------------------------------------*
 * File:	misc.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1996      mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2000 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2003-2006 Marc Lehmann <schmorp@schmorp.de>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */

char *
rxvt_wcstombs (const wchar_t *str, int len)
{
  if (len < 0) len = wcslen (str);

  mbstate mbs;
  char *r = (char *)rxvt_malloc (len * MB_CUR_MAX + 1);

  char *dst = r;
  while (len--)
    {
      ssize_t l = wcrtomb (dst, *str++, mbs);

      if (l < 0)
        {
          *dst++ = '?';
          wcrtomb (0, 0, mbs); // reset undefined state
        }
      else
        dst += l;
    }

  *dst++ = 0;

  return (char *)rxvt_realloc (r, dst - r);
}

wchar_t *
rxvt_mbstowcs (const char *str, int len)
{
  if (len < 0) len = strlen (str);

  wchar_t *r = (wchar_t *)rxvt_malloc ((len + 1) * sizeof (wchar_t));

  if ((ssize_t)mbstowcs (r, str, len + 1) < 0)
    *r = 0;

  return r;
}

char *
rxvt_wcstoutf8 (const wchar_t *str, int len)
{
  if (len < 0) len = wcslen (str);

  char *r = (char *)rxvt_malloc (len * 4 + 1);
  char *p = r;

  while (len--)
    {
      unicode_t w = *str++ & UNICODE_MASK;

      if      (w < 0x000080)
        *p++ = w;
      else if (w < 0x000800)
        *p++ = 0xc0 | ( w >>  6),
        *p++ = 0x80 | ( w        & 0x3f);
      else if (w < 0x010000)
        *p++ = 0xe0 | ( w >> 12),
        *p++ = 0x80 | ((w >>  6) & 0x3f),
        *p++ = 0x80 | ( w        & 0x3f);
      else if (w < 0x110000)
        *p++ = 0xf0 | ( w >> 18),
        *p++ = 0x80 | ((w >> 12) & 0x3f),
        *p++ = 0x80 | ((w >>  6) & 0x3f),
        *p++ = 0x80 | ( w        & 0x3f);
      else
        *p++ = '?';
    }

  *p++ = 0;

  return (char *)rxvt_realloc (r, p - r);
}

wchar_t *
rxvt_utf8towcs (const char *str, int len)
{
  if (len < 0) len = strlen (str);

  wchar_t *r = (wchar_t *)rxvt_malloc ((len + 1) * sizeof (wchar_t)),
          *p = r;

  unsigned char *s = (unsigned char *)str,
                *e = s + len;

  for (;;)
    {
      len = e - s;

      if (len == 0)
        break;
      else if (s[0] < 0x80)
        *p++ = *s++;
      else if (len >= 2
               && s[0] >= 0xc2 && s[0] <= 0xdf
               && (s[1] & 0xc0) == 0x80)
        {
          *p++ = ((s[0] & 0x1f) << 6)
               |  (s[1] & 0x3f);
          s += 2;
        }
      else if (len >= 3
               && (   (s[0] == 0xe0                 && s[1] >= 0xa0 && s[1] <= 0xbf)
                   || (s[0] >= 0xe1 && s[0] <= 0xec && s[1] >= 0x80 && s[1] <= 0xbf)
                   || (s[0] == 0xed                 && s[1] >= 0x80 && s[1] <= 0x9f)
                   || (s[0] >= 0xee && s[0] <= 0xef && s[1] >= 0x80 && s[1] <= 0xbf)
                  )
               && (s[2] & 0xc0) == 0x80)
        {
          *p++ = ((s[0] & 0x0f) << 12)
               | ((s[1] & 0x3f) <<  6)
               |  (s[2] & 0x3f);
          s += 3;
        }
      else if (len >= 4
               && (   (s[0] == 0xf0                 && s[1] >= 0x90 && s[1] <= 0xbf)
                   || (s[0] >= 0xf1 && s[0] <= 0xf3 && s[1] >= 0x80 && s[1] <= 0xbf)
                   || (s[0] == 0xf4                 && s[1] >= 0x80 && s[1] <= 0x8f)
                  )
               && (s[2] & 0xc0) == 0x80
               && (s[3] & 0xc0) == 0x80)
        {
          *p++ = ((s[0] & 0x07) << 18)
               | ((s[1] & 0x3f) << 12)
               | ((s[2] & 0x3f) <<  6)
               |  (s[3] & 0x3f);
          s += 4;
        }
      else
        {
          *p++ = 0xfffd;
          s++;
        }
    }

  *p = 0;

  return r;
}

const char *
rxvt_basename (const char *str) NOTHROW
{
  const char *base = strrchr (str, '/');

  return base ? base + 1 : str;
}

/*
 * Print an error message
 */
void
rxvt_vlog (const char *fmt, va_list arg_ptr) NOTHROW
{
  char msg[1024];

  vsnprintf (msg, sizeof msg, fmt, arg_ptr);

  if (GET_R && GET_R->log_hook)
    (*GET_R->log_hook) (msg);
  else
    write (STDOUT_FILENO, msg, strlen (msg));
}

void
rxvt_log (const char *fmt,...) NOTHROW
{
  va_list arg_ptr;

  va_start (arg_ptr, fmt);
  rxvt_vlog (fmt, arg_ptr);
  va_end (arg_ptr);
}

/*
 * Print an error message
 */
void
rxvt_warn (const char *fmt,...) NOTHROW
{
  va_list arg_ptr;

  rxvt_log ("%s: ", RESNAME);

  va_start (arg_ptr, fmt);
  rxvt_vlog (fmt, arg_ptr);
  va_end (arg_ptr);
}

void
rxvt_fatal (const char *fmt,...) THROW ((class rxvt_failure_exception))
{
  va_list arg_ptr;

  rxvt_log ("%s: ", RESNAME);

  va_start (arg_ptr, fmt);
  rxvt_vlog (fmt, arg_ptr);
  va_end (arg_ptr);

  rxvt_exit_failure ();
}

void
rxvt_exit_failure () THROW ((class rxvt_failure_exception))
{
  static class rxvt_failure_exception rxvt_failure_exception;
  throw (rxvt_failure_exception);
}

/*
 * remove leading/trailing space in place.
 */
char *
rxvt_strtrim (char *str) NOTHROW
{
  char *r, *s;

  if (!str || !*str)		/* shortcut */
    return str;

  /* skip leading spaces */
  for (s = str; *s && isspace (*s); s++) ;

  /* goto end of string */
  r = s + strlen (s) - 1;

  /* dump return and other trailing whitespace */
  while (r > s && isspace (*r))
    r--;

  memmove (str, s, r + 1 - s);
  str[r + 1 - s] = 0;

  return str;
}

/*
 * Split a string into an array based on the given delimiter, stripping leading and
 * trailing spaces from each entry.  Empty strings are properly returned
 */
char **
rxvt_strsplit (char delim, const char *str) NOTHROW
{
  int l, n;
  char *s, *t;
  char **ret;

  s = strdup (str ? str : "");

  for (n = 1, t = s; *t; t++)
    if (*t == delim)
      n++;

  ret = (char **)malloc ((n + 1) * sizeof (char *));
  ret[n] = NULL;

  for (l = 0, t = s; l < n; l++)
    {
      for (; *t && *t != delim; t++)
        ;
      *t = '\0';
      ret[l] = s;
      rxvt_strtrim (ret[l]);
      s = ++t;
    }

  return ret;
}

void *
rxvt_malloc (size_t size)
{
  void *p = malloc (size);

  if (!p)
    rxvt_fatal ("memory allocation failure. aborting.\n");

  return p;
}

void *
rxvt_calloc (size_t number, size_t size)
{
  void *p = calloc (number, size);

  if (!p)
    rxvt_fatal ("memory allocation failure. aborting.\n");

  return p;
}

void *
rxvt_realloc (void *ptr, size_t size)
{
  void *p = realloc (ptr, size);

  if (!p)
    rxvt_fatal ("memory allocation failure. aborting.\n");

  return p;
}
