/*----------------------------------------------------------------------*
 * File:	background.C - former xpm.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2008 Marc Lehmann <schmorp@schmorp.de>
 * Copyright (c) 2007      Sasha Vasko <sasha@aftercode.net>
 * Copyright (c) 2010      Emanuele Giaquinta <e.giaquinta@glauco.it>
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

#include <math.h>
#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */

#if XRENDER
# include <X11/extensions/Xrender.h>
#endif

#ifndef FilterConvolution
#define FilterConvolution "convolution"
#endif

#ifndef RepeatPad
#define RepeatPad True
#endif

#ifdef HAVE_BG_PIXMAP
# if XRENDER
static Picture
create_xrender_mask (Display *dpy, Drawable drawable, Bool argb, Bool component_alpha)
{
  Pixmap pixmap = XCreatePixmap (dpy, drawable, 1, 1, argb ? 32 : 8);

  XRenderPictFormat *format = XRenderFindStandardFormat (dpy, argb ? PictStandardARGB32 : PictStandardA8);
  XRenderPictureAttributes pa;
  pa.repeat = True;
  pa.component_alpha = component_alpha;
  Picture mask = XRenderCreatePicture (dpy, pixmap, format, CPRepeat | CPComponentAlpha, &pa);

  XFreePixmap (dpy, pixmap);

  return mask;
}
# endif

void
rxvt_term::bg_destroy ()
{
#ifdef HAVE_AFTERIMAGE
  if (original_asim)
    safe_asimage_destroy (original_asim);
  if (asv)
    destroy_asvisual (asv, 0);
  if (asimman)
    destroy_image_manager (asimman, 0);
#endif

#ifdef HAVE_PIXBUF
  if (pixbuf)
    g_object_unref (pixbuf);
#endif

  if (bg_pixmap)
    XFreePixmap (dpy, bg_pixmap);
}

bool
rxvt_term::bg_set_position (int x, int y)
{

  if (target_x != x
      || target_y != y)
    {
      target_x = x;
      target_y = y;
      return true;
    }
  return false;
}

bool
rxvt_term::bg_window_size_sensitive ()
{
# ifdef ENABLE_TRANSPARENCY
  if (bg_flags & BG_IS_TRANSPARENT)
    return true;
# endif

# ifdef BG_IMAGE_FROM_FILE
  if (bg_flags & BG_IS_FROM_FILE)
    {
      if (bg_flags & BG_IS_SIZE_SENSITIVE)
        return true;
    }
# endif

  return false;
}

bool
rxvt_term::bg_window_position_sensitive ()
{
# ifdef ENABLE_TRANSPARENCY
  if (bg_flags & BG_IS_TRANSPARENT)
    return true;
# endif

# ifdef BG_IMAGE_FROM_FILE
  if (bg_flags & BG_IS_FROM_FILE)
    {
      if (bg_flags & BG_ROOT_ALIGN)
        return true;
    }
# endif

  return false;
}

# ifdef BG_IMAGE_FROM_FILE
static inline int
make_align_position (int align, int window_size, int image_size)
{
  int diff = window_size - image_size;
  int smaller = min (image_size, window_size);

  if (align >= 0 && align <= 100)
    return diff * align / 100;
  else if (align > 100 && align <= 200)
    return (align - 100) * smaller / 100 + window_size - smaller;
  else if (align >= -100 && align < 0)
    return (align + 100) * smaller / 100 - image_size;
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

  min_it (dst_size, target_size - dst_pos);
  return src_pos;
}

