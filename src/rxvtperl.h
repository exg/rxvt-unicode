/*
 * rxvtperl.h
 */

#ifndef RXVTPERL_H_
#define RXVTPERL_H_

#if ENABLE_PERL

#include "rxvt.h"

#define PERL_INVOKE(args) rxvt_perl.invoke args

enum data_type {
  DT_END,
  DT_INT,
  DT_LONG,
  DT_STRING,
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
};

extern struct rxvt_perl_interp rxvt_perl;

#else
# define PERL_INVOKE(args) false
#endif

#endif

