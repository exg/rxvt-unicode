/*****************************************************************\


	Library of X window functions which call Windows 32
	equivalent functions.

	Some data structures are maintained by this code,
	simulating the operation of an X server and window manager.

	Aug/Sep-92	xyz	$$1 Created.
        Oct-92  abc $$2 Added color stuff.

\******************************************************************/

#ifndef __XNT
#define __XNT

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include "ntdef.h"

/* Local Data */

static void                   NT_set_GC_pen();
static void                   NT_set_GC_brush();
static HPEN                   NT_get_GC_pen();
static HBRUSH                 NT_get_GC_brush();
static HBRUSH                 NT_get_GC_bgbrush();

void NT_set_rop();
double NT_deg64_to_rad(int a);

/*----------------------------------------------------------------*\
| FUNCTIONS TO MAINTAIN AN INTERNAL LIST OF WINDOWS AND THEIR      |
| ATTRIBUTES - AS WOULD BE MAINTAINED IN THE X SERVER.             |
\*----------------------------------------------------------------*/

/* Structure to hold pen and brush info in GC ext_data field */

typedef struct NTGC_
{
	HPEN	pen;
	HBRUSH	brush;
	HBRUSH	bgbrush;
} NTGC;

HDC
cjh_get_dc(NT_window *window)
{
	/* pixmaps have to do SelectObject() on their dc's */
	if (window->hDC == INVALID_HANDLE)
		if (window->w == INVALID_HANDLE)
		{
			window->hDC= CreateDC("DISPLAY", NULL, NULL, NULL);
		}
	else
		window->hDC=GetDC(window->w);
    return window->hDC;
}

int
cjh_rel_dc(NT_window *window,HDC dc)
{
	return TRUE;
	/*	return ReleaseDC(window, dc); */
}

HDC
drawableGetDC(Drawable drawable)
{
	cjh_get_dc((NT_window *)drawable);
}

int
drawableRelDC(Drawable drawable, HDC hDC)
{
	cjh_rel_dc((NT_window *)drawable, hDC);
}



/*****************************************************************\

	Function: XOpenDisplay
	Inputs:   Display name

	Comments: Fills out a Display structure and a Visual and Screen.
		  Hopefully all the X macros should work with this
		  structure.  Note that the default visual is for a
		  True colour screen (24bits).
\*****************************************************************/
Display *
XOpenDisplay (name)
const char *name;
{
	static char vstring[]="NT Xlibemu",
		*vs,*dn;

	Display *d = NULL;
	Screen *scrd;
	static Depth dlist[1];
	static Visual vlist[1];
	Colormap cmap;
	RECT rect;
	int depth;
	HDC rootDC = CreateDC("DISPLAY",NULL,NULL,NULL);

	depth = GetDeviceCaps(rootDC, BITSPIXEL);
	
	xtrace("XOpenDisplay\n");

	initQ();
	
	dlist[0].depth=depth;
	dlist[0].nvisuals=1;
	dlist[0].visuals=vlist;
	vlist[0].ext_data=NULL;
	vlist[0].visualid=0;
	vlist[0].class=PseudoColor;
	vlist[0].bits_per_rgb=8;
	vlist[0].map_entries=256;
	vlist[0].red_mask=255;
	vlist[0].green_mask=255<<8;
	vlist[0].blue_mask=255<<16;
	scrd=(Screen *) allocateMemory (sizeof (Screen));
	(NT_window*)(scrd->root)= NT_new_window();
	((NT_window*)(scrd->root))->w=GetDesktopWindow();
	((NT_window*)(scrd->root))->parent=0;
	GetWindowRect(GetDesktopWindow(),&rect);
	scrd->width=rect.right-rect.left;
	scrd->height=rect.bottom-rect.top;
	scrd->mwidth=260;
	scrd->mheight=190;
	scrd->ndepths=1;
	scrd->depths=dlist;
	scrd->root_depth=depth;
	scrd->root_visual=vlist;
	scrd->default_gc=NULL;
	scrd->cmap=cmap;
	scrd->white_pixel=0xffffff;
	scrd->black_pixel=0;

	d=(Display *) allocateMemory (sizeof (Display));
	scrd->display=d;
	vs=(char *) allocateMemory (sizeof (char)*strlen (vstring)+1);
	dn=(char *) allocateMemory (sizeof (char)*strlen (name)+1);
	strcpy (vs,vstring);
	strcpy (dn,name);
	d->ext_data=NULL;
	d->fd=0;
	d->proto_major_version=11;
	d->proto_minor_version=4;
	d->vendor=vs;
	d->release=4;
	d->display_name=dn;
	d->nscreens=1;
	d->screens=scrd;
	d->max_keycode=255;

	return (d);
}


int
XCloseDisplay(Display *display)
{
  NT_window *wanderer;

  xtrace("XCloseDisplay\n");
/* Do something ? */
/* Must GlobalDelete all atoms/properties leftover */
  return 0;
}

char *
XDisplayString(Display *display)
{
	return (display->display_name);
}


int
XFlush(Display *display)
{
	xtrace("XFlush\n");
	return 0;
}


int
XSync(display,discard)
Display *display;
int discard;
{
	/* Do nothing here either */
	return 0;
}

/*****************************************************************\

	Function: XGetVisualInfo
	Inputs:   display, info mask, template, number of matches.
	Returned: List of XVisualInfo structures, one for each matching
		  Visual.

	Comments: Behaves like X routine, but there is only ever one
		  Visual, so the returned list is never longer than one.

\*****************************************************************/
XVisualInfo *
XGetVisualInfo(display,vinm,vint,n)
Display *display;
long vinm;
XVisualInfo *vint;
int *n;
{
	static XVisualInfo xvi;
	int status=1;
	xtrace("XGetVisualInfo\n");

	if ((vinm&VisualIDMask|vinm==VisualAllMask)&&
            vint->visualid!=display->screens->root_visual->visualid)
		status=0;
	if ((vinm&VisualScreenMask|vinm==VisualAllMask)&&
	    vint->screen!=0)
		status=0;
	if ((vinm&VisualDepthMask|vinm==VisualAllMask)&&
	    vint->depth!=24)
		status=0;
	if ((vinm&VisualClassMask|vinm==VisualAllMask)&&
	    vint->class!=display->screens->root_visual->class)
		status=0;
	if ((vinm&VisualRedMaskMask|vinm==VisualAllMask)&&
	    vint->red_mask!=display->screens->root_visual->red_mask)
		status=0;
	if ((vinm&VisualGreenMaskMask|vinm==VisualAllMask)&&
	    vint->green_mask!=display->screens->root_visual->green_mask)
		status=0;
	if ((vinm&VisualBlueMaskMask|vinm==VisualAllMask)&&
	    vint->blue_mask!=display->screens->root_visual->blue_mask)
		status=0;
	if ((vinm&VisualColormapSizeMask|vinm==VisualAllMask)&&
	    vint->colormap_size!=display->screens->root_visual->map_entries)
		status=0;
	if ((vinm&VisualBitsPerRGBMask|vinm==VisualAllMask)&&
	    vint->bits_per_rgb!=display->screens->root_visual->bits_per_rgb)
		status=0;
	if (status==1)
	{
		xvi.visualid=display->screens->root_visual->visualid;
		xvi.screen=0;
		xvi.depth=display->screens->root_visual->bits_per_rgb;
		xvi.class=display->screens->root_visual->class;
		xvi.red_mask=display->screens->root_visual->red_mask;
		xvi.green_mask=display->screens->root_visual->green_mask;
		xvi.blue_mask=display->screens->root_visual->blue_mask;
		xvi.colormap_size=display->screens->root_visual->map_entries;
		xvi.bits_per_rgb=display->screens->root_visual->bits_per_rgb;
		xvi.visual=display->screens->root_visual;
		*n=1;
		return (&xvi);
	}
	*n=0;
	return (&xvi);
}

StatusDef XMatchVisualInfo(
    Display*		display,
    int			screen,
    int			depth,
    int			class,
    XVisualInfo*	vinfo_return)
{
	int status=0;
	xtrace("XMatchVisualInfo\n");
	return status;
}

/*****************************************************************\

	Function: XClearWindow
	Inputs:   display, window

	Comments: As mentioned, the Window structure is not the one windows
		  recognises.  The background colour for the window is
		  stored in this structure.

		  The sequence of GetDC, CreateBrush/Pen, SelectObject,
		  <draw stuff>, DeleteObject, ReleaseDC occurs in all the
		  drawing functions.

\*****************************************************************/
int
XClearWindow(display, w)
Display *display;
Window w;
{
	RECT  rRect;
	HBRUSH hbrush;
	HDC hDC;
	HANDLE oldObj;
	int oldROP;
	NT_window *window = (NT_window *)w;
	xtrace("XClearWindow\n");

	if (VALID_WINDOW(window)) {
		rRect.left= rRect.right=rRect.bottom=rRect.top =0;
	
		hDC = cjh_get_dc(window);
		oldROP = SetROP2(hDC,R2_COPYPEN);
		hbrush = window->bg;
		oldObj = SelectObject(hDC, hbrush);
		GetClientRect(window->w, &rRect);
		FillRect(hDC, &rRect, hbrush);
		SelectObject(hDC, oldObj);
		//	DeleteObject(hbrush);
		SetROP2(hDC,oldROP);
		cjh_rel_dc(window,hDC);
	}
	
	return 0;
}

/*****************************************************************\

	Function: XCreateSimpleWindow
	Inputs:   display, parent window, geometry, border width,
		  border colour, background colour.
	Returned: Window ID

	Comments: The first time a window is made by the application, it
		  has to be registered.
		  To simulate the action of a window manager, the toplevel
		  client window is reparented and a frame window is created.
		  A MapNotify event is sent to the new client.
		  Note that child windows are created in the manner of the
		  default X behaviour, ie. each is clipped individually.


        NOTE:     This routine has now changed. As part of our role as
                  Window manager, we now defer creation of the windows until
                  they are mapped. The fact that a window has been created
                  and not mapped is flagged to other routines by setting the
                  w element of the structure to -1.
                  WE STILL CREATE THE Window STRUCTURES.
                  (SEE XMapWindow)

\*****************************************************************/

Window
XCreateSimpleWindow(display, parent,x, y, w, h, brd, brd_col, bg)
Display *display;
Window  parent;
int     x, y;
unsigned int brd,w,h;
unsigned long bg, brd_col;
{
	NT_window  *canvas;
	xtrace("XCreateSimpleWindow\n");

	canvas = NT_new_window();
	
	canvas->x = x;
	canvas->y = y;
	canvas->wdth = w;
	canvas->hght = h;
	NT_add_child((NT_window *)parent,canvas);
	canvas->bg=CreateSolidBrush (CNUMTORGB(bg));
	canvas->parent=(NT_window *)parent;
	canvas->title_text = NULL;
	if (canvas->parent->w == GetDesktopWindow())
	{
		if (x==0 && y==0)
		{
			canvas->x = -1;
			canvas->y = -1;
		}
		canvas->top_flag = TRUE;
	}
	else
		canvas->top_flag = 0;
	return ((Window)canvas);
}


/*****************************************************************\

	Function: XCreateWindow
	Inputs:   display, parent window, geometry, border width, depth,
		  class, visual, attributes mask, attributes structure
	Returned: Window ID

	Comments: Simply calls XCreateSimpleWindow.  Some of the arguments
		  are ignored :-).

\*****************************************************************/

Window
XCreateWindow(display,parent,x,y,width,height,bw,depth,class,visual,
			valuemask,attr)
Display *display;
Window  parent;
int x,y;
unsigned int width,height,bw;
int depth;
unsigned int class;
Visual *visual;
unsigned long valuemask;
XSetWindowAttributes *attr;
{
	xtrace("XCreateWindow\n");
	return (XCreateSimpleWindow(display,parent,x,y,width,height,bw,
		attr->border_pixel,attr->background_pixel));
}


/*****************************************************************\

	Function: XDestroyWindow
	Inputs:   Display, window to be destroyed.

	Comments: Removes a window from the server.  

\*****************************************************************/
int
XDestroyWindow(display,w)
Display *display;
Window w;
{
	NT_window *ntw = (NT_window *)w;
	xtrace("XDestroyWindow\n");
        if (ntw->hDC != INVALID_HANDLE)
		{
          ReleaseDC(ntw->w,ntw->hDC);
		  ntw->hDC =  INVALID_HANDLE;
		}
		
	/*DestroyWindow(w->w);*/
	NT_delete_window(ntw);   /* Remove window from data structure */
	return 0;
}


/*****************************************************************\

	Function: XGetGeometry
	Inputs:   display, window
	Returned: root window, screen depth, geometry, border width

	Comments: fetches information from the windows kernel and our
		  display structure.

\*****************************************************************/

StatusDef
XGetGeometry(display,w,root,x,y,width,height,bw,depth)
Display *display;
Drawable w;
Window *root;
int *x,*y;
unsigned int *width,*height;
unsigned int *bw,*depth;
{
	NT_window *ntw = (NT_window *)w;
	RECT r;
	xtrace("XGetGeometry\n");

	*root=display->screens[0].root;
	*depth=24;

	GetWindowRect(ntw->w,&r);
	*x=r.left;
	*y=r.top;
	GetClientRect(ntw->w,&r);
	*width=r.right-r.left;
	if (*width<ntw->minx)
		*width=ntw->minx;
	*height=r.bottom-r.top;
	if (*height<ntw->miny)
		*height=ntw->miny;
	*bw=(*width-(r.right-r.left))/2;
	
	return 0;
}


/*****************************************************************\

	Function: XGetWindowAttributes
	Inputs:   display, window, attributes
	Returned: 1 = ok.

	Comments: Fills out attributes structure.

\*****************************************************************/

StatusDef
XGetWindowAttributes(display,w,wattr)
Display *display;
Window w;
XWindowAttributes *wattr;
{
	xtrace("XGetWindowAttributes\n");
	XGetGeometry(display,w,&wattr->root,&wattr->x,&wattr->y,&wattr->width,
		&wattr->height,&wattr->border_width,&wattr->depth);
	wattr->class=InputOutput;
	wattr->bit_gravity=StaticGravity;
	wattr->win_gravity=CenterGravity;
	wattr->backing_store=NotUseful;
	wattr->backing_planes=0;
	wattr->backing_pixel=0;
	wattr->save_under=0;
	wattr->colormap=None;
	wattr->map_installed=TRUE;
	wattr->map_state=IsViewable;
	wattr->override_redirect=FALSE;
	wattr->screen=display->screens;
	return (1);
}



int
XSelectInput(display, window, mask)
Display *display;
Window  window;
long    mask;
{
	NT_window *ntw = (NT_window *)window;
	xtrace("XSelectInput\n");
	ntw->mask=mask;
	return 0;
}

void NT_dispError(char *msg)
{
	LPVOID lpMsgBuf=NULL;
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,
				  GetLastError(),
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  (LPTSTR) &lpMsgBuf,
				  0,
				  NULL);
	MessageBox( NULL, lpMsgBuf, msg, MB_OK|MB_ICONINFORMATION );
	LocalFree( lpMsgBuf );
}


/*****************************************************************\

	Function: XMapWindow
	Inputs:   display, window to be mapped

	Comments: If the specified window is not already mapped, this
		  routine calls the Windows function which displays it.
		  Again, frames have to be mapped as well as clients.

\*****************************************************************/
int
XMapWindow(display, window)
Display *display;
Window window;
{
	NT_window *ntw = (NT_window *)window;
	RECT rect;
	unsigned char *hints;
	Atom property;
	Atom ret_type;
	int ret_format;
	DWORD frame_style;
	long ret_nitems;
	long ret_after;
	HDC hDC;
	char *title = "";
	xtrace("XMapWindow\n");
	
	if (ntw->w == INVALID_HANDLE)
	{
		frame_style = WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;
		if (ntw->top_flag)
		{
			/* frame_style = WS_CLIPCHILDREN;
			   frame_style |=  WS_BORDER;
			   frame_style |= WS_THICKFRAME;
			   frame_style |= WS_CAPTION;
			   frame_style |= WS_POPUP;
			   frame_style |= WS_SYSMENU;
			   frame_style |= WS_MINIMIZEBOX;
			   frame_style |= WS_MAXIMIZEBOX;
			*/
			frame_style = WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN;
			ntw->hght +=  GetSystemMetrics(SM_CYSIZE)+(GetSystemMetrics(SM_CYBORDER)+GetSystemMetrics(SM_CYFRAME))*2;
			ntw->wdth += (GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXFRAME))*2;
			title = ntw->title_text;
			if (ntw->x == -1 && ntw->y == -1)
			{
				ntw->x = CW_USEDEFAULT;
				ntw->y = CW_USEDEFAULT;
			}
		}
		else if (ntw->x == -1 && ntw->y == -1)
		{
			GetClientRect(ntw->parent->w,&rect);
			ntw->x = rect.left;
			ntw->y = rect.top;
			ntw->wdth = rect.right-rect.left;
			ntw->hght = rect.bottom - rect.top;
		}

		ntw->hDC = INVALID_HANDLE;
		
		if (ntw->parent->w == INVALID_HANDLE)
		{
			XMapWindow(display, (Window)ntw->parent);
		}
		ntw->w = NT_create_window(title,frame_style,
								 ntw->x,ntw->y,ntw->wdth,ntw->hght,
								 ntw->parent->w);
		if (ntw->w==NULL) NT_dispError("create window1");
		hDC = cjh_get_dc(ntw);
		PostMessage(ntw->w,USR_MapNotify,0,0);
	}
	if (IsWindowVisible(ntw->w)==0)
	{
		ShowWindow(ntw->w, SW_SHOW);
		PostMessage(ntw->w,USR_MapNotify,0,0);
	}
	UpdateWindow(ntw->w);
	return 0;
}

int
XIconifyWindow(Display *display,
    Window w,
    int screen_number)
{
	xtrace("XIconifyWindow\n");
	return 0;
}


/*****************************************************************\

	Function: XCreateGC
	Inputs:   display, window, mask of setup values, setup values.
	Returned: GC pointer.

	Comments: Fills out a GC structure with the X defaults unless
		  the caller specifies otherwise.

\*****************************************************************/

