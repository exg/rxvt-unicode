
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include "ntdef.h"

/* a crude method to avoid losing the selection when
   calling EmptyClipboard, which is necessary to do
   every time the selection content changes, otherwise
   windows apps use a cached copy of the selection */
static volatile int destroyClipboardCatcher = 0;
static NT_window *NT_CWIN = NULL;
void
catchNextDestroyClipboard()
{
    destroyClipboardCatcher=1;
}

typedef struct WinEvent_
{
    NT_window *window;
    UINT message;
    UINT wParam;
    LONG lParam;
} WinEvent;

#define W11_QSIZE 100
typedef struct WinEventQ_
{
    int num;
    int avail;
    int next;
    int count;
    int dispatch;
    proto_W11EventHandler *handler;
    WinEvent list[W11_QSIZE];
} WinEventQ;

static WinEventQ *wineventq = NULL;

void
initQ() {
    int i;
    WinEventQ *q = (WinEventQ *)allocateMemory(sizeof(WinEventQ));
    q->num=W11_QSIZE-1;
    q->avail=0;
    q->next=0;
    q->count=0;
    q->dispatch=0;
    q->handler=NULL;
    for (i=0; i<W11_QSIZE; i++) {
	q->list[i].message=0;
	q->list[i].window = NULL;
    }
    wineventq = q;
}

static int
getQdEvent(WinEventQ *q, XEvent *ev)
{
    WinEvent we;
    if (q->count<=0) {
	cjh_printf("Queue is empty\n");
	return 0;
    }
    we = q->list[q->next];
    WinEventToXEvent(&we,ev);
    q->next++;
    q->count--;
    if (q->next>q->num) q->next=0;	
    return 1;
}

static void
QSendEvent(WinEventQ *q) 
{
    XEvent e;
    if (q->handler != NULL) {
	if (getQdEvent(q,&e)) (q->handler)(&e);
    }
}

static int
QEvent(WinEventQ *q, NT_window *window,UINT message,UINT wParam,LONG lParam)
{
    q->list[q->avail].window=window;
    q->list[q->avail].message=message;
    q->list[q->avail].wParam=wParam;
    q->list[q->avail].lParam=lParam;
    q->avail++; q->count++;
    if (q->avail>q->num) q->avail=0;
    if (q->dispatch) QSendEvent(q);
    return 1;
}


/* Allow application to install an event handler call back.
   This will make some actions such as moving the window work
   better.

   The event handler should look like:
   void process_xevent(XEvent *ev) { }

   To install it:
   W11AddEventHandler(display,process_xevent);

   The specific problem is that calling DefWindowProc()
   in response to a WM_SYSCOMMAND will cause windows to run its
   own event loop waiting for the mouse up event.  The application
   therefore cannot rely on it's main event loop to get run for
   each event.  Without running multiple threads, or setjmp, there
   is little recourse for alerting the application in the
   traditional X manner to Expose events while the window is
   being moved.
*/

void W11AddEventHandler(Display *d, proto_W11EventHandler *ev) 
{
    wineventq->handler = ev;
}


static void
doTranslateMessage(MSG *m)
{
	if ((m->message == WM_KEYDOWN) && 
	    ((m->wParam == VK_BACK) ||
	     (((m->wParam == VK_ADD) || (m->wParam == VK_SUBTRACT)) &&
	      (GetKeyState(VK_SHIFT) & 0x8000)))) return;
	if ((m->message == WM_SYSKEYDOWN) && (m->wParam == VK_F10))
	{
	    m->message = WM_KEYDOWN;
	    return;
	}
	TranslateMessage(m);
}

static LONG
NT_default(HWND hWnd,UINT message,UINT wParam,LONG lParam)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static void
NT_wakeup(HWND hWnd) 
{
    PostMessage(hWnd,USR_WakeUp,0,0L);
}

/*****************************************************************\

	Function: MainWndProc
	Inputs:   Window handle, message, message parameters.

	Comments: This is called when messages are sent to the application
		  but not to the application's message queue.  If an
		  event can be processed, it is done here.  The equivalent
		  XEvent is filled out in l_event, which is picked up
		  in the X event routines.  Some events are not received
		  from Windows, eg Enter/LeaveNotify, so these are made
		  up as required.

	Caution:  The application does not see HWND, but Window.

\*****************************************************************/

