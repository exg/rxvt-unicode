/*----------------------------------------------------------------------*
 * File:	keyboard.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005      WU Fengguang
 * Copyright (c) 2005-2006 Marc Lehmann <pcg@goof.com>
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

#include <cstring>

#include "rxvtperl.h"
#include "keyboard.h"
#include "command.h"

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
 * it is computed from hash_budget_size[]:
 * index: 0      I1         I2         I3             In
 * value: 0...0, N1, 0...0, N2, 0...0, N3,    ...,    Nn, 0...0
 *        0...0, 0.......0, N1.....N1, N1+N2...N1+N2, ... (the compution of hash[])
 * or we can say
 * hash_budget_size[Ii] = Ni; hash_budget_size[elsewhere] = 0,
 * where
 * set {I1, I2, ..., In} = { hashkey of keymap[0]->keysym, ..., keymap[keymap.size-1]->keysym }
 * where hashkey of keymap[i]->keysym = keymap[i]->keysym & KEYSYM_HASH_MASK
 *       n(the number of groups) = the number of non-zero member of hash_budget_size[];
 *       Ni(the size of group i) = hash_budget_size[Ii].
 */

#if STOCK_KEYMAP
////////////////////////////////////////////////////////////////////////////////
// default keycode translation map and keyevent handlers

keysym_t keyboard_manager::stock_keymap[] = {
  /* examples */
  /*        keysym,                state, range,                  type,              str */
//{XK_ISO_Left_Tab,                    0,     1,      keysym_t::STRING,           "\033[Z"},
//{            'a',                    0,    26, keysym_t::RANGE_META8,           "a" "%c"},
//{            'a',          ControlMask,    26, keysym_t::RANGE_META8,          "" "%c"},
//{        XK_Left,                    0,     4,        keysym_t::LIST,     ".\033[.DACB."},
//{        XK_Left,            ShiftMask,     4,        keysym_t::LIST,     ".\033[.dacb."},
//{        XK_Left,          ControlMask,     4,        keysym_t::LIST,     ".\033O.dacb."},
//{         XK_Tab,          ControlMask,     1,      keysym_t::STRING,      "\033<C-Tab>"},
//{  XK_apostrophe,          ControlMask,     1,      keysym_t::STRING,        "\033<C-'>"},
//{       XK_slash,          ControlMask,     1,      keysym_t::STRING,        "\033<C-/>"},
//{   XK_semicolon,          ControlMask,     1,      keysym_t::STRING,        "\033<C-;>"},
//{       XK_grave,          ControlMask,     1,      keysym_t::STRING,        "\033<C-`>"},
//{       XK_comma,          ControlMask,     1,      keysym_t::STRING,     "\033<C-\054>"},
//{      XK_Return,          ControlMask,     1,      keysym_t::STRING,   "\033<C-Return>"},
//{      XK_Return,            ShiftMask,     1,      keysym_t::STRING,   "\033<S-Return>"},
//{            ' ',            ShiftMask,     1,      keysym_t::STRING,    "\033<S-Space>"},
//{            '.',          ControlMask,     1,      keysym_t::STRING,        "\033<C-.>"},
//{            '0',          ControlMask,    10,       keysym_t::RANGE,   "0" "\033<C-%c>"},
//{            '0', MetaMask|ControlMask,    10,       keysym_t::RANGE, "0" "\033<M-C-%c>"},
//{            'a', MetaMask|ControlMask,    26,       keysym_t::RANGE, "a" "\033<M-C-%c>"},
};
#endif

static void
output_string (rxvt_term *rt, const char *str)
{
  if (strncmp (str, "command:", 8) == 0)
    rt->cmd_write (str + 8, strlen (str) - 8);
  else if (strncmp (str, "perl:", 5) == 0)
    HOOK_INVOKE((rt, HOOK_USER_COMMAND, DT_STR, str + 5, DT_END));
  else
    rt->tt_write (str, strlen (str));
}

static void
output_string_meta8 (rxvt_term *rt, unsigned int state, char *buf, int buflen)
{
  if (state & rt->ModMetaMask)
    {
#ifdef META8_OPTION
      if (rt->meta_char == 0x80)	/* set 8-bit on */
        {
          for (char *ch = buf; ch < buf + buflen; ch++)
            *ch |= 0x80;
        }
      else if (rt->meta_char == C0_ESC)	/* escape prefix */
#endif
        {
          const char ch = C0_ESC;
          rt->tt_write (&ch, 1);
        }
    }

  rt->tt_write (buf, buflen);
}

static int
format_keyrange_string (const char *str, int keysym_offset, char *buf, int bufsize)
{
  size_t len = snprintf (buf, bufsize, str + 1, keysym_offset + str [0]);

  if (len >= (size_t)bufsize)
    {
      rxvt_warn ("format_keyrange_string: formatting failed, ignoring key.\n");
      *buf = 0;
    }

  return len;
}

