#ifndef MENUBAR_H_
#define MENUBAR_H_

typedef struct {
    short           type;	/* must not be changed; first element */
    short           len;	/* strlen (str) */
    unsigned char  *str;	/* action to take */
} action_t;

typedef struct {
    short           type;	/* must not be changed; first element */
    struct menu_t  *menu;	/* sub-menu */
} submenu_t;

typedef struct menuitem_t {
    struct menuitem_t *prev;	/* prev menu-item */
    struct menuitem_t *next;	/* next menu-item */
    char           *name;	/* character string displayed */
    char           *name2;	/* character string displayed (right) */
    short           len;	/* strlen (name) */
    short           len2;	/* strlen (name) */
    union {
        short           type;	/* must not be changed; first element */
        action_t        action;
        submenu_t       submenu;
    } entry;
} menuitem_t;

enum menuitem_t_action {
    MenuLabel,
    MenuAction,
    MenuTerminalAction,
    MenuSubMenu
};

typedef struct menu_t {
    struct menu_t  *parent;	/* parent menu */
    struct menu_t  *prev;	/* prev menu */
    struct menu_t  *next;	/* next menu */
    menuitem_t     *head;	/* double-linked list */
    menuitem_t     *tail;	/* double-linked list */
    menuitem_t     *item;	/* current item */
    char           *name;	/* menu name */
    short           len;	/* strlen (name) */
    short           width;	/* maximum menu width [chars] */
    Window          win;	/* window of the menu */
    struct rxvt_drawable *drawable;
    short           x;		/* x location [pixels] (chars if parent == NULL) */
    short           y;		/* y location [pixels] */
    short           w, h;	/* window width, height [pixels] */
} menu_t;

typedef struct bar_t {
    menu_t         *head, *tail;	/* double-linked list of menus */
    char           *title;	/* title to put in the empty menuBar */
#if (MENUBAR_MAX > 1)
# define MAXNAME 16
    char            name[MAXNAME];	/* name to use to refer to menubar */
    struct bar_t   *next, *prev;	/* circular linked-list */
#endif				/* (MENUBAR_MAX > 1) */
#define NARROWS	4
    action_t        arrows[NARROWS];
} bar_t;

/* #define DEBUG_MENU */
/* #define DEBUG_MENU_LAYOUT */
/* #define DEBUG_MENUBAR_STACKING */

#define HSPACE		1	/* one space */
#define isSeparator(name)	((name)[0] == '\0')
#define HEIGHT_SEPARATOR	(SHADOW + 1)
#define HEIGHT_TEXT		(Height2Pixel(1) + 2)

#define MENU_DELAY_USEC	0	/* no delay */

#define SEPARATOR_NAME		"-"
#define MENUITEM_BEG		'{'
#define MENUITEM_END		'}'
#define COMMENT_CHAR		'#'

#define DOT	"."
#define DOTS	".."

#endif /* _MENUBAR_H_ */
