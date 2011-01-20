/*
 * background.h
 */

#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#ifdef HAVE_AFTERIMAGE
# include <afterimage.h>
# undef min
# undef max
#endif

#ifdef HAVE_PIXBUF
#include <gdk-pixbuf/gdk-pixbuf.h>
#endif

#if defined(BG_IMAGE_FROM_FILE) || defined(ENABLE_TRANSPARENCY)
# define HAVE_BG_PIXMAP 1 /* to simplify further usage */
struct bgPixmap_t
{
  bgPixmap_t ();
  void destroy ();

  enum {
    propScale       = 1 <<  1,
    rootAlign       = 1 <<  2,
    geometryFlags   = propScale | rootAlign,

    tintSet         = 1 <<  8,
    tintNeeded      = 1 <<  9,
    tintWholesome   = 1 << 10,
    tintFlags       = tintSet | tintNeeded | tintWholesome,

    blurNeeded      = 1 << 11,

    HAS_RENDER      = 1 << 12,
    HAS_RENDER_CONV = 1 << 13,
    CLIENT_RENDER   = 1 << 14,

    isTransparent   = 1 << 16,
    isInvalid       = 1 << 17,
    hasChanged      = 1 << 19,
    sizeSensitive   = 1 << 20,
  };

  unsigned int flags;

  enum {
    transpPmapTiled       = 1 << 0,
    transpPmapTinted      = tintNeeded,
    transpPmapBlurred     = blurNeeded,
    transpTransformations = tintNeeded | blurNeeded,
  }; /* these flags are returned by make_transparency_pixmap if called */

# ifdef  BG_IMAGE_FROM_FILE
#  ifdef HAVE_AFTERIMAGE
  ASImage *original_asim;
#  endif

#  ifdef HAVE_PIXBUF
  GdkPixbuf *pixbuf;
  bool pixbuf_to_pixmap (GdkPixbuf *pixbuf, Pixmap pixmap, GC gc,
                         int src_x, int src_y, int dst_x, int dst_y,
                         unsigned int width, unsigned int height);
#  endif

  void get_image_geometry (int image_width, int image_height, int &w, int &h, int &x, int &y);
  bool render_image (unsigned long background_flags);
  bool have_image;

  enum {
    noScale = 0,
    windowScale = 100,
    defaultScale = windowScale,
    centerAlign = 50,
    defaultAlign = centerAlign,
  };

  unsigned int h_scale, v_scale;/* percents of the window size */
  int h_align, v_align;         /* percents of the window size:
                                  0 - left align, 50 - center, 100 - right */
  bool set_geometry (const char *geom, bool update = false);
  void set_defaultGeometry ()
  {
    h_scale = v_scale = defaultScale;
    h_align = v_align = defaultAlign;
  }

  bool set_file (const char *file);
# endif /* BG_IMAGE_FROM_FILE */

  rxvt_term *target;
  void set_target (rxvt_term *new_target);

# ifdef ENABLE_TRANSPARENCY
  Pixmap      root_pixmap; /* current root pixmap set */
  rxvt_color  tint;
  int         shade;
  int         h_blurRadius, v_blurRadius;

  bool set_transparent ();
  bool set_blur_radius (const char *geom);
  bool set_tint (rxvt_color &new_tint);
  bool set_shade (const char *shade_str);
  bool blur_pixmap (Pixmap pixmap, Visual *visual, int width, int height);
  bool tint_pixmap (Pixmap pixmap, Visual *visual, int width, int height);
  void set_root_pixmap ();

  unsigned long make_transparency_pixmap ();/* returns combination of the transpTransformations flags */
# endif
  double valid_since;

  Pixmap pixmap;
  unsigned int pmap_width, pmap_height;

  int target_x;
  int target_y;
  bool set_position (int x, int y);
  bool window_size_sensitive ();
  bool window_position_sensitive ();

  bool render ();
  void invalidate ()
  {
    flags |= isInvalid;
  }
};
#else
# undef HAVE_BG_PIXMAP
#endif



#endif /* _BACKGROUND_H_ */
