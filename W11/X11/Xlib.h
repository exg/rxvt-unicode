/* 

Copyright 1985, 1986, 1987, 1991, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

#ifndef __XLIB_H
#define __XLIB_H

/* typedef struct _XrmHashBucketRec; */
typedef unsigned long Atom;
typedef unsigned long Time;
typedef unsigned long XID;

#ifndef Bool
#define Bool int
#endif

typedef int BoolDef;

typedef XID Window;
typedef XID Drawable;
typedef XID Font;
typedef XID Pixmap;
typedef XID Cursor;
typedef XID Colormap;
typedef XID KeySym;
typedef XID GContext;

typedef unsigned char KeyCode;

typedef char *XPointer;
typedef unsigned long VisualID;

#define PIXEL_ALREADY_TYPEDEFED
typedef unsigned long Pixel;

#define XDestroyImage(ximage) \
	((*((ximage)->f.destroy_image))((ximage)))
#define XPutPixel(ximage, x, y, pixel) \
	((*((ximage)->f.put_pixel))((ximage), (x), (y), (pixel)))

#define AllPlanes 		((unsigned long)~0L)
#define ScreenOfDisplay(dpy, scr)(&((_XPrivDisplay)dpy)->screens[scr])
#define DefaultVisual(dpy, scr) (ScreenOfDisplay(dpy,scr)->root_visual)
#define DefaultDepth(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->root_depth)
#define DefaultColormap(dpy, scr)(ScreenOfDisplay(dpy,scr)->cmap)
#define DefaultScreen(dpy) 	(((_XPrivDisplay)dpy)->default_screen)
#define BlackPixel(dpy, scr)	(ScreenOfDisplay(dpy,scr)->black_pixel)
#define WhitePixel(dpy, scr)	(ScreenOfDisplay(dpy,scr)->white_pixel)
#define RootWindow(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->root)
#define DefaultRootWindow(dpy) 	(ScreenOfDisplay(dpy,DefaultScreen(dpy))->root)
#define DisplayWidth(dpy, scr) 	(ScreenOfDisplay(dpy,scr)->width)
#define DisplayHeight(dpy, scr) (ScreenOfDisplay(dpy,scr)->height)

#ifndef None
#define None                 0L	/* universal null resource or null atom */
#endif

