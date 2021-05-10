/*--------------------------------*-C-*---------------------------------*
 * Copyright 1997 1998 Oezguer Kesim <kesim@math.fu-berlin.de>
 * Copyright 1992, 1993 Robert Nation <nation@rocket.sanders.lockheed.com>
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
 *----------------------------------------------------------------------*/
#ifdef HAVE_CONFIG_H
# include <config.h>
#else
/* # define STDC_HEADERS */
# define HAVE_UNISTD_H
# define TIME_WITH_SYS_TIME
# define HAVE_SYS_TIME_H
# ifdef _AIX
#  define HAVE_SYS_SELECT_H
# endif
#endif

#include "../src/version.h"
#include "feature.h"

#include <ctype.h>
/* #ifdef STDC_HEADERS */
# include <stdarg.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
/* #endif */

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if defined (__svr4__)
# include <sys/resource.h>	/* for struct rlimit */
#endif

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#include <sys/stat.h>

#ifdef MAIL
#include <dirent.h>
#endif

#include <X11/Intrinsic.h>	/* Xlib, Xutil, Xresource, Xfuncproto */

#define APL_CLASS	"Clock"
#define APL_NAME	"rclock"
#define MSG_CLASS	"Appointment"
#define MSG_NAME	"Appointment"
#define CONFIG_FILE	".rclock"

#ifndef EXIT_SUCCESS	/* missed from <stdlib.h> ? */
# define EXIT_SUCCESS	0
# define EXIT_FAILURE	1
#endif

/*----------------------------------------------------------------------*/

static Display*	Xdisplay;	/* X display */
static int	Xfd;		/* file descriptor of server connection */
static GC	Xgc, Xrvgc;	/* normal, reverse video GC */

#define	Xscreen		DefaultScreen (Xdisplay)
#define Xcmap		DefaultColormap (Xdisplay, Xscreen)
#define Xroot		DefaultRootWindow (Xdisplay)

/* windows and their sizes */
typedef struct {
   Window win;
   int width, height;
} mywindow_t;

static mywindow_t	Clock = {None, 80, 80};	/* parent window */

#define fgColor	0
#define bgColor 1
static const char *	rs_color [2] = { FG_COLOR_NAME, BG_COLOR_NAME };
static Pixel		PixColors [2];
static const char *	rs_geometry = NULL;

#ifdef ICONWIN
static const char *	rs_iconGeometry = NULL;
static mywindow_t	Icon = {None, 65, 65};		/* icon window */
static int		iconic_state = NormalState;	/* iconic startup? */
#endif

#ifdef REMINDERS
static mywindow_t	Msg = {0, 0, 0};		/* message window */
static struct {
   Window
# ifndef NO_REMINDER_EXEC
     Start,
# endif
   Dismiss,
   Defer;
   int width, height;
} msgButton;

static XFontStruct * Xfont;
#define FontHeight()	(Xfont->ascent + Xfont->descent)
static int	Msg_Mapped = 0;		/* message window mapped? */
static int	reminderTime = -1;
static char	message [256] = "";
#ifndef NO_REMINDER_EXEC
static char	execPrgm [256] = "";
#endif
static const char * reminders_file = NULL;	/* name of ~/.rclock file */
#ifdef DATE_ON_CLOCK_FACE
static int show_date = 1;                   /* show date on face of clock */
#endif
#endif

#ifdef ADJUST_TIME
static int	adjustTime = 0;
#else
# define	adjustTime	0
#endif

#ifdef	CENTURY
# if (CENTURY < 1900)
Error, Cenury incorrectly set.
# endif
#else
# define CENTURY 1900
#endif

static int	clockUpdate = CLOCKUPDATE;

#ifdef MAIL
static int mailUpdate = MAILUPDATE;
static char * mail_file = NULL;
#ifndef MAIL_SPAWN
static char * mail_spawn = NULL;
#endif
static int is_maildir = 0;
#endif

static XSizeHints szHint = {
   PMinSize | PResizeInc | PBaseSize | PWinGravity,
   0, 0, 80, 80,		/* x, y, width and height */
   1, 1,			/* Min width and height */
   0, 0,			/* Max width and height */
   1, 1,			/* Width and height increments */
   {1, 1},			/* x, y increments */
   {1, 1},			/* Aspect ratio - not used */
   0, 0,			/* base size */
   NorthWestGravity		/* gravity */
};

/* subroutine declarations */
static void	geometry2sizehint (mywindow_t * /* win */,
				   const char * /* geom */);
static void	Create_Windows (int /* argc */,
				char * /* argv */ []);
static void	getXevent (void);
static void	print_error (const char * /* fmt */, ...);

static void	Draw_Window (mywindow_t * /* this_win */,
			     int /* full_redraw */);
static void	Reminder (void);
static void	Next_Reminder (int /* update_only */);

/* Arguments for Next_Reminder() */
#define REPLACE 0
#define UPDATE 1

/*----------------------------------------------------------------------*/

