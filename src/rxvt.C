#include "../config.h"
#include "rxvt.h"

/*----------------------------------------------------------------------*/
/* main () */
int
main (int argc, const char *const *argv)
try
  {
    rxvt_init_signals ();

    rxvt_term *t = new rxvt_term;

    if (!t->init (argc, argv))
      return EXIT_FAILURE;

    iom.loop ();

    return EXIT_SUCCESS;
  }
catch (const class rxvt_failure_exception &e)
  {
    return EXIT_FAILURE;
  }
