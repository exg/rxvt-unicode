#include "rxvtlib.h"
#include "iom.h"

/*----------------------------------------------------------------------*/
/* main() */
/* INTPROTO */
int
main(int argc, const char *const *argv)
{
  rxvt_init_signals ();

  if (!rxvt_init(argc, argv))
    return EXIT_FAILURE;

  iom.loop ();

  return EXIT_SUCCESS;
}
