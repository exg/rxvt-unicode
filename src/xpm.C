/*----------------------------------------------------------------------*
 * File:	xpm.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1997      Carsten Haitzler <raster@zip.com.au>
 * Copyright (c) 1997,1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright (c) 1998-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2005-2006 Marc Lehmann <pcg@goof.com>
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
 *---------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */

#ifdef XPM_BACKGROUND

#ifndef HAVE_AFTERIMAGE
/*
 * search for FILE in the current working directory, and within the
 * colon-delimited PATHLIST, adding the file extension EXT if required.
 *
 * FILE is either semi-colon or zero terminated
 */
static char *
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

/*
 * Calculate tiling sizes and increments
 * At start, p == 0, incr == xpmwidthheight
 */
static void
rxvt_pixmap_incr (unsigned int *wh, unsigned int *xy, float *incr, float *p, unsigned int widthheight, unsigned int xpmwidthheight)
{
  unsigned int    cwh, cxy;
  float           cincr, cp;

  cp = 0;
  cincr = (float)xpmwidthheight;
  cxy = *xy;
  cwh = *wh;
  if (cwh == 1)
    {	/* display one image, no horizontal/vertical scaling */
      cincr = (float)widthheight;
      if (xpmwidthheight <= widthheight)
        {
          cwh = xpmwidthheight;
          cxy = (cxy * (widthheight - cwh)) / 100;	/* beware! order */
          cwh += cxy;
        }
      else
        {
          cxy = 0;
          cwh = widthheight;
        }
    }
  else if (cwh < 10)
    {	/* fit WH images across/down screen */
      cincr *= cwh;
      cxy = 0;
      cwh = widthheight;
    }
  else
    {
      cincr *= 100.0 / cwh;
      if (cwh < 100)
        {	/* contract */
          float           pos;

          cwh = (cwh * widthheight) / 100;
          pos = (float)cxy / 100 * widthheight - (cwh / 2);

          cxy = (widthheight - cwh);
          if (pos <= 0)
            cxy = 0;
          else if (pos < cxy)
            cxy = (int) pos;
          cwh += cxy;
        }
      else
        {	/* expand */
          if (cxy > 0)
            {	/* position */
              float           pos;

              pos = (float)cxy / 100 * xpmwidthheight - (cincr / 2);
              cp = xpmwidthheight - cincr;
              if (pos <= 0)
                cp = 0;
              else if (pos < cp)
                cp = pos;
            }
          cxy = 0;
          cwh = widthheight;
        }
    }
  cincr /= widthheight;
  *wh = cwh;
  *xy = cxy;
  *incr = cincr;
  *p = cp;
}
#endif

/*
 * These GEOM strings indicate absolute size/position:
 * @ `WxH+X+Y'
 * @ `WxH+X'    -> Y = X
 * @ `WxH'      -> Y = X = 50
 * @ `W+X+Y'    -> H = W
 * @ `W+X'      -> H = W, Y = X
 * @ `W'        -> H = W, X = Y = 50
 * @ `0xH'      -> H *= H/100, X = Y = 50 (W unchanged)
 * @ `Wx0'      -> W *= W/100, X = Y = 50 (H unchanged)
 * @ `=+X+Y'    -> (H, W unchanged)
 * @ `=+X'      -> Y = X (H, W unchanged)
 *
 * These GEOM strings adjust position relative to current position:
 * @ `+X+Y'
 * @ `+X'       -> Y = X
 *
 * And this GEOM string is for querying current scale/position:
 * @ `?'
 */
