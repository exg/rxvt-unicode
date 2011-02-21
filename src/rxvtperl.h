/*
 * rxvtperl.h
 */

#ifndef RXVTPERL_H_
#define RXVTPERL_H_

#if ENABLE_PERL

#define SHOULD_INVOKE(htype) \
   (  (htype) == HOOK_INIT		\
   || (htype) == HOOK_DESTROY		\
   || (htype) == HOOK_REFRESH_BEGIN	\
   || (htype) == HOOK_REFRESH_END	\
   || perl.should_invoke [htype])
#define HOOK_INVOKE(args) rxvt_perl.invoke args

enum data_type {
  DT_END,     // no further arguments
  DT_INT,
  DT_LONG,
  DT_STR,     // 0-terminated string
  DT_STR_LEN, // string + length
  DT_WCS_LEN, // wchar_t* + length
  DT_LCS_LEN, // long* + length
  DT_XEVENT,
};

enum hook_type {
# define def(sym) HOOK_ ## sym,
# include "hookinc.h"
# undef def
  HOOK_NUM,
};

struct rxvt_perl_term
{
  void *self;
  unsigned long grabtime;
  uint8_t should_invoke[HOOK_NUM];
};

struct rxvt_perl_interp
{
  char **perl_environ;

  ~rxvt_perl_interp ();

  void init (rxvt_term *term);
  bool invoke (rxvt_term *term, hook_type htype, ...);
  void line_update (rxvt_term *term);
  void selection_finish (rxvt_selection *sel, char *data, unsigned int len);
};

extern struct rxvt_perl_interp rxvt_perl;

#else

#define SHOULD_INVOKE(htype) false
#define HOOK_INVOKE(args) false

#endif

#endif

