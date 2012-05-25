/*----------------------------------------------------------------------*
 * File:	background.C - former xpm.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2008 Marc Lehmann <schmorp@schmorp.de>
 * Copyright (c) 2007      Sasha Vasko <sasha@aftercode.net>
 * Copyright (c) 2010-2012 Emanuele Giaquinta <e.giaquinta@glauco.it>
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
  for (vector<rxvt_image>::iterator bg_image = image_vec.begin (); bg_image < image_vec.end (); bg_image++)
    {
      if ((bg_image->flags & IM_IS_SIZE_SENSITIVE)
          || bg_image->width () > szHint.width
          || bg_image->height () > szHint.height)
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
  for (vector<rxvt_image>::iterator bg_image = image_vec.begin (); bg_image < image_vec.end (); bg_image++)
    {
      if (bg_image->flags & IM_ROOT_ALIGN)
        return true;
    }
# endif

  return false;
}

# ifdef BG_IMAGE_FROM_FILE
static inline int
make_align_position (int align, int window_size, int image_size)
{
  if (align >= 0 && align <= 100)
    return lerp (0, window_size - image_size, align);
  else if (align > 100)
    return lerp (window_size - image_size, window_size, align - 100);
  else
    return lerp (-image_size, 0, align + 100);
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

static void
parse_style (const char *style, int &x, int &y, unsigned int &w, unsigned int &h, uint8_t &flags)
{
  if (!strcasecmp (style, "tiled"))
    {
      flags = IM_TILE;
      w = h = noScale;
      x = y = 0;
    }
  else if (!strcasecmp (style, "aspect-stretched"))
    {
      flags = IM_KEEP_ASPECT;
      w = h = windowScale;
      x = y = centerAlign;
    }
  else if (!strcasecmp (style, "stretched"))
    {
      flags = 0;
      w = h = windowScale;
      x = y = centerAlign;
    }
  else if (!strcasecmp (style, "centered"))
    {
      flags = 0;
      w = h = noScale;
      x = y = centerAlign;
    }
  else if (!strcasecmp (style, "root-tiled"))
    {
      flags = IM_TILE|IM_ROOT_ALIGN;
      w = h = noScale;
      x = y = 0;
    }
}

bool
rxvt_image::set_geometry (const char *geom, bool update)
{
  bool changed = false;
  int geom_flags = 0;
  int x = h_align;
  int y = v_align;
  unsigned int w = h_scale;
  unsigned int h = v_scale;
  uint8_t new_flags = 0;

  if (geom == NULL)
    return false;

  if (geom[0])
    {
      char **arr = rxvt_strsplit (':', geom);

      for (int i = 0; arr[i]; i++)
        {
          if (!strncasecmp (arr[i], "style=", 6))
            {
              parse_style (arr[i] + 6, x, y, w, h, new_flags);
              geom_flags = WidthValue|HeightValue|XValue|YValue;
            }
          else if (!strcasecmp (arr[i], "op=tile"))
            new_flags |= IM_TILE;
          else if (!strcasecmp (arr[i], "op=keep-aspect"))
            new_flags |= IM_KEEP_ASPECT;
          else if (!strcasecmp (arr[i], "op=root-align"))
            new_flags |= IM_ROOT_ALIGN;

          // deprecated
          else if (!strcasecmp (arr[i], "tile"))
            {
              new_flags |= IM_TILE;
              w = h = noScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "propscale"))
            {
              new_flags |= IM_KEEP_ASPECT;
              w = h = windowScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "hscale"))
            {
              new_flags |= IM_TILE;
              w = windowScale;
              h = noScale;
              geom_flags |= WidthValue|HeightValue;
            }
          else if (!strcasecmp (arr[i], "vscale"))
            {
              new_flags |= IM_TILE;
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
              new_flags |= IM_TILE|IM_ROOT_ALIGN;
              w = h = noScale;
              geom_flags |= WidthValue|HeightValue;
            }

          else
            geom_flags |= XParseGeometry (arr[i], &x, &y, &w, &h);
        } /* done parsing ops */

      rxvt_free_strsplit (arr);
    }

  new_flags |= flags & ~IM_GEOMETRY_FLAGS;

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

  clamp_it (x, -100, 200);
  clamp_it (y, -100, 200);

  if (flags != new_flags
      || h_scale != w
      || v_scale != h
      || h_align != x
      || v_align != y)
    {
      flags = new_flags;
      h_scale = w;
      v_scale = h;
      h_align = x;
      v_align = y;
      changed = true;
    }

  if (!(flags & IM_TILE)
      || h_scale || v_scale
      || (!(flags & IM_ROOT_ALIGN) && (h_align || v_align)))
    flags |= IM_IS_SIZE_SENSITIVE;
  else
    flags &= ~IM_IS_SIZE_SENSITIVE;

  return changed;
}