// return: priority_of_a - priority_of_b
static int
compare_priority (keysym_t *a, keysym_t *b)
{
  // (the more '1's in state; the less range): the greater priority
  int ca = rxvt_popcount (a->state /* & OtherModMask */);
  int cb = rxvt_popcount (b->state /* & OtherModMask */);

  if (ca != cb)
    return ca - cb;
//else if (a->state != b->state) // this behavior is to be disscussed
//  return b->state - a->state;
  else
    return b->range - a->range;
}

////////////////////////////////////////////////////////////////////////////////
keyboard_manager::keyboard_manager ()
{
  keymap.reserve (256);
  hash [0] = 1;			// hash[0] != 0 indicates uninitialized data
}

keyboard_manager::~keyboard_manager ()
{
  clear ();
}

void
keyboard_manager::clear ()
{
  keymap.clear ();
  hash [0] = 2;

  for (unsigned int i = 0; i < user_translations.size (); ++i)
    {
      free ((void *)user_translations [i]);
      user_translations [i] = 0;
    }

  for (unsigned int i = 0; i < user_keymap.size (); ++i)
    {
      delete user_keymap [i];
      user_keymap [i] = 0;
    }

  user_keymap.clear ();
  user_translations.clear ();
}

// a wrapper for register_keymap,
// so that outside codes don't have to know so much details.
//
// the string 'trans' is copied to an internal managed buffer,
// so the caller can free memory of 'trans' at any time.
void
keyboard_manager::register_user_translation (KeySym keysym, unsigned int state, const char *trans)
{
  keysym_t *key = new keysym_t;
  wchar_t *wc = rxvt_mbstowcs (trans);
  char *translation = rxvt_wcstoutf8 (wc);
  free (wc);

  if (key && translation)
    {
      key->keysym = keysym;
      key->state  = state;
      key->range  = 1;
      key->str    = translation;
      key->type   = keysym_t::STRING;

      if (strncmp (translation, "list", 4) == 0 && translation [4])
        {
          char *middle = strchr  (translation + 5, translation [4]);
          char *suffix = strrchr (translation + 5, translation [4]);

          if (suffix && middle && suffix > middle + 1)
            {
              key->type  = keysym_t::LIST;
              key->range = suffix - middle - 1;

              memmove (translation, translation + 4, strlen (translation + 4) + 1);
            }
          else
            rxvt_warn ("cannot parse list-type keysym '%s', treating as normal keysym.\n", translation);
        }
      else if (strncmp (translation, "builtin:", 8) == 0)
        key->type = keysym_t::BUILTIN;

      user_keymap.push_back (key);
      user_translations.push_back (translation);
      register_keymap (key);
    }
  else
    {
      delete key;
      free ((void *)translation);
      rxvt_fatal ("out of memory, aborting.\n");
    }
}

void
keyboard_manager::register_keymap (keysym_t *key)
{
  if (keymap.size () == keymap.capacity ())
    keymap.reserve (keymap.size () * 2);

  keymap.push_back (key);
  hash[0] = 3;
}

void
keyboard_manager::register_done ()
{
#if STOCK_KEYMAP
  int n = sizeof (stock_keymap) / sizeof (keysym_t);

  //TODO: shield against repeated calls and empty keymap
  //if (keymap.back () != &stock_keymap[n - 1])
    for (int i = 0; i < n; ++i)
      register_keymap (&stock_keymap[i]);
#endif

  purge_duplicate_keymap ();

  setup_hash ();
}

bool
keyboard_manager::dispatch (rxvt_term *term, KeySym keysym, unsigned int state)
{
  assert (hash[0] == 0 && "register_done() need to be called");

  state &= OtherModMask; // mask out uninteresting modifiers

  if (state & term->ModMetaMask)    state |= MetaMask;
  if (state & term->ModNumLockMask) state |= NumLockMask;
  if (state & term->ModLevel3Mask)  state |= Level3Mask;

  if (!!(term->priv_modes & PrivMode_aplKP) != !!(state & ShiftMask))
    state |= AppKeypadMask;

  int index = find_keysym (keysym, state);

  if (index >= 0)
    {
      const keysym_t &key = *keymap [index];

      if (key.type != keysym_t::BUILTIN)
        {
          int keysym_offset = keysym - key.keysym;

          wchar_t *wc = rxvt_utf8towcs (key.str);
          char *str = rxvt_wcstombs (wc);
          // TODO: do (some) translations, unescaping etc, here (allow \u escape etc.)
          free (wc);

          switch (key.type)
            {
              case keysym_t::STRING:
                output_string (term, str);
                break;

              case keysym_t::RANGE:
                {
                  char buf[STRING_MAX];

                  if (format_keyrange_string (str, keysym_offset, buf, sizeof (buf)) > 0)
                    output_string (term, buf);
                }
                break;

              case keysym_t::RANGE_META8:
                {
                  int len;
                  char buf[STRING_MAX];

                  len = format_keyrange_string (str, keysym_offset, buf, sizeof (buf));
                  if (len > 0)
                    output_string_meta8 (term, state, buf, len);
                }
                break;

              case keysym_t::LIST:
                {
                  char buf[STRING_MAX];

                  char *prefix, *middle, *suffix;

                  prefix = str;
                  middle = strchr  (prefix + 1, *prefix);
                  suffix = strrchr (middle + 1, *prefix);

                  memcpy (buf, prefix + 1, middle - prefix - 1);
                  buf [middle - prefix - 1] = middle [keysym_offset + 1];
                  strcpy (buf + (middle - prefix), suffix + 1);

                  output_string (term, buf);
                }
                break;
            }

          free (str);

          return true;
        }
    }

  return false;
}

