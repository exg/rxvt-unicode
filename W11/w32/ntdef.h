

#ifndef __NTDEF
#define __NTDEF
#include <windows.h>

#define INVALID_HANDLE ((HANDLE) -2)
#define NONMAPPED_HANDLE  ((HANDLE) -3)
#define VALID_WINDOW(x) (x && (((NT_window *)x)->w != INVALID_HANDLE))
#define xtrace
#define cjh_printf

#define CNUMTORGB(x) x
	 /* #define printf(x)   *//* x */ 
	 /* #define SetSystemPaletteUse(x) *//* x */

/*	Windows NT Special event aliases	*/

#define USR_MapNotify       0x0401
#define USR_EnterNotify     0x0402
#define USR_LeaveNotify     0x0403
#define USR_Expose          0x0404
#define USR_ResizeRequest   0x0405
#define USR_WakeUp          0x0406
#define USR_ConvertSelection 0x0407

struct NT_child
{
	struct NT_window *w;
	struct NT_child *next;
} NT_child;

typedef struct NT_window
{
	HWND w;
	HBRUSH bg;
	int parentRelative;
	struct NT_window *parent;      /* parent of this window */
	struct NT_window *next;        /* next window in list */
	struct NT_child *child;        /* points to list of children */
	int    x, y;                   /* Position */
	unsigned int wdth, hght;       /* Dimensions */
	char *title_text;
	struct NT_prop_list *props;    /* linked list of properties.*/
	HDC    hDC;
	HBITMAP hBitmap;
	int min;
	int minx, miny;  /* minimum window size */
	int top_flag;
	long mask;   /* selectInputMask */
} NT_window;

/* Routine declarations */

struct NT_window      *NT_find_window_from_id();
int                    NT_delete_window();

struct NT_window      *NT_new_window();
int NT_add_child(NT_window *parent,NT_window *child);
struct NT_window *NT_find_child(NT_window *w,unsigned long mask,
								   unsigned long val);
int NT_del_child(NT_window *parent,NT_window *child);
void freeMemory(void *p);
void *allocateMemory(int s);
void initQ();
void catchNextDestroyClipboard();

void NT_SetAtom(ATOM class);
HWND NT_create_window(char *title,DWORD style,int x,int y,int w, int h,HWND parent);
LONG NT_handleMsg(HWND hWnd,UINT message,UINT wParam,LONG lParam);
HBITMAP NT_getWallpaper();
void NT_moveWindow(NT_window *ntw, BOOL repaint);
#endif
