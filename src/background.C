/*----------------------------------------------------------------------*
 * File:	background.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2008 Marc Lehmann <schmorp@schmorp.de>
 * Copyright (c) 2007      Sasha Vasko <sasha@aftercode.net>
 * Copyright (c) 2010-2012 Emanuele Giaquinta <e.giaquinta@glauco.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifdef HAVE_BG_PIXMAP

void
rxvt_term::bg_destroy ()
{
# if BG_IMAGE_FROM_ROOT
  delete root_img;
  root_img = 0;
# endif

# if BG_IMAGE_FROM_FILE
  fimage.destroy ();
# endif
}

bool
rxvt_term::bg_window_size_sensitive ()
{
# if BG_IMAGE_FROM_ROOT
  if (root_img)
    return true;
# endif

# if BG_IMAGE_FROM_FILE
  if (fimage.img)
    {
      if ((fimage.flags & IM_IS_SIZE_SENSITIVE)
          || fimage.img->w > szHint.width
          || fimage.img->h > szHint.height)
        return true;
    }
# endif

  return false;
}

bool
rxvt_term::bg_window_position_sensitive ()
{
# if BG_IMAGE_FROM_ROOT
  if (root_img)
    return true;
# endif

# if BG_IMAGE_FROM_FILE
  if (fimage.img)
    {
      if (fimage.flags & IM_ROOT_ALIGN)
        return true;
    }
# endif

  return false;
}

# if BG_IMAGE_FROM_FILE
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

  if (is_size_sensitive ())
    flags |= IM_IS_SIZE_SENSITIVE;
  else
    flags &= ~IM_IS_SIZE_SENSITIVE;

  return changed;
}

void
rxvt_term::render_image (rxvt_image &image)
{
  int image_width = image.img->w;
  int image_height = image.img->h;
  int parent_width = szHint.width;
  int parent_height = szHint.height;
  int h_scale = min (image.h_scale, 32767 * 100 / parent_width);
  int v_scale = min (image.v_scale, 32767 * 100 / parent_height);

  int w;
  int h;
  int x;
  int y;

  w = h_scale * parent_width / 100;
  h = v_scale * parent_height / 100;

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
      x = -parent_x;
      y = -parent_y;
    }
  else
    {
      x = make_align_position (image.h_align, parent_width, w);
      y = make_align_position (image.v_align, parent_height, h);
    }

  if (!(image.flags & IM_ROOT_ALIGN)
      && (x >= parent_width
          || y >= parent_height
          || x + w <= 0
          || y + h <= 0))
    return;

  rxvt_img *img = image.img->scale (w, h);

  if (image.flags & IM_TILE)
    img->repeat_mode (RepeatNormal);
  else
    img->repeat_mode (RepeatNone);
  img->sub_rect (-x, -y, parent_width, parent_height)->replace (img);

  if (bg_img)
    img->draw (bg_img, PictOpOver, image.alpha * 1. / 0xffff);

  XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);
  img->convert_format (format, pix_colors [Color_bg])->replace (img);

  delete bg_img;
  bg_img = img;
}

rxvt_image::rxvt_image ()
{
  alpha = 0xffff;
  flags = 0;
  h_scale =
  v_scale = defaultScale;
  h_align =
  v_align = defaultAlign;

  img = 0;
}

void
rxvt_image::set_file_geometry (rxvt_screen *s, const char *file)
{
  if (!file || !*file)
    return;

  const char *p = strchr (file, ';');

  if (p)
    {
      size_t len = p - file;
      char *f = rxvt_temp_buf<char> (len + 1);
      memcpy (f, file, len);
      f[len] = '\0';
      file = f;
    }

  set_file (s, file);
  alpha = 0x8000;
  set_geometry (p ? p + 1 : "");
}

void
rxvt_image::set_file (rxvt_screen *s, const char *file)
{
  rxvt_img *img2 = rxvt_img::new_from_file (s, file);
  delete img;
  img = img2;
}

# endif /* BG_IMAGE_FROM_FILE */

bool
image_effects::set_blur (const char *geom)
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
image_effects::set_tint (const rxvt_color &new_tint)
{
  if (!tint_set || tint != new_tint)
    {
      tint = new_tint;
      tint_set = true;

      return true;
    }

  return false;
}

bool
image_effects::set_shade (const char *shade_str)
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

# if BG_IMAGE_FROM_ROOT
/*
 * Builds a pixmap of the same size as the terminal window that contains
 * the tiled portion of the root pixmap that is supposed to be covered by
 * our window.
 */
void
rxvt_term::render_root_image ()
{
  /* root dimensions may change from call to call - but Display structure should
   * be always up-to-date, so let's use it :
   */
  int screen = display->screen;
  int root_width = DisplayWidth (dpy, screen);
  int root_height = DisplayHeight (dpy, screen);
  int parent_width = szHint.width;
  int parent_height = szHint.height;
  int sx, sy;

  sx = parent_x;
  sy = parent_y;

  /* check if we are outside of the visible part of the virtual screen : */
  if (sx + parent_width <= 0 || sy + parent_height <= 0
      || sx >= root_width || sy >= root_height)
    return;

  while (sx < 0) sx += root_img->w;
  while (sy < 0) sy += root_img->h;

  rxvt_img *img = root_img->sub_rect (sx, sy, parent_width, parent_height);

  if (root_effects.need_blur ())
    img->blur (root_effects.h_blurRadius, root_effects.v_blurRadius)->replace (img);

  if (root_effects.need_tint ())
    {
      rgba c (rgba::MAX_CC, rgba::MAX_CC, rgba::MAX_CC);

      if (root_effects.tint_set)
        root_effects.tint.get (c);
      rxvt_img::nv factor = root_effects.shade / 100. - 1.;
      img->shade (factor, c)->replace (img);
    }

  XRenderPictFormat *format = XRenderFindVisualFormat (dpy, visual);
  img->convert_format (format, pix_colors [Color_bg])->replace (img);

  delete bg_img;
  bg_img = img;
}
# endif /* BG_IMAGE_FROM_ROOT */

void
rxvt_term::bg_render ()
{
  if (bg_flags & BG_INHIBIT_RENDER)
    return;

  delete bg_img;
  bg_img = 0;
  bg_flags = 0;

  if (!mapped)
    return;

# if BG_IMAGE_FROM_ROOT
  if (root_img)
    {
      render_root_image ();
      bg_flags |= BG_IS_TRANSPARENT;
    }
# endif

# if BG_IMAGE_FROM_FILE
  if (fimage.img)
    render_image (fimage);
# endif

  scr_recolor (false);
  bg_flags |= BG_NEEDS_REFRESH;

  bg_valid_since = ev::now ();
}

void
rxvt_term::bg_init ()
{
#if BG_IMAGE_FROM_ROOT
  if (option (Opt_transparent))
    {
      if (rs [Rs_blurradius])
        root_effects.set_blur (rs [Rs_blurradius]);

      if (ISSET_PIXCOLOR (Color_tint))
        root_effects.set_tint (pix_colors_focused [Color_tint]);

      if (rs [Rs_shade])
        root_effects.set_shade (rs [Rs_shade]);

      rxvt_img::new_from_root (this)->replace (root_img);
      XSelectInput (dpy, display->root, PropertyChangeMask);
      rootwin_ev.start (display, display->root);
    }
#endif

#if BG_IMAGE_FROM_FILE
  if (rs[Rs_backgroundPixmap])
    {
      fimage.set_file_geometry (this, rs[Rs_backgroundPixmap]);
      if (!bg_window_position_sensitive ())
        update_background ();
    }
#endif
}

#endif /* HAVE_BG_PIXMAP */