GC
XCreateGC(display, window, mask, gc_values)
Display *display;
Drawable window;
unsigned long mask;
XGCValues *gc_values;
{
	GC	local_gc;
	int	size;
	char	*ptr;
	xtrace("XCreateGC\n");

	size = sizeof(GC);

	ptr = (char *)allocateMemory((size_t)1000);
	local_gc = (GC)ptr;
	local_gc->ext_data = NULL;
	local_gc->gid=(GContext) window;
	local_gc->rects=FALSE;
	local_gc->dashes=FALSE;

	if (mask&GCArcMode)
		local_gc->values.arc_mode=gc_values->arc_mode;
	else
		local_gc->values.arc_mode=ArcPieSlice;
	if (mask&GCBackground)
		local_gc->values.background=gc_values->background;
	else
		local_gc->values.background=display->screens->white_pixel;
	if (mask&GCCapStyle)
		local_gc->values.cap_style=gc_values->cap_style;
	else
		local_gc->values.cap_style=CapButt;
	if (mask&GCClipMask)
		local_gc->values.clip_mask=gc_values->clip_mask;
	else
		local_gc->values.clip_mask=None;
	if (mask&GCClipXOrigin)
		local_gc->values.clip_x_origin=gc_values->clip_x_origin;
	else
		local_gc->values.clip_x_origin=0;
	if (mask&GCClipYOrigin)
		local_gc->values.clip_y_origin=gc_values->clip_y_origin;
	else
		local_gc->values.clip_y_origin=0;
	if (mask&GCDashList)
		local_gc->values.dashes=gc_values->dashes;
	else
		local_gc->values.dashes=4;
	if (mask&GCDashOffset)
		local_gc->values.dash_offset=gc_values->dash_offset;
	else
		local_gc->values.dash_offset=0;
	if (mask&GCFillRule)
		local_gc->values.fill_rule=gc_values->fill_rule;
	else
		local_gc->values.fill_rule=EvenOddRule;
	if (mask&GCFillStyle)
		local_gc->values.fill_style=gc_values->fill_style;
	else
		local_gc->values.fill_style=FillSolid;
	if (mask&GCFont)
		local_gc->values.font=gc_values->font;
	else
		local_gc->values.font= 999;/*"fixed";*/
	if (mask&GCForeground)
		local_gc->values.foreground=gc_values->foreground;
	else
		local_gc->values.foreground=display->screens->black_pixel;
	if (mask&GCFunction)
		local_gc->values.function=gc_values->function;
	else
		local_gc->values.function=GXcopy;
	if (mask&GCGraphicsExposures)
		local_gc->values.graphics_exposures=gc_values->graphics_exposures;
	else
		local_gc->values.graphics_exposures=True;
	if (mask&GCJoinStyle)
		local_gc->values.join_style=gc_values->join_style;
	else
		local_gc->values.join_style=JoinMiter;
	if (mask&GCLineStyle)
		local_gc->values.line_style=gc_values->line_style;
	else
		local_gc->values.line_style=LineSolid;
	if (mask&GCLineWidth)
		local_gc->values.line_width=gc_values->line_width;
	else
		local_gc->values.line_width=0;
	if (mask&GCPlaneMask)
		local_gc->values.plane_mask=gc_values->plane_mask;
	else
		local_gc->values.plane_mask=255;
	if (mask&GCStipple)
		local_gc->values.stipple=gc_values->stipple;
	else
		local_gc->values.stipple=0;
	if (mask&GCSubwindowMode)
		local_gc->values.subwindow_mode=gc_values->subwindow_mode;
	else
		local_gc->values.subwindow_mode=ClipByChildren;
	if (mask&GCTile)
		local_gc->values.tile=gc_values->tile;
	else
		local_gc->values.tile=0;
	if (mask&GCTileStipXOrigin)
		local_gc->values.ts_x_origin=gc_values->ts_x_origin;
	else
		local_gc->values.ts_x_origin=0;
	if (mask&GCTileStipYOrigin)
		local_gc->values.ts_y_origin=gc_values->ts_y_origin;
	else
		local_gc->values.ts_y_origin=0;

	local_gc->dirty = ~0;

	return (local_gc);
}

int
XFreeGC(display, gc)
Display *display;
GC gc;
{
	freeMemory(gc);
}

/*****************************************************************\

	Function: XSetForeground
	Inputs:   display, gc, colour.

	Comments: Colour is an RGB triple (24bits).
\*****************************************************************/
int
XSetForeground(display, gc, color)
Display *display;
GC gc;
unsigned long    color;
{
	xtrace("XSetForegrond\n");
	gc->values.foreground=color;
	gc->dirty |= GCForeground;
	return 0;
}


/*****************************************************************\

	Function: XDrawString
	Inputs:   display, window, gc, position, string, length of string.

	Comments: Writes text to the screen in the manner of X windows.
		  Note that the y origin is on the text baseline, ie.
		  the lowest part of a letter o rests on the baseline and
		  descenders fall below it.
		  The text is transparent.

\*****************************************************************/

int
XDrawString(Display *display, Drawable window,
    GC gc, int x, int y, const char* str, int len)
{
	HDC          hDC;
	TEXTMETRIC   tmet;
	HFONT old;
	xtrace("XDrawString\n");

	if (VALID_WINDOW(window))
	{
		hDC = drawableGetDC(window);
		SetBkMode(hDC,TRANSPARENT);
		SetTextColor(hDC,CNUMTORGB(gc->values.foreground));
		old=SelectObject(hDC,(HFONT)gc->values.font);
		GetTextMetrics(hDC,&tmet);
		TextOut(hDC,x,y-tmet.tmAscent,str,len);
		SelectObject(hDC,old);
		drawableRelDC(window,hDC);
	}
	return 0;
}



int
XDrawString16(Display *display, Drawable window,
    GC gc, int x, int y,
    const XChar2b* str,
    int len)
{
	xtrace("XDrawString16\n");
	XDrawString(display, window, gc, x, y, (const char*)str, len*2);
	return 0;
}
XDrawImageString(
	Display* display,
	Drawable d,
	GC gc,
	int x,
	int y,
	const char* string,
	int length)
{
	HDC hDC = NULL;
	TEXTMETRIC   tmet;
	HFONT old;
	xtrace("XDrawImageString\n");
	if (VALID_WINDOW(d))
	{
	    hDC = drawableGetDC(d);
	    SetBkMode(hDC,TRANSPARENT);
	    SetBkColor(hDC, CNUMTORGB(gc->values.background));	
	    SetTextColor(hDC,CNUMTORGB(gc->values.foreground));
	    old=SelectObject(hDC,(HFONT)gc->values.font);
	    if (GetTextMetrics(hDC,&tmet) && length>0) {
		RECT fill;
		fill.top = y-tmet.tmAscent;
		fill.bottom = y+tmet.tmDescent;
		fill.left = x;
		fill.right = x + (tmet.tmAveCharWidth * length);
		FillRect( hDC, &fill, NT_get_GC_bgbrush(hDC,gc));
	    }
	    TextOut( hDC, x, y-tmet.tmAscent, string, length ); 
	    SelectObject(hDC,old);
	    drawableRelDC(d,hDC);
	}	
	return 0;
}

int
XDrawImageString16(Display *display, Drawable window,
    GC gc, int x, int y,
    const XChar2b* str,
    int len)
{
	xtrace("XDrawImageString16\n");
	XDrawImageString(display, window, gc, x, y, (const char*)str, len*2);
	return 0;
}


/*****************************************************************\

	Function: XFillRectangle
	Inputs:   display, window, gc, geometry.

	Comments: fills rectangles in uniform colours.  No tiles/Pixmaps
		  are implemented yet.

\*****************************************************************/

int
XFillRectangle (display,window,gc,x,y,w,h)
Display *display;
Drawable window;
GC gc;
int x,y;
unsigned int w,h;
{
	RECT rct;
	HBRUSH hbrush;
	HDC hDC;
	HANDLE oldObj;
	int ret;
	xtrace("XFillRectangle\n");

	if (VALID_WINDOW(window))
	{
		hDC = drawableGetDC(window);
		NT_set_rop(hDC,gc);
		hbrush = NT_get_GC_brush(hDC,gc);
		oldObj = SelectObject(hDC, hbrush);
		rct.left=(LONG) x;
		rct.right=(LONG) (x+w);
		rct.top=(LONG) y;
		rct.bottom=(LONG) (y+h);
		ret=FillRect(hDC, &rct, hbrush);
		SelectObject(hDC, oldObj);
		drawableRelDC(window,hDC);
	}
	return (ret);
}


/*****************************************************************\

	Function: XClearArea
	Inputs:   display, geometry, exposure events allowed.

	Comments: Straightforward.

\*****************************************************************/
int
XClearArea(display,w,x,y,width,height,exposures)
Display *display;
Window w;
int x,y;
unsigned int width,height;
BoolDef exposures;
{
	NT_window *ntw = (NT_window *)w;
	RECT rct;
	HBRUSH hbrush;
	HDC hDC;
	HANDLE oldObj;
	int oldROP;
	xtrace("XClearArea\n");

	if (VALID_WINDOW(ntw))
	{
		hDC = cjh_get_dc(ntw);
		oldROP = SetROP2(hDC,R2_COPYPEN);
		hbrush=ntw->bg;
		oldObj = SelectObject(hDC,hbrush);
		GetClientRect(ntw->w,&rct);

		if ((width != 0) && (height != 0))
		{
			rct.left=(LONG)x;
			rct.right=(LONG)(x+width);
			rct.top=(LONG)y;
			rct.bottom=(LONG)(y+height);
			FillRect(hDC,&rct,hbrush);
		}

		SelectObject(hDC, oldObj);
		// DeleteObject(hbrush);
		SetROP2(hDC,oldROP);
		cjh_rel_dc(ntw,hDC);
	}
	return 0;
}


Region
XCreateRegion()
{
	HRGN hrgn;
	xtrace("XCreateRegion\n");

	hrgn=CreateRectRgn(0,0,1,1);
	return ((Region)hrgn);
}


/* Untested.  The Region stuff needs thinking about. */

int
XClipBox(hrgn,rect)
Region hrgn;
XRectangle *rect;
{
	RECT rct;
	xtrace("XClipBox\n");

	GetRgnBox((HRGN)hrgn,&rct);
	rect->x=(short)rct.left;
	rect->y=(short)rct.top;
	rect->width=(unsigned short)(rct.right-rct.left);
	rect->height=(unsigned short)(rct.bottom-rct.top);
	return TRUE;/*(rect);*/
}


int
XSetRegion(display,gc,hrgn)
Display *display;
GC gc;
Region hrgn;
{
	/* What to do here ? */
	xtrace("XSetRegion\n");
	return 0;
}


int
XDestroyRegion(hrgn)
Region hrgn;
{
	xtrace("XDestroyRegion\n");
	DeleteObject(hrgn);
	return 0;
}


int
XUnionRectWithRegion(rect,hrgnsrc,hrgndest)
XRectangle *rect;
Region hrgnsrc,hrgndest;
{
	HRGN temp;
	xtrace("XUnionRectWithRegion\n");
	temp=CreateRectRgn(rect->x,rect->y,rect->x+rect->width,
				rect->y+rect->height);
	CombineRgn((HRGN)hrgndest,(HRGN)hrgnsrc,temp,RGN_OR);
	return 0;
}


/*****************************************************************\

	Function: XDrawArc
	Inputs:   display, window, gc, bounding box geometry, arc angles.

	Comments: Works fine.

\*****************************************************************/

int
XDrawArc(display,w,gc,x,y,width,height,a1,a2)
Display *display;
Drawable w;
GC gc;
int x,y;
unsigned int width,height;
int a1,a2;
{
	HDC hDC;
	HPEN hpen;
	int tmp;
	double NT_deg64_to_rad();
	HANDLE oldObj;
	xtrace("XDrawArc\n");
	if (a2>=0)
		a2+=a1;
	else
	{
		tmp=a1;
		a1-=a2;
		a2=tmp;
	}

	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		hpen = NT_get_GC_pen(hDC,gc);
		oldObj = SelectObject(hDC,hpen);
		Arc(hDC,x,y,x+width-1,y+height-1,
			(int) (x+width/2+width*cos(NT_deg64_to_rad(a1))),
			(int) (y+height/2-height*sin(NT_deg64_to_rad(a1))),
			(int) (x+width/2+width*cos(NT_deg64_to_rad(a2))),
			(int) (y+height/2-height*sin(NT_deg64_to_rad(a2))));
		SelectObject(hDC, oldObj);
		drawableRelDC(w,hDC);
	}
	return 0;
}


/*****************************************************************\

	Function: XFillArc
	Inputs:   display, window, gc, geometry as above.

	Comments: Not tested at all, but should work.

\*****************************************************************/

int
XFillArc(display,w,gc,x,y,width,height,a1,a2)
Display *display;
Drawable w;
GC gc;
int x,y;
unsigned int width,height;
int a1,a2;
{
	HDC hDC;
	HBRUSH hbrush;
	int tmp;
        HANDLE oldObj;
		xtrace("XFillArc\n");
	if (a2>=0)
		a2+=a1;
	else
	{
		tmp=a1;
		a1-=a2;
		a2=tmp;
	}
	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		hbrush = NT_get_GC_brush(hDC,gc);
		oldObj = SelectObject(hDC,hbrush);
		if (gc->values.arc_mode==ArcChord)
		{
			Chord(hDC,x,y,x+width,y+height,
				  (int) (x+width/2+width*cos(NT_deg64_to_rad(a1))),
				  (int) (y+height/2+height*sin(NT_deg64_to_rad(a1))),
				  (int) (x+width/2+width*cos(NT_deg64_to_rad(a2))),
				  (int) (y+height/2+height*sin(NT_deg64_to_rad(a2))));
		}
		else
		{
			Pie(hDC,x,y,x+width,y+height,
				(int) (x+width/2+width*cos(NT_deg64_to_rad(a1))),
				(int) (y+height/2+height*sin(NT_deg64_to_rad(a1))),
				(int) (x+width/2+width*cos(NT_deg64_to_rad(a2))),
				(int) (y+height/2+height*sin(NT_deg64_to_rad(a2))));
		}
		SelectObject(hDC, oldObj);
		drawableRelDC(w,hDC);
	}
	return 0;
}


/*****************************************************************\

	Function: XFillPolygon
	Inputs:   display, window, gc, points list, number of points,
		  shape hint, relative drawing mode.

	Comments: Works for convex polygons.  Untested on otherwise.
	          Optimisation hints are unused, as is the mode.

\*****************************************************************/

int
XFillPolygon(display,w,gc,points,nps,shape,mode)
Display *display;
Drawable w;
GC gc;
XPoint *points;
int nps,shape,mode;
{
	HBRUSH hbrush;
	int n;
	POINT ntps[1000];
	HDC hDC;
	HANDLE oldObj;
	xtrace("XFillPolygon\n");
 	/*ntps=allocateMemory(sizeof(POINT)*nps);*/
	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		hbrush = NT_get_GC_brush(hDC,gc);
		oldObj = SelectObject(hDC,hbrush);
		for (n=0;n<nps;++n)
		{
			(ntps+n)->x=(LONG)points->x;
			(ntps+n)->y=(LONG)(points++)->y;
		}
		Polygon(hDC,ntps,nps);
		SelectObject(hDC, oldObj);
		drawableRelDC(w,hDC);
	}
	
	/*free(ntps);*/
	return 0;
}


/*****************************************************************\

	Function: XDrawLine
	Inputs:   display, window, geometry.

	Comments: Seems to work ok.

\*****************************************************************/

int
XDrawLine(display,w,gc,x1,y1,x2,y2)
Display *display;
Drawable w;
GC gc;
int x1,y1,x2,y2;
{
	HDC hDC;
	HPEN hpen;
	RECT da;
	HANDLE oldObj;
	xtrace("XDrawLine\n");

	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		hpen = NT_get_GC_pen(hDC,gc);
		oldObj = SelectObject(hDC,hpen);
		MoveToEx(hDC,x1,y1,NULL);
		LineTo(hDC,x2,y2);
		SelectObject(hDC, oldObj);
		drawableRelDC(w,hDC);
	}
	return 0;
}


/*****************************************************************\

	Function: XDrawLines
	Inputs:   display, window, gc, points list, number of points, mode.

	Comments: Untested.

\*****************************************************************/

int
XDrawLines(display,w,gc,points,nps,mode)
Display *display;
Drawable w;
GC gc;
XPoint *points;
int nps,mode;
{
	HPEN hpen;
	int n;
	POINT pts[1000];
	HDC hDC;
	HANDLE oldObj;
	xtrace("XDrawLines\n");

	pts->x=(LONG)points->x;
	pts->y=(LONG)points->y;

	for(n=1;n<nps;++n)
		if (mode==CoordModeOrigin)
		{
			(pts+n)->x=(LONG)(points+n)->x;
			(pts+n)->y=(LONG)(points+n)->y;
		}
		else
		{
			(pts+n)->x=(LONG)(points+n)->x+(pts+n-1)->x;
			(pts+n)->y=(LONG)(points+n)->y+(pts+n-1)->y;
		}

	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		hpen = NT_get_GC_pen(hDC,gc);
		oldObj = SelectObject(hDC,hpen);
		Polyline(hDC,pts,nps);
		SelectObject(hDC, oldObj);
		drawableRelDC(w,hDC);
	}
	return 0;
}


/*****************************************************************\

	Function: XDrawPoints
	Inputs:   display, window, gc, points list, number of points, mode.

	Comments: Untested.

\*****************************************************************/

int
XDrawPoints(display,w,gc,points,nps,mode)
Display *display;
Drawable w;
GC gc;
XPoint *points;
int nps,mode;
{
	HDC hDC;
	int n;
	xtrace("XDrawPoints\n");
	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		SetPixelV(hDC,points->x,points->y,CNUMTORGB(gc->values.foreground));
		for (n=1;n<nps;++n)
		{
			if (mode==CoordModeOrigin)
				SetPixelV(hDC,(points+n)->x,(points+n)->y,
						  CNUMTORGB(gc->values.foreground));
			else
				SetPixelV(hDC,(points+n-1)->x+(points+n)->x,
					  (points+n-1)->y+(points+n)->y,
					  CNUMTORGB(gc->values.foreground));
		}
		drawableRelDC(w,hDC);
	}
	return 0;
}
int
XDrawPoint(display,w,gc,x,y)
Display *display;
Drawable w;
GC gc;
int x,y;
{
	HDC hDC;
	xtrace("XDrawPoint\n");
	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		SetPixelV(hDC,x,y,CNUMTORGB(gc->values.foreground));
		drawableRelDC(w,hDC);
	}
	return 0;
}


