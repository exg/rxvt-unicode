/*----------------------------------------------------------------------*
 * File:	keyboard.C
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

#include "../config.h"
#include "rxvt.h"

#ifdef KEYSYM_RESOURCE

#include <string.h>

#include "rxvtperl.h"
#include "keyboard.h"

/* an intro to the data structure:
 *
 * vector keymap[] is grouped.
 *
 * inside each group, elements are sorted by the criteria given by compare_priority().
 * the lookup of keysym is done in two steps:
 * 1) locate the group corresponds to the keysym;
 * 2) do a linear search inside the group.
 *
 * array hash[] effectively defines a map from a keysym to a group in keymap[].
 *
 * each group has its address(the index of first group element in keymap[]),
 * which is computed and stored in hash[].
 * hash[] stores the addresses in the form of:
 * index: 0      I1       I2       I3            In
 * value: 0...0, A1...A1, A2...A2, A3...A3, ..., An...An
 * where
 * A1 = 0;
 * Ai+1 = N1 + N2 + ... + Ni.
 * it is computed from hash_bucket_size[]:
 * index: 0      I1         I2         I3             In
 * value: 0...0, N1, 0...0, N2, 0...0, N3,    ...,    Nn, 0...0
 *        0...0, 0.......0, N1.....N1, N1+N2...N1+N2, ... (the computation of hash[])
 * or we can say
 * hash_bucket_size[Ii] = Ni; hash_bucket_size[elsewhere] = 0,
 * where
 * set {I1, I2, ..., In} = { hashkey of keymap[0]->keysym, ..., keymap[keymap.size-1]->keysym }
 * where hashkey of keymap[i]->keysym = keymap[i]->keysym & KEYSYM_HASH_MASK
 *       n(the number of groups) = the number of non-zero member of hash_bucket_size[];
 *       Ni(the size of group i) = hash_bucket_size[Ii].
 */

static void
output_string (rxvt_term *term, const char *str)
{
  if (strncmp (str, "command:", 8) == 0)
    term->cmdbuf_append (str + 8, strlen (str) - 8);
  else if (strncmp (str, "perl:", 5) == 0)
    HOOK_INVOKE((term, HOOK_USER_COMMAND, DT_STR, str + 5, DT_END));
  else
    term->tt_write (str, strlen (str));
}

