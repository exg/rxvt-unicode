/*--------------------------------*-C-*---------------------------------*
 * File:	menubar.c
 *----------------------------------------------------------------------*
 * $Id: menubar.C,v 1.3 2003/11/25 11:52:42 pcg Exp $
 *
 * Copyright (c) 1997,1998  mj olesen <olesen@me.QueensU.CA>
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
 *----------------------------------------------------------------------*
 * refer.html (or refer.txt) contains up-to-date documentation.  The
 * summary that appears at the end of this file was taken from there.
 *----------------------------------------------------------------------*/

#include "../config.h"		/* NECESSARY */
#include "rxvt.h"		/* NECESSARY */
#ifdef MENUBAR
#include "version.h"
#include "menubar.h"
#include "menubar.intpro"	/* PROTOS for internal routines */

#define Menu_PixelWidth(menu)					\
    (2 * SHADOW + Width2Pixel ((menu)->width + 3 * HSPACE))

static const struct {
    const char      name;	/* (l)eft, (u)p, (d)own, (r)ight */
    const unsigned char str[5];	/* str[0] = STRLEN (str+1) */
} Arrows[NARROWS] = {
    { 'l', "\003\033[D" },
    { 'u', "\003\033[A" },
    { 'd', "\003\033[B" },
    { 'r', "\003\033[C" }
};

/*}}} */

/*
 * find an item called NAME in MENU
 */
/* INTPROTO */
menuitem_t     *
rxvt_menuitem_find(const menu_t *menu, const char *name)
{
    menuitem_t     *item;

#ifdef DEBUG_STRICT
    assert(name != NULL);
    assert(menu != NULL);
#endif

/* find the last item in the menu, this is good for separators */
    for (item = menu->tail; item != NULL; item = item->prev) {
	if (item->entry.type == MenuSubMenu) {
	    if (!STRCMP(name, (item->entry.submenu.menu)->name))
		break;
	} else if ((isSeparator(name) && isSeparator(item->name))
		   || !STRCMP(name, item->name))
	    break;
    }
    return item;
}

/*
 * unlink ITEM from its MENU and free its memory
 */
/* INTPROTO */
void
rxvt_menuitem_free(pR_ menu_t *menu, menuitem_t *item)
{
/* disconnect */
    menuitem_t     *prev, *next;

#ifdef DEBUG_STRICT
    assert(menu != NULL);
#endif

    prev = item->prev;
    next = item->next;
    if (prev != NULL)
	prev->next = next;
    if (next != NULL)
	next->prev = prev;

/* new head, tail */
    if (menu->tail == item)
	menu->tail = prev;
    if (menu->head == item)
	menu->head = next;

    switch (item->entry.type) {
    case MenuAction:
    case MenuTerminalAction:
	free(item->entry.action.str);
	break;
    case MenuSubMenu:
	rxvt_menu_delete(aR_ item->entry.submenu.menu);
	break;
    }
    if (item->name != NULL)
	free(item->name);
    if (item->name2 != NULL)
	free(item->name2);
    free(item);
}

/*
 * sort command vs. terminal actions and
 * remove the first character of STR if it's '\0'
 */
/* INTPROTO */
int
rxvt_action_type(action_t *action, unsigned char *str)
{
    unsigned int    len;

#if defined (DEBUG_MENU) || defined (DEBUG_MENUARROWS)
    len = STRLEN(str);
    fprintf(stderr, "(len %d) = %s\n", len, str);
#else
    len = rxvt_Str_escaped((char *)str);
#endif

    if (!len)
	return -1;

/* sort command vs. terminal actions */
    action->type = MenuAction;
    if (str[0] == '\0') {
	/* the functional equivalent: memmove (str, str+1, len); */
	unsigned char  *dst = (str);
	unsigned char  *src = (str + 1);
	unsigned char  *end = (str + len);

	while (src <= end)
	    *dst++ = *src++;

	len--;			/* decrement length */
	if (str[0] != '\0')
	    action->type = MenuTerminalAction;
    }
    action->str = str;
    action->len = len;

    return 0;
}

/* INTPROTO */
int
rxvt_action_dispatch(pR_ action_t *action)
{
    switch (action->type) {
    case MenuTerminalAction:
	rxvt_cmd_write(aR_ action->str, action->len);
	break;

    case MenuAction:
	rxvt_tt_write(aR_ action->str, action->len);
	break;

    default:
	return -1;
	break;
    }
    return 0;
}

/* return the arrow index corresponding to NAME */
/* INTPROTO */
int
rxvt_menuarrow_find(char name)
{
    int             i;

    for (i = 0; i < NARROWS; i++)
	if (name == Arrows[i].name)
	    return i;
    return -1;
}

/* free the memory associated with arrow NAME of the current menubar */
/* INTPROTO */
void
rxvt_menuarrow_free(pR_ char name)
{
    int             i;

    if (name) {
	i = rxvt_menuarrow_find(name);
	if (i >= 0) {
	    action_t       *act = &(R->CurrentBar->arrows[i]);

	    switch (act->type) {
	    case MenuAction:
	    case MenuTerminalAction:
		free(act->str);
		act->str = NULL;
		act->len = 0;
		break;
	    }
	    act->type = MenuLabel;
	}
    } else {
	for (i = 0; i < NARROWS; i++)
	    rxvt_menuarrow_free(aR_ Arrows[i].name);
    }
}

/* INTPROTO */
void
rxvt_menuarrow_add(pR_ char *string)
{
    int             i;
    unsigned        xtra_len;
    char           *p;
    struct {
	char           *str;
	int             len;
    }
    beg = { NULL, 0 },
    end = { NULL, 0 },
    *cur,
    parse[NARROWS];

    MEMSET(parse, 0, sizeof(parse));

/* fprintf(stderr, "add arrows = `%s'\n", string); */
    for (p = string; p != NULL && *p; string = p) {
	p = (string + 3);
	/* fprintf(stderr, "parsing at %s\n", string); */
	switch (string[1]) {
	case 'b':
	    cur = &beg;
	    break;
	case 'e':
	    cur = &end;
	    break;

	default:
	    i = rxvt_menuarrow_find(string[1]);
	    if (i >= 0)
		cur = &(parse[i]);
	    else
		continue;	/* not found */
	    break;
	}

	string = p;
	cur->str = string;
	cur->len = 0;

	if (cur == &end) {
	    p = STRCHR(string, '\0');
	} else {
	    char           *next = string;

	    while (1) {
		p = STRCHR(next, '<');
		if (p != NULL) {
		    if (p[1] && p[2] == '>')
			break;
		    /* parsed */
		} else {
		    if (beg.str == NULL)	/* no end needed */
			p = STRCHR(next, '\0');
		    break;
		}
		next = (p + 1);
	    }
	}

	if (p == NULL)
	    return;
	cur->len = (p - string);
    }

#ifdef DEBUG_MENUARROWS
    cur = &beg;
    fprintf(stderr, "<b>(len %d) = %.*s\n",
	    cur->len, cur->len, (cur->str ? cur->str : ""));
    for (i = 0; i < NARROWS; i++) {
	cur = &(parse[i]);
	fprintf(stderr, "<%c>(len %d) = %.*s\n",
		Arrows[i].name,
		cur->len, cur->len, (cur->str ? cur->str : ""));
    }
    cur = &end;
    fprintf(stderr, "<e>(len %d) = %.*s\n",
	    cur->len, cur->len, (cur->str ? cur->str : ""));
#endif

    xtra_len = (beg.len + end.len);
    for (i = 0; i < NARROWS; i++) {
	if (xtra_len || parse[i].len)
	    rxvt_menuarrow_free(aR_ Arrows[i].name);
    }

    for (i = 0; i < NARROWS; i++) {
	unsigned char  *str;
	unsigned int    len;

	if (!parse[i].len)
	    continue;

	str = rxvt_malloc(parse[i].len + xtra_len + 1);

	len = 0;
	if (beg.len) {
	    STRNCPY(str + len, beg.str, beg.len);
	    len += beg.len;
	}
	STRNCPY(str + len, parse[i].str, parse[i].len);
	len += parse[i].len;

	if (end.len) {
	    STRNCPY(str + len, end.str, end.len);
	    len += end.len;
	}
	str[len] = '\0';

#ifdef DEBUG_MENUARROWS
	fprintf(stderr, "<%c>(len %d) = %s\n", Arrows[i].name, len, str);
#endif
	if (rxvt_action_type(&(R->CurrentBar->arrows[i]), str) < 0)
	    free(str);
    }
}