/*****************************************************************\

	Function: XDrawRectangle
	Inputs:   display, window, gc, geometry

	Comments: Seems to work.

\*****************************************************************/

int
XDrawRectangle(display,w,gc,x,y,width,height)
Display *display;
Drawable w;
GC gc;
int x,y;
unsigned int width,height;
{
	HDC hDC;
	RECT rect;
	HBRUSH hbrush;
	HPEN hpen;
	HANDLE oldbrush, oldpen;
	xtrace("XDrawRectangle\n");
	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		hbrush = NT_get_GC_brush(hDC,gc);
		rect.left=(LONG)x;
		rect.right=(LONG)(x+width);
		rect.top=(LONG)y;
		rect.bottom=(LONG)(y+height);
		oldbrush = SelectObject(hDC,GetStockObject(NULL_BRUSH));
		hpen = NT_get_GC_pen(hDC,gc);
		oldpen = SelectObject(hDC,hpen);

		Rectangle(hDC,(int)rect.left,(int)rect.top,(int)rect.right,(int)rect.bottom);
		/*
		  FrameRect(hDC,&rect,hbrush);
		*/
		SelectObject(hDC, oldbrush);
		SelectObject(hDC, oldpen);
		drawableRelDC(w,hDC);
	}
	return 0;
}


/*****************************************************************\

	Function: XDrawSegments
	Inputs: display, window, gc, segment list, number of segments.

	Comments: Untested.

\*****************************************************************/

int
XDrawSegments(display,w,gc,segs,nsegs)
Display *display;
Drawable w;
GC gc;
XSegment *segs;
int nsegs;
{
	HDC hDC;
	HPEN hpen;
	int n;
	HANDLE oldObj;
	xtrace("XDrawSegments\n");
	if (VALID_WINDOW(w))
	{
		hDC = drawableGetDC(w);
		hpen = NT_get_GC_pen(hDC,gc);
		oldObj = SelectObject(hDC,hpen);
		SetBkMode(hDC,TRANSPARENT);
		for (n=0;n<nsegs;n++)
		{
			MoveToEx(hDC,(segs+n)->x1,(segs+n)->y1,NULL);
			LineTo(hDC,(segs+n)->x2,(segs+n)->y2);
		}
		SelectObject(hDC, oldObj);
		drawableRelDC(w,hDC);
	}
	return 0;
}

Pixmap
XCreatePixmap(display,drawable,width,height,depth)
Display *display;
Drawable drawable;
unsigned int width, height;
unsigned int depth;
{
	RECT rct;
	NT_window *w = (NT_window *)NT_new_window();
	HDC parenthDC = drawableGetDC(drawable);
	w->hDC = CreateCompatibleDC(parenthDC);
	w->hBitmap = CreateCompatibleBitmap(parenthDC,width,height);
	SelectObject(w->hDC, w->hBitmap);

	rct.left=(LONG) 0;
	rct.right=(LONG) width;
	rct.top=(LONG) 0;
	rct.bottom=(LONG) height;
	FillRect(w->hDC, &rct, GetStockObject(WHITE_BRUSH));

	drawableRelDC(drawable,parenthDC);

	w->w = NONMAPPED_HANDLE;
	w->x=0;
	w->y=0;
	w->wdth = width;
	w->hght = height;
	w->min = depth;
	return (Pixmap)w;
}

const char revBytes[]={
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0, 0x10, 0x90, 0x50,
	0xd0, 0x30, 0xb0, 0x70, 0xf0, 0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8,
	0x68, 0xe8, 0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8, 0x04,
	0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4, 0x14, 0x94, 0x54, 0xd4,
	0x34, 0xb4, 0x74, 0xf4, 0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c,
	0xec, 0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc, 0x02, 0x82,
	0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2, 0x12, 0x92, 0x52, 0xd2, 0x32,
	0xb2, 0x72, 0xf2, 0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa, 0x06, 0x86, 0x46,
	0xc6, 0x26, 0xa6, 0x66, 0xe6, 0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6,
	0x76, 0xf6, 0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee, 0x1e,
	0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe, 0x01, 0x81, 0x41, 0xc1,
	0x21, 0xa1, 0x61, 0xe1, 0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71,
	0xf1, 0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9, 0x19, 0x99,
	0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9, 0x05, 0x85, 0x45, 0xc5, 0x25,
	0xa5, 0x65, 0xe5, 0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed, 0x1d, 0x9d, 0x5d,
	0xdd, 0x3d, 0xbd, 0x7d, 0xfd, 0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3,
	0x63, 0xe3, 0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3, 0x0b,
	0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb, 0x1b, 0x9b, 0x5b, 0xdb,
	0x3b, 0xbb, 0x7b, 0xfb, 0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67,
	0xe7, 0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7, 0x0f, 0x8f,
	0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef, 0x1f, 0x9f, 0x5f, 0xdf, 0x3f,
	0xbf, 0x7f, 0xff
};


Pixmap
XCreateBitmapFromData(Display *display,
    Drawable drawable, const char *data,
    unsigned int width, unsigned int height)
{
	NT_window *w = (NT_window *)NT_new_window();
	HDC parenthDC = drawableGetDC(drawable);
	w->hDC = CreateCompatibleDC(parenthDC);

	{
		int i,j;
		char *newdata;
		int bytes = (width+7)>>3;
		int newbytes = (bytes&1)?bytes+1:bytes;
		newdata = allocateMemory(newbytes*height);
		for (i=0;i<height;i++)
		{
			for (j=0;j<bytes;j++)
			{
				newdata[(i*newbytes)+j]=revBytes[(unsigned char)data[(i*bytes)+j]];		
			}
		}
		w->hBitmap = CreateBitmap(width,height,1,1,newdata);
		freeMemory(newdata);
	}
	
	SelectObject(w->hDC, w->hBitmap);	
	drawableRelDC(drawable,parenthDC);
	w->x=0;
	w->y=0;
	w->wdth = width;
	w->hght = height;
	w->min = 1;
	return (Pixmap)w;
}

int
XFreePixmap(display, pixmap)
	 Display *display;
	 Pixmap pixmap;
{
	NT_window *w = (NT_window *)pixmap;
	NT_delete_window(w);
	return 0;
}

int
XCopyArea(display, src, dest, gc, src_x, src_y, width, height, dest_x, dest_y)
Display *display;
Drawable src, dest;
GC gc;
int src_x, src_y;
unsigned int width, height;
int dest_x, dest_y;
{
	HDC hsrc, hdst;
	hsrc = drawableGetDC(src);
	if (VALID_WINDOW(dest))
	{
		hdst = drawableGetDC(dest);
		(void)BitBlt(hdst,dest_x,dest_y,width,height,hsrc,src_x,src_y,SRCCOPY);
		drawableRelDC(src,hsrc);
	}
	drawableRelDC(dest,hdst);
	return 0;
}

XImage *
NT_XCreateImage()
{
	return NULL;
}

int NT_XDestroyImage(ximage)
XImage *ximage;
{
	/* freeMemory(ximage->data); */
  freeMemory(ximage);
  return 1;
}

unsigned long
NT_XGetPixel(ximage,x,y)
XImage *ximage;
int x,y;
{
	return 0;
}
int
NT_XPutPixel(ximage,x,y,pixel)
XImage *ximage;
int x,y;
unsigned long pixel;
{
	return 0;
}
XImage *
NT_XSubImage(ximage,x,y,w,h)
XImage *ximage;
int x,y;
unsigned int w,h;
{
	return NULL;
}
int
NT_XAddPixel(ximage,value)
XImage *ximage;
unsigned long value;
{
	return 0;
}

XImage *
XGetImage(display,drawable,x,y,width,height,plane_mask,format)
Display *display;
Drawable drawable;
int x,y;
unsigned int width, height;
unsigned long plane_mask;
int format;
{
	return NULL;
}

XImage *
XCreateImage(display,visual,depth,format,offset,data,width,height, bitmap_pad, bytes_per_line)
Display *display;
Visual *visual;
unsigned int depth;
int format;
int offset;
char *data;
unsigned int width, height;
int bitmap_pad, bytes_per_line;
{
    XImage *img = (XImage *) allocateMemory(sizeof(XImage));

    if (img) {
		img->depth = 24; /* depth; */
		img->format = format;
		img->xoffset = offset;
		img->data = data;
		img->width = width;
		img->height = height;
		img->bitmap_pad = 32;
		img->bytes_per_line=width*((24)>>3);
		img->bits_per_pixel = 24; /* depth; */
		img->bitmap_bit_order = LSBFirst;
		img->byte_order = MSBFirst;
		img->blue_mask = 0x0ff00000;
		img->green_mask=0x00ff0000;
		img->red_mask= 0x0000ff00;
		
		img->f.create_image = NT_XCreateImage;
		img->f.destroy_image = NT_XDestroyImage;
		img->f.get_pixel = NT_XGetPixel;
		img->f.put_pixel = NT_XPutPixel;
		img->f.sub_image = NT_XSubImage;
		img->f.add_pixel = NT_XAddPixel;
		
    }
	
	return img;
}
void
DrawBitmap(HDC hdc, HBITMAP hBitmap, int xStart, int yStart)
{
	BITMAP bm;
	HDC hdcMem;
	DWORD dwSize;
	POINT ptSize, ptOrg;
	hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem, hBitmap);
	SetMapMode(hdcMem,GetMapMode(hdc));
	GetObject(hBitmap, sizeof(BITMAP), (LPVOID)&bm);
	ptSize.x = bm.bmWidth;
	ptSize.y = bm.bmHeight;
	DPtoLP(hdc, &ptSize,1);
	ptOrg.x=0;
	ptOrg.y=0;
	DPtoLP(hdcMem,&ptOrg,1);
	BitBlt(hdc,xStart,yStart,ptSize.x,ptSize.y,hdcMem,ptOrg.x,ptOrg.y,SRCCOPY);
	DeleteDC(hdcMem);
}
/*
static unsigned char wBrickBits[]={
	0xff,0x0c,0x0c,0x0c, 0xff,0xc0,0xc0,0xc0,
	0xff,0x0c,0xff,0xff, 0xff,0xff,0xc0,0xc0,
	0xff,0x0c,0xff,0xff, 0xff,0xff,0xc0,0xc0,
	0xff,0x0c,0x0c,0x0c, 0xff,0xc0,0xc0,0xc0
};
*/
int
XPutImage(display,w,gc,image,sx,sy,dx,dy,width,height)
Display *display;
Drawable w;
XImage *image;
GC gc;
int sx,sy,dx,dy;
unsigned int width,height;
{
	BITMAPINFO bmInfo;
	NT_window *pix = (NT_window *)w;
	int res;
	if (VALID_WINDOW(w))
	{
		HDC hDC = drawableGetDC(w);
		bmInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmInfo.bmiHeader.biWidth = width;
		bmInfo.bmiHeader.biHeight = height;
		bmInfo.bmiHeader.biPlanes = 1;
		bmInfo.bmiHeader.biBitCount = 24; /*image->depth; */
		bmInfo.bmiHeader.biCompression = BI_RGB;
		bmInfo.bmiHeader.biSizeImage = 0;
		bmInfo.bmiHeader.biXPelsPerMeter = 3600;
		bmInfo.bmiHeader.biYPelsPerMeter = 3600;
		bmInfo.bmiHeader.biClrUsed = 0;
		bmInfo.bmiHeader.biClrImportant = 0;
		res = SetDIBitsToDevice(hDC,0,0,width,height,0,0,0,height,image->data,&bmInfo,DIB_RGB_COLORS);
		/*	BitBlt(CreateDC("DISPLAY",NULL,NULL,NULL),10,0,width,height,hDC,0,0,SRCCOPY); */
		if (res==0)
			printf("SetDIBitsfailed %d\n",res,GetLastError());
		drawableRelDC(w,hDC);
	}
	return 0;
}

int
XSetWindowBackground(display, w, bg)
	 Display *display;
	 Window w;
	 unsigned long bg;
{
	NT_window *window = (NT_window *)w;
	xtrace("XSetWindowBackground\n");
	DeleteObject(window->bg);
	window->bg=CreateSolidBrush(CNUMTORGB(bg));
	return 0;
}

int
XSetWindowBackgroundPixmap(display, w, background_tile)
Display *display;
Window w;
Pixmap background_tile;
{
	NT_window *window = (NT_window *)w;
	NT_window *wpix = (NT_window *)background_tile;
	BITMAPINFO *bmInfo;
	BITMAP bm;
	int res;
	
	xtrace("XSetWindowBackgroundPixmap\n");
	if (background_tile==ParentRelative)
	{
		if (!window->parentRelative)
		{
			HBITMAP hb = NT_getWallpaper();
			if (hb!=NULL) {
				DeleteObject(window->bg);
				window->bg = CreatePatternBrush(hb);
				window->parentRelative=1;
				NT_configureNotify(window,window->x,window->y);
			}
		}
	}
	else
	{
		GetObject(wpix->hBitmap, sizeof(BITMAP), &bm);
	
		bmInfo = allocateMemory(sizeof(BITMAPINFO) + ( (bm.bmBitsPixel>>3)* bm.bmWidth*bm.bmHeight));
		bmInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmInfo->bmiHeader.biWidth = bm.bmWidth;
		bmInfo->bmiHeader.biHeight = bm.bmHeight;
		bmInfo->bmiHeader.biPlanes = 1;
		bmInfo->bmiHeader.biBitCount = bm.bmBitsPixel;
		bmInfo->bmiHeader.biCompression = BI_RGB;
		bmInfo->bmiHeader.biSizeImage =  0;
		bmInfo->bmiHeader.biClrImportant = 0; 
		bmInfo->bmiHeader.biClrUsed = 0;
	
		res =GetDIBits(wpix->hDC,wpix->hBitmap,0,bm.bmHeight,bmInfo->bmiColors,bmInfo,DIB_RGB_COLORS);
		if (res==0)
			printf("getDIBits failed %d\n",res,GetLastError());
	
		DeleteObject(window->bg);
		window->bg = CreateDIBPatternBrushPt(bmInfo, DIB_RGB_COLORS);
		freeMemory(bmInfo);
	}
}


/*****************************************************************\

	Function: XSetFillStyle
	Inputs:   display, gc, fill style.

	Comments: ZZzzz...

\*****************************************************************/

int
XSetFillStyle(display,gc,fs)
Display *display;
GC gc;
int fs;
{
	xtrace("XSetFillStyle\n");
	gc->values.fill_style=fs;
	gc->dirty |= GCFillStyle;
	return 0;
}


int
XSetDashes(Display *display,
    GC gc, int dash_offset,
    const char * dash_list,
    int n)
{
	xtrace("XSetDashes\n");
	return 0;
}


int
XChangeWindowAttributes(display,w,vmask,attr)
Display *display;
Window w;
unsigned long vmask;
XSetWindowAttributes *attr;
{
	xtrace("XChangeWindowAttributes\n");
	return 0;
}


/*****************************************************************\

	Function: XLowerWindow
	Inputs:   display, window to be lowered.

	Comments: Make sure if the window has a frame that that gets lowered
		  too.

\*****************************************************************/