int
rxvt_term::scale_pixmap (const char *geom)
{
  int flags, changed = 0;
  int x = 0, y = 0;
  unsigned int w = 0, h = 0;
  unsigned int n;
  char *p;
  bgPixmap_t *bgpixmap = &bgPixmap;

#define MAXLEN_GEOM		sizeof("[10000x10000+10000+10000]")

  if (geom == NULL)
    return 0;

  char str[MAXLEN_GEOM];

  if (!strcmp (geom, "?"))
    {
      sprintf (str, "[%dx%d+%d+%d]",	/* can't presume snprintf () ! */
              min (bgpixmap->w, 32767), min (bgpixmap->h, 32767),
              min (bgpixmap->x, 32767), min (bgpixmap->y, 32767));
      process_xterm_seq (XTerm_title, str, CHAR_ST);
      return 0;
    }

  if ((p = strchr (geom, ';')) == NULL)
    p = strchr (geom, '\0');

  n = (p - geom);
  if (n < MAXLEN_GEOM)
    {
      strncpy (str, geom, n);
      str[n] = '\0';

      if (strcmp(str, "auto") == 0)
        {
          if (!bgpixmap->auto_resize)
            changed++;
          bgpixmap->auto_resize = True ;
          w = szHint.width ;
          h = szHint.height ;
          flags = WidthValue|HeightValue ;
        }
      else
        {
          bgpixmap->auto_resize = False ;
          flags = XParseGeometry (str, &x, &y, &w, &h);
        }

      if (!flags)
        {
          flags |= WidthValue;
          w = 0;
        }			/* default is tile */

      if (flags & WidthValue)
        {
          if (!(flags & XValue))
            x = 50;

          if (!(flags & HeightValue))
            h = w;

          if (w && !h)
            {
              w = (bgpixmap->w * w) / 100;
              h = bgpixmap->h;
            }
          else if (h && !w)
            {
              w = bgpixmap->w;
              h = (bgpixmap->h * h) / 100;
            }

          min_it (w, 32767);
          min_it (h, 32767);

          if (bgpixmap->w != (short)w)
            {
              bgpixmap->w = (short)w;
              changed++;
            }

          if (bgpixmap->h != (short)h)
            {
              bgpixmap->h = (short)h;
              changed++;
            }
        }

      if (!(flags & YValue))
        {
          if (flags & XNegative)
            flags |= YNegative;

          y = x;
        }

      if (!(flags & WidthValue) && geom[0] != '=')
        {
          x += bgpixmap->x;
          y += bgpixmap->y;
        }

      if (xpmAttr.width && xpmAttr.height)
        {
          x = MOD(x, xpmAttr.width);
          y = MOD(y, xpmAttr.height);
        }

      if (bgpixmap->x != x)
        {
          bgpixmap->x = x;
          changed++;
        }

      if (bgpixmap->y != y)
        {
          bgpixmap->y = y;
          changed++;
        }
    }

  return changed;
}

void
rxvt_term::resize_pixmap ()
{
  XGCValues gcvalue;
  GC gc;

  if (pixmap != None)
    {
      XFreePixmap (dpy, pixmap);
      pixmap = None ;
    }

  if (bgPixmap.pixmap == None)
    { /* So be it: I'm not using pixmaps */
      pixmap = None;

#ifdef ENABLE_TRANSPARENCY
      if (!option (Opt_transparent) || !am_transparent)
#endif
        XSetWindowBackground (dpy, vt, pix_colors[Color_bg]);

      return;
    }

  gcvalue.foreground = pix_colors[Color_bg];
  gc = XCreateGC (dpy, vt, GCForeground, &gcvalue);

  if (bgPixmap.pixmap != None)
    {	/* we have a specified pixmap */
      unsigned int w = bgPixmap.w, h = bgPixmap.h,
                   x = bgPixmap.x, y = bgPixmap.y;
      unsigned int xpmh = xpmAttr.height,
                   xpmw = xpmAttr.width;

      if (bgPixmap.auto_resize)
        {
          w = szHint.width ;
          h = szHint.height ;
        }
      /*
       * don't zoom pixmap too much nor expand really small pixmaps
       */
      if (w > 32767 || h > 32767)
        w = 1;
      else if (width > (10 * xpmw)
               || height > (10 * xpmh))
        w = 0;		/* tile */

      if (!w)
        {
          /* basic X tiling - let the X server do it */
          pixmap = XCreatePixmap (dpy, vt, xpmw, xpmh, depth);

          XCopyArea (dpy, bgPixmap.pixmap, pixmap, gc, x, y, xpmw - x, xpmh - y,        0,        0);
          XCopyArea (dpy, bgPixmap.pixmap, pixmap, gc, x, 0, xpmw - x,        y,        0, xpmh - y);
          XCopyArea (dpy, bgPixmap.pixmap, pixmap, gc, 0, y,        x, xpmh - y, xpmw - x,        0);
          XCopyArea (dpy, bgPixmap.pixmap, pixmap, gc, 0, 0,        x,        y, xpmw - x, xpmh - y);
        }
      else
#ifdef HAVE_AFTERIMAGE
#ifdef ENABLE_TRANSPARENCY
      if (!option(Opt_transparent) || !am_transparent)
      /* will do that in check_our_parents otherwise */
#endif
        {
          ASImage *scaled_im = scale_asimage (asv, original_asim, w, h, ASA_XImage, 0, ASIMAGE_QUALITY_DEFAULT);
          if (scaled_im)
            {
              pixmap = asimage2pixmap(asv, display->root, scaled_im, gc, True);
              destroy_asimage (&scaled_im);
            }
        }
#else   /* HAVE_AFTERIMAGE */
        {
          float incr, p;
          Pixmap tmp;

          pixmap = XCreatePixmap (dpy, vt, width, height, depth);
          /*
           * horizontal scaling
           */
          rxvt_pixmap_incr (&w, &x, &incr, &p, width, xpmw);

          tmp = XCreatePixmap (dpy, vt, width, xpmh, depth);
          XFillRectangle (dpy, tmp, gc, 0, 0, width, xpmh);

          for ( /*nil */ ; x < w; x++, p += incr)
            {
              if (p >= xpmw)
                p = 0;

              /* copy one column from the original pixmap to the tmp pixmap */
              XCopyArea (dpy, bgPixmap.pixmap, tmp, gc, (int)p, 0, 1, xpmh, (int)x, 0);
            }

          /*
           * vertical scaling
           */
          rxvt_pixmap_incr (&h, &y, &incr, &p, height, xpmh);

          if (y > 0)
            XFillRectangle (dpy, pixmap, gc, 0, 0, width, y);

          if (h < height)
            XFillRectangle (dpy, pixmap, gc, 0, (int)h, width, height - h + 1);

          for ( /*nil */ ; y < h; y++, p += incr)
            {
              if (p >= xpmh)
                p = 0;

              /* copy one row from the tmp pixmap to the main pixmap */
              XCopyArea (dpy, tmp, pixmap, gc, 0, (int)p, width, 1, 0, (int)y);
            }

          XFreePixmap (dpy, tmp);
        }
#endif /* HAVE_AFTERIMAGE */
    }

  XSetWindowBackgroundPixmap (dpy, vt, pixmap);

  XFreeGC (dpy, gc);
#ifdef ENABLE_TRANSPARENCY
  am_transparent = 0;
#endif
}

