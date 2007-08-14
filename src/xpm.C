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
 * +X   	    Adjust position horizontally X% and vertically X%
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
# ifdef XPM_BACKGROUND
#  ifdef HAVE_AFTERIMAGE
  if (original_asim != NULL)
#  endif
    {
      if (h_scale != 0 || v_scale != 0)
        return true;
    }
# endif
# ifdef ENABLE_TRANSPARENCY
  if (flags & isTransparent)
    return true;
# endif
  return false;
}

# ifdef XPM_BACKGROUND
static inline bool
check_set_scale_value (int geom_flags, int flag, unsigned int &scale, unsigned int new_value)
{
  if (geom_flags & flag)
    {
      if (new_value > 1000)
        new_value = 1000;
      if (new_value != scale)
        {
          scale = new_value;
          return true;
        }
    }
  return false;
}

static inline bool
check_set_align_value (int geom_flags, int flag, int &align, int new_value)
{
  if (geom_flags & flag)
    {
      if (new_value < -100)
        new_value = -100;
      else if (new_value > 200)
        new_value = 200;
      if (new_value != align)
        {
          align = new_value;
          return true;
        }
    }
  return false;
}

static inline int
make_align_position (int align, int window_size, int image_size)
{
  int diff = window_size - image_size;
  int smaller = MIN (image_size,window_size);

  if (align >= 0 && align <= 50)
    return diff * align / 100;
  else if (align > 50 && align <= 100)
    return window_size - image_size - diff * (100 - align) / 100;
  else if (align > 100 && align <= 200 )
    return ((align - 100) * smaller / 100) + window_size - smaller;
  else if (align > -100 && align < 0)
    return ((align + 100) * smaller / 100) - image_size;
  return 0;
}

static inline int
make_clip_rectangle (int pos, int size, int target_size, int &dst_pos, int &dst_size)
{
  int src_pos = 0;
  dst_pos = 0;
  dst_size = size;
  if (pos < 0 && size > target_size)
    {
      src_pos = -pos;
      dst_size += pos;
    }
  else if (pos > 0)
    dst_pos = pos;

  if (dst_pos + dst_size > target_size)
    dst_size = target_size - dst_pos;
  return src_pos;
}