#define ParentRelative       1L	
#define CopyFromParent       0L	
#define PointerWindow        0L	/* destination window in SendEvent */
#define InputFocus           1L	/* destination window in SendEvent */
#define PointerRoot          1L	/* focus window in SetInputFocus */
#define AnyPropertyType      0L	/* special Atom, passed to GetProperty */
#define AnyKey		     0L	/* special Key Code, passed to GrabKey */
#define AnyButton            0L	/* special Button Code, passed to GrabButton */
#define AllTemporary         0L	/* special Resource ID passed to KillClient */
#define CurrentTime          0L	/* special Time */
#define NoSymbol	     0L	/* special KeySym */
#define NoEventMask			0L
#define KeyPressMask			(1L<<0)  
#define KeyReleaseMask			(1L<<1)  
#define ButtonPressMask			(1L<<2)  
#define ButtonReleaseMask		(1L<<3)  
#define EnterWindowMask			(1L<<4)  
#define LeaveWindowMask			(1L<<5)  
#define PointerMotionMask		(1L<<6)  
#define PointerMotionHintMask		(1L<<7)  
#define Button1MotionMask		(1L<<8)  
#define Button2MotionMask		(1L<<9)  
#define Button3MotionMask		(1L<<10) 
#define Button4MotionMask		(1L<<11) 
#define Button5MotionMask		(1L<<12) 
#define ButtonMotionMask		(1L<<13) 
#define KeymapStateMask			(1L<<14)
#define ExposureMask			(1L<<15) 
#define VisibilityChangeMask		(1L<<16) 
#define StructureNotifyMask		(1L<<17) 
#define ResizeRedirectMask		(1L<<18) 
#define SubstructureNotifyMask		(1L<<19) 
#define SubstructureRedirectMask	(1L<<20) 
#define FocusChangeMask			(1L<<21) 
#define PropertyChangeMask		(1L<<22) 
#define ColormapChangeMask		(1L<<23) 
#define OwnerGrabButtonMask		(1L<<24) 
#define KeyPress		2
#define KeyRelease		3
#define ButtonPress		4
#define ButtonRelease		5
#define MotionNotify		6
#define EnterNotify		7
#define LeaveNotify		8
#define FocusIn			9
#define FocusOut		10
#define KeymapNotify		11
#define Expose			12
#define GraphicsExpose		13
#define NoExpose		14
#define VisibilityNotify	15
#define CreateNotify		16
#define DestroyNotify		17
#define UnmapNotify		18
#define MapNotify		19
#define MapRequest		20
#define ReparentNotify		21
#define ConfigureNotify		22
#define ConfigureRequest	23
#define GravityNotify		24
#define ResizeRequest		25
#define CirculateNotify		26
#define CirculateRequest	27
#define PropertyNotify		28
#define SelectionClear		29
#define SelectionRequest	30
#define SelectionNotify		31
#define ColormapNotify		32
#define ClientMessage		33
#define MappingNotify		34
#define LASTEvent		35	/* must be bigger than any event # */
#define ShiftMask		(1<<0)
#define LockMask		(1<<1)
#define ControlMask		(1<<2)
#define Mod1Mask		(1<<3)
#define Mod2Mask		(1<<4)
#define Mod3Mask		(1<<5)
#define Mod4Mask		(1<<6)
#define Mod5Mask		(1<<7)
#define ShiftMapIndex		0
#define LockMapIndex		1
#define ControlMapIndex		2
#define Mod1MapIndex		3
#define Mod2MapIndex		4
#define Mod3MapIndex		5
#define Mod4MapIndex		6
#define Mod5MapIndex		7
#define Button1Mask		(1<<8)
#define Button2Mask		(1<<9)
#define Button3Mask		(1<<10)
#define Button4Mask		(1<<11)
#define Button5Mask		(1<<12)
#define AnyModifier		(1<<15)  /* used in GrabButton, GrabKey */
#define Button1			1
#define Button2			2
#define Button3			3
#define Button4			4
#define Button5			5
#define NotifyNormal		0
#define NotifyGrab		1
#define NotifyUngrab		2
#define NotifyWhileGrabbed	3
#define NotifyHint		1	/* for MotionNotify events */
#define NotifyAncestor		0
#define NotifyVirtual		1
#define NotifyInferior		2
#define NotifyNonlinear		3
#define NotifyNonlinearVirtual	4
#define NotifyPointer		5
#define NotifyPointerRoot	6
#define NotifyDetailNone	7
#define VisibilityUnobscured		0
#define VisibilityPartiallyObscured	1
#define VisibilityFullyObscured		2
#define PlaceOnTop		0
#define PlaceOnBottom		1
#define FamilyInternet		0
#define FamilyDECnet		1
#define FamilyChaos		2
#define PropertyNewValue	0
#define PropertyDelete		1
#define ColormapUninstalled	0
#define ColormapInstalled	1
#define GrabModeSync		0
#define GrabModeAsync		1
#define GrabSuccess		0
#define AlreadyGrabbed		1
#define GrabInvalidTime		2
#define GrabNotViewable		3
#define GrabFrozen		4
#define AsyncPointer		0
#define SyncPointer		1
#define ReplayPointer		2
#define AsyncKeyboard		3
#define SyncKeyboard		4
#define ReplayKeyboard		5
#define AsyncBoth		6
#define SyncBoth		7
#define RevertToNone		(int)None
#define RevertToPointerRoot	(int)PointerRoot
#define RevertToParent		2
#define Success		   0	/* everything's okay */
#define BadRequest	   1	/* bad request code */
#define BadValue	   2	/* int parameter out of range */
#define BadWindow	   3	/* parameter not a Window */
#define BadPixmap	   4	/* parameter not a Pixmap */
#define BadAtom		   5	/* parameter not an Atom */
#define BadCursor	   6	/* parameter not a Cursor */
#define BadFont		   7	/* parameter not a Font */
#define BadMatch	   8	/* parameter mismatch */
#define BadDrawable	   9	/* parameter not a Pixmap or Window */
#define BadAccess	  10   
#define BadAlloc	  11	/* insufficient resources */
#define BadColor	  12	/* no such colormap */
#define BadGC		  13	/* parameter not a GC */
#define BadIDChoice	  14	/* choice not in range or already used */
#define BadName		  15	/* font or color name doesn't exist */
#define BadLength	  16	/* Request length incorrect */
#define BadImplementation 17	/* server is defective */
#define FirstExtensionError	128
#define LastExtensionError	255
#define InputOutput		1
#define InputOnly		2
#define CWBackPixmap		(1L<<0)
#define CWBackPixel		(1L<<1)
#define CWBorderPixmap		(1L<<2)
#define CWBorderPixel           (1L<<3)
#define CWBitGravity		(1L<<4)
#define CWWinGravity		(1L<<5)
#define CWBackingStore          (1L<<6)
#define CWBackingPlanes	        (1L<<7)
#define CWBackingPixel	        (1L<<8)
#define CWOverrideRedirect	(1L<<9)
#define CWSaveUnder		(1L<<10)
#define CWEventMask		(1L<<11)
#define CWDontPropagate	        (1L<<12)
#define CWColormap		(1L<<13)
#define CWCursor	        (1L<<14)
#define CWX			(1<<0)
#define CWY			(1<<1)
#define CWWidth			(1<<2)
#define CWHeight		(1<<3)
#define CWBorderWidth		(1<<4)
#define CWSibling		(1<<5)
#define CWStackMode		(1<<6)
#define ForgetGravity		0
#define NorthWestGravity	1
#define NorthGravity		2
#define NorthEastGravity	3
#define WestGravity		4
#define CenterGravity		5
#define EastGravity		6
#define SouthWestGravity	7
#define SouthGravity		8
#define SouthEastGravity	9
#define StaticGravity		10
#define UnmapGravity		0
#define NotUseful               0
#define WhenMapped              1
#define Always                  2
#define IsUnmapped		0
#define IsUnviewable		1
#define IsViewable		2
#define SetModeInsert           0
#define SetModeDelete           1
#define DestroyAll              0
#define RetainPermanent         1
#define RetainTemporary         2
#define Above                   0
#define Below                   1
#define TopIf                   2
#define BottomIf                3
#define Opposite                4
#define RaiseLowest             0
#define LowerHighest            1
#define PropModeReplace         0
#define PropModePrepend         1
#define PropModeAppend          2
#define	GXclear			0x0		/* 0 */
#define GXand			0x1		/* src AND dst */
#define GXandReverse		0x2		/* src AND NOT dst */
#define GXcopy			0x3		/* src */
#define GXandInverted		0x4		/* NOT src AND dst */
#define	GXnoop			0x5		/* dst */
#define GXxor			0x6		/* src XOR dst */
#define GXor			0x7		/* src OR dst */
#define GXnor			0x8		/* NOT src AND NOT dst */
#define GXequiv			0x9		/* NOT src XOR dst */
#define GXinvert		0xa		/* NOT dst */
#define GXorReverse		0xb		/* src OR NOT dst */
#define GXcopyInverted		0xc		/* NOT src */
#define GXorInverted		0xd		/* NOT src OR dst */
#define GXnand			0xe		/* NOT src OR NOT dst */
#define GXset			0xf		/* 1 */
#define LineSolid		0
#define LineOnOffDash		1
#define LineDoubleDash		2
#define CapNotLast		0
#define CapButt			1
#define CapRound		2
#define CapProjecting		3
#define JoinMiter		0
#define JoinRound		1
#define JoinBevel		2
#define FillSolid		0
#define FillTiled		1
#define FillStippled		2
#define FillOpaqueStippled	3
#define EvenOddRule		0
#define WindingRule		1
#define ClipByChildren		0
#define IncludeInferiors	1
#define Unsorted		0
#define YSorted			1
#define YXSorted		2
#define YXBanded		3
#define CoordModeOrigin		0	/* relative to the origin */
#define CoordModePrevious       1	/* relative to previous point */
#define Complex			0	/* paths may intersect */
#define Nonconvex		1	/* no paths intersect, but not convex */
#define Convex			2	/* wholly convex */
#define ArcChord		0	/* join endpoints of arc */
#define ArcPieSlice		1	/* join endpoints to center of arc */
#define GCFunction              (1L<<0)
#define GCPlaneMask             (1L<<1)
#define GCForeground            (1L<<2)
#define GCBackground            (1L<<3)
#define GCLineWidth             (1L<<4)
#define GCLineStyle             (1L<<5)
#define GCCapStyle              (1L<<6)
#define GCJoinStyle		(1L<<7)
#define GCFillStyle		(1L<<8)
#define GCFillRule		(1L<<9) 
#define GCTile			(1L<<10)
#define GCStipple		(1L<<11)
#define GCTileStipXOrigin	(1L<<12)
#define GCTileStipYOrigin	(1L<<13)
#define GCFont 			(1L<<14)
#define GCSubwindowMode		(1L<<15)
#define GCGraphicsExposures     (1L<<16)
#define GCClipXOrigin		(1L<<17)
#define GCClipYOrigin		(1L<<18)
#define GCClipMask		(1L<<19)
#define GCDashOffset		(1L<<20)
#define GCDashList		(1L<<21)
#define GCArcMode		(1L<<22)
#define GCLastBit		22
#define FontLeftToRight		0
#define FontRightToLeft		1
#define FontChange		255
#define XYBitmap		0	/* depth 1, XYFormat */
#define XYPixmap		1	/* depth == drawable depth */
#define ZPixmap			2	/* depth == drawable depth */
#define AllocNone		0	/* create map with no entries */
#define AllocAll		1	/* allocate entire map writeable */
#define DoRed			(1<<0)
#define DoGreen			(1<<1)
#define DoBlue			(1<<2)
#define CursorShape		0	/* largest size that can be displayed */
#define TileShape		1	/* size tiled fastest */
#define StippleShape		2	/* size stippled fastest */
#define AutoRepeatModeOff	0
#define AutoRepeatModeOn	1
#define AutoRepeatModeDefault	2
#define LedModeOff		0
#define LedModeOn		1
#define KBKeyClickPercent	(1L<<0)
#define KBBellPercent		(1L<<1)
#define KBBellPitch		(1L<<2)
#define KBBellDuration		(1L<<3)
#define KBLed			(1L<<4)
#define KBLedMode		(1L<<5)
#define KBKey			(1L<<6)
#define KBAutoRepeatMode	(1L<<7)
#define MappingSuccess     	0
#define MappingBusy        	1
#define MappingFailed		2
#define MappingModifier		0
#define MappingKeyboard		1
#define MappingPointer		2
#define DontPreferBlanking	0
#define PreferBlanking		1
#define DefaultBlanking		2
#define DisableScreenSaver	0
#define DisableScreenInterval	0
#define DontAllowExposures	0
#define AllowExposures		1
#define DefaultExposures	2
#define ScreenSaverReset 0
#define ScreenSaverActive 1
#define HostInsert		0
#define HostDelete		1
#define EnableAccess		1      
#define DisableAccess		0
#define StaticGray		0
#define GrayScale		1
#define StaticColor		2
#define PseudoColor		3
#define TrueColor		4
#define DirectColor		5
#define LSBFirst		0
#define MSBFirst		1