static void
usage (void)
{
   int i;
   struct {
      const char * const opt;
      const char * const desc;
   } optList[] = {
#define optList_size()		(sizeof(optList)/sizeof(optList[0]))
	{ "-display displayname", "X server to contact" },
	{ "-geometry geom",	"size (in pixels) and position" },
	{ "-bg color",		"background color" },
	{ "-fg color",		"foreground color" },
#ifdef REMINDERS
	{ "-fn fontname",	"normal font for messages" },
#ifdef DATE_ON_CLOCK_FACE
	{ "-nodate",	    "do not display date on the clock face" },
#endif
#endif
#ifdef ICONWIN
	{ "-iconic",		"start iconic" },
#endif
#ifdef ADJUST_TIME
	{ "-adjust +/-ddhhmm",	"adjust clock time" },
#endif
	{ "-update seconds",	"clock update interval" },
#ifdef MAIL
	{ "-mail seconds",	"check $MAIL interval" },
	{ "-mailfile file",	"file to use for mail checking" },
#ifndef MAIL_SPAWN
	{ "-mailspawn cmd",	"execute `cmd` when clock is clicked" },
#endif
#endif
	{ "#geom",		"icon window geometry" }
   };

   fprintf (stderr, "\nUsage v" VERSION ":\n  " APL_NAME " [options]\n\n"
	    "where options include:\n");

   for (i = 0; i < optList_size(); i++)
     fprintf (stderr, "    %-29s%s\n", optList[i].opt, optList[i].desc);
}


/****************
 * Check out if we are using a maildir drop (qmail)
 * Note: this changes  mail_dir to hold the "new" diretory
 */
#ifdef MAIL
static void
CheckMaildir()
{
    struct stat st;
    char *buf, *p;

    if( !*mail_file || stat(mail_file, &st) || !S_ISDIR(st.st_mode) )
	return; /* no */

    if( !(buf = malloc(strlen(mail_file)+5)) ) {
	print_error ("malloc error");
	exit( EXIT_FAILURE );
    }
    strcpy(buf,mail_file);
    p = buf+strlen(buf);
    if( p[-1] != '/' )
	*p++ = '/';

    strcpy(p, "tmp" );
    if( stat(buf, &st) || !S_ISDIR(st.st_mode ) )
	goto leave;
    strcpy(p, "cur" );
    if( stat(buf, &st) || !S_ISDIR(st.st_mode ) )
	goto leave;
    strcpy(p, "new" );
    if( stat(buf, &st) || !S_ISDIR(st.st_mode ) )
	goto leave;

    mail_file = buf;
    is_maildir = 1;
    return;
  leave:
    free(buf);
}
#endif

/*----------------------------------------------------------------------*
 * rclock - Rob's clock
 * simple X windows clock with appointment reminder
 *----------------------------------------------------------------------*/
int
main (int argc, char * argv [])
{
   int i;
   char * opt, * val;
   const char * display_name = NULL;
   XGCValues gcv;

#ifdef REMINDERS
   const char * rs_font = FONT_NAME;

   /* find the ~/.rclock file */
   if ((val = getenv ("HOME")) != NULL)
     {
	char * p = malloc (strlen (CONFIG_FILE) + strlen (val) + 2);
	if (p == NULL)
	  goto Malloc_Error;

	strcpy (p, val);
	strcat (p, "/" CONFIG_FILE);

	reminders_file = p;
     }
#endif
#ifdef MAIL
   val = getenv ("MAIL");	/* get the mail spool file name */
#ifdef MAIL_SPOOL
   if (val == NULL)		/* csh doesn't set $MAIL */
     {
	const char * spool = MAIL_SPOOL;
	char * user = getenv ("USER");	/* assume this works */
	val = malloc (strlen (spool) + strlen (user) + 1);
	if (val == NULL)
	  goto Malloc_Error;
	strcpy (val, spool);
	strcat (val, user);
     }
#endif
   mail_file = val;
   if( mail_file )
       CheckMaildir();
#endif

   if ((display_name = getenv ("DISPLAY")) == NULL)
     display_name = ":0";

   /* parse the command line */
   for (i = 1; i < argc; i += 2)
     {
	opt = argv [i];
	val = argv [i+1];

	switch (*opt++) {
	 case '-':
	   break;

	 case '#':
#ifdef ICONWIN
	   rs_iconGeometry = opt;   /* drop */
#endif
	 default:
	   continue;
	   break;
	}

	if (*opt == 'd' && val)		display_name = val;	/* "d", "display" */
	else if (*opt == 'g' && val)	rs_geometry = val;	/* "g", "geometry" */
#ifdef ICONWIN
	else if (*opt == 'i')	/* "ic", "iconic" */
	  {
	     iconic_state = IconicState;
	     i--;	/* no argument */
	  }
#endif
	else if (!strcmp (opt, "fg") && val)	rs_color [fgColor] = val;
	else if (!strcmp (opt, "bg") && val)	rs_color [bgColor] = val;
#ifdef REMINDERS
	else if (!strcmp (opt, "fn") && val)	rs_font = val;
#ifdef DATE_ON_CLOCK_FACE
	else if (!strcmp (opt, "nodate"))
	  {
	     show_date = 0;
	     i--;	/* no argument */
	  }
#endif
#endif
	else if (!strcmp (opt, "update") && val)
	  {
	     int x = atoi (val);
	     if (x < 1 || x > 60)
	       print_error ("update: %d sec", clockUpdate);
	     else
	       clockUpdate = x;
	  }
#ifdef MAIL
	else if (!strcmp (opt, "mail") && val)
	  {
	     int x = atoi (val);
	     if (x < 1)
	       print_error ("mail update: %d sec", mailUpdate);
	     else
	       mailUpdate = x;
	  }
	else if (!strcmp (opt, "mailfile") && val)
	  {
	     /* If the mail environment is not set, then mail_file was created
	      * with a malloc.  We need to free it.
	      */
	     if( getenv ("MAIL") == NULL)
		     free( mail_file);
	     /* assume user knows what he's doing, don't check that file is valid...*/
	     mail_file = val;
	  }
#ifndef MAIL_SPAWN
	else if (!strcmp (opt, "mailspawn") && val)
	  {
			mail_spawn = val;
	  }
#endif
#endif	/* MAIL */
#ifdef ADJUST_TIME
	else if (!strcmp (opt, "adjust") && val)
	  {
	     /* convert ddhhmm to seconds, minimal error checking */
	     int x = atoi (val);
	     adjustTime = ((((abs (x) / 10000) % 100) * 24	/* days */
			    + ((abs (x) / 100) % 100)) * 60	/* hours */
			   + (abs (x) % 100)) * 60;		/* minutes */
	     if (x < 0)
	       adjustTime = -adjustTime;
	  }
#endif	/* ADJUST_TIME */
	else
	  {
	     usage ();
	     goto Abort;
	  }
     }

   /* open display */
   Xdisplay = XOpenDisplay (display_name);
   if (!Xdisplay)
     {
	print_error ("can't open display %s", display_name);
	goto Abort;
     }

   /* get display info */
   Xfd = XConnectionNumber (Xdisplay);
     {
	const char * const color_msg = "can't load color \"%s\"";
	XColor xcol;

	/* allocate foreground/background colors */
	if (!XParseColor (Xdisplay, Xcmap, rs_color [fgColor], &xcol) ||
	    !XAllocColor (Xdisplay, Xcmap, &xcol))
	  {
	     print_error (color_msg, rs_color [fgColor]);
	     goto Abort;
	  }
	PixColors [fgColor] = xcol.pixel;

	if (!XParseColor (Xdisplay, Xcmap, rs_color [bgColor], &xcol) ||
	    !XAllocColor (Xdisplay, Xcmap, &xcol))
	  {
	     print_error (color_msg, rs_color [bgColor]);
	     goto Abort;
	  }
	PixColors [bgColor] = xcol.pixel;
     }

#ifdef REMINDERS
   /* load the font for messages */
   if ((Xfont = XLoadQueryFont (Xdisplay, rs_font)) == NULL)
     {
	print_error ("can't load font \"%s\"", rs_font);
	goto Abort;
     }
   gcv.font = Xfont->fid;
#endif

   Create_Windows (argc, argv);
   /*  Create the graphics contexts */
   gcv.foreground = PixColors [fgColor];
   gcv.background = PixColors [bgColor];

   Xgc = XCreateGC (Xdisplay, Clock.win,
#ifdef REMINDERS
		    GCFont |
#endif
		    GCForeground | GCBackground, &gcv);

   gcv.foreground = PixColors [bgColor];
   gcv.background = PixColors [fgColor];
   Xrvgc = XCreateGC (Xdisplay, Clock.win,
#ifdef REMINDERS
		      GCFont |
#endif
		      GCForeground | GCBackground, &gcv);

   getXevent ();
   return EXIT_SUCCESS;

   Malloc_Error:
   print_error ("malloc error");
   Abort:
   print_error ("aborting");
   return EXIT_FAILURE;
}