int
XLowerWindow(display,w)
Display *display;
Window w;
{
	NT_window *ntw=(NT_window *)w;
	xtrace("XLowerWindow\n");
	SetWindowPos((HWND)ntw->w,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	return 0;
}


/*****************************************************************\

	Function: XMapRaised
	Inputs:   display, window.

	Comments: Frames get raised first.

\*****************************************************************/

int
XMapRaised(display,w)
Display *display;
Window w;
{
	NT_window *ntw=(NT_window *)w;
	xtrace("XMapRaised\n");
	XMapWindow(display,w);
	SetWindowPos(ntw->w,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	return 0;
}


/*****************************************************************\

	Function: XMapSubwindows
	Inputs:   display, window.

	Comments: Unfortunately, the optimisations normally made by
		  the X server are not made here.

\*****************************************************************/

int
XMapSubwindows(display,w)
Display *display;
Window w;
{
	NT_window *ntw=(NT_window *)w;
	struct NT_child *tmp;

	xtrace("XMapSubWindows\n");
	tmp=ntw->child;
	while (tmp!=NULL)
	{
		XMapWindow(display,(Window)tmp->w);
		tmp=tmp->next;
        }
	return 0;
}


/*****************************************************************\

	Function: XQueryTree
	Inputs:   display, window.
	Returned: root window, parent window, list of children, status.

	Comments: Not fully implemented.  The child field is wrong.

\*****************************************************************/

StatusDef
XQueryTree(display,w,root,parent,ch,n)
Display *display;
Window w;
Window* root;
Window* parent;
Window** ch;
unsigned int *n;
{
	NT_window *ntw=(NT_window *)w;
	StatusDef status=1;

	xtrace("XQueryTree\n");
	*parent=(Window)ntw->parent;
	if (ntw->parent==NULL)
		status=0;
	*root=display->screens[0].root;
	*ch=NULL;
	*n=0;
	return (status);
}


/*****************************************************************\

	Function: XRaiseWindow
	Inputs:   display, window.

	Comments: Recursive raising of window and its children.

\*****************************************************************/

int
XRaiseWindow(display,w)
Display *display;
Window w;
{
	NT_window *ntw=(NT_window *)w;
	struct NT_child *tmp;
	xtrace("XRaiseWindows\n");

	BringWindowToTop(ntw->w);
	tmp=ntw->child;
	while (tmp!=NULL)
	{
		XRaiseWindow(display,(Window)tmp->w);
		tmp=tmp->next;
	}
	return 0;
}


/*****************************************************************\

	Function: XRootWindow
	Inputs:   display, screen number
	Returned: root window ID.

	Comments: Info taken from display structure.

\*****************************************************************/

Window
XRootWindow(display,scr)
Display *display;
int scr;
{
	xtrace("XRootWindow\n");
	return(display->screens[0].root);
}


/*****************************************************************\

	Function: XRootWindowOfScreen
	Inputs:   screen pointer
	Returned: root window ID.

	Comments: ZZzzz...

\*****************************************************************/

Window
XRootWindowOfScreen(scr)
Screen *scr;
{
	xtrace("XRootWindowOfScreen\n");
	return(scr->root);
}


/*****************************************************************\

	Function: XTranslateCoordinates
	Inputs:   display, source window, destination window, source x, y.
	Returned: destination x, y, child window if any.

	Comments: Seems to work properly.

\*****************************************************************/

BoolDef
XTranslateCoordinates(display,sw,dw,sx,sy,dx,dy,ch)
Display *display;
Window sw,dw;
int sx,sy,*dx,*dy;
Window *ch;
{
	xtrace("XTranslateCoordinates\n");
	return (True);
}


/*****************************************************************\

	Function: XUnmapWindow
	Inputs:   display, window.

	Comments: Removes a window and its frame, if it has one, from the
		  screen.

\*****************************************************************/

int
XUnmapWindow(display,w)
Display *display;
Window w;
{
	NT_window *ntw=(NT_window *)w;
	xtrace("XUnmapWindow\n");
	ShowWindow(ntw->w,SW_HIDE);
	return 0;
}


/*****************************************************************\

	Function: XCopyGC
	Inputs:   display, source gc, values mask, destination gc.

	Comments: Copies masked elements from source to destination.

\*****************************************************************/

int
XCopyGC(display,sgc,vmask,dgc)
Display *display;
GC sgc,dgc;
unsigned long vmask;
{
	xtrace("XCopyGC\n");
	if (vmask&GCFunction)
		dgc->values.function=sgc->values.function;
	if (vmask&GCPlaneMask)
		dgc->values.plane_mask=sgc->values.plane_mask;
	if (vmask&GCForeground)
		dgc->values.foreground=sgc->values.foreground;
	if (vmask&GCBackground)
		dgc->values.background=sgc->values.background;
	if (vmask&GCLineWidth)
		dgc->values.line_width=sgc->values.line_width;
	if (vmask&GCLineStyle)
		dgc->values.line_style=sgc->values.line_style;
	if (vmask&GCCapStyle)
		dgc->values.cap_style=sgc->values.cap_style;
	if (vmask&GCJoinStyle)
		dgc->values.join_style=sgc->values.join_style;
	if (vmask&GCFillStyle)
		dgc->values.fill_style=sgc->values.fill_style;
	if (vmask&GCFillRule)
		dgc->values.fill_rule=sgc->values.fill_rule;
	if (vmask&GCTile)
		dgc->values.tile=sgc->values.tile;
	if (vmask&GCStipple)
		dgc->values.stipple=sgc->values.stipple;
	if (vmask&GCTileStipXOrigin)
		dgc->values.ts_x_origin=sgc->values.ts_x_origin;
	if (vmask&GCTileStipYOrigin)
		dgc->values.ts_y_origin=sgc->values.ts_y_origin;
	if (vmask&GCFont)
		dgc->values.font=sgc->values.font;
	if (vmask&GCSubwindowMode)
		dgc->values.subwindow_mode=sgc->values.subwindow_mode;
	if (vmask&GCGraphicsExposures)
		dgc->values.graphics_exposures=sgc->values.graphics_exposures;
	if (vmask&GCClipXOrigin)
		dgc->values.clip_x_origin=sgc->values.clip_x_origin;
	if (vmask&GCClipYOrigin)
		dgc->values.clip_y_origin=sgc->values.clip_y_origin;
	if (vmask&GCClipMask)
		dgc->values.clip_mask=sgc->values.clip_mask;
	if (vmask&GCDashList)
		dgc->values.dashes=sgc->values.dashes;
	if (vmask&GCArcMode)
		dgc->values.arc_mode=sgc->values.arc_mode;

	dgc->dirty = 0xFFFF;
	return 0;
}


int
XSetClipMask(display,gc,cmask)
Display *display;
GC gc;
Pixmap cmask;
{
	xtrace("XSetClipMask\n");
	return 0;
}


int
XSetClipRectangles(display,gc,clx,cly,rs,n,order)
Display *display;
GC gc;
int clx,cly;
XRectangle *rs;
int n,order;
{
	xtrace("XSetClipRectangles\n");
	return 0;
}


/*****************************************************************\

	Function: XSetFunction
	Inputs:   display, gc, graphics operation.

	Comments: ZZzzz...

\*****************************************************************/

int
XSetFunction(display,gc,fn)
Display *display;
GC gc;
int fn;
{
	xtrace("XSetFunction\n");
	gc->values.function=fn;
	gc->dirty |= GCFunction;
	return 0;
}


/*****************************************************************\

	Function: XSetLineAttributes
	Inputs:   display, gc, line width, line style, cap style, join style.

	Comments: These all have windows equivalents.

\*****************************************************************/

int
XSetLineAttributes(display,gc,lw,ls,cs,js)
Display *display;
GC gc;
unsigned int lw;
int ls,cs,js;
{
	xtrace("XSetLineAttributes\n");
	gc->values.line_width=lw;
	gc->values.line_style=ls;
	gc->values.cap_style=cs;
	gc->values.join_style=js;
	gc->dirty |= GCLineWidth | GCLineStyle | GCCapStyle | GCJoinStyle;
	return 0;
}


/*****************************************************************\

	Function: XSetPlaneMask
	Inputs:   display, gc, plane mask.

	Comments: What's a plane mask?

\*****************************************************************/

int
XSetPlaneMask(display,gc,pmask)
Display *display;
GC gc;
unsigned long pmask;
{
	xtrace("XSetPlaneMask\n");
	gc->values.plane_mask=pmask;
	return 0;
}


int
XSetTile(display,gc,tile)
Display *display;
GC gc;
Pixmap tile;
{
	xtrace("XSetTile\n");
	return 0;
}


StatusDef
XAllocColorCells(display,cmap,cont,pmasks,np,pixels,nc)
Display *display;
Colormap cmap;
BoolDef cont;
unsigned long *pmasks;
unsigned int np;
unsigned long *pixels;
unsigned int nc;
{
	unsigned int i;
	xtrace("XAllocColorCells\n");
	for (i = 0;i<nc;i++)
		pixels[i] = i;
	if(np==1)
	{
		*pmasks = nc;
	}
	return 1;
}


/*****************************************************************\

	Function: XAllocColorPlanes
	Inputs:   display, colour map, contiguous flag, pixel value list,
		  number of colours, number of reds, greens, blues.
	Returned: red mask, green mask, blue mask, status.

	Comments: Works for True Colour only.

\*****************************************************************/

StatusDef
XAllocColorPlanes(display,cmap,cont,pixels,nc,nr,ng,nb,rmask,gmask,bmask)
Display *display;
Colormap cmap;
BoolDef cont;
unsigned long *pixels;
int nc;
int nr,ng,nb;
unsigned long *rmask,*gmask,*bmask;
{
	xtrace("XAllocColorPlanes\n");
	*rmask=255;
	*gmask=255<<8;
	*bmask=255<<16;
	return (1);
}


StatusDef
XAllocNamedColor(Display *display,
    Colormap cmap, const char *cname,
    XColor *cell, XColor *rgb)
{
	xtrace("XAllocNamedColor\n");
	return 0;
}


Colormap
XCreateColormap(display,w,visual,alloc)
Display *display;
Window w;
Visual *visual;
int alloc;
{
	xtrace("XCreateColormap\n");
	return 0;
}


StatusDef
XGetStandardColormap(display,w,cmapinf,prop)
Display *display;
Window w;
XStandardColormap *cmapinf;
Atom prop;
{
	xtrace("XGetStandardColormap\n");
	return 0;
}

StatusDef
XAllocColor(display,cmap,xc)
Display *display;
Colormap cmap;
XColor *xc;
{
	xtrace("XAllocColor\n");
	xc->pixel = RGB((BYTE) (xc->red>>8)&0xff, (BYTE) (xc->green>>8)&0xff, (BYTE) (xc->blue>>8)&0xff);
	return 1;
}

int
XQueryColor(display,cmap,cell)
Display *display;
Colormap cmap;
XColor *cell;
{
	xtrace("XQueryColor\n");
	cell->red = (cell->pixel>>8)&0x0ff00;
	cell->green = (cell->pixel)&0x0ff00;
	cell->blue = (cell->pixel<<8)&0x0ff00;
  return 1;
}


int
XQueryColors(display,cmap,cells,nc)
Display *display;
Colormap cmap;
XColor *cells;
int nc;
{
	int i;

	xtrace("XQueryColors\n");
	for (i=0;i<nc;i++)
	{
		cells[i].red=(cells[i].pixel>>8)&0xff00;
		cells[i].green=(cells[i].pixel)&0xff00;
		cells[i].blue=(cells[i].pixel<<8)&0xff00;
	}
	return 0;
}


int
XStoreColor(display,cmap,cell)
Display *display;
Colormap cmap;
XColor *cell;
{
	xtrace("XStoreColor\n");
	return 0;
}



int
XStoreColors(display,cmap,cells,nc)
Display *display;
Colormap cmap;
XColor *cells;
int nc;
{
	int i;
	xtrace("XStoreColors\n");
	return 0;
}

char **
XGetFontPath(display,nps)
Display *display;
int *nps;
{
	xtrace("XGetFontPath\n");
	return (NULL);
}


BoolDef
XGetFontProperty(fstruct,atom,val)
XFontStruct *fstruct;
Atom atom;
unsigned long *val;
{
	xtrace("XGetFontProperty\n");
	return (0);
}

static BYTE
getCharSetFromName(const char *name,char **lang)
{
    static struct {
	char *label;
	BYTE id;
    } charset[] = {
	"-gb2312", GB2312_CHARSET,
	"-big5", CHINESEBIG5_CHARSET,
	"-jis", SHIFTJIS_CHARSET,
	NULL, DEFAULT_CHARSET
    };
    int i;
    char *p;
    if (!name)
    {
	if (lang) *lang=NULL;
	return DEFAULT_CHARSET;
    }
    else if (lang) *lang=(char *)name+strlen((char *)name);
    for (i=0; charset[i].label; i++)
	if (NULL!=(p=strstr(name, charset[i].label)))
	{
	    if (lang) *lang=p;
	    break;
	}
    return charset[i].id;
}

/* attempts to translate the font name into
** something win32 understands.
** 
*/
Font
NT_loadfont(name)
char *name;
{
    LOGFONT lf;
    HFONT hfont;
    char *p,*q,*lang;
    int size = 0;
    char buff[33];
	
    memset(&lf,0,sizeof(lf));
    lf.lfHeight = -13;
	
    lf.lfWeight = FW_NORMAL;
    lf.lfFaceName[0]='\0';
	
    lf.lfCharSet = getCharSetFromName(name,&lang);
    
    if (name && strstr(name,"-bold"))
    {
	lf.lfWeight = FW_BOLD;
    }
    for(p=name, q=buff; p&&p<=lang; p++)
    {
	if (((!*p || *p=='-') && q!=buff) || (q-buff>31))
	{
	    *q++='\0';
	    if (lf.lfFaceName[0]=='\0')
	    {
		if (isalpha(buff[0]) || IsDBCSLeadByte(buff[0]))
		    strcpy(lf.lfFaceName, buff);
		else if (isdigit(buff[0]) && ((q=strchr(buff,'x')) || (q=strchr(buff,'X'))))
		{
		    strcpy(lf.lfFaceName, "Courier New");
		    size = -atoi(q+1);
		}
		else if (size == 0 && isdigit(buff[0]))
		    size = -atoi(buff);
	    }
	    else if (size == 0 && isdigit(buff[0]))
	    {				
		size = -atoi(buff);
	    }
	    q = buff;
	}
	else
	    *q++ = *p;
	if (!*p) break;
    }
	
    if (size > 99)
	lf.lfHeight = - (size/10);
    else if (size)
	lf.lfHeight = - size;
    
    if (!strcmp(lf.lfFaceName,"lucidatypewriter"))
	strcpy(lf.lfFaceName,"Lucida Console");
    else if (lf.lfFaceName[0]=='\0'&&lf.lfCharSet==DEFAULT_CHARSET)
	strcpy(lf.lfFaceName,"Courier New");
    hfont = CreateFontIndirect(&lf);
    return (Font)hfont;
}

XFontStruct *
XLoadQueryFont(Display *display, const char *name)
{
    XFontStruct *fs;
    TEXTMETRIC tm;
    HDC hdc;
    HWND root;
    HFONT old;
    int i;
	
    xtrace("XLoadQueryFont\n");
    fs = allocateMemory(sizeof(XFontStruct));
    fs->fid = NT_loadfont(name);

    root=GetDesktopWindow();
    hdc=GetDC(root);
    old = SelectObject(hdc, (HFONT)fs->fid);
    GetTextMetrics(hdc, &tm);
    fs->min_bounds.width = tm.tmAveCharWidth;
    fs->max_bounds.width = tm.tmMaxCharWidth;
    fs->n_properties = 0;
    fs->properties = NULL;
    fs->min_char_or_byte2 =tm.tmFirstChar;
    fs->max_char_or_byte2 =tm.tmLastChar;
    fs->per_char = (XCharStruct*)allocateMemory(sizeof(XCharStruct)*(tm.tmLastChar+1));
    ZeroMemory(fs->per_char, sizeof(XCharStruct)*(tm.tmLastChar+1));

    if (!(tm.tmPitchAndFamily & TMPF_FIXED_PITCH)) {
	for (i=tm.tmFirstChar; i<=tm.tmLastChar; i++) {
	    fs->per_char[i].width = tm.tmAveCharWidth;
	    fs->per_char[i].rbearing = tm.tmAveCharWidth;
 	}
    } else if (tm.tmPitchAndFamily & TMPF_TRUETYPE) {
	for (i=tm.tmFirstChar; i<=tm.tmLastChar; i++)
	{
	    ABC abc;
	    if (!GetCharABCWidths(hdc,i,i,&abc))
	    {
		abc.abcA = 0;
		abc.abcB = tm.tmAveCharWidth;
		abc.abcC = 0;
	    }
	    fs->per_char[i].lbearing = abc.abcA;
	    fs->per_char[i].width = abc.abcB;
	    fs->per_char[i].rbearing = abc.abcB+abc.abcC;		    
	}
    } else {
	for (i=tm.tmFirstChar; i<=tm.tmLastChar; i++)
	{
	    INT fw;
	    if (!GetCharWidth(hdc,i,i,&fw))
	    {
		fw = tm.tmAveCharWidth;
	    }
	    fs->per_char[i].lbearing = 0;
	    fs->per_char[i].width = fw;
	    fs->per_char[i].rbearing = fw;
	}
    }
    if (getCharSetFromName(name,NULL) == DEFAULT_CHARSET)
    {
	int last = tm.tmLastChar;

	switch(tm.tmCharSet) {
	    case CHINESEBIG5_CHARSET:
	    case GB2312_CHARSET:
	    case SHIFTJIS_CHARSET:
		if (tm.tmLastChar > 127) last = 127;
		break;
	}

	if (last >= tm.tmFirstChar)
	{
	    fs->max_bounds = fs->per_char[last];
	    fs->min_bounds = fs->per_char[last];
	    for (i=tm.tmFirstChar; i<last; i++)
	    {
		if (fs->per_char[i].lbearing > fs->max_bounds.lbearing)
		    fs->max_bounds.lbearing = fs->per_char[i].lbearing;
		else if(fs->per_char[i].lbearing < fs->min_bounds.lbearing)
		    fs->min_bounds.lbearing = fs->per_char[i].lbearing;

		if (fs->per_char[i].width > fs->max_bounds.width)
		    fs->max_bounds.width = fs->per_char[i].width;
		else if(fs->per_char[i].width < fs->min_bounds.width)
		    fs->min_bounds.width = fs->per_char[i].width;

		if (fs->per_char[i].rbearing > fs->max_bounds.rbearing)
		    fs->max_bounds.rbearing = fs->per_char[i].rbearing;
		else if(fs->per_char[i].rbearing < fs->min_bounds.rbearing)
		    fs->min_bounds.rbearing = fs->per_char[i].rbearing;
	    }
	}
    }
    fs->ascent = tm.tmAscent;
    fs->descent= tm.tmDescent;
    SelectObject(hdc,old);
    ReleaseDC(root,hdc);
    return(fs);
}

XFontStruct *
XQueryFont(display, font_id)
Display *display;
XID     font_id;
{
	static XFontStruct fs;

	xtrace("XQueryFont\n");
	return(&fs);
}

KeySym
XKeycodeToKeysym(display, keycode, index)
Display *display;
unsigned int keycode;
int     index;
{
	xtrace("XKeycodeToKeysym\n");
	if (keycode == 254) return XK_Alt_L;
	else if (keycode == 255) return XK_Num_Lock;
	return(NoSymbol);
}
KeyCode
XKeysymToKeycode(display, keysym)
Display *display;
KeySym keysym;
{
	xtrace("XKeycodeToKeysym\n");
	if (keysym == XK_Alt_L) return 254;
	else if (keysym == XK_Num_Lock) return 255;
	return(0);
}
KeySym
XStringToKeysym(const char *str)
{
	xtrace("XStringToKeysym\n");
	return(NoSymbol);
}

XModifierKeymap *
XGetModifierMapping(display)
Display *display;
{
	XModifierKeymap *map = NULL;
	xtrace("XGetModifierMapping\n");
	map = (XModifierKeymap *)allocateMemory(sizeof(XModifierKeymap));
	map->max_keypermod = 1;
	map->modifiermap = (KeyCode *)allocateMemory(sizeof(KeyCode)*8);
	map->modifiermap[0]=0;
	map->modifiermap[1]=0;
	map->modifiermap[2]=0;
	map->modifiermap[3]=XKeysymToKeycode(display,XK_Alt_L);
	map->modifiermap[4]=0;
	map->modifiermap[5]=0;
	map->modifiermap[6]=0;
	map->modifiermap[7]=XKeysymToKeycode(display,XK_Num_Lock);
	return(map);
}
int
XFreeModifiermap(XModifierKeymap *modmap)
{
	xtrace("XFreeModifiermap\n");
	freeMemory(modmap->modifiermap);
	freeMemory(modmap);
	return 0;
}


int
XSetFont(display,gc,font)
Display *display;
GC gc;
Font font;
{
	xtrace("XSetFont\n");
	gc->values.font = font;
	return 0;
}


int
XSetFontPath(display,dirs,nd)
Display *display;
char **dirs;
int nd;
{
	xtrace("XSetFontPath\n");
	return 0;
}


/*****************************************************************\

	Function: XTextExtents
	Inputs:   font structure, string, string length.
	Returned: writing direction, max ascent, max descent, font overall
		  characteristics.

	Comments: The design of windows fonts is similar to X, as far as
		  ascent and descent are concerned.  However, they are
		  positioned differently on the screen (see XDrawText).

\*****************************************************************/

static HDC desktopHDC;
static int firstTE = TRUE;
int
XTextExtents(fstruct,str,nc,dir,ascent,descent,overall)
XFontStruct *fstruct;
const char *str;
int nc;
int *dir,*ascent,*descent;
XCharStruct *overall;
{
	TEXTMETRIC tmet;
	HDC hdc;
	SIZE tsize;
	HWND desktop;
	HFONT old;
	
	xtrace("XTextExtents\n");
	if (firstTE)
	{
		firstTE = FALSE;
		desktop=GetDesktopWindow();
		desktopHDC=GetDC(desktop);
	}
	hdc = desktopHDC;
	old = SelectObject(hdc, (HFONT)fstruct->fid);
	GetTextMetrics(hdc,&tmet);
	GetTextExtentPoint(hdc,str,nc,&tsize);
	*dir=FontLeftToRight;
	*ascent=tmet.tmAscent + 1;
	*descent=tmet.tmDescent;
	overall->ascent=(short)(tmet.tmAscent + 1);
	overall->descent=(short)tmet.tmDescent;
	overall->width=(short)tsize.cx;
	overall->lbearing=0;
	overall->rbearing=(short)tsize.cx;
	/* cjh_rel_dc(desktop,hdc);*/
	SelectObject(hdc,old);
	return 0;
}


int
XTextExtents16(fstruct,str,nc,dir,ascent,descent,overall)
XFontStruct *fstruct;
const XChar2b *str;
int nc;
int *dir,*ascent,*descent;
XCharStruct *overall;
{
	xtrace("XTextExtents16\n");
	return 0;
}


/*****************************************************************\


	Function: XTextWidth
	Inputs:   font structure, string, length of string.
	Returned: String width in pixels.

	Comments:

\*****************************************************************/

int
XTextWidth(fstruct,str,co)
XFontStruct *fstruct;
const char *str;
int co;
{
	HDC hdc;
	SIZE tsize;
	HWND root;
	HFONT old;
	xtrace("XTextWidth\n");

	if(firstTE)
	{
		firstTE = FALSE;
		root=GetDesktopWindow();
		hdc=GetDC(root);
	}
	old = SelectObject(hdc, (HFONT)fstruct->fid);
	GetTextExtentPoint(hdc,str,co,&tsize);
	SelectObject(hdc,old);
	/*cjh_rel_dc(root,hdc);*/
	return (tsize.cx);
}


int
XTextWidth16(fstruct,str,co)
XFontStruct *fstruct;
const XChar2b *str;
int co;
{
	xtrace("XTextWidth16\n");
	return 0;
}


int
XGetErrorDatabaseText(display,name,msg,defstr,buf,len)
Display *display;
const char *name,*msg;
const char *defstr;
char *buf;
int len;
{
	static char def_err[]="Errors not implemented";
	int n;

	xtrace("XGetErrorDatabaseText\n");
	while (n<len && def_err[n] != 0)
		*(buf+n)=def_err[n++];
	*(buf+n)=0;
	return 0;
}


int
XGetErrorText(display,code,buf,len)
Display *display;
int code;
char *buf;
int len;
{
	xtrace("XGetErrorText\n");
	return 0;
}


XErrorHandler
XSetErrorHandler(handler)
XErrorHandler handler;
{
	xtrace("XSetErrorHandler\n");
	return 0;
}


/*****************************************************************\


	Function: XDefaultScreen
	Inputs:   display
	Returned: default screen number

	Comments:

\*****************************************************************/

int
XDefaultScreen(display)
Display *display;
{
	xtrace("XDefaultScreen\n");
	return (display->default_screen);
}

Visual *
XDefaultVisual(display, screen)
Display *display;
int screen;
{
	xtrace("XDefaultVisual\n");
	return DefaultVisual(display, screen);
}
int
XDefaultDepth(display, screen)
Display *display;
int screen;
{
	xtrace("XDefaultDepth\n");
	return DefaultDepth(display, screen);
}
Colormap
XDefaultColormap(display, screen)
Display *display;
int screen;
{
	xtrace("XDefaultColormap\n");
	return DefaultColormap(display, screen);
}


/*****************************************************************\


	Function: XScreenOfDisplay
	Inputs:   display,screen number
	Returned: screen list.

	Comments:

\*****************************************************************/

Screen *
XScreenOfDisplay(display,scr)
Display *display;
int scr;
{
	xtrace("XScreenOfDisplay\n");
	return (display->screens);
}


Cursor
XCreateFontCursor(display,shape)
Display *display;
unsigned int shape;
{
	xtrace("XCreateFontCursor\n");
	return 0;
}


int
XRecolorCursor(display,cursor,fg,bg)
Display *display;
Cursor cursor;
XColor *fg,*bg;
{
	xtrace("XRecolorCursor\n");
	return 0;
}


/*****************************************************************\


	Function: XWarpPointer
	Inputs:   display, source window, destination window, source window
		  geometry, destination x, y.

	Comments: Not knowingly tested.

\*****************************************************************/

int
XWarpPointer(display,sw,dw,sx,sy,swidth,sheight,dx,dy)
Display *display;
Window sw,dw;
int sx,sy;
unsigned int swidth,sheight;
int dx,dy;
{
	NT_window *ntsw=(NT_window *)sw;
	NT_window *ntdw=(NT_window *)dw;

	POINT cpt,tmp;
	RECT srct;
	HDC hDC;

	xtrace("XWarpPointer\n");
	GetCursorPos(&cpt);
	if (ntsw==None)
	{
		if (ntdw==None)
			SetCursorPos(dx,dy);
		else
		{
			tmp.x=dx;
			tmp.y=dy;
			ClientToScreen(ntdw->w,&tmp);
			SetCursorPos(tmp.x,tmp.y);
		}
	}
	else
	{
		GetWindowRect(ntsw->w,&srct);
		tmp.x=sx;
		tmp.y=sy;
		ClientToScreen(ntsw->w,&tmp);
		if (swidth==0)
			swidth=srct.right-sx;
		if (sheight==0)
			sheight=srct.bottom-sy;
		hDC = cjh_get_dc(ntdw);
		if (cpt.x>=tmp.x && cpt.x<tmp.x+(int)swidth &&
		    cpt.y>=tmp.y && cpt.y<tmp.y+(int)sheight &&
		    PtVisible(hDC,cpt.x,cpt.y))
		{
			if (ntdw==None)
				SetCursorPos(cpt.x+dx,cpt.y+dy);
			else
			{
				tmp.x=dx;
				tmp.y=dy;
				ClientToScreen(ntdw->w,&tmp);
				SetCursorPos(tmp.x,tmp.y);
			}
		}
		cjh_rel_dc(ntdw,hDC);
	}
	return 0;
}


/*****************************************************************\


	Function: XBell
	Inputs:   display, percent loudness.

	Comments: Don't throw away your CD just yet.

\*****************************************************************/

int
XBell(display,pc)
Display *display;
int pc;
{
	xtrace("XBell\n");
	MessageBeep(MB_OK);
	return 0;
}


/*****************************************************************\


	Function: XGetInputFocus
	Inputs:   display, focus window, revert to window.

	Comments: We don't have the data in place for the revert to field
		  to work.

\*****************************************************************/

int
XGetInputFocus(display,focus,revto)
Display *display;
Window *focus;
int *revto;
{
	xtrace("XGetInputFocus\n");
	*focus=(Window)GetFocus();  /* Note: returns NULL if the focus window */
	revto=RevertToNone; /*       belongs to another thread.       */
	return 0;
}


/*****************************************************************\


	Function: XSetInputFocus
	Inputs:   display, focus window, revert to window, time.

	Comments: Set focus to requested client window.

\*****************************************************************/

int
XSetInputFocus(display,focus,revto,time)
Display *display;
Window focus;
int revto;
Time time;
{
	NT_window *ntw=(NT_window *)focus;
	xtrace("XSetInputFocus\n");
	SetFocus(ntw->w);
	return 0;
}


int
XLookupString(event,buf,nbytes,keysym,status)
XKeyEvent *event;
char *buf;
int nbytes;
KeySym *keysym;
XComposeStatus *status;
{
	xtrace("XLookupString\n");
	*buf=event->keycode;
	*keysym=event->keycode;
	return 1;
}


int
XRefreshKeyboardMapping(event)
XMappingEvent *event;
{
	xtrace("XRefreshKeyboardMapping\n");
	return 0;
}


int
XSetClassHint(display,w,chints)
Display *display;
Window w;
XClassHint *chints;
{
	xtrace("XSetClassHint\n");
	return 0;
}


/*****************************************************************\


	Function: XSetNormalHints
	Inputs:   display, window, size hints.

	Comments: Assuming the role of the window manager, we alter the
		  window geometry as requested.

\*****************************************************************/

int
XSetNormalHints(display,w,hints)
Display *display;
Window w;
XSizeHints *hints;
{
	NT_window *ntw=(NT_window *)w;
	UINT ff;

	xtrace("XSetNormalHints\n");

	if (!hints->flags&PPosition)
		ff=SWP_NOMOVE;
	else
		ff=0;
	
	if (!hints->flags&PSize)
		ff=ff|SWP_NOSIZE;

	if (hints->flags &USPosition)
	{
		ntw->x = hints->x;
		ntw->y = hints->y;
	}
	if (hints->flags & USSize)
	{
		ntw->wdth = hints->width;
		ntw->hght = hints->height;
	}
	
	if (hints->flags&PMinSize)
	{
		ntw->minx = hints->min_width;
		ntw->miny = hints->min_height;
	}
	
	if (VALID_WINDOW(ntw) && (hints->flags & USSize || hints->flags &USPosition))
	{
		SetWindowPos(ntw->w,HWND_TOPMOST,hints->x,hints->y,
					 hints->width,hints->height,ff|SWP_SHOWWINDOW);
	}
	return 0;
}


int
XSetWMHints(display,w,wmhints)
Display *display;
Window w;
XWMHints *wmhints;
{
	xtrace("XSetWMHints\n");
	return 0;
}


StatusDef
XSetWMProtocols(display,w,prots,co)
Display *display;
Window w;
Atom *prots;
int co;
{
	xtrace("XSetWMProtocols\n");
	return 0;
}


/*****************************************************************\


	Function: XStoreName
	Inputs:   display, window, window name.

	Comments: Only set names to the frame windows, otherwise captions
		  appear in the client areas.

\*****************************************************************/

int
XStoreName(display,w,wname)
Display *display;
Window w;
const char *wname;
{
	NT_window *ntw=(NT_window *)w;
	int status = 0;

	xtrace("XStoreName\n");
	if (ntw->top_flag)
	{
		ntw->title_text = strdup(wname);
		if (VALID_WINDOW(ntw))
			status = SetWindowText (ntw->w , wname);
	}
	return(status);
}

StatusDef
XFetchName(
    Display *display,
    Window w,
    char **window_name_return)
{
	NT_window *ntw=(NT_window *)w;
	int status = 1;
	xtrace("XFetchName\n");
	*window_name_return = NULL;
	if (ntw->title_text!=NULL)
	{
		*window_name_return = strdup(ntw->title_text);
		status =0;
	}
	return(status);
}



/*****************************************************************\


	Function: XDoesBackingStore
	Inputs:   screen

	Comments: No backing store at the moment.  Windows doesn't seem
		  to support it, unless we do it ourselves with compatible
		  bitmaps.

\*****************************************************************/

int
XDoesBackingStore(scr)
Screen *scr;
{
	xtrace("XDoesBackingStore\n");
	return(0);
}


XExtCodes *
XInitExtension(display,name)
Display *display;
const char *name;
{
	xtrace("XInitExtension\n");
	return 0;
}


/*****************************************************************\


	Function: XFree
	Inputs:   data to be freed.

	Comments: This might need changing sometime.  No crashes so far.

\*****************************************************************/

int
XFree(data)
void *data;
{
	xtrace("XFree\n");
	freeMemory(data);
	return 0;
}

/*****************************************************************\


	Function: XServerVendor
	Inputs:   display.
	Returned: string of vendor's name.

	Comments: Copied from the display structure.

\*****************************************************************/

char *
XServerVendor(display)
Display *display;
{
	xtrace("XServerVendor\n");
	return (display->vendor);
}


int
XSetIconName(display,w,iname)
Display *display;
Window w;
const char *iname;
{
	xtrace("XSetIconName\n");
	return 0;
}
int
XGetIconName(display,w,iname)
Display *display;
Window w;
char **iname;
{
	xtrace("XGetIconName\n");
	*iname = NULL;
	return 0;
}


int
XSetSelectionOwner(display, sel, owner, time)
Display* display;
Atom sel;
Window owner;
Time time;
{
    HWND hwnd = owner ? ((NT_window*)owner)->w : NULL;
	HWND cowner = GetClipboardOwner();
	OpenClipboard(hwnd);
	if (cowner==hwnd)
		catchNextDestroyClipboard();
	EmptyClipboard();
	CloseClipboard();
	return 0;
}

Window
XGetSelectionOwner(display,selection)
Display* display;
Atom selection;
{
	HWND hwnd = NULL;
	Window w;
	xtrace("XGetSelectionOwner\n");
	hwnd = GetClipboardOwner();
	w = (Window)NT_find_window_from_id(hwnd);
	if (!w) w=None;
	return w;
}

/*****************************************************************\

	Function: NT_set_rop
	Inputs:   window device context, X graphics context

	Comments: Sets the graphics drawing operation.

\*****************************************************************/

void
NT_set_rop(hdc,gc)
HDC hdc;
GC gc;
{
	switch (gc->values.function)
	{
		case GXclear:
			SetROP2(hdc,R2_BLACK);
			break;

		case GXand:
			SetROP2(hdc,R2_MASKPEN);
			break;

		case GXandReverse:
			SetROP2(hdc,R2_MASKPENNOT);
			break;

		case GXcopy:
			SetROP2(hdc,R2_COPYPEN);
			break;

		case GXnoop:
			SetROP2(hdc,R2_NOP);
			break;

		case GXxor:
			SetROP2(hdc,R2_XORPEN);/*XORPEN);*/
			break;

		case GXor:
			SetROP2(hdc,R2_MERGEPEN);
			break;

		case GXnor:
			SetROP2(hdc,R2_NOTMERGEPEN);
			break;

		case GXequiv:
			SetROP2(hdc,R2_NOTXORPEN);
			break;

		case GXinvert:
			SetROP2(hdc,R2_NOT);
			break;

		case GXorReverse:
			SetROP2(hdc,R2_MERGEPENNOT);
			break;

		case GXcopyInverted:
			SetROP2(hdc,R2_NOTCOPYPEN);
			break;

		case GXorInverted:
			SetROP2(hdc,R2_MERGENOTPEN);
			break;

		case GXnand:
			SetROP2(hdc,R2_NOTMASKPEN);
			break;

		case GXset:
			SetROP2(hdc,R2_WHITE);
			break;
	}
}

/*****************************************************************\

	Function: NT_check_update_GC
	Inputs:   gc - Graphics Context

	Comments: Check what has changed in the GC and modify the
		  pen and brush accordingly.

\*****************************************************************/
static int
NT_check_update_GC(gc)
GC gc;
{
	DWORD style=PS_GEOMETRIC;
	LOGBRUSH lbr;
	int	 width;
	NTGC	*lntgc;

	if ( gc->ext_data == NULL )
	{
		gc->ext_data = (XExtData *)allocateMemory(sizeof(XExtData));
		lntgc = (NTGC *)allocateMemory(sizeof(NTGC));
		gc->ext_data->private_data = (char *)lntgc;
		lntgc->pen = INVALID_HANDLE;
		lntgc->brush = INVALID_HANDLE;
		lntgc->bgbrush = INVALID_HANDLE;
		gc->dirty=~0;
	}

	if ((gc->dirty & GCForeground) ||
		 (gc->dirty & GCLineStyle)  ||
	     (gc->dirty & GCCapStyle)   ||
	     (gc->dirty & GCJoinStyle)  ||
	     (gc->dirty & GCLineWidth) )
	{
		lbr.lbStyle=BS_SOLID;
		lbr.lbColor=CNUMTORGB(gc->values.foreground);
		lbr.lbHatch=0;

		if (gc->values.line_style==LineDoubleDash)
			style|=PS_DASHDOT;
		else if (gc->values.line_style==LineOnOffDash)
			style|=PS_DASH;
		else
			style|=PS_SOLID;

		if (gc->values.cap_style==CapProjecting)
			style|=PS_ENDCAP_SQUARE;
		else if (gc->values.cap_style==CapRound)
			style|=PS_ENDCAP_ROUND;
		else
			style|=PS_ENDCAP_FLAT;

		if (gc->values.join_style==JoinRound)
			style|=PS_JOIN_ROUND;
		else if (gc->values.join_style==JoinMiter)
			style|=PS_JOIN_MITER;
		else
			style|=PS_JOIN_BEVEL;
		width=gc->values.line_width;
		if (width==0)
			width=1;

		lntgc = (NTGC *)gc->ext_data->private_data;
		if ( lntgc->pen != INVALID_HANDLE )
			DeleteObject(lntgc->pen);

		lntgc->pen = ExtCreatePen(style,width,&lbr,0,NULL);
	}
	if (gc->values.fill_style != FillSolid)
	{
		if ((gc->dirty & GCStipple))
		{
			NT_window *pixmap = (NT_window *)gc->values.stipple;
			lntgc->brush = CreatePatternBrush(pixmap->hBitmap);
		}
	}
	else if ((gc->dirty & GCForeground) )
	{
		lntgc = (NTGC *)gc->ext_data->private_data;
		if ( lntgc->brush != INVALID_HANDLE )
			DeleteObject(lntgc->brush);
		lntgc->brush = CreateSolidBrush(CNUMTORGB(gc->values.foreground));

	}

	if ((gc->dirty & GCBackground) )
	{
		lntgc = (NTGC *)gc->ext_data->private_data;
		if ( lntgc->bgbrush != INVALID_HANDLE )
			DeleteObject(lntgc->bgbrush);
		lntgc->bgbrush = CreateSolidBrush(CNUMTORGB(gc->values.background));

	}

	gc->dirty = 0;

	return(1);
}


/*****************************************************************\

	Function: NT_get_GC_pen
	Inputs:   device context, graphics context

	Comments: Sets the device context and pen according to the
		  graphics context.

\*****************************************************************/
static HPEN
NT_get_GC_pen(hdc,gc)
HDC hdc;
GC gc;
{
	NTGC *lntgc;

	NT_check_update_GC(gc);
	NT_set_rop(hdc,gc);

	lntgc = (NTGC *)gc->ext_data->private_data;

	return(lntgc->pen);
}


/*****************************************************************\

	Function: NT_get_GC_brush
	Inputs:   device context, graphics context
	Returns:  handle for brush.

	Comments: Same as above for painting operations.

\*****************************************************************/
static HBRUSH
NT_get_GC_brush(hdc,gc)
HDC hdc;
GC gc;
{
	NTGC *lntgc;

        NT_check_update_GC(gc);

	if (gc->values.fill_rule==EvenOddRule)
		SetPolyFillMode(hdc,ALTERNATE);
	else
		SetPolyFillMode(hdc,WINDING);
	
	if (gc->values.fill_style == FillSolid || 
	    gc->values.fill_style == FillOpaqueStippled)
	{
		SetTextColor(hdc, CNUMTORGB(gc->values.foreground));
		SetBkColor(hdc, CNUMTORGB(gc->values.background));
	}

	NT_set_rop(hdc,gc);

	lntgc = (NTGC *)gc->ext_data->private_data;

	return(lntgc->brush);
}

static HBRUSH
NT_get_GC_bgbrush(hdc,gc)
HDC hdc;
GC gc;
{
	NTGC *lntgc;
        NT_check_update_GC(gc);
	lntgc = (NTGC *)gc->ext_data->private_data;
	return(lntgc->bgbrush);
}



/*****************************************************************\

	Function: NT_deg64_to_rad
	Inputs:   angle (in 64ths of a degree)

	Comments: Converts int angle to double in radians.

\*****************************************************************/

double
NT_deg64_to_rad(a)
int a;
{
	return ((double)a/64.0*0.017453);
}


/******************************************************************/
/*                                                                */
/*               Atoms and properties code.                       */
/*                                                                */
/******************************************************************/

static char **nt_atoms;
static int num_nt_atoms = 0;
static int max_num_nt_atoms = 0;
#define ATOMS_BLOCK_SIZE 40

/******************************************************************\

         Function:  XInternAtom
         Inputs:    Display, property name, creation flag.

         Comments:  Could be made much more efficient.

\******************************************************************/

Atom
XInternAtom(display, property_name, only_if_exists)
Display *display;
const char *property_name;
BoolDef only_if_exists;
{
	int i;
	char **new_nt_atoms;

	xtrace("XInternAtom\n");
	if (strcmp(property_name,"VT_SELECTION")==0)
	{
		return XA_LAST_PREDEFINED + 667;
	}
	
	for (i=0;i< num_nt_atoms ;i++)
		if (strcmp(nt_atoms[i],property_name) == 0)
			return XA_LAST_PREDEFINED + i;
	
	if (only_if_exists)
		return None;

	if (num_nt_atoms >= max_num_nt_atoms)
	{
		new_nt_atoms = (char **)realloc(nt_atoms,(max_num_nt_atoms + ATOMS_BLOCK_SIZE)*sizeof(char *));
		if (!new_nt_atoms)
			return None;
		nt_atoms = new_nt_atoms;
		max_num_nt_atoms+= ATOMS_BLOCK_SIZE;
		nt_atoms[num_nt_atoms] = allocateMemory(strlen(property_name)+1);
		if (!nt_atoms[num_nt_atoms])
			return None;
		strcpy(nt_atoms[num_nt_atoms],property_name);
		return (XA_LAST_PREDEFINED +  num_nt_atoms++);
	}
}

/******************************************************************\

         Function:  XGetAtomName
         Inputs:    Display,Atom

         Comments:  None.

\******************************************************************/
char *
XGetAtomName(display,atom)
Display *display;
Atom atom;
{
	char *ret_string;
	xtrace("XGetAtomName\n");
	if (atom > num_nt_atoms + XA_LAST_PREDEFINED)
		return NULL;

	if (! (ret_string = allocateMemory(strlen(nt_atoms[atom - XA_LAST_PREDEFINED])+1)))
		return FALSE;

	strcpy(ret_string,nt_atoms[atom]);

	return ret_string;
}

/******************************************************************\

         Function:  XChangeProperty
         Inputs:    Display,Window,Property,type,format,mode,data,
                    nelements.

         Comments:  None.

\******************************************************************/
int
XChangeProperty(
	Display *display,
	Window window,
	Atom property,
	Atom type,
	int format,
	int mode,
	const unsigned char *data,
	int nelements)
{
	NT_window *ntw=(NT_window *)window;
	struct NT_prop_list *wanderer, *new;
	xtrace("XChangeProperty\n");

	if (property == XA_CUT_BUFFER0 && type==XA_STRING)
	{
		HGLOBAL handle=NULL;
		char *buffer=NULL;
		int i,j,cr=0;
		for (i=0; i<nelements; i++)
			if (data[i]=='\n' && (i==0 || data[i-1]!='\r')) cr++;
		
		handle = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, nelements+cr+1);
		if (!handle) return;
		OpenClipboard(ntw->w);
		buffer = GlobalLock(handle);
		for (i=j=0; i<nelements; buffer[j++]=data[i++])
			if (data[i]=='\n' && (i==0 || data[i-1]!='\r')) buffer[j++]='\r';
		buffer[j++]='\0';
		GlobalUnlock(handle);
		SetClipboardData(CF_TEXT, handle);
		handle = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(LCID));
		if (handle) {
			*(LCID*)GlobalLock(handle) = GetThreadLocale();
			GlobalUnlock(handle);
			SetClipboardData(CF_LOCALE, handle);
		}
		CloseClipboard();
		return;
	}
}

