#include "ntdef.h"

static struct NT_window *window_list=NULL;

void
freeMemory(void *p) {
	if (p!=NULL) free(p);
}
void *
allocateMemory(int s) {
	void *p=NULL;
	if (s)
	{
		p=(void *)malloc(s);
		if (p) memset(p,0,s);
	}
	return p;
}


/*---------------------------------------------------*\
| Function: NT_new_window                             |
| Purpose:  Add a new window id to the Window table   |
| Return:   Pointer to the new Window structure.      |
\*---------------------------------------------------*/
struct NT_window *
NT_new_window()
{
	struct NT_window *new;
	xtrace("NT_new_window\n");
	new = (struct NT_window *) allocateMemory (sizeof(struct NT_window));
	new->next = window_list;
	new->child=NULL;
	new->min = 1;
	new->minx =0;
	new->miny =0;
	new->w = INVALID_HANDLE;
	new->parent= NULL;
	new->hBitmap = INVALID_HANDLE;
	new->hDC = INVALID_HANDLE;
	window_list = new;
	cjh_printf("NEW window %x\n",window_list);
	return(window_list);
}
	
/*---------------------------------------------------*\
| Function: NT_delete_window                          |
| Purpose:  Remove a window from the window list      |
| Input:    w - pointer to window data                |
| Return:   TRUE if deleted                           |
\*---------------------------------------------------*/
int
NT_delete_window(struct NT_window *w)
{
	NT_window *f;
	xtrace("NT_delete_window\n");
	
	if (w->w != INVALID_HANDLE)
	{
		/* ShowWindow(w->w,SW_HIDE);*/
		DestroyWindow(w->w);
		w->w=INVALID_HANDLE;
	}
	if (w->hBitmap != INVALID_HANDLE)
	{
		DeleteObject(w->hBitmap);
		w->hBitmap = INVALID_HANDLE;
	}
	if (w->hDC != INVALID_HANDLE)
	{
		DeleteDC(w->hDC);
		w->hDC=INVALID_HANDLE;
	}
	
	if (window_list == w)
		window_list=w->next;
	else
	{
		for (f=window_list; f!=NULL && f->next!=w; f=f->next);
		if (f!=NULL)
			f->next = w->next;
	}
	freeMemory(w);
	return TRUE;
}

/*------------------------------------------------*\
| Function: NT_find_window_from_id                 |
| Purpose:  Find the window in the window list     |
|           from the HWND id of the window.        |
| Input:    w - Window id (Windows HWND)           |
| Return:   pointer to NT_window structure for w.  |
\*------------------------------------------------*/
struct NT_window *
NT_find_window_from_id(HWND w)
{
	struct NT_window *current = window_list;
	/* xtrace("NT_find_window_from_id\n"); */
	
	while ( current != NULL &&
			current->w != w )
		current = current->next;
	if(current)
		return(current);
	current=window_list;
	return NULL;
}

/*****************************************************************\

	Function: NT_add_child
	Inputs:   parent and child window IDs.
	Returned: 1

	Comments: When a child window is created (eg. client canvas) we
		  update our internal list of windows and their children.
		  New children are added to the front of the list.

\*****************************************************************/

int
NT_add_child(parent,child)
NT_window *parent,*child;
{
	struct NT_child *new;

	new=(struct NT_child *) allocateMemory (sizeof(struct NT_child));
	new->w=child;
	new->next = parent->child;
	parent->child=new;
	return(1);
}

struct NT_window *
NT_find_child(NT_window *w,unsigned long mask,
								unsigned long val)
{
	struct NT_window *ret = NULL;
	struct NT_child *child = NULL;
	if (w)
	{
		if ((w->mask&mask)==val) ret=w;
		child = w->child;
		while(!ret && child) {
			ret = NT_find_child(child->w, mask, val);
			child=child->next;
		}
	}
	return ret;
}



/*****************************************************************\

	Function: NT_del_child
	Inputs:   parent and child window IDs.
	Returned: TRUE if window is removed, FALSE otherwise.

	Comments: Finds child window if it exits, and it so removes it from
		  the window list.

\*****************************************************************/

int
NT_del_child(parent,child)
struct NT_window *parent;
struct NT_window *child;
{
	struct NT_child *current,*last;
	int status=FALSE;

	if (parent->child==NULL)
	{
	}
	else if (parent->child->w==child)
	{
		current = parent->child;
		parent->child=parent->child->next;
		freeMemory(current);
		status=TRUE;
	}
	else
	{
		last=parent->child;
		current=parent->child->next;
		while (current->w!=child && current!=NULL)
		{
			last=current;
			current=current->next;
		}
		if (current!=NULL)
		{
			last->next=current->next;
			freeMemory(current);
			status=TRUE;
		}
	}
	return(status);
}

