#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <X11/Xlib.h>

struct rxvt_term;

#define R_SB_NEXT               1
#define R_SB_XTERM              2
#define R_SB_PLAIN              4
#define R_SB_RXVT               8

#define R_SB_ALIGN_CENTRE       0
#define R_SB_ALIGN_TOP          1
#define R_SB_ALIGN_BOTTOM       2

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
  STATE_IDLE = 1,
  STATE_MOTION,
  STATE_UP,
  STATE_DOWN,
};

struct scrollBar_t
{
  rxvt_term *term;
  char            state;        /* scrollbar state                          */
  char            init;         /* scrollbar has been initialised           */
  unsigned int    beg;          /* slider sub-window begin height           */
  unsigned int    end;          /* slider sub-window end height             */
  unsigned int    top;          /* slider top position                      */
  unsigned int    bot;          /* slider bottom position                   */
  unsigned int    style;        /* style: rxvt, xterm, next                 */
  unsigned int    width;        /* scrollbar width                          */
  int             shadow;       /* scrollbar shadow width                   */
  int             last_bot;     /* scrollbar last bottom position           */
  int             last_top;     /* scrollbar last top position              */
  int             last_state;   /* scrollbar last state                     */
  unsigned char   align;
  Window          win;
  Cursor          leftptr_cursor;
  int             (scrollBar_t::*update)(int);
  void setup (rxvt_term *);
  void resize ();
  int map (int);
  int show (int);
  void destroy ();

  bool upButton (int y)
  {
    if (style == R_SB_NEXT)
      return y > end && y <= end + width + 1;
    if (style == R_SB_RXVT)
      return y < beg;
    return false;
  }
  bool dnButton (int y)
  {
    if (style == R_SB_NEXT)
      return y > end + width + 1;
    if (style == R_SB_RXVT)
      return y > end;
    return false;
  }
  unsigned min_height ()
  {
    return style == R_SB_NEXT ? SB_THUMB_MIN_HEIGHT : 10;
  }
  unsigned size ()
  {
    return end - beg - min_height ();
  }
  unsigned total_width ()
  {
    return width + shadow * 2;
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

#define scrollbar_above_slider(y)       ((y) < scrollBar.top)
#define scrollbar_below_slider(y)       ((y) > scrollBar.bot)
#define scrollbar_position(y)           ((y) - scrollBar.beg)

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