bool
rxvt_term::bg_set_geometry (const char *geom, bool update)
{
  bool changed = false;
  int geom_flags = 0;
  int x = h_align;
  int y = v_align;
  unsigned int w = h_scale;
  unsigned int h = v_scale;
  unsigned long new_flags = 0;

  if (geom == NULL)
    return false;

  if (geom[0])
    {
      char **arr = rxvt_strsplit (':', geom);

      for (int i = 0; arr[i]; i++)
        {
          if (!strcasecmp (arr[i], "style=tiled"))
            {
              new_flags = BG_TILE;
              w = h = noScale;
              x = y = 0;
              geom_flags = WidthValue|HeightValue|XValue|YValue;
            }
          else if (!strcasecmp (arr[i], "style=aspect-stretched"))
            {
              new_flags = BG_KEEP_ASPECT;
              w = h = windowScale;
              x = y = centerAlign;
              geom_flags = WidthValue|HeightValue|XValue|YValue;
            }
          else if (!strcasecmp (arr[i], "style=stretched"))
            {
              new_flags = 0;
              w = h = windowScale;
              geom_flags = WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "style=centered"))
            {
              new_flags = 0;
              w = h = noScale;
              x = y = centerAlign;
              geom_flags = WidthValue|HeightValue|XValue|YValue;
            }
          else if (!strcasecmp (arr[i], "style=root-tiled"))
            {
              new_flags = BG_TILE|BG_ROOT_ALIGN;
              w = h = noScale;
              geom_flags = WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "op=tile"))
            new_flags |= BG_TILE;
          else if (!strcasecmp (arr[i], "op=keep-aspect"))
            new_flags |= BG_KEEP_ASPECT;
          else if (!strcasecmp (arr[i], "op=root-align"))
            new_flags |= BG_ROOT_ALIGN;

          // deprecated
          else if (!strcasecmp (arr[i], "tile"))
            {
              new_flags |= BG_TILE;
              w = h = noScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "propscale"))
            {
              new_flags |= BG_KEEP_ASPECT;
              w = h = windowScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "hscale"))
            {
              new_flags |= BG_TILE;
              w = windowScale;
              h = noScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "vscale"))
            {
              new_flags |= BG_TILE;
              h = windowScale;
              w = noScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "scale"))
            {
              w = h = windowScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "auto"))
            {
              w = h = windowScale;
              x = y = centerAlign;
              geom_flags |= WidthValue|HeightValue|XValue|YValue;
            }
          else if (!strcasecmp (arr[i], "root"))
            {
              new_flags |= BG_TILE|BG_ROOT_ALIGN;
              w = h = noScale;
              geom_flags |= WidthValue|HeightValue;
            }

          else
            geom_flags |= XParseGeometry (arr[i], &x, &y, &w, &h);
        } /* done parsing ops */

      rxvt_free_strsplit (arr);
    }

  new_flags |= bg_flags & ~BG_GEOMETRY_FLAGS;

  if (!update)
    {
      if (!(geom_flags & XValue))
        x = y = defaultAlign;
      else if (!(geom_flags & YValue))
        y = x;

      if (!(geom_flags & (WidthValue|HeightValue)))
        w = h = defaultScale;
      else if (!(geom_flags & HeightValue))
        h = w;
      else if (!(geom_flags & WidthValue))
        w = h;
    }

  min_it (w, 1000);
  min_it (h, 1000);
  clamp_it (x, -100, 200);
  clamp_it (y, -100, 200);

  if (bg_flags != new_flags
      || h_scale != w
      || v_scale != h
      || h_align != x
      || v_align != y)
    {
      bg_flags = new_flags;
      h_scale = w;
      v_scale = h;
      h_align = x;
      v_align = y;
      changed = true;
    }

  return changed;
}

void
rxvt_term::get_image_geometry (int image_width, int image_height, int &w, int &h, int &x, int &y)
{
  int target_width = szHint.width;
  int target_height = szHint.height;

  w = h_scale * target_width / 100;
  h = v_scale * target_height / 100;

  if (bg_flags & BG_KEEP_ASPECT)
    {
      float scale = (float)w / image_width;
      min_it (scale, (float)h / image_height);
      w = image_width * scale + 0.5;
      h = image_height * scale + 0.5;
    }

  if (!w) w = image_width;
  if (!h) h = image_height;

  if (bg_flags & BG_ROOT_ALIGN)
    {
      x = -target_x;
      y = -target_y;
    }
  else
    {
      x = make_align_position (h_align, target_width, w);
      y = make_align_position (v_align, target_height, h);
    }

  bg_flags &= ~BG_IS_SIZE_SENSITIVE;
  if (!(bg_flags & BG_TILE)
      || h_scale || v_scale
      || (!(bg_flags & BG_ROOT_ALIGN) && (h_align || v_align))
      || w > target_width || h > target_height)
    bg_flags |= BG_IS_SIZE_SENSITIVE;
}

