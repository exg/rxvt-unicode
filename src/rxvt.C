#include "rxvtlib.h"
#include "iom.h"

/*----------------------------------------------------------------------*/
/* main() */
/* INTPROTO */
int
main(int argc, const char *const *argv)
{
    if (rxvt_init(argc, argv) == NULL)
	return EXIT_FAILURE;

    iom.loop ();

    return EXIT_SUCCESS;
}
