/*
 * $Id: grxlib.c,v 1.1 2003/11/24 17:28:08 pcg Exp $
 */

#include "../../config.h"
#include "rxvt.h"
#include "init.h"		/* for GET_TERMIOS / SET_TERMIOS */
#include "grxlib.h"

/*----------------------------------------------------------------------*/

void
Done(void)
{
    putchar(':');
}

void
StartLine(long id)
{
    printf("\033GL%ld", id);
}

void
StartPoint(long id)
{
    printf("\033GP%ld", id);
}

void
StartFill(long id)
{
    printf("\033GF%ld", id);
}

void
Extend(int x, int y)
{
    printf(";%d;%d", x, y);
}

void
FillArea(int x1, int y1, int x2, int y2)
{
    printf(";%d;%d;%d;%d", x1, y1, x2, y2);
}

void
PlaceText(long id, int x, int y, int mode, char *text)
{
    printf("\033GT%ld;%d;%d;%d;%d:%s", id, x, y, mode, strlen(text), text);
    fflush(stdout);
}

void
ClearWindow(long id)
{
    printf("\033GC%ld:", id);
}

void
ForeColor(int col)
{
    printf("\033[3%dm", (col < 0 || col > 7) ? 0 : col);
}

void
DefaultRendition(void)
{
    printf("\033[m");
}

#define LINESZ	100
static char     line[LINESZ];
static FILE    *infd = NULL;

long
CreateWin(int x, int y, int w, int h)
{
    long            id = 0;

    fflush(stdout);
    printf("\033GW%d;%d;%d;%d:", x, y, w, h);
    fflush(stdout);
    while (1) {
	if ((fgets(line, LINESZ, infd) != NULL) &&
	    (sscanf(line, "\033W%ld", &id) == 1))
	    break;
    }
    return id;
}

void
QueryWin(long id, int *nfwidth, int *nfheight)
{
    int             id1, x, y, width, height, fwidth, fheight;

    printf("\033GG%ld:", id);
    fflush(stdout);
    while (1) {
	if ((fgets(line, sizeof(line), infd) != NULL) &&
	    (sscanf(line, "\033G%ld %ld %ld %ld %ld %ld %ld %ld %ld",
		    &id1, &x, &y, &width, &height,
		    &fwidth, &fheight, nfwidth, nfheight) != 0))
	    break;
    }
}

int
WaitForCarriageReturn(long *win, int *x, int *y)
{
    int             i, len;

    fgets(line, LINESZ, infd);
    line[LINESZ - 1] = 0;
    len = strlen(line);
    for (i = 0; i < len; i++) {
	if (line[i] == '\033') {
	    int             ret = 1;

	    i++;
	    switch (line[i]) {
	    case 'R':
		ret++;
		/* drop */
	    case 'P':
		sscanf(&line[i + 1], "%ld;%d;%d", win, x, y);
		return ret;
		break;
	    }
	}
    }
    return 0;
}

static int      fno2;
static ttymode_t ttmode;

int
InitializeGraphics(int scroll_text_up)
{
    int             fno, i;
    char           *screen_tty;
    struct winsize  winsize;

    fno = fileno(stdout);
    if (!isatty(fno)) {
	fprintf(stderr, "stdout must be a tty\n");
	return 0;
    }
    screen_tty = ttyname(fno);

#ifdef HAVE_TERMIOS_H
    GET_TERMIOS(fno, &ttmode);
    ttmode.c_lflag &= ~ECHO;
    SET_TERMIOS(fno, &ttmode);
#endif

    infd = fopen(screen_tty, "rw");

#ifdef HAVE_TERMIOS_H
    fno2 = fileno(infd);
    GET_TERMIOS(fno2, &ttmode);
    ttmode.c_lflag &= ~ECHO;
    SET_TERMIOS(fno2, &ttmode);
#endif

    /* query rxvt to find if graphics are available */
    fflush(stdout);
    printf("\033GQ");
    fflush(stdout);
    while (1) {
	if ((fgets(line, LINESZ, infd) != NULL) &&
	    (sscanf(line, "\033G%d", &i) == 1)) {
	    if (!i) {
		fprintf(stderr, "rxvt graphics not available\n");
		CloseGraphics();
		return 0;
	    }
	    break;
	}
    }
    if (scroll_text_up) {
	ioctl(fno, TIOCGWINSZ, &winsize);
	fflush(stdout);
	for (i = 0; i < winsize.ws_row; i++)
	    putchar('\n');
	fflush(stdout);
    }
    return i;
}

void
CloseGraphics(void)
{
    DefaultRendition();
    fflush(stdout);
#ifdef HAVE_TERMIOS_H
    ttmode.c_lflag |= ECHO;
    SET_TERMIOS(fno2, &ttmode);
#endif
    fclose(infd);
}

/*----------------------- end-of-file (C source) -----------------------*/
