#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include <X11/Xlib.h>

struct rxvt_term;

typedef struct {
  char            state;        /* scrollbar state                          */
  char            init;         /* scrollbar has been initialised           */
  unsigned int    beg;          /* slider sub-window begin height           */
  unsigned int    end;          /* slider sub-window end height             */
  unsigned int    top;          /* slider top position                      */
  unsigned int    bot;          /* slider bottom position                   */
  unsigned int    style;        /* style: rxvt, xterm, next                 */
  unsigned int    width;        /* scrollbar width                          */
  int             shadow;       /* scrollbar shadow width                   */
  Window          win;
  int             (rxvt_term::*update)(int, int, int, int);

  void setIdle()   { state =  1 ; }
  void setMotion() { state = 'm'; }
  void setUp()     { state = 'U'; }
  void setDn()     { state = 'D'; }
} scrollBar_t;

#define scrollbar_TotalWidth()  (scrollBar.width + scrollBar.shadow * 2)
#define scrollbar_isMotion()    (scrollBar.state == 'm')
#define scrollbar_isUp()        (scrollBar.state == 'U')
#define scrollbar_isDn()        (scrollBar.state == 'D')
#define scrollbar_isUpDn()      isupper (scrollBar.state)
#define isScrollbarWindow(w)    (scrollBar.state && (w) == scrollBar.win)

#define scrollbarnext_dnval()   (scrollBar.end + (scrollBar.width + 1))
#define scrollbarnext_upButton(y)       ((y) > scrollBar.end \
                                         && (y) <= scrollbarnext_dnval ())
#define scrollbarnext_dnButton(y)       ((y) > scrollbarnext_dnval())
#define SCROLLNEXT_MINHEIGHT    SB_THUMB_MIN_HEIGHT
#define scrollbarrxvt_upButton(y)       ((y) < scrollBar.beg)
#define scrollbarrxvt_dnButton(y)       ((y) > scrollBar.end)
#define SCROLLRXVT_MINHEIGHT    10

#define scrollbar_minheight()   (scrollBar.style == R_SB_NEXT        \
                                 ? SCROLLNEXT_MINHEIGHT                 \
                                 : SCROLLRXVT_MINHEIGHT)
#define scrollbar_above_slider(y)       ((y) < scrollBar.top)
#define scrollbar_below_slider(y)       ((y) > scrollBar.bot)
#define scrollbar_position(y)           ((y) - scrollBar.beg)
#define scrollbar_size()                (scrollBar.end - scrollBar.beg \
                                         - scrollbar_minheight ())

#define R_SB_ALIGN_CENTRE       0
#define R_SB_ALIGN_TOP          1
#define R_SB_ALIGN_BOTTOM       2

#define R_SB_NEXT               1
#define R_SB_XTERM              2
#define R_SB_PLAIN              4
#define R_SB_RXVT               8

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
