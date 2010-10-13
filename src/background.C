/*----------------------------------------------------------------------*
 * File:	background.C - former xpm.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2008 Marc Lehmann <pcg@goof.com>
 * Copyright (c) 2007      Sasha Vasko <sasha@aftercode.net>
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

#include <cmath>
#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */

#define DO_TIMING_TEST 0

#if DO_TIMING_TEST
# include <sys/time.h>
#define TIMING_TEST_START(id) \
  struct timeval timing_test_##id##_stv; \
  gettimeofday (&timing_test_##id##_stv, NULL);

#define TIMING_TEST_PRINT_RESULT(id) \
  do { \
    struct timeval tv; \
    gettimeofday (&tv, NULL); \
    tv.tv_sec -= (timing_test_##id##_stv).tv_sec; \
    fprintf (stderr, "%s: %s: %d: elapsed  %ld usec\n", #id, __FILE__, __LINE__, \
             tv.tv_sec * 1000000 + tv.tv_usec - (timing_test_##id##_stv).tv_usec); \
  } while (0)

#else
#define TIMING_TEST_START(id) do {} while (0)
#define TIMING_TEST_PRINT_RESULT(id) do {} while (0)
#endif

/*
 * Pixmap geometry string interpretation :
 * Each geometry string contains zero or one scale/position
 * adjustment and may optionally be followed by a colon and one or more
 * colon-delimited pixmap operations.
 * The following table shows the valid geometry strings and their
 * effects on the background image :
 *
 * WxH+X+Y    Set scaling to W% by H%, and position to X% by Y%.
 *            W and H are percentages of the terminal window size.
 *            X and Y are also percentages; e.g., +50+50 centers
 *            the image in the window.
 * WxH+X      Assumes Y == X
 * WxH        Assumes Y == X == 50 (centers the image)
 * W+X+Y      Assumes H == W
 * W+X        Assumes H == W and Y == X
 * W          Assumes H == W and Y == X == 50
 *
 * Adjusting position only :
 * =+X+Y      Set position to X% by Y% (absolute).
 * =+X        Set position to X% by X%.
 * +X+Y       Adjust position horizontally X% and vertically Y%
 *            from current position (relative).
 * +X         Adjust position horizontally X% and vertically X%
 *            from current position.
 *
 * Adjusting scale only :
 * Wx0        Multiply horizontal scaling factor by W%
 * 0xH        Multiply vertical scaling factor by H%
 * 0x0        No scaling (show image at normal size).
 *
 * Pixmap Operations : (should be prepended by a colon)
 * tile       Tile image. Scaling/position modifiers above will affect
 *            the tile size and origin.
 * propscale  When scaling, scale proportionally. That is, maintain the
 *            proper aspect ratio for the image. Any portion of the
 *            background not covered by the image is filled with the
 *            current background color.
 * hscale     Scale horizontally, tile vertically ?
 * vscale     Tile horizontally, scale vertically ?
 * scale      Scale both up and down
 * auto       Same as 100x100+50+50
 */

#ifdef HAVE_BG_PIXMAP
bgPixmap_t::bgPixmap_t ()
{
  // this is basically redundant as bgPixmap_t is only used in
  // zero_initialised-derived structs
#ifdef HAVE_AFTERIMAGE
  original_asim = NULL;
#endif
#ifdef HAVE_PIXBUF
  pixbuf = NULL;
#endif
#ifdef BG_IMAGE_FROM_FILE
  have_image = false;
  h_scale = v_scale = 0;
  h_align = v_align = 0;
#endif
#ifdef ENABLE_TRANSPARENCY
  shade = 100;
#endif
  flags = 0;
  pixmap = None;
  valid_since = invalid_since = 0;
  target = 0;
}

void
bgPixmap_t::destroy ()
{
#ifdef HAVE_AFTERIMAGE
  if (original_asim)
    safe_asimage_destroy (original_asim);
#endif

#ifdef HAVE_PIXBUF
  if (pixbuf)
    g_object_unref (pixbuf);
#endif

  if (pixmap && target)
    XFreePixmap (target->dpy, pixmap);
}

bool
bgPixmap_t::window_size_sensitive ()
{
# ifdef ENABLE_TRANSPARENCY
  if (flags & isTransparent)
    return true;
# endif

# ifdef BG_IMAGE_FROM_FILE
  if (have_image)
    {
      if (flags & sizeSensitive)
        return true;
    }
# endif

  return false;
}

bool
bgPixmap_t::window_position_sensitive ()
{
# ifdef ENABLE_TRANSPARENCY
  if (flags & isTransparent)
    return true;
# endif

# ifdef BG_IMAGE_FROM_FILE
  if (have_image)
    {
      if (flags & rootAlign)
        return true;
    }
# endif

  return false;
};

bool bgPixmap_t::need_client_side_rendering ()
{
# ifdef HAVE_AFTERIMAGE
  if (original_asim)
    return true;
# endif
# ifdef ENABLE_TRANSPARENCY
  if (flags & isTransparent)
    {
#  ifdef HAVE_AFTERIMAGE
      if ((flags & blurNeeded) && !(flags & blurServerSide))
        return true;
#  endif
      if ((flags & tintNeeded) && !(flags & tintServerSide))
        return true;
    }
# endif
  return false;
}

# ifdef BG_IMAGE_FROM_FILE
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
  int smaller = min (image_size, window_size);

  if (align >= 0 && align <= 100)
    return diff * align / 100;
  else if (align > 100 && align <= 200 )
    return ((align - 100) * smaller / 100) + window_size - smaller;
  else if (align >= -100 && align < 0)
    return ((align + 100) * smaller / 100) - image_size;
  return 0;
}

static inline int
make_clip_rectangle (int pos, int size, int target_size, int &dst_pos, int &dst_size)
{
  int src_pos = 0;
  dst_pos = pos;
  dst_size = size;
  if (pos < 0)
    {
      src_pos = -pos;
      dst_pos = 0;
      dst_size += pos;
    }

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
  const char *p;
#  define MAXLEN_GEOM		256 /* could be longer than regular geometry string */

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

      memcpy (str, geom, n);
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
            {
              /* new geometry is an adjustment to the old one ! */
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
                {
                  /* use default geometry - centered */
                  x = y = defaultAlign;
                }
              else if (!(geom_flags & YValue))
                y = x;

              if ((geom_flags & (WidthValue|HeightValue)) == 0)
                {
                  /* use default geometry - scaled */
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
        {
          /* default geometry - scaled and centered */
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

#  define CHECK_GEOM_OPS(op_str)  (strncasecmp (ops, (op_str), sizeof (op_str) - 1) == 0)
              if (CHECK_GEOM_OPS ("tile"))
                {
                  w = h = noScale;
                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS ("propscale"))
                {
                  new_flags |= propScale;
                }
              else if (CHECK_GEOM_OPS ("hscale"))
                {
                  if (w == 0) w = windowScale;

                  h = noScale;
                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS ("vscale"))
                {
                  if (h == 0) h = windowScale;

                  w = noScale;
                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS ("scale"))
                {
                  if (h == 0) h = windowScale;
                  if (w == 0) w = windowScale;

                  geom_flags |= WidthValue|HeightValue;
                }
              else if (CHECK_GEOM_OPS ("auto"))
                {
                  w = h = windowScale;
                  x = y = centerAlign;
                  geom_flags |= WidthValue|HeightValue|XValue|YValue;
                }
              else if (CHECK_GEOM_OPS ("root"))
                {
                  new_flags |= rootAlign;
                  w = h = noScale;
                  geom_flags |= WidthValue|HeightValue;
                }
#  undef CHECK_GEOM_OPS

              while (*ops != ':' && *ops != '\0') ++ops;
            } /* done parsing ops */
        }

      if (check_set_scale_value (geom_flags, WidthValue, h_scale, w))  ++changed;
      if (check_set_scale_value (geom_flags, HeightValue, v_scale, h)) ++changed;
      if (check_set_align_value (geom_flags, XValue, h_align, x))      ++changed;
      if (check_set_align_value (geom_flags, YValue, v_align, y))      ++changed;
    }

  if (new_flags != flags)
    {
      flags = new_flags;
      changed++;
    }

  //fprintf (stderr, "flags = %lX, scale = %ux%u, align=%+d%+d\n",
  //         flags, h_scale, v_scale, h_align, v_align);
  return (changed > 0);
}

void
bgPixmap_t::get_image_geometry (int image_width, int image_height, int &w, int &h, int &x, int &y)
{
  int target_width = target->szHint.width;
  int target_height = target->szHint.height;

  if (flags & propScale)
    {
      float scale = (float)target_width / image_width;
      min_it (scale, (float)target_height / image_height);
      w = image_width * scale + 0.5;
      h = image_height * scale + 0.5;
    }
  else
    {
      w = h_scale * target_width / 100;
      h = v_scale * target_height / 100;
    }

  if (!w) w = image_width;
  if (!h) h = image_height;

  if (flags & rootAlign)
    {
      target->get_window_origin (x, y);
      x = -x;
      y = -y;
    }
  else
    {
      x = make_align_position (h_align, target_width, w);
      y = make_align_position (v_align, target_height, h);
    }

  flags &= ~sizeSensitive;
  if (h_scale != 0 || v_scale != 0
      || h_align != 0 || v_align != 0
      || w > target_width || h > target_height)
    flags |= sizeSensitive;
}

#  ifdef HAVE_AFTERIMAGE
bool
bgPixmap_t::render_image (unsigned long background_flags)
{
  if (target == NULL)
    return false;

  target->init_asv ();

  ASImage *background = NULL;
  ARGB32 background_tint = TINT_LEAVE_SAME;

#   ifdef ENABLE_TRANSPARENCY
  if (background_flags)
    background = pixmap2ximage (target->asv, pixmap, 0, 0, pmap_width, pmap_height, AllPlanes, 100);

  if (!(background_flags & transpPmapTinted) && (flags & tintNeeded))
    {
      ShadingInfo as_shade;
      as_shade.shading = shade;

      rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
      if (flags & tintSet)
        tint.get (c);
      as_shade.tintColor.red = c.r;
      as_shade.tintColor.green = c.g;
      as_shade.tintColor.blue = c.b;

      background_tint = shading2tint32 (&as_shade);
    }

  if (!(background_flags & transpPmapBlurred) && (flags & blurNeeded) && background != NULL)
    {
      ASImage *tmp = blur_asimage_gauss (target->asv, background, h_blurRadius, v_blurRadius, 0xFFFFFFFF,
                                         (original_asim == NULL || tint == TINT_LEAVE_SAME) ? ASA_XImage : ASA_ASImage,
                                         100, ASIMAGE_QUALITY_DEFAULT);
      if (tmp)
        {
          destroy_asimage (&background);
          background = tmp;
        }
    }
#   endif

  ASImage *result = 0;

  int target_width    = target->szHint.width;
  int target_height   = target->szHint.height;
  int new_pmap_width  = target_width;
  int new_pmap_height = target_height;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  TIMING_TEST_START (asim);

  if (original_asim)
    get_image_geometry (original_asim->width, original_asim->height, w, h, x, y);

  if (!original_asim
      || (!(flags & rootAlign)
          && (x >= target_width
              || y >= target_height
              || (x + w <= 0)
              || (y + h <= 0))))
    {
      if (background)
        {
          new_pmap_width = background->width;
          new_pmap_height = background->height;
          result = background;

          if (background_tint != TINT_LEAVE_SAME)
            {
              ASImage *tmp = tile_asimage (target->asv, background, 0, 0,
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

      if ((w != original_asim->width)
          || (h != original_asim->height))
        {
          result = scale_asimage (target->asv, original_asim,
                                  w, h,
                                  background ? ASA_ASImage : ASA_XImage,
                                  100, ASIMAGE_QUALITY_DEFAULT);
        }

      if (background == NULL)
        {
          if (h_scale == 0 || v_scale == 0)
            {
              /* if tiling - pixmap has to be sized exactly as the image,
                 but there is no need to make it bigger than the window! */
              new_pmap_width = min (result->width, target_width);
              new_pmap_height = min (result->height, target_height);

              /* we also need to tile our image in both directions */
              ASImage *tmp = tile_asimage (target->asv, result,
                                           (int)result->width - x,
                                           (int)result->height - y,
                                           new_pmap_width,
                                           new_pmap_height,
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
        {
          /* if blending background and image - pixmap has to be sized same as target window */
          ASImageLayer *layers = create_image_layers (2);

          layers[0].im = background;
          layers[0].clip_width = target_width;
          layers[0].clip_height = target_height;
          layers[0].tint = background_tint;
          layers[1].im = result;

          if (h_scale == 0 || v_scale == 0)
            {
              /* tile horizontally */
              while (x > 0) x -= (int)result->width;
              layers[1].dst_x = x;
              layers[1].clip_width = result->width+target_width;
            }
          else
            {
              /* clip horizontally */
              layers[1].dst_x = x;
              layers[1].clip_width = result->width;
            }

          if (h_scale == 0 || v_scale == 0)
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
  TIMING_TEST_PRINT_RESULT (asim);

  bool ret = false;

  if (result)
    {
      XGCValues gcv;
      GC gc;

      if (pixmap)
        {
          if (pmap_width != new_pmap_width
              || pmap_height != new_pmap_height
              || pmap_depth != target->depth)
            {
              XFreePixmap (target->dpy, pixmap);
              pixmap = None;
            }
        }

      /* create Pixmap */
      if (pixmap == None)
        {
          pixmap = XCreatePixmap (target->dpy, target->vt, new_pmap_width, new_pmap_height, target->depth);
          pmap_width = new_pmap_width;
          pmap_height = new_pmap_height;
          pmap_depth = target->depth;
        }
      /* fill with background color (if result's not completely overlapping it) */
      gcv.foreground = target->pix_colors[Color_bg];
      gc = XCreateGC (target->dpy, target->vt, GCForeground, &gcv);

      int src_x = 0, src_y = 0, dst_x = 0, dst_y = 0;
      int dst_width = result->width, dst_height = result->height;
      if (background == NULL)
        {
          if (!(h_scale == 0 || v_scale == 0))
            {
              src_x = make_clip_rectangle (x, result->width , new_pmap_width , dst_x, dst_width );
              src_y = make_clip_rectangle (y, result->height, new_pmap_height, dst_y, dst_height);
            }

          if (dst_x > 0 || dst_y > 0
              || dst_x + dst_width < new_pmap_width
              || dst_y + dst_height < new_pmap_height)
            XFillRectangle (target->dpy, pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);
        }

      /* put result on pixmap */
      if (dst_x < new_pmap_width && dst_y < new_pmap_height)
        asimage2drawable (target->asv, pixmap, result, gc, src_x, src_y, dst_x, dst_y, dst_width, dst_height, True);

      if (result != background && result != original_asim)
        destroy_asimage (&result);

      XFreeGC (target->dpy, gc);
      TIMING_TEST_PRINT_RESULT (asim);

      ret = true;
    }

  if (background)
    destroy_asimage (&background);

  return ret;
}
#  endif /* HAVE_AFTERIMAGE */

#  ifdef HAVE_PIXBUF
bool
bgPixmap_t::render_image (unsigned long background_flags)
{
  if (target == NULL)
    return false;

  if (!pixbuf)
    return false;

#if !XFT
  if (background_flags)
    return false;
#endif

  GdkPixbuf *result;

  int image_width = gdk_pixbuf_get_width (pixbuf);
  int image_height = gdk_pixbuf_get_height (pixbuf);

  int target_width    = target->szHint.width;
  int target_height   = target->szHint.height;
  int new_pmap_width  = target_width;
  int new_pmap_height = target_height;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  get_image_geometry (image_width, image_height, w, h, x, y);

  if (!(flags & rootAlign)
      && (x >= target_width
          || y >= target_height
          || (x + w <= 0)
          || (y + h <= 0)))
    return false;

  result = pixbuf;

  if ((w != image_width)
      || (h != image_height))
    {
      result = gdk_pixbuf_scale_simple (pixbuf,
                                        w, h,
                                        GDK_INTERP_BILINEAR);
    }

  bool ret = false;

  if (result)
    {
      XGCValues gcv;
      GC gc;
      Pixmap root_pmap;

      image_width = gdk_pixbuf_get_width (result);
      image_height = gdk_pixbuf_get_height (result);

      if (background_flags)
        {
          root_pmap = pixmap;
          pixmap = None;
        }
      else
        {
          if (h_scale == 0 || v_scale == 0)
            {
              new_pmap_width = min (image_width, target_width);
              new_pmap_height = min (image_height, target_height);
            }
        }

      if (pixmap)
        {
          if (pmap_width != new_pmap_width
              || pmap_height != new_pmap_height
              || pmap_depth != target->depth)
            {
              XFreePixmap (target->dpy, pixmap);
              pixmap = None;
            }
        }

      if (pixmap == None)
        {
          pixmap = XCreatePixmap (target->dpy, target->vt, new_pmap_width, new_pmap_height, target->depth);
          pmap_width = new_pmap_width;
          pmap_height = new_pmap_height;
          pmap_depth = target->depth;
        }

      gcv.foreground = target->pix_colors[Color_bg];
      gc = XCreateGC (target->dpy, target->vt, GCForeground, &gcv);

      if (h_scale == 0 || v_scale == 0)
        {
          Pixmap tile = XCreatePixmap (target->dpy, target->vt, image_width, image_height, target->depth);
          gdk_pixbuf_xlib_render_to_drawable (result, tile, gc,
                                              0, 0,
                                              0, 0,
                                              image_width, image_height,
                                              XLIB_RGB_DITHER_NONE,
                                              0, 0);

          gcv.tile = tile;
          gcv.fill_style = FillTiled;
          gcv.ts_x_origin = x;
          gcv.ts_y_origin = y;
          XChangeGC (target->dpy, gc, GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gcv);

          XFillRectangle (target->dpy, pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);
          XFreePixmap (target->dpy, tile);
        }
      else
        {
          int src_x, src_y, dst_x, dst_y;
          int dst_width, dst_height;

          src_x = make_clip_rectangle (x, image_width , new_pmap_width , dst_x, dst_width );
          src_y = make_clip_rectangle (y, image_height, new_pmap_height, dst_y, dst_height);

          if (dst_x > 0 || dst_y > 0
              || dst_x + dst_width < new_pmap_width
              || dst_y + dst_height < new_pmap_height)
            XFillRectangle (target->dpy, pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);

          if (dst_x < new_pmap_width && dst_y < new_pmap_height)
            gdk_pixbuf_xlib_render_to_drawable (result, pixmap, gc,
                                                src_x, src_y,
                                                dst_x, dst_y,
                                                dst_width, dst_height,
                                                XLIB_RGB_DITHER_NONE,
                                                0, 0);
        }

#if XFT
      if (background_flags)
        {
          Display *dpy = target->dpy;
          XRenderPictureAttributes pa;

          XRenderPictFormat *src_format = XRenderFindVisualFormat (dpy, DefaultVisual (dpy, target->display->screen));
          Picture src = XRenderCreatePicture (dpy, root_pmap, src_format, 0, &pa);

          XRenderPictFormat *dst_format = XRenderFindVisualFormat (dpy, target->visual);
          Picture dst = XRenderCreatePicture (dpy, pixmap, dst_format, 0, &pa);

          pa.repeat = True;
          Pixmap mask_pmap = XCreatePixmap (dpy, target->vt, 1, 1, 8);
          XRenderPictFormat *mask_format = XRenderFindStandardFormat (dpy, PictStandardA8);
          Picture mask = XRenderCreatePicture (dpy, mask_pmap, mask_format, CPRepeat, &pa);
          XFreePixmap (dpy, mask_pmap);

          if (src && dst && mask)
            {
              XRenderColor mask_c;

              mask_c.alpha = 0x8000;
              mask_c.red = 0;
              mask_c.green = 0;
              mask_c.blue = 0;
              XRenderFillRectangle (dpy, PictOpSrc, mask, &mask_c, 0, 0, 1, 1);
              XRenderComposite (dpy, PictOpOver, src, mask, dst, 0, 0, 0, 0, 0, 0, target_width, target_height);
            }

          XRenderFreePicture (dpy, src);
          XRenderFreePicture (dpy, dst);
          XRenderFreePicture (dpy, mask);

          XFreePixmap (dpy, root_pmap);
        }
#endif

      if (result != pixbuf)
        g_object_unref (result);

      XFreeGC (target->dpy, gc);

      ret = true;
    }

  return ret;
}
#  endif /* HAVE_PIXBUF */

bool
bgPixmap_t::set_file (const char *file)
{
  assert (file);

  if (*file)
    {
      if (const char *p = strchr (file, ';'))
        {
          size_t len = p - file;
          char *f = rxvt_temp_buf<char> (len + 1);
          memcpy (f, file, len);
          f[len] = '\0';
          file = f;
        }

#  ifdef HAVE_AFTERIMAGE
      if (!target->asimman)
        target->asimman = create_generic_imageman (target->rs[Rs_path]);
      original_asim = get_asimage (target->asimman, file, 0xFFFFFFFF, 100);
      if (original_asim)
        have_image = true;
      return have_image;
#  endif

#  ifdef HAVE_PIXBUF
      pixbuf = gdk_pixbuf_new_from_file (file, NULL);
      if (pixbuf)
        have_image = true;
      return have_image;
#  endif
    }

  return false;
}

# endif /* BG_IMAGE_FROM_FILE */

# ifdef ENABLE_TRANSPARENCY
bool
bgPixmap_t::set_transparent ()
{
  if (!(flags & isTransparent))
    {
      flags |= isTransparent;
      return true;
    }

  return false;
}

bool
bgPixmap_t::set_blur_radius (const char *geom)
{
  int changed = 0;
  unsigned int hr, vr;
  int junk;
  int geom_flags = XParseGeometry (geom, &junk, &junk, &hr, &vr);

  if (!(geom_flags & WidthValue))
    hr = 1;
  if (!(geom_flags & HeightValue))
    vr = hr;

  min_it (hr, 128);
  min_it (vr, 128);

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

  if (v_blurRadius == 0 && h_blurRadius == 0)
    flags &= ~blurNeeded;
  else
    flags |= blurNeeded;

#if XFT
      XFilters *filters = XRenderQueryFilters (target->dpy, target->display->root);
      if (filters)
        {
          for (int i = 0; i < filters->nfilter; i++)
            if (!strcmp (filters->filter[i], FilterConvolution))
              flags |= bgPixmap_t::blurServerSide;

          XFree (filters);
        }
#endif

  return (changed > 0);
}

static inline unsigned long
compute_tint_shade_flags (rxvt_color *tint, int shade)
{
  unsigned long flags = 0;
  rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
  bool has_shade = shade != 100;

  if (tint)
    {
      tint->get (c);
#  define IS_COMPONENT_WHOLESOME(cmp)  ((cmp) <= 0x000700 || (cmp) >= 0x00f700)
      if (!has_shade && IS_COMPONENT_WHOLESOME (c.r)
          && IS_COMPONENT_WHOLESOME (c.g)
          && IS_COMPONENT_WHOLESOME (c.b))
        flags |= bgPixmap_t::tintWholesome;
#  undef  IS_COMPONENT_WHOLESOME
    }

  if (has_shade)
    flags |= bgPixmap_t::tintNeeded;
  else if (tint)
    {
      if ((c.r > 0x000700 || c.g > 0x000700 || c.b > 0x000700)
          && (c.r < 0x00f700 || c.g < 0x00f700 || c.b < 0x00f700))
        {
          flags |= bgPixmap_t::tintNeeded;
        }
    }

  if (flags & bgPixmap_t::tintNeeded)
    {
      if (flags & bgPixmap_t::tintWholesome)
        flags |= bgPixmap_t::tintServerSide;
      else
        {
#if XFT
          flags |= bgPixmap_t::tintServerSide;
#endif
        }
    }

  return flags;
}

bool
bgPixmap_t::set_tint (rxvt_color &new_tint)
{
  if (!(flags & tintSet) || tint != new_tint)
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
      flags = (flags & ~tintFlags) | new_flags;
      return true;
    }

  return false;
}

bool
bgPixmap_t::set_shade (const char *shade_str)
{
  int new_shade = (shade_str) ? atoi (shade_str) : 100;

  clamp_it (new_shade, -100, 200);
  if (new_shade < 0)
    new_shade = 200 - (100 + new_shade);

  if (new_shade != shade)
    {
      unsigned long new_flags = compute_tint_shade_flags ((flags & tintSet) ? &tint : NULL, new_shade);
      shade = new_shade;
      flags = (flags & (~tintFlags | tintSet)) | new_flags;
      return true;
    }

  return false;
}

static void
get_gaussian_kernel (int radius, int width, double *kernel, XFixed *params)
{
    double sigma = radius / 2.0;
    double scale = sqrt (2.0 * M_PI) * sigma;
    double sum = 0.0;

    for (int i = 0; i < width; i++)
      {
	double x = i - width / 2;
	kernel[i] = exp (-(x * x) / (2.0 * sigma * sigma)) / scale;
	sum += kernel[i];
      }

    params[0] = XDoubleToFixed (width);
    params[1] = XDoubleToFixed (1);

    for (int i = 0; i < width; i++)
      params[i+2] = XDoubleToFixed (kernel[i] / sum);
}

bool
bgPixmap_t::blur_pixmap (Pixmap pixmap, Visual *visual, int width, int height)
{
  bool ret = false;
#if XFT
  int size = max (h_blurRadius, v_blurRadius) * 2 + 1;
  double *kernel = (double *)malloc (size * sizeof (double));
  XFixed *params = (XFixed *)malloc ((size + 2) * sizeof (XFixed));

  Display *dpy = target->dpy;
  XRenderPictureAttributes pa;
  XRenderPictFormat *format = XRenderFindVisualFormat (dpy, target->visual);

  Picture src = XRenderCreatePicture (dpy, pixmap, format, 0, &pa);
  Picture dst = XRenderCreatePicture (dpy, pixmap, format, 0, &pa);

  if (kernel && params && src && dst)
    {
      if (h_blurRadius)
        {
          size = h_blurRadius * 2 + 1;
          get_gaussian_kernel (h_blurRadius, size, kernel, params);

          XRenderSetPictureFilter (dpy, src, FilterConvolution, params, size+2);
          XRenderComposite (dpy,
                            PictOpSrc,
                            src,
                            None,
                            dst,
                            0, 0,
                            0, 0,
                            0, 0,
                            width, height);
        }

      if (v_blurRadius)
        {
          size = v_blurRadius * 2 + 1;
          get_gaussian_kernel (v_blurRadius, size, kernel, params);
          swap (params[0], params[1]);

          XRenderSetPictureFilter (dpy, src, FilterConvolution, params, size+2);
          XRenderComposite (dpy,
                            PictOpSrc,
                            src,
                            None,
                            dst,
                            0, 0,
                            0, 0,
                            0, 0,
                            width, height);
        }

      ret = true;
    }

  free (kernel);
  free (params);
  XRenderFreePicture (dpy, src);
  XRenderFreePicture (dpy, dst);
#endif
  return ret;
}

bool
bgPixmap_t::tint_pixmap (Pixmap pixmap, Visual *visual, int width, int height)
{
  Display *dpy = target->dpy;
  bool ret = false;

  if (flags & tintWholesome)
    {
      XGCValues gcv;
      GC gc;

      /* In this case we can tint image server-side getting significant
       * performance improvements, as we eliminate XImage transfer
       */
      gcv.foreground = Pixel (tint);
      gcv.function = GXand;
      gcv.fill_style = FillSolid;
      gc = XCreateGC (dpy, pixmap, GCFillStyle | GCForeground | GCFunction, &gcv);
      if (gc)
        {
          XFillRectangle (dpy, pixmap, gc, 0, 0, width, height);
          ret = true;
          XFreeGC (dpy, gc);
        }
    }
  else
    {
#  if XFT
      rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);

      if (flags & tintSet)
        tint.get (c);

      if (shade <= 100)
        {
          c.r = (c.r * shade) / 100;
          c.g = (c.g * shade) / 100;
          c.b = (c.b * shade) / 100;
        }
      else
        {
          c.r = ((0xffff - c.r) * (200 - shade)) / 100;
          c.g = ((0xffff - c.g) * (200 - shade)) / 100;
          c.b = ((0xffff - c.b) * (200 - shade)) / 100;
        }

      XRenderPictFormat *solid_format = XRenderFindStandardFormat (dpy, PictStandardARGB32);
      XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);
      XRenderPictureAttributes pa;

      Picture back_pic = XRenderCreatePicture (dpy, pixmap, format, 0, &pa);

      pa.repeat = True;

      Pixmap overlay_pmap = XCreatePixmap (dpy, pixmap, 1, 1, 32);
      Picture overlay_pic = XRenderCreatePicture (dpy, overlay_pmap, solid_format, CPRepeat, &pa);
      XFreePixmap (dpy, overlay_pmap);

      pa.component_alpha = True;
      Pixmap mask_pmap = XCreatePixmap (dpy, pixmap, 1, 1, 32);
      Picture mask_pic = XRenderCreatePicture (dpy, mask_pmap, solid_format, CPRepeat|CPComponentAlpha, &pa);
      XFreePixmap (dpy, mask_pmap);

      if (mask_pic && overlay_pic && back_pic)
        {
          XRenderColor mask_c;

          memset (&mask_c, (shade > 100) ? 0xFF : 0x0, sizeof (mask_c));
          mask_c.alpha = 0xffff;
          XRenderFillRectangle (dpy, PictOpSrc, overlay_pic, &mask_c, 0, 0, 1, 1);

          mask_c.alpha = 0;
          mask_c.red = 0xffff - c.r;
          mask_c.green = 0xffff - c.g;
          mask_c.blue = 0xffff - c.b;
          XRenderFillRectangle (dpy, PictOpSrc, mask_pic, &mask_c, 0, 0, 1, 1);
          XRenderComposite (dpy, PictOpOver, overlay_pic, mask_pic, back_pic, 0, 0, 0, 0, 0, 0, width, height);
          ret = true;
        }

      XRenderFreePicture (dpy, mask_pic);
      XRenderFreePicture (dpy, overlay_pic);
      XRenderFreePicture (dpy, back_pic);
#   if DO_TIMING_TEST
      XSync (dpy, False);
#   endif
#  endif
    }

  return ret;
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

  /* root dimensions may change from call to call - but Display structure should
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
  GC gc;

  TIMING_TEST_START (tp);
  target->get_window_origin (sx, sy);

  /* check if we are outside of the visible part of the virtual screen : */
  if (sx + window_width <= 0 || sy + window_height <= 0
      || sx >= root_width || sy >= root_height)
    return 0;

  if (root_pixmap != None)
    {
      /* we want to validate the pixmap and get its size at the same time : */
      int junk;
      unsigned int ujunk;
      /* root pixmap may be bad - allow a error */
      target->allowedxerror = -1;

      if (!XGetGeometry (dpy, root_pixmap, &root, &junk, &junk, &root_pmap_width, &root_pmap_height, &ujunk, &ujunk))
        root_pixmap = None;

      target->allowedxerror = 0;
    }

  if (root_pixmap == None)
    return 0;

  Pixmap tiled_root_pmap = XCreatePixmap (dpy, root, window_width, window_height, root_depth);

  if (tiled_root_pmap == None) /* something really bad happened - abort */
    return 0;

  /* straightforward pixmap copy */
  gcv.tile = root_pixmap;
  gcv.fill_style = FillTiled;

  while (sx < 0) sx += (int)root_width;
  while (sy < 0) sy += (int)root_height;

  gcv.ts_x_origin = -sx;
  gcv.ts_y_origin = -sy;
  gc = XCreateGC (dpy, root, GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gcv);

  if (gc)
    {
      XFillRectangle (dpy, tiled_root_pmap, gc, 0, 0, window_width, window_height);
      result |= transpPmapTiled;
      XFreeGC (dpy, gc);
    }
  TIMING_TEST_PRINT_RESULT (tp);

  if (tiled_root_pmap != None)
    {
      if (!need_client_side_rendering ())
        {
          if ((flags & blurNeeded))
            {
              if (blur_pixmap (tiled_root_pmap, DefaultVisual (dpy, target->display->screen), window_width, window_height))
                result |= transpPmapBlurred;
            }
          if ((flags & tintNeeded))
            {
              if (tint_pixmap (tiled_root_pmap, DefaultVisual (dpy, target->display->screen), window_width, window_height))
                result |= transpPmapTinted;
            }
        } /* server side rendering completed */

      if (pixmap)
        XFreePixmap (dpy, pixmap);

      pixmap = tiled_root_pmap;
      pmap_width = window_width;
      pmap_height = window_height;
      pmap_depth = root_depth;
    }

  TIMING_TEST_PRINT_RESULT (tp);

  return result;
}

bool
bgPixmap_t::set_root_pixmap ()
{
  Pixmap new_root_pixmap = target->get_pixmap_property (XA_XROOTPMAP_ID);
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
static void ShadeXImage(rxvt_term *term, XImage *srcImage, int shade, int rm, int gm, int bm);
# endif

bool
bgPixmap_t::render ()
{
  unsigned long background_flags = 0;

  if (target == NULL)
    return false;

  TIMING_TEST_START (tp);

  invalidate ();
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

# ifdef BG_IMAGE_FROM_FILE
  if (have_image
      || (background_flags & transpTransformations) != (flags & transpTransformations))
    {
      if (render_image (background_flags))
        flags = flags & ~isInvalid;
    }
# endif

  XImage *result = NULL;

  if (background_flags && (flags & isInvalid))
    {
      result = XGetImage (target->dpy, pixmap, 0, 0, pmap_width, pmap_height, AllPlanes, ZPixmap);
    }

  if (result)
    {
#  if !defined(HAVE_AFTERIMAGE) && !XFT
      /* our own client-side tinting */
      /* ATTENTION: We ASSUME that XFT will let us do all the tinting necessary server-side.
         This may need to be changed in need_client_side_rendering() logic is altered !!! */
      if (!(background_flags & transpPmapTinted) && (flags & tintNeeded))
        {
          rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
          if (flags & tintSet)
            tint.get (c);
          ShadeXImage (target, result, shade, c.r, c.g, c.b);
        }
#  endif

      GC gc = XCreateGC (target->dpy, target->vt, 0UL, NULL);

      if (gc)
        {
          if (/*pmap_depth != target->depth &&*/ pixmap != None)
            {
              XFreePixmap (target->dpy, pixmap);
              pixmap = None;
            }

          if (pixmap == None)
            {
              pixmap = XCreatePixmap (target->dpy, target->vt, result->width, result->height, target->depth);
              pmap_width  = result->width;
              pmap_height = result->height;
              pmap_depth  = target->depth;
            }

          if (pmap_depth != result->depth)
            {
              /* Bad Match error will ensue ! stupid X !!!! */
              if (result->depth == 24 && pmap_depth == 32)
                result->depth = 32;
              else if (result->depth == 32 && pmap_depth == 24)
                result->depth = 24;
              else
                {
                  /* TODO: implement image recoding */
                }
            }

          if (pmap_depth == result->depth)
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
    }

  apply ();

  XSync (target->dpy, False);
  valid_since = ev::now ();

  TIMING_TEST_PRINT_RESULT (tp);

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
bgPixmap_t::apply ()
{
  if (target)
    {
      flags &= ~isVtOrigin;

      if (pixmap != None)
        {
          /* set target's background to pixmap */
# ifdef ENABLE_TRANSPARENCY
          if (flags & isTransparent)
            {
              XSetWindowBackgroundPixmap (target->dpy, target->parent[0], pixmap);
              XSetWindowBackgroundPixmap (target->dpy, target->vt, ParentRelative);

              if (target->scrollBar.win)
                XSetWindowBackgroundPixmap (target->dpy, target->scrollBar.win, ParentRelative);
            }
          else
# endif
            {
              flags |= isVtOrigin;
              /* force old pixmap dereference in case it was transparent before :*/
              XSetWindowBackground (target->dpy, target->parent[0], target->pix_colors[Color_border]);
              XSetWindowBackgroundPixmap (target->dpy, target->vt, pixmap);
              /* do we also need to set scrollbar's background here ? */

              if (target->scrollBar.win)
                XSetWindowBackground (target->dpy, target->scrollBar.win, target->pix_colors[Color_border]);
            }
        }
      else
        {
          /* set target background to a pixel */
          XSetWindowBackground (target->dpy, target->parent[0], target->pix_colors[Color_border]);
          XSetWindowBackground (target->dpy, target->vt, target->pix_colors[Color_bg]);
          /* do we also need to set scrollbar's background here ? */
          if (target->scrollBar.win)
            XSetWindowBackground (target->dpy, target->scrollBar.win, target->pix_colors[Color_border]);
        }

      /* don't want Expose on the parent or vt. It is better to use
         scr_touch or we get a great deal of flicker otherwise: */
      XClearWindow (target->dpy, target->parent[0]);

      if (target->scrollBar.state && target->scrollBar.win)
        {
          target->scrollBar.state = STATE_IDLE;
          target->scrollBar.show (0);
        }

      target->want_refresh = 1;
      flags |= hasChanged;
    }
}

#endif /* HAVE_BG_PIXMAP */

#if defined(ENABLE_TRANSPARENCY) && !defined(HAVE_AFTERIMAGE) && !XFT
/* taken from aterm-0.4.2 */

typedef uint32_t RUINT32T;

static void
ShadeXImage(rxvt_term *term, XImage *srcImage, int shade, int rm, int gm, int bm)
{
  int sh_r, sh_g, sh_b;
  RUINT32T mask_r, mask_g, mask_b;
  RUINT32T *lookup, *lookup_r, *lookup_g, *lookup_b;
  unsigned int lower_lim_r, lower_lim_g, lower_lim_b;
  unsigned int upper_lim_r, upper_lim_g, upper_lim_b;
  int i;

  Visual *visual = term->visual;

  if (visual->c_class != TrueColor || srcImage->format != ZPixmap) return ;

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
  if (shade > 100) {
    shade = 200 - shade;

    lower_lim_r = 65535-rm;
    lower_lim_g = 65535-gm;
    lower_lim_b = 65535-bm;

    lower_lim_r = 65535-(unsigned int)(((RUINT32T)lower_lim_r)*((RUINT32T)shade)/100);
    lower_lim_g = 65535-(unsigned int)(((RUINT32T)lower_lim_g)*((RUINT32T)shade)/100);
    lower_lim_b = 65535-(unsigned int)(((RUINT32T)lower_lim_b)*((RUINT32T)shade)/100);

    upper_lim_r = upper_lim_g = upper_lim_b = 65535;
  } else {

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
#endif /* defined(ENABLE_TRANSPARENCY) && !defined(HAVE_AFTERIMAGE) */
