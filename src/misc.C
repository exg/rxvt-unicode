/*--------------------------------*-C-*---------------------------------*
 * File:	misc.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1996      mj olesen <olesen@me.QueensU.CA> Queen's Univ at Kingston
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2000 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2003-2004 Marc Lehmann <pcg@goof.com>
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
        *dst++ = '?';
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
        *p++ = 0xe0 | ( w >> 12       ),
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

char *
rxvt_strdup (const char *str)
{
  return str ? strdup (str) : 0;
}

char *
rxvt_r_basename (const char *str)
{
  char *base = strrchr (str, '/');

  return (char *) (base ? base + 1 : str);
}

/*
 * Print an error message
 */
void
rxvt_vlog (const char *fmt, va_list arg_ptr)
{
  char msg[1024];

  vsnprintf (msg, sizeof msg, fmt, arg_ptr);

  if (GET_R && GET_R->log_hook)
    (*GET_R->log_hook) (msg);
  else
    write (STDOUT_FILENO, msg, strlen (msg));
}

void
rxvt_log (const char *fmt,...)
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
rxvt_warn (const char *fmt,...)
{
  va_list arg_ptr;

  rxvt_log ("%s: ", RESNAME);

  va_start (arg_ptr, fmt);
  rxvt_vlog (fmt, arg_ptr);
  va_end (arg_ptr);
}

void
rxvt_fatal (const char *fmt,...)
{
  va_list arg_ptr;

  rxvt_log ("%s: ", RESNAME);

  va_start (arg_ptr, fmt);
  rxvt_vlog (fmt, arg_ptr);
  va_end (arg_ptr);

  rxvt_exit_failure ();
}

class rxvt_failure_exception rxvt_failure_exception;

void
rxvt_exit_failure ()
{
  throw (rxvt_failure_exception);
}

/*
 * check that the first characters of S1 match S2
 *
 * No Match
 *      return: 0
 * Match
 *      return: strlen (S2)
 */
int
rxvt_Str_match (const char *s1, const char *s2)
{
  int n = strlen (s2);

  return ((strncmp (s1, s2, n) == 0) ? n : 0);
}

const char *
rxvt_Str_skip_space (const char *str)
{
  if (str)
    while (*str && isspace (*str))
      str++;

  return str;
}

/*
 * remove leading/trailing space and strip-off leading/trailing quotes.
 * in place.
 */
char           *
rxvt_Str_trim (char *str)
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

#if 0
  /* skip matching leading/trailing quotes */
  if (*s == '"' && *r == '"' && n > 1)
    {
      s++;
      n -= 2;
    }
#endif

  memmove (str, s, r + 1 - s);
  str[r + 1 - s] = 0;

  return str;
}

/*
 * in-place interpretation of string:
 *
 *      backslash-escaped:      "\a\b\E\e\n\r\t", "\octal"
 *      Ctrl chars:     ^@ .. ^_, ^?
 *
 *      Emacs-style:    "M-" prefix
 *
 * Also,
 *      "M-x" prefixed strings, append "\r" if needed
 *      "\E]" prefixed strings (XTerm escape sequence) append ST if needed
 *
 * returns the converted string length
 */
