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

/*
 * Pixmap geometry string interpretation :
 * Each geometry string contains zero or one scale/position
 * adjustment and may optionally be followed by a colon and one or more
 * colon-delimited pixmap operations.
 * The following table shows the valid geometry strings and their
 * affects on the background image :
 *
 * WxH+X+Y  	Set scaling to W% by H%, and position to X% by Y%.
 *            W and H are percentages of the terminal window size.
 *            X and Y are also percentages; e.g., +50+50 centers
 *            the image in the window.
 * WxH+X 	    Assumes Y == X
 * WxH 	      Assumes Y == X == 50 (centers the image)
 * W+X+Y 	    Assumes H == W
 * W+X 	      Assumes H == W and Y == X
 * W 	        Assumes H == W and Y == X == 50
 *
 * Adjusting position only :
 * =+X+Y 	    Set position to X% by Y% (absolute).
 * =+X 	      Set position to X% by X%.
 * +X+Y 	    Adjust position horizontally X% and vertically Y%
 *            from current position (relative).
 * +X+X 	    Adjust position horizontally X% and vertically X%
 *            from current position.
 *
 * Adjusting scale only :
 * Wx0 	      Multiply horizontal scaling factor by W%
 * 0xH 	      Multiply vertical scaling factor by H%
 * 0x0 	      No scaling (show image at normal size).
 *
 * Pixmap Operations : (should be prepended by a colon)
 * tile 	    Tile image. Scaling/position modifiers above will affect
 *            the tile size and origin.
 * propscale 	When scaling, scale proportionally. That is, maintain the
 *            proper aspect ratio for the image. Any portion of the
 *            background not covered by the image is filled with the
 *            current background color.
 * hscale     Scale horizontally, tile vertically ?
 * vscale     Tile horizontally, scale vertically ?
 * scale      Scale both up and down
 * auto       Same as 100x100+50+50
 */

#ifdef HAVE_BG_PIXMAP
bool
bgPixmap_t::window_size_sensitive ()
{
  return (flags&(bgPmap_Scale|bgPmap_Transparent));
}

#ifdef XPM_BACKGROUND
bool
bgPixmap_t::handle_geometry (const char *geom)
{
  int geom_flags, changed = 0;
  int x = 0, y = 0;
  unsigned int w = 0, h = 0;
  unsigned int n;
  unsigned long new_flags = (flags&(~bgPmap_geometryFlags)) ;
  char *p;
#define MAXLEN_GEOM		sizeof("[10000x10000+10000+10000]")

  if (geom == NULL)
    return false;

  char str[MAXLEN_GEOM];

  if (!strcmp (geom, "?"))
    {
#if 0 /* TODO: */    
      sprintf (str, "[%dx%d+%d+%d]",	/* can't presume snprintf () ! */
              min (h_scale, 32767), min (v_scale, 32767),
              min (h_align, 32767), min (v_align, 32767));
      process_xterm_seq (XTerm_title, str, CHAR_ST);
#endif      
      return false;
    }

  if ((p = strchr (geom, ';')) == NULL)
    p = strchr (geom, '\0');

  n = (p - geom);
  if (n < MAXLEN_GEOM)
    {
      new_flags |= bgPmap_geometrySet;

      strncpy (str, geom, n);
      str[n] = '\0';

      if (strcmp(str, "auto") == 0)
        {
          w = h = 100;
          geom_flags = WidthValue|HeightValue ;
        }
      else
        {
          geom_flags = XParseGeometry (str, &x, &y, &w, &h);
        }
/* code below is garbage and needs to be rewritten */
      if (!geom_flags)
        {
          geom_flags |= WidthValue;
          w = 0;
        }			/* default is tile */

      if (geom_flags & WidthValue)
        {
          if (!(geom_flags & XValue))
            x = 50;

          if (!(geom_flags & HeightValue))
            h = w;

          if (w && !h)
            {
              w = (h_scale * w) / 100;
              h = v_scale;
            }
          else if (h && !w)
            {
              w = h_scale;
              h = (v_scale * h) / 100;
            }

          min_it (w, 32767);
          min_it (h, 32767);

          if (h_scale != w)
            {
              h_scale = w;
              changed++;
            }

          if (v_scale != h)
            {
              v_scale = h;
              changed++;
            }
        }
      if (!(geom_flags & YValue))
        {
          if (geom_flags & XNegative)
            geom_flags |= YNegative;

          y = x;
        }

      if (!(geom_flags & WidthValue) && geom[0] != '=')
        {
          x += h_align;
          y += v_align;
        }

      if (h_align != x)
        {
          h_align = x;
          changed++;
        }

      if (v_align != y)
        {
          v_align = y;
          changed++;
        }

      if (h_scale != 0)
        new_flags |= bgPmap_hScale;
      if (v_scale != 0)
        new_flags |= bgPmap_vScale;
    }

  if (new_flags != flags)
    {
      flags = new_flags;
      changed++;
    }
  return (changed > 0);
}