/*
 * translate geometry string to appropriate sizehint
 */
static void
geometry2sizehint (mywindow_t * win, const char * geom)
{
   int x, y, flags;
   unsigned int width, height;

   /* copy in values */
   szHint.width  = win->width;
   szHint.height = win->height;

   if (geom == NULL)
     return;

   flags = XParseGeometry (geom, &x, &y, &width, &height);

   if (flags & WidthValue)
     {
	szHint.width = width + szHint.base_width;
	szHint.flags |= USSize;
     }
   if (flags & HeightValue)
     {
	szHint.height = height + szHint.base_height;
	szHint.flags |= USSize;
     }

   if (flags & XValue)
     {
	if (flags & XNegative)
	  {
	     x += (DisplayWidth (Xdisplay, Xscreen) - szHint.width);
	     szHint.win_gravity = NorthEastGravity;
	  }
	szHint.x = x;
	szHint.flags |= USPosition;
     }
   if (flags & YValue)
     {
	if (flags & YNegative)
	  {
	     y += (DisplayHeight (Xdisplay, Xscreen) - szHint.height);
	     szHint.win_gravity = (szHint.win_gravity == NorthEastGravity ?
				     SouthEastGravity : SouthWestGravity);
	  }
	szHint.y = y;
	szHint.flags |= USPosition;
     }

   /* copy out values */
   win->width  = szHint.width;
   win->height = szHint.height;
}

/*
 * Open and map the windows
 */
