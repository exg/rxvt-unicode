#include "rxvtlib.h"

/*----------------------------------------------------------------------*/
/* main() */
/* INTPROTO */
int
main(int argc, const char *const *argv)
{
    if (rxvt_init(argc, argv) == NULL)
	return EXIT_FAILURE;

    dR;
    rxvt_main_loop(aR);	/* main processing loop */
    return EXIT_SUCCESS;
}