int
rxvt_Str_escaped (char *str)
{
  char            ch, *s, *d;
  int             i, num, append = 0;

  if (!str || !*str)
    return 0;

  d = s = str;

  if (*s == 'M' && s[1] == '-')
    {
      /* Emacs convenience, replace leading `M-..' with `\E..' */
      *d++ = C0_ESC;
      s += 2;
      if (toupper (*s) == 'X')
        /* append carriage-return for `M-xcommand' */
        for (*d++ = 'x', append = '\r', s++; isspace (*s); s++) ;
    }
  for (; (ch = *s++);)
    {
      if (ch == '\\')
        {
          ch = *s++;
          if (ch >= '0' && ch <= '7')
            {	/* octal */
              num = ch - '0';
              for (i = 0; i < 2; i++, s++)
                {
                  ch = *s;
                  if (ch < '0' || ch > '7')
                    break;
                  num = num * 8 + ch - '0';
                }
              ch = (char)num;
            }
          else if (ch == 'a')
            ch = C0_BEL;	/* bell */
          else if (ch == 'b')
            ch = C0_BS;	/* backspace */
          else if (ch == 'E' || ch == 'e')
            ch = C0_ESC;	/* escape */
          else if (ch == 'n')
            ch = '\n';	/* newline */
          else if (ch == 'r')
            ch = '\r';	/* carriage-return */
          else if (ch == 't')
            ch = C0_HT;	/* tab */
        }
      else if (ch == '^')
        {
          ch = *s++;
          ch = toupper (ch);
          ch = (ch == '?' ? 127 : (ch - '@'));
        }
      *d++ = ch;
    }

  /* ESC] is an XTerm escape sequence, must be terminated */
  if (*str == '\0' && str[1] == C0_ESC && str[2] == ']')
    append = CHAR_ST;

  /* add trailing character as required */
  if (append && d[-1] != append)
    *d++ = append;
  *d = '\0';

  return (d - str);
}

/*
 * Split a comma-separated string into an array, stripping leading and
 * trailing spaces from each entry.  Empty strings are properly returned
 * Caller should free each entry and array when done
 */
char          **
rxvt_splitcommastring (const char *cs)
{
  int             l, n, p;
  const char     *s, *t;
  char          **ret;

  if ((s = cs) == NULL)
    s = "";

  for (n = 1, t = s; *t; t++)
    if (*t == ',')
      n++;

  ret = (char **)malloc ((n + 1) * sizeof (char *));
  ret[n] = NULL;

  for (l = 0, t = s; l < n; l++)
    {
      for ( ; *t && *t != ','; t++) ;
      p = t - s;
      ret[l] = (char *)malloc (p + 1);
      strncpy (ret[l], s, p);
      ret[l][p] = '\0';
      rxvt_Str_trim (ret[l]);
      s = ++t;
    }

  return ret;
}

void
rxvt_freecommastring (char **cs)
{
  for (int i = 0; cs[i]; ++i)
    free (cs[i]);

  free (cs);
}

/*----------------------------------------------------------------------*
 * file searching
 */

/* #define DEBUG_SEARCH_PATH */

#if defined (XPM_BACKGROUND) || (MENUBAR_MAX)
/*
 * search for FILE in the current working directory, and within the
 * colon-delimited PATHLIST, adding the file extension EXT if required.
 *
 * FILE is either semi-colon or zero terminated
 */
char           *
rxvt_File_search_path (const char *pathlist, const char *file, const char *ext)
{
  int             maxpath, len;
  const char     *p, *path;
  char            name[256];

  if (!access (file, R_OK))	/* found (plain name) in current directory */
    return strdup (file);

  /* semi-colon delimited */
  if ((p = strchr (file, ';')))
    len = (p - file);
  else
    len = strlen (file);

#ifdef DEBUG_SEARCH_PATH
  getcwd (name, sizeof (name));
  fprintf (stderr, "pwd: \"%s\"\n", name);
  fprintf (stderr, "find: \"%.*s\"\n", len, file);
#endif

  /* leave room for an extra '/' and trailing '\0' */
  maxpath = sizeof (name) - (len + (ext ? strlen (ext) : 0) + 2);
  if (maxpath <= 0)
    return NULL;

  /* check if we can find it now */
  strncpy (name, file, len);
  name[len] = '\0';

  if (!access (name, R_OK))
    return strdup (name);
  if (ext)
    {
      strcat (name, ext);
      if (!access (name, R_OK))
        return strdup (name);
    }
  for (path = pathlist; path != NULL && *path != '\0'; path = p)
    {
      int             n;

      /* colon delimited */
      if ((p = strchr (path, ':')) == NULL)
        p = strchr (path, '\0');

      n = (p - path);
      if (*p != '\0')
        p++;

      if (n > 0 && n <= maxpath)
        {
          strncpy (name, path, n);
          if (name[n - 1] != '/')
            name[n++] = '/';
          name[n] = '\0';
          strncat (name, file, len);

          if (!access (name, R_OK))
            return strdup (name);
          if (ext)
            {
              strcat (name, ext);
              if (!access (name, R_OK))
                return strdup (name);
            }
        }
    }
  return NULL;
}