// return: priority_of_a - priority_of_b
static int
compare_priority (keysym_t *a, keysym_t *b)
{
  // (the more '1's in state; the less range): the greater priority
  int ca = ecb_popcount32 (a->state /* & OtherModMask */);
  int cb = ecb_popcount32 (b->state /* & OtherModMask */);

  if (ca != cb)
    return ca - cb;
//else if (a->state != b->state) // this behavior is to be discussed
//  return b->state - a->state;
  else
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
keyboard_manager::keyboard_manager ()
{
  keymap.reserve (256);
  hash [0] = 1;			// hash[0] != 0 indicates uninitialized data
}

keyboard_manager::~keyboard_manager ()
{
  for (unsigned int i = 0; i < keymap.size (); ++i)
    {
      free (keymap [i]->str);
      delete keymap [i];
    }
}

void
keyboard_manager::register_user_translation (KeySym keysym, unsigned int state, const wchar_t *ws)
{
  char *translation = rxvt_wcstoutf8 (ws);

  keysym_t *key = new keysym_t;

  key->keysym = keysym;
  key->state  = state;
  key->str    = translation;
  key->type   = keysym_t::STRING;

  if (strncmp (translation, "builtin:", 8) == 0)
    key->type = keysym_t::BUILTIN;

  if (keymap.size () == keymap.capacity ())
    keymap.reserve (keymap.size () * 2);

  keymap.push_back (key);
  hash[0] = 3;
}

bool
keyboard_manager::dispatch (rxvt_term *term, KeySym keysym, unsigned int state)
{
  assert (("register_done() need to be called", hash[0] == 0));

  state &= OtherModMask; // mask out uninteresting modifiers

  if (state & term->ModMetaMask)    state |= MetaMask;
  if (state & term->ModNumLockMask) state |= NumLockMask;
  if (state & term->ModLevel3Mask)  state |= Level3Mask;

  if (!!(term->priv_modes & PrivMode_aplKP) != !!(state & ShiftMask))
    state |= AppKeypadMask;

  int index = find_keysym (keysym, state);

  if (index >= 0)
    {
      keysym_t *key = keymap [index];

      if (key->type != keysym_t::BUILTIN)
        {
          wchar_t *ws = rxvt_utf8towcs (key->str);
          char *str = rxvt_wcstombs (ws);
          // TODO: do (some) translations, unescaping etc, here (allow \u escape etc.)
          free (ws);

          output_string (term, str);

          free (str);

          return true;
        }
    }

  return false;
}

void
keyboard_manager::register_done ()
{
  unsigned int i, index, hashkey;
  vector <keysym_t *> sorted_keymap;
  uint16_t hash_bucket_size[KEYSYM_HASH_BUCKETS];	// size of each bucket

  memset (hash_bucket_size, 0, sizeof (hash_bucket_size));

  // determine hash bucket size
  for (i = 0; i < keymap.size (); ++i)
    {
      hashkey = keymap [i]->keysym & KEYSYM_HASH_MASK;
      ++hash_bucket_size [hashkey];
    }

  // now we know the size of each bucket
  // compute the index of each bucket
  hash [0] = 0;
  for (index = 0, i = 1; i < KEYSYM_HASH_BUCKETS; ++i)
    {
      index += hash_bucket_size [i - 1];
      hash [i] = index;
    }

  // and allocate just enough space
  sorted_keymap.insert (sorted_keymap.begin (), index + hash_bucket_size [i - 1], 0);

  memset (hash_bucket_size, 0, sizeof (hash_bucket_size));

  // fill in sorted_keymap
  // it is sorted in each bucket
  for (i = 0; i < keymap.size (); ++i)
    {
      hashkey = keymap [i]->keysym & KEYSYM_HASH_MASK;

      index = hash [hashkey] + hash_bucket_size [hashkey];

      while (index > hash [hashkey]
             && compare_priority (keymap [i], sorted_keymap [index - 1]) > 0)
        {
          sorted_keymap [index] = sorted_keymap [index - 1];
          --index;
        }

      sorted_keymap [index] = keymap [i];
      ++hash_bucket_size [hashkey];
    }

  keymap.swap (sorted_keymap);

#ifndef NDEBUG
  // check for invariants
  for (i = 0; i < KEYSYM_HASH_BUCKETS; ++i)
    {
      index = hash[i];
      for (int j = 0; j < hash_bucket_size [i]; ++j)
        {
          assert (i == (keymap [index + j]->keysym & KEYSYM_HASH_MASK));

          if (j)
            assert (compare_priority (keymap [index + j - 1],
                    keymap [index + j]) >= 0);
        }
    }

  // this should be able to detect most possible bugs
  for (i = 0; i < sorted_keymap.size (); ++i)
    {
      keysym_t *a = sorted_keymap[i];
      int index = find_keysym (a->keysym, a->state);

      assert (index >= 0);
      keysym_t *b = keymap [index];
      assert (i == index	// the normally expected result
              || a->keysym == b->keysym
              && compare_priority (a, b) <= 0);	// is effectively the same or a closer match
    }
#endif
}

int
keyboard_manager::find_keysym (KeySym keysym, unsigned int state)
{
  int hashkey = keysym & KEYSYM_HASH_MASK;
  unsigned int index = hash [hashkey];
  unsigned int end = hashkey < KEYSYM_HASH_BUCKETS - 1
                     ? hash [hashkey + 1]
                     : keymap.size ();

  for (; index < end; ++index)
    {
      keysym_t *key = keymap [index];

      if (key->keysym == keysym
          // match only the specified bits in state and ignore others
          && (key->state & state) == key->state)
        return index;
    }

  return -1;
}

#endif /* KEYSYM_RESOURCE */
// vim:et:ts=2:sw=2