static void
Create_Windows (int argc, char * argv [])
{
   XClassHint classHint;
   XWMHints wmHint;

   geometry2sizehint (&Clock, rs_geometry);
   Clock.win = XCreateSimpleWindow (Xdisplay, Xroot,
				    szHint.x, szHint.y,
				    Clock.width, Clock.height,
				    0,
				    PixColors [fgColor],
				    PixColors [bgColor]);

#ifdef ICONWIN
   geometry2sizehint (&Icon, rs_iconGeometry);
   Icon.win = XCreateSimpleWindow (Xdisplay, Xroot,
				   szHint.x, szHint.y,
				   Icon.width, Icon.height,
				   0,
				   PixColors [fgColor],
				   PixColors [bgColor]);
   wmHint.initial_state = iconic_state;
   wmHint.icon_window = Icon.win;
   wmHint.flags = InputHint | StateHint | IconWindowHint;
#else
   wmHint.flags = InputHint;
#endif
   wmHint.input = True;

   /* ignore warning about discarded `const' */
   classHint.res_name  = APL_NAME;
   classHint.res_class = APL_CLASS;
   XSetWMProperties (Xdisplay, Clock.win, NULL, NULL, argv, argc,
		     &szHint, &wmHint, &classHint);

   XSelectInput (Xdisplay, Clock.win,
		 (ExposureMask|StructureNotifyMask|ButtonPressMask));

#ifdef ICONWIN
   XSelectInput (Xdisplay, Icon.win,
		 (ExposureMask|ButtonPressMask));
#endif
   XMapWindow (Xdisplay, Clock.win);

   /* create, but don't map a window for appointment reminders */
#ifdef REMINDERS
   Msg.win = XCreateSimpleWindow (Xdisplay, Xroot,
				  szHint.x, szHint.y,
				  szHint.width, szHint.height,
				  0,
				  PixColors [fgColor],
				  PixColors [bgColor]);

   szHint.flags |= USPosition;
   /* ignore warning about discarded `const' */
   classHint.res_name  = MSG_NAME;
   classHint.res_class = MSG_CLASS;
   wmHint.input = True;
   wmHint.flags = InputHint;

   XSetWMProperties (Xdisplay, Msg.win, NULL, NULL, argv, argc,
		     &szHint, &wmHint, &classHint);
     {
	char * str = MSG_NAME;
	XStoreName (Xdisplay, Msg.win, str);
	XSetIconName (Xdisplay, Msg.win, str);
     }

   XSelectInput (Xdisplay, Msg.win,
		 (ExposureMask|ButtonPressMask|KeyPressMask));

   /* font already loaded */

   msgButton.width  = 4 + 5 * XTextWidth (Xfont, "M", 1);
   msgButton.height = 4 + FontHeight ();

   msgButton.Dismiss = XCreateSimpleWindow (Xdisplay, Msg.win,
					    0, 0,
					    msgButton.width, msgButton.height,
					    0,
					    PixColors [bgColor],
					    PixColors [fgColor]);

   XMapWindow (Xdisplay, msgButton.Dismiss);

   msgButton.Defer = XCreateSimpleWindow (Xdisplay, Msg.win,
					  0, 0,
					  msgButton.width, msgButton.height,
					  0,
					  PixColors [bgColor],
					  PixColors [fgColor]);
   XMapWindow (Xdisplay, msgButton.Defer);

#ifndef NO_REMINDER_EXEC
   msgButton.Start = XCreateSimpleWindow (Xdisplay, Msg.win,
                                          0, 0,
					  msgButton.width, msgButton.height,
                                          0,
					  PixColors [bgColor],
					  PixColors [fgColor]);
   XMapWindow (Xdisplay, msgButton.Start);
#endif	/* NO_REMINDER_EXEC */
#endif
}

static time_t
mk_time (struct tm * tmval)
{
   return (tmval->tm_min
	   + 60 * (tmval->tm_hour
		   + 24 * (tmval->tm_mday
			   + 31 * ((tmval->tm_mon+1)
				   + 12 * tmval->tm_year))));
}


#ifdef MAIL
static int
MailAvailable()
{
    struct stat st;

    if( is_maildir ) {
	DIR *dirp;
	struct dirent *d;

	if( (dirp=opendir( mail_file )) ) {
	    while( (d=readdir(dirp)) ) {
		if( *d->d_name == '.' )
		    continue;
		if( isdigit(*d->d_name) ) {
		    closedir(dirp);
		    return 1;
		}
	    }
	    closedir(dirp);
	}
	return 0;
    }
    else
	return !stat(mail_file, &st) &&
	       (st.st_size > 0) && (st.st_mtime >= st.st_atime);
}
#endif

/*----------------------------------------------------------------------*
 * Redraw the whole window after an exposure or size change.
 * After a timeout, only redraw the hands.
 * Provide reminder if needed.
 *----------------------------------------------------------------------*/