int
XDeleteProperty(
      Display *display,
      Window w,
      Atom property)
{
	return 0;
}

static char *
NT_getClipboard() 
{
    char *ret = NULL;
    char *s,*t;
    int size, format=0;
    HGLOBAL handle;
    LPVOID data;
    OpenClipboard(NULL);
    for (format = EnumClipboardFormats(format); format; format = EnumClipboardFormats(format)) {
	if (format != CF_TEXT && format != CF_UNICODETEXT) continue;
	handle = GetClipboardData(format);
	if (handle==NULL) break;
	data = GlobalLock(handle);
	if (format == CF_UNICODETEXT) {
	    int conv = CP_THREAD_ACP;
	    size = WideCharToMultiByte(conv,0,data,-1,NULL,0,NULL,NULL);
	    if (size == 0) /* NT doesn't support CF_THREAD_ACP */
		size = WideCharToMultiByte( (conv = CP_UTF8),0,data,-1,NULL,0,NULL,NULL);
	    if (size==0) continue; /* let windows convert it */
	    ret = allocateMemory(size);
	    WideCharToMultiByte(conv, 0, data, -1, ret, size, NULL, NULL);
	} else {
	    size = strlen(data)+1;
	    ret = allocateMemory(size);
	    strcpy(ret,data);
	}    
	GlobalUnlock(handle);
	break;
    }
    CloseClipboard();
    for (s=t=ret;s && (*t=*s++);) if (*t!='\r') t++;
    return ret;
}