/*****************************************************************\

	Function: WinMain
	Inputs:   instance, previous instance, command line arguments,
		  default start up.

	Comments: Called instead of main() as the execution entry point.

\*****************************************************************/
#ifdef NOTCYGWIN
#define MAX_COMMAND_ARGS 20
static HANDLE hInstance,hPrevInstance;
int APIENTRY
WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{
        static char *command_args[MAX_COMMAND_ARGS];
        static int num_command_args;
        static char proEng[] = "proe";
        char *wordPtr,*tempPtr;
        int i,quote;
	hInstance=hInst;
	hPrevInstance=hPrevInst;

        for (i=0;i<MAX_COMMAND_ARGS;i++)
          command_args[i] = NULL;

        wordPtr = lpCmdLine;
        quote = 0;
        num_command_args = 1;
        command_args[0] = proEng;
        while  (*wordPtr && (*wordPtr == ' ' || *wordPtr == '\t'))
           wordPtr++;
        if (*wordPtr == '\"')
        {
          quote = 1;
          wordPtr++;
        }
        if (!*wordPtr)
          main(0,NULL);
        else
        {
          while (*wordPtr && num_command_args < MAX_COMMAND_ARGS)
          {
            tempPtr = wordPtr;
            if (quote)
            {
              while (*tempPtr && *tempPtr != '\"')
                tempPtr++;
              quote = 0;
            }
            else
              while (*tempPtr && *tempPtr != ' ')
                tempPtr++;
            if (*tempPtr)
              *(tempPtr++) = '\0';
            command_args[num_command_args++] = wordPtr;
            wordPtr = tempPtr;
            while (*wordPtr && (*wordPtr == ' ' || *wordPtr == '\t'))
              wordPtr++;
            if (*wordPtr == '\"')
            {
              quote = 1;
              wordPtr++;
            }
          }
          main(num_command_args,command_args);
        }

}
#endif

static ATOM atom=0;
void
NT_SetAtom(ATOM class)
{
	atom = class;
}

HWND
NT_create_window(char *title,DWORD style,int x,int y,int w, int h,HWND parent)
{
	HMODULE hInst = NULL; /* GetModuleHandleA(NULL); */
	return CreateWindow((LPCTSTR)MAKELONG(atom,0),title,style,x,y,w,h,
								 parent,NULL,hInst,NULL);
}
HBITMAP
NT_getWallpaper()
{
	HBITMAP hb = NULL;
	do {
		HKEY hKey;
		TCHAR wallpaper[MAX_PATH];
		DWORD dwBufLen = MAX_PATH;
		LONG lRet;

		lRet = RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, KEY_QUERY_VALUE, &hKey);
		if (lRet != ERROR_SUCCESS) break;

		lRet = RegQueryValueEx(hKey, TEXT("Wallpaper"), NULL, NULL, (LPBYTE)wallpaper, &dwBufLen);
		if (lRet != ERROR_SUCCESS) break;

		RegCloseKey(hKey);
		hb = LoadImage(NULL, wallpaper, IMAGE_BITMAP, 0, 0,
               LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );
	} while(0);
	return hb;
}

void
NT_moveWindow(NT_window *ntw, BOOL repaint)
{
	MoveWindow(ntw->w, ntw->x,ntw->y,ntw->wdth,ntw->hght,repaint);
	NT_configureNotify(ntw,ntw->x,ntw->y);
}

NT_set_origin(NT_window *ntw,int x, int y)
{
	HDC hdc;
	hdc = GetDC(ntw->w);
	SetBrushOrgEx(hdc,-x,-y,NULL);
	ReleaseDC(ntw->w,hdc);
}


NT_confChild(NT_window *ntw,int x, int y)
{
	HDC hdc;
	struct NT_child *child = NULL;
	if (ntw)
	{
		if (ntw->parentRelative) NT_set_origin(ntw,x,y);
		child = ntw->child;
		while(child && child->w) {
			NT_confChild(child->w, child->w->x+x, child->w->y+y);
			child=child->next;
		}
	}
}

NT_configureNotify(NT_window *ntw,int x, int y)
{
	while(ntw && ntw->parent) {
		ntw=ntw->parent;
	}
	NT_confChild(ntw,ntw->x,ntw->y);
}



/*
HBITMAP
NT_getWallpaper() 
{
	WCHAR wszWallpaper [MAX_PATH];
	CHAR szWallpaper[MAX_PATH];
	HRESULT hr;
	HBITMAP hb = NULL;
	IActiveDesktop* pIAD = NULL;
	do {
		CoInitialize ( NULL );
		hr = CoCreateInstance ( CLSID_ActiveDesktop,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IActiveDesktop,
                            (void**) &pIAD );
		if (! SUCCEEDED(hr) ) break;
        hr = pIAD->GetWallpaper ( wszWallpaper, MAX_PATH, 0 );

        if (! SUCCEEDED(hr) ) break;
		wsprintf(szWallpaper,"%ls",wszWallpaper);
		hb = LoadImage(NULL, szWallpaper, IMAGE_BITMAP, 0, 0,
               LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );
	} while(0);
	if (pIAD) pIAD->Release();
	CoUninitialize();
	return hb;
}
*/
