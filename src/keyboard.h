#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "feature.h"
#include "rxvtutil.h"

#ifdef KEYSYM_RESOURCE

#define KEYSYM_HASH_BITS        9       /* lowest #bits of keysym is used as hash key */
#define KEYSYM_HASH_BUDGETS     (1<<KEYSYM_HASH_BITS)
#define KEYSYM_HASH_MASK        (KEYSYM_HASH_BUDGETS-1)

#define MetaMask                0x0100
#define NumLockMask             0x0200
#define AllModMask              (ShiftMask | LockMask | ControlMask \
                                | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)

#if (AllModMask & (MetaMask | NumLockMask)) != 0
#error redefine MetaMask or NumLockMask!
#endif

struct rxvt_term;
struct keysym_t;
typedef void (keyevent_handler) (rxvt_term *rt,
                                 keysym_t *key,
                                 KeySym keysym,
                                 unsigned int state);
typedef unsigned short u16;

struct keysym_t
{
  KeySym            keysym;
  /* only the lower 8 bits of state are used for matching according to X.h */
  /* the higher bits are preserved for Meta/NumLock keys */
  /* which are mapped to corresponding lower bits at register time */
  u16               state;    /* indicates each modifiers' DOWN/UP status         */
  u16               range;    /* =1: single keysym; >1: a of range keysyms        */
  keyevent_handler *handler;  /* event handler                                    */
  const char       *str;      /* would normally be a keycode translation          */
};


class keyboard_manager
{
public:
  keyboard_manager (rxvt_term *rt);
  ~keyboard_manager ();

  void clear ();
  void register_user_translation (KeySym keysym, unsigned int state, const char *trans);
  void register_done ();        // call this to make newly registered keymaps take effect
  bool dispatch (KeySym keysym, unsigned int state);

private:
  void register_keymap (keysym_t *key);
  void purge_duplicate_keymap ();
  void setup_hash ();
  int find_keysym (KeySym keysym, unsigned int state);

private:
  rxvt_term * const term_;

  u16 hash_[KEYSYM_HASH_BUDGETS];               //
  vector<keysym_t *> keymap_;

  // stock keymaps are all static data
  static keysym_t stock_keymap_[];
  // user keymaps and their .string are dynamicly allocated and freed
  vector<keysym_t *> user_keymap_;
  vector<const char *> user_translations_;
};

#endif /* KEYSYM_RESOURCE */
#endif /* KEYBOARD_H_ */
// vim:et:ts=2:sw=2
