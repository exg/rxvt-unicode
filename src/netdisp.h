/* On Solaris link with -lsocket and -lnsl */
#include <sys/types.h>
#include <sys/socket.h>

/* these next two are probably only on Sun (not Solaris) */
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#ifdef HAVE_SYS_BYTEORDER_H
#include <sys/byteorder.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>

#include "netdisp.intpro"	/* PROTOS for internal routines */