void
rxvt_term::resize_pixmap ()
{
  XGCValues gcvalue;
  GC gc;
  unsigned int w = bgPixmap.h_scale*szHint.width/100;
  unsigned int h = bgPixmap.v_scale*szHint.height/100;
  int x = bgPixmap.h_align*szHint.width/100;
  int y = bgPixmap.v_align*szHint.height/100;
#ifdef HAVE_AFTERIMAGE
  ASImage *im = bgPixmap.original_asim;
#else
  void *im = NULL;
#endif
/* preliminary cleanup - this needs to be integrated with check_our_parents() code */
  if (bgPixmap.pixmap != None)
    {
      XFreePixmap (dpy, bgPixmap.pixmap);
      bgPixmap.pixmap = None ;
    }
#ifdef ENABLE_TRANSPARENCY
  if (option(Opt_transparent) && am_transparent)
    {
      /*  we need to re-generate transparency pixmap in that case ! */
      check_our_parents ();
      return;      
    }
#endif

  if (im == NULL)
    { /* So be it: I'm not using pixmaps */
      XSetWindowBackground (dpy, vt, pix_colors[Color_bg]);
      return;
    }

  gcvalue.foreground = pix_colors[Color_bg];
  gc = XCreateGC (dpy, vt, GCForeground, &gcvalue);

  /* don't zoom pixmap too much nor expand really small pixmaps  */
  if (w > 16000)
    w = 1;
  if (h > 16000)
    h = 1;
  
#ifdef HAVE_AFTERIMAGE
  if (w == 0)
    w = im->width;
  if (h == 0)
    h = im->height;

  if (w != im->width || h != im->height)
    {
      ASImage *tmp = scale_asimage (asv, im, w, h, (x == 0 && y == 0)?ASA_XImage:ASA_ASImage, 0, ASIMAGE_QUALITY_DEFAULT);
      if (tmp != NULL)
        im = tmp;
    }
  bgPixmap.pmap_width = MIN(w,szHint.width);
  bgPixmap.pmap_height = MIN(h,szHint.height);
#if 0 /* TODO: fix that! */
  if (x != 0 || y != 0)
    {
      ASImage *tmp = tile_asimage (asv, im, x, y, w, h, TINT_LEAVE_SAME, ASA_XImage, 0, ASIMAGE_QUALITY_DEFAULT);
      if (tmp != NULL)
        {
          if (im != bgPixmap.original_asim)
            destroy_asimage (&im);
          im = tmp;
        }
    }
#endif    
  bgPixmap.pixmap = XCreatePixmap (dpy, vt, bgPixmap.pmap_width, bgPixmap.pmap_height, depth);
  bgPixmap.pmap_depth = depth;

  asimage2drawable (asv, bgPixmap.pixmap, im, gc, 0, 0, 0, 0, bgPixmap.pmap_width, bgPixmap.pmap_height, True);

  if (im != bgPixmap.original_asim)
    destroy_asimage (&im);
#endif
  if( bgPixmap.pixmap )
    XSetWindowBackgroundPixmap (dpy, vt, bgPixmap.pixmap);

  XFreeGC (dpy, gc);
}

void
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
#ifdef HAVE_AFTERIMAGE
      if (asimman == NULL)
        asimman = create_generic_imageman(rs[Rs_path]);
      if ((f = strchr (file, ';')) == NULL)
        bgPixmap.original_asim = get_asimage( asimman, file, 0xFFFFFFFF, 100 );
      else
        {
          size_t len = f - file;
          f = (char *)malloc (len + 1);
          strncpy (f, file, len);
          f[len] = '\0';
          bgPixmap.original_asim = get_asimage( asimman, f, 0xFFFFFFFF, 100 );
          free( f );
        }
#endif    
    }

  resize_pixmap ();
}

#endif				/* XPM_BACKGROUND */
#endif				/* HAVE_BG_PIXMAP */