/* INTPROTO */
menuitem_t     *
rxvt_menuitem_add(menu_t *menu, const char *name, const char *name2, const char *action)
{
    menuitem_t     *item;
    unsigned int    len;

#ifdef DEBUG_STRICT
    assert(name != NULL);
    assert(action != NULL);
#endif

    if (menu == NULL)
	return NULL;

    if (isSeparator(name)) {
	/* add separator, no action */
	name = "";
	action = "";
    } else {
	/*
	 * add/replace existing menu item
	 */
	item = rxvt_menuitem_find(menu, name);
	if (item != NULL) {
	    if (item->name2 != NULL && name2 != NULL) {
		free(item->name2);
		item->len2 = 0;
		item->name2 = NULL;
	    }
	    switch (item->entry.type) {
	    case MenuAction:
	    case MenuTerminalAction:
		free(item->entry.action.str);
		item->entry.action.str = NULL;
		break;
	    }
	    goto Item_Found;
	}
    }
/* allocate a new itemect */
    item = (menuitem_t *) rxvt_malloc(sizeof(menuitem_t));

    item->len2 = 0;
    item->name2 = NULL;

    len = STRLEN(name);
    item->name = rxvt_malloc(len + 1);
    STRCPY(item->name, name);
    if (name[0] == '.' && name[1] != '.')
	len = 0;		/* hidden menu name */
    item->len = len;

/* add to tail of list */
    item->prev = menu->tail;
    item->next = NULL;

    if (menu->tail != NULL)
	(menu->tail)->next = item;
    menu->tail = item;
/* fix head */
    if (menu->head == NULL)
	menu->head = item;

/*
 * add action
 */
  Item_Found:
    if (name2 != NULL && item->name2 == NULL) {
	len = STRLEN(name2);
	if (len == 0)
	    item->name2 = NULL;
	else {
	    item->name2 = rxvt_malloc(len + 1);
	    STRCPY(item->name2, name2);
	}
	item->len2 = len;
    }
    item->entry.type = MenuLabel;
    len = STRLEN(action);

    if (len == 0 && item->name2 != NULL) {
	action = item->name2;
	len = item->len2;
    }
    if (len) {
	unsigned char  *str = rxvt_malloc(len + 1);

	STRCPY(str, action);

	if (rxvt_action_type(&(item->entry.action), str) < 0)
	    free(str);
    }
/* new item and a possible increase in width */
    if (menu->width < (item->len + item->len2))
	menu->width = (item->len + item->len2);

    return item;
}

/*
 * search for the base starting menu for NAME.
 * return a pointer to the portion of NAME that remains
 */
/* INTPROTO */
char           *
rxvt_menu_find_base(pR_ menu_t **menu, char *path)
{
    menu_t         *m = NULL;
    menuitem_t     *item;

#ifdef DEBUG_STRICT
    assert(menu != NULL);
    assert(R->CurrentBar != NULL);
#endif

    if (path[0] == '\0')
	return path;

    if (STRCHR(path, '/') != NULL) {
	char           *p = path;

	while ((p = STRCHR(p, '/')) != NULL) {
	    p++;
	    if (*p == '/')
		path = p;
	}
	if (path[0] == '/') {
	    path++;
	    *menu = NULL;
	}
	while ((p = STRCHR(path, '/')) != NULL) {
	    p[0] = '\0';
	    if (path[0] == '\0')
		return NULL;
	    if (!STRCMP(path, DOT)) {
		/* nothing to do */
	    } else if (!STRCMP(path, DOTS)) {
		if (*menu != NULL)
		    *menu = (*menu)->parent;
	    } else {
		path = rxvt_menu_find_base(aR_ menu, path);
		if (path[0] != '\0') {	/* not found */
		    p[0] = '/';	/* fix-up name again */
		    return path;
		}
	    }

	    path = (p + 1);
	}
    }
    if (!STRCMP(path, DOTS)) {
	path += STRLEN(DOTS);
	if (*menu != NULL)
	    *menu = (*menu)->parent;
	return path;
    }
/* find this menu */
    if (*menu == NULL) {
	for (m = R->CurrentBar->tail; m != NULL; m = m->prev) {
	    if (!STRCMP(path, m->name))
		break;
	}
    } else {
	/* find this menu */
	for (item = (*menu)->tail; item != NULL; item = item->prev) {
	    if (item->entry.type == MenuSubMenu
		&& !STRCMP(path, (item->entry.submenu.menu)->name)) {
		m = (item->entry.submenu.menu);
		break;
	    }
	}
    }
    if (m != NULL) {
	*menu = m;
	path += STRLEN(path);
    }
    return path;
}

/*
 * delete this entire menu
 */
/* INTPROTO */
menu_t         *
rxvt_menu_delete(pR_ menu_t *menu)
{
    menu_t         *parent = NULL, *prev, *next;
    menuitem_t     *item;
    bar_t          *CurrentBar = R->CurrentBar;

#ifdef DEBUG_STRICT
    assert(CurrentBar != NULL);
#endif

/* delete the entire menu */
    if (menu == NULL)
	return NULL;

    parent = menu->parent;

/* unlink MENU */
    prev = menu->prev;
    next = menu->next;
    if (prev != NULL)
	prev->next = next;
    if (next != NULL)
	next->prev = prev;

/* fix the index */
    if (parent == NULL) {
	const int       len = (menu->len + HSPACE);

	if (CurrentBar->tail == menu)
	    CurrentBar->tail = prev;
	if (CurrentBar->head == menu)
	    CurrentBar->head = next;

	for (next = menu->next; next != NULL; next = next->next)
	    next->x -= len;
    } else {
	for (item = parent->tail; item != NULL; item = item->prev) {
	    if (item->entry.type == MenuSubMenu
		&& item->entry.submenu.menu == menu) {
		item->entry.submenu.menu = NULL;
		rxvt_menuitem_free(aR_ menu->parent, item);
		break;
	    }
	}
    }

    item = menu->tail;
    while (item != NULL) {
	menuitem_t     *p = item->prev;

	rxvt_menuitem_free(aR_ menu, item);
	item = p;
    }

    if (menu->name != NULL)
	free(menu->name);
    free(menu);

    return parent;
}

/* INTPROTO */
menu_t         *
rxvt_menu_add(pR_ menu_t *parent, char *path)
{
    menu_t         *menu;
    bar_t          *CurrentBar = R->CurrentBar;

#ifdef DEBUG_STRICT
    assert(CurrentBar != NULL);
#endif

    if (STRCHR(path, '/') != NULL) {
	char           *p;

	if (path[0] == '/') {
	    /* shouldn't happen */
	    path++;
	    parent = NULL;
	}
	while ((p = STRCHR(path, '/')) != NULL) {
	    p[0] = '\0';
	    if (path[0] == '\0')
		return NULL;

	    parent = rxvt_menu_add(aR_ parent, path);
	    path = (p + 1);
	}
    }
    if (!STRCMP(path, DOTS))
	return (parent != NULL ? parent->parent : parent);

    if (!STRCMP(path, DOT) || path[0] == '\0')
	return parent;

/* allocate a new menu */
    menu = (menu_t *) rxvt_malloc(sizeof(menu_t));

    menu->width = 0;
    menu->parent = parent;
    menu->len = STRLEN(path);
    menu->name = rxvt_malloc((menu->len + 1));
    STRCPY(menu->name, path);

/* initialize head/tail */
    menu->head = menu->tail = NULL;
    menu->prev = menu->next = NULL;

    menu->win = None;
    menu->x = menu->y = menu->w = menu->h = 0;
    menu->item = NULL;

/* add to tail of list */
    if (parent == NULL) {
	menu->prev = CurrentBar->tail;
	if (CurrentBar->tail != NULL)
	    CurrentBar->tail->next = menu;
	CurrentBar->tail = menu;
	if (CurrentBar->head == NULL)
	    CurrentBar->head = menu;	/* fix head */
	if (menu->prev)
	    menu->x = (menu->prev->x + menu->prev->len + HSPACE);
    } else {
	menuitem_t     *item;

	item = rxvt_menuitem_add(parent, path, "", "");
	if (item == NULL) {
	    free(menu);
	    return parent;
	}
#ifdef DEBUG_STRICT
	assert(item->entry.type == MenuLabel);
#endif
	item->entry.type = MenuSubMenu;
	item->entry.submenu.menu = menu;
    }

    return menu;
}

