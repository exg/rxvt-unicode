#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <X11/Xlib.h>
#include "rxvtutil.h"

struct rxvt_term;

#define SB_WIDTH_NEXT           19
#define SB_WIDTH_XTERM          15
#define SB_WIDTH_PLAIN          7
#ifndef SB_WIDTH_RXVT
# define SB_WIDTH_RXVT          10
#endif

/*
 * NeXT scrollbar defines
 */
#define SB_PADDING              1
#define SB_BORDER_WIDTH         1
#define SB_BEVEL_WIDTH_UPPER_LEFT       1
#define SB_BEVEL_WIDTH_LOWER_RIGHT      2
#define SB_LEFT_PADDING         (SB_PADDING + SB_BORDER_WIDTH)
#define SB_MARGIN_SPACE         (SB_PADDING * 2)
#define SB_BUTTON_WIDTH         (SB_WIDTH_NEXT - SB_MARGIN_SPACE - SB_BORDER_WIDTH)
#define SB_BUTTON_HEIGHT        (SB_BUTTON_WIDTH)
#define SB_BUTTON_SINGLE_HEIGHT (SB_BUTTON_HEIGHT + SB_PADDING)
#define SB_BUTTON_BOTH_HEIGHT   (SB_BUTTON_SINGLE_HEIGHT * 2)
#define SB_BUTTON_TOTAL_HEIGHT  (SB_BUTTON_BOTH_HEIGHT + SB_PADDING)
#define SB_BUTTON_BEVEL_X       (SB_LEFT_PADDING)
#define SB_BUTTON_FACE_X        (SB_BUTTON_BEVEL_X + SB_BEVEL_WIDTH_UPPER_LEFT)
#define SB_THUMB_MIN_HEIGHT     (SB_BUTTON_WIDTH - (SB_PADDING * 2))

enum sb_state {
  SB_STATE_OFF,
  SB_STATE_IDLE,
  SB_STATE_MOTION,
  SB_STATE_UP,
  SB_STATE_DOWN,
};

enum sb_style {
  SB_STYLE_NEXT  = 1,
  SB_STYLE_XTERM = 2,
  SB_STYLE_PLAIN = 4,
  SB_STYLE_RXVT  = 8,
};

enum sb_align {
  SB_ALIGN_CENTRE,
  SB_ALIGN_TOP,
  SB_ALIGN_BOTTOM,
};

struct scrollBar_t
{
  rxvt_term *term;
  sb_state        state;        /* scrollbar state                          */
  char            init;         /* scrollbar has been initialised           */
  int             beg;          /* slider sub-window begin height           */
  int             end;          /* slider sub-window end height             */
  int             top;          /* slider top position                      */
  int             bot;          /* slider bottom position                   */
  sb_style        style;        /* style: rxvt, xterm, next                 */
  int             width;        /* scrollbar width                          */
  int             shadow;       /* scrollbar shadow width                   */
  int             last_bot;     /* scrollbar last bottom position           */
  int             last_top;     /* scrollbar last top position              */
  int             last_state;   /* scrollbar last state                     */
  sb_align        align;
  Window          win;
  Cursor          leftptr_cursor;
  int             (scrollBar_t::*update)(int);
  void setup (rxvt_term *);
  void resize ();
  void map (int);
  int show (int);
  void destroy ();

  bool upButton (int y)
  {
    if (style == SB_STYLE_NEXT)
      return y > end && y <= end + width + 1;
    if (style == SB_STYLE_RXVT)
      return y < beg;
    return false;
  }
  bool dnButton (int y)
  {
    if (style == SB_STYLE_NEXT)
      return y > end + width + 1;
    if (style == SB_STYLE_RXVT)
      return y > end;
    return false;
  }
  int min_height ()
  {
    return style == SB_STYLE_NEXT ? SB_THUMB_MIN_HEIGHT : 10;
  }
  int size ()
  {
    return max (end - beg, 0);
  }
  int total_width ()
  {
    return width + shadow * 2;
  }
  bool above_slider (int y)
  {
    return y < top;
  }
  bool below_slider (int y)
  {
    return y > bot;
  }
  int position (int y)
  {
    return y - beg;
  }


#if defined(NEXT_SCROLLBAR)
  GC              blackGC,
                  whiteGC,
                  grayGC,
                  darkGC,
                  stippleGC;
  Pixmap          dimple,
                  upArrow,
                  downArrow,
                  upArrowHi,
                  downArrowHi;
#endif

#if defined(RXVT_SCROLLBAR)
  GC              scrollbarGC,
                  topShadowGC,
                  botShadowGC;
#endif

#if defined(XTERM_SCROLLBAR)
  GC              xscrollbarGC,
                  ShadowGC;
#endif

#if defined(PLAIN_SCROLLBAR)
  GC              pscrollbarGC;
#endif

private:
  // update style dependent data
  void update_data ();

  // scrollbar-next.C
  int show_next (int);
  // scrollbar-rxvt.C
  int show_rxvt (int);
  // scrollbar-xterm.C
  int show_xterm (int);
  // scrollbar-plain.C
  int show_plain (int);

  void init_next ();
};

 /*
  *    +-------------+
  *    |             | <---< SB_PADDING
  *    | ::::::::::: |
  *    | ::::::::::: |
  *   '''''''''''''''''
  *   ,,,,,,,,,,,,,,,,,
  *    | ::::::::::: |
  *    | ::::::::::: |
  *    |  +---------------< SB_BEVEL_WIDTH_UPPER_LEFT
  *    |  | :::::::: |
  *    |  V :::: vv-------< SB_BEVEL_WIDTH_LOWER_RIGHT
  *    | +---------+ |
  *    | | ......%%| |
  *    | | ......%%| |
  *    | | .. ()..%%| |
  *    | | ......%%| |
  *    | | %%%%%%%%| |
  *    | +---------+ | <.........................
  *    |             | <---< SB_PADDING         :
  *    | +---------+ | <-+..........            :---< SB_BUTTON_TOTAL_HEIGHT
  *    | | ......%%| |   |         :            :
  *    | | ../\..%%| |   |---< SB_BUTTON_HEIGHT :
  *    | | %%%%%%%%| |   |         :            :
  *    | +---------+ | <-+         :            :
  *    |             |             :            :
  *    | +---------+ | <-+         :---< SB_BUTTON_BOTH_HEIGHT
  *    | | ......%%| |   |         :            :
  *    | | ..\/..%%| |   |         :            :
  *    | | %%%%%%%%| |   |---< SB_BUTTON_SINGLE_HEIGHT
  *    | +---------+ |   |         :            :
  *    |             |   |         :            :
  *    +-------------+ <-+.........:............:
  *    ^^|_________| :
  *    ||     |      :
  *    ||     +---< SB_BUTTON_WIDTH
  *    ||            :
  *    |+------< SB_PADDING
  *    |:            :
  *    +----< SB_BORDER_WIDTH
  *     :            :
  *     :............:
  *           |
  *           +---< SB_WIDTH_NEXT
  */

#endif