static void
Draw_Window (mywindow_t * W, int full_redraw)
{
   /* pre-computed values for sin() x1000, to avoid using floats */
   static const short Sin [60] = {
      0,
      105, 208, 309, 407, 500, 588, 669,
      743, 809, 866, 914, 951, 978, 995,
      1000,
      995, 978, 951, 914, 866, 809, 743,
      669, 588, 500, 407, 309, 208, 105,
      0,
      -105, -208, -309, -407, -500, -588, -669,
      -743, -809, -866, -914, -951, -978, -995,
      -1000,
      -995, -978, -951, -914, -866, -809, -743,
      -669, -588, -500, -407, -309, -208, -105
   };

   static int savedDay = -1;

   time_t currentTime;
   struct tm * tmval;
   int ctr_x, ctr_y;

   typedef struct {
      int h_x, h_y;		/* hour */
      int m_x, m_y;		/* minute */
      int s_x, s_y;		/* second */
   } hands_t;			/* hand positions (x,y) */

   hands_t HandsNow, * pHandsOld;

   GC X_gc, X_rvgc;

   static hands_t HandsOld = { -1 };
#ifdef ICONWIN
   static hands_t HandsOld_icon = { -1 };
#endif
#ifdef REMINDERS
   static int lastUpdateTime = -10;
#endif

#ifdef MAIL
   static time_t mailTime = 0;
   static int MailUp = 0, MailUp_rvideo = 0;
#ifdef ICONWIN
   static int MailUp_icon = 0;
#endif
#endif	/* MAIL */

   currentTime = time (NULL) + adjustTime;	/* get the current time */
   tmval = localtime (&currentTime);

#ifdef MAIL
#ifdef REMINDERS
   if (W->win != Msg.win)
#endif
     {
	int * pMailUp = (
#ifdef ICONWIN
			 W->win == Icon.win ? &MailUp_icon :
#endif
			 &MailUp);

	if ((currentTime - mailTime) >= mailUpdate)
	  {
	     struct stat st;

	     if (
#ifdef ICONWIN
		 MailUp != MailUp_icon ? MailUp :
#endif
		   ((mail_file != NULL) && MailAvailable() )   )
	       {
		  if (!*pMailUp)
		    {
		       *pMailUp = 1;
		       full_redraw = 1;
		       XSetWindowBackground (Xdisplay, W->win,
					     PixColors [fgColor]);
#ifdef MAIL_BELL
		       XBell (Xdisplay, 0);
#endif
		    }
	       }
	     else
	       {
		  if (*pMailUp)
		    {
		       *pMailUp = 0;
		       full_redraw = 1;
		       XSetWindowBackground (Xdisplay, W->win,
					     PixColors [bgColor]);
		    }
	       }
#ifdef ICONWIN
	     if (MailUp == MailUp_icon)
#endif
	       mailTime = currentTime;

	     MailUp_rvideo = *pMailUp;
	  }
     }
#endif	/* MAIL */

   /* once every day, update the window and icon name */
   if (tmval->tm_yday != savedDay)
     {
	char str [20];

	savedDay = tmval->tm_yday;
	strftime (str, sizeof(str), "%a %h %d", tmval);
	XStoreName (Xdisplay, Clock.win, str);
	XSetIconName (Xdisplay, Clock.win, str);
     }

   if (full_redraw)
     XClearWindow (Xdisplay, W->win);

#ifdef REMINDERS
   /* for a message window, just re-draw the message */
   if (W->win == Msg.win)
     {
	char * beg, * next;
	int lines;

	for (beg = message, lines = 0; beg; beg = next, lines++)
	  {
	     char * end;

	     if ((end = strstr (beg, "\\n")) == NULL)
	       {
		  end = beg + strlen (beg);
		  next = NULL;
	       }
	     else
	       {
		  next = end + 2;
	       }

	     XDrawString (Xdisplay, Msg.win,
			  Xgc,
			  (Msg.width -
			   XTextWidth (Xfont, beg, (end-beg))) / 2,
			  10 + Xfont->ascent + FontHeight () * lines,
			  beg, (end-beg));
	  }

	XDrawString (Xdisplay, msgButton.Dismiss,
		     Xrvgc,
		     (msgButton.width - XTextWidth (Xfont, "Done", 4)) / 2,
		     Xfont->ascent + 2,
		     "Done", 4);

	XDrawString (Xdisplay, msgButton.Defer,
		     Xrvgc,
		     (msgButton.width - XTextWidth (Xfont, "Defer", 5)) / 2,
		     Xfont->ascent + 2,
		     "Defer", 5);

# ifndef NO_REMINDER_EXEC
	XDrawString (Xdisplay, msgButton.Start,
		     Xrvgc,
		     (msgButton.width - XTextWidth (Xfont, "Start", 5)) / 2,
		     Xfont->ascent + 2,
		     "Start", 5);

        if (strlen (execPrgm) > 1)
	  XMapWindow (Xdisplay, msgButton.Start);
        else
	  XUnmapWindow (Xdisplay, msgButton.Start);
# endif	/* NO_REMINDER_EXEC */
	return;
     }

   /*
    * Convert multi-field time info to a single integer with a resolution
    * in minutes.
    */
   currentTime = mk_time (tmval);

   /* is there a reminder pending? */
   if (reminderTime >= 0 && currentTime >= reminderTime)
     Reminder ();

   /* every 10 minutes, or at start of day, check for revised entries */
   if (!Msg_Mapped &&
       (currentTime > lastUpdateTime + REMINDERS_TIME ||
	(currentTime != lastUpdateTime &&
	 tmval->tm_hour == 0 && tmval->tm_min == 0)))
     {
	Next_Reminder (UPDATE);
	lastUpdateTime = currentTime;
     }
#endif

   /*
    * draw clock
    */

   ctr_x = (W->width  / 2);
   ctr_y = (W->height / 2);

#define XPOS(i,val) (ctr_x + (W->width  * Sin[i%60] * (val) + 100000) / 200000)
#define YPOS(i,val) (ctr_y - (W->height * Sin[(i+15)%60] * (val) + 100000) / 200000)
   /*
    * how to draw the clock face
    */

   /* calculate the positions of the hands */
     {
	int angle = (tmval->tm_hour % 12) * 5 + (tmval->tm_min / 12);
	HandsNow.h_x = XPOS (angle, 60);
	HandsNow.h_y = YPOS (angle, 60);
     }
     {
	int angle = tmval->tm_min;
	HandsNow.m_x = XPOS (angle, 80);
	HandsNow.m_y = YPOS (angle, 80);
     }
   if (clockUpdate == 1)
     {
	int angle = tmval->tm_sec;
	HandsNow.s_x = XPOS (angle, 85);
	HandsNow.s_y = YPOS (angle, 85);
     }

   pHandsOld = (
#ifdef ICONWIN
		W->win == Icon.win ?  &HandsOld_icon :
#endif
		&HandsOld);

#ifdef MAIL
   if (MailUp_rvideo)
     {
	X_gc = Xrvgc;
	X_rvgc = Xgc;
     }
   else
#endif
     {
	X_gc = Xgc;
	X_rvgc = Xrvgc;
     }

   /*
    * Draw the date in the lower half of the clock window.
    * The code is enclosed in REMINDERS because it uses the same
    * font as the reminders code.
		* I believe this should be drawn always so it does not get
		* "swept away" by the minute hand.
    */
#ifdef REMINDERS && DATE_ON_CLOCK_FACE
   if( show_date)
     {
	char date[10];
	currentTime = time (NULL) + adjustTime;	/* get the current time */
	tmval = localtime (&currentTime);
	strftime (date, sizeof(date), "%d", tmval);
	XDrawString (Xdisplay, W->win, X_gc,
	  ctr_x - XTextWidth(Xfont, date, strlen(date))/2,
	  ctr_y + FontHeight() + (ctr_y - FontHeight())/2,
	  date, strlen(date));
     }
#endif

   if (full_redraw)
     {
	int angle;
	/*
	 * draw clock face
	 */
#ifdef SUBTICKS
	for (angle = 0; angle < 60; angle++)
	  XDrawPoint (Xdisplay, W->win, X_gc,
		      XPOS (angle, 95),
		      YPOS (angle, 95));
#endif
	for (angle = 0; angle < 60; angle += 5)
	  XDrawLine (Xdisplay, W->win, X_gc,
		     XPOS (angle, 90),
		     YPOS (angle, 90),
		     XPOS (angle, 100),
		     YPOS (angle, 100));
     }
   else if (memcmp (pHandsOld, &HandsNow, sizeof(hands_t)))
     {
	int i, j;
	/*
	 * erase old hands
	 */
	for (i = -1; i < 2; i++) for (j = -1; j < 2; j++)
	  {
	     /* hour/minute hands */
	     XDrawLine (Xdisplay, W->win, X_rvgc,
			ctr_x + i,
			ctr_y + j,
			pHandsOld->h_x, pHandsOld->h_y);
	     XDrawLine (Xdisplay, W->win, X_rvgc,
			ctr_x + i,
			ctr_y + j,
			pHandsOld->m_x, pHandsOld->m_y);
	  }

	if (clockUpdate == 1)	/* seconds hand */
	  XDrawLine (Xdisplay,
		     W->win, X_rvgc,
		     ctr_x,
		     ctr_y,
		     pHandsOld->s_x, pHandsOld->s_y);
     }

   if (full_redraw || memcmp (pHandsOld, &HandsNow, sizeof(hands_t)))
     {
	int i, j;
	/*
	 * draw new hands
	 */
	for (i = -1; i < 2; i++) for (j = -1; j < 2; j++)
	  {
	     /* hour/minute hands */
	     XDrawLine (Xdisplay, W->win, X_gc,
			ctr_x + i,
			ctr_y + j,
			HandsNow.h_x, HandsNow.h_y);

	     XDrawLine (Xdisplay, W->win, X_gc,
			ctr_x + i,
			ctr_y + j,
			HandsNow.m_x, HandsNow.m_y);
	  }
	if (clockUpdate == 1)	/* seconds hand */
	  XDrawLine (Xdisplay, W->win, X_gc,
		     ctr_x,
		     ctr_y,
		     HandsNow.s_x, HandsNow.s_y);

	*pHandsOld = HandsNow;
     }
}