bool
bgPixmap_t::set_geometry (const char *geom)
{
  int geom_flags = 0, changed = 0;
  int x = 0, y = 0;
  unsigned int w = 0, h = 0;
  unsigned int n;
  unsigned long new_flags = (flags & (~geometryFlags));
  char *p;
#  define MAXLEN_GEOM		256 /* could be longer then regular geometry string */

  if (geom == NULL)
    return false;

  char str[MAXLEN_GEOM];

  while (isspace(*geom)) ++geom;
  if ((p = strchr (geom, ';')) == NULL)
    p = strchr (geom, '\0');

  n = (p - geom);
  if (n < MAXLEN_GEOM)
    {
      char *ops;
      new_flags |= geometrySet;

      strncpy (str, geom, n);
      str[n] = '\0';
      if (str[0] == ':')
        ops = &str[0];
      else if (str[0] != 'x' && str[0] != 'X' && isalpha(str[0]))
        ops = &str[0];
      else
        {
          char *tmp;
          ops = strchr (str, ':');
          if (ops != NULL)
            {
              for (tmp = ops-1; tmp >= str && isspace(*tmp); --tmp);
              *(++tmp) = '\0';
              if (ops == tmp) ++ops;
            }
        }

      if (ops > str || ops == NULL)
        {
          /* we have geometry string - let's handle it prior to applying ops */
          geom_flags = XParseGeometry (str, &x, &y, &w, &h);

          if ((geom_flags & XValue) && !(geom_flags & YValue))
            {
              y = x;
              geom_flags |= YValue;
            }

          if (flags & geometrySet)
            {/* new geometry is an adjustment to the old one ! */
              if ((geom_flags & WidthValue) && (geom_flags & HeightValue))
                {
                  if (w == 0 && h != 0)
                    {
                      w = h_scale;
                      h = (v_scale * h) / 100;
                    }
                  else if (h == 0 && w != 0)
                    {
                      w = (h_scale * w) / 100;
                      h = v_scale;
                    }
                }
              if (geom_flags & XValue)
                {
                  if (str[0] != '=')
                    {
                      y += v_align;
                      x += h_align;
                    }
                }
            }
          else /* setting up geometry from scratch */
            {
              if (!(geom_flags & XValue))
                {/* use default geometry - centered */
                  x = y = defaultAlign;
                }
              else if (!(geom_flags & YValue))
                y = x;

              if ((geom_flags & (WidthValue|HeightValue)) == 0)
                {/* use default geometry - scaled */
                  w = h = defaultScale;
                }
              else if (geom_flags & WidthValue)
                {
                  if (!(geom_flags & HeightValue))
                    h = w;
                }
              else
                w = h;
            }
        } /* done parsing geometry string */
      else if (!(flags & geometrySet))
        { /* default geometry - scaled and centered */
          x = y = defaultAlign;
          w = h = defaultScale;
        }
        
      if (!(flags & geometrySet))
        geom_flags |= WidthValue|HeightValue|XValue|YValue;

      if (ops)
        {
          while (*ops)
            {
              while (*ops == ':' || isspace(*ops)) ++ops;
#  define CHECK_GEOM_OPS(op_str)  (strncasecmp (ops, (op_str), sizeof(op_str)-1) == 0)
              if (CHECK_GEOM_OPS("tile"))
                {
                  w = h = 0;
                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS("propscale"))
                {
                  if (w == 0 && h == 0)
                    {
                      w = 100;
                      geom_flags |= WidthValue;
                    }
                  new_flags |= propScale;
                }
              else if (CHECK_GEOM_OPS("hscale"))
                {
                  if (w == 0)
                    w = 100;
                  h = 0;
                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS("vscale"))
                {
                  if (h == 0)
                    h = 100;
                  w = 0;
                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS("scale"))
                {
                  if (h == 0)
                    h = 100;
                  if (w == 0)
                    w = 100;
                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS("auto"))
                {
                  w = h = 100;
                  x = y = 50;
                  geom_flags |= WidthValue|HeightValue|XValue|YValue;
                }
#  undef CHECK_GEOM_OPS
              while (*ops != ':' && *ops != '\0') ++ops;
            } /* done parsing ops */
        }

      if (check_set_scale_value (geom_flags, WidthValue, h_scale, w))
        ++changed;
      if (check_set_scale_value (geom_flags, HeightValue, v_scale, h))
        ++changed;
      if (check_set_align_value (geom_flags, XValue, h_align, x))
        ++changed;
      if (check_set_align_value (geom_flags, YValue, v_align, y))
        ++changed;
    }

  if (new_flags != flags)
    {
      flags = new_flags;
      changed++;
    }
//fprintf( stderr, "flags = %lX, scale = %ux%u, align=%+d%+d\n",
//         flags, h_scale, v_scale, h_align, v_align);
  return (changed > 0);
}