Pixmap
rxvt_term::set_bgPixmap (const char *file)
{
  char *f;

  assert (file != NULL);

  if (bgPixmap.pixmap != None)
    {
      XFreePixmap (dpy, bgPixmap.pixmap);
      bgPixmap.pixmap = None;
    }

  XSetWindowBackground (dpy, vt, pix_colors[Color_bg]);

  if (*file != '\0')
    {
      /*      XWindowAttributes attr; */

      /*
       * we already have the required attributes
       */
      /*      XGetWindowAttributes (dpy, vt, &attr); */

#ifdef HAVE_AFTERIMAGE
      if (asimman == NULL)
        asimman = create_generic_imageman(rs[Rs_path]);
      if ((f = strchr (file, ';')) == NULL)
        original_asim = get_asimage( asimman, file, 0xFFFFFFFF, 100 );
      else
        {
          size_t len = f - file;
          f = (char *)malloc (len + 1);
          strncpy (f, file, len);
          f[len] = '\0';
          original_asim = get_asimage( asimman, f, 0xFFFFFFFF, 100 );
          free( f );
        }
      if (original_asim)
        {
          bgPixmap.pixmap = asimage2pixmap (asv, display->root, original_asim, NULL, True);
          xpmAttr.width = original_asim->width ;
          xpmAttr.height = original_asim->height ;
        }
#else /* HAVE_AFTERIMAGE */
      xpmAttr.closeness = 30000;
      xpmAttr.colormap = cmap;
      xpmAttr.visual = visual;
      xpmAttr.depth = depth;
      xpmAttr.valuemask = (XpmCloseness | XpmColormap | XpmVisual
                           | XpmDepth | XpmSize | XpmReturnPixels);

      /* search environment variables here too */
      f = rxvt_File_search_path (rs[Rs_path], file, ".xpm");
      if (f == NULL
          || XpmReadFileToPixmap (dpy, display->root, f,
                                  &bgPixmap.pixmap, NULL,
                                  &xpmAttr))
        {
          char *p;

          /* semi-colon delimited */
          if ((p = strchr (file, ';')) == NULL)
            p = strchr (file, '\0');

          rxvt_warn ("couldn't load XPM file \"%.*s\", ignoring.\n", (p - file), file);
        }
      free (f);
#endif /* HAVE_AFTERIMAGE */
    }

  resize_pixmap ();
  return bgPixmap.pixmap;
}

#endif				/* XPM_BACKGROUND */

#ifdef ENABLE_TRANSPARENCY
#if TINTING && !defined(HAVE_AFTERIMAGE)
/* taken from aterm-0.4.2 */

typedef uint32_t RUINT32T;

