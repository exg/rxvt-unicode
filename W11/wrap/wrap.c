
#include <X11/Xlib.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "wrap.h"

void __cdecl WinMainCRTStartup(void) { mainCRTStartup(); }

static void *_lib=NULL;
static char *_libname = NULL;

static void *_loadfunc(char *name)
{
	void *f = NULL;
	if (_lib==NULL) {
		if (_libname==NULL) {
			fprintf(stderr,"%s called before XOpenDisplay!\n",name);
			exit(3);
		}
		_lib = LoadLibrary(_libname);
		if (_lib == NULL) {
			fprintf(stderr,"failed to load %s.dll\n",_libname);
			exit(1);
		}
	}
	f = GetProcAddress(_lib,name);
	if (f==NULL) {
		fprintf(stderr,"failed to find %s\n",name);
		exit(2);
	}	
	return f;
}

static void _freelib()
{
	FreeLibrary(_lib);
	_lib=NULL;
}
typedef LONG (proto_WndProc)(HWND hWnd,UINT message,UINT wParam,LONG lParam);
static proto_WndProc *func_WndProc = NULL;
LONG __stdcall WndProc(HWND hWnd,UINT message,UINT wParam,LONG lParam)
{
	if (!func_WndProc) func_WndProc=(proto_WndProc *)_loadfunc("NT_handleMsg");
	return (func_WndProc)(hWnd,message,wParam,lParam);
}

static void
hideConsole()
{
	HWND conwin;
	HANDLE hConsole;
	CONSOLE_SCREEN_BUFFER_INFO buffInfo;
	SECURITY_ATTRIBUTES  sa;

 	char app_name[40];
	sprintf(app_name, "rxvt%08x", (unsigned int)GetCurrentThreadId());
	/* from eConsole source */
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;	
	hConsole = CreateFile( "CONOUT$", GENERIC_WRITE | GENERIC_READ,
						   FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,	
						   OPEN_EXISTING, 0, 0 );
	if (GetConsoleScreenBufferInfo(hConsole,&buffInfo) &&
		buffInfo.dwCursorPosition.X==0 &&
		buffInfo.dwCursorPosition.Y==0)
	{
		/* find the console window, from eConsole source */
		SetConsoleTitle( app_name );
		while ((conwin = FindWindow( NULL, app_name))==NULL)
			Sleep( 40 );
		ShowWindowAsync(conwin, SW_HIDE);
	}
}


ATOM
_register_window_class()
{
	WNDCLASS  wc;
	HANDLE    curInstance = GetModuleHandleA(NULL);
	char app_name[40];
	sprintf(app_name, "rxvt%08x", (unsigned int)GetCurrentThreadId());	
	hideConsole();
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; /* CS_OWNDC */
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = curInstance;
	wc.hIcon = LoadIcon(curInstance, MAKEINTRESOURCE( IDI_RXVT ));
	wc.hCursor =  LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName =  NULL;
	wc.lpszClassName = app_name;
	return RegisterClass(&wc);
}

typedef void (proto_NT_SetAtom)(ATOM class);
static proto_NT_SetAtom *func_NT_SetAtom = NULL;
void _set_atom(ATOM class)
{
	if (!func_NT_SetAtom) func_NT_SetAtom=(proto_NT_SetAtom *)_loadfunc("NT_SetAtom");
	(func_NT_SetAtom)(class);
}

typedef void (proto_W11AddEventHandler)(Display *d, proto_W11EventHandler *ev);
static proto_W11AddEventHandler *func_W11AddEventHandler = NULL;
void W11AddEventHandler(Display *d, proto_W11EventHandler *ev)
{
	if (func_W11AddEventHandler) 
	    (func_W11AddEventHandler)(d,ev);
}

typedef Display * (proto_XOpenDisplay)(const char *name);
static proto_XOpenDisplay *func_XOpenDisplay = NULL;
Display * XOpenDisplay(const char *name) {
    ATOM class;
    char *env_var=NULL;
    if (_libname==NULL)
    {
	env_var = getenv("W11_LIBRARY");
	if (env_var!=NULL) _libname = strdup(env_var);
	else if (name==NULL || !strcmp(name,":0")) {
	    _libname="libW11";
	    class = _register_window_class();
	    _set_atom(class);
	    func_W11AddEventHandler=(proto_W11AddEventHandler *)_loadfunc("W11AddEventHandler");
	}	
	else _libname="libX11";
    }
    if (!func_XOpenDisplay) func_XOpenDisplay=(proto_XOpenDisplay *)_loadfunc("XOpenDisplay");
    return (func_XOpenDisplay)(name);
}

typedef struct {
    char    *name;
    XPointer value;
} XIMArg;

static void
_XIMVaToList(va_list var, XIMArg *args)
{
    char   *attr;
	int i = 0;
    if (!args) return;
    for (attr = va_arg(var, char*); attr && i<11; attr = va_arg(var, char*)) {
	    args->name = attr;
	    args->value = va_arg(var, XPointer);
	    args++;
		i++;
    }
	for(;i<11;i++) {
		args->name=NULL;
		args->value=NULL;
		args++;
	}
}

typedef XIC (proto_XCreateIC)(XIM im, ...);
static proto_XCreateIC *func_XCreateIC = NULL;
XIC
XCreateIC(XIM im, ...)
{
    va_list var;
    XIMArg a[11];

    va_start(var, im);
    _XIMVaToList(var, a);
    va_end(var);
	
	if (a[10].name!=NULL) {
		fprintf(stderr,"call to XCreateIC with more than 20 args\n");
		exit(5);
	}
	
	if (!func_XCreateIC) func_XCreateIC=(proto_XCreateIC *)_loadfunc("XCreateIC");
	return (func_XCreateIC)(im,
						   a[0].name, a[0].value,
						   a[1].name, a[1].value,
						   a[2].name, a[2].value,
						   a[3].name, a[3].value,
						   a[4].name, a[4].value,
						   a[5].name, a[5].value,
						   a[6].name, a[6].value,
						   a[7].name, a[7].value,
						   a[8].name, a[8].value,
						   a[9].name, a[9].value,
						   NULL);
}

typedef char * (proto_XGetIMValues)(XIM im, ...);
static proto_XGetIMValues *func_XGetIMValues = NULL;
char *
XGetIMValues(XIM im, ...)
{
    va_list var;
    XIMArg a[11];

    va_start(var, im);
    _XIMVaToList(var, a);
    va_end(var);
	
	if (a[10].name!=NULL) {
		fprintf(stderr,"call to XGetIMValues with more than 20 args\n");
		exit(5);
	}

	if (!func_XGetIMValues) func_XGetIMValues=(proto_XGetIMValues *)_loadfunc("XGetIMValues");
	return (func_XGetIMValues)(im,
						   a[0].name, a[0].value,
						   a[1].name, a[1].value,
						   a[2].name, a[2].value,
						   a[3].name, a[3].value,
						   a[4].name, a[4].value,
						   a[5].name, a[5].value,
						   a[6].name, a[6].value,
						   a[7].name, a[7].value,
						   a[8].name, a[8].value,
						   a[9].name, a[9].value,
						   NULL);
}


#include "xwrappers.gen"