#define True 1
#define False 0

#define XK_Alt_L		0xFFE9	/* Left alt */

#define XK_BackSpace		0xFF08	/* back space, back char */
#define XK_Tab			0xFF09
#define XK_Linefeed		0xFF0A	/* Linefeed, LF */
#define XK_Clear		0xFF0B
#define XK_Return		0xFF0D	/* Return, enter */
#define XK_Pause		0xFF13	/* Pause, hold */
#define XK_Scroll_Lock		0xFF14
#define XK_Sys_Req		0xFF15
#define XK_Escape		0xFF1B
#define XK_Delete		0xFFFF	/* Delete, rubout */

#define XK_Home			0xFF50
#define XK_Left			0xFF51	/* Move left, left arrow */
#define XK_Up			0xFF52	/* Move up, up arrow */
#define XK_Right		0xFF53	/* Move right, right arrow */
#define XK_Down			0xFF54	/* Move down, down arrow */
#define XK_Prior		0xFF55	/* Prior, previous */
#define XK_Page_Up		0xFF55
#define XK_Next			0xFF56	/* Next */
#define XK_Page_Down		0xFF56
#define XK_End			0xFF57	/* EOL */
#define XK_Begin		0xFF58	/* BOL */

#define XK_Select		0xFF60	/* Select, mark */
#define XK_Print		0xFF61
#define XK_Execute		0xFF62	/* Execute, run, do */
#define XK_Insert		0xFF63	/* Insert, insert here */
#define XK_Undo			0xFF65	/* Undo, oops */
#define XK_Redo			0xFF66	/* redo, again */
#define XK_Menu			0xFF67
#define XK_Find			0xFF68	/* Find, search */
#define XK_Cancel		0xFF69	/* Cancel, stop, abort, exit */
#define XK_Help			0xFF6A	/* Help */
#define XK_Break		0xFF6B
#define XK_Mode_switch		0xFF7E	/* Character set switch */
#define XK_script_switch        0xFF7E  /* Alias for mode_switch */
#define XK_Num_Lock		0xFF7F

#define XK_F1			0xFFBE
#define XK_F2			0xFFBF
#define XK_F3			0xFFC0
#define XK_F4			0xFFC1
#define XK_F5			0xFFC2
#define XK_F6			0xFFC3
#define XK_F7			0xFFC4
#define XK_F8			0xFFC5
#define XK_F9			0xFFC6
#define XK_F10			0xFFC7
#define XK_F11			0xFFC8
#define XK_L1			0xFFC8
#define XK_F12			0xFFC9

#define VisualNoMask		0x0
#define VisualIDMask 		0x1
#define VisualScreenMask	0x2
#define VisualDepthMask		0x4
#define VisualClassMask		0x8
#define VisualRedMaskMask	0x10
#define VisualGreenMaskMask	0x20
#define VisualBlueMaskMask	0x40
#define VisualColormapSizeMask	0x80
#define VisualBitsPerRGBMask	0x100
#define VisualAllMask		0x1FF

#define USPosition	(1L << 0) /* user specified x, y */
#define USSize		(1L << 1) /* user specified width, height */

#define PPosition	(1L << 2) /* program specified position */
#define PSize		(1L << 3) /* program specified size */
#define PMinSize	(1L << 4) /* program specified minimum size */
#define PMaxSize	(1L << 5) /* program specified maximum size */
#define PResizeInc	(1L << 6) /* program specified resize increments */
#define PAspect		(1L << 7) /* program specified min and max aspect ratios */
#define PBaseSize	(1L << 8) /* program specified base for incrementing */
#define PWinGravity	(1L << 9) /* program specified window gravity */

#define NoValue		0x0000
#define XValue  	0x0001
#define YValue		0x0002
#define WidthValue  	0x0004
#define HeightValue  	0x0008
#define AllValues 	0x000F
#define XNegative 	0x0010
#define YNegative 	0x0020

#define XNoMemory -1
#define XLocaleNotSupported -2
#define XConverterNotFound -3

#define LC_CTYPE 2

/* used by rxvt/src/main.c */
#define NormalState 1
#define IconicState 3
#define InputHint (1L << 0)
#define StateHint (1L << 1)
#define IconWindowHint (1L << 3)
#define WindowGroupHint (1L << 6)
#define XC_left_ptr 68
#define XC_xterm 152

typedef struct _XRegion *Region;
typedef struct _XOC *XOC, *XFontSet;

typedef struct _XExtData {
	int number;		/* number returned by XRegisterExtension */
	struct _XExtData *next;	/* next item on list of data for structure */
	int (*free_private)(	/* called to free private storage */
	struct _XExtData *extension
	);
	XPointer private_data;	/* data private to this extension. */
} XExtData;

typedef struct {		/* public to extension, cannot be changed */
	int extension;		/* extension number */
	int major_opcode;	/* major op-code assigned by server */
	int first_event;	/* first event number for the extension */
	int first_error;	/* first error number for the extension */
} XExtCodes;

typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	VisualID visualid;	/* visual id of this visual */
#if defined(__cplusplus) || defined(c_plusplus)
	int c_class;		/* C++ class of screen (monochrome, etc.) */
#else
	int class;		/* class of screen (monochrome, etc.) */
#endif
	unsigned long red_mask, green_mask, blue_mask;	/* mask values */
	int bits_per_rgb;	/* log base 2 of distinct color values */
	int map_entries;	/* color map entries */
} Visual;

typedef struct {
	int depth;		/* this depth (Z) of the depth */
	int nvisuals;		/* number of Visual types at this depth */
	Visual *visuals;	/* list of visuals possible at this depth */
} Depth;