static void
ShadeXImage(rxvt_term *term, XImage* srcImage, int shade, int rm, int gm, int bm)
{
  int sh_r, sh_g, sh_b;
  RUINT32T mask_r, mask_g, mask_b;
  RUINT32T *lookup, *lookup_r, *lookup_g, *lookup_b;
  unsigned int lower_lim_r, lower_lim_g, lower_lim_b;
  unsigned int upper_lim_r, upper_lim_g, upper_lim_b;
  int i;

  Visual *visual = term->visual;

  if( visual->c_class != TrueColor || srcImage->format != ZPixmap ) return ;

  /* for convenience */
  mask_r = visual->red_mask;
  mask_g = visual->green_mask;
  mask_b = visual->blue_mask;

  /* boring lookup table pre-initialization */
  switch (srcImage->bits_per_pixel) {
    case 15:
      if ((mask_r != 0x7c00) ||
          (mask_g != 0x03e0) ||
          (mask_b != 0x001f))
        return;
        lookup = (RUINT32T *) malloc (sizeof (RUINT32T)*(32+32+32));
        lookup_r = lookup;
        lookup_g = lookup+32;
        lookup_b = lookup+32+32;
        sh_r = 10;
        sh_g = 5;
        sh_b = 0;
      break;
    case 16:
      if ((mask_r != 0xf800) ||
          (mask_g != 0x07e0) ||
          (mask_b != 0x001f))
        return;
        lookup = (RUINT32T *) malloc (sizeof (RUINT32T)*(32+64+32));
        lookup_r = lookup;
        lookup_g = lookup+32;
        lookup_b = lookup+32+64;
        sh_r = 11;
        sh_g = 5;
        sh_b = 0;
      break;
    case 24:
      if ((mask_r != 0xff0000) ||
          (mask_g != 0x00ff00) ||
          (mask_b != 0x0000ff))
        return;
        lookup = (RUINT32T *) malloc (sizeof (RUINT32T)*(256+256+256));
        lookup_r = lookup;
        lookup_g = lookup+256;
        lookup_b = lookup+256+256;
        sh_r = 16;
        sh_g = 8;
        sh_b = 0;
      break;
    case 32:
      if ((mask_r != 0xff0000) ||
          (mask_g != 0x00ff00) ||
          (mask_b != 0x0000ff))
        return;
        lookup = (RUINT32T *) malloc (sizeof (RUINT32T)*(256+256+256));
        lookup_r = lookup;
        lookup_g = lookup+256;
        lookup_b = lookup+256+256;
        sh_r = 16;
        sh_g = 8;
        sh_b = 0;
      break;
    default:
      return; /* we do not support this color depth */
  }

  /* prepare limits for color transformation (each channel is handled separately) */
  if (shade < 0) {
    shade = -shade;
    if (shade < 0) shade = 0;
    if (shade > 100) shade = 100;

    lower_lim_r = 65535-rm;
    lower_lim_g = 65535-gm;
    lower_lim_b = 65535-bm;

    lower_lim_r = 65535-(unsigned int)(((RUINT32T)lower_lim_r)*((RUINT32T)shade)/100);
    lower_lim_g = 65535-(unsigned int)(((RUINT32T)lower_lim_g)*((RUINT32T)shade)/100);
    lower_lim_b = 65535-(unsigned int)(((RUINT32T)lower_lim_b)*((RUINT32T)shade)/100);

    upper_lim_r = upper_lim_g = upper_lim_b = 65535;
  } else {
    if (shade < 0) shade = 0;
    if (shade > 100) shade = 100;

    lower_lim_r = lower_lim_g = lower_lim_b = 0;

    upper_lim_r = (unsigned int)((((RUINT32T)rm)*((RUINT32T)shade))/100);
    upper_lim_g = (unsigned int)((((RUINT32T)gm)*((RUINT32T)shade))/100);
    upper_lim_b = (unsigned int)((((RUINT32T)bm)*((RUINT32T)shade))/100);
  }

  /* switch red and blue bytes if necessary, we need it for some weird XServers like XFree86 3.3.3.1 */
  if ((srcImage->bits_per_pixel == 24) && (mask_r >= 0xFF0000 ))
  {
    unsigned int tmp;

    tmp = lower_lim_r;
    lower_lim_r = lower_lim_b;
    lower_lim_b = tmp;

    tmp = upper_lim_r;
    upper_lim_r = upper_lim_b;
    upper_lim_b = tmp;
  }

  /* fill our lookup tables */
  for (i = 0; i <= mask_r>>sh_r; i++)
  {
    RUINT32T tmp;
    tmp = ((RUINT32T)i)*((RUINT32T)(upper_lim_r-lower_lim_r));
    tmp += ((RUINT32T)(mask_r>>sh_r))*((RUINT32T)lower_lim_r);
    lookup_r[i] = (tmp/65535)<<sh_r;
  }
  for (i = 0; i <= mask_g>>sh_g; i++)
  {
    RUINT32T tmp;
    tmp = ((RUINT32T)i)*((RUINT32T)(upper_lim_g-lower_lim_g));
    tmp += ((RUINT32T)(mask_g>>sh_g))*((RUINT32T)lower_lim_g);
    lookup_g[i] = (tmp/65535)<<sh_g;
  }
  for (i = 0; i <= mask_b>>sh_b; i++)
  {
    RUINT32T tmp;
    tmp = ((RUINT32T)i)*((RUINT32T)(upper_lim_b-lower_lim_b));
    tmp += ((RUINT32T)(mask_b>>sh_b))*((RUINT32T)lower_lim_b);
    lookup_b[i] = (tmp/65535)<<sh_b;
  }

  /* apply table to input image (replacing colors by newly calculated ones) */
  switch (srcImage->bits_per_pixel)
  {
    case 15:
    {
      unsigned short *p1, *pf, *p, *pl;
      p1 = (unsigned short *) srcImage->data;
      pf = (unsigned short *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);
      while (p1 < pf)
      {
        p = p1;
        pl = p1 + srcImage->width;
        for (; p < pl; p++)
        {
          *p = lookup_r[(*p & 0x7c00)>>10] |
               lookup_g[(*p & 0x03e0)>> 5] |
               lookup_b[(*p & 0x001f)];
        }
        p1 = (unsigned short *) ((char *) p1 + srcImage->bytes_per_line);
      }
      break;
    }
    case 16:
    {
      unsigned short *p1, *pf, *p, *pl;
      p1 = (unsigned short *) srcImage->data;
      pf = (unsigned short *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);
      while (p1 < pf)
      {
        p = p1;
        pl = p1 + srcImage->width;
        for (; p < pl; p++)
        {
          *p = lookup_r[(*p & 0xf800)>>11] |
               lookup_g[(*p & 0x07e0)>> 5] |
               lookup_b[(*p & 0x001f)];
        }
        p1 = (unsigned short *) ((char *) p1 + srcImage->bytes_per_line);
      }
      break;
    }
    case 24:
    {
      unsigned char *p1, *pf, *p, *pl;
      p1 = (unsigned char *) srcImage->data;
      pf = (unsigned char *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);
      while (p1 < pf)
      {
        p = p1;
        pl = p1 + srcImage->width * 3;
        for (; p < pl; p += 3)
        {
          p[0] = lookup_r[(p[0] & 0xff0000)>>16];
          p[1] = lookup_r[(p[1] & 0x00ff00)>> 8];
          p[2] = lookup_r[(p[2] & 0x0000ff)];
        }
        p1 = (unsigned char *) ((char *) p1 + srcImage->bytes_per_line);
      }
      break;
    }
    case 32:
    {
      RUINT32T *p1, *pf, *p, *pl;
      p1 = (RUINT32T *) srcImage->data;
      pf = (RUINT32T *) (srcImage->data + srcImage->height * srcImage->bytes_per_line);

      while (p1 < pf)
      {
        p = p1;
        pl = p1 + srcImage->width;
        for (; p < pl; p++)
        {
          *p = lookup_r[(*p & 0xff0000)>>16] |
               lookup_g[(*p & 0x00ff00)>> 8] |
               lookup_b[(*p & 0x0000ff)] |
               (*p & ~0xffffff);
        }
        p1 = (RUINT32T *) ((char *) p1 + srcImage->bytes_per_line);
      }
      break;
    }
  }

  free (lookup);
}
#endif