void
rxvt_term::get_image_geometry (rxvt_image &image, int &w, int &h, int &x, int &y)
{
  int image_width = image.width ();
  int image_height = image.height ();
  int target_width = szHint.width;
  int target_height = szHint.height;
  int h_scale = min (image.h_scale, 32767 * 100 / target_width);
  int v_scale = min (image.v_scale, 32767 * 100 / target_height);

  w = h_scale * target_width / 100;
  h = v_scale * target_height / 100;

  if (image.flags & IM_KEEP_ASPECT)
    {
      float scale = (float)w / image_width;
      min_it (scale, (float)h / image_height);
      w = image_width * scale + 0.5;
      h = image_height * scale + 0.5;
    }

  if (!w) w = image_width;
  if (!h) h = image_height;

  if (image.flags & IM_ROOT_ALIGN)
    {
      x = -target_x;
      y = -target_y;
    }
  else
    {
      x = make_align_position (image.h_align, target_width, w);
      y = make_align_position (image.v_align, target_height, h);
    }
}

#  ifdef HAVE_PIXBUF
bool
rxvt_term::pixbuf_to_pixmap (GdkPixbuf *pixbuf, Pixmap pixmap, GC gc,
                             int src_x, int src_y, int dst_x, int dst_y,
                             unsigned int width, unsigned int height, bool argb)
{
  XImage *ximage;
  char *line;
  int width_r, width_g, width_b, width_a;
  int sh_r, sh_g, sh_b, sh_a;
  uint32_t red_mask, green_mask, blue_mask, alpha_mask;
  int rowstride;
  int channels;
  unsigned char *row;

  if (visual->c_class != TrueColor)
    return false;

  if (argb)
    {
      red_mask   = 0xff << 16;
      green_mask = 0xff << 8;
      blue_mask  = 0xff;
      alpha_mask = 0xff << 24;
    }
  else
    {
      red_mask   = visual->red_mask;
      green_mask = visual->green_mask;
      blue_mask  = visual->blue_mask;
#if XRENDER
      XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);
      if (format)
        alpha_mask = (uint32_t)format->direct.alphaMask << format->direct.alpha;
      else
#endif
        alpha_mask = 0;
    }

  width_r = ecb_popcount32 (red_mask);
  width_g = ecb_popcount32 (green_mask);
  width_b = ecb_popcount32 (blue_mask);
  width_a = ecb_popcount32 (alpha_mask);

  if (width_r > 8 || width_g > 8 || width_b > 8 || width_a > 8)
    return false;

  sh_r = ecb_ctz32 (red_mask);
  sh_g = ecb_ctz32 (green_mask);
  sh_b = ecb_ctz32 (blue_mask);
  sh_a = ecb_ctz32 (alpha_mask);

  if (width > 32767 || height > 32767)
    return false;

  ximage = XCreateImage (dpy, visual, argb ? 32 : depth, ZPixmap, 0, 0,
                         width, height, 32, 0);
  if (!ximage)
    return false;

  if (height > INT_MAX / ximage->bytes_per_line
      || !(ximage->data = (char *)malloc (height * ximage->bytes_per_line)))
    {
      XDestroyImage (ximage);
      return false;
    }

  ximage->byte_order = ecb_big_endian () ? MSBFirst : LSBFirst;

  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  channels = gdk_pixbuf_get_n_channels (pixbuf);
  row = gdk_pixbuf_get_pixels (pixbuf) + src_y * rowstride + src_x * channels;
  line = ximage->data;

  rgba c (0, 0, 0);

  if (channels == 4 && alpha_mask == 0)
    {
      pix_colors[Color_bg].get (c);
      c.r >>= 8;
      c.g >>= 8;
      c.b >>= 8;
    }

  for (int y = 0; y < height; y++)
    {
      for (int x = 0; x < width; x++)
        {
          unsigned char *pixel = row + x * channels;
          uint32_t value;
          unsigned char r, g, b, a;

          if (channels == 4)
            {
              a = pixel[3];
              r = (pixel[0] * a + c.r * (0xff - a)) / 0xff;
              g = (pixel[1] * a + c.g * (0xff - a)) / 0xff;
              b = (pixel[2] * a + c.b * (0xff - a)) / 0xff;
            }
          else
            {
              a = 0xff;
              r = pixel[0];
              g = pixel[1];
              b = pixel[2];
            }

          value  = ((r >> (8 - width_r)) << sh_r)
                 | ((g >> (8 - width_g)) << sh_g)
                 | ((b >> (8 - width_b)) << sh_b)
                 | ((a >> (8 - width_a)) << sh_a);

          if (ximage->bits_per_pixel == 32)
            ((uint32_t *)line)[x] = value;
          else
            XPutPixel (ximage, x, y, value);
        }

      row += rowstride;
      line += ximage->bytes_per_line;
    }

  XPutImage (dpy, pixmap, gc, ximage, 0, 0, dst_x, dst_y, width, height);
  XDestroyImage (ximage);
  return true;
}