typedef struct {
	int function;		/* logical operation */
	unsigned long plane_mask;/* plane mask */
	unsigned long foreground;/* foreground pixel */
	unsigned long background;/* background pixel */
	int line_width;		/* line width */
	int line_style;	 	/* LineSolid, LineOnOffDash, LineDoubleDash */
	int cap_style;	  	/* CapNotLast, CapButt, 
				   CapRound, CapProjecting */
	int join_style;	 	/* JoinMiter, JoinRound, JoinBevel */
	int fill_style;	 	/* FillSolid, FillTiled, 
				   FillStippled, FillOpaeueStippled */
	int fill_rule;	  	/* EvenOddRule, WindingRule */
	int arc_mode;		/* ArcChord, ArcPieSlice */
	Pixmap tile;		/* tile pixmap for tiling operations */
	Pixmap stipple;		/* stipple 1 plane pixmap for stipping */
	int ts_x_origin;	/* offset for tile or stipple operations */
	int ts_y_origin;
        Font font;	        /* default text font for text operations */
	int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
	BoolDef graphics_exposures;/* boolean, should exposures be generated */
	int clip_x_origin;	/* origin for clipping */
	int clip_y_origin;
	Pixmap clip_mask;	/* bitmap clipping; other calls for rects */
	int dash_offset;	/* patterned/dashed line information */
	char dashes;
} XGCValues;

typedef struct _XGC
{
    XExtData *ext_data;	/* hook for extension to hang data */
    GContext gid;	/* protocol ID for graphics context */
	int rects;
	int dashes;
	XGCValues values;
	int dirty;
} *GC;

typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XDisplay *display;/* back pointer to display structure */
	Window root;		/* Root window id. */
	int width, height;	/* width and height of screen */
	int mwidth, mheight;	/* width and height of  in millimeters */
	int ndepths;		/* number of depths possible */
	Depth *depths;		/* list of allowable depths on the screen */
	int root_depth;		/* bits per pixel */
	Visual *root_visual;	/* root visual */
	GC default_gc;		/* GC for the root root visual */
	Colormap cmap;		/* default color map */
	unsigned long white_pixel;
	unsigned long black_pixel;	/* White and Black pixel values */
	int max_maps, min_maps;	/* max and min color maps */
	int backing_store;	/* Never, WhenMapped, Always */
	BoolDef save_unders;	
	long root_input_mask;	/* initial root input mask */
} Screen;

typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	int depth;		/* depth of this image format */
	int bits_per_pixel;	/* bits/pixel at this depth */
	int scanline_pad;	/* scanline must padded to this multiple */
} ScreenFormat;

typedef struct _XDisplay
{
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XPrivate *private1;
	int fd;			/* Network socket. */
	int private2;
	int proto_major_version;/* major version of server's X protocol */
	int proto_minor_version;/* minor version of servers X protocol */
	char *vendor;		/* vendor of the server hardware */
	XID private3;
	XID private4;
	XID private5;
	int private6;
	XID (*resource_alloc)(	/* allocator function */
		struct _XDisplay*
	);
	int byte_order;		/* screen byte order, LSBFirst, MSBFirst */
	int bitmap_unit;	/* padding and data requirements */
	int bitmap_pad;		/* padding requirements on bitmaps */
	int bitmap_bit_order;	/* LeastSignificant or MostSignificant */
	int nformats;		/* number of pixmap formats in list */
	ScreenFormat *pixmap_format;	/* pixmap format list */
	int private8;
	int release;		/* release of the server */
	struct _XPrivate *private9, *private10;
	int qlen;		/* Length of input event queue */
	unsigned long last_request_read; /* seq number of last event read */
	unsigned long request;	/* sequence number of last request. */
	XPointer private11;
	XPointer private12;
	XPointer private13;
	XPointer private14;
	unsigned max_request_size; /* maximum number 32 bit words in request*/
	struct _XrmHashBucketRec *db;
	int (*private15)(
		struct _XDisplay*
		);
	char *display_name;	/* "host:display" string used on this connect*/
	int default_screen;	/* default screen for operations */
	int nscreens;		/* number of screens on this server*/
	Screen *screens;	/* pointer to list of screens */
	unsigned long motion_buffer;	/* size of motion buffer */
	unsigned long private16;
	int min_keycode;	/* minimum defined keycode */
	int max_keycode;	/* maximum defined keycode */
	XPointer private17;
	XPointer private18;
	int private19;
	char *xdefaults;	/* contents of defaults from server */
	/* there is more to this structure, but it is private to Xlib */
}
Display, *_XPrivDisplay;

typedef int StatusDef;

typedef struct {		/* normal 16 bit characters are two bytes */
    unsigned char byte1;
    unsigned char byte2;
} XChar2b;

typedef struct {
    short	lbearing;	/* origin to left edge of raster */
    short	rbearing;	/* origin to right edge of raster */
    short	width;		/* advance to next char's origin */
    short	ascent;		/* baseline to top edge of raster */
    short	descent;	/* baseline to bottom edge of raster */
    unsigned short attributes;	/* per char flags (not predefined) */
} XCharStruct;

typedef struct {
	char *res_name;
	char *res_class;
} XClassHint;

typedef struct {
	unsigned long pixel;
	unsigned short red, green, blue;
	char flags;  /* do_red, do_green, do_blue */
	char pad;
} XColor;

typedef struct _XComposeStatus {
    XPointer compose_ptr;	/* state table pointer */
    int chars_matched;		/* match state */
} XComposeStatus;

/* events --- goes on for a bit */
typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int keycode;	/* detail */
	BoolDef same_screen;	/* same screen flag */
} XKeyEvent;

typedef XKeyEvent XKeyPressedEvent;
typedef XKeyEvent XKeyReleasedEvent;

typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int button;	/* detail */
	BoolDef same_screen;	/* same screen flag */
} XButtonEvent;
typedef XButtonEvent XButtonPressedEvent;
typedef XButtonEvent XButtonReleasedEvent;

typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	char is_hint;		/* detail */
	BoolDef same_screen;	/* same screen flag */
} XMotionEvent;
typedef XMotionEvent XPointerMovedEvent;

typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time;		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	int detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior, 
	 * NotifyNonlinear,NotifyNonlinearVirtual
	 */
	BoolDef same_screen;	/* same screen flag */
	BoolDef focus;		/* boolean focus */
	unsigned int state;	/* key or button mask */
} XCrossingEvent;
typedef XCrossingEvent XEnterWindowEvent;
typedef XCrossingEvent XLeaveWindowEvent;

typedef struct {
	int type;		/* FocusIn or FocusOut */
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;		/* window of event */
	int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	int detail;
	/*
	 * NotifyAncestor, NotifyVirtual, NotifyInferior, 
	 * NotifyNonlinear,NotifyNonlinearVirtual, NotifyPointer,
	 * NotifyPointerRoot, NotifyDetailNone 
	 */
} XFocusChangeEvent;
typedef XFocusChangeEvent XFocusInEvent;
typedef XFocusChangeEvent XFocusOutEvent;