int
XGetWindowProperty(display,window,property,long_offset,long_length,delete,
                   req_type,actual_type_return,actual_format_return,
                   nitems_return,bytes_after_return,prop_return)
Display *display;
Window window;
Atom property;
long long_offset;
long long_length;
BoolDef delete;
Atom req_type;
Atom *actual_type_return;
int *actual_format_return;
unsigned long *nitems_return;
unsigned long *bytes_after_return;
unsigned char **prop_return;
{
    xtrace("XGetWindowProperty\n");
    if (property == XA_CUT_BUFFER0 && prop_return && (*prop_return = NT_getClipboard())) {
	*nitems_return=strlen(*prop_return);
	*actual_type_return = XA_STRING;
	*bytes_after_return = 0;
    }    
    else {
	*prop_return = NULL;
	*actual_type_return = None;
	*actual_format_return = 0;
	*bytes_after_return = 0;
    }
    return 0;
}



char **
XListExtensions(display,ret_num)
Display *display;
int *ret_num;
{
	*ret_num = 0;
	xtrace("XListExtensions\n");
	return NULL;
}

XFreeExtensionList(list)
char **list;
{
	xtrace("XFreeExtensionList\n");
	return 0;
}

XChangeGC(
	Display* display,
	GC gc,
	unsigned long mask,
	XGCValues* gc_values)
{
	xtrace("XChangeGC\n");
	if (mask&GCArcMode)
		gc->values.arc_mode=gc_values->arc_mode;
	if (mask&GCBackground)
		gc->values.background=gc_values->background;
	if (mask&GCCapStyle)
		gc->values.cap_style=gc_values->cap_style;
	if (mask&GCClipMask)
		gc->values.clip_mask=gc_values->clip_mask;
	if (mask&GCClipXOrigin)
		gc->values.clip_x_origin=gc_values->clip_x_origin;
	if (mask&GCClipYOrigin)
		gc->values.clip_y_origin=gc_values->clip_y_origin;
	if (mask&GCDashList)
		gc->values.dashes=gc_values->dashes;
	if (mask&GCDashOffset)
		gc->values.dash_offset=gc_values->dash_offset;
	if (mask&GCFillRule)
		gc->values.fill_rule=gc_values->fill_rule;
	if (mask&GCFillStyle)
		gc->values.fill_style=gc_values->fill_style;
	if (mask&GCFont)
		gc->values.font=gc_values->font;
	if (mask&GCForeground)
		gc->values.foreground=gc_values->foreground;
	if (mask&GCFunction)
		gc->values.function=gc_values->function;
	if (mask&GCGraphicsExposures)
		gc->values.graphics_exposures=gc_values->graphics_exposures;
	if (mask&GCJoinStyle)
		gc->values.join_style=gc_values->join_style;
	if (mask&GCLineStyle)
		gc->values.line_style=gc_values->line_style;
	if (mask&GCLineWidth)
		gc->values.line_width=gc_values->line_width;
	if (mask&GCPlaneMask)
		gc->values.plane_mask=gc_values->plane_mask;
	if (mask&GCStipple)
		gc->values.stipple=gc_values->stipple;
	if (mask&GCSubwindowMode)
		gc->values.subwindow_mode=gc_values->subwindow_mode;
	if (mask&GCTile)
		gc->values.tile=gc_values->tile;
	if (mask&GCTileStipXOrigin)
		gc->values.ts_x_origin=gc_values->ts_x_origin;
	if (mask&GCTileStipYOrigin)
		gc->values.ts_y_origin=gc_values->ts_y_origin;
	gc->dirty |= mask;
	return 0;
}

int
XConnectionNumber(Display* display)
{
    int fd;
    xtrace("XConnectionNumber\n");
    fd = open ("/dev/windows", O_NONBLOCK, 0);
    return fd;
}

XFreeFont(Display* display,XFontStruct* font_struct)
{
	xtrace("XFreeFont\n");
	return 0;
}

char *
_Xsetlocale(int category, const char *name)
{
	xtrace("Xsetlocale\n");
	return (char *)name;
}

char *
XSetLocaleModifiers(const char* modifier_list)
{
	xtrace("XSetLocaleModifiers\n");
	return NULL;
}
XIM
XOpenIM(
	Display* dpy,
	struct _XrmHashBucketRec* rdb,
	char* res_name,
	char* res_class)
{
	xtrace("XOpenIM\n");
	return 0;
}
char *
XGetIMValues(XIM im , ...)
{
	xtrace("XGetIMValues\n");
	return NULL;
}
XIC
XCreateIC(XIM im , ...)
{
	xtrace("XCreateIC\n");
	return 0;
}
StatusDef
XCloseIM(XIM im)
{
	xtrace("XCloseIM\n");
	return 0;
}

char *
XrmQuarkToString(void *quark)
{
	xtrace("XrmQuarkToString\n");
	return NULL;
}

int
XmbLookupString(
	XIC ic,
	XKeyPressedEvent* event,
	char* buffer_return,
	int bytes_buffer,
	KeySym* keysym_return,
	StatusDef* status_return)
{
	xtrace("XmbLookupString\n");
	return 0;
}
int
XmbTextPropertyToTextList(
	Display *display,
	XTextProperty *text_prop,
	char ***list_return,
	int *count_return)
{
	xtrace("XmbTextPropertyToTextList\n");
	/* in the future copy this, for 2.7.1 rxvt leaks this, so don't yet */
	*list_return = (char **)allocateMemory(sizeof(char *));
	*list_return[0] = text_prop->value;
	*count_return = 1;
	return 0;	
}

void
XFreeStringList(char **list)
{
	freeMemory(list);
}

int
XmbTextListToTextProperty(
	 Display *display,
	 char **list,
	 int count,
	 XICCEncodingStyle style,
	 XTextProperty *text_prop_return)
{
	int ret = 0;
	if (count!=1) ret = XNoMemory;
	text_prop_return->value = strdup(list[0]);
	switch (style)
	{
		case XStringStyle: text_prop_return->encoding = XA_STRING; break;
		/* case XCompoundTextStyle: text_prop_return->encoding = XA_COMPOUND; break; */
		default: ret = XLocaleNotSupported;
	}
	text_prop_return->format = 8;
	text_prop_return->nitems = count;
	return ret;
}
void XmbDrawString(
	Display *display,
	Drawable d,
	XFontSet font_set,
	GC gc,
	int x, int y,
	char *string,
	int num_bytes) 
{
	xtrace("XmbDrawString\n");
}


void
XSetICFocus(XIC ic)
{
	xtrace("XSetICFocus\n");
}
void
XUnsetICFocus(XIC ic)
{
	xtrace("XUnsetICFocus\n");
}



/* lifted from emacs */
/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */

static int
read_integer (string, NextString)
     register char *string;
     char **NextString;
{
	register int Result = 0;
	int Sign = 1;
  
	if (*string == '+')
		string++;
	else if (*string == '-')
    {
		string++;
		Sign = -1;
    }
	for (; (*string >= '0') && (*string <= '9'); string++)
    {
		Result = (Result * 10) + (*string - '0');
    }
	*NextString = string;
	if (Sign >= 0)
		return (Result);
	else
		return (-Result);
}

/* lifted from emacs */
int
XParseGeometry(
	const char* string,
	int* x,
	int* y,
	unsigned int* width,
	unsigned int* height)
{
	int mask = NoValue;
	register char *strind;
	unsigned int tempWidth, tempHeight;
	int tempX, tempY;
	char *nextCharacter;
  
	if ((string == NULL) || (*string == '\0')) return (mask);
	if (*string == '=')
		string++;  /* ignore possible '=' at beg of geometry spec */
  
	strind = (char *)string;
	if (*strind != '+' && *strind != '-' && *strind != 'x') 
    {
		tempWidth = read_integer (strind, &nextCharacter);
		if (strind == nextCharacter) 
			return (0);
		strind = nextCharacter;
		mask |= WidthValue;
    }
  
	if (*strind == 'x' || *strind == 'X') 
    {	
		strind++;
		tempHeight = read_integer (strind, &nextCharacter);
		if (strind == nextCharacter)
			return (0);
		strind = nextCharacter;
		mask |= HeightValue;
    }
  
	if ((*strind == '+') || (*strind == '-')) 
    {
		if (*strind == '-') 
		{
			strind++;
			tempX = -read_integer (strind, &nextCharacter);
			if (strind == nextCharacter)
				return (0);
			strind = nextCharacter;
			mask |= XNegative;

		}
		else
		{	
			strind++;
			tempX = read_integer (strind, &nextCharacter);
			if (strind == nextCharacter)
				return (0);
			strind = nextCharacter;
		}
		mask |= XValue;
		if ((*strind == '+') || (*strind == '-')) 
		{
			if (*strind == '-') 
			{
				strind++;
				tempY = -read_integer (strind, &nextCharacter);
				if (strind == nextCharacter)
					return (0);
				strind = nextCharacter;
				mask |= YNegative;

			}
			else
			{
				strind++;
				tempY = read_integer (strind, &nextCharacter);
				if (strind == nextCharacter)
					return (0);
				strind = nextCharacter;
			}
			mask |= YValue;
		}
    }
  
	/* If strind isn't at the end of the string the it's an invalid
	   geometry specification. */
  
	if (*strind != '\0') return (0);
  
	if (mask & XValue)
		*x = tempX;
	if (mask & YValue)
		*y = tempY;
	if (mask & WidthValue)
		*width = tempWidth;
	if (mask & HeightValue)
		*height = tempHeight;
	return (mask);
}

XResizeWindow(
	Display* display,
	Window w,
	unsigned int width,
	unsigned int height)
{
	RECT r; 
	NT_window *ntw=(NT_window *)w;
	xtrace("XResizeWindow\n");
	r.left = ntw->x;
	r.top = ntw->y;
	r.right = r.left + width;
	r.bottom = r.top + height;
	if (ntw->top_flag)
	  AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);
	if (VALID_WINDOW(ntw))
	  MoveWindow(ntw->w, r.left, r.top, 
		     r.right-r.left, r.bottom-r.top,TRUE);
	return 0;
}

void
XSetWMNormalHints(Display* display,Window w,XSizeHints* hints)
{
	xtrace("XSetWMNormalHints\n");
	XSetNormalHints(display,w,hints);
}

void
XSetWMProperties(
	Display* display,
	Window w,
	XTextProperty* window_name,
	XTextProperty* icon_name,
	char** argv,
	int argc,
	XSizeHints* normal_hints,
	XWMHints* wm_hints,
	XClassHint* class_hints)
{
	xtrace("XSetWMProperties\n");
	XSetNormalHints(display,w,normal_hints);
}
XDefineCursor(Display* display,Window w,Cursor cursor)
{
	xtrace("XDefineCursor\n");
	return 0;
}

XMoveResizeWindow(
	Display* display,
	Window w,
	int x,
	int y,
	unsigned int width,
	unsigned int height)
{
	NT_window *ntw=(NT_window *)w;
	xtrace("XMoveResizeWindow\n");
	ntw->x = x;
	ntw->y = y;
	ntw->wdth = width;
	ntw->hght = height;
	if (VALID_WINDOW(ntw)) {
		NT_moveWindow(ntw,TRUE);
	}
	
}

