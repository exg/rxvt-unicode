#include "../../config.h"

#include <stdio.h>
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <sys/wait.h>

int verb = 1;

int
main(int argc)
{
    int             i, pid;
    int             fd_pty, fd_tty;
    const char     *ttydev;

    if (argc != 1)
	verb = 0;
    fd_pty = fd_tty = -1;
    ttydev = NULL;
    fd_pty = rxvt_get_pty(&fd_tty, &ttydev);
    if (fd_pty < 0) {
	fprintf(stderr, "rxvt_get_pty() failed\n");
	exit(1);
    }
    if (verb)
	fprintf(stderr, "rxvt_get_pty() OK.  Found %s (opened pty fd: %d)\n", ttydev, fd_pty);

    if (fd_tty < 0) {
# ifdef HAVE_REVOKE
	revoke(ttydev);
# endif
	if ((fd_tty = rxvt_get_tty(ttydev)) < 0) {
	    fprintf(stderr, "rxvt_get_tty() failed on %s", ttydev);
	    exit(1);
	}
	if (verb)
	    fprintf(stderr, "rxvt_get_tty() OK.  (opened tty fd: %d)\n", fd_tty);
    } else if (verb)
	fprintf(stderr, "Didn't call: rxvt_get_tty() (already have fd)\n");

    for (i = 0; i < 255; i++)
	if (i != fd_pty && i != 2 && i != fd_tty)
	    close(i);
    pid = fork();
    if (pid) {
	int             exitst = 1;
	int             j = 0;

	close(fd_tty);
	for ( ; ++j < 5; ) {
	    if (waitpid(-1, &i, WNOHANG) != 0)
		break;
	    sleep(1);
	}
	if (j == 5)
	    kill(pid, 2);
	if (WIFEXITED(i)) {
	    exitst = WEXITSTATUS(i);
	    if (exitst == 0)
	        fprintf(stderr, "OK\n");
	    else
	        fprintf(stderr, "Failed.  Child exit status: %d\n", WEXITSTATUS(i));
	} else
	    fprintf(stderr, "Interrupted\n");
	exit(exitst);
    }
    close(fd_pty);
    if (verb)
	fprintf(stderr, "Calling: rxvt_control_tty\n");
    i = rxvt_control_tty(fd_tty, ttydev);
    if (i < 0) {
	fprintf(stderr, "could not obtain control of tty\n");
	exit(1);
    }
    exit(0);
    /* NOTREACHED */
}