#  ifdef HAVE_AFTERIMAGE
bool
bgPixmap_t::render_asim (ASImage *background, ARGB32 background_tint)
{
  if (target == NULL)
    return false;

  int target_width = (int)target->szHint.width;
  int target_height = (int)target->szHint.height;
  int new_pmap_width = target_width, new_pmap_height = target_height;
  ASImage *result = NULL;

  int x = 0;
  int y = 0;
  int w = h_scale * target_width / 100;
  int h = v_scale * target_height / 100;

  if (original_asim)
    {
      x = make_align_position (h_align, target_width, w > 0 ? w : (int)original_asim->width);
      y = make_align_position (v_align, target_height, h > 0 ? h : (int)original_asim->height);
    }

  if (original_asim == NULL
      || x >= target_width
      || y >= target_height
      || (w > 0 && x + w <= 0)
      || (h > 0 && y + h <= 0))
    {
      if (background)
        {
          new_pmap_width = background->width;
          new_pmap_height = background->height;
          result = background;
          if (background_tint != TINT_LEAVE_SAME)
            {
              ASImage* tmp = tile_asimage (target->asv, background, 0, 0,
                                           target_width, target_height, background_tint,
                                           ASA_XImage, 100, ASIMAGE_QUALITY_DEFAULT);
              if (tmp)
                result = tmp;
            }
        }
      else
        new_pmap_width = new_pmap_height = 0;
    }
  else
    {
      result = original_asim;
      if ((w > 0 && w != original_asim->width)
          || (h > 0 && h != original_asim->height))
        {
          result = scale_asimage (target->asv, original_asim, 
                                  w > 0 ? w : original_asim->width, 
                                  h > 0 ? h : original_asim->height,
                                  background ? ASA_ASImage : ASA_XImage,
                                  100, ASIMAGE_QUALITY_DEFAULT);
        }
      if (background == NULL)
        {/* if tiling - pixmap has to be sized exactly as the image */
          if (h_scale == 0)
            new_pmap_width = result->width;
          if (v_scale == 0)
            new_pmap_height = result->height;
          /* we also need to tile our image in one or both directions */
          if (h_scale == 0 || v_scale == 0)
            {
              ASImage *tmp = tile_asimage (target->asv, result,
                                            (h_scale > 0) ? 0 : (int)result->width - x, 
                                            (v_scale > 0) ? 0 : (int)result->height - y, 
                                            result->width, result->height,
                                            TINT_LEAVE_SAME, ASA_XImage,
                                            100, ASIMAGE_QUALITY_DEFAULT);
              if (tmp)
                {
                  if (result != original_asim)
                    destroy_asimage (&result);
                  result = tmp;
                }
            }
        }
      else
        {/* if blending background and image - pixmap has to be sized same as target window */
          ASImageLayer *layers = create_image_layers (2);
          ASImage *merged_im = NULL;

          layers[0].im = background;
          layers[0].clip_width = target_width;
          layers[0].clip_height = target_height;
          layers[0].tint = background_tint;
          layers[1].im = result;
          if (w <= 0)
            {/* tile horizontally */
              while (x > 0) x -= (int)result->width;
              layers[1].dst_x = x;
              layers[1].clip_width = result->width+target_width;
            }
          else
            {/* clip horizontally */
              layers[1].dst_x = x;
              layers[1].clip_width = result->width;
            }
          if (h <= 0)
            {
              while (y > 0) y -= (int)result->height;
              layers[1].dst_y = y;
              layers[1].clip_height = result->height + target_height;
            }
          else
            {
              layers[1].dst_y = y;
              layers[1].clip_height = result->height;
            }
          if (target->rs[Rs_blendtype])
            {
              layers[1].merge_scanlines = blend_scanlines_name2func (target->rs[Rs_blendtype]);
              if (layers[1].merge_scanlines == NULL)
                layers[1].merge_scanlines = alphablend_scanlines;
            }
          ASImage *tmp = merge_layers (target->asv, layers, 2, target_width, target_height,
                                       ASA_XImage, 0, ASIMAGE_QUALITY_DEFAULT);
          if (tmp)
            {
              if (result != original_asim)
                destroy_asimage (&result);
              result = tmp;
            }
          free (layers);
        }
    }

  if (pixmap)
    {
      if (result == NULL
          || pmap_width != new_pmap_width
          || pmap_height != new_pmap_height
          || pmap_depth != target->depth)
        {
          XFreePixmap (target->dpy, pixmap);
          pixmap = None;
        }
    }

  if (result)
    {
      XGCValues gcv;
      GC gc;

      /* create Pixmap */
      if (pixmap == None)
        {
          pixmap = XCreatePixmap (target->dpy, target->vt, new_pmap_width, new_pmap_height, target->depth);
          pmap_width = new_pmap_width;
          pmap_height = new_pmap_height;
          pmap_depth = target->depth;
        }
      /* fill with background color ( if result's not completely overlapping it)*/
      gcv.foreground = target->pix_colors[Color_bg];
      gc = XCreateGC (target->dpy, target->vt, GCForeground, &gcv);

      int src_x = 0, src_y = 0, dst_x = 0, dst_y = 0;
      int dst_width = result->width, dst_height = result->height;
      if (background == NULL)
        {
          if (h_scale > 0)
            src_x = make_clip_rectangle (x, result->width, new_pmap_width, dst_x, dst_width);
          if (v_scale > 0)
            src_y = make_clip_rectangle (y, result->height, new_pmap_height, dst_y, dst_height);

          if (dst_x > 0 || dst_y > 0
              || dst_x + dst_width < new_pmap_width
              || dst_y + dst_height < new_pmap_height)
            {
              XFillRectangle (target->dpy, pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);
            }
        }

      /* put result on pixmap */
      if (dst_x < new_pmap_width && dst_y < new_pmap_height)
        asimage2drawable (target->asv, pixmap, result, gc, src_x, src_y, dst_x, dst_y, dst_width, dst_height, True);
    
      if (result != background && result != original_asim)
        destroy_asimage (&result);

      XFreeGC (target->dpy, gc);
    }

  return true;
}
#  endif /* HAVE_AFTERIMAGE */