/* queued messages
   WM_KEYDOWN
   WM_KEYUP
   WM_CHAR
   WM_MOUSEMOVE
   WM_BUTTONxx
   WM_TIMER
   WM_PAINT
   WM_QUIT
   */

LONG NT_handleMsg(
    HWND hWnd,		  /* window handle		     */
    UINT message,		  /* type of message		     */
    UINT wParam,		  /* additional information	     */
    LONG lParam)		  /* additional information	     */
{
    RECT rect;
    WINDOWPOS *posStruct;
    unsigned long int st=0L;
    NT_window *window;
    long mask;
    PAINTSTRUCT paintStruct;

    /*	if (message == WM_CLOSE) exit(0); */
	
    window = NT_find_window_from_id(hWnd);
    if (window == NULL) return  (NT_default(hWnd, message, wParam, lParam));

    mask = window->mask;
	
    switch (message) {
	/* we'll handle these, later */
	case WM_KILLFOCUS:
	    QEvent(wineventq,window,message,wParam,lParam);
	    NT_wakeup(hWnd);
	    break;
	case WM_SETFOCUS:
	case WM_QUIT:
	case WM_CLOSE:
	case WM_DESTROY:
	case WM_SYSCHAR: /* alt-keys go here */
	case WM_CHAR:
	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
	case USR_MapNotify:
	case USR_EnterNotify:
	case WM_MOVE:
#if defined(WIN9X)
	case WM_SIZING:
#endif			
	case WM_SIZE:			
	    QEvent(wineventq,window,message,wParam,lParam);
	    break;
	case WM_DESTROYCLIPBOARD:
	    if (destroyClipboardCatcher)
		destroyClipboardCatcher=0;
	    else {
		QEvent(wineventq,window,message,wParam,lParam);
		NT_wakeup(hWnd);
            }
	    break;
	case WM_PAINT:
	    BeginPaint(hWnd,&paintStruct);
	    FillRect(paintStruct.hdc, &paintStruct.rcPaint,window->bg);
	    QEvent(wineventq,window,message,
		   (((paintStruct.rcPaint.right-paintStruct.rcPaint.left)&0xffff) |
		    (((paintStruct.rcPaint.bottom-paintStruct.rcPaint.top)&0xffff)<<16)),
		   (((paintStruct.rcPaint.left)&0xffff) | (((paintStruct.rcPaint.top)&0xffff)<<16)));

	    EndPaint(hWnd,&paintStruct);
	    break;
	    /* capture the mouse on button down to emulate x */
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	    SetCapture(hWnd);
	    QEvent(wineventq,window,message,wParam,lParam);
	    break;
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	    ReleaseCapture();
	    QEvent(wineventq,window,message,wParam,lParam);
	    break;
	case WM_MOUSEMOVE:
	    if ((mask&PointerMotionMask) ||
		((mask&Button1MotionMask)&& (wParam&MK_LBUTTON)) ||
		((mask&Button2MotionMask)&& (wParam&MK_MBUTTON)) ||
		((mask&Button3MotionMask)&& (wParam&MK_RBUTTON)) ||
		((mask&ButtonMotionMask)&&((wParam&(MK_LBUTTON|MK_MBUTTON|MK_RBUTTON))))
		)
		QEvent(wineventq,window,message,wParam,lParam);
	    else
		return (NT_default(hWnd, message, wParam, lParam));
	    break;
	case WM_MOUSEWHEEL:
	    /* this event only seems to go to the top most window.
	       see if child windows accept it. */
	    window = NT_find_child(window,ButtonPressMask|Button2MotionMask|Button3MotionMask,
				   ButtonPressMask|Button3MotionMask );
	    if (window && ((window->mask)&ButtonPressMask)) 
		QEvent(wineventq,window,message,wParam,lParam);
	    else
		return (NT_default(hWnd, message, wParam, lParam));
	    break;
	case WM_ERASEBKGND:
	    /* don't erase the background */
	    return 1;
	    break;
	case WM_SYSCOMMAND:
	    wineventq->dispatch++;
	    NT_default(hWnd, message, wParam, lParam);
	    wineventq->dispatch--;
	    break;
	case WM_KEYDOWN:
	    switch (wParam)
	    {
		case VK_CANCEL:
		case VK_CLEAR:
		case VK_PAUSE:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_SELECT:
		case VK_PRINT:
		case VK_EXECUTE:
		case VK_INSERT:
		case VK_DELETE:
		case VK_HELP:
		case VK_NUMLOCK:
		case VK_SCROLL:
		case VK_BACK:
                case VK_F1:
                case VK_F2:
                case VK_F3:
                case VK_F4:
                case VK_F5:
                case VK_F6:
                case VK_F7:
                case VK_F8:
                case VK_F9:
                case VK_F10:
                case VK_F11:
	        case VK_F12:
	        case VK_ADD:
	        case VK_SUBTRACT:
		    QEvent(wineventq,window,message,wParam,lParam);
		    break;
		default:
		    return  (NT_default(hWnd, message, wParam, lParam));
		    break;
					
	    }
	    break;
	default:			  /* Passes it on if unproccessed    */
	    return (NT_default(hWnd, message, wParam, lParam));
    }
    return 0L;
}