#  ifdef HAVE_AFTERIMAGE
bool
rxvt_term::render_image (unsigned long tr_flags)
{
  init_asv ();

  ASImage *background = NULL;
  ARGB32 background_tint = TINT_LEAVE_SAME;

#   ifdef ENABLE_TRANSPARENCY
  if (tr_flags)
    background = pixmap2ximage (asv, bg_pixmap, 0, 0, bg_pmap_width, bg_pmap_height, AllPlanes, 100);

  if (tr_flags & BG_NEEDS_TINT)
    {
      ShadingInfo as_shade;
      as_shade.shading = shade;

      rgba c (rgba::MAX_CC,rgba::MAX_CC,rgba::MAX_CC);
      if (bg_flags & BG_TINT_SET)
        tint.get (c);
      as_shade.tintColor.red = c.r;
      as_shade.tintColor.green = c.g;
      as_shade.tintColor.blue = c.b;

      background_tint = shading2tint32 (&as_shade);
    }

  if ((tr_flags & BG_NEEDS_BLUR) && background != NULL)
    {
      ASImage *tmp = blur_asimage_gauss (asv, background, h_blurRadius, v_blurRadius, 0xFFFFFFFF,
                                         ASA_XImage,
                                         100, ASIMAGE_QUALITY_DEFAULT);
      if (tmp)
        {
          destroy_asimage (&background);
          background = tmp;
        }
    }
#   endif

  ASImage *result = 0;

  int target_width    = szHint.width;
  int target_height   = szHint.height;
  int new_pmap_width  = target_width;
  int new_pmap_height = target_height;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  if (original_asim)
    get_image_geometry (original_asim->width, original_asim->height, w, h, x, y);

  if (!original_asim
      || (!(bg_flags & BG_ROOT_ALIGN)
          && (x >= target_width
              || y >= target_height
              || x + w <= 0
              || y + h <= 0)))
    {
      if (background)
        {
          new_pmap_width = background->width;
          new_pmap_height = background->height;
          result = background;

          if (background_tint != TINT_LEAVE_SAME)
            {
              ASImage *tmp = tile_asimage (asv, background, 0, 0,
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

      if (w != original_asim->width
          || h != original_asim->height)
        {
          result = scale_asimage (asv, original_asim,
                                  w, h,
                                  ASA_XImage,
                                  100, ASIMAGE_QUALITY_DEFAULT);
        }

      if (background == NULL)
        {
          if (bg_flags & BG_TILE)
            {
              /* if tiling - pixmap has to be sized exactly as the image,
                 but there is no need to make it bigger than the window! */
              new_pmap_width = min (result->width, target_width);
              new_pmap_height = min (result->height, target_height);

              /* we also need to tile our image in both directions */
              ASImage *tmp = tile_asimage (asv, result,
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

          if (bg_flags & BG_TILE)
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

          if (bg_flags & BG_TILE)
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

          if (rs[Rs_blendtype])
            {
              layers[1].merge_scanlines = blend_scanlines_name2func (rs[Rs_blendtype]);
              if (layers[1].merge_scanlines == NULL)
                layers[1].merge_scanlines = alphablend_scanlines;
            }

          ASImage *tmp = merge_layers (asv, layers, 2, target_width, target_height,
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

  bool ret = false;

  if (result)
    {
      XGCValues gcv;
      GC gc;

      /* create Pixmap */
      if (bg_pixmap == None
          || bg_pmap_width != new_pmap_width
          || bg_pmap_height != new_pmap_height)
        {
          if (bg_pixmap)
            XFreePixmap (dpy, bg_pixmap);
          bg_pixmap = XCreatePixmap (dpy, vt, new_pmap_width, new_pmap_height, depth);
          bg_pmap_width = new_pmap_width;
          bg_pmap_height = new_pmap_height;
        }
      /* fill with background color (if result's not completely overlapping it) */
      gcv.foreground = pix_colors[Color_bg];
      gc = XCreateGC (dpy, vt, GCForeground, &gcv);

      int src_x = 0, src_y = 0, dst_x = 0, dst_y = 0;
      int dst_width = result->width, dst_height = result->height;
      if (background == NULL)
        {
          if (!(bg_flags & BG_TILE))
            {
              src_x = make_clip_rectangle (x, result->width , new_pmap_width , dst_x, dst_width );
              src_y = make_clip_rectangle (y, result->height, new_pmap_height, dst_y, dst_height);
            }

          if (dst_x > 0 || dst_y > 0
              || dst_x + dst_width < new_pmap_width
              || dst_y + dst_height < new_pmap_height)
            XFillRectangle (dpy, bg_pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);
        }

      /* put result on pixmap */
      if (dst_x < new_pmap_width && dst_y < new_pmap_height)
        asimage2drawable (asv, bg_pixmap, result, gc, src_x, src_y, dst_x, dst_y, dst_width, dst_height, True);

      if (result != background && result != original_asim)
        destroy_asimage (&result);

      XFreeGC (dpy, gc);

      ret = true;
    }

  if (background)
    destroy_asimage (&background);

  return ret;
}
#  endif /* HAVE_AFTERIMAGE */

#  ifdef HAVE_PIXBUF
bool
rxvt_term::pixbuf_to_pixmap (GdkPixbuf *pixbuf, Pixmap pixmap, GC gc,
                             int src_x, int src_y, int dst_x, int dst_y,
                             unsigned int width, unsigned int height)
{
  XImage *ximage;
  char *data, *line;
  int bytes_per_pixel;
  int width_r, width_g, width_b;
  int sh_r, sh_g, sh_b;
  int rowstride;
  int channels;
  unsigned char *row;

  if (visual->c_class != TrueColor)
    return false;

  if (depth == 24 || depth == 32)
    bytes_per_pixel = 4;
  else if (depth == 15 || depth == 16)
    bytes_per_pixel = 2;
  else
    return false;

  width_r = ecb_popcount32 (visual->red_mask);
  width_g = ecb_popcount32 (visual->green_mask);
  width_b = ecb_popcount32 (visual->blue_mask);

  if (width_r > 8 || width_g > 8 || width_b > 8)
    return false;

  sh_r = ecb_ctz32 (visual->red_mask);
  sh_g = ecb_ctz32 (visual->green_mask);
  sh_b = ecb_ctz32 (visual->blue_mask);

  if (width > INT_MAX / height / bytes_per_pixel)
    return false;

  data = (char *)malloc (width * height * bytes_per_pixel);
  if (!data)
    return false;

  ximage = XCreateImage (dpy, visual, depth, ZPixmap, 0, data,
                         width, height, bytes_per_pixel * 8, 0);
  if (!ximage)
    {
      free (data);
      return false;
    }

  ximage->byte_order = ecb_big_endian () ? MSBFirst : LSBFirst;

  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  channels = gdk_pixbuf_get_n_channels (pixbuf);
  row = gdk_pixbuf_get_pixels (pixbuf) + src_y * rowstride + src_x * channels;
  line = data;

  for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
        {
          unsigned char *pixel = row + x * channels;
          uint32_t value;

          value  = ((pixel[0] >> (8 - width_r)) << sh_r)
                 | ((pixel[1] >> (8 - width_g)) << sh_g)
                 | ((pixel[2] >> (8 - width_b)) << sh_b);

          if (bytes_per_pixel == 4)
            ((uint32_t *)line)[x] = value;
          else
            ((uint16_t *)line)[x] = value;
        }

      row += rowstride;
      line += ximage->bytes_per_line;
    }

  XPutImage (dpy, pixmap, gc, ximage, 0, 0, dst_x, dst_y, width, height);
  XDestroyImage (ximage);
  return true;
}

bool
rxvt_term::render_image (unsigned long tr_flags)
{
  if (!pixbuf)
    return false;

  if (tr_flags
      && !(bg_flags & BG_HAS_RENDER))
    return false;

  GdkPixbuf *result;

  int image_width = gdk_pixbuf_get_width (pixbuf);
  int image_height = gdk_pixbuf_get_height (pixbuf);

  int target_width    = szHint.width;
  int target_height   = szHint.height;
  int new_pmap_width  = target_width;
  int new_pmap_height = target_height;

  int x = 0;
  int y = 0;
  int w = 0;
  int h = 0;

  get_image_geometry (image_width, image_height, w, h, x, y);

  if (!(bg_flags & BG_ROOT_ALIGN)
      && (x >= target_width
          || y >= target_height
          || x + w <= 0
          || y + h <= 0))
    return false;

  result = pixbuf;

  if (w != image_width
      || h != image_height)
    {
      result = gdk_pixbuf_scale_simple (pixbuf,
                                        w, h,
                                        GDK_INTERP_BILINEAR);
    }

  if (!result)
    return false;

  bool ret = false;

  XGCValues gcv;
  GC gc;
  Pixmap root_pmap;

  image_width = gdk_pixbuf_get_width (result);
  image_height = gdk_pixbuf_get_height (result);

  if (tr_flags)
    {
      root_pmap = bg_pixmap;
      bg_pixmap = None;
    }
  else
    {
      if (bg_flags & BG_TILE)
        {
          new_pmap_width = min (image_width, target_width);
          new_pmap_height = min (image_height, target_height);
        }
    }

  if (bg_pixmap == None
      || bg_pmap_width != new_pmap_width
      || bg_pmap_height != new_pmap_height)
    {
      if (bg_pixmap)
        XFreePixmap (dpy, bg_pixmap);
      bg_pixmap = XCreatePixmap (dpy, vt, new_pmap_width, new_pmap_height, depth);
      bg_pmap_width = new_pmap_width;
      bg_pmap_height = new_pmap_height;
    }

  gcv.foreground = pix_colors[Color_bg];
  gc = XCreateGC (dpy, vt, GCForeground, &gcv);

  if (gc)
    {
      if (bg_flags & BG_TILE)
        {
          Pixmap tile = XCreatePixmap (dpy, vt, image_width, image_height, depth);
          pixbuf_to_pixmap (result, tile, gc,
                            0, 0,
                            0, 0,
                            image_width, image_height);

          gcv.tile = tile;
          gcv.fill_style = FillTiled;
          gcv.ts_x_origin = x;
          gcv.ts_y_origin = y;
          XChangeGC (dpy, gc, GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gcv);

          XFillRectangle (dpy, bg_pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);
          XFreePixmap (dpy, tile);
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
            XFillRectangle (dpy, bg_pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);

          if (dst_x < new_pmap_width && dst_y < new_pmap_height)
            pixbuf_to_pixmap (result, bg_pixmap, gc,
                              src_x, src_y,
                              dst_x, dst_y,
                              dst_width, dst_height);
        }

#if XRENDER
      if (tr_flags)
        {
          XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);

          Picture src = XRenderCreatePicture (dpy, root_pmap, format, 0, 0);

          Picture dst = XRenderCreatePicture (dpy, bg_pixmap, format, 0, 0);

          Picture mask = create_xrender_mask (dpy, vt, False, False);

          XRenderColor mask_c;

          mask_c.alpha = 0x8000;
          mask_c.red   =
          mask_c.green =
          mask_c.blue  = 0;
          XRenderFillRectangle (dpy, PictOpSrc, mask, &mask_c, 0, 0, 1, 1);

          XRenderComposite (dpy, PictOpOver, src, mask, dst, 0, 0, 0, 0, 0, 0, target_width, target_height);

          XRenderFreePicture (dpy, src);
          XRenderFreePicture (dpy, dst);
          XRenderFreePicture (dpy, mask);
        }
#endif

      XFreeGC (dpy, gc);

      ret = true;
    }

  if (result != pixbuf)
    g_object_unref (result);

  if (tr_flags)
    XFreePixmap (dpy, root_pmap);

  return ret;
}
#  endif /* HAVE_PIXBUF */

bool
rxvt_term::bg_set_file (const char *file)
{
  if (!file || !*file)
    return false;

  bool ret = false;
  const char *p = strchr (file, ';');

  if (p)
    {
      size_t len = p - file;
      char *f = rxvt_temp_buf<char> (len + 1);
      memcpy (f, file, len);
      f[len] = '\0';
      file = f;
    }

#  ifdef HAVE_AFTERIMAGE
  if (!asimman)
    asimman = create_generic_imageman (rs[Rs_path]);
  ASImage *image = get_asimage (asimman, file, 0xFFFFFFFF, 100);
  if (image)
    {
      if (original_asim)
        safe_asimage_destroy (original_asim);
      original_asim = image;
      bg_flags |= BG_IS_FROM_FILE | BG_CLIENT_RENDER;
      ret = true;
    }
#  endif

#  ifdef HAVE_PIXBUF
  GdkPixbuf *image = gdk_pixbuf_new_from_file (file, NULL);
  if (image)
    {
      if (pixbuf)
        g_object_unref (pixbuf);
      pixbuf = image;
      bg_flags |= BG_IS_FROM_FILE;
      ret = true;
    }
#  endif

  if (ret)
    {
      if (p)
        bg_set_geometry (p + 1);
      else
        bg_set_default_geometry ();
    }

  return ret;
}

# endif /* BG_IMAGE_FROM_FILE */

# ifdef ENABLE_TRANSPARENCY
bool
rxvt_term::bg_set_transparent ()
{
  if (!(bg_flags & BG_IS_TRANSPARENT))
    {
      bg_flags |= BG_IS_TRANSPARENT;
      return true;
    }

  return false;
}

bool
rxvt_term::bg_set_blur (const char *geom)
{
  bool changed = false;
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
      changed = true;
      h_blurRadius = hr;
    }

  if (v_blurRadius != vr)
    {
      changed = true;
      v_blurRadius = vr;
    }

  if (h_blurRadius == 0 || v_blurRadius == 0)
    bg_flags &= ~BG_NEEDS_BLUR;
  else
    bg_flags |= BG_NEEDS_BLUR;

  return changed;
}

void
rxvt_term::set_tint_shade_flags ()
{
  rgba c;
  bool has_shade = shade != 100;

  bg_flags &= ~BG_TINT_FLAGS;

  if (bg_flags & BG_TINT_SET)
    {
      tint.get (c);
      if (!has_shade
          && (c.r <= 0x00ff || c.r >= 0xff00)
          && (c.g <= 0x00ff || c.g >= 0xff00)
          && (c.b <= 0x00ff || c.b >= 0xff00))
        bg_flags |= BG_TINT_BITAND;
    }

  if (has_shade || (bg_flags & BG_TINT_SET))
    bg_flags |= BG_NEEDS_TINT;
}

bool
rxvt_term::bg_set_tint (rxvt_color &new_tint)
{
  if (!(bg_flags & BG_TINT_SET) || tint != new_tint)
    {
      tint = new_tint;
      bg_flags |= BG_TINT_SET;
      set_tint_shade_flags ();
      return true;
    }

  return false;
}

bool
rxvt_term::bg_set_shade (const char *shade_str)
{
  int new_shade = atoi (shade_str);

  clamp_it (new_shade, -100, 200);
  if (new_shade < 0)
    new_shade = 200 - (100 + new_shade);

  if (new_shade != shade)
    {
      shade = new_shade;
      set_tint_shade_flags ();
      return true;
    }

  return false;
}

#if XRENDER
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
#endif

bool
rxvt_term::blur_pixmap (Pixmap pixmap, Visual *visual, int width, int height, int depth)
{
  bool ret = false;
#if XRENDER
  if (!(bg_flags & BG_HAS_RENDER_CONV))
    return false;

  int size = max (h_blurRadius, v_blurRadius) * 2 + 1;
  double *kernel = (double *)malloc (size * sizeof (double));
  XFixed *params = (XFixed *)malloc ((size + 2) * sizeof (XFixed));

  XRenderPictureAttributes pa;
  XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);

  pa.repeat = RepeatPad;
  Picture src = XRenderCreatePicture (dpy, pixmap, format, CPRepeat, &pa);
  Pixmap tmp = XCreatePixmap (dpy, pixmap, width, height, depth);
  Picture dst = XRenderCreatePicture (dpy, tmp, format, CPRepeat, &pa);
  XFreePixmap (dpy, tmp);

  if (kernel && params)
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

      ::swap (src, dst);

      size = v_blurRadius * 2 + 1;
      get_gaussian_kernel (v_blurRadius, size, kernel, params);
      ::swap (params[0], params[1]);

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
rxvt_term::tint_pixmap (Pixmap pixmap, Visual *visual, int width, int height)
{
  bool ret = false;

  if (bg_flags & BG_TINT_BITAND)
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
#  if XRENDER
  else if (bg_flags & BG_HAS_RENDER)
    {
      rgba c (rgba::MAX_CC, rgba::MAX_CC, rgba::MAX_CC);

      if (bg_flags & BG_TINT_SET)
        tint.get (c);

      if (shade <= 100)
        {
          c.r = c.r * shade / 100;
          c.g = c.g * shade / 100;
          c.b = c.b * shade / 100;
        }
      else
        {
          c.r = c.r * (200 - shade) / 100;
          c.g = c.g * (200 - shade) / 100;
          c.b = c.b * (200 - shade) / 100;
        }

      XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);

      Picture back_pic = XRenderCreatePicture (dpy, pixmap, format, 0, 0);

      Picture overlay_pic = create_xrender_mask (dpy, pixmap, True, False);

      Picture mask_pic = create_xrender_mask (dpy, pixmap, True, True);

      XRenderColor mask_c;

      mask_c.alpha = 0xffff;
      mask_c.red   =
      mask_c.green =
      mask_c.blue  = 0;
      XRenderFillRectangle (dpy, PictOpSrc, overlay_pic, &mask_c, 0, 0, 1, 1);

      mask_c.alpha = 0;
      mask_c.red   = 0xffff - c.r;
      mask_c.green = 0xffff - c.g;
      mask_c.blue  = 0xffff - c.b;
      XRenderFillRectangle (dpy, PictOpSrc, mask_pic, &mask_c, 0, 0, 1, 1);

      XRenderComposite (dpy, PictOpOver, overlay_pic, mask_pic, back_pic, 0, 0, 0, 0, 0, 0, width, height);

      if (shade > 100)
        {
          mask_c.alpha = 0;
          mask_c.red   =
          mask_c.green =
          mask_c.blue  = 0xffff * (shade - 100) / 100;
          XRenderFillRectangle (dpy, PictOpSrc, overlay_pic, &mask_c, 0, 0, 1, 1);

          XRenderComposite (dpy, PictOpOver, overlay_pic, None, back_pic, 0, 0, 0, 0, 0, 0, width, height);
        }

      ret = true;

      XRenderFreePicture (dpy, mask_pic);
      XRenderFreePicture (dpy, overlay_pic);
      XRenderFreePicture (dpy, back_pic);
    }
#  endif

  return ret;
}

/*
 * Builds a pixmap of the same size as the terminal window that contains
 * the tiled portion of the root pixmap that is supposed to be covered by
 * our window.
 */
unsigned long
rxvt_term::make_transparency_pixmap ()
{
  unsigned long result = 0;

  /* root dimensions may change from call to call - but Display structure should
   * be always up-to-date, so let's use it :
   */
  int screen = display->screen;
  int root_depth = DefaultDepth (dpy, screen);
  int root_width = DisplayWidth (dpy, screen);
  int root_height = DisplayHeight (dpy, screen);
  unsigned int root_pmap_width, root_pmap_height;
  int window_width = szHint.width;
  int window_height = szHint.height;
  int sx, sy;
  XGCValues gcv;
  GC gc;

  sx = target_x;
  sy = target_y;

  /* check if we are outside of the visible part of the virtual screen : */
  if (sx + window_width <= 0 || sy + window_height <= 0
      || sx >= root_width || sy >= root_height)
    return 0;

  // validate root pixmap and get its size
  if (root_pixmap != None)
    {
      Window wdummy;
      int idummy;
      unsigned int udummy;

      allowedxerror = -1;

      if (!XGetGeometry (dpy, root_pixmap, &wdummy, &idummy, &idummy, &root_pmap_width, &root_pmap_height, &udummy, &udummy))
        root_pixmap = None;

      allowedxerror = 0;
    }

  Pixmap recoded_root_pmap = root_pixmap;

  if (root_pixmap != None && root_depth != depth)
    {
#if XRENDER
      if (bg_flags & BG_HAS_RENDER)
        {
          recoded_root_pmap = XCreatePixmap (dpy, vt, root_pmap_width, root_pmap_height, depth);

          XRenderPictFormat *src_format = XRenderFindVisualFormat (dpy, DefaultVisual (dpy, screen));
          Picture src = XRenderCreatePicture (dpy, root_pixmap, src_format, 0, 0);

          XRenderPictFormat *dst_format = XRenderFindVisualFormat (dpy, visual);
          Picture dst = XRenderCreatePicture (dpy, recoded_root_pmap, dst_format, 0, 0);

          XRenderComposite (dpy, PictOpSrc, src, None, dst, 0, 0, 0, 0, 0, 0, root_pmap_width, root_pmap_height);

          XRenderFreePicture (dpy, src);
          XRenderFreePicture (dpy, dst);
        }
      else
#endif
      recoded_root_pmap = None;
    }

  if (recoded_root_pmap == None)
    return 0;

  if (bg_pixmap == None
      || bg_pmap_width != window_width
      || bg_pmap_height != window_height)
    {
      if (bg_pixmap)
        XFreePixmap (dpy, bg_pixmap);
      bg_pixmap = XCreatePixmap (dpy, vt, window_width, window_height, depth);
      bg_pmap_width = window_width;
      bg_pmap_height = window_height;
    }

  /* straightforward pixmap copy */
  while (sx < 0) sx += root_pmap_width;
  while (sy < 0) sy += root_pmap_height;

  gcv.tile = recoded_root_pmap;
  gcv.fill_style = FillTiled;
  gcv.ts_x_origin = -sx;
  gcv.ts_y_origin = -sy;
  gc = XCreateGC (dpy, vt, GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gcv);

  if (gc)
    {
      XFillRectangle (dpy, bg_pixmap, gc, 0, 0, window_width, window_height);
      result |= BG_IS_VALID | (bg_flags & BG_EFFECTS_FLAGS);

      if (!(bg_flags & BG_CLIENT_RENDER))
        {
          if (bg_flags & BG_NEEDS_BLUR)
            {
              if (blur_pixmap (bg_pixmap, visual, window_width, window_height, depth))
                result &= ~BG_NEEDS_BLUR;
            }
          if (bg_flags & BG_NEEDS_TINT)
            {
              if (tint_pixmap (bg_pixmap, visual, window_width, window_height))
                result &= ~BG_NEEDS_TINT;
            }
#  ifndef HAVE_AFTERIMAGE
          if (result & BG_NEEDS_TINT)
            {
              XImage *ximage = XGetImage (dpy, bg_pixmap, 0, 0, bg_pmap_width, bg_pmap_height, AllPlanes, ZPixmap);
              if (ximage)
                {
                  /* our own client-side tinting */
                  tint_ximage (DefaultVisual (dpy, display->screen), ximage);

                  XPutImage (dpy, bg_pixmap, gc, ximage, 0, 0, 0, 0, ximage->width, ximage->height);
                  XDestroyImage (ximage);
                }
            }
#  endif
        } /* server side rendering completed */

      XFreeGC (dpy, gc);
    }

  if (recoded_root_pmap != root_pixmap)
    XFreePixmap (dpy, recoded_root_pmap);

  return result;
}

void
rxvt_term::bg_set_root_pixmap ()
{
  Pixmap new_root_pixmap = get_pixmap_property (xa[XA_XROOTPMAP_ID]);
  if (new_root_pixmap == None)
    new_root_pixmap = get_pixmap_property (xa[XA_ESETROOT_PMAP_ID]);

  root_pixmap = new_root_pixmap;
}
# endif /* ENABLE_TRANSPARENCY */

bool
rxvt_term::bg_render ()
{
  unsigned long tr_flags = 0;

  bg_invalidate ();
# ifdef ENABLE_TRANSPARENCY
  if (bg_flags & BG_IS_TRANSPARENT)
    {
      /*  we need to re-generate transparency pixmap in that case ! */
      tr_flags = make_transparency_pixmap ();
      if (tr_flags)
        bg_flags |= BG_IS_VALID;
    }
# endif

# ifdef BG_IMAGE_FROM_FILE
  if ((bg_flags & BG_IS_FROM_FILE)
      || (tr_flags & BG_EFFECTS_FLAGS))
    {
      if (render_image (tr_flags))
        bg_flags |= BG_IS_VALID;
    }
# endif

  if (!(bg_flags & BG_IS_VALID))
    {
      if (bg_pixmap != None)
        {
          XFreePixmap (dpy, bg_pixmap);
          bg_pixmap = None;
        }
    }

  scr_recolour (false);
  bg_flags |= BG_NEEDS_REFRESH;

  bg_valid_since = ev::now ();

  return true;
}

void
rxvt_term::bg_init ()
{
#ifdef ENABLE_TRANSPARENCY
  shade = 100;
#endif

  bg_flags &= ~(BG_HAS_RENDER | BG_HAS_RENDER_CONV);
#if XRENDER
  int major, minor;
  if (XRenderQueryVersion (dpy, &major, &minor))
    bg_flags |= BG_HAS_RENDER;
  XFilters *filters = XRenderQueryFilters (dpy, vt);
  if (filters)
    {
      for (int i = 0; i < filters->nfilter; i++)
        if (!strcmp (filters->filter[i], FilterConvolution))
          bg_flags |= BG_HAS_RENDER_CONV;

      XFree (filters);
    }
#endif
}

#endif /* HAVE_BG_PIXMAP */

#if defined(ENABLE_TRANSPARENCY) && !defined(HAVE_AFTERIMAGE)
/* based on code from aterm-0.4.2 */

static inline void
fill_lut (uint32_t *lookup, uint32_t mask, int sh, unsigned short low, unsigned short high)
{
  for (int i = 0; i <= mask >> sh; i++)
    {
      uint32_t tmp;
      tmp = i * high;
      tmp += (mask >> sh) * low;
      lookup[i] = (tmp / 0xffff) << sh;
    }
}

void
rxvt_term::tint_ximage (Visual *visual, XImage *ximage)
{
  int sh_r, sh_g, sh_b;
  uint32_t mask_r, mask_g, mask_b;
  uint32_t *lookup, *lookup_r, *lookup_g, *lookup_b;
  unsigned short low;
  int host_byte_order = ecb_big_endian () ? MSBFirst : LSBFirst;

  if (visual->c_class != TrueColor || ximage->format != ZPixmap) return;

  /* for convenience */
  mask_r = visual->red_mask;
  mask_g = visual->green_mask;
  mask_b = visual->blue_mask;

  /* boring lookup table pre-initialization */
  switch (ximage->depth)
    {
      case 15:
        if ((mask_r != 0x7c00) ||
            (mask_g != 0x03e0) ||
            (mask_b != 0x001f))
          return;
        lookup = (uint32_t *) malloc (sizeof (uint32_t)*(32+32+32));
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
        lookup = (uint32_t *) malloc (sizeof (uint32_t)*(32+64+32));
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
        lookup = (uint32_t *) malloc (sizeof (uint32_t)*(256+256+256));
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
        lookup = (uint32_t *) malloc (sizeof (uint32_t)*(256+256+256));
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

  rgba c (rgba::MAX_CC, rgba::MAX_CC, rgba::MAX_CC);

  if (bg_flags & BG_TINT_SET)
    tint.get (c);

  /* prepare limits for color transformation (each channel is handled separately) */
  if (shade > 100)
    {
      c.r = c.r * (200 - shade) / 100;
      c.g = c.g * (200 - shade) / 100;
      c.b = c.b * (200 - shade) / 100;

      low = 0xffff * (shade - 100) / 100;
    }
  else
    {
      c.r = c.r * shade / 100;
      c.g = c.g * shade / 100;
      c.b = c.b * shade / 100;

      low = 0;
    }

  /* fill our lookup tables */
  fill_lut (lookup_r, mask_r, sh_r, low, c.r);
  fill_lut (lookup_g, mask_g, sh_g, low, c.g);
  fill_lut (lookup_b, mask_b, sh_b, low, c.b);

  /* apply table to input image (replacing colors by newly calculated ones) */
  if (ximage->bits_per_pixel == 32
      && (ximage->depth == 24 || ximage->depth == 32)
      && ximage->byte_order == host_byte_order)
    {
      uint32_t *p1, *pf, *p, *pl;
      p1 = (uint32_t *) ximage->data;
      pf = (uint32_t *) (ximage->data + ximage->height * ximage->bytes_per_line);

      while (p1 < pf)
        {
          p = p1;
          pl = p1 + ximage->width;
          for (; p < pl; p++)
            {
              *p = lookup_r[(*p & 0xff0000) >> 16] |
                   lookup_g[(*p & 0x00ff00) >> 8] |
                   lookup_b[(*p & 0x0000ff)] |
                   (*p & 0xff000000);
            }
          p1 = (uint32_t *) ((char *) p1 + ximage->bytes_per_line);
        }
    }
  else
    {
      for (int y = 0; y < ximage->height; y++)
        for (int x = 0; x < ximage->width; x++)
          {
            unsigned long pixel = XGetPixel (ximage, x, y);
            pixel = lookup_r[(pixel & mask_r) >> sh_r] |
                    lookup_g[(pixel & mask_g) >> sh_g] |
                    lookup_b[(pixel & mask_b) >> sh_b];
            XPutPixel (ximage, x, y, pixel);
          }
    }

  free (lookup);
}
#endif /* defined(ENABLE_TRANSPARENCY) && !defined(HAVE_AFTERIMAGE) */