/* generated on EnterWindow and FocusIn  when KeyMapState selected */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	char key_vector[32];
} XKeymapEvent;	

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
} XExposeEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Drawable drawable;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XGraphicsExposeEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Drawable drawable;
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XNoExposeEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int state;		/* Visibility state */
} XVisibilityEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;		/* parent of the window */
	Window window;		/* window id of window created */
	int x, y;		/* window location */
	int width, height;	/* size of window */
	int border_width;	/* border width */
	BoolDef override_redirect;	/* creation should be overridden */
} XCreateWindowEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
} XDestroyWindowEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	BoolDef from_configure;
} XUnmapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	BoolDef override_redirect;	/* boolean, is override set... */
} XMapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
} XMapRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	Window parent;
	int x, y;
	BoolDef override_redirect;
} XReparentEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int x, y;
	int width, height;
	int border_width;
	Window above;
	BoolDef override_redirect;
} XConfigureEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int x, y;
} XGravityEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int width, height;
} XResizeRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
	int x, y;
	int width, height;
	int border_width;
	Window above;
	int detail;		/* Above, Below, TopIf, BottomIf, Opposite */
	unsigned long value_mask;
} XConfigureRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom atom;
	Time time;
	int state;		/* NewValue, Deleted */
} XPropertyEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom selection;
	Time time;
} XSelectionClearEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window owner;
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;
	Time time;
} XSelectionRequestEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;		/* ATOM or None */
	Time time;
} XSelectionEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Colormap colormap;	/* COLORMAP or None */
#if defined(__cplusplus) || defined(c_plusplus)
	BoolDef c_new;		/* C++ */
#else
	BoolDef new;
#endif
	int state;		/* ColormapInstalled, ColormapUninstalled */
} XColormapEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom message_type;
	int format;
	union {
		char b[20];
		short s[10];
		long l[5];
		} data;
} XClientMessageEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;		/* unused */
	int request;		/* one of MappingModifier, MappingKeyboard,
				   MappingPointer */
	int first_keycode;	/* first keycode */
	int count;		/* defines range of change w. first_keycode*/
} XMappingEvent;

typedef struct {
	int type;
	Display *display;	/* Display the event was read from */
	XID resourceid;		/* resource id */
	unsigned long serial;	/* serial number of failed request */
	unsigned char error_code;	/* error code of failed request */
	unsigned char request_code;	/* Major op-code of failed request */
	unsigned char minor_code;	/* Minor op-code of failed request */
} XErrorEvent;

typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	BoolDef send_event;	/* true if this came from a SendEvent request */
	Display *display;/* Display the event was read from */
	Window window;	/* window on which event was requested in event mask */
} XAnyEvent;

/*
 * this union is defined so Xlib can always use the same sized
 * event structure internally, to avoid memory fragmentation.
 */
typedef union _XEvent {
        int type;		/* must not be changed; first element */
	XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclient;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
	long pad[24];
} XEvent;

typedef int (*XErrorHandler) (	    /* WARNING, this type not in Xlib spec */
    Display*		/* display */,
    XErrorEvent*	/* error_event */
);


typedef struct {
    Atom name;
    unsigned long card32;
} XFontProp;

typedef struct {
    XExtData	*ext_data;	/* hook for extension to hang data */
    Font        fid;            /* Font id for this font */
    unsigned	direction;	/* hint about direction the font is painted */
    unsigned	min_char_or_byte2;/* first character */
    unsigned	max_char_or_byte2;/* last character */
    unsigned	min_byte1;	/* first row that exists */
    unsigned	max_byte1;	/* last row that exists */
    BoolDef all_chars_exist;/* flag if all characters have non-zero size*/
    unsigned	default_char;	/* char to print for undefined character */
    int         n_properties;   /* how many properties there are */
    XFontProp	*properties;	/* pointer to array of additional properties*/
    XCharStruct	min_bounds;	/* minimum bounds over all existing char*/
    XCharStruct	max_bounds;	/* maximum bounds over all existing char*/
    XCharStruct	*per_char;	/* first_char to last_char information */
    int		ascent;		/* log. extent above baseline for spacing */
    int		descent;	/* log. descent below baseline for spacing */
} XFontStruct;

typedef enum {
    XStringStyle,		/* STRING */
    XCompoundTextStyle,		/* COMPOUND_TEXT */
    XTextStyle,			/* text in owner's encoding (current locale)*/
    XStdICCTextStyle,		/* STRING, else COMPOUND_TEXT */
    XUTF8StringStyle		/* UTF8_STRING */
} XICCEncodingStyle;

typedef struct _XIM *XIM;
typedef struct _XIC *XIC;

/*
 * Data structure for "image" data, used by image manipulation routines.
 */
typedef struct _XImage {
    int width, height;		/* size of image */
    int xoffset;		/* number of pixels offset in X direction */
    int format;			/* XYBitmap, XYPixmap, ZPixmap */
    char *data;			/* pointer to image data */
    int byte_order;		/* data byte order, LSBFirst, MSBFirst */
    int bitmap_unit;		/* quant. of scanline 8, 16, 32 */
    int bitmap_bit_order;	/* LSBFirst, MSBFirst */
    int bitmap_pad;		/* 8, 16, 32 either XY or ZPixmap */
    int depth;			/* depth of image */
    int bytes_per_line;		/* accelarator to next line */
    int bits_per_pixel;		/* bits per pixel (ZPixmap) */
    unsigned long red_mask;	/* bits in z arrangment */
    unsigned long green_mask;
    unsigned long blue_mask;
    XPointer obdata;		/* hook for the object routines to hang on */
    struct funcs {		/* image manipulation routines */
	struct _XImage *(*create_image)(
		struct _XDisplay* /* display */,
		Visual*		/* visual */,
		unsigned int	/* depth */,
		int		/* format */,
		int		/* offset */,
		char*		/* data */,
		unsigned int	/* width */,
		unsigned int	/* height */,
		int		/* bitmap_pad */,
		int		/* bytes_per_line */);
	int (*destroy_image)        (struct _XImage *);
	unsigned long (*get_pixel)  (struct _XImage *, int, int);
	int (*put_pixel)            (struct _XImage *, int, int, unsigned long);
	struct _XImage *(*sub_image)(struct _XImage *, int, int, unsigned int, unsigned int);
	int (*add_pixel)            (struct _XImage *, long);
	} f;
} XImage;

typedef struct {
 	int max_keypermod;	/* The server's max # of keys per modifier */
 	KeyCode *modifiermap;	/* An 8 by max_keypermod array of modifiers */
} XModifierKeymap;

typedef struct {
    short x, y;
} XPoint;

typedef struct {
    short x, y;
    unsigned short width, height;
} XRectangle;

typedef struct {
    short x1, y1, x2, y2;
} XSegment;

typedef struct {
    Pixmap background_pixmap;	/* background or None or ParentRelative */
    unsigned long background_pixel;	/* background pixel */
    Pixmap border_pixmap;	/* border of the window */
    unsigned long border_pixel;	/* border pixel value */
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preseved if possible */
    unsigned long backing_pixel;/* value to use in restoring planes */
    BoolDef save_under;		/* should bits under be saved? (popups) */
    long event_mask;		/* set of events that should be saved */
    long do_not_propagate_mask;	/* set of events that should not propagate */
    BoolDef override_redirect;	/* boolean value for override-redirect */
    Colormap colormap;		/* color map to be associated with window */
    Cursor cursor;		/* cursor to be displayed (or None) */
} XSetWindowAttributes;

typedef struct {
	long flags;	/* marks which fields in this structure are defined */
	int x, y;		/* obsolete for new window mgrs, but clients */
	int width, height;	/* should set so old wm's don't mess up */
	int min_width, min_height;
	int max_width, max_height;
    	int width_inc, height_inc;
	struct {
		int x;	/* numerator */
		int y;	/* denominator */
	} min_aspect, max_aspect;
	int base_width, base_height;		/* added by ICCCM version 1 */
	int win_gravity;			/* added by ICCCM version 1 */
} XSizeHints;