#ifdef REMINDERS
/*
 * Read a single integer from *pstr, returns default value if it finds "*"
 * DELIM = trailing delimiter to skip
 */
static int
GetOneNum (char ** pstr, int def)
{
   int num, hit = 0;

   for (num = 0; isdigit (**pstr); (*pstr)++)
     {
	num = num * 10 + (**pstr - '0');
	hit = 1;
     }
   if (!hit)
     {
	num = def;
	while (**pstr == '*') (*pstr)++;
     }
   return num;
}

/*
 * find if TODAY is found in PSTR
 */
static int
isToday (char ** pstr, int wday)
{
   const char * dayNames = DAY_NAMES;
   int rval, today;

   today = dayNames [wday];
   /* no day specified is same as wildcard */
   if (!strchr (dayNames, tolower (**pstr)))
     return 1;

   for (rval = 0; strchr (dayNames, tolower (**pstr)); (*pstr)++)
     {
	if (today == tolower (**pstr) || **pstr == '*')
	  rval = 1;		/* found it */
     }
   return rval;
}

static char *
trim_string (char * str)
{
   if (str && *str)
     {
	int n;
	while (*str && isspace (*str)) str++;

	n = strlen (str) - 1;
	while (n > 0 && isspace (str [n])) n--;
	str [n+1] = '\0';
     }
   return str;
}

# ifndef NO_REMINDER_EXEC
static char *
extract_program (char * text)
{
   char * prgm = text;
   while ((prgm = strchr (prgm, ';')) != NULL)
     {
	if (*(prgm-1) == '\\')    /* backslash escaped */
	  {
	     /* remove backslash - avoid memmove() */
	     int i, n = strlen (prgm);
	     for (i = 0; i <= n; i++)
	       prgm [i - 1] = prgm [i];
	  }
	else
	  {
	     *prgm++ = '\0';
	     /* remove leading/trailing space */
	     prgm = trim_string (prgm);
	     break;
	  }
     }
   return prgm;
}
# endif	/* NO_REMINDER_EXEC */

/*
 * Read the ~/.rclock file and find the next reminder
 *
 * update_only = 1
 *	look for a reminder whose time is greater than the current time,
 *	but less than the currently set reminder time
 *
 * update_only = 0
 *	look for a reminder whose time is greater than the reminder that
 *	just went off
 */