bool
bgPixmap_t::set_file (const char *file)
{
  char *f;

  assert (file != NULL);

  if (*file != '\0')
    {
#  ifdef HAVE_AFTERIMAGE
      if (target->asimman == NULL)
        target->asimman = create_generic_imageman(target->rs[Rs_path]);
      if ((f = strchr (file, ';')) == NULL)
        original_asim = get_asimage( target->asimman, file, 0xFFFFFFFF, 100 );
      else
        {
          size_t len = f - file;
          f = (char *)malloc (len + 1);
          strncpy (f, file, len);
          f[len] = '\0';
          original_asim = get_asimage( target->asimman, f, 0xFFFFFFFF, 100 );
          free( f );
        }
      return (original_asim != NULL);
#  endif    
    }
  return false;
}

# endif	/* XPM_BACKGROUND */

# ifdef ENABLE_TRANSPARENCY
bool 
bgPixmap_t::set_transparent ()
{
  if (!(flags & isTransparent))
    {
      flags |= isTransparent;
      return true;
    }
}

bool
bgPixmap_t::set_blur_radius (const char *geom)
{
  int changed = 0;
  unsigned int hr, vr;
  int junk;
  int geom_flags = XParseGeometry (geom, &junk, &junk, &hr, &vr);
  if (!(geom_flags&WidthValue))
    hr = 1;
  if (!(geom_flags&HeightValue))
    vr = hr;

  if (h_blurRadius != hr)
    {
      ++changed;
      h_blurRadius = hr;
    }
  if (v_blurRadius != vr)
    {
      ++changed;
      v_blurRadius = vr;
    }
  return (changed>0);
}

static inline unsigned long
compute_tint_shade_flags (rxvt_color *tint, int shade)
{
  unsigned long flags = 0;

  if (shade > 0 && shade <100)
    flags |= bgPixmap_t::tintNeeded;
  else if (tint)
    {
      rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
      tint->get (c);

      flags |= bgPixmap_t::tintNeeded;
      if ((c.r > 0x000700 || c.g > 0x000700 || c.b > 0x000700)
          && (c.r < 0x00f700 || c.g < 0x00f700 || c.b < 0x00f700))
      {
         flags |= bgPixmap_t::tintNeeded;
#define IS_COMPONENT_WHOLESOME(cmp)  ((cmp) <= 0x000700 || (cmp) >= 0x00f700)
          if (IS_COMPONENT_WHOLESOME (c.r)
              && IS_COMPONENT_WHOLESOME (c.g)
              && IS_COMPONENT_WHOLESOME (c.b))
            flags |= bgPixmap_t::tintServerSide;
#undef  IS_COMPONENT_WHOLESOME
      }
    }
  return flags;
}