/*
 * Check our parents are still who we think they are.
 * Do transparency updates if required
 */
int
rxvt_term::check_our_parents ()
{
  check_our_parents_ev.stop ();
  check_our_parents_ev.start (NOW + .1);
  return 0;
}

void
rxvt_term::check_our_parents_cb (time_watcher &w)
{
  int i, pchanged, aformat, rootdepth;
  unsigned long nitems, bytes_after;
  Atom atype;
  unsigned char *prop = NULL;
  Window root, oldp, *list;
  Pixmap rootpixmap = None;
  XWindowAttributes wattr, wrootattr;
  int sx, sy;
  Window cr;
  unsigned int rootpixmap_w = 0, rootpixmap_h = 0;

  pchanged = 0;

  if (!option (Opt_transparent))
    return /*pchanged*/;	/* Don't try any more */

#if 0
  struct timeval stv;
	gettimeofday (&stv,NULL);
#define PRINT_BACKGROUND_OP_TIME do{ struct timeval tv;gettimeofday (&tv,NULL); tv.tv_sec-= stv.tv_sec;\
                                     fprintf (stderr,"%d: elapsed  %ld usec\n",__LINE__,\
                                              tv.tv_sec*1000000+tv.tv_usec-stv.tv_usec );}while(0)
#else                                           
#define PRINT_BACKGROUND_OP_TIME do{}while(0)                                          
#endif
 

  XGetWindowAttributes (dpy, display->root, &wrootattr);
  rootdepth = wrootattr.depth;

  XGetWindowAttributes (dpy, parent[0], &wattr);

  if (rootdepth != wattr.depth)
    {
      if (am_transparent)
        {
          pchanged = 1;
          XSetWindowBackground (dpy, vt, pix_colors_focused[Color_bg]);
          am_transparent = am_pixmap_trans = 0;
        }

      return /*pchanged*/;	/* Don't try any more */
    }

  /* Get all X ops out of the queue so that our information is up-to-date. */
  XSync (dpy, False);

  XTranslateCoordinates (dpy, parent[0], display->root,
                          0, 0, &sx, &sy, &cr);
    /* check if we are outside of the visible part of the virtual screen : */
  if( sx + (int)szHint.width <= 0 || sy + (int)szHint.height <= 0
      || sx >= wrootattr.width || sy >= wrootattr.height )
    return /* 0 */ ;
  /*
   * Make the frame window set by the window manager have
   * the root background. Some window managers put multiple nested frame
   * windows for each client, so we have to take care about that.
   */
  i = (xa[XA_XROOTPMAP_ID]
       && XGetWindowProperty (dpy, display->root, xa[XA_XROOTPMAP_ID],
                              0L, 1L, False, XA_PIXMAP, &atype, &aformat,
                              &nitems, &bytes_after, &prop) == Success);

  if (!i || prop == NULL)
     i = (xa[XA_ESETROOT_PMAP_ID]
          && XGetWindowProperty (dpy, display->root, xa[XA_ESETROOT_PMAP_ID],
                                 0L, 1L, False, XA_PIXMAP, &atype, &aformat,
                                 &nitems, &bytes_after, &prop) == Success);

  /* TODO: the below logic needs to be cleaned up */
  if (!i || prop == NULL
#if TINTING
      || (!ISSET_PIXCOLOR (Color_tint) && rs[Rs_shade] == NULL
#ifdef HAVE_AFTERIMAGE
          && original_asim == NULL && rs[Rs_blurradius] == NULL
#endif
         )
#endif
      )
    rootpixmap = None;
  else
    {
      int junk;
      unsigned int ujunk;
      /* root pixmap may be bad - allow a error */
      allowedxerror = -1;
      if ((rootpixmap = *(Pixmap *)prop) != None)
        if (!XGetGeometry (dpy, rootpixmap, &root, &junk, &junk, &rootpixmap_w, &rootpixmap_h, &ujunk, &ujunk))
          rootpixmap = None;
      allowedxerror = 0;
    }

  if (prop != NULL)
    XFree (prop);

  if (rootpixmap != None)
    {
      Bool success = False;
      GC gc = NULL;
      XGCValues gcvalue;
      int shade = 100;
      rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
      Bool whole_tint = False, no_tint = True;

      while (sx < 0) sx += (int)wrootattr.width;
      while (sy < 0) sy += (int)wrootattr.height;

#if TINTING
      if (rs[Rs_shade])
        shade = atoi (rs[Rs_shade]);
      if (ISSET_PIXCOLOR (Color_tint))
        pix_colors_focused [Color_tint].get (c);
#define IS_COMPONENT_WHOLESOME(c)  ((c) <=0x000700 || (c)>=0x00f700)
      if (shade >= 100)
        whole_tint = (IS_COMPONENT_WHOLESOME(c.r)
                      && IS_COMPONENT_WHOLESOME(c.g)
                      && IS_COMPONENT_WHOLESOME(c.b));
      no_tint = (c.r >= 0x00f700 && c.g >= 0x00f700 && c.b >= 0x00f700);
#undef  IS_COMPONENT_WHOLESOME
#endif /* TINTING */
      /* theer are no performance advantages to reusing same pixmap */
      if (pixmap != None)
        XFreePixmap (dpy, pixmap);
      pixmap = XCreatePixmap (dpy, vt, szHint.width, szHint.height, rootdepth);

#if 0 /* TODO : identify cases where this will be detrimental to performance : */
      /* we want to tile root pixmap into our own pixmap in this cases :
       * 1) rootpixmap does not cover our window entirely
       * 2) whole_tint - we can use server-side tinting or tinting disabled
       */
      if ( whole_tint || no_tint || pmap_w < sx + szHint.width || pmap_h < sy + szHint.height)
        {
        }
#endif
      gcvalue.tile = rootpixmap;
      gcvalue.fill_style = FillTiled;
      gcvalue.ts_x_origin = -sx;
      gcvalue.ts_y_origin = -sy;
      gc = XCreateGC (dpy, rootpixmap, GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gcvalue);
      XFillRectangle (dpy, pixmap, gc, 0, 0, szHint.width, szHint.height);

#if TINTING
      if (whole_tint && !no_tint)
        {
          /* In this case we can tint image server-side getting significant
           * performance improvements, as we eliminate XImage transfer
           */
          gcvalue.foreground = Pixel (pix_colors_focused [Color_tint]);
          gcvalue.function = GXand;
          gcvalue.fill_style = FillSolid;
          XChangeGC (dpy, gc, GCFillStyle | GCForeground | GCFunction, &gcvalue);
          XFillRectangle (dpy, pixmap, gc, 0, 0, szHint.width, szHint.height);
        }
#endif
      success = True;
#ifdef HAVE_AFTERIMAGE
      if (rs[Rs_blurradius] || original_asim != NULL || (!whole_tint && (!no_tint || shade !=100)))
        {
          ARGB32 tint = TINT_LEAVE_SAME;
          ASImage *back_im = NULL;

          back_im = pixmap2ximage (asv, pixmap, 0, 0, szHint.width, szHint.height, AllPlanes, 100);
          if (back_im != NULL)
            {
              if (!whole_tint && (!no_tint || shade !=100))
                {
                  ShadingInfo as_shade;
                  as_shade.shading = shade;
                  as_shade.tintColor.red = c.r;
                  as_shade.tintColor.green = c.g;
                  as_shade.tintColor.blue = c.b;
                  tint = shading2tint32 (&as_shade);
                }

              if (rs[Rs_blurradius] && back_im)
                {
                  ASImage* tmp;
                  int junk;
                  unsigned int hr = 1, vr = 1;
                  int flags = XParseGeometry (rs[Rs_blurradius], &junk, &junk, &hr, &vr);
                  if (!(flags&WidthValue))
                    hr = 1;
                  if (!(flags&HeightValue))
                    vr = hr;
                  tmp = blur_asimage_gauss (asv, back_im, hr, vr, 0xFFFFFFFF,
                                            (original_asim == NULL || tint == TINT_LEAVE_SAME)?ASA_XImage:ASA_ASImage,
                                            100, ASIMAGE_QUALITY_DEFAULT);
                  if (tmp)
                    {
                      destroy_asimage (&back_im);
                      back_im = tmp;
                    }
                }

              if (original_asim != NULL)
                {
                  ASImageLayer *layers = create_image_layers (2);
                  ASImage *merged_im = NULL;
                  int fore_w, fore_h;

                  layers[0].im = back_im;
                  layers[0].clip_width = szHint.width;
                  layers[0].clip_height = szHint.height;
                  layers[0].tint = tint;
                  layers[1].im = original_asim;
                  if (bgPixmap.auto_resize)
                    {
                      fore_w = szHint.width;
                      fore_h = szHint.height;
                    }
                  else
                    {
                      fore_w = bgPixmap.w;
                      fore_h = bgPixmap.h;
                    }
                  if (fore_w != original_asim->width
                      || fore_h != original_asim->height)
                    {
                      layers[1].im = scale_asimage (asv,
                                                    original_asim,
                                                    fore_w, fore_h,
                                                    ASA_ASImage, 100,
                                                    ASIMAGE_QUALITY_DEFAULT);
                    }
                  layers[1].clip_width = szHint.width;
                  layers[1].clip_height = szHint.height;

                  if (rs[Rs_blendtype])
                    {
                      layers[1].merge_scanlines = blend_scanlines_name2func (rs[Rs_blendtype]);
                      if (layers[1].merge_scanlines == NULL)
                        layers[1].merge_scanlines = alphablend_scanlines;
                    }
                  PRINT_BACKGROUND_OP_TIME;
                  merged_im = merge_layers (asv, layers, 2, szHint.width, szHint.height,
                                            ASA_XImage, 0, ASIMAGE_QUALITY_DEFAULT);
                  if (layers[1].im != original_asim)
                      destroy_asimage (&(layers[1].im));
                  free (layers);

                  if (merged_im != NULL)
                    {
                      destroy_asimage (&back_im);
                      back_im = merged_im;
                    }
                    PRINT_BACKGROUND_OP_TIME;
                }
              else if (tint != TINT_LEAVE_SAME)
                {
                  ASImage* tmp = tile_asimage (asv, back_im, 0, 0, szHint.width, szHint.height, tint, ASA_XImage, 100, ASIMAGE_QUALITY_DEFAULT);
                  if (tmp)
                    {
                      destroy_asimage (&back_im);
                      back_im = tmp;
                    }
                    PRINT_BACKGROUND_OP_TIME;
                }
              asimage2drawable (asv, pixmap, back_im, gc, 0, 0, 0, 0, szHint.width, szHint.height, True);
              destroy_asimage (&back_im);
            } /* back_im != NULL */
          else
            success = False;
        }
#else  /* HAVE_AFTERIMAGE */
#if TINTING
      if (!whole_tint && (!no_tint || shade !=100))
        {
          XImage *image = XGetImage (dpy, pixmap, 0, 0, szHint.width, szHint.height, AllPlanes, ZPixmap);
          success = False;
          if (image != NULL)
            {
              PRINT_BACKGROUND_OP_TIME;
              if (gc == NULL)
                gc = XCreateGC (dpy, vt, 0UL, &gcvalue);
              if (ISSET_PIXCOLOR (Color_tint) || shade != 100)
                ShadeXImage (this, image, shade, c.r, c.g, c.b);
              XPutImage (dpy, pixmap, gc, image, 0, 0, 0, 0, image->width, image->height);
              XDestroyImage (image);
              success = True;
            }
        }
#endif
#endif  /* HAVE_AFTERIMAGE */
      PRINT_BACKGROUND_OP_TIME;

      if (gc != NULL)
        XFreeGC (dpy, gc);

      if (!success)
        {
          if (am_transparent && am_pixmap_trans)
            {
              pchanged = 1;
              if (pixmap != None)
                {
                  XFreePixmap (dpy, pixmap);
                  pixmap = None;
                }
            }

          am_pixmap_trans = 0;
        }
      else
        {
          XSetWindowBackgroundPixmap (dpy, parent[0], pixmap);
          XClearWindow (dpy, parent[0]);

          if (!am_transparent || !am_pixmap_trans)
            pchanged = 1;

          am_transparent = am_pixmap_trans = 1;
        }
    } /* rootpixmap != None */

  if (am_pixmap_trans)
    XSetWindowBackgroundPixmap (dpy, vt, ParentRelative);
  else
    {
      unsigned int n;
      /*
       * InheritPixmap transparency
       */
      for (i = 1; i < (int) (sizeof (parent) / sizeof (Window)); i++)
        {
          oldp = parent[i];
          XQueryTree (dpy, parent[i - 1], &root, &parent[i], &list, &n);
          XFree (list);

          if (parent[i] == display->root)
            {
              if (oldp != None)
                pchanged = 1;

              break;
            }

          if (oldp != parent[i])
            pchanged = 1;
        }

      n = 0;

      if (pchanged)
        for (; n < (unsigned int)i; n++)
          {
            XGetWindowAttributes (dpy, parent[n], &wattr);

            if (wattr.depth != rootdepth || wattr.c_class == InputOnly)
              {
                n = (int) (sizeof (parent) / sizeof (Window)) + 1;
                break;
              }
          }

      if (n > (sizeof (parent) / sizeof (parent[0])))
        {
          XSetWindowBackground (dpy, parent[0], pix_colors_focused[Color_border]);
          XSetWindowBackground (dpy, vt, pix_colors_focused[Color_bg]);
          am_transparent = 0;
          /* XXX: also turn off Opt_transparent? */
        }
      else
        {
          for (n = 0; n < (unsigned int)i; n++)
            {
              XSetWindowBackgroundPixmap (dpy, parent[n], ParentRelative);
              XClearWindow (dpy, parent[n]);
            }

          XSetWindowBackgroundPixmap (dpy, vt, ParentRelative);
          am_transparent = 1;
        }

      for (; i < (int) (sizeof (parent) / sizeof (Window)); i++)
        parent[i] = None;
    }

  if (scrollBar.win)
    {
      XSetWindowBackgroundPixmap (dpy, scrollBar.win, ParentRelative);
      scrollBar.setIdle ();
      scrollbar_show (0);
    }

  if (am_transparent)
    {
      want_refresh = want_full_refresh = 1;
      if (am_pixmap_trans)
        flush ();
    }

//  return pchanged;
}
#endif