/* INTPROTO */
void
rxvt_drawbox_menubar(pR_ int x, int len, int state)
{
    GC              top, bot;

    x = Width2Pixel(x);
    len = Width2Pixel(len + HSPACE);
    if (x >= R->TermWin.width)
	return;
    else if (x + len >= R->TermWin.width)
	len = (TermWin_TotalWidth() - x);

#ifdef MENUBAR_SHADOW_IN
    state = -state;
#endif
    switch (state) {
    case +1:
	top = R->topShadowGC;
	bot = R->botShadowGC;
	break;			/* SHADOW_OUT */
    case -1:
	top = R->botShadowGC;
	bot = R->topShadowGC;
	break;			/* SHADOW_IN */
    default:
	top = bot = R->scrollbarGC;
	break;			/* neutral */
    }

    rxvt_Draw_Shadow(R->Xdisplay, R->menuBar.win, top, bot,
		     x, 0, len, menuBar_TotalHeight());
}

/* INTPROTO */
void
rxvt_drawtriangle(pR_ int x, int y, int state)
{
    GC              top, bot;
    int             w;

#ifdef MENU_SHADOW_IN
    state = -state;
#endif
    switch (state) {
    case +1:
	top = R->topShadowGC;
	bot = R->botShadowGC;
	break;			/* SHADOW_OUT */
    case -1:
	top = R->botShadowGC;
	bot = R->topShadowGC;
	break;			/* SHADOW_IN */
    default:
	top = bot = R->scrollbarGC;
	break;			/* neutral */
    }

    w = Height2Pixel(1) - 2 * SHADOW;

    x -= SHADOW + (3 * w / 2);
    y += SHADOW * 3;

    rxvt_Draw_Triangle(R->Xdisplay, R->ActiveMenu->win, top, bot, x, y, w,
		       'r');
}

/* INTPROTO */
void
rxvt_drawbox_menuitem(pR_ int y, int state)
{
    GC              top, bot;

#ifdef MENU_SHADOW_IN
    state = -state;
#endif
    switch (state) {
    case +1:
	top = R->topShadowGC;
	bot = R->botShadowGC;
	break;			/* SHADOW_OUT */
    case -1:
	top = R->botShadowGC;
	bot = R->topShadowGC;
	break;			/* SHADOW_IN */
    default:
	top = bot = R->scrollbarGC;
	break;			/* neutral */
    }

    rxvt_Draw_Shadow(R->Xdisplay, R->ActiveMenu->win, top, bot,
		     SHADOW + 0, SHADOW + y,
		     R->ActiveMenu->w - 2 * (SHADOW),
		     HEIGHT_TEXT + 2 * SHADOW);
    XFlush(R->Xdisplay);
}

#ifdef DEBUG_MENU_LAYOUT
/* INTPROTO */
void
rxvt_print_menu_ancestors(menu_t *menu)
{
    if (menu == NULL) {
	fprintf(stderr, "Top Level menu\n");
	return;
    }
    fprintf(stderr, "menu %s ", menu->name);
    if (menu->parent != NULL) {
	menuitem_t     *item;

	for (item = menu->parent->head; item != NULL; item = item->next) {
	    if (item->entry.type == MenuSubMenu
		&& item->entry.submenu.menu == menu) {
		break;
	    }
	}
	if (item == NULL) {
	    fprintf(stderr, "is an orphan!\n");
	    return;
	}
    }
    fprintf(stderr, "\n");
    rxvt_print_menu_ancestors(menu->parent);
}

/* INTPROTO */
void
rxvt_print_menu_descendants(menu_t *menu)
{
    menuitem_t     *item;
    menu_t         *parent;
    int             i, level = 0;

    parent = menu;
    do {
	level++;
	parent = parent->parent;
    }
    while (parent != NULL);

    for (i = 0; i < level; i++)
	fprintf(stderr, ">");
    fprintf(stderr, "%s\n", menu->name);

    for (item = menu->head; item != NULL; item = item->next) {
	if (item->entry.type == MenuSubMenu) {
	    if (item->entry.submenu.menu == NULL)
		fprintf(stderr, "> %s == NULL\n", item->name);
	    else
		rxvt_print_menu_descendants(item->entry.submenu.menu);
	} else {
	    for (i = 0; i < level; i++)
		fprintf(stderr, "+");
	    if (item->entry.type == MenuLabel)
		fprintf(stderr, "label: ");
	    fprintf(stderr, "%s\n", item->name);
	}
    }

    for (i = 0; i < level; i++)
	fprintf(stderr, "<");
    fprintf(stderr, "\n");
}
#endif

/* pop up/down the current menu and redraw the menuBar button */
/* INTPROTO */
void
rxvt_menu_show(pR)
{
    int             x, y, xright;
    menu_t         *ActiveMenu = R->ActiveMenu;
    menuitem_t     *item;

    if (ActiveMenu == NULL)
	return;

    x = ActiveMenu->x;
    if (ActiveMenu->parent == NULL) {
	register int    h;

	rxvt_drawbox_menubar(aR_ x, ActiveMenu->len, -1);
	x = Width2Pixel(x);

	ActiveMenu->y = 1;
	ActiveMenu->w = Menu_PixelWidth(ActiveMenu);

	if ((x + ActiveMenu->w) >= R->TermWin.width)
	    x = (TermWin_TotalWidth() - ActiveMenu->w);

	/* find the height */
	for (h = 0, item = ActiveMenu->head; item != NULL; item = item->next)
	    h += isSeparator(item->name) ? HEIGHT_SEPARATOR
					 : HEIGHT_TEXT + 2 * SHADOW;
	ActiveMenu->h = h + 2 * SHADOW;
    }
    if (ActiveMenu->win == None) {
	ActiveMenu->win = XCreateSimpleWindow(R->Xdisplay, R->TermWin.vt,
					      x, ActiveMenu->y,
					      ActiveMenu->w, ActiveMenu->h,
					      0,
					      R->PixColors[Color_fg],
					      R->PixColors[Color_scroll]);
	XMapWindow(R->Xdisplay, ActiveMenu->win);
    }
    rxvt_Draw_Shadow(R->Xdisplay, ActiveMenu->win,
		     R->topShadowGC, R->botShadowGC,
		     0, 0, ActiveMenu->w, ActiveMenu->h);

/* determine the correct right-alignment */
    for (xright = 0, item = ActiveMenu->head; item != NULL; item = item->next)
	if (item->len2 > xright)
	    xright = item->len2;

    for (y = 0, item = ActiveMenu->head; item != NULL; item = item->next) {
	const int       xoff = (SHADOW + Width2Pixel(HSPACE) / 2);
	register int    h;
	GC              gc = R->menubarGC;

	if (isSeparator(item->name)) {
	    rxvt_Draw_Shadow(R->Xdisplay, ActiveMenu->win,
			     R->topShadowGC, R->botShadowGC,
			     SHADOW, y + SHADOW + 1,
			     ActiveMenu->w - 2 * SHADOW, 0);
	    h = HEIGHT_SEPARATOR;
	} else {
	    char           *name = item->name;
	    int             len = item->len;

	    if (item->entry.type == MenuLabel) {
		gc = R->botShadowGC;
	    } else if (item->entry.type == MenuSubMenu) {
		int             x1, y1;
		menuitem_t     *it;
		menu_t         *menu = item->entry.submenu.menu;

		rxvt_drawtriangle(aR_ ActiveMenu->w, y, +1);

		name = menu->name;
		len = menu->len;

		y1 = ActiveMenu->y + y;

		menu->w = Menu_PixelWidth(menu);

		/* place sub-menu at midpoint of parent menu */
		x1 = ActiveMenu->w / 2;
		if (x1 > menu->w)	/* right-flush menu if too small */
		    x1 += (x1 - menu->w);
		x1 += x;

		/* find the height of this submenu */
		for (h = 0, it = menu->head; it != NULL; it = it->next)
		    h += isSeparator(it->name) ? HEIGHT_SEPARATOR
					       : HEIGHT_TEXT + 2 * SHADOW;
		menu->h = h + 2 * SHADOW;

		/* ensure menu is in window limits */
		if ((x1 + menu->w) >= R->TermWin.width)
		    x1 = (TermWin_TotalWidth() - menu->w);

		if ((y1 + menu->h) >= R->TermWin.height)
		    y1 = (TermWin_TotalHeight() - menu->h);

		menu->x = (x1 < 0 ? 0 : x1);
		menu->y = (y1 < 0 ? 0 : y1);
	    } else if (item->name2 && !STRCMP(name, item->name2))
		name = NULL;

	    if (len && name) {
#ifdef USE_XIM
		if (R->TermWin.fontset)
		    XmbDrawString(R->Xdisplay,
				  ActiveMenu->win, R->TermWin.fontset,
				  gc, xoff,
				  2 * SHADOW + y + R->TermWin.font->ascent + 1,
				  name, len);
		else
#endif
		    XDrawString(R->Xdisplay, ActiveMenu->win, gc, xoff,
				2 * SHADOW + y + R->TermWin.font->ascent + 1,
				name, len);
	    }

	    len = item->len2;
	    name = item->name2;
	    if (len && name) {
#ifdef USE_XIM
		if (R->TermWin.fontset)
		    XmbDrawString(R->Xdisplay,
				  ActiveMenu->win, R->TermWin.fontset,
				  gc,
				  ActiveMenu->w - (xoff + Width2Pixel(xright)),
				  2 * SHADOW + y + R->TermWin.font->ascent + 1,
				  name, len);
		else
#endif
		    XDrawString(R->Xdisplay, ActiveMenu->win, gc,
				ActiveMenu->w - (xoff + Width2Pixel(xright)),
				2 * SHADOW + y + R->TermWin.font->ascent + 1,
				name, len);
	    }
	    h = HEIGHT_TEXT + 2 * SHADOW;
	}
	y += h;
    }
}