bool
bgPixmap_t::set_tint (rxvt_color &new_tint)
{
  if (tint != new_tint)
    {
      unsigned long new_flags = compute_tint_shade_flags (&new_tint, shade);
      tint = new_tint;
      flags = (flags & ~tintFlags) | new_flags | tintSet;
      return true;
    }
  return false;
}

bool
bgPixmap_t::unset_tint ()
{
  unsigned long new_flags = compute_tint_shade_flags (NULL, shade);

  if (new_flags != (flags & tintFlags))
    {
     flags = (flags&~tintFlags)|new_flags;
     return true;
    }
  return false;
}

bool
bgPixmap_t::set_shade (const char *shade_str)
{
  int new_shade = (shade_str) ? atoi (shade_str) : 0;

  if (new_shade == 100)
    new_shade = 0;

  if (new_shade != shade)
    {
      unsigned long new_flags = compute_tint_shade_flags (&tint, new_shade);
      shade = new_shade;
      flags = (flags & ~tintFlags) | new_flags;
      return true;
    }
  return false;
}


/* make_transparency_pixmap() 
 * Builds a pixmap sized the same as terminal window, with depth same as the root window
 * that pixmap contains tiled portion of the root pixmap that is supposed to be covered by 
 * our window.
 */
unsigned long
bgPixmap_t::make_transparency_pixmap ()
{
  unsigned long result = 0;

  if (target == NULL)
    return 0;

  /* root dimentions may change from call to call - but Display structure should
   * be always up-to-date, so let's use it :
   */
  Window root = target->display->root;
  int screen = target->display->screen;
  Display *dpy = target->dpy;
  int root_width = DisplayWidth (dpy, screen);
  int root_height = DisplayHeight (dpy, screen);
  unsigned int root_pmap_width, root_pmap_height;
  int window_width = target->szHint.width;
  int window_height = target->szHint.height;
  int sx, sy;
  XGCValues gcv;

  target->get_window_origin (sx, sy);

  /* check if we are outside of the visible part of the virtual screen : */
  if (sx + window_width <= 0 || sy + window_height <= 0
      || sx >= root_width || sy >= root_height)
    return 0;

  if (root_pixmap != None)
    {/* we want to validate the pixmap and get it's size at the same time : */
      int junk;
      unsigned int ujunk;
      /* root pixmap may be bad - allow a error */
      target->allowedxerror = -1;
      if (!XGetGeometry (dpy, root_pixmap, &root, &junk, &junk, &root_pmap_width, &root_pmap_height, &ujunk, &ujunk))
        root_pixmap = None;
      target->allowedxerror = 0;
    }

  Pixmap tiled_root_pmap = XCreatePixmap (dpy, root, window_width, window_height, root_depth);
  GC gc = NULL;

  if (tiled_root_pmap == None) /* something really bad happened - abort */
    return 0;

  if (root_pixmap == None)
    { /* use tricks to obtain the root background image :*/
      /* we want to create Overrideredirect window overlapping out window
         with background type of Parent Relative and then grab it */
      XSetWindowAttributes attr;
      Window src;
      bool success = false;

      attr.background_pixmap = ParentRelative;
      attr.backing_store = Always;
      attr.event_mask = ExposureMask;
      attr.override_redirect = True;
  	  src = XCreateWindow (dpy, root, sx, sy, window_width, window_height, 0,
                           CopyFromParent, CopyFromParent, CopyFromParent,
                           CWBackPixmap|CWBackingStore|CWOverrideRedirect|CWEventMask,
                           &attr);

      if (src != None)
        {
          XEvent event;
          int ev_count = 0;
          XGrabServer (dpy);
          XMapRaised (dpy, src);
          XSync (dpy, False);
          /* XSync should get window where it's properly exposed,
           * but to be on the safe side - let's check for the actuall event to arrive : */
          while (XCheckWindowEvent (dpy, src, ExposureMask, &event))
            ++ev_count;
          if (ev_count > 0);
            { /* hooray! - we can grab the image! */
              gc = XCreateGC (dpy, root, 0, NULL);
              if (gc)
                {
                  XCopyArea (dpy, src, tiled_root_pmap, gc, 0, 0, window_width, window_height, 0, 0);
                  success = true;
                }
            }
          XDestroyWindow (dpy, src);
          XUngrabServer (dpy);
          //fprintf (stderr, "%s:%d: ev_count = %d\n", __FUNCTION__, __LINE__, ev_count);
        }
        if (!success)
          {
            XFreePixmap (dpy, tiled_root_pmap);
            tiled_root_pmap = None;
          }
        else
          result |= transpPmapTiled;
    }
  else
    {/* strightforward pixmap copy */
      gcv.tile = root_pixmap;
      gcv.fill_style = FillTiled;
      while (sx < 0) sx += (int)window_width;
      while (sy < 0) sy += (int)window_height;
      gcv.ts_x_origin = -sx;
      gcv.ts_y_origin = -sy;
      gc = XCreateGC (dpy, root, GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gcv);
      if (gc)
        {
          XFillRectangle (dpy, tiled_root_pmap, gc, 0, 0, window_width, window_height);
          result |= transpPmapTiled;
        }
    }

    if (tiled_root_pmap != None)
      {
        if (flags & tintNeeded) 
          {
            if ((flags & tintServerSide)
                && h_blurRadius <= 1  && v_blurRadius <= 1
# ifdef HAVE_AFTERIMAGE
                && original_asim == NULL
# endif
               )
              { /* In this case we can tint image server-side getting significant
                 * performance improvements, as we eliminate XImage transfer
                 */
                gcv.foreground = Pixel (tint);
                gcv.function = GXand;
                gcv.fill_style = FillSolid;
                if (gc)
                  XChangeGC (dpy, gc, GCFillStyle | GCForeground | GCFunction, &gcv);
                else
                  gc = XCreateGC (dpy, root, GCFillStyle | GCForeground | GCFunction, &gcv);
                if (gc)
                  {
                    XFillRectangle (dpy, tiled_root_pmap, gc, 0, 0, window_width, window_height);
                    result |= transpPmapTinted;
                  }
              }
           }
        if (pixmap)
          XFreePixmap (dpy, pixmap);
        pixmap = tiled_root_pmap;
        pmap_width = window_width;
        pmap_height = window_height;
        pmap_depth = root_depth;
      }
      
    if (gc)
      XFreeGC (dpy, gc);
    
    return result;    
}