XMoveWindow(
	Display* display,
	Window w,
	int x,
	int y)
{
	NT_window *ntw=(NT_window *)w;
	xtrace("XMoveWindow\n");
	ntw->x = x;
	ntw->y = y;
	if (VALID_WINDOW(ntw)) {
		NT_moveWindow(ntw,TRUE);
	}
	return 0;
}

/* 
 * xcolors.c --
 *
 *	This file contains the routines used to map from X color
 *	names to RGB and pixel values.
 *
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * SCCS: @(#) xcolors.c 1.3 96/12/17 13:07:02
 */

/*
 * Define an array that defines the mapping from color names to RGB values.
 * Note that this array must be kept sorted alphabetically so that the
 * binary search used in XParseColor will succeed.
 */
typedef struct {
    char *name;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} XColorEntry;

static XColorEntry xColors[] = {
    "alice blue", 240, 248, 255,
    "AliceBlue", 240, 248, 255,
    "antique white", 250, 235, 215,
    "AntiqueWhite", 250, 235, 215,
    "AntiqueWhite1", 255, 239, 219,
    "AntiqueWhite2", 238, 223, 204,
    "AntiqueWhite3", 205, 192, 176,
    "AntiqueWhite4", 139, 131, 120,
    "aquamarine", 127, 255, 212,
    "aquamarine1", 127, 255, 212,
    "aquamarine2", 118, 238, 198,
    "aquamarine3", 102, 205, 170,
    "aquamarine4", 69, 139, 116,
    "azure", 240, 255, 255,
    "azure1", 240, 255, 255,
    "azure2", 224, 238, 238,
    "azure3", 193, 205, 205,
    "azure4", 131, 139, 139,
    "beige", 245, 245, 220,
    "bisque", 255, 228, 196,
    "bisque1", 255, 228, 196,
    "bisque2", 238, 213, 183,
    "bisque3", 205, 183, 158,
    "bisque4", 139, 125, 107,
    "black", 0, 0, 0,
    "blanched almond", 255, 235, 205,
    "BlanchedAlmond", 255, 235, 205,
    "blue", 0, 0, 255,
    "blue violet", 138, 43, 226,
    "blue1", 0, 0, 255,
    "blue2", 0, 0, 238,
    "blue3", 0, 0, 205,
    "blue4", 0, 0, 139,
    "BlueViolet", 138, 43, 226,
    "brown", 165, 42, 42,
    "brown1", 255, 64, 64,
    "brown2", 238, 59, 59,
    "brown3", 205, 51, 51,
    "brown4", 139, 35, 35,
    "burlywood", 222, 184, 135,
    "burlywood1", 255, 211, 155,
    "burlywood2", 238, 197, 145,
    "burlywood3", 205, 170, 125,
    "burlywood4", 139, 115, 85,
    "cadet blue", 95, 158, 160,
    "CadetBlue", 95, 158, 160,
    "CadetBlue1", 152, 245, 255,
    "CadetBlue2", 142, 229, 238,
    "CadetBlue3", 122, 197, 205,
    "CadetBlue4", 83, 134, 139,
    "chartreuse", 127, 255, 0,
    "chartreuse1", 127, 255, 0,
    "chartreuse2", 118, 238, 0,
    "chartreuse3", 102, 205, 0,
    "chartreuse4", 69, 139, 0,
    "chocolate", 210, 105, 30,
    "chocolate1", 255, 127, 36,
    "chocolate2", 238, 118, 33,
    "chocolate3", 205, 102, 29,
    "chocolate4", 139, 69, 19,
    "coral", 255, 127, 80,
    "coral1", 255, 114, 86,
    "coral2", 238, 106, 80,
    "coral3", 205, 91, 69,
    "coral4", 139, 62, 47,
    "cornflower blue", 100, 149, 237,
    "CornflowerBlue", 100, 149, 237,
    "cornsilk", 255, 248, 220,
    "cornsilk1", 255, 248, 220,
    "cornsilk2", 238, 232, 205,
    "cornsilk3", 205, 200, 177,
    "cornsilk4", 139, 136, 120,
    "cyan", 0, 255, 255,
    "cyan1", 0, 255, 255,
    "cyan2", 0, 238, 238,
    "cyan3", 0, 205, 205,
    "cyan4", 0, 139, 139,
    "dark goldenrod", 184, 134, 11,
    "dark green", 0, 100, 0,
    "dark khaki", 189, 183, 107,
    "dark olive green", 85, 107, 47,
    "dark orange", 255, 140, 0,
    "dark orchid", 153, 50, 204,
    "dark salmon", 233, 150, 122,
    "dark sea green", 143, 188, 143,
    "dark slate blue", 72, 61, 139,
    "dark slate gray", 47, 79, 79,
    "dark slate grey", 47, 79, 79,
    "dark turquoise", 0, 206, 209,
    "dark violet", 148, 0, 211,
    "DarkGoldenrod", 184, 134, 11,
    "DarkGoldenrod1", 255, 185, 15,
    "DarkGoldenrod2", 238, 173, 14,
    "DarkGoldenrod3", 205, 149, 12,
    "DarkGoldenrod4", 139, 101, 8,
    "DarkGreen", 0, 100, 0,
    "DarkKhaki", 189, 183, 107,
    "DarkOliveGreen", 85, 107, 47,
    "DarkOliveGreen1", 202, 255, 112,
    "DarkOliveGreen2", 188, 238, 104,
    "DarkOliveGreen3", 162, 205, 90,
    "DarkOliveGreen4", 110, 139, 61,
    "DarkOrange", 255, 140, 0,
    "DarkOrange1", 255, 127, 0,
    "DarkOrange2", 238, 118, 0,
    "DarkOrange3", 205, 102, 0,
    "DarkOrange4", 139, 69, 0,
    "DarkOrchid", 153, 50, 204,
    "DarkOrchid1", 191, 62, 255,
    "DarkOrchid2", 178, 58, 238,
    "DarkOrchid3", 154, 50, 205,
    "DarkOrchid4", 104, 34, 139,
    "DarkSalmon", 233, 150, 122,
    "DarkSeaGreen", 143, 188, 143,
    "DarkSeaGreen1", 193, 255, 193,
    "DarkSeaGreen2", 180, 238, 180,
    "DarkSeaGreen3", 155, 205, 155,
    "DarkSeaGreen4", 105, 139, 105,
    "DarkSlateBlue", 72, 61, 139,
    "DarkSlateGray", 47, 79, 79,
    "DarkSlateGray1", 151, 255, 255,
    "DarkSlateGray2", 141, 238, 238,
    "DarkSlateGray3", 121, 205, 205,
    "DarkSlateGray4", 82, 139, 139,
    "DarkSlateGrey", 47, 79, 79,
    "DarkTurquoise", 0, 206, 209,
    "DarkViolet", 148, 0, 211,
    "deep pink", 255, 20, 147,
    "deep sky blue", 0, 191, 255,
    "DeepPink", 255, 20, 147,
    "DeepPink1", 255, 20, 147,
    "DeepPink2", 238, 18, 137,
    "DeepPink3", 205, 16, 118,
    "DeepPink4", 139, 10, 80,
    "DeepSkyBlue", 0, 191, 255,
    "DeepSkyBlue1", 0, 191, 255,
    "DeepSkyBlue2", 0, 178, 238,
    "DeepSkyBlue3", 0, 154, 205,
    "DeepSkyBlue4", 0, 104, 139,
    "dim gray", 105, 105, 105,
    "dim grey", 105, 105, 105,
    "DimGray", 105, 105, 105,
    "DimGrey", 105, 105, 105,
    "dodger blue", 30, 144, 255,
    "DodgerBlue", 30, 144, 255,
    "DodgerBlue1", 30, 144, 255,
    "DodgerBlue2", 28, 134, 238,
    "DodgerBlue3", 24, 116, 205,
    "DodgerBlue4", 16, 78, 139,
    "firebrick", 178, 34, 34,
    "firebrick1", 255, 48, 48,
    "firebrick2", 238, 44, 44,
    "firebrick3", 205, 38, 38,
    "firebrick4", 139, 26, 26,
    "floral white", 255, 250, 240,
    "FloralWhite", 255, 250, 240,
    "forest green", 34, 139, 34,
    "ForestGreen", 34, 139, 34,
    "gainsboro", 220, 220, 220,
    "ghost white", 248, 248, 255,
    "GhostWhite", 248, 248, 255,
    "gold", 255, 215, 0,
    "gold1", 255, 215, 0,
    "gold2", 238, 201, 0,
    "gold3", 205, 173, 0,
    "gold4", 139, 117, 0,
    "goldenrod", 218, 165, 32,
    "goldenrod1", 255, 193, 37,
    "goldenrod2", 238, 180, 34,
    "goldenrod3", 205, 155, 29,
    "goldenrod4", 139, 105, 20,
    "gray", 190, 190, 190,
    "gray0", 0, 0, 0,
    "gray1", 3, 3, 3,
    "gray10", 26, 26, 26,
    "gray100", 255, 255, 255,
    "gray11", 28, 28, 28,
    "gray12", 31, 31, 31,
    "gray13", 33, 33, 33,
    "gray14", 36, 36, 36,
    "gray15", 38, 38, 38,
    "gray16", 41, 41, 41,
    "gray17", 43, 43, 43,
    "gray18", 46, 46, 46,
    "gray19", 48, 48, 48,
    "gray2", 5, 5, 5,
    "gray20", 51, 51, 51,
    "gray21", 54, 54, 54,
    "gray22", 56, 56, 56,
    "gray23", 59, 59, 59,
    "gray24", 61, 61, 61,
    "gray25", 64, 64, 64,
    "gray26", 66, 66, 66,
    "gray27", 69, 69, 69,
    "gray28", 71, 71, 71,
    "gray29", 74, 74, 74,
    "gray3", 8, 8, 8,
    "gray30", 77, 77, 77,
    "gray31", 79, 79, 79,
    "gray32", 82, 82, 82,
    "gray33", 84, 84, 84,
    "gray34", 87, 87, 87,
    "gray35", 89, 89, 89,
    "gray36", 92, 92, 92,
    "gray37", 94, 94, 94,
    "gray38", 97, 97, 97,
    "gray39", 99, 99, 99,
    "gray4", 10, 10, 10,
    "gray40", 102, 102, 102,
    "gray41", 105, 105, 105,
    "gray42", 107, 107, 107,
    "gray43", 110, 110, 110,
    "gray44", 112, 112, 112,
    "gray45", 115, 115, 115,
    "gray46", 117, 117, 117,
    "gray47", 120, 120, 120,
    "gray48", 122, 122, 122,
    "gray49", 125, 125, 125,
    "gray5", 13, 13, 13,
    "gray50", 127, 127, 127,
    "gray51", 130, 130, 130,
    "gray52", 133, 133, 133,
    "gray53", 135, 135, 135,
    "gray54", 138, 138, 138,
    "gray55", 140, 140, 140,
    "gray56", 143, 143, 143,
    "gray57", 145, 145, 145,
    "gray58", 148, 148, 148,
    "gray59", 150, 150, 150,
    "gray6", 15, 15, 15,
    "gray60", 153, 153, 153,
    "gray61", 156, 156, 156,
    "gray62", 158, 158, 158,
    "gray63", 161, 161, 161,
    "gray64", 163, 163, 163,
    "gray65", 166, 166, 166,
    "gray66", 168, 168, 168,
    "gray67", 171, 171, 171,
    "gray68", 173, 173, 173,
    "gray69", 176, 176, 176,
    "gray7", 18, 18, 18,
    "gray70", 179, 179, 179,
    "gray71", 181, 181, 181,
    "gray72", 184, 184, 184,
    "gray73", 186, 186, 186,
    "gray74", 189, 189, 189,
    "gray75", 191, 191, 191,
    "gray76", 194, 194, 194,
    "gray77", 196, 196, 196,
    "gray78", 199, 199, 199,
    "gray79", 201, 201, 201,
    "gray8", 20, 20, 20,
    "gray80", 204, 204, 204,
    "gray81", 207, 207, 207,
    "gray82", 209, 209, 209,
    "gray83", 212, 212, 212,
    "gray84", 214, 214, 214,
    "gray85", 217, 217, 217,
    "gray86", 219, 219, 219,
    "gray87", 222, 222, 222,
    "gray88", 224, 224, 224,
    "gray89", 227, 227, 227,
    "gray9", 23, 23, 23,
    "gray90", 229, 229, 229,
    "gray91", 232, 232, 232,
    "gray92", 235, 235, 235,
    "gray93", 237, 237, 237,
    "gray94", 240, 240, 240,
    "gray95", 242, 242, 242,
    "gray96", 245, 245, 245,
    "gray97", 247, 247, 247,
    "gray98", 250, 250, 250,
    "gray99", 252, 252, 252,
    "green", 0, 255, 0,
    "green yellow", 173, 255, 47,
    "green1", 0, 255, 0,
    "green2", 0, 238, 0,
    "green3", 0, 205, 0,
    "green4", 0, 139, 0,
    "GreenYellow", 173, 255, 47,
    "grey", 190, 190, 190,
    "grey0", 0, 0, 0,
    "grey1", 3, 3, 3,
    "grey10", 26, 26, 26,
    "grey100", 255, 255, 255,
    "grey11", 28, 28, 28,
    "grey12", 31, 31, 31,
    "grey13", 33, 33, 33,
    "grey14", 36, 36, 36,
    "grey15", 38, 38, 38,
    "grey16", 41, 41, 41,
    "grey17", 43, 43, 43,
    "grey18", 46, 46, 46,
    "grey19", 48, 48, 48,
    "grey2", 5, 5, 5,
    "grey20", 51, 51, 51,
    "grey21", 54, 54, 54,
    "grey22", 56, 56, 56,
    "grey23", 59, 59, 59,
    "grey24", 61, 61, 61,
    "grey25", 64, 64, 64,
    "grey26", 66, 66, 66,
    "grey27", 69, 69, 69,
    "grey28", 71, 71, 71,
    "grey29", 74, 74, 74,
    "grey3", 8, 8, 8,
    "grey30", 77, 77, 77,
    "grey31", 79, 79, 79,
    "grey32", 82, 82, 82,
    "grey33", 84, 84, 84,
    "grey34", 87, 87, 87,
    "grey35", 89, 89, 89,
    "grey36", 92, 92, 92,
    "grey37", 94, 94, 94,
    "grey38", 97, 97, 97,
    "grey39", 99, 99, 99,
    "grey4", 10, 10, 10,
    "grey40", 102, 102, 102,
    "grey41", 105, 105, 105,
    "grey42", 107, 107, 107,
    "grey43", 110, 110, 110,
    "grey44", 112, 112, 112,
    "grey45", 115, 115, 115,
    "grey46", 117, 117, 117,
    "grey47", 120, 120, 120,
    "grey48", 122, 122, 122,
    "grey49", 125, 125, 125,
    "grey5", 13, 13, 13,
    "grey50", 127, 127, 127,
    "grey51", 130, 130, 130,
    "grey52", 133, 133, 133,
    "grey53", 135, 135, 135,
    "grey54", 138, 138, 138,
    "grey55", 140, 140, 140,
    "grey56", 143, 143, 143,
    "grey57", 145, 145, 145,
    "grey58", 148, 148, 148,
    "grey59", 150, 150, 150,
    "grey6", 15, 15, 15,
    "grey60", 153, 153, 153,
    "grey61", 156, 156, 156,
    "grey62", 158, 158, 158,
    "grey63", 161, 161, 161,
    "grey64", 163, 163, 163,
    "grey65", 166, 166, 166,
    "grey66", 168, 168, 168,
    "grey67", 171, 171, 171,
    "grey68", 173, 173, 173,
    "grey69", 176, 176, 176,
    "grey7", 18, 18, 18,
    "grey70", 179, 179, 179,
    "grey71", 181, 181, 181,
    "grey72", 184, 184, 184,
    "grey73", 186, 186, 186,
    "grey74", 189, 189, 189,
    "grey75", 191, 191, 191,
    "grey76", 194, 194, 194,
    "grey77", 196, 196, 196,
    "grey78", 199, 199, 199,
    "grey79", 201, 201, 201,
    "grey8", 20, 20, 20,
    "grey80", 204, 204, 204,
    "grey81", 207, 207, 207,
    "grey82", 209, 209, 209,
    "grey83", 212, 212, 212,
    "grey84", 214, 214, 214,
    "grey85", 217, 217, 217,
    "grey86", 219, 219, 219,
    "grey87", 222, 222, 222,
    "grey88", 224, 224, 224,
    "grey89", 227, 227, 227,
    "grey9", 23, 23, 23,
    "grey90", 229, 229, 229,
    "grey91", 232, 232, 232,
    "grey92", 235, 235, 235,
    "grey93", 237, 237, 237,
    "grey94", 240, 240, 240,
    "grey95", 242, 242, 242,
    "grey96", 245, 245, 245,
    "grey97", 247, 247, 247,
    "grey98", 250, 250, 250,
    "grey99", 252, 252, 252,
    "honeydew", 240, 255, 240,
    "honeydew1", 240, 255, 240,
    "honeydew2", 224, 238, 224,
    "honeydew3", 193, 205, 193,
    "honeydew4", 131, 139, 131,
    "hot pink", 255, 105, 180,
    "HotPink", 255, 105, 180,
    "HotPink1", 255, 110, 180,
    "HotPink2", 238, 106, 167,
    "HotPink3", 205, 96, 144,
    "HotPink4", 139, 58, 98,
    "indian red", 205, 92, 92,
    "IndianRed", 205, 92, 92,
    "IndianRed1", 255, 106, 106,
    "IndianRed2", 238, 99, 99,
    "IndianRed3", 205, 85, 85,
    "IndianRed4", 139, 58, 58,
    "ivory", 255, 255, 240,
    "ivory1", 255, 255, 240,
    "ivory2", 238, 238, 224,
    "ivory3", 205, 205, 193,
    "ivory4", 139, 139, 131,
    "khaki", 240, 230, 140,
    "khaki1", 255, 246, 143,
    "khaki2", 238, 230, 133,
    "khaki3", 205, 198, 115,
    "khaki4", 139, 134, 78,
    "lavender", 230, 230, 250,
    "lavender blush", 255, 240, 245,
    "LavenderBlush", 255, 240, 245,
    "LavenderBlush1", 255, 240, 245,
    "LavenderBlush2", 238, 224, 229,
    "LavenderBlush3", 205, 193, 197,
    "LavenderBlush4", 139, 131, 134,
    "lawn green", 124, 252, 0,
    "LawnGreen", 124, 252, 0,
    "lemon chiffon", 255, 250, 205,
    "LemonChiffon", 255, 250, 205,
    "LemonChiffon1", 255, 250, 205,
    "LemonChiffon2", 238, 233, 191,
    "LemonChiffon3", 205, 201, 165,
    "LemonChiffon4", 139, 137, 112,
    "light blue", 173, 216, 230,
    "light coral", 240, 128, 128,
    "light cyan", 224, 255, 255,
    "light goldenrod", 238, 221, 130,
    "light goldenrod yellow", 250, 250, 210,
    "light gray", 211, 211, 211,
    "light grey", 211, 211, 211,
    "light pink", 255, 182, 193,
    "light salmon", 255, 160, 122,
    "light sea green", 32, 178, 170,
    "light sky blue", 135, 206, 250,
    "light slate blue", 132, 112, 255,
    "light slate gray", 119, 136, 153,
    "light slate grey", 119, 136, 153,
    "light steel blue", 176, 196, 222,
    "light yellow", 255, 255, 224,
    "LightBlue", 173, 216, 230,
    "LightBlue1", 191, 239, 255,
    "LightBlue2", 178, 223, 238,
    "LightBlue3", 154, 192, 205,
    "LightBlue4", 104, 131, 139,
    "LightCoral", 240, 128, 128,
    "LightCyan", 224, 255, 255,
    "LightCyan1", 224, 255, 255,
    "LightCyan2", 209, 238, 238,
    "LightCyan3", 180, 205, 205,
    "LightCyan4", 122, 139, 139,
    "LightGoldenrod", 238, 221, 130,
    "LightGoldenrod1", 255, 236, 139,
    "LightGoldenrod2", 238, 220, 130,
    "LightGoldenrod3", 205, 190, 112,
    "LightGoldenrod4", 139, 129, 76,
    "LightGoldenrodYellow", 250, 250, 210,
    "LightGray", 211, 211, 211,
    "LightGrey", 211, 211, 211,
    "LightPink", 255, 182, 193,
    "LightPink1", 255, 174, 185,
    "LightPink2", 238, 162, 173,
    "LightPink3", 205, 140, 149,
    "LightPink4", 139, 95, 101,
    "LightSalmon", 255, 160, 122,
    "LightSalmon1", 255, 160, 122,
    "LightSalmon2", 238, 149, 114,
    "LightSalmon3", 205, 129, 98,
    "LightSalmon4", 139, 87, 66,
    "LightSeaGreen", 32, 178, 170,
    "LightSkyBlue", 135, 206, 250,
    "LightSkyBlue1", 176, 226, 255,
    "LightSkyBlue2", 164, 211, 238,
    "LightSkyBlue3", 141, 182, 205,
    "LightSkyBlue4", 96, 123, 139,
    "LightSlateBlue", 132, 112, 255,
    "LightSlateGray", 119, 136, 153,
    "LightSlateGrey", 119, 136, 153,
    "LightSteelBlue", 176, 196, 222,
    "LightSteelBlue1", 202, 225, 255,
    "LightSteelBlue2", 188, 210, 238,
    "LightSteelBlue3", 162, 181, 205,
    "LightSteelBlue4", 110, 123, 139,
    "LightYellow", 255, 255, 224,
    "LightYellow1", 255, 255, 224,
    "LightYellow2", 238, 238, 209,
    "LightYellow3", 205, 205, 180,
    "LightYellow4", 139, 139, 122,
    "lime green", 50, 205, 50,
    "LimeGreen", 50, 205, 50,
    "linen", 250, 240, 230,
    "magenta", 255, 0, 255,
    "magenta1", 255, 0, 255,
    "magenta2", 238, 0, 238,
    "magenta3", 205, 0, 205,
    "magenta4", 139, 0, 139,
    "maroon", 176, 48, 96,
    "maroon1", 255, 52, 179,
    "maroon2", 238, 48, 167,
    "maroon3", 205, 41, 144,
    "maroon4", 139, 28, 98,
    "medium aquamarine", 102, 205, 170,
    "medium blue", 0, 0, 205,
    "medium orchid", 186, 85, 211,
    "medium purple", 147, 112, 219,
    "medium sea green", 60, 179, 113,
    "medium slate blue", 123, 104, 238,
    "medium spring green", 0, 250, 154,
    "medium turquoise", 72, 209, 204,
    "medium violet red", 199, 21, 133,
    "MediumAquamarine", 102, 205, 170,
    "MediumBlue", 0, 0, 205,
    "MediumOrchid", 186, 85, 211,
    "MediumOrchid1", 224, 102, 255,
    "MediumOrchid2", 209, 95, 238,
    "MediumOrchid3", 180, 82, 205,
    "MediumOrchid4", 122, 55, 139,
    "MediumPurple", 147, 112, 219,
    "MediumPurple1", 171, 130, 255,
    "MediumPurple2", 159, 121, 238,
    "MediumPurple3", 137, 104, 205,
    "MediumPurple4", 93, 71, 139,
    "MediumSeaGreen", 60, 179, 113,
    "MediumSlateBlue", 123, 104, 238,
    "MediumSpringGreen", 0, 250, 154,
    "MediumTurquoise", 72, 209, 204,
    "MediumVioletRed", 199, 21, 133,
    "midnight blue", 25, 25, 112,
    "MidnightBlue", 25, 25, 112,
    "mint cream", 245, 255, 250,
    "MintCream", 245, 255, 250,
    "misty rose", 255, 228, 225,
    "MistyRose", 255, 228, 225,
    "MistyRose1", 255, 228, 225,
    "MistyRose2", 238, 213, 210,
    "MistyRose3", 205, 183, 181,
    "MistyRose4", 139, 125, 123,
    "moccasin", 255, 228, 181,
    "navajo white", 255, 222, 173,
    "NavajoWhite", 255, 222, 173,
    "NavajoWhite1", 255, 222, 173,
    "NavajoWhite2", 238, 207, 161,
    "NavajoWhite3", 205, 179, 139,
    "NavajoWhite4", 139, 121, 94,
    "navy", 0, 0, 128,
    "navy blue", 0, 0, 128,
    "NavyBlue", 0, 0, 128,
    "old lace", 253, 245, 230,
    "OldLace", 253, 245, 230,
    "olive drab", 107, 142, 35,
    "OliveDrab", 107, 142, 35,
    "OliveDrab1", 192, 255, 62,
    "OliveDrab2", 179, 238, 58,
    "OliveDrab3", 154, 205, 50,
    "OliveDrab4", 105, 139, 34,
    "orange", 255, 165, 0,
    "orange red", 255, 69, 0,
    "orange1", 255, 165, 0,
    "orange2", 238, 154, 0,
    "orange3", 205, 133, 0,
    "orange4", 139, 90, 0,
    "OrangeRed", 255, 69, 0,
    "OrangeRed1", 255, 69, 0,
    "OrangeRed2", 238, 64, 0,
    "OrangeRed3", 205, 55, 0,
    "OrangeRed4", 139, 37, 0,
    "orchid", 218, 112, 214,
    "orchid1", 255, 131, 250,
    "orchid2", 238, 122, 233,
    "orchid3", 205, 105, 201,
    "orchid4", 139, 71, 137,
    "pale goldenrod", 238, 232, 170,
    "pale green", 152, 251, 152,
    "pale turquoise", 175, 238, 238,
    "pale violet red", 219, 112, 147,
    "PaleGoldenrod", 238, 232, 170,
    "PaleGreen", 152, 251, 152,
    "PaleGreen1", 154, 255, 154,
    "PaleGreen2", 144, 238, 144,
    "PaleGreen3", 124, 205, 124,
    "PaleGreen4", 84, 139, 84,
    "PaleTurquoise", 175, 238, 238,
    "PaleTurquoise1", 187, 255, 255,
    "PaleTurquoise2", 174, 238, 238,
    "PaleTurquoise3", 150, 205, 205,
    "PaleTurquoise4", 102, 139, 139,
    "PaleVioletRed", 219, 112, 147,
    "PaleVioletRed1", 255, 130, 171,
    "PaleVioletRed2", 238, 121, 159,
    "PaleVioletRed3", 205, 104, 137,
    "PaleVioletRed4", 139, 71, 93,
    "papaya whip", 255, 239, 213,
    "PapayaWhip", 255, 239, 213,
    "peach puff", 255, 218, 185,
    "PeachPuff", 255, 218, 185,
    "PeachPuff1", 255, 218, 185,
    "PeachPuff2", 238, 203, 173,
    "PeachPuff3", 205, 175, 149,
    "PeachPuff4", 139, 119, 101,
    "peru", 205, 133, 63,
    "pink", 255, 192, 203,
    "pink1", 255, 181, 197,
    "pink2", 238, 169, 184,
    "pink3", 205, 145, 158,
    "pink4", 139, 99, 108,
    "plum", 221, 160, 221,
    "plum1", 255, 187, 255,
    "plum2", 238, 174, 238,
    "plum3", 205, 150, 205,
    "plum4", 139, 102, 139,
    "powder blue", 176, 224, 230,
    "PowderBlue", 176, 224, 230,
    "purple", 160, 32, 240,
    "purple1", 155, 48, 255,
    "purple2", 145, 44, 238,
    "purple3", 125, 38, 205,
    "purple4", 85, 26, 139,
    "red", 255, 0, 0,
    "red1", 255, 0, 0,
    "red2", 238, 0, 0,
    "red3", 205, 0, 0,
    "red4", 139, 0, 0,
    "rosy brown", 188, 143, 143,
    "RosyBrown", 188, 143, 143,
    "RosyBrown1", 255, 193, 193,
    "RosyBrown2", 238, 180, 180,
    "RosyBrown3", 205, 155, 155,
    "RosyBrown4", 139, 105, 105,
    "royal blue", 65, 105, 225,
    "RoyalBlue", 65, 105, 225,
    "RoyalBlue1", 72, 118, 255,
    "RoyalBlue2", 67, 110, 238,
    "RoyalBlue3", 58, 95, 205,
    "RoyalBlue4", 39, 64, 139,
    "saddle brown", 139, 69, 19,
    "SaddleBrown", 139, 69, 19,
    "salmon", 250, 128, 114,
    "salmon1", 255, 140, 105,
    "salmon2", 238, 130, 98,
    "salmon3", 205, 112, 84,
    "salmon4", 139, 76, 57,
    "sandy brown", 244, 164, 96,
    "SandyBrown", 244, 164, 96,
    "sea green", 46, 139, 87,
    "SeaGreen", 46, 139, 87,
    "SeaGreen1", 84, 255, 159,
    "SeaGreen2", 78, 238, 148,
    "SeaGreen3", 67, 205, 128,
    "SeaGreen4", 46, 139, 87,
    "seashell", 255, 245, 238,
    "seashell1", 255, 245, 238,
    "seashell2", 238, 229, 222,
    "seashell3", 205, 197, 191,
    "seashell4", 139, 134, 130,
    "sienna", 160, 82, 45,
    "sienna1", 255, 130, 71,
    "sienna2", 238, 121, 66,
    "sienna3", 205, 104, 57,
    "sienna4", 139, 71, 38,
    "sky blue", 135, 206, 235,
    "SkyBlue", 135, 206, 235,
    "SkyBlue1", 135, 206, 255,
    "SkyBlue2", 126, 192, 238,
    "SkyBlue3", 108, 166, 205,
    "SkyBlue4", 74, 112, 139,
    "slate blue", 106, 90, 205,
    "slate gray", 112, 128, 144,
    "slate grey", 112, 128, 144,
    "SlateBlue", 106, 90, 205,
    "SlateBlue1", 131, 111, 255,
    "SlateBlue2", 122, 103, 238,
    "SlateBlue3", 105, 89, 205,
    "SlateBlue4", 71, 60, 139,
    "SlateGray", 112, 128, 144,
    "SlateGray1", 198, 226, 255,
    "SlateGray2", 185, 211, 238,
    "SlateGray3", 159, 182, 205,
    "SlateGray4", 108, 123, 139,
    "SlateGrey", 112, 128, 144,
    "snow", 255, 250, 250,
    "snow1", 255, 250, 250,
    "snow2", 238, 233, 233,
    "snow3", 205, 201, 201,
    "snow4", 139, 137, 137,
    "spring green", 0, 255, 127,
    "SpringGreen", 0, 255, 127,
    "SpringGreen1", 0, 255, 127,
    "SpringGreen2", 0, 238, 118,
    "SpringGreen3", 0, 205, 102,
    "SpringGreen4", 0, 139, 69,
    "steel blue", 70, 130, 180,
    "SteelBlue", 70, 130, 180,
    "SteelBlue1", 99, 184, 255,
    "SteelBlue2", 92, 172, 238,
    "SteelBlue3", 79, 148, 205,
    "SteelBlue4", 54, 100, 139,
    "tan", 210, 180, 140,
    "tan1", 255, 165, 79,
    "tan2", 238, 154, 73,
    "tan3", 205, 133, 63,
    "tan4", 139, 90, 43,
    "thistle", 216, 191, 216,
    "thistle1", 255, 225, 255,
    "thistle2", 238, 210, 238,
    "thistle3", 205, 181, 205,
    "thistle4", 139, 123, 139,
    "tomato", 255, 99, 71,
    "tomato1", 255, 99, 71,
    "tomato2", 238, 92, 66,
    "tomato3", 205, 79, 57,
    "tomato4", 139, 54, 38,
    "turquoise", 64, 224, 208,
    "turquoise1", 0, 245, 255,
    "turquoise2", 0, 229, 238,
    "turquoise3", 0, 197, 205,
    "turquoise4", 0, 134, 139,
    "violet", 238, 130, 238,
    "violet red", 208, 32, 144,
    "VioletRed", 208, 32, 144,
    "VioletRed1", 255, 62, 150,
    "VioletRed2", 238, 58, 140,
    "VioletRed3", 205, 50, 120,
    "VioletRed4", 139, 34, 82,
    "wheat", 245, 222, 179,
    "wheat1", 255, 231, 186,
    "wheat2", 238, 216, 174,
    "wheat3", 205, 186, 150,
    "wheat4", 139, 126, 102,
    "white", 255, 255, 255,
    "white smoke", 245, 245, 245,
    "WhiteSmoke", 245, 245, 245,
    "yellow", 255, 255, 0,
    "yellow green", 154, 205, 50,
    "yellow1", 255, 255, 0,
    "yellow2", 238, 238, 0,
    "yellow3", 205, 205, 0,
    "yellow4", 139, 139, 0,
    "YellowGreen", 154, 205, 50,
    NULL, 0, 0, 0
};