/* INTPROTO */
void
rxvt_menu_display(pR_ void (*update)(rxvt_t *))
{
    menu_t         *ActiveMenu = R->ActiveMenu;

    if (ActiveMenu == NULL)
	return;
    if (ActiveMenu->win != None)
	XDestroyWindow(R->Xdisplay, ActiveMenu->win);
    ActiveMenu->win = None;
    ActiveMenu->item = NULL;

    if (ActiveMenu->parent == NULL)
	rxvt_drawbox_menubar(aR_ ActiveMenu->x, ActiveMenu->len, +1);
    R->ActiveMenu = ActiveMenu->parent;
    update(r);
}

/* INTPROTO */
void
rxvt_menu_hide_all(pR)
{
    rxvt_menu_display(aR_ rxvt_menu_hide_all);
}

/* INTPROTO */
void
rxvt_menu_hide(pR)
{
    rxvt_menu_display(aR_ rxvt_menu_show);
}

/* INTPROTO */
void
rxvt_menu_clear(pR_ menu_t *menu)
{
    if (menu != NULL) {
	menuitem_t     *item = menu->tail;

	while (item != NULL) {
	    rxvt_menuitem_free(aR_ menu, item);
	    /* it didn't get freed ... why? */
	    if (item == menu->tail)
		return;
	    item = menu->tail;
	}
	menu->width = 0;
    }
}

/* INTPROTO */
void
rxvt_menubar_clear(pR)
{
    bar_t          *CurrentBar = R->CurrentBar;

    if (CurrentBar != NULL) {
	menu_t         *menu = CurrentBar->tail;

	while (menu != NULL) {
	    menu_t         *prev = menu->prev;

	    rxvt_menu_delete(aR_ menu);
	    menu = prev;
	}
	CurrentBar->head = CurrentBar->tail = NULL;

	if (CurrentBar->title) {
	    free(CurrentBar->title);
	    CurrentBar->title = NULL;
	}
	rxvt_menuarrow_free(aR_ 0);	/* remove all arrow functions */
    }
    R->ActiveMenu = NULL;
}

#if (MENUBAR_MAX > 1)
/* find if menu already exists */
/* INTPROTO */
bar_t          *
rxvt_menubar_find(pR_ const char *name)
{
    bar_t          *bar = R->CurrentBar;

#ifdef DEBUG_MENUBAR_STACKING
    fprintf(stderr, "looking for [menu:%s] ...", name ? name : "(nil)");
#endif
    if (bar == NULL || name == NULL)
	return NULL;

    if (STRLEN(name) && STRCMP(name, "*")) {
	do {
	    if (!STRCMP(bar->name, name)) {
#ifdef DEBUG_MENUBAR_STACKING
		fprintf(stderr, " found!\n");
#endif
		return bar;
	    }
	    bar = bar->next;
	}
	while (bar != R->CurrentBar);
	bar = NULL;
    }
#ifdef DEBUG_MENUBAR_STACKING
    fprintf(stderr, "%s found!\n", (bar ? "" : " NOT"));
#endif

    return bar;
}

/* INTPROTO */
int
rxvt_menubar_push(pR_ const char *name)
{
    int             ret = 1;
    bar_t          *bar;

    if (R->CurrentBar == NULL) {
	/* allocate first one */
	bar = (bar_t *) rxvt_malloc(sizeof(bar_t));

	MEMSET(bar, 0, sizeof(bar_t));
	/* circular linked-list */
	bar->next = bar->prev = bar;
	bar->head = bar->tail = NULL;
	bar->title = NULL;
	R->CurrentBar = bar;
	R->Nbars++;

	rxvt_menubar_clear(aR);
    } else {
	/* find if menu already exists */
	bar = rxvt_menubar_find(aR_ name);
	if (bar != NULL) {
	    /* found it, use it */
	    R->CurrentBar = bar;
	} else {
	    /* create if needed, or reuse the existing empty menubar */
	    if (R->CurrentBar->head != NULL) {
		/* need to malloc another one */
		if (R->Nbars < MENUBAR_MAX)
		    bar = (bar_t *) rxvt_malloc(sizeof(bar_t));
		else
		    bar = NULL;

		/* malloc failed or too many menubars, reuse another */
		if (bar == NULL) {
		    bar = R->CurrentBar->next;
		    ret = -1;
		} else {
		    bar->head = bar->tail = NULL;
		    bar->title = NULL;

		    bar->next = R->CurrentBar->next;
		    R->CurrentBar->next = bar;
		    bar->prev = R->CurrentBar;
		    bar->next->prev = bar;

		    R->Nbars++;
		}
		R->CurrentBar = bar;

	    }
	    rxvt_menubar_clear(aR);
	}
    }

/* give menubar this name */
    STRNCPY(R->CurrentBar->name, name, MAXNAME);
    R->CurrentBar->name[MAXNAME - 1] = '\0';

    return ret;
}

/* switch to a menu called NAME and remove it */
/* INTPROTO */
void
rxvt_menubar_remove(pR_ const char *name)
{
    bar_t          *bar;

    if ((bar = rxvt_menubar_find(aR_ name)) == NULL)
	return;
    R->CurrentBar = bar;

    do {
	rxvt_menubar_clear(aR);
	/*
	 * pop a menubar, clean it up first
	 */
	if (R->CurrentBar != NULL) {
	    bar_t          *prev = R->CurrentBar->prev;
	    bar_t          *next = R->CurrentBar->next;

	    if (prev == next && prev == R->CurrentBar) {	/* only 1 left */
		prev = NULL;
		R->Nbars = 0;	/* safety */
	    } else {
		next->prev = prev;
		prev->next = next;
		R->Nbars--;
	    }

	    free(R->CurrentBar);
	    R->CurrentBar = prev;
	}
    }
    while (R->CurrentBar && !STRCMP(name, "*"));
}