typedef struct {
	Colormap colormap;
	unsigned long red_max;
	unsigned long red_mult;
	unsigned long green_max;
	unsigned long green_mult;
	unsigned long blue_max;
	unsigned long blue_mult;
	unsigned long base_pixel;
	VisualID visualid;		/* added by ICCCM version 1 */
	XID killid;			/* added by ICCCM version 1 */
} XStandardColormap;

typedef struct {
    unsigned char *value;		/* same as Property routines */
    Atom encoding;			/* prop type */
    int format;				/* prop data format: 8, 16, or 32 */
    unsigned long nitems;		/* number of data items in value */
} XTextProperty;

typedef struct {
  Visual *visual;
  VisualID visualid;
  int screen;
  int depth;
#if defined(__cplusplus) || defined(c_plusplus)
  int c_class;					/* C++ */
#else
  int class;
#endif
  unsigned long red_mask;
  unsigned long green_mask;
  unsigned long blue_mask;
  int colormap_size;
  int bits_per_rgb;
} XVisualInfo;

typedef struct {
	long flags;	/* marks which fields in this structure are defined */
	BoolDef input;	/* does this application rely on the window manager to
			get keyboard input? */
	int initial_state;	/* see below */
	Pixmap icon_pixmap;	/* pixmap to be used as icon */
	Window icon_window; 	/* window to be used as icon */
	int icon_x, icon_y; 	/* initial position of icon */
	Pixmap icon_mask;	/* icon mask bitmap */
	XID window_group;	/* id of related window group */
	/* this structure may be extended in the future */
} XWMHints;

typedef struct {
    int x, y;			/* location of window */
    int width, height;		/* width and height of window */
    int border_width;		/* border width of window */
    int depth;          	/* depth of window */
    Visual *visual;		/* the associated visual structure */
    Window root;        	/* root of screen containing window */
#if defined(__cplusplus) || defined(c_plusplus)
    int c_class;		/* C++ InputOutput, InputOnly*/
#else
    int class;			/* InputOutput, InputOnly*/
#endif
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preserved if possible */
    unsigned long backing_pixel;/* value to be used when restoring planes */
    BoolDef save_under;		/* boolean, should bits under be saved? */
    Colormap colormap;		/* color map to be associated with window */
    BoolDef map_installed;		/* boolean, is color map currently installed*/
    int map_state;		/* IsUnmapped, IsUnviewable, IsViewable */
    long all_event_masks;	/* set of events all people have interest in*/
    long your_event_mask;	/* my event mask */
    long do_not_propagate_mask; /* set of events that should not propagate */
    BoolDef override_redirect;	/* boolean value for override-redirect */
    Screen *screen;		/* back pointer to correct screen */
} XWindowAttributes;

struct _XrmHashBucketRec;

/* needed for xdefaults.c */
typedef struct _XrmHashBucketRec *XrmDatabase;
typedef enum {XrmBindTightly, XrmBindLoosely} XrmBinding, *XrmBindingList;
typedef int       XrmQuark, *XrmQuarkList;
#define NULLQUARK ((XrmQuark) 0)
typedef XrmQuark     XrmRepresentation;
typedef struct {
    unsigned int    size;
    XPointer	    addr;
} XrmValue, *XrmValuePtr;

/* extend for defining an event callback */
#define USING_W11LIB
typedef void (proto_W11EventHandler)(XEvent *ev);
void W11AddEventHandler(Display *display, proto_W11EventHandler *ev);

/* functions */

Display *XOpenDisplay(const char *name);

int XCloseDisplay(Display *display);

char *XDisplayString(Display *display);

int XSync(Display *display,int discard);

int XFlush(Display *display);

XVisualInfo *XGetVisualInfo(
			    Display *display,
			    long vinm,
			    XVisualInfo *vint,
			    int *n);

StatusDef XMatchVisualInfo(
    Display*		display,
    int			screen,
    int			depth,
    int			class,
    XVisualInfo*	vinfo_return);

int XClearWindow(Display *display, Window w);

Window XCreateSimpleWindow(
		    Display *display,
		    Window  parent,
		    int     x, 
		    int y,
		    unsigned int brd,
		    unsigned int w,
		    unsigned int h,
		    unsigned long bg, 
		    unsigned long brd_col);

Window XCreateWindow(
	      Display *display,
	      Window  parent,
	      int x,
	      int y,
	      unsigned int width,
	      unsigned int height,
	      unsigned int bw,
	      int depth,
	      unsigned int class,
	      Visual *visual,
	      unsigned long valuemask,
	      XSetWindowAttributes *attr);

int XDestroyWindow(
	       Display *display,
	       Window w);

StatusDef XGetGeometry(
	     Display *display,
	     Drawable w,
	     Window *root,
	     int *x,
	     int *y,
	     unsigned int *width,
	     unsigned int *height,
	     unsigned int *bw,
	     unsigned int *depth);

StatusDef XGetWindowAttributes(Display *display,
		     Window w,
		     XWindowAttributes *wattr);

int XSelectInput(Display *display,
	     Window  window,
	     long    mask);

int XMapWindow(Display *display,
	   Window window);

int XIconifyWindow(Display *display,
	       Window w,
	       int screen_number);
GC XCreateGC(
	  Display *display,
	  Drawable window,
	  unsigned long mask,
	  XGCValues *gc_values);

int XFreeGC(
	Display *display,
	GC gc);

int XSetForeground(
	       Display *display,
	       GC gc,
	       unsigned long    color);
int XDrawString(Display *display, 
	    Drawable window,
	    GC gc, 
	    int x, 
	    int y, 
	    const char* str, 
	    int len);
int XDrawString16(Display *display, 
	      Drawable window,
	      GC gc, int x, int y,
	      const XChar2b* str,
	      int len);

int XDrawImageString(
	Display* display,
	Drawable d,
	GC gc,
	int x,
	int y,
	const char* string,
	int length);

int XDrawImageString16(Display *display, 
		   Drawable window,
		   GC gc, int x, int y,
		   const XChar2b* str,
		   int len);

int XFillRectangle(
	       Display *display,
	       Drawable window,
	       GC gc,
	       int x, int y,
	       unsigned int w, unsigned int h);
int XClearArea(
	   Display *display,
	   Window w,
	   int x, int y,
	   unsigned int width, unsigned int height,
	   BoolDef exposures);

Region XCreateRegion();

int XClipBox(
	 Region hrgn,
	 XRectangle *rect);

int XSetRegion(
	   Display *display,
	   GC gc,
	   Region hrgn);

int XDestroyRegion(Region hrgn);

int XUnionRectWithRegion(XRectangle *rect,
		     Region hrgnsrc,
		     Region hrgndest);
int XDrawArc(
	 Display *display,
	 Drawable w,
	 GC gc,
	 int x, int y,
	 unsigned int width,unsigned int height,
	 int a1, int a2);

int XFillArc(
	 Display *display,
	 Drawable w,
	 GC gc,
	 int x, int y,
	 unsigned int width,unsigned height,
	 int a1, int a2);

