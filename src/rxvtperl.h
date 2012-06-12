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

  // this object must be zero-initialised
  rxvt_perl_term ()
  {
    should_invoke [HOOK_INIT]     =
    should_invoke [HOOK_DESTROY]  = 1;
  }
};

struct rxvt_perl_interp
{
  char **perl_environ;

  ~rxvt_perl_interp ();

  void init ();
  void init (rxvt_term *term);
  void eval (const char *str);
  bool invoke (rxvt_term *term, hook_type htype, ...);
  void line_update (rxvt_term *term);
  void selection_finish (rxvt_selection *sel, char *data, unsigned int len);
  void usage (rxvt_term *term, int type);

  enum
  {
    RESOURCE_AVAILABLE = 1, // whether the option is valid
    RESOURCE_ARG       = 2  // whether the option eats the next arg
  };
  uint8_t parse_resource (rxvt_term *term, const char *name, bool arg, bool longopt, bool flag, const char *value);
};

extern struct rxvt_perl_interp rxvt_perl;

#else

#define SHOULD_INVOKE(htype) false
#define HOOK_INVOKE(args) false

#endif

#endif