/* INTPROTO */
void
rxvt_action_decode(FILE *fp, action_t *act)
{
    unsigned char  *str;
    short           len;

    if (act == NULL || (len = act->len) == 0 || (str = act->str) == NULL)
	return;

    if (act->type == MenuTerminalAction) {
	fprintf(fp, "^@");
	/* can strip trailing ^G from XTerm sequence */
	if (str[0] == C0_ESC && str[1] == ']' && str[len - 1] == C0_BEL)
	    len--;
    } else if (str[0] == C0_ESC) {
	switch (str[1]) {
	case '[':
	case ']':
	    break;

	case 'x':
	    /* can strip trailing '\r' from M-x sequence */
	    if (str[len - 1] == '\r')
		len--;
	    /* FALLTHROUGH */

	default:
	    fprintf(fp, "M-");	/* meta prefix */
	    str++;
	    len--;
	    break;
	}
    }
/*
 * control character form is preferred, since backslash-escaping
 * can be really ugly looking when the backslashes themselves also
 * have to be escaped to avoid Shell (or whatever scripting
 * language) interpretation
 */
    while (len > 0) {
	unsigned char   ch = *str++;

	switch (ch) {
	case C0_ESC:
	    fprintf(fp, "\\E");
	    break;		/* escape */
	case '\r':
	    fprintf(fp, "\\r");
	    break;		/* carriage-return */
	case '\\':
	    fprintf(fp, "\\\\");
	    break;		/* backslash */
	case '^':
	    fprintf(fp, "\\^");
	    break;		/* caret */
	case 127:
	    fprintf(fp, "^?");
	default:
	    if (ch <= 31)
		fprintf(fp, "^%c", ('@' + ch));
	    else if (ch > 127)
		fprintf(fp, "\\%o", ch);
	    else
		fprintf(fp, "%c", ch);
	    break;
	}
	len--;
    }
    fprintf(fp, "\n");
}

/* INTPROTO */
void
rxvt_menu_dump(FILE *fp, menu_t *menu)
{
    menuitem_t     *item;

/* create a new menu and clear it */
    fprintf(fp, (menu->parent ? "./%s/*\n" : "/%s/*\n"), menu->name);

    for (item = menu->head; item != NULL; item = item->next) {
	switch (item->entry.type) {
	case MenuSubMenu:
	    if (item->entry.submenu.menu == NULL)
		fprintf(fp, "> %s == NULL\n", item->name);
	    else
		rxvt_menu_dump(fp, item->entry.submenu.menu);
	    break;

	case MenuLabel:
	    fprintf(fp, "{%s}\n", (STRLEN(item->name) ? item->name : "-"));
	    break;

	case MenuTerminalAction:
	case MenuAction:
	    fprintf(fp, "{%s}", item->name);
	    if (item->name2 != NULL && STRLEN(item->name2))
		fprintf(fp, "{%s}", item->name2);
	    fprintf(fp, "\t");
	    rxvt_action_decode(fp, &(item->entry.action));
	    break;
	}
    }

    fprintf(fp, (menu->parent ? "../\n" : "/\n\n"));
}

/* INTPROTO */
void
rxvt_menubar_dump(pR_ FILE *fp)
{
    bar_t          *bar = R->CurrentBar;
    time_t          t;

    if (bar == NULL || fp == NULL)
	return;
    time(&t);

    fprintf(fp,
	    "# " APL_SUBCLASS " (%s)  Pid: %u\n# Date: %s\n\n",
	    R->rs[Rs_name], (unsigned int)getpid(), ctime(&t));

/* dump in reverse order */
    bar = R->CurrentBar->prev;
    do {
	menu_t         *menu;
	int             i;

	fprintf(fp, "[menu:%s]\n", bar->name);

	if (bar->title != NULL)
	    fprintf(fp, "[title:%s]\n", bar->title);

	for (i = 0; i < NARROWS; i++) {
	    switch (bar->arrows[i].type) {
	    case MenuTerminalAction:
	    case MenuAction:
		fprintf(fp, "<%c>", Arrows[i].name);
		rxvt_action_decode(fp, &(bar->arrows[i]));
		break;
	    }
	}
	fprintf(fp, "\n");

	for (menu = bar->head; menu != NULL; menu = menu->next)
	    rxvt_menu_dump(fp, menu);

	fprintf(fp, "\n[done:%s]\n\n", bar->name);
	bar = bar->prev;
    }
    while (bar != R->CurrentBar->prev);
}
#endif				/* (MENUBAR_MAX > 1) */

/*
 * read in menubar commands from FILENAME
 * ignore all input before the tag line [menu] or [menu:???]
 *
 * Note that since File_find () is used, FILENAME can be semi-colon
 * delimited such that the second part can refer to a tag
 * so that a large `database' of menus can be collected together
 *
 * FILENAME = "file"
 * FILENAME = "file;"
 *      read `file' starting with first [menu] or [menu:???] line
 *
 * FILENAME = "file;tag"
 *      read `file' starting with [menu:tag]
 */
/* EXTPROTO */
void
rxvt_menubar_read(pR_ const char *filename)
{
/* read in a menu from a file */
    FILE           *fp;
    char            buffer[256];
    char           *p, *file, *tag = NULL;

    file = (char *)rxvt_File_find(filename, ".menu", R->rs[Rs_path]);
    if (file == NULL)
	return;
    fp = fopen(file, "rb");
    free(file);
    if (fp == NULL)
	return;

#if (MENUBAR_MAX > 1)
/* semi-colon delimited */
    if ((tag = STRCHR(filename, ';')) != NULL) {
	tag++;
	if (*tag == '\0')
	    tag = NULL;
    }
#endif				/* (MENUBAR_MAX > 1) */
#ifdef DEBUG_MENU
    fprintf(stderr, "[read:%s]\n", p);
    if (tag)
	fprintf(stderr, "looking for [menu:%s]\n", tag);
#endif

    while ((p = fgets(buffer, sizeof(buffer), fp)) != NULL) {
	int             n;

	if ((n = rxvt_Str_match(p, "[menu")) != 0) {
	    if (tag) {
		/* looking for [menu:tag] */
		if (p[n] == ':' && p[n + 1] != ']') {
		    n++;
		    n += rxvt_Str_match(p + n, tag);
		    if (p[n] == ']') {
#ifdef DEBUG_MENU
			fprintf(stderr, "[menu:%s]\n", tag);
#endif
			break;
		    }
		}
	    } else if (p[n] == ':' || p[n] == ']')
		break;
	}
    }

/* found [menu], [menu:???] tag */
    while (p != NULL) {
	int             n;

#ifdef DEBUG_MENU
	fprintf(stderr, "read line = %s\n", p);
#endif

	/* looking for [done:tag] or [done:] */
	if ((n = rxvt_Str_match(p, "[done")) != 0) {
	    if (p[n] == ']') {
		R->menu_readonly = 1;
		break;
	    } else if (p[n] == ':') {
		n++;
		if (p[n] == ']') {
		    R->menu_readonly = 1;
		    break;
		} else if (tag) {
		    n += rxvt_Str_match(p + n, tag);
		    if (p[n] == ']') {
#ifdef DEBUG_MENU
			fprintf(stderr, "[done:%s]\n", tag);
#endif
			R->menu_readonly = 1;
			break;
		    }
		} else {
		    /* what? ... skip this line */
		    p[0] = COMMENT_CHAR;
		}
	    }
	}
	/*
	 * remove leading/trailing space
	 * and strip-off leading/trailing quotes
	 * skip blank or comment lines
	 */
	rxvt_Str_trim(p);
	if (*p && *p != '#') {
	    R->menu_readonly = 0;	/* if case we read another file */
	    rxvt_menubar_dispatch(aR_ p);
	}
	/* get another line */
	p = fgets(buffer, sizeof(buffer), fp);
    }

    fclose(fp);
}

/*
 * user interface for building/deleting and otherwise managing menus
 */