char           *
rxvt_File_find (const char *file, const char *ext, const char *path)
{
  char           *f;

  if (file == NULL || *file == '\0')
    return NULL;

  /* search environment variables here too */
  if ((f = rxvt_File_search_path (path, file, ext)) == NULL)
#ifdef PATH_ENV
    if ((f = rxvt_File_search_path (getenv (PATH_ENV), file, ext)) == NULL)
#endif
      f = rxvt_File_search_path (getenv ("PATH"), file, ext);

#ifdef DEBUG_SEARCH_PATH
  if (f)
    fprintf (stderr, "found: \"%s\"\n", f);
#endif

  return f;
}
#endif				/* defined (XPM_BACKGROUND) || (MENUBAR_MAX) */

/*----------------------------------------------------------------------*
 * miscellaneous drawing routines
 */

/*
 * Draw top/left and bottom/right border shadows around windows
 */
#if defined(RXVT_SCROLLBAR) || defined(MENUBAR)
void
rxvt_Draw_Shadow (Display *display, Window win, GC topShadow, GC botShadow, int x, int y, int w, int h)
{
  int             shadow;

  shadow = (w == 0 || h == 0) ? 1 : SHADOW;
  w += x - 1;
  h += y - 1;
  for (; shadow-- > 0; x++, y++, w--, h--)
    {
      XDrawLine (display, win, topShadow, x, y, w, y);
      XDrawLine (display, win, topShadow, x, y, x, h);
      XDrawLine (display, win, botShadow, w, h, w, y + 1);
      XDrawLine (display, win, botShadow, w, h, x + 1, h);
    }
}
#endif

/* button shapes */
#ifdef MENUBAR
void
rxvt_Draw_Triangle (Display *display, Window win, GC topShadow, GC botShadow, int x, int y, int w, int type)
{
  switch (type)
    {
      case 'r':			/* right triangle */
        XDrawLine (display, win, topShadow, x, y, x, y + w);
        XDrawLine (display, win, topShadow, x, y, x + w, y + w / 2);
        XDrawLine (display, win, botShadow, x, y + w, x + w, y + w / 2);
        break;

      case 'l':			/* left triangle */
        XDrawLine (display, win, botShadow, x + w, y + w, x + w, y);
        XDrawLine (display, win, botShadow, x + w, y + w, x, y + w / 2);
        XDrawLine (display, win, topShadow, x, y + w / 2, x + w, y);
        break;

      case 'd':			/* down triangle */
        XDrawLine (display, win, topShadow, x, y, x + w / 2, y + w);
        XDrawLine (display, win, topShadow, x, y, x + w, y);
        XDrawLine (display, win, botShadow, x + w, y, x + w / 2, y + w);
        break;

      case 'u':			/* up triangle */
        XDrawLine (display, win, botShadow, x + w, y + w, x + w / 2, y);
        XDrawLine (display, win, botShadow, x + w, y + w, x, y + w);
        XDrawLine (display, win, topShadow, x, y + w, x + w / 2, y);
        break;
#if 0
      case 's':			/* square */
        XDrawLine (display, win, topShadow, x + w, y, x, y);
        XDrawLine (display, win, topShadow, x, y, x, y + w);
        XDrawLine (display, win, botShadow, x, y + w, x + w, y + w);
        XDrawLine (display, win, botShadow, x + w, y + w, x + w, y);
        break;
#endif

    }
}
#endif

// should nto be use din interactive programs, for obvious reasons
void rxvt_usleep (int usecs)
{
#if HAVE_NANOSLEEP
  struct timespec ts;

  ts.tv_sec = 0;
  ts.tv_nsec = usecs * 1000;
  nanosleep (&ts, NULL);
#else                 
  /* use select for timing */
  struct timeval  tv;

  tv.tv_sec = 0;
  tv.tv_usec = usecs;
  select (0, NULL, NULL, NULL, &tv);
#endif                
}

/*----------------------- end-of-file (C source) -----------------------*/