/*
 * This value will be set to the number of colors in the color table
 * the first time it is needed.
 */

static int numXColors = 0;

/*
 * Forward declarations for functions used only in this file.
 */

static int	FindColor(const char *name, XColor *colorPtr);

int strcasecmp(const char *a, const char *b)
{
	int i=0,c;
	if((a==NULL)||(b==NULL)) return -1;
	
	while(((!(c=toupper(a[i])-toupper(b[i])))&&a[i]&&b[i])) i++;
	return c;
}
/*
 *----------------------------------------------------------------------
 *
 * FindColor --
 *
 *	This routine finds the color entry that corresponds to the
 *	specified color.
 *
 * Results:
 *	Returns non-zero on success.  The RGB values of the XColor
 *	will be initialized to the proper values on success.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int
FindColor(name, colorPtr)
    const char *name;
    XColor *colorPtr;
{
    int l, u, r, i;

    /*
     * Count the number of elements in the color array if we haven't
     * done so yet.
     */

    if (numXColors == 0) {
	XColorEntry *ePtr;
	for (ePtr = xColors; ePtr->name != NULL; ePtr++) {
	    numXColors++;
	}
    }

    /*
     * Perform a binary search on the sorted array of colors.
     */

    l = 0;
    u = numXColors - 1;
    while (l <= u) {
	i = (l + u) / 2;
	r = strcasecmp(name, xColors[i].name);
	if (r == 0) {
	    break;
	} else if (r < 0) {
	    u = i-1;
	} else {
	    l = i+1;
	}
    }
    if (l > u) {
	return 0;
    }
    colorPtr->red = xColors[i].red << 8;
    colorPtr->green = xColors[i].green << 8;
    colorPtr->blue = xColors[i].blue << 8;
    return 1;
}

/*
 *----------------------------------------------------------------------
 *
 * XParseColor --
 *
 *	Partial implementation of X color name parsing interface.
 *
 * Results:
 *	Returns non-zero on success.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

StatusDef
XParseColor(display, map, spec, colorPtr)
    Display *display;
    Colormap map;
    const char* spec;
    XColor *colorPtr;
{
    if (spec[0] == '#') {
	char fmt[16];
	int i, red, green, blue;
	
	if ((i = strlen(spec+1))%3) {
	    return 0;
	}
	i /= 3;

	sprintf(fmt, "%%%dx%%%dx%%%dx", i, i, i);
	if (sscanf(spec+1, fmt, &red, &green, &blue) != 3) {
	    return 0;
	}
	colorPtr->red = ((unsigned short) red) << (4 * (4 - i));
	colorPtr->green = ((unsigned short) green) << (4 * (4 - i));
	colorPtr->blue = ((unsigned short) blue) << (4 * (4 - i));
    } else {
	if (!FindColor(spec, colorPtr)) {
	    return 0;
	}
    }
    colorPtr->pixel = ((colorPtr->red)>>8)&0xff;
    colorPtr->flags = DoRed|DoGreen|DoBlue;
    colorPtr->pad = 0;
    return 1;
}
/** xpm support */
int
XFreeColors(display, cmap, pixels, npixels, planes)
Display *display;
Colormap cmap;
unsigned long pixels[];
int npixels;
unsigned long planes;
{
	return 0;
}

int
XGrabServer(display)
	 Display *display;
{
	return 0;
}

int
XUngrabServer(display)
	 Display *display;
{
	return 0;
}


#endif