/* EXTPROTO */
void
rxvt_menubar_dispatch(pR_ char *str)
{
    int             n, cmd;
    char           *path, *name, *name2;

    if (menubar_visible(r) && R->ActiveMenu != NULL)
	rxvt_menubar_expose(aR);
    else
	R->ActiveMenu = NULL;

    cmd = *str;
    switch (cmd) {
    case '.':
    case '/':			/* absolute & relative path */
    case MENUITEM_BEG:		/* menuitem */
	/* add `+' prefix for these cases */
	cmd = '+';
	break;

    case '+':
    case '-':
	str++;			/* skip cmd character */
	break;

    case '<':
#if (MENUBAR_MAX > 1)
	if (R->CurrentBar == NULL)
	    break;
#endif				/* (MENUBAR_MAX > 1) */
	if (str[1] && str[2] == '>')	/* arrow commands */
	    rxvt_menuarrow_add(aR_ str);
	break;

    case '[':			/* extended command */
	while (str[0] == '[') {
	    char           *next = (++str);	/* skip leading '[' */

	    if (str[0] == ':') {	/* [:command:] */
		do {
		    next++;
		    if ((next = STRCHR(next, ':')) == NULL)
			return;	/* parse error */
		}
		while (next[1] != ']');
		/* remove and skip ':]' */
		*next = '\0';
		next += 2;
	    } else {
		if ((next = STRCHR(next, ']')) == NULL)
		    return;	/* parse error */
		/* remove and skip ']' */
		*next = '\0';
		next++;
	    }

	    if (str[0] == ':') {
		int             saved;

		/* try and dispatch it, regardless of read/write status */
		saved = R->menu_readonly;
		R->menu_readonly = 0;
		rxvt_menubar_dispatch(aR_ str + 1);
		R->menu_readonly = saved;
	    }
	    /* these ones don't require menu stacking */
	    else if (!STRCMP(str, "clear")) {
		rxvt_menubar_clear(aR);
	    } else if (!STRCMP(str, "done") || rxvt_Str_match(str, "done:")) {
		R->menu_readonly = 1;
	    } else if (!STRCMP(str, "show")) {
		rxvt_map_menuBar(aR_ 1);
		R->menu_readonly = 1;
	    } else if (!STRCMP(str, "hide")) {
		rxvt_map_menuBar(aR_ 0);
		R->menu_readonly = 1;
	    } else if ((n = rxvt_Str_match(str, "read:")) != 0) {
		/* read in a menu from a file */
		str += n;
		rxvt_menubar_read(aR_ str);
	    } else if ((n = rxvt_Str_match(str, "title:")) != 0) {
		str += n;
		if (R->CurrentBar != NULL && !R->menu_readonly) {
		    if (*str) {
			name = rxvt_realloc(R->CurrentBar->title,
					    STRLEN(str) + 1);
			if (name != NULL) {
			    STRCPY(name, str);
			    R->CurrentBar->title = name;
			}
			rxvt_menubar_expose(aR);
		    } else {
			free(R->CurrentBar->title);
			R->CurrentBar->title = NULL;
		    }
		}
	    } else if ((n = rxvt_Str_match(str, "pixmap:")) != 0) {
		str += n;
		rxvt_xterm_seq(aR_ XTerm_Pixmap, str, CHAR_ST);
	    }
#if (MENUBAR_MAX > 1)
	    else if ((n = rxvt_Str_match(str, "rm")) != 0) {
		str += n;
		switch (str[0]) {
		case ':':
		    str++;
		/* FALLTHROUGH */
		case '\0':
		/* FALLTHROUGH */
		case '*':
		    rxvt_menubar_remove(aR_ str);
		    break;
		}
		R->menu_readonly = 1;
	    } else if ((n = rxvt_Str_match(str, "menu")) != 0) {
		str += n;
		switch (str[0]) {
		case ':':
		    str++;
		    /* add/access menuBar */
		    if (*str != '\0' && *str != '*')
			rxvt_menubar_push(aR_ str);
		    break;
		default:
		    if (R->CurrentBar == NULL) {
			rxvt_menubar_push(aR_ "default");
		    }
		}

		if (R->CurrentBar != NULL)
		    R->menu_readonly = 0;	/* allow menu build commands */
	    } else if (!STRCMP(str, "dump")) {
		/* dump current menubars to a file */
		FILE           *fp;

		/* enough space to hold the results */
		char            buffer[32];

		sprintf(buffer, "/tmp/" APL_SUBCLASS "-%u",
			(unsigned int)getpid());

		if ((fp = fopen(buffer, "wb")) != NULL) {
		    rxvt_xterm_seq(aR_ XTerm_title, buffer, CHAR_ST);
		    rxvt_menubar_dump(aR_ fp);
		    fclose(fp);
		}
	    } else if (!STRCMP(str, "next")) {
		if (R->CurrentBar) {
		    R->CurrentBar = R->CurrentBar->next;
		    R->menu_readonly = 1;
		}
	    } else if (!STRCMP(str, "prev")) {
		if (R->CurrentBar) {
		    R->CurrentBar = R->CurrentBar->prev;
		    R->menu_readonly = 1;
		}
	    } else if (!STRCMP(str, "swap")) {
		/* swap the top 2 menus */
		if (R->CurrentBar) {
		    bar_t          *cbprev = R->CurrentBar->prev;
		    bar_t          *cbnext = R->CurrentBar->next;

		    cbprev->next = cbnext;
		    cbnext->prev = cbprev;

		    R->CurrentBar->next = cbprev;
		    R->CurrentBar->prev = cbprev->prev;

		    cbprev->prev->next = R->CurrentBar;
		    cbprev->prev = R->CurrentBar;

		    R->CurrentBar = cbprev;
		    R->menu_readonly = 1;
		}
	    }
#endif				/* (MENUBAR_MAX > 1) */
	    str = next;

	    R->BuildMenu = R->ActiveMenu = NULL;
	    rxvt_menubar_expose(aR);
#ifdef DEBUG_MENUBAR_STACKING
	    fprintf(stderr, "menus are read%s\n",
		    R->menu_readonly ? "only" : "/write");
#endif
	}
	return;
	break;
    }

#if (MENUBAR_MAX > 1)
    if (R->CurrentBar == NULL)
	return;
    if (R->menu_readonly) {
#ifdef DEBUG_MENUBAR_STACKING
	fprintf(stderr, "menus are read%s\n",
		R->menu_readonly ? "only" : "/write");
#endif
	return;
    }
#endif				/* (MENUBAR_MAX > 1) */

    switch (cmd) {
    case '+':
    case '-':
	path = name = str;

	name2 = NULL;
	/* parse STR, allow spaces inside (name)  */
	if (path[0] != '\0') {
	    name = STRCHR(path, MENUITEM_BEG);
	    str = STRCHR(path, MENUITEM_END);
	    if (name != NULL || str != NULL) {
		if (name == NULL || str == NULL || str <= (name + 1)
		    || (name > path && name[-1] != '/')) {
		    rxvt_print_error("menu error <%s>\n", path);
		    break;
		}
		if (str[1] == MENUITEM_BEG) {
		    name2 = (str + 2);
		    str = STRCHR(name2, MENUITEM_END);

		    if (str == NULL) {
			rxvt_print_error("menu error <%s>\n", path);
			break;
		    }
		    name2[-2] = '\0';	/* remove prev MENUITEM_END */
		}
		if (name > path && name[-1] == '/')
		    name[-1] = '\0';

		*name++ = '\0';	/* delimit */
		*str++ = '\0';	/* delimit */

		while (isspace(*str))
		    str++;	/* skip space */
	    }
#ifdef DEBUG_MENU
	    fprintf(stderr,
		    "`%c' path = <%s>, name = <%s>, name2 = <%s>, action = <%s>\n",
		    cmd, (path ? path : "(nil)"), (name ? name : "(nil)"),
		    (name2 ? name2 : "(nil)"), (str ? str : "(nil)")
		);
#endif
	}
	/* process the different commands */
	switch (cmd) {
	case '+':		/* add/replace existing menu or menuitem */
	    if (path[0] != '\0') {
		int             len;

		path = rxvt_menu_find_base(aR_ &(R->BuildMenu), path);
		len = STRLEN(path);

		/* don't allow menus called `*' */
		if (path[0] == '*') {
		    rxvt_menu_clear(aR_ R->BuildMenu);
		    break;
		} else if (len >= 2 && !STRCMP((path + len - 2), "/*")) {
		    path[len - 2] = '\0';
		}
		if (path[0] != '\0')
		    R->BuildMenu = rxvt_menu_add(aR_ R->BuildMenu, path);
	    }
	    if (name != NULL && name[0] != '\0')
		rxvt_menuitem_add(R->BuildMenu,
				  (STRCMP(name, SEPARATOR_NAME) ? name : ""),
				  name2, str);
	    break;

	case '-':		/* delete menu entry */
	    if (!STRCMP(path, "/*") && (name == NULL || name[0] == '\0')) {
		rxvt_menubar_clear(aR);
		R->BuildMenu = NULL;
		rxvt_menubar_expose(aR);
		break;
	    } else if (path[0] != '\0') {
		int             len;
		menu_t         *menu = R->BuildMenu;

		path = rxvt_menu_find_base(aR_ &menu, path);
		len = STRLEN(path);

		/* submenu called `*' clears all menu items */
		if (path[0] == '*') {
		    rxvt_menu_clear(aR_ menu);
		    break;	/* done */
		} else if (len >= 2 && !STRCMP(&path[len - 2], "/*")) {
		    /* done */
		    break;
		} else if (path[0] != '\0') {
		    R->BuildMenu = NULL;
		    break;
		} else
		    R->BuildMenu = menu;
	    }
	    if (R->BuildMenu != NULL) {
		if (name == NULL || name[0] == '\0')
		    R->BuildMenu = rxvt_menu_delete(aR_ R->BuildMenu);
		else {
		    const char     *n1;
		    menuitem_t     *item;
		    menu_t         *BuildMenu = R->BuildMenu;

		    n1 = STRCMP(name, SEPARATOR_NAME) ? name : "";
		    item = rxvt_menuitem_find(BuildMenu, n1);
		    if (item != NULL && item->entry.type != MenuSubMenu) {
			rxvt_menuitem_free(aR_ BuildMenu, item);

			/* fix up the width */
			BuildMenu->width = 0;
			for (item = BuildMenu->head; item != NULL;
			     item = item->next) {
			    short           l = item->len + item->len2;

			    MAX_IT(BuildMenu->width, l);
			}
		    }
		}
		rxvt_menubar_expose(aR);
	    }
	    break;
	}
	break;
    }
}

