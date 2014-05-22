/*----------------------------------------------------------------------*
 * File:	rxvtimg.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2012      Marc Lehmann <schmorp@schmorp.de>
 * Copyright (c) 2012      Emanuele Giaquinta <e.giaquinta@glauco.it>
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

#ifndef IMG_H
#define IMG_H

#if HAVE_IMG

#define float_to_component(d) (int32_t)((d) * 65535.99)

#include <X11/extensions/Xrender.h>

struct rxvt_img
{
  typedef double nv;

  // *could* also hold the Pixmap itself
  struct pixref
  {
    int cnt;
    int w, h;
    bool ours; // false if we don't own the pixmap at all

    pixref (int w, int h)
    : cnt(1), w(w), h(h), ours(true)
    {
    }
  };

  rxvt_screen *s;
  Pixmap pm;
  pixref *ref; // shared refcnt
  int x, y, w, h, repeat;
  XRenderPictFormat *format;

  rxvt_img (rxvt_screen *screen, XRenderPictFormat *format, int x, int y, int width, int height, int repeat = RepeatNormal);
  rxvt_img (const rxvt_img &img);
  void alloc ();

# if HAVE_PIXBUF
  static rxvt_img *new_from_pixbuf (rxvt_screen *s, GdkPixbuf *pb); // from pixbuf
  static rxvt_img *new_from_file (rxvt_screen *s, const char *filename); // via pixbuf
# endif
  static rxvt_img *new_from_root (rxvt_screen *s); // get root pixmap

  ~rxvt_img ();

  Pixmap steal ()
  {
    ref->ours = false;
    return pm;
  }

  // inplace
  void move (int dx, int dy)
  {
    x += dx;
    y += dy;
  }

  void repeat_mode (int repeat)
  {
    this->repeat = repeat;
  }

  void unshare (); // prepare for write
  void fill (const rgba &c);
  void fill (const rgba &c, int x, int y, int w, int h);
  void add_alpha ();
  //void linear_gradient  (const XLinearGradient  *gradient, const XFixed *stops, const XRenderColor *colors, int nstops);
  //void radial_gradient  (const XRadialGradient  *gradient, const XFixed *stops, const XRenderColor *colors, int nstops);
  //void conical_gradient (const XConicalGradient *gradient, const XFixed *stops, const XRenderColor *colors, int nstops);

  void brightness (int32_t r, int32_t g, int32_t b, int32_t a);
  void contrast (int32_t r, int32_t g, int32_t b, int32_t a);

  void brightness (nv r, nv g, nv b, nv a = 1.)
  {
    brightness (float_to_component (r),
                float_to_component (g),
                float_to_component (b),
                float_to_component (a));
  }

  void contrast (nv r, nv g, nv b, nv a = 1.)
  {
    contrast (float_to_component (r),
              float_to_component (g),
              float_to_component (b),
              float_to_component (a));
  }

  void draw (rxvt_img *img, int op = PictOpOver, nv mask = 1.);
#if 0
  void draw (rxvt_img *img, int op = PictOpOver, nv mask = 1.,
             nv px, nv py, nv qx, nv qy, nv rx, nv ry, nv sx, nv sy);
#endif

  // copy
  rxvt_img *reify (); // make x, y 0, make real width/height
  rxvt_img *blur (int rh, int rv);
  rxvt_img *clone ();
  rxvt_img *sub_rect (int x, int y, int width, int height);
  rxvt_img *transform (const nv matrix[3][3]);
  rxvt_img *scale (int new_width, int new_height);
  rxvt_img *rotate (int cx, int cy, nv phi);
  rxvt_img *convert_format (XRenderPictFormat *format, const rgba &bg);
  rxvt_img *tint (const rgba &c);
  rxvt_img *shade (nv factor, rgba c = rgba (rgba::MAX_CC, rgba::MAX_CC, rgba::MAX_CC));
  rxvt_img *filter (const char *name, int nparams = 0, nv *params = 0);
  rxvt_img *muladd (nv mul, nv add); // general multiply and add, implemented as the biggest hack ever :/

  // egregious helper category
  rxvt_img *replace (rxvt_img *&p)
  {
    delete p;
    p = this;
    return this;
  }

  /* these are considered private */
  void destroy ();
  rxvt_img *new_empty ();
  Picture picture ();
  rxvt_img *transform (const nv *matrix);
};

#endif

#endif