bool
bgPixmap_t::set_root_pixmap ()
{
  Pixmap new_root_pixmap = None;
  
  new_root_pixmap = target->get_pixmap_property (XA_XROOTPMAP_ID);
  if (new_root_pixmap == None)
    new_root_pixmap = target->get_pixmap_property (XA_ESETROOT_PMAP_ID);

  if (new_root_pixmap != root_pixmap)
    {
      root_pixmap = new_root_pixmap;
      return true;
    }
  return false;
}
# endif /* ENABLE_TRANSPARENCY */

# ifndef HAVE_AFTERIMAGE
static void ShadeXImage(rxvt_term *term, XImage* srcImage, int shade, int rm, int gm, int bm);
#endif


bool
bgPixmap_t::render ()
{
  unsigned long background_flags = 0;

  if (target == NULL)
    return false;

  invalidate();
# ifdef ENABLE_TRANSPARENCY
  if (flags & isTransparent)
    {
      /*  we need to re-generate transparency pixmap in that case ! */
      background_flags = make_transparency_pixmap ();
      if (background_flags == 0)
        return false;
      else if ((background_flags & transpTransformations) == (flags & transpTransformations)
                && pmap_depth == target->depth)
        flags = flags & ~isInvalid;
    }
# endif

  XImage *result = NULL;
# ifdef HAVE_AFTERIMAGE
  if (original_asim
      || (background_flags & transpTransformations) != (flags & transpTransformations))
    {
      ASImage *background = NULL;
      ARGB32 as_tint = TINT_LEAVE_SAME;
      if (background_flags)
          background = pixmap2ximage (target->asv, pixmap, 0, 0, pmap_width, pmap_height, AllPlanes, 100);

      if (!(background_flags & transpPmapTinted) && (flags & tintNeeded))
        {
          ShadingInfo as_shade;
          as_shade.shading = (shade == 0) ? 100 : shade;

          rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
          tint.get (c);
          as_shade.tintColor.red = c.r;
          as_shade.tintColor.green = c.g;
          as_shade.tintColor.blue = c.b;

          as_tint = shading2tint32 (&as_shade);
        }
      if (render_asim (background, as_tint))
        flags = flags & ~isInvalid;
      if (background)
          destroy_asimage (&background);
    }
  else if (background_flags && pmap_depth != target->depth)
    {
      result = XGetImage (target->dpy, pixmap, 0, 0, pmap_width, pmap_height, AllPlanes, ZPixmap);
    }
# else /* our own client-side tinting */
  if (background_flags && (flags & isInvalid))
    {
      result = XGetImage (target->dpy, pixmap, 0, 0, pmap_width, pmap_height, AllPlanes, ZPixmap);
      if (result != NULL && !(background_flags & transpPmapTinted) && (flags & tintNeeded))
        {
          rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
          tint.get (c);
          ShadeXImage (target, result, shade, c.r, c.g, c.b);
        }
    }
# endif
  if (result != NULL)
    {
      GC gc = XCreateGC (target->dpy, target->vt, 0UL, NULL);
      if (gc)
        {
          if (pmap_depth != target->depth && pixmap != None)
            {
              XFreePixmap (target->dpy, pixmap);
              pixmap = None;
            }
          if (pixmap == None)
            {
              pixmap = XCreatePixmap (target->dpy, target->vt, result->width, result->height, target->depth);
              pmap_width = result->width;
              pmap_height = result->height;
              pmap_depth = target->depth;
            }
          XPutImage (target->dpy, pixmap, gc, result, 0, 0, 0, 0, result->width, result->height);
          XFreeGC (target->dpy, gc);
          flags = flags & ~isInvalid;
        }
        XDestroyImage (result);
    }

  if (flags & isInvalid)
    {
      if (pixmap != None)
        {
          XFreePixmap (target->dpy, pixmap);
          pixmap = None;
        }
// TODO : we need to get rid of that garbadge :
      target->am_transparent = target->am_pixmap_trans = 0;
    }
  else
    target->am_transparent = target->am_pixmap_trans = 1;

  apply ();

  return true;      
}