static void
Next_Reminder (int update_only)
{
   struct tm * tmval;
   char buffer [256];
#ifndef INT_MAX
# define INT_MAX	1e8
#endif
   time_t currentTime;
   int savedTime = INT_MAX;
   FILE * fd;

   if (reminders_file == NULL || (fd = fopen (reminders_file, "r")) == NULL)
     {
	reminderTime = -1;	/* no config file, no reminders */
	return;
     }

   currentTime = time (NULL) + adjustTime;	/* get the current time */
   tmval = localtime (&currentTime);
   currentTime = mk_time (tmval);

   /* initial startup*/
   if (reminderTime < 0)
     {
	/* ignore reminders that have already occurred */
	reminderTime = currentTime;
# ifndef NO_REMINDER_EXEC
	/* scan for programs run on start-up */
	while (fgets (buffer, sizeof(buffer), fd))
	  {
	     char * prgm, * text;

	     text = trim_string (buffer);
	     if (*text != ';') continue;

	     prgm = extract_program (text);
	     if (prgm != NULL && strlen (prgm) > 1)
	       system (prgm);
	  }
	rewind (fd);
# endif /* NO_REMINDER_EXEC */
     }

   /* now scan for next reminder */
   while (fgets (buffer, sizeof(buffer), fd))
     {
	int testTime, hh, mm, mo, dd, yy;
	char * text;

	text = trim_string (buffer);
	if (*text == '#') continue;	/* comment */
	if (*text == ';') continue;	/* program run on startup */
	/*
	 * parse the line, format is hh:mm mo/dd/yr message; program
	 * any of hh, mm, mo, dd, yr could be a wildcard `*'
	 */
	hh = GetOneNum (&text, tmval->tm_hour);  if (*text == ':') text++;
	mm = GetOneNum (&text, 0);

	while (isspace (*text)) text++;
	if (!isToday (&text, tmval->tm_wday)) continue;
	while (isspace (*text)) text++;

	mo = GetOneNum (&text, tmval->tm_mon+1); if (*text == '/') text++;
	dd = GetOneNum (&text, tmval->tm_mday);  if (*text == '/') text++;
	yy = GetOneNum (&text, tmval->tm_year);

	/* handle 20th/21st centuries */
	if (yy > CENTURY)
	  yy -= 1900;
	else if (yy < CENTURY)
	  yy += (CENTURY - 1900);

	while (isspace (*text)) text++;
	if (!*text) continue;

	testTime = (mm + 60 * (hh + 24 * (dd + 31 * (mo + 12 * yy))));

	if (testTime > (update_only ? currentTime : reminderTime))
	  {
#ifndef NO_REMINDER_EXEC
	     char * prgm = extract_program (text);
#endif	/* NO_REMINDER_EXEC */
	     /* trim leading/trailing space */
	     text = trim_string (text);

	     /*
	      * have a reminder whose time is greater than the last
	      * reminder, now make sure it is the smallest available
	      */
	     if (testTime < savedTime)
	       {
		  savedTime = testTime;
		  strncpy (message, text, sizeof(message));
#ifndef NO_REMINDER_EXEC
		  strncpy (execPrgm, (prgm ? prgm : ""), sizeof(execPrgm));
#endif
	       }
	     else if (testTime == savedTime)
	       {
                  if (strlen (text))
		    {
		       int n = (sizeof(message) - strlen (message) - 3);
		       if (n > 0)
			 {
			    /* for co-occurring events */
			    strcat (message, "\\n");
			    strncat (message, text, n);
			 }
		    }
#ifndef NO_REMINDER_EXEC
                  if (prgm != NULL)
		    {
		       int n = (sizeof(execPrgm) - strlen (execPrgm) - 2);
		       if ((n > 0) && (n >= strlen (prgm)))
			 {
			    /* for co-occurring programs */
			    strcat (execPrgm, ";");
			    strncat (execPrgm, prgm, n);
			 }
		    }
#endif /* NO_REMINDER_EXEC */
	       }
	  }
     }

   reminderTime = (savedTime < INT_MAX) ? savedTime : -1;
   fclose (fd);
}

/*
 * Provide reminder by mapping the message window
 */
static void
Reminder (void)
{
   char * beg, * next;
   int lines;

   if (Msg_Mapped)
     return;

#ifndef NO_REMINDER_EXEC
   if (strlen (message) == 0)
     {
	if (strlen (execPrgm) > 1)
	  {
	     system (execPrgm);
	     Next_Reminder (REPLACE);
	  }
	return;			/* some sort of error */
     }
#endif

   /* compute the window size */
#ifdef NO_REMINDER_EXEC
   Msg.width = 10 * XTextWidth (Xfont, "M", 1);
#else
   Msg.width = 18 * XTextWidth (Xfont, "M", 1);
#endif

   for (beg = message, lines = 1; beg; beg = next, lines++)
     {
	int width;
	char * end;

	if ((end = strstr (beg, "\\n")) == NULL)
	  {
	     end = beg + strlen (beg);
	     next = NULL;
	  }
	else
	  {
	     next = end + 2;
	  }

	width = XTextWidth (Xfont, beg, (end-beg));
	if (Msg.width < width)
	  Msg.width = width;
     }

   Msg.width += 30;
   Msg.height = (lines+1) * FontHeight () + 30;

   /* resize and centre the window */
   XMoveResizeWindow (Xdisplay, Msg.win,
		      (DisplayWidth (Xdisplay, Xscreen)  - Msg.width ) / 2,
		      (DisplayHeight (Xdisplay, Xscreen) - Msg.height) / 2,
		      Msg.width, Msg.height);

#define BUTTON_MARGIN	8

   XMoveWindow (Xdisplay, msgButton.Dismiss,
		BUTTON_MARGIN,
		(Msg.height - msgButton.height - BUTTON_MARGIN));
   XMoveWindow (Xdisplay, msgButton.Defer,
		(Msg.width - msgButton.width - BUTTON_MARGIN),
		(Msg.height - msgButton.height - BUTTON_MARGIN));
#ifndef NO_REMINDER_EXEC
   XMoveWindow (Xdisplay, msgButton.Start,
		(Msg.width - msgButton.width) / 2,
		(Msg.height - msgButton.height - BUTTON_MARGIN));
#endif

   XMapRaised (Xdisplay, Msg.win);
   XBell (Xdisplay, 0);
   Msg_Mapped = 1;
}
#endif	/* REMINDERS */