/*****************************************************************\

	Function: NT_get_state
	Inputs: 

	Comments: Get the keyboard state

\*****************************************************************/

static unsigned int
NT_get_state()
{
	unsigned int state = 0;
	if (GetKeyState(VK_SHIFT)   & 0x8000) state |= ShiftMask;
	if (GetKeyState(VK_CONTROL) & 0x8000)
	{
		if (!(GetKeyState(VK_MENU) & 0x8000)) 
			state |= ControlMask;
	}
	else if (GetKeyState(VK_MENU)    & 0x8000)
		state |= Mod1Mask;
	if (GetKeyState(VK_CAPITAL) & 0x0001) state |= LockMask;
	if (GetKeyState(VK_NUMLOCK) & 0x0001) state |= Mod5Mask;
	if (GetKeyState(VK_SCROLL)  & 0x0001) state |= Mod3Mask;
	if (GetKeyState(VK_LBUTTON) & 0x8000) state |= Button1Mask;
	if (GetKeyState(VK_MBUTTON) & 0x8000) state |= Button2Mask;
	if (GetKeyState(VK_RBUTTON) & 0x8000) state |= Button3Mask;
	return state;
}

int
WinEventToXEvent(
    WinEvent *we,
    XEvent *event)
{
    POINT pt;
    RECT rect;
    unsigned long int st=0L;
    UINT key;
    HWND hWnd;
    UINT wParam;
    LONG lParam;
    NT_window *window;
	
    do {		
	if (event==NULL) break;
	if (we == NULL) break;

	window = we->window;
	wParam = we->wParam;
	lParam = we->lParam;
		
	event->type=-1;
	event->xbutton.subwindow = None;
	hWnd = window->w;
		
	switch (we->message) {
	    case WM_SETFOCUS:
		event->type=FocusIn;
		event->xfocus.window=(Window)window;
		break;
	    case WM_KILLFOCUS:
		event->type=FocusOut;
		event->xfocus.window=(Window)window;
		break;	
				/*	case WM_ERASEBKGND: */
	    case WM_PAINT:
		event->type=Expose;
		event->xexpose.x=LOWORD(lParam); /* right */
		event->xexpose.y=HIWORD(lParam); /* top */
		event->xexpose.width=LOWORD(wParam);
		event->xexpose.height=HIWORD(wParam);
		event->xexpose.count=0;
		event->xexpose.window=(Window)window;
		break;
	    case WM_LBUTTONDOWN:
	    case WM_LBUTTONDBLCLK:
		event->type = ButtonPress;
		event->xbutton.x = LOWORD (lParam);
		event->xbutton.y = HIWORD (lParam);
		if ( wParam & MK_SHIFT )
		    event->xbutton.button=Button2;
		else
		    event->xbutton.button=Button1;
		event->xbutton.window = (Window)window;
		event->xbutton.time=GetTickCount();
		break;
	    case WM_LBUTTONUP:
		event->type=ButtonRelease;
		event->xbutton.x=LOWORD(lParam);
		event->xbutton.y=HIWORD (lParam);
		if ( wParam & MK_SHIFT )
		    event->xbutton.button=Button2;
		else
		    event->xbutton.button=Button1;
		event->xbutton.window=(Window)window;
		event->xbutton.time=GetTickCount();
		break;
	    case WM_MBUTTONDOWN:
	    case WM_MBUTTONDBLCLK:
		event->type=ButtonPress;
		event->xbutton.x=LOWORD(lParam);
		event->xbutton.y=HIWORD (lParam);
		event->xbutton.button=Button2;
		event->xbutton.window=(Window)window;
		event->xbutton.time=GetTickCount();
		break;
	    case WM_MBUTTONUP:
		event->type=ButtonRelease;
		event->xbutton.x=LOWORD(lParam);
		event->xbutton.y=HIWORD (lParam);
		event->xbutton.button=Button2;
		event->xbutton.window=(Window)window;
		event->xbutton.time=GetTickCount();
		break;
	    case WM_RBUTTONDOWN:
	    case WM_RBUTTONDBLCLK:
		event->type=ButtonPress;
		event->xbutton.x=LOWORD(lParam);
		event->xbutton.y=HIWORD (lParam);
		event->xbutton.button=Button3;
		event->xbutton.window=(Window)window;
		event->xbutton.time=GetTickCount();
		break;
	    case WM_RBUTTONUP:
		event->type=ButtonRelease;
		event->xbutton.x=LOWORD(lParam);
		event->xbutton.y=HIWORD (lParam);
		event->xbutton.button=Button3;
		event->xbutton.window=(Window)window;
		event->xbutton.time=GetTickCount();
		break;
	    case WM_MOUSEMOVE:
		if (hWnd!=(HWND)NT_CWIN)  /* Mouse in different window? */
		{
		    if (NT_CWIN==NULL) /* No previous window */
		    {
			event->type = EnterNotify;
			event->xcrossing.x = LOWORD(lParam);
			event->xcrossing.y = HIWORD(lParam);
			event->xcrossing.window = (Window)window;
		    }
		    else
		    {
			event->type=LeaveNotify;
			event->xcrossing.x=LOWORD(lParam);
			event->xcrossing.y=HIWORD(lParam);
			event->xcrossing.window = (Window)NT_find_window_from_id(NT_CWIN);
		    }
		}
		else
		{
		    event->type=MotionNotify;    /* Fill out mouse event */
		    event->xmotion.window=(Window)window;
		    event->xmotion.x=pt.x=LOWORD(lParam);
		    event->xmotion.y=pt.y=HIWORD(lParam);
		    event->xmotion.time=GetTickCount();
		    ClientToScreen(hWnd,&pt);     /* Translate coordinates */
		    event->xmotion.x_root=pt.x;
		    event->xmotion.y_root=pt.y;
		    if (wParam&MK_CONTROL)
			st|=ControlMask;
		    if (wParam&MK_SHIFT)
			st|=ShiftMask;
		    if (wParam&MK_LBUTTON)
			st|=Button1Mask;
		    if (wParam&MK_MBUTTON)
			st|=Button2Mask;
		    if (wParam&MK_RBUTTON)
			st|=Button3Mask;
		    event->xmotion.state=st;
		}
		NT_CWIN=(NT_window *)hWnd;
		break;
	    case WM_MOUSEWHEEL:
		event->type=ButtonRelease;
		event->xbutton.x=LOWORD(lParam);
		event->xbutton.y=HIWORD (lParam);
		event->xbutton.button=HIWORD(wParam)>32768?Button5:Button4;
		event->xbutton.window=(Window)window;
		event->xbutton.time=GetTickCount();
		if (wParam&MK_CONTROL)
		    st|=ControlMask;
		if (wParam&MK_SHIFT)
		    st|=ShiftMask;
		if (wParam&MK_LBUTTON)
		    st|=Button1Mask;
		if (wParam&MK_MBUTTON)
		    st|=Button2Mask;
		if (wParam&MK_RBUTTON)
		    st|=Button3Mask;
		event->xbutton.state=st;
		break;
	    case WM_SYSCHAR:
	    case WM_CHAR:
		event->type=KeyPress;
		event->xkey.state=NT_get_state();
		event->xkey.x=event->xkey.y=0; /* Inside the window */
		event->xkey.keycode=LOWORD(wParam);
		if (GetKeyState(VK_CONTROL) & 0x8000) {
		    if (event->xkey.keycode == 32) { event->xkey.keycode=0; }
		    if (event->xkey.keycode >255 ) { event->xkey.keycode=0; }
		}
		event->xkey.window=(Window)window;
		break;
	    case WM_KEYDOWN:
		event->type=KeyPress;
		switch (wParam)
		{
		    case VK_CANCEL:  key=XK_Cancel;      break;
		    case VK_CLEAR:   key=XK_Clear;       break;
		/*  causes AltGr to create a keypress */
		/*  case VK_MENU:    key=XK_Alt_L;       break;*/
		    case VK_PAUSE:   key=XK_Pause;       break;
		    case VK_PRIOR:   key=XK_Prior;       break;
		    case VK_NEXT:    key=XK_Next;        break;
		    case VK_END:     key=XK_End;         break;
		    case VK_HOME:    key=XK_Home;        break;
		    case VK_LEFT:    key=XK_Left;        break;
		    case VK_UP:      key=XK_Up;          break;
		    case VK_RIGHT:   key=XK_Right;       break;
		    case VK_DOWN:    key=XK_Down;        break;
		    case VK_SELECT:  key=XK_Select;      break;
		    case VK_PRINT:   key=XK_Print;       break;
		    case VK_EXECUTE: key=XK_Execute;     break;
		    case VK_INSERT:  key=XK_Insert;      break;
		    case VK_DELETE:  key=XK_Delete;      break;
		    case VK_HELP:    key=XK_Help;        break;
		    case VK_NUMLOCK: key=XK_Num_Lock;    break;
		    case VK_SCROLL:  key=XK_Scroll_Lock; break;
		    case VK_BACK:    key=XK_BackSpace; break;
                    case VK_F1:      key=XK_F1;          break;
                    case VK_F2:      key=XK_F2;          break;
                    case VK_F3:      key=XK_F3;          break;
                    case VK_F4:      key=XK_F4;          break;
                    case VK_F5:      key=XK_F5;          break;
                    case VK_F6:      key=XK_F6;          break;
                    case VK_F7:      key=XK_F7;          break;
                    case VK_F8:      key=XK_F8;          break;
                    case VK_F9:      key=XK_F9;          break;
                    case VK_F10:     key=XK_F10;         break;
                    case VK_F11:     key=XK_F11;         break;
                    case VK_F12:     key=XK_F12;         break;
		    case VK_ADD:     key=XK_KP_Add;      break;
		    case VK_SUBTRACT:key=XK_KP_Subtract; break;
		    default:         key=0;              break;
		}
		if (key == 0) {
		    event->type = -1;
		}
		else
		{
		    event->xkey.keycode=key;
		    event->xkey.window=(Window)window;
		    event->xkey.state=NT_get_state();
		    event->xkey.x=event->xkey.y=0; /* Inside the window */
		}
		break;
	    case WM_DESTROY:
	    case WM_QUIT:
	    case WM_CLOSE:
		event->type = ClientMessage;
		event->xclient.format = 32;
		event->xclient.data.l[0] = XInternAtom(NULL,"WM_DELETE_WINDOW", FALSE);
		break;
	    case USR_EnterNotify:
		event->type = EnterNotify;
		event->xcrossing.x = LOWORD(lParam);
		event->xcrossing.y = HIWORD(lParam);
		event->xcrossing.window = (Window)window;
		break;
	    case WM_MOVE:
		if (window->min==0)
		{
		    window->x =  LOWORD(lParam);
		    window->y =  HIWORD(lParam);
		    NT_configureNotify(window,window->x,window->y);
		    event->type = ConfigureNotify;
		    event->xconfigure.window = (Window)window;
		    event->xconfigure.x = 0;  /* client area is always @ 0 */
		    event->xconfigure.y = 0;
		    event->xconfigure.width = window->wdth;
		    event->xconfigure.height = window->hght;
		    event->xconfigure.above = Above;
		}
		break;
	    case WM_SIZING:
		event->type = ConfigureNotify;
		window->wdth =	LOWORD(lParam);
		if (window->wdth<window->minx)
		    window->wdth = window->minx;
		window->hght =	HIWORD(lParam);
		if (window->hght<window->minx)
		    window->hght = window->miny;
		NT_configureNotify(window,window->x,window->y);
		event->xconfigure.window = (Window)window;
		event->xconfigure.x = 0;
		event->xconfigure.y = 0;
		event->xconfigure.width = window->wdth;
		event->xconfigure.height = window->hght;
		event->xconfigure.above = Above;
		break;
	    case WM_SIZE:
		switch(wParam)
		{
		    case SIZE_MINIMIZED:
			event->type=UnmapNotify;
			window->min=1;
			break;
		    default:
			event->type = ConfigureNotify;
			window->wdth =  LOWORD(lParam);
			if (window->wdth<window->minx)
			    window->wdth = window->minx;
			window->hght =  HIWORD(lParam);
			if (window->hght<window->minx)
			    window->hght = window->miny;
			event->xconfigure.window = (Window)window;
			event->xconfigure.x = 0;
			event->xconfigure.y = 0;
			event->xconfigure.width = window->wdth;
			event->xconfigure.height = window->hght;
			event->xconfigure.above = Above;
#if !defined(WIN9X)						
			if (window->min) event->type=MapNotify;
#endif						
			window->min=0;
			break;
		}
		break;
	    case WM_DESTROYCLIPBOARD:
		event->type = SelectionClear;
		event->xselectionclear.time = GetTickCount();
		break;
	    case USR_MapNotify:
		event->type=MapNotify;
		break;
	    case USR_ConvertSelection:
		event->type=SelectionNotify;
		event->xselection.requestor = (Window)window;
		event->xselection.property = XA_CUT_BUFFER0;			
		break;
	    default:
		break;
	}
    } while(0);
    return (event==NULL?0: (event->type==-1?0:1));
}
/*****************************************************************\


	Function: XCheckWindowEvent
	Inputs:   display, window, event mask.
	Returned: pointer to filled in event structure, status.

	Comments: This is fudged at the moment to work with the toolkit.
		  The event system needs rewriting to account properly for
		  event masks.

\*****************************************************************/