/* INTPROTO */
void
rxvt_draw_Arrows(pR_ int name, int state)
{
    GC              top, bot;

    int             i;

#ifdef MENU_SHADOW_IN
    state = -state;
#endif
    switch (state) {
    case +1:
	top = R->topShadowGC;
	bot = R->botShadowGC;
	break;			/* SHADOW_OUT */
    case -1:
	top = R->botShadowGC;
	bot = R->topShadowGC;
	break;			/* SHADOW_IN */
    default:
	top = bot = R->scrollbarGC;
	break;			/* neutral */
    }

    if (!R->Arrows_x)
	return;

    for (i = 0; i < NARROWS; i++) {
	const int       w = Width2Pixel(1);
	const int       y = (menuBar_TotalHeight() - w) / 2;
	int             x = R->Arrows_x + (5 * Width2Pixel(i)) / 4;

	if (!name || name == Arrows[i].name)
	    rxvt_Draw_Triangle(R->Xdisplay, R->menuBar.win, top, bot, x, y, w,
			       Arrows[i].name);
    }
    XFlush(R->Xdisplay);
}

/* EXTPROTO */
void
rxvt_menubar_expose(pR)
{
    menu_t         *menu;
    int             x;

    if (!menubar_visible(r) || R->menuBar.win == 0)
	return;

    if (R->menubarGC == None) {
	/* Create the graphics context */
	XGCValues       gcvalue;

	gcvalue.font = R->TermWin.font->fid;

	gcvalue.foreground = (XDEPTH <= 2 ? R->PixColors[Color_fg]
					  : R->PixColors[Color_Black]);
	R->menubarGC = XCreateGC(R->Xdisplay, R->menuBar.win,
				    GCForeground | GCFont, &gcvalue);

    }
/* make sure the font is correct */
    XSetFont(R->Xdisplay, R->menubarGC, R->TermWin.font->fid);
    XSetFont(R->Xdisplay, R->botShadowGC, R->TermWin.font->fid);
    XClearWindow(R->Xdisplay, R->menuBar.win);

    rxvt_menu_hide_all(aR);

    x = 0;
    if (R->CurrentBar != NULL) {
	for (menu = R->CurrentBar->head; menu != NULL; menu = menu->next) {
	    int             len = menu->len;

	    x = (menu->x + menu->len + HSPACE);

#ifdef DEBUG_MENU_LAYOUT
	    rxvt_print_menu_descendants(menu);
#endif

	    if (x >= R->TermWin.ncol)
		len = (R->TermWin.ncol - (menu->x + HSPACE));

	    rxvt_drawbox_menubar(aR_ menu->x, len, +1);
#ifdef USE_XIM
	    if (R->TermWin.fontset)
		XmbDrawString(R->Xdisplay,
			      R->menuBar.win, R->TermWin.fontset,
			      R->menubarGC,
			      (Width2Pixel(menu->x) + Width2Pixel(HSPACE) / 2),
			      menuBar_height() - SHADOW, menu->name, len);
	    else
#endif
		XDrawString(R->Xdisplay, R->menuBar.win, R->menubarGC,
			    (Width2Pixel(menu->x) + Width2Pixel(HSPACE) / 2),
			    menuBar_height() - SHADOW, menu->name, len);

	    if (x >= R->TermWin.ncol)
		break;
	}
    }
    rxvt_drawbox_menubar(aR_ x, R->TermWin.ncol, (R->CurrentBar ? +1 : -1));

/* add the menuBar title, if it exists and there's plenty of room */
    R->Arrows_x = 0;
    if (x < R->TermWin.ncol) {
	const char     *str;
	int             ncol;
	unsigned int    len;
	char            title[256];

	ncol = (int)R->TermWin.ncol;
	if (x < (ncol - (NARROWS + 1))) {
	    ncol -= (NARROWS + 1);
	    R->Arrows_x = Width2Pixel(ncol);
	}
	rxvt_draw_Arrows(aR_ 0, +1);

	str = (R->CurrentBar
	       && R->CurrentBar->title) ? R->CurrentBar->title : "%n-%v";
	for (len = 0; str[0] && len < sizeof(title) - 1; str++) {
	    const char     *s = NULL;

	    switch (str[0]) {
	    case '%':
		str++;
		switch (str[0]) {
		case 'n':
		    s = R->rs[Rs_name];
		    break;	/* resource name */
		case 'v':
		    s = VERSION;
		    break;	/* version number */
		case '%':
		    s = "%";
		    break;	/* literal '%' */
		}
		if (s != NULL)
		    while (*s && len < sizeof(title) - 1)
			title[len++] = *s++;
		break;

	    default:
		title[len++] = str[0];
		break;
	    }
	}
	title[len] = '\0';

	ncol -= (x + len + HSPACE);
	if (len > 0 && ncol >= 0) {
#ifdef USE_XIM
	    if (R->TermWin.fontset)
		XmbDrawString(R->Xdisplay,
			      R->menuBar.win, R->TermWin.fontset,
			      R->menubarGC,
			      Width2Pixel(x) + Width2Pixel(ncol + HSPACE) / 2,
			      menuBar_height() - SHADOW, title, len);
	    else
#endif
		XDrawString(R->Xdisplay, R->menuBar.win, R->menubarGC,
			    Width2Pixel(x) + Width2Pixel(ncol + HSPACE) / 2,
			    menuBar_height() - SHADOW, title, len);
	}
    }
}

/* INTPROTO */
int
rxvt_menubar_mapping(pR_ int map)
{
    int             change = 0;

    if (map && !menubar_visible(r)) {
	R->menuBar.state = 1;
	if (R->menuBar.win == 0)
	    return 0;
	XMapWindow(R->Xdisplay, R->menuBar.win);
	change = 1;
    } else if (!map && menubar_visible(r)) {
	rxvt_menubar_expose(aR);
	R->menuBar.state = 0;
	XUnmapWindow(R->Xdisplay, R->menuBar.win);
	change = 1;
    } else
	rxvt_menubar_expose(aR);

    return change;
}