int XFillPolygon(
	     Display *display,
	     Drawable w,
	     GC gc,
	     XPoint *points,
	     int nps, int shape, int mode);

int XDrawLine(
	  Display *display,
	  Drawable w,
	  GC gc,
	  int x1,int y1, int x2,int y2);

int XDrawLines(
	   Display *display,
	   Drawable w,
	   GC gc,
	   XPoint *points,
	   int nps,int mode);

int XDrawPoints(
	    Display *display,
	    Drawable w,
	    GC gc,
	    XPoint *points,
	    int nps, int mode);

int XDrawPoint(
	   Display *display,
	   Drawable w,
	   GC gc,
	   int x, int y);
int XDrawRectangle(
	       Display *display,
	       Drawable w,
	       GC gc,
	       int x, int y,
	       unsigned int width, unsigned int height);

int XDrawSegments(
	      Display *display,
	      Drawable w,
	      GC gc,
	      XSegment *segs,
	      int nsegs);

Pixmap XCreatePixmap(
	      Display *display,
	      Drawable drawable,
	      unsigned int width, unsigned int height,
	      unsigned int depth);

Pixmap XCreateBitmapFromData(Display *display,
		      Drawable drawable, const char *data,
		      unsigned int width, unsigned int height);
int XFreePixmap(
	 Display *display,
	 Pixmap pixmap);

int XCopyArea(
	  Display *display,
	  Drawable src,
	  Drawable dest,
	  GC gc,
	  int src_x, int src_y,
	  unsigned int width, unsigned int height,
	  int dest_x, int dest_y);

XImage *XGetImage(
		  Display *display,
		  Drawable drawable,
		  int x, int y,
		  unsigned int width, unsigned int height,
		  unsigned long plane_mask,
		  int format);

XImage *XCreateImage(
	     Display *display,
	     Visual *visual,
	     unsigned int depth,
	     int format,
	     int offset,
	     char *data,
	     unsigned int width, unsigned int height,
	     int bitmap_pad, int bytes_per_line);
int XPutImage(
	  Display *display,
	  Drawable w,
	  GC gc,
	  XImage *image,
	  int sx,int sy,int dx,int dy,
	  unsigned int width,unsigned int height);

int XSetWindowBackground(
		     Display *display,
		     Window w,
		     unsigned long bg);

int XSetWindowBackgroundPixmap(
			   Display *display,
			   Window w,
			   Pixmap background_tile);

int XSetFillStyle(
	      Display *display,
	      GC gc,
	      int fs);

int XSetDashes(Display *display,
	   GC gc, int dash_offset,
	   const char * dash_list,
	   int n);

int XChangeWindowAttributes(
			Display *display,
			Window w,
			unsigned long vmask,
			XSetWindowAttributes *attr);

int XLowerWindow(Display *display,
	     Window w);

int XMapRaised(
	   Display *display,
	   Window w);

int
XMapSubwindows(
	       Display *display,
	       Window w);

StatusDef
XQueryTree(
	   Display *display,
	   Window w,
	   Window* root,
	   Window* parent,
	   Window** ch,
	   unsigned int *n);
int
XRaiseWindow(
	     Display *display,
	     Window w);

Window
XRootWindow(
	    Display *display,
	    int scr);
Window
XRootWindowOfScreen(Screen *scr);

BoolDef XTranslateCoordinates(
		      Display *display,
		      Window sw, Window dw,
		      int sx, int sy, int *dx, int *dy,
		      Window *ch);

int
XUnmapWindow(Display *display,
	     Window w);
int
XCopyGC(
	Display *display,
	GC sgc,
	unsigned long vmask,
	GC dgc);
int
XSetClipMask(
	     Display *display,
	     GC gc,
	     Pixmap cmask);

int
XSetClipRectangles(
		   Display *display,
		   GC gc,
		   int clx, int cly,
		   XRectangle *rs,
		   int n, int order);
int
XSetFunction(
	     Display *display,
	     GC gc,
	     int fn);
int
XSetLineAttributes(
		   Display *display,
		   GC gc,
		   unsigned int lw,
		   int ls, int cs, int js);
int
XSetPlaneMask(
	      Display *display,
	      GC gc,
	      unsigned long pmask);
int XSetTile(
	     Display *display,
	     GC gc,
	     Pixmap tile);
StatusDef
XAllocColorCells(
		 Display *display,
		 Colormap cmap,
		 BoolDef cont,
		 unsigned long *pmasks,
		 unsigned int np,
		 unsigned long *pixels,
		 unsigned int nc);
StatusDef
XAllocColorPlanes(
		  Display *display,
		  Colormap cmap,
		  BoolDef cont,
		  unsigned long *pixels,
		  int nc,
		  int nr,int ng,int nb,
		  unsigned long *rmask,
		  unsigned long *gmask,
		  unsigned long *bmask);
StatusDef
XAllocNamedColor(Display *display,
		 Colormap cmap, const char *cname,
		 XColor *cell, XColor *rgb);
Colormap
XCreateColormap(
		Display *display,
		Window w,
		Visual *visual,
		int alloc);
StatusDef
XGetStandardColormap(
		     Display *display,
		     Window w,
		     XStandardColormap *cmapinf,
		     Atom prop);
StatusDef
XAllocColor(
	    Display *display,
	    Colormap cmap,
	    XColor *xc);
int
XQueryColor(
	    Display *display,
	    Colormap cmap,
	    XColor *cell);
int
XQueryColors(
	     Display *display,
	     Colormap cmap,
	     XColor *cells,
	     int nc);
int
XStoreColor(
	    Display *display,
	    Colormap cmap,
	    XColor *cell);
int
XStoreColors(
	     Display *display,
	     Colormap cmap,
	     XColor *cells,
	     int nc);
char **
XGetFontPath(
	     Display *display,
	     int *nps);
BoolDef XGetFontProperty(
		 XFontStruct *fstruct,
		 Atom atom,
		 unsigned long *val);

XFontStruct *
XLoadQueryFont(Display *display, const char *name);

XFontStruct *
XQueryFont(
	   Display *display,
	   XID     font_id);

KeySym
XKeycodeToKeysym(
		 Display *display,
		 unsigned int keycode,
		 int     index);

KeyCode
XKeysymToKeycode(
		 Display *display,
		 KeySym keysym);

KeySym
XStringToKeysym(const char *str);

XModifierKeymap *
XGetModifierMapping(Display *display);

int
XFreeModifiermap(XModifierKeymap *modmap);

int
XSetFont(
	 Display *display,
	 GC gc,
	 Font font);
int
XSetFontPath(
	     Display *display,
	     char **dirs,
	     int nd);
int
XTextExtents(
	     XFontStruct *fstruct,
	     const char *str,
	     int nc,
	     int *dir,int *ascent,int *descent,
	     XCharStruct *overall);
int
XTextExtents16(
	       XFontStruct *fstruct,
	       const XChar2b *str,
	       int nc,
	       int *dir, int *ascent, int *descent,
	       XCharStruct *overall);

int
XTextWidth(
	   XFontStruct *fstruct,
	   const char *str,
	   int co);
