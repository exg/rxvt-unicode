/*
 * $Id: qplot.c,v 1.1 2003/11/24 17:28:08 pcg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include "grxlib.h"

#define Real float

#ifndef GRX_SCALE
#  define GRX_SCALE	10000
#endif

#define DEFAULT_DATA_FILE	"data"

static void
axis_round(Real * min, Real * max, Real * grid_spacing)
{
    int             logspace;

    logspace = (int)(log10((*max - *min) / 10.0) + 0.5);
    *grid_spacing = pow(10, (double)logspace);
    *min = (Real) ((int)(*min / (*grid_spacing))) * (*grid_spacing);
    *max = (Real) ((int)(*max / (*grid_spacing)) + 1) * (*grid_spacing);
}

static int
nice_end(int junk)
{
    CloseGraphics();
    putchar('\n');
    exit(EXIT_SUCCESS);
    return 0;
}

int
main(int argc, char **argv)
{
    char           *file = NULL;
    int             Do_Start = 1, tmp;
    int             m, p, i, j, n, nchars, theight, twidth, xclick, yclick;
    int             downx = 1000, downy = 1000, upx, upy;
    long            id, winclick;
    Real            xmax, xmin, ymax, ymin, xdiff, ydiff, xgrid_spacing,
		    ygrid_spacing;
    Real           *x, *y, *nls;
    LineFunction    linetype = StartLine;
    char            axis[100], line[256];
    FILE           *fd;

    x = (Real *)malloc(1000000 * sizeof(Real));
    y = (Real *)malloc(1000000 * sizeof(Real));
    nls = (Real *)malloc(1000 * sizeof(Real));
    if (x == NULL || y == NULL || nls == NULL) {
	fprintf(stderr, "Can't allocate initial memory\n");
	exit(1);
    }

    ymax = xmax = -HUGE_VAL;
    ymin = xmin = HUGE_VAL;

    for (i = 1; i < argc; i++) {
	if (*argv[i] == '-') {
	    if (!strcmp(argv[i], "-nl"))
		linetype = StartPoint;
	    else if (argv[i][1] == '\0')	/* use stdin */
		file = argv[i];
	    else {

		fprintf(stderr, "Usage:\n\t %s [options] [file]\n\n", argv[0]);
		fprintf(stderr,
			"where options include:\n"
			"  -pt                   plot with points instead of lines\n\n");

		fprintf(stderr,
			"file name `-' specifies stdin\n"
			"if no file name is specified, "
			"the default is \"%s\"\n\n", DEFAULT_DATA_FILE);

		return EXIT_FAILURE;
	    }
	} else
	    file = argv[i];
    }

    if (file && !strcmp(file, "-")) {
	fd = stdin;
	file = "stdin";
    } else {
	if (file == NULL)
	    file = DEFAULT_DATA_FILE;

	if ((fd = fopen(file, "r")) == NULL) {
	    fprintf(stderr, "%s: can't open file \"%s\"\n", argv[0], file);
	    return EXIT_FAILURE;
	}
    }
    m = 0;
    p = 0;
    while (fgets(line, sizeof(line), fd) != NULL) {
	if (sscanf(line, "%f %f", &x[m], &y[m]) == 2) {
	    if (x[m] > xmax)
		xmax = x[m];
	    else if (x[m] < xmin)
		xmin = x[m];
	    if (y[m] > ymax)
		ymax = y[m];
	    else if (y[m] < ymin)
		ymin = y[m];
	    m++;
	} else {
	    nls[p] = m;
	    p++;
	}
    }
    nls[p++] = m;

    if (m == 0)
	return;

    signal(SIGTERM, nice_end);
    signal(SIGSTOP, nice_end);
    signal(SIGTSTP, nice_end);
    signal(SIGINT, nice_end);
    signal(SIGQUIT, nice_end);
    if (!InitializeGraphics(1))
	return EXIT_FAILURE;

    n = 1;
    do {
	axis_round(&xmin, &xmax, &xgrid_spacing);
	axis_round(&ymin, &ymax, &ygrid_spacing);

	id = CreateWin(0, 0, GRX_SCALE, GRX_SCALE);
	if (id == 0) {
	    fprintf(stderr, "Help id = 0\n");
	    return EXIT_FAILURE;
	}
	/* Fill the window in black for real eye-catching graphics! */
	ForeColor(0);
	StartFill(id);
	FillArea(0, 0, GRX_SCALE, GRX_SCALE);
	Done();

	/* draw outline box in white */
	ForeColor(7);

	/* Draw outline box */
	StartLine(id);
	Extend(1000, 1000);
	Extend(1000, 9000);
	Extend(9000, 9000);
	Extend(9000, 1000);
	Extend(1000, 1000);
	Done();

	/* Draw the data - either lines or dots */
	xdiff = 8000 / (xmax - xmin);
	ydiff = 8000 / (ymax - ymin);

	for (i = j = 0; j < p; j++) {
	    int             n = 0;

	    ForeColor(j % 6 + 1);
	    while (((x[i] < xmin) || (x[i] > xmax) ||
		    (y[i] < ymin) || (y[i] > ymax)) && (i < nls[j]))
		i++;

	    while (i < nls[j]) {
		if (n == 0)
		    linetype(id);
		Extend(1000 + (x[i] - xmin) * xdiff,
		       9000 - (y[i] - ymin) * ydiff);
		n++;
		if (n > 450) {
		    Done();
		    n = 0;
		    continue;
		}
		i++;
		while ((i < nls[j]) &&
		       ((x[i] < xmin) || (x[i] > xmax) ||
			(y[i] < ymin) || (y[i] > ymax)))
		    i++;
	    }
	    if (n > 0)
		Done();
	}

	/* Do axis labels in black */
	ForeColor(7);
	QueryWin(id, &twidth, &theight);
	PlaceText(id, GRX_SCALE / 2, 0, HCENTER_TEXT | TOP_TEXT, file);
	PlaceText(id, GRX_SCALE / 2, GRX_SCALE, HCENTER_TEXT | BOTTOM_TEXT,
		  "X");
	PlaceText(id, 0, GRX_SCALE / 2, LEFT_TEXT | VCENTER_TEXT, "Y");
	sprintf(axis, "%f", ymax);
	nchars = 1000 / twidth;
	axis[nchars] = 0;
	PlaceText(id, GRX_SCALE / 10, GRX_SCALE / 10,
		  RIGHT_TEXT | TOP_TEXT, axis);
	sprintf(axis, "%f", ymin);
	axis[nchars] = 0;
	PlaceText(id, GRX_SCALE / 10, 9 * GRX_SCALE / 10,
		  RIGHT_TEXT | BOTTOM_TEXT, axis);
	sprintf(axis, "%f", xmax);
	PlaceText(id, 9 * GRX_SCALE / 10, 9 * GRX_SCALE / 10,
		  HCENTER_TEXT | TOP_TEXT, axis);
	sprintf(axis, "%f", xmin);
	PlaceText(id, GRX_SCALE / 10, 9 * GRX_SCALE / 10,
		  HCENTER_TEXT | TOP_TEXT, axis);
	fflush(stdout);

	do {
	    n = WaitForCarriageReturn(&winclick, &xclick, &yclick);
	    switch (n) {
	    case 1:
		downx = xclick;
		downy = yclick;
		break;
	    case 2:
		upx = xclick;
		upy = yclick;
		if (upx < downx) {
		    tmp = downx;
		    downx = upx;
		    upx = tmp;
		}
		if (upy < downy) {
		    tmp = downy;
		    downy = upy;
		    upy = tmp;
		}
		xmin = (xmax - xmin) * (downx - 1000) / (8000) + xmin;
		xmax = (xmax - xmin) * (upx - 1000) / (8000) + xmin;
		ymax = ymax - (ymax - ymin) * (downy - 1000) / (8000);
		ymin = ymax - (ymax - ymin) * (upy - 1000) / (8000);
		break;
	    }
	} while (n && (n != 2));
    } while (n);
    nice_end(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

/*----------------------- end-of-file (C source) -----------------------*/
