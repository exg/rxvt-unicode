/*
 * rxvtperl.h
 */

#ifndef RXVTPERL_H_
#define RXVTPERL_H_

#if ENABLE_PERL
# define SHOULD_INVOKE(htype) rxvt_perl.should_invoke [htype]
# define HOOK_INVOKE(args) rxvt_perl.invoke args

#include "rxvt.h"

enum data_type {
  DT_END,
  DT_INT,
  DT_LONG,
  DT_STRING,
  DT_STRING_LEN,
  DT_USTRING_LEN,
  DT_XEVENT,
};

enum hook_type {
# define def(sym) HOOK_ ## sym,
# include "hookinc.h"
# undef def
  HOOK_NUM,
};

struct rxvt_perl_interp
{
  rxvt_perl_interp ();
  ~rxvt_perl_interp ();

  bool should_invoke[HOOK_NUM];

  void init ();
  bool invoke (rxvt_term *term, hook_type htype, ...);
  void line_update (rxvt_term *term);
};

extern struct rxvt_perl_interp rxvt_perl;

#else
# define SHOULD_INVOKE(htype) false
# define HOOK_INVOKE(args) false
#endif

#endif