bool 
bgPixmap_t::set_target (rxvt_term *new_target)
{
  if (new_target)
    if (target != new_target)
      {
        target = new_target;
# ifdef ENABLE_TRANSPARENCY
        root_depth = DefaultDepthOfScreen (ScreenOfDisplay (target->dpy, target->display->screen));
# endif
        return true;
      }
  return false;
}

void
bgPixmap_t::apply()
{
  if (target)
    {
      if (pixmap != None)
        { /* set target's background to pixmap */
# ifdef ENABLE_TRANSPARENCY
          if (flags & isTransparent)
            {
              XSetWindowBackgroundPixmap (target->dpy, target->parent[0], pixmap);
              XSetWindowBackgroundPixmap (target->dpy, target->vt, ParentRelative);
#  if HAVE_SCROLLBARS
              if (target->scrollBar.win)
                XSetWindowBackgroundPixmap (target->dpy, target->scrollBar.win, ParentRelative);
#  endif
            }
          else
# endif
            {
              /* force old pixmap dereference in case it was transparent before :*/
              XSetWindowBackground (target->dpy, target->parent[0], target->pix_colors[Color_border]);
              XSetWindowBackgroundPixmap (target->dpy, target->vt, pixmap);
              /* do we also need to set scrollbar's background here ? */
# if HAVE_SCROLLBARS
              if (target->scrollBar.win)
                  XSetWindowBackground (target->dpy, target->scrollBar.win, target->pix_colors[Color_border]);
# endif
            }
        }
      else
        { /* set target background to a pixel */
          XSetWindowBackground (target->dpy, target->parent[0], target->pix_colors[Color_border]);
          XSetWindowBackground (target->dpy, target->vt, target->pix_colors[Color_bg]);
          /* do we also need to set scrollbar's background here ? */
# if HAVE_SCROLLBARS
          if (target->scrollBar.win)
              XSetWindowBackground (target->dpy, target->scrollBar.win, target->pix_colors[Color_border]);
# endif
        }
      /* don't want Expose on the parent */
      XClearArea (target->dpy, target->parent[0], 0, 0, 0, 0, False);
      /* do want Expose on the vt */
      XClearArea (target->dpy, target->parent[0], 0, 0, 0, 0, True);
#if HAVE_SCROLLBARS
      if (target->scrollBar.win)
        {
          target->scrollBar.setIdle ();
          target->scrollbar_show (0);
        }
#endif
      target->want_refresh = target->want_full_refresh = 1;
      target->flush ();
    }
}
#endif				/* HAVE_BG_PIXMAP */