/* INTPROTO */
int
rxvt_menu_select(pR_ XButtonEvent *ev)
{
    menuitem_t     *thisitem, *item = NULL;
    int             this_y, y;
    menu_t         *ActiveMenu = R->ActiveMenu;

    Window          unused_root, unused_child;
    int             unused_root_x, unused_root_y;
    unsigned int    unused_mask;

    if (ActiveMenu == NULL)
	return 0;

    XQueryPointer(R->Xdisplay, ActiveMenu->win,
		  &unused_root, &unused_child,
		  &unused_root_x, &unused_root_y,
		  &(ev->x), &(ev->y), &unused_mask);

    if (ActiveMenu->parent != NULL && (ev->x < 0 || ev->y < 0)) {
	rxvt_menu_hide(aR);
	return 1;
    }
/* determine the menu item corresponding to the Y index */
    y = SHADOW;
    if (ev->x >= 0 && ev->x <= (ActiveMenu->w - SHADOW)) {
	for (item = ActiveMenu->head; item != NULL; item = item->next) {
	    int             h = HEIGHT_TEXT + 2 * SHADOW;

	    if (isSeparator(item->name))
		h = HEIGHT_SEPARATOR;
	    else if (ev->y >= y && ev->y < (y + h))
		break;
	    y += h;
	}
    }
    if (item == NULL && ev->type == ButtonRelease) {
	rxvt_menu_hide_all(aR);
	return 0;
    }
    thisitem = item;
    this_y = y - SHADOW;

/* erase the last item */
    if (ActiveMenu->item != NULL) {
	if (ActiveMenu->item != thisitem) {
	    for (y = 0, item = ActiveMenu->head; item != NULL;
		 item = item->next) {
		int             h;

		if (isSeparator(item->name))
		    h = HEIGHT_SEPARATOR;
		else if (item == ActiveMenu->item) {
		    /* erase old menuitem */
		    rxvt_drawbox_menuitem(aR_ y, 0);	/* No Shadow */
		    if (item->entry.type == MenuSubMenu)
			rxvt_drawtriangle(aR_ ActiveMenu->w, y, +1);
		    break;
		} else
		    h = HEIGHT_TEXT + 2 * SHADOW;
		y += h;
	    }
	} else {
	    switch (ev->type) {
	    case ButtonRelease:
		switch (item->entry.type) {
		case MenuLabel:
		case MenuSubMenu:
		    rxvt_menu_hide_all(aR);
		    break;

		case MenuAction:
		case MenuTerminalAction:
		    rxvt_drawbox_menuitem(aR_ this_y, -1);
		    {
#ifdef HAVE_NANOSLEEP
			struct timespec rqt;

			rqt.tv_sec = 0;
			rqt.tv_nsec = MENU_DELAY_USEC * 1000;
			nanosleep(&rqt, NULL);
#else
		    /* use select for timing */
			struct timeval  tv;

			tv.tv_sec = 0;
			tv.tv_usec = MENU_DELAY_USEC;
			select(0, NULL, NULL, NULL, &tv);
#endif
		    }
		    /* remove menu before sending keys to the application */
		    rxvt_menu_hide_all(aR);
#ifndef DEBUG_MENU
		    rxvt_action_dispatch(aR_ &(item->entry.action));
#else				/* DEBUG_MENU */
		    fprintf(stderr, "%s: %s\n", item->name,
			    item->entry.action.str);
#endif				/* DEBUG_MENU */
		    break;
		}
		break;

	    default:
		if (item->entry.type == MenuSubMenu)
		    goto DoMenu;
		break;
	    }
	    return 0;
	}
    }
  DoMenu:
    ActiveMenu->item = thisitem;
    y = this_y;
    if (thisitem != NULL) {
	item = ActiveMenu->item;
	if (item->entry.type != MenuLabel)
	    rxvt_drawbox_menuitem(aR_ y, +1);
	if (item->entry.type == MenuSubMenu) {
	    int             x;

	    rxvt_drawtriangle(aR_ ActiveMenu->w, y, -1);

	    x = ev->x + (ActiveMenu->parent
			 ? ActiveMenu->x
			 : Width2Pixel(ActiveMenu->x));

	    if (x >= item->entry.submenu.menu->x) {
		R->ActiveMenu = item->entry.submenu.menu;
		rxvt_menu_show(aR);
		return 1;
	    }
	}
    }
    return 0;
}

/* INTPROTO */
void
rxvt_menubar_select(pR_ XButtonEvent *ev)
{
    menu_t         *menu = NULL;

/* determine the pulldown menu corresponding to the X index */
    if (ev->y >= 0 && ev->y <= menuBar_height() && R->CurrentBar != NULL) {
	for (menu = R->CurrentBar->head; menu != NULL; menu = menu->next) {
	    int             x = Width2Pixel(menu->x);
	    int             w = Width2Pixel(menu->len + HSPACE);

	    if ((ev->x >= x && ev->x < x + w))
		break;
	}
    }
    switch (ev->type) {
    case ButtonRelease:
	rxvt_menu_hide_all(aR);
	break;

    case ButtonPress:
	if (menu == NULL && R->Arrows_x && ev->x >= R->Arrows_x) {
	    int             i;

	    for (i = 0; i < NARROWS; i++) {
		if (ev->x >= (R->Arrows_x + (Width2Pixel(4 * i + i)) / 4)
		    && ev->x < (R->Arrows_x
			    	+ (Width2Pixel(4 * i + i + 4)) / 4)) {
		    rxvt_draw_Arrows(aR_ Arrows[i].name, -1);
		    {
#ifdef HAVE_NANOSLEEP
			struct timespec rqt;

			rqt.tv_sec = 0;
			rqt.tv_nsec = MENU_DELAY_USEC * 1000;
			nanosleep(&rqt, NULL);
#else
		    /* use select for timing */
			struct timeval  tv;

			tv.tv_sec = 0;
			tv.tv_usec = MENU_DELAY_USEC;
			select(0, NULL, NULL, NULL, &tv);
#endif
		    }
		    rxvt_draw_Arrows(aR_ Arrows[i].name, +1);
#ifdef DEBUG_MENUARROWS
		    fprintf(stderr, "'%c': ", Arrows[i].name);

		    if (R->CurrentBar == NULL
			|| (R->CurrentBar->arrows[i].type != MenuAction
			    && R->CurrentBar->arrows[i].type !=
			    MenuTerminalAction)) {
			if (Arrows[i].str != NULL && Arrows[i].str[0])
			    fprintf(stderr, "(default) \\033%s\n",
				    &(Arrows[i].str[2]));
		    } else {
			fprintf(stderr, "%s\n",
				R->CurrentBar->arrows[i].str);
		    }
#else				/* DEBUG_MENUARROWS */
		    if (R->CurrentBar == NULL
			|| rxvt_action_dispatch(r,
						&(R->CurrentBar->arrows[i]))
		       ) {
			if (Arrows[i].str != NULL && Arrows[i].str[0] != 0)
			    rxvt_tt_write(aR_ (Arrows[i].str + 1),
					  Arrows[i].str[0]);
		    }
#endif				/* DEBUG_MENUARROWS */
		    return;
		}
	    }
	}
	/* FALLTHROUGH */

    default:
	/*
	 * press menubar or move to a new entry
	 */
	if (menu != NULL && menu != R->ActiveMenu) {
	    rxvt_menu_hide_all(aR);	/* pop down old menu */
	    R->ActiveMenu = menu;
	    rxvt_menu_show(aR);	/* pop up new menu */
	}
	break;
    }
}

/*
 * general dispatch routine,
 * it would be nice to have `sticky' menus
 */
/* EXTPROTO */
void
rxvt_menubar_control(pR_ XButtonEvent *ev)
{
    switch (ev->type) {
    case ButtonPress:
	if (ev->button == Button1)
	    rxvt_menubar_select(aR_ ev);
	break;

    case ButtonRelease:
	if (ev->button == Button1)
	    rxvt_menu_select(aR_ ev);
	break;

    case MotionNotify:
	while (XCheckTypedWindowEvent(R->Xdisplay, R->TermWin.parent[0],
				      MotionNotify, (XEvent *) ev)) ;

	if (R->ActiveMenu)
	    while (rxvt_menu_select(aR_ ev)) ;
	else
	    ev->y = -1;
	if (ev->y < 0) {
	    Window          unused_root, unused_child;
	    int             unused_root_x, unused_root_y;
	    unsigned int    unused_mask;

	    XQueryPointer(R->Xdisplay, R->menuBar.win,
			  &unused_root, &unused_child,
			  &unused_root_x, &unused_root_y,
			  &(ev->x), &(ev->y), &unused_mask);
	    rxvt_menubar_select(aR_ ev);
	}
	break;
    }
}

/* EXTPROTO */
void
rxvt_map_menuBar(pR_ int map)
{
    if (rxvt_menubar_mapping(aR_ map))
	rxvt_resize_all_windows(aR_ 0, 0, 0);
}
#endif
/*----------------------- end-of-file (C source) -----------------------*/