bool
rxvt_term::render_image (rxvt_image &image)
{
  GdkPixbuf *pixbuf = image.pixbuf;
  if (!pixbuf)
    return false;

  bool need_blend = bg_flags & BG_IS_VALID;

  if (need_blend
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

  get_image_geometry (image, w, h, x, y);

  if (!(image.flags & IM_ROOT_ALIGN)
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
  Pixmap tmp_pixmap;

  image_width = gdk_pixbuf_get_width (result);
  image_height = gdk_pixbuf_get_height (result);

  if (need_blend)
    tmp_pixmap = XCreatePixmap (dpy, vt, new_pmap_width, new_pmap_height, 32);
  else
    {
      // optimise bg pixmap size when tiling, but only if there are no
      // other pixbufs to render. Otherwise, the bg pixmap size must
      // be equal to the window size.
      if ((image.flags & IM_TILE)
          && image_vec.size () == 1)
        {
          new_pmap_width = min (image_width, target_width);
          new_pmap_height = min (image_height, target_height);
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

      tmp_pixmap = bg_pixmap;
    }

  gcv.foreground = pix_colors[Color_bg];
  gc = XCreateGC (dpy, tmp_pixmap, GCForeground, &gcv);

  if (gc)
    {
      if (image.flags & IM_TILE)
        {
          Pixmap tile = XCreatePixmap (dpy, vt, image_width, image_height, need_blend ? 32 : depth);
          pixbuf_to_pixmap (result, tile, gc,
                            0, 0,
                            0, 0,
                            image_width, image_height, need_blend);

          gcv.tile = tile;
          gcv.fill_style = FillTiled;
          gcv.ts_x_origin = x;
          gcv.ts_y_origin = y;
          XChangeGC (dpy, gc, GCFillStyle | GCTile | GCTileStipXOrigin | GCTileStipYOrigin, &gcv);

          XFillRectangle (dpy, tmp_pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);
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
            XFillRectangle (dpy, tmp_pixmap, gc, 0, 0, new_pmap_width, new_pmap_height);

          if (dst_x < new_pmap_width && dst_y < new_pmap_height)
            pixbuf_to_pixmap (result, tmp_pixmap, gc,
                              src_x, src_y,
                              dst_x, dst_y,
                              dst_width, dst_height, need_blend);
        }

#if XRENDER
      if (need_blend)
        {
          XRenderPictFormat *argb_format = XRenderFindStandardFormat (dpy, PictStandardARGB32);
          XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);

          Picture src = XRenderCreatePicture (dpy, tmp_pixmap, argb_format, 0, 0);

          Picture dst = XRenderCreatePicture (dpy, bg_pixmap, format, 0, 0);

          Picture mask = create_xrender_mask (dpy, vt, False, False);

          XRenderColor mask_c;

          mask_c.alpha = gdk_pixbuf_get_has_alpha (image.pixbuf) ? 0xffff : image.alpha;
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

  if (need_blend)
    XFreePixmap (dpy, tmp_pixmap);

  return ret;
}
#  endif /* HAVE_PIXBUF */

#  ifndef NO_RESOURCES
static int
rxvt_define_image (XrmDatabase *database ecb_unused,
                   XrmBindingList bindings ecb_unused,
                   XrmQuarkList quarks,
                   XrmRepresentation *type ecb_unused,
                   XrmValue *value,
                   XPointer closure ecb_unused)
{
  int size;

  for (size = 0; quarks[size] != NULLQUARK; size++)
    ;

  if (size >= 2)
    {
      int id = strtol (XrmQuarkToString (quarks[size-2]), 0, 0);
      if (id >= 1)
        GET_R->parse_image (id, XrmQuarkToString (quarks[size-1]), (char *)value->addr);
    }
  return False;
}

void
rxvt_term::parse_image (int id, const char *type, const char *arg)
{
  rxvt_image *image;

  for (image = image_vec.begin (); image < image_vec.end (); image++)
    if (image->id == id)
      break;

  if (image == image_vec.end ())
    {
      image = new_image ();
      image->id = id;
    }
}
#  endif

rxvt_image::rxvt_image ()
{
  id      =
  alpha   =
  flags   =
  h_scale =
  v_scale =
  h_align =
  v_align = 0;

#  ifdef HAVE_PIXBUF
  pixbuf.reset (0);
#  endif
}

bool
rxvt_image::set_file_geometry (const char *file)
{
  if (!file || !*file)
    return false;

  const char *p = strchr (file, ';');

  if (p)
    {
      size_t len = p - file;
      char *f = rxvt_temp_buf<char> (len + 1);
      memcpy (f, file, len);
      f[len] = '\0';
      file = f;
    }

  bool ret = set_file (file);
  alpha = 0x8000;
  if (ret && p)
    set_geometry (p + 1);
  return ret;
}

bool
rxvt_image::set_file (const char *file)
{
  bool ret = false;

#  ifdef HAVE_PIXBUF
  GdkPixbuf *image = gdk_pixbuf_new_from_file (file, NULL);
  if (image)
    {
      if (pixbuf)
        g_object_unref (pixbuf);
      pixbuf.reset (image);
      ret = true;
    }
#  endif

  if (ret)
    {
      alpha = 0xffff;
      flags = IM_IS_SET | IM_IS_SIZE_SENSITIVE;
      h_scale = v_scale = defaultScale;
      h_align = v_align = defaultAlign;
    }

  return ret;
}

# endif /* BG_IMAGE_FROM_FILE */

# ifdef ENABLE_TRANSPARENCY
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

  return changed;
}

bool
rxvt_term::bg_set_tint (rxvt_color &new_tint)
{
  if (!(bg_flags & BG_TINT_SET) || tint != new_tint)
    {
      tint = new_tint;
      bg_flags |= BG_TINT_SET;

      rgba c;
      tint.get (c);
      if ((c.r <= 0x00ff || c.r >= 0xff00)
          && (c.g <= 0x00ff || c.g >= 0xff00)
          && (c.b <= 0x00ff || c.b >= 0xff00))
        bg_flags |= BG_TINT_BITAND;
      else
        bg_flags &= ~BG_TINT_BITAND;

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
rxvt_term::blur_pixmap (Pixmap pixmap, int width, int height)
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
rxvt_term::tint_pixmap (Pixmap pixmap, int width, int height)
{
  bool ret = false;

  if (shade == 100 && (bg_flags & BG_TINT_BITAND))
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
bool
rxvt_term::make_transparency_pixmap ()
{
  bool ret = false;

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
      ret = true;
      bool need_blur = h_blurRadius && v_blurRadius;
      bool need_tint = shade != 100 || (bg_flags & BG_TINT_SET);

      if (!(bg_flags & BG_CLIENT_RENDER))
        {
          if (need_blur)
            {
              if (blur_pixmap (bg_pixmap, window_width, window_height))
                need_blur = false;
            }
          if (need_tint)
            {
              if (tint_pixmap (bg_pixmap, window_width, window_height))
                need_tint = false;
            }
          if (need_tint)
            {
              XImage *ximage = XGetImage (dpy, bg_pixmap, 0, 0, bg_pmap_width, bg_pmap_height, AllPlanes, ZPixmap);
              if (ximage)
                {
                  /* our own client-side tinting */
                  tint_ximage (ximage);

                  XPutImage (dpy, bg_pixmap, gc, ximage, 0, 0, 0, 0, ximage->width, ximage->height);
                  XDestroyImage (ximage);
                }
            }
        } /* server side rendering completed */

      XFreeGC (dpy, gc);
    }

  if (recoded_root_pmap != root_pixmap)
    XFreePixmap (dpy, recoded_root_pmap);

  return ret;
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
  bg_invalidate ();
# ifdef ENABLE_TRANSPARENCY
  if (bg_flags & BG_IS_TRANSPARENT)
    {
      /*  we need to re-generate transparency pixmap in that case ! */
      if (make_transparency_pixmap ())
        bg_flags |= BG_IS_VALID;
    }
# endif

# ifdef BG_IMAGE_FROM_FILE
  for (vector<rxvt_image>::iterator bg_image = image_vec.begin (); bg_image < image_vec.end (); bg_image++)
    {
      if (render_image (*bg_image))
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

#ifdef BG_IMAGE_FROM_FILE
  if (rs[Rs_backgroundPixmap])
    {
      rxvt_image *image = new_image ();
      if (!image->set_file_geometry (rs[Rs_backgroundPixmap]))
        image_vec.pop_back ();
    }

# ifndef NO_RESOURCES
  find_resources ("image", "Image", XrmEnumAllLevels, rxvt_define_image);
  vector<rxvt_image>::iterator bg_image = image_vec.begin ();
  while (bg_image != image_vec.end ())
    {
      if (!(bg_image->flags & IM_IS_SET))
        bg_image = image_vec.erase (bg_image);
      else
        bg_image++;
    }
# endif

  if (image_vec.size () > 0
      && !bg_window_position_sensitive ())
    update_background ();
#endif
}

#endif /* HAVE_BG_PIXMAP */

#ifdef ENABLE_TRANSPARENCY
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
rxvt_term::tint_ximage (XImage *ximage)
{
  unsigned int size_r, size_g, size_b;
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
  sh_r = ecb_ctz32 (mask_r);
  sh_g = ecb_ctz32 (mask_g);
  sh_b = ecb_ctz32 (mask_b);

  size_r = mask_r >> sh_r;
  size_g = mask_g >> sh_g;
  size_b = mask_b >> sh_b;

  if (size_r++ > 255 || size_g++ > 255 || size_b++ > 255)
    return;

  lookup = (uint32_t *)malloc (sizeof (uint32_t) * (size_r + size_g + size_b));
  lookup_r = lookup;
  lookup_g = lookup + size_r;
  lookup_b = lookup + size_r + size_g;

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
      && ximage->byte_order == host_byte_order)
    {
      char *line = ximage->data;

      for (int y = 0; y < ximage->height; y++)
        {
          uint32_t *p = (uint32_t *)line;
          for (int x = 0; x < ximage->width; x++)
            {
              *p = lookup_r[(*p & mask_r) >> sh_r] |
                   lookup_g[(*p & mask_g) >> sh_g] |
                   lookup_b[(*p & mask_b) >> sh_b];
              p++;
            }
          line += ximage->bytes_per_line;
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
#endif /* ENABLE_TRANSPARENCY */
