/* 
 * $Id: grxlib.h,v 1.2 2003/11/24 17:31:28 pcg Exp $
 */
#include "rxvtgrx.h"		/* text alignment */

/*function pointer to either StartLine or StartPoint */
typedef void (*LineFunction) (long id);

#ifdef __cplusplus
extern "C" {
#endif
   extern void	StartLine (long id);
   extern void	StartPoint (long id);
   extern void	Extend (int x, int y);
   extern void	StartFill (long id);
   extern void	FillArea (int x1, int y1, int x2, int y2);
   extern void	Done (void);
   extern void	PlaceText (long id, int x, int y, int mode, char *text);

   extern void	ClearWindow (long id);
   extern long	CreateWin (int x, int y, int w, int h);
   extern void	QueryWin (long id, int *nfwidth, int *nfheight);
   extern void	ForeColor (int color);
   extern void	DefaultRendition (void);
   extern int	WaitForCarriageReturn (long *win, int *x, int *y);
   extern int	InitializeGraphics (int scroll_text_up);
   extern void	CloseGraphics (void);
#ifdef __cplusplus
}
#endif
/*----------------------- end-of-file (C header) -----------------------*/