int
XTextWidth16(
	     XFontStruct *fstruct,
	     const XChar2b *str,
	     int co);
int
XGetErrorDatabaseText(
		      Display *display,
		      const char *name, const char *msg,
		      const char *defstr,
		      char *buf,
		      int len);

int
XGetErrorText(
	      Display *display,
	      int code,
	      char *buf,
	      int len);

XErrorHandler
XSetErrorHandler(XErrorHandler handler);

int
XDefaultScreen(Display *display);

Visual *
XDefaultVisual(
	       Display *display,
	       int screen);

int
XDefaultDepth(
	      Display *display,
	      int screen);

Colormap
XDefaultColormap(
		 Display *display,
		 int screen);
Screen *
XScreenOfDisplay(
		 Display *display,
		 int scr);
Cursor
XCreateFontCursor(
		  Display *display,
		  unsigned int shape);
int
XRecolorCursor(
	       Display *display,
	       Cursor cursor,
	       XColor *fg,XColor *bg);

int
XWarpPointer(
	     Display *display,
	     Window sw,Window dw,
	     int sx,int sy,
	     unsigned int swidth,unsigned int sheight,
	     int dx, int dy);
int
XBell(
      Display *display,
      int pc);

int
XGetInputFocus(
	       Display *display,
	       Window *focus,
	       int *revto);
int
XSetInputFocus(
	       Display *display,
	       Window focus,
	       int revto,
	       Time time);
int
XLookupString(
	      XKeyEvent *event,
	      char *buf,
	      int nbytes,
	      KeySym *keysym,
	      XComposeStatus *status);

int
XRefreshKeyboardMapping(XMappingEvent *event);

int
XSetClassHint(
	      Display *display,
	      Window w,
	      XClassHint *chints);
int
XSetNormalHints(
		Display *display,
		Window w,
		XSizeHints *hints);

int
XSetWMHints(
	    Display *display,
	    Window w,
	    XWMHints *wmhints);
StatusDef
XSetWMProtocols(
		Display *display,
		Window w,
		Atom *prots,
		int co);
int
XStoreName(Display *display,
	   Window w,
	   const char *wname);
StatusDef
XFetchName(
    Display *display,
    Window w,
    char **window_name_return);

int
XDoesBackingStore(Screen *scr);

XExtCodes *
XInitExtension(Display *display,
	       const char *name);
int
XFree(void *data);

char *
XServerVendor(Display *display);

int
XSetIconName(Display *display,
	     Window w,
	     const char *iname);
int
XGetIconName(
	     Display *display,
	     Window w,
	     char **iname);
int
XSetSelectionOwner(
		   Display* display,
		   Atom sel,
		   Window owner,
		   Time time);
Window
XGetSelectionOwner(
		   Display* display,
		   Atom selection);

int
XConvertSelection(
		  Display *display,
		  Atom sel, Atom target, Atom prop,
		  Window req,
		  Time time);
BoolDef XCheckTypedEvent(
		 Display *display,
		 int ev,
		 XEvent *rep);
BoolDef XCheckWindowEvent(
		  Display *display,
		  Window w,
		  long emask,
		  XEvent *ev);
int
XPending(Display *display);

int
XPutBackEvent(Display *display,
	      XEvent *event);

StatusDef
XSendEvent(
	   Display *display,
	   Window w,
	   BoolDef prop,
	   long emask,
	   XEvent *event);

BoolDef XCheckTypedWindowEvent(
	Display* display,
	Window w,
	int event_type,
	XEvent* event_return);
int
XWindowEvent(
	     Display *display,
	     Window w,
	     long emask,
	     XEvent *rep);

int
XNextEvent(
	   Display *display,
	   XEvent  *event);

Atom
XInternAtom(
	    Display *display,
	    const char *property_name,
	    BoolDef only_if_exists);
char *
XGetAtomName(
	     Display *display,
	     Atom atom);

int
XChangeProperty(
	Display *display,
	Window window,
	Atom property,
	Atom type,
	int format,
	int mode,
	const unsigned char *data,
	int nelements);

int
XGetWindowProperty(
		   Display *display,
		   Window window,
		   Atom property,
		   long long_offset,
		   long long_length,
		   BoolDef delete,
		   Atom req_type,
		   Atom *actual_type_return,
		   int *actual_format_return,
		   unsigned long *nitems_return,
		   unsigned long *bytes_after_return,
		   unsigned char **prop_return);

char **
XListExtensions(
		Display *display,
		int *ret_num);

int XFreeExtensionList(char **list);

int
XChangeGC(
	Display* display,
	GC gc,
	unsigned long mask,
	XGCValues* gc_values);

int
XConnectionNumber(Display* display);

int
XFreeFont(Display* display,XFontStruct* font_struct);

char *
XSetLocaleModifiers(const char* modifier_list);

XIM
XOpenIM(
	Display* dpy,
	struct _XrmHashBucketRec* rdb,
	char* res_name,
	char* res_class);

char *
XGetIMValues(XIM im , ...);

XIC XCreateIC(XIM im , ...);

StatusDef
XCloseIM(XIM im);

BoolDef XFilterEvent(XEvent* event,Window window);

char *
XrmQuarkToString(void *quark);

int
XmbLookupString(
	XIC ic,
	XKeyPressedEvent* event,
	char* buffer_return,
	int bytes_buffer,
	KeySym* keysym_return,
	StatusDef* status_return);
int
XmbTextPropertyToTextList(
	Display *display,
	XTextProperty *text_prop,
	char ***list_return,
	int *count_return);

void
XFreeStringList(char **list);

int XmbTextListToTextProperty(
	 Display *display,
	 char **list,
	 int count,
	 XICCEncodingStyle style,
	 XTextProperty *text_prop_return);

void
XSetICFocus(XIC ic);

void
XUnsetICFocus(XIC ic);

BoolDef XQueryPointer(
	Display* display,
	Window w,
	Window* root_return,
	Window* child_return,
	int* root_x_return,
	int* root_y_return,
	int* win_x_return,
	int* win_y_return,
	unsigned int* mask_return);

int XParseGeometry(
	const char* string,
	int* x,
	int* y,
	unsigned int* width,
	unsigned int* height);

int XResizeWindow(
	Display* display,
	Window w,
	unsigned int width,
	unsigned int height);

void XSetWMNormalHints(Display* display,Window w,XSizeHints* hints);

void XSetWMProperties(
	Display* display,
	Window w,
	XTextProperty* window_name,
	XTextProperty* icon_name,
	char** argv,
	int argc,
	XSizeHints* normal_hints,
	XWMHints* wm_hints,
	XClassHint* class_hints);

int XDefineCursor(Display* display,Window w,Cursor cursor);

int XMoveResizeWindow(
	Display* display,
	Window w,
	int x,
	int y,
	unsigned int width,
	unsigned int height);

int XMoveWindow(
	Display* display,
	Window w,
	int x,
	int y);

StatusDef
XParseColor(
    Display *display,
    Colormap map,
    const char* spec,
    XColor *colorPtr);

int
XFreeColors(Display *display,
	    Colormap cmap,
	    unsigned long pixels[],
	    int npixels,
	    unsigned long planes);
int
XGrabServer(Display *display);

int
XUngrabServer(Display *display);


#endif