void
rxvt_term::get_window_origin (int &x, int &y)
{
  Window cr;
  XTranslateCoordinates (dpy, parent[0], display->root, 0, 0, &x, &y, &cr);
}

Pixmap
rxvt_term::get_pixmap_property (int prop_id)
{
  if (prop_id > 0 && prop_id < NUM_XA)
    if (xa[prop_id])
      {
        int aformat, rootdepth;
        unsigned long nitems, bytes_after;
        Atom atype;
        unsigned char *prop = NULL;
        int result = XGetWindowProperty (dpy, display->root, xa[prop_id],
                                         0L, 1L, False, XA_PIXMAP, &atype, &aformat,
                                         &nitems, &bytes_after, &prop);
        if (result == Success && prop && atype == XA_PIXMAP)
          {
            return *(Pixmap *)prop;
          }
      }
  return None;
}


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

  if (visual->c_class != TrueColor || srcImage->format != ZPixmap) return ;

  if (shade == 0)
    shade = 100;
    
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
#if 0  /* replaced by a bgPixmap_t::render() - leve here temporarily for reference */

  int i, aformat, rootdepth;
  unsigned long nitems, bytes_after;
  Atom atype;
  unsigned char *prop = NULL;
  Window root, oldp, *list;
  Pixmap rootpixmap = None;
  XWindowAttributes wattr, wrootattr;
  int sx, sy;
  Window cr;
  unsigned int rootpixmap_w = 0, rootpixmap_h = 0;

  if (!option (Opt_transparent))
    return;	/* Don't try any more */
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
          XSetWindowBackground (dpy, vt, pix_colors_focused[Color_bg]);
          am_transparent = am_pixmap_trans = 0;
        }

      return;	/* Don't try any more */
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

  if (!i || prop == NULL)
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

      bgPixmap.make_transparency_pixmap();

      if (whole_tint && !no_tint)
        {
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
              /* TODO: temporary fix - redo the logic, so that same function can do both
                 transparency and non-transparency */
              bgPixmap.render_asim (back_im, tint);
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
      
      bgPixmap.apply();
      if (!success)
          am_pixmap_trans = 0;
      else
        {
          am_transparent = am_pixmap_trans = 1;
        }
    } /* rootpixmap != None */

  if (am_pixmap_trans)
    {
      if (scrollBar.win)
        {
          scrollBar.setIdle ();
          scrollbar_show (0);
        }

      if (am_transparent)
        {
          want_refresh = want_full_refresh = 1;
          if (am_pixmap_trans)
            flush ();
        }
    }
#endif    
}
#endif