#ifndef _POSIX_VERSION
# if defined (__svr4__)
static int
getdtablesize (void)
{
   struct rlimit rlim;
   getrlimit (RLIMIT_NOFILE, &rlim);
   return rlim.rlim_cur;
}
# endif
#endif

/*
 * Loops forever, looking for stuff to do. Sleeps 1 minute if nothing to do
 */
static void
getXevent (void)
{
   XEvent ev;
   int num_fds;		/* number of file descriptors being used */
   struct timeval tm;
   struct tm * tmval;
   Atom wmDeleteWindow;
   fd_set in_fdset;

   /* Enable delete window protocol */
   wmDeleteWindow = XInternAtom (Xdisplay, "WM_DELETE_WINDOW", False);
   XSetWMProtocols (Xdisplay, Clock.win, &wmDeleteWindow, 1);
#ifdef ICONWIN
   XSetWMProtocols (Xdisplay, Icon.win,  &wmDeleteWindow, 1);
#endif
#ifdef REMINDERS
   XSetWMProtocols (Xdisplay, Msg.win,   &wmDeleteWindow, 1);
#endif

#ifdef _POSIX_VERSION
   num_fds = sysconf (_SC_OPEN_MAX);
#else
   num_fds = getdtablesize ();
#endif
#ifdef FD_SETSIZE
    if (num_fds > FD_SETSIZE)
       num_fds = FD_SETSIZE;
#endif

   while (1) {
      /* take care of all pending X events */
      while (XPending (Xdisplay)) {
	 XNextEvent (Xdisplay, &ev);
	 switch (ev.type) {
	  case ClientMessage:
	    /* check for delete window requests */
	    if ((ev.xclient.format == 32) &&
		(ev.xclient.data.l[0] == wmDeleteWindow))
	      {
#ifdef REMINDERS
		 if (ev.xany.window == Msg.win)
		   {
		      XUnmapWindow (Xdisplay, Msg.win);
		      Msg_Mapped = 0;
		      Next_Reminder (REPLACE);
		   }
		 else
#endif
		   return;	/* delete window is how this terminates */
	      }
	    break;

	  case Expose:
	  case GraphicsExpose:
	    /* need to re-draw a window */
	    if (ev.xany.window == Clock.win)
	      Draw_Window (&Clock, 1);
#ifdef ICONWIN
	    else if (ev.xany.window == Icon.win)
	      Draw_Window (&Icon, 1);
#endif
#ifdef REMINDERS
	    else
	      Draw_Window (&Msg, 1);
#endif
	    break;

	  case ConfigureNotify:
	    /* window has been re-sized */
	    if (ev.xany.window == Clock.win)
	      {
		 Clock.width  = ev.xconfigure.width;
		 Clock.height = ev.xconfigure.height;
	      }
	    break;

#ifdef REMINDERS
	  case KeyPress:
	    /* any key press to dismiss message window */
	    if (ev.xany.window == Msg.win)
	      {
		 Next_Reminder (REPLACE);
		 Msg_Mapped = 0;
		 XUnmapWindow (Xdisplay, Msg.win);
	      }
	    break;
#endif

	  case ButtonPress:
#ifdef REMINDERS
	    /* button press to dismiss message window */
	    if (ev.xany.window == Msg.win)
	      {
		 if (ev.xbutton.subwindow == msgButton.Dismiss)
		   {
		      Next_Reminder (REPLACE);
		      Msg_Mapped = 0;
		      XUnmapWindow (Xdisplay, Msg.win);
		   }
		 else if (ev.xbutton.subwindow == msgButton.Defer)
		   {
		      time_t t = time (NULL) + adjustTime;
		      tmval = localtime (&t);
		      reminderTime = mk_time (tmval) + DEFER_TIME;
		      Msg_Mapped = 0;
		      XUnmapWindow (Xdisplay, Msg.win);
		   }
#ifndef NO_REMINDER_EXEC
                 else if (ev.xbutton.subwindow == msgButton.Start)
                   {
		      system (execPrgm);
                      Next_Reminder (REPLACE);
                      Msg_Mapped = 0;
                      XUnmapWindow (Xdisplay, Msg.win);
                   }
#endif	/* NO_REMINDER_EXEC */
	      }
#endif
#ifdef MAIL
	    if (ev.xany.window == Clock.win)
	      {
#ifdef MAIL_SPAWN
		 /* left button action - spawn a mail reader */
		 if (ev.xbutton.button == Button1)
		   system (MAIL_SPAWN);
#else
		 if ( (ev.xbutton.button == Button1) && (mail_spawn != NULL) )
		   system(mail_spawn);
#endif
		 /* redraw the window */
		 Draw_Window (&Clock, 1);
	      }
#endif
            break;
	 }
      }

      /* Now wait for time out or new X event */
      FD_ZERO (&in_fdset);
      FD_SET (Xfd, &in_fdset);
      tm.tv_sec = clockUpdate;
      tm.tv_usec = 0;
      select (num_fds, &in_fdset, NULL, NULL, &tm);

      Draw_Window (&Clock, 0);
#ifdef ICONWIN
      Draw_Window (&Icon, 0);
#endif
   }
}

/*
 * Print an error message.
 */
static void
print_error (const char * fmt, ...)
{
   va_list arg_ptr;

   va_start (arg_ptr, fmt);
   fprintf (stderr, APL_NAME ": ");
   vfprintf (stderr, fmt, arg_ptr);
   fprintf (stderr,"\n");
   va_end (arg_ptr);
}
/*----------------------- end-of-file (C source) -----------------------*/
