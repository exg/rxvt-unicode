/*----------------------------------------------------------------------*
 * File:	keyboard.h
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005      WU Fengguang
 * Copyright (c) 2005-2006 Marc Lehmann <schmorp@schmorp.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *----------------------------------------------------------------------*/

#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#ifdef KEYSYM_RESOURCE

#include <inttypes.h>

#include "rxvtutil.h"

#define KEYSYM_HASH_BITS        4       /* lowest #bits of keysym is used as hash key */
#define KEYSYM_HASH_BUCKETS     (1<<KEYSYM_HASH_BITS)
#define KEYSYM_HASH_MASK        (KEYSYM_HASH_BUCKETS-1)

#define MetaMask                0x0100
#define NumLockMask             0x0200
#define AppKeypadMask           0x0400
#define Level3Mask              0x0800 // currently not supported
#define OtherModMask            (ShiftMask | LockMask | ControlMask \
                                | Mod1Mask | Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask)

#if OtherModMask > 0xff
# error FATAL: X modifiers might clash with rxvt-unicode ones
#endif

struct rxvt_term;

struct keysym_t
{
  enum keysym_type {
    STRING, BUILTIN,
  };

  KeySym      keysym;
  /* only the lower 8 bits of state are used for matching according to X.h */
  /* the higher bits are preserved for Meta/NumLock keys */
  /* which are mapped to corresponding lower bits at register time */
  uint16_t    state;    /* indicates each modifiers' DOWN/UP status         */
  keysym_type type;
  char        *str;      /* the key's definition encoded in UTF-8 */
};

class keyboard_manager
{
public:
  keyboard_manager ();
  ~keyboard_manager ();

  void register_user_translation (KeySym keysym, unsigned int state, const wchar_t *ws);
  void register_done ();        // call this to make newly registered key bindings take effect
  bool dispatch (rxvt_term *term, KeySym keysym, unsigned int state);

private:
  int find_keysym (KeySym keysym, unsigned int state);

private:
  uint16_t hash[KEYSYM_HASH_BUCKETS];
  vector<keysym_t *> keymap;
};

#endif /* KEYSYM_RESOURCE */

#endif /* KEYBOARD_H_ */