// purge duplicate keymap entries
void keyboard_manager::purge_duplicate_keymap ()
{
  for (unsigned int i = 0; i < keymap.size (); ++i)
    {
      for (unsigned int j = 0; j < i; ++j)
        {
          if (keymap [i] == keymap [j])
            {
              while (keymap [i] == keymap.back ())
                keymap.pop_back ();

              if (i < keymap.size ())
                {
                  keymap[i] = keymap.back ();
                  keymap.pop_back ();
                }

              break;
            }
        }
    }
}

void
keyboard_manager::setup_hash ()
{
  unsigned int i, index, hashkey;
  vector <keysym_t *> sorted_keymap;
  uint16_t hash_budget_size[KEYSYM_HASH_BUDGETS];	// size of each budget
  uint16_t hash_budget_counter[KEYSYM_HASH_BUDGETS];	// #elements in each budget

  memset (hash_budget_size, 0, sizeof (hash_budget_size));
  memset (hash_budget_counter, 0, sizeof (hash_budget_counter));

  // determine hash bucket size
  for (i = 0; i < keymap.size (); ++i)
    for (int j = min (keymap [i]->range, KEYSYM_HASH_BUDGETS) - 1; j >= 0; --j)
      {
        hashkey = (keymap [i]->keysym + j) & KEYSYM_HASH_MASK;
        ++hash_budget_size [hashkey];
      }

  // now we know the size of each budget
  // compute the index of each budget
  hash [0] = 0;
  for (index = 0, i = 1; i < KEYSYM_HASH_BUDGETS; ++i)
    {
      index += hash_budget_size [i - 1];
      hash [i] = index;
    }

  // and allocate just enough space
  sorted_keymap.insert (sorted_keymap.begin (), index + hash_budget_size [i - 1], 0);

  // fill in sorted_keymap
  // it is sorted in each budget
  for (i = 0; i < keymap.size (); ++i)
    for (int j = min (keymap [i]->range, KEYSYM_HASH_BUDGETS) - 1; j >= 0; --j)
      {
        hashkey = (keymap [i]->keysym + j) & KEYSYM_HASH_MASK;

        index = hash [hashkey] + hash_budget_counter [hashkey];

        while (index > hash [hashkey]
               && compare_priority (keymap [i], sorted_keymap [index - 1]) > 0)
          {
            sorted_keymap [index] = sorted_keymap [index - 1];
            --index;
          }

        sorted_keymap [index] = keymap [i];
        ++hash_budget_counter [hashkey];
      }

  keymap.swap (sorted_keymap);

#ifndef NDEBUG
  // check for invariants
  for (i = 0; i < KEYSYM_HASH_BUDGETS; ++i)
    {
      index = hash[i];
      for (int j = 0; j < hash_budget_size [i]; ++j)
        {
          if (keymap [index + j]->range == 1)
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
      for (int j = 0; j < a->range; ++j)
        {
          int index = find_keysym (a->keysym + j, a->state);

          assert (index >= 0);
          keysym_t *b = keymap [index];
          assert (i == index	// the normally expected result
                  || IN_RANGE_INC (a->keysym + j, b->keysym, b->keysym + b->range)
                  && compare_priority (a, b) <= 0);	// is effectively the same or a closer match
        }
    }
#endif
}

int
keyboard_manager::find_keysym (KeySym keysym, unsigned int state)
{
  int hashkey = keysym & KEYSYM_HASH_MASK;
  unsigned int index = hash [hashkey];
  unsigned int end = hashkey < KEYSYM_HASH_BUDGETS - 1
                     ? hash [hashkey + 1]
                     : keymap.size ();

  for (; index < end; ++index)
    {
      keysym_t *key = keymap [index];

      if (key->keysym <= keysym && keysym < key->keysym + key->range
          // match only the specified bits in state and ignore others
          && (key->state & state) == key->state)
        return index;
    }

  return -1;
}

#endif /* KEYSYM_RESOURCE */
// vim:et:ts=2:sw=2