BoolDef
XCheckTypedEvent(display,ev,rep)
Display *display;
int ev;
XEvent *rep;
{
	xtrace("XCheckTypedEvent\n");
	return (False);
}

BoolDef
XCheckWindowEvent(display,w,emask,ev)
Display *display;
Window w;
long emask;
XEvent *ev;
{
	NT_window *ntw=(NT_window *)w;
	MSG msg;
	BoolDef status = 0;

	xtrace("XCheckWindowEvent\n");
	if (emask&0)
		if (PeekMessage(&msg,ntw->w,USR_MapNotify,
						USR_MapNotify,PM_REMOVE)||
		    PeekMessage(&msg,ntw->w,WM_PAINT,WM_PAINT,PM_REMOVE))
		{
			cjh_printf("removed message\n");
			ev->type=ConfigureNotify;
			status = 1;
		}
	return(status);
}

/*
  XPending checks for x events pending.
  We don't know if we have x events until we process
  the win events.
  */
int
XPending (display)
Display *display;
{
	MSG msg;
	/*	xtrace("XPending\n"); */
	while(wineventq->count<=0 && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		doTranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return wineventq->count;
}

int
XPutBackEvent(display,event)
Display *display;
XEvent *event;
{
	xtrace("XPutBackEvent\n");
	return 0;
}


StatusDef
XSendEvent(display,w,prop,emask,event)
Display *display;
Window w;
BoolDef prop;
long emask;
XEvent *event;
{
	xtrace("XSendEvent\n");
	return 0;
}

/* I'm tempted to flush the windows queue
**   before checking, but I think that would
**   break the assumtion that most of the WM_PAINT
**   messges are only going to be dispatched when
**   the app is directly calling us.
*/

BoolDef
XCheckTypedWindowEvent(
	Display* display,
	Window w,
	int event_type,
	XEvent* event_return)
{
	int i,j;
	xtrace("XCheckTypedWindowEvent\n");
	if (w==0) return 0;
	/*
	i = wineventq->next;
	while(i != wineventq->avail)
	{
		if (wineventq->list[i].window==(NT_window*)w)
		{
			WinEventToXEvent(&wineventq->list[i],event_return);
			if (event_return->type == event_type)
			{
				break;
			}
		}
		i++;
		if (i>wineventq->num) i=0;
	}
	if (i != wineventq->avail)
	{
		while(i != wineventq->next)
		{
			j =i-1;
			if (j<0) j= wineventq->num;
			copyWinEvent(&wineventq->list[i],&wineventq->list[j]);
			i = j;
		}
		wineventq->next++;
		wineventq->count--;
		cjh_printf("removed an event\n");
		return 1;
	}
	*/
	return 0;
}

/*****************************************************************\


	Function: XWindowEvent
	Inputs:   display, window, event mask.
	Returned: pointer to filled in event structure.

	Comments: This is fudged at the moment to work with the toolkit.
		  The event system needs rewriting to account properly for
		  event masks.

\*****************************************************************/

int
XWindowEvent(display,w,emask,rep)
Display *display;
Window w;
long emask;
XEvent *rep;
{
	NT_window *ntw=(NT_window *)w;
	MSG msg;

	xtrace("XWindowEvent\n");
	if (emask&ExposureMask)
	{
		GetMessage(&msg,ntw->w,USR_MapNotify,USR_MapNotify);
		rep->type=ConfigureNotify;
	}
	return 0;
}



/*****************************************************************\

	Function: XNextEvent
	Inputs:   display, event structure pointer.

	Comments: Windows routines receive messages (events) in two ways:
		  firstly by GetMessage, which takes messages from the
		  calling thread's message queue, and secondly by the
		  window function being called with events as arguments.
		  To simulate the X system, we get messages from the queue
		  and pass them to the window function anyway, which
		  processes them and fills out the local XEvent structure.
		  DispatchMessage calls the window procedure and waits until
		  it returns. Translate message turns WM_KEYUP/DOWN messages
		  into WM_CHAR.

\*****************************************************************/

int
XNextEvent(Display *display,XEvent  *event)
{
    MSG msg;
	
    xtrace("XNextEvent\n");

    /* if there isn't already an event in the pipe, this will block */
    while(wineventq->count <= 0 && GetMessage(&msg, NULL, 0, 0)>0)
    {		
	doTranslateMessage(&msg);
	DispatchMessage(&msg);
    }
    if (wineventq->count>0)
    {
	getQdEvent(wineventq,event);
    }
    else
    {
	/* hmm, GetMessage failed, maybe we're supposed to quit */
	event->type=ClientMessage;
	event->xclient.format = 32;
	event->xclient.data.l[0] = XInternAtom(NULL,"WM_DELETE_WINDOW", FALSE);
	return 1;
    }
    return 1;
}

BoolDef
XFilterEvent(XEvent* event,Window window)
{
	xtrace("XFilterEvent\n");
	return 0;
}

BoolDef
XQueryPointer(
	Display* display,
	Window w,
	Window* root_return,
	Window* child_return,
	int* root_x_return,
	int* root_y_return,
	int* win_x_return,
	int* win_y_return,
	unsigned int* mask_return)
{
    POINT point;
	RECT rect;
	xtrace("XQueryPointer\n");
    GetCursorPos(&point);
    *root_x_return = point.x;
    *root_y_return = point.y;
	GetWindowRect(((NT_window*)w)->w,&rect);
	*win_x_return= point.x - rect.left;
	*win_y_return= point.y - rect.top;
	*mask_return = NT_get_state();
    return True;
}

int
XConvertSelection(
    Display *display,
    Atom sel, Atom target, Atom prop,
    Window req,	
    Time time)
{
    xtrace("XConvertSelection\n");
    QEvent(wineventq,(NT_window*)req,USR_ConvertSelection,0,0L);
    NT_wakeup(((NT_window*)req)->w);
    return 0;
}

