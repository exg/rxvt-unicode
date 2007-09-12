/*
 * background.h
 */

#ifndef BACKGROUND_H_
#define BACKGROUND_H_

#ifdef HAVE_AFTERIMAGE
#  include <afterimage.h>
#endif

#if defined(XPM_BACKGROUND) || defined(ENABLE_TRANSPARENCY)
# define HAVE_BG_PIXMAP 1/* to simplify further usage */
struct  bgPixmap_t {

  enum {
    geometrySet     = (1UL<<0),
    propScale       = (1UL<<1),
    geometryFlags   = (geometrySet|propScale),

    tintSet         = (1UL<<8),
    tintNeeded      = (1UL<<9),
    tintWholesome   = (1UL<<10),
    tintServerSide  = (1UL<<11),
    tintFlags       = (tintSet|tintServerSide|tintNeeded|tintWholesome),
    blurNeeded      = (1UL<<12),
    blurServerSide  = (1UL<<13), /* this don't work yet */

    isTransparent   = (1UL<<16),
    isInvalid       = (1UL<<17),
    isVtOrigin      = (1UL<<18)  /* if set pixmap has origin at corner of
                                    vt window instead of parent[0]! */
  };

  unsigned long flags;

  enum {
    transpPmapTiled = (1UL<<0),
    transpPmapTinted = tintNeeded,
    transpPmapBlured = blurNeeded,
    transpTransformations = (tintNeeded|blurNeeded)
  }; /* this flags are returned by make_transparency_pixmap if called */

# ifdef  XPM_BACKGROUND
#  ifdef HAVE_AFTERIMAGE
  ASImage *original_asim;
  bool render_asim (ASImage *background, ARGB32 background_tint);
#  endif

  enum { defaultScale = 100, defaultAlign = 50 };

  unsigned int h_scale, v_scale;/* percents of the window size */
  int h_align, v_align;         /* percents of the window size:
                                  0 - left align, 50 - center, 100 - right */
  void unset_geometry () { flags = flags & ~geometryFlags; };
  bool set_geometry (const char *geom);
  void set_defaultGeometry ()
  {
    h_scale = v_scale = defaultScale;
    h_align = v_align = defaultAlign;
    flags |= geometrySet;
  };

  bool set_file (const char *file);
# endif /* XPM_BACKGROUND */

  rxvt_term *target;
  bool set_target (rxvt_term *new_target);

# ifdef ENABLE_TRANSPARENCY
  int         root_depth; /* obtained when target is set */
  Pixmap      root_pixmap; /* current root pixmap set */
  rxvt_color  tint;
  int         shade;
  int         h_blurRadius, v_blurRadius;

  bool set_transparent ();
  bool set_blur_radius (const char *geom);
  bool set_tint (rxvt_color &new_tint);
  bool unset_tint ();
  bool set_shade (const char *shade_str);
  bool set_root_pixmap ();

  unsigned long make_transparency_pixmap ();/* returns combination of the transpTransformations flags */
# endif
  double invalid_since;

  Pixmap pixmap;
  unsigned int pmap_width, pmap_height;
  unsigned int pmap_depth;

  bool window_size_sensitive ();
  bool window_position_sensitive () {
    return (flags & isTransparent);
  };

  bool is_parentOrigin () {
    return !(flags & isVtOrigin);
  };

  bool need_client_side_rendering ();
  void apply ();
  bool render ();
  void invalidate () {
    if (!(flags & isInvalid))
      { 
        flags |= isInvalid; 
        invalid_since = NOW;
      }
  };
};
#else
# undef HAVE_BG_PIXMAP
#endif



#endif	/* _BACKGROUND_H_ */