#ifdef ENABLE_TRANSPARENCY
#ifndef HAVE_AFTERIMAGE
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
      || (!ISSET_PIXCOLOR (Color_tint) && rs[Rs_shade] == NULL
#ifdef HAVE_AFTERIMAGE
          && bgPixmap.original_asim == NULL && rs[Rs_blurradius] == NULL
#endif
         )
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
      /* theer are no performance advantages to reusing same pixmap */
      if (bgPixmap.pixmap != None)
        XFreePixmap (dpy, bgPixmap.pixmap);
      bgPixmap.pixmap = XCreatePixmap (dpy, vt, szHint.width, szHint.height, rootdepth);
      bgPixmap.pmap_width = szHint.width;
      bgPixmap.pmap_height = szHint.height;
      bgPixmap.pmap_depth = rootdepth;

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
      XFillRectangle (dpy, bgPixmap.pixmap, gc, 0, 0, szHint.width, szHint.height);

      if (whole_tint && !no_tint)
        {
          /* In this case we can tint image server-side getting significant
           * performance improvements, as we eliminate XImage transfer
           */
          gcvalue.foreground = Pixel (pix_colors_focused [Color_tint]);
          gcvalue.function = GXand;
          gcvalue.fill_style = FillSolid;
          XChangeGC (dpy, gc, GCFillStyle | GCForeground | GCFunction, &gcvalue);
          XFillRectangle (dpy, bgPixmap.pixmap, gc, 0, 0, szHint.width, szHint.height);
        }
      success = True;
#ifdef HAVE_AFTERIMAGE
      if (rs[Rs_blurradius] || bgPixmap.original_asim != NULL || (!whole_tint && (!no_tint || shade !=100)))
        {
          ARGB32 tint = TINT_LEAVE_SAME;
          ASImage *back_im = NULL;

          back_im = pixmap2ximage (asv, bgPixmap.pixmap, 0, 0, szHint.width, szHint.height, AllPlanes, 100);
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
                                            (bgPixmap.original_asim == NULL || tint == TINT_LEAVE_SAME)?ASA_XImage:ASA_ASImage,
                                            100, ASIMAGE_QUALITY_DEFAULT);
                  if (tmp)
                    {
                      destroy_asimage (&back_im);
                      back_im = tmp;
                    }
                }

              if (bgPixmap.original_asim != NULL)
                {
                  ASImageLayer *layers = create_image_layers (2);
                  ASImage *merged_im = NULL;
                  int fore_w, fore_h;

                  layers[0].im = back_im;
                  layers[0].clip_width = szHint.width;
                  layers[0].clip_height = szHint.height;
                  layers[0].tint = tint;
                  layers[1].im = bgPixmap.original_asim;

                  fore_w = (bgPixmap.h_scale == 0) ? bgPixmap.original_asim->width : bgPixmap.h_scale*szHint.width/100;
                  fore_h = (bgPixmap.v_scale == 0) ? bgPixmap.original_asim->height : bgPixmap.v_scale*szHint.height/100;

                  if (fore_w != bgPixmap.original_asim->width
                      || fore_h != bgPixmap.original_asim->height)
                    {
                      layers[1].im = scale_asimage (asv,
                                                    bgPixmap.original_asim,
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
                  if (layers[1].im != bgPixmap.original_asim)
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
              asimage2drawable (asv, bgPixmap.pixmap, back_im, gc, 0, 0, 0, 0, szHint.width, szHint.height, True);
              destroy_asimage (&back_im);
            } /* back_im != NULL */
          else
            success = False;
        }
#else  /* HAVE_AFTERIMAGE */
      if (!whole_tint && (!no_tint || shade !=100))
        {
          XImage *image = XGetImage (dpy, bgPixmap.pixmap, 0, 0, szHint.width, szHint.height, AllPlanes, ZPixmap);
          success = False;
          if (image != NULL)
            {
              PRINT_BACKGROUND_OP_TIME;
              if (gc == NULL)
                gc = XCreateGC (dpy, vt, 0UL, &gcvalue);
              if (ISSET_PIXCOLOR (Color_tint) || shade != 100)
                ShadeXImage (this, image, shade, c.r, c.g, c.b);
              XPutImage (dpy, bgPixmap.pixmap, gc, image, 0, 0, 0, 0, image->width, image->height);
              XDestroyImage (image);
              success = True;
            }
        }
#endif  /* HAVE_AFTERIMAGE */
      PRINT_BACKGROUND_OP_TIME;

      if (gc != NULL)
        XFreeGC (dpy, gc);

      if (!success)
        {
          if (am_transparent && am_pixmap_trans)
            {
              pchanged = 1;
              if (bgPixmap.pixmap != None)
                {
                  XFreePixmap (dpy, bgPixmap.pixmap);
                  bgPixmap.pixmap = None;
                }
            }

          am_pixmap_trans = 0;
        }
      else
        {
          XSetWindowBackgroundPixmap (dpy, parent[0], bgPixmap.pixmap);
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
