#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include <inttypes.h>

#include "feature.h"
#include "rxvtutil.h"

#ifdef KEYSYM_RESOURCE

#define KEYSYM_HASH_BITS        4       /* lowest #bits of keysym is used as hash key */
#define KEYSYM_HASH_BUDGETS     (1<<KEYSYM_HASH_BITS)
#define KEYSYM_HASH_MASK        (KEYSYM_HASH_BUDGETS-1)

#define MetaMask                0x0100
#define NumLockMask             0x0200
#define AppKeypadMask           0x0400
#define OtherModMask            (ShiftMask | LockMask | ControlMask \
                                | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)

#if (OtherModMask & (MetaMask | NumLockMask | AppKeypadMask)) != 0
# error FATAL: MetaMask, NumLockMask and/or AppKeypadMask clashes with X modifiers!
#endif

struct rxvt_term;
struct keysym_t;

typedef void (keyevent_handler) (rxvt_term *rt,
                                 keysym_t *key,
                                 KeySym keysym,
                                 unsigned int state);

struct keysym_t
{
  enum keysym_type {
    NORMAL, RANGE, RANGE_META8, LIST
  };

  KeySym      keysym;
  /* only the lower 8 bits of state are used for matching according to X.h */
  /* the higher bits are preserved for Meta/NumLock keys */
  /* which are mapped to corresponding lower bits at register time */
  uint16_t    state;    /* indicates each modifiers' DOWN/UP status         */
  uint16_t    range;    /* =1: single keysym; >1: a of range keysyms        */
  keysym_type type;
  const char  *str;      /* would normally be a keycode translation in UTF-8 */
};

class keyboard_manager
{
public:
  keyboard_manager ();
  ~keyboard_manager ();

  void clear ();
  void register_user_translation (KeySym keysym, unsigned int state, const char *trans);
  void register_done ();        // call this to make newly registered keymaps take effect
  bool dispatch (rxvt_term *term, KeySym keysym, unsigned int state);

private:
  void register_keymap (keysym_t *key);
  void purge_duplicate_keymap ();
  void setup_hash ();
  int find_keysym (KeySym keysym, unsigned int state);

private:
  uint16_t hash[KEYSYM_HASH_BUDGETS];
  vector<keysym_t *> keymap;

  // stock keymaps are all static data
  static keysym_t stock_keymap[];
  // user keymaps and their .string are dynamicaly allocated and freed
  vector<keysym_t *> user_keymap;
  vector<const char *> user_translations;
};

#endif /* KEYSYM_RESOURCE */
#endif /* KEYBOARD_H_ */
// vim:et:sw=2
