#include "../config.h"
#include "rxvt.h"
#include "keyboard.h"
#include "command.h"
#include <string.h>
#include <X11/X.h>

#ifdef KEYSYM_RESOURCE

////////////////////////////////////////////////////////////////////////////////
// default keycode translation map and keyevent handlers

keysym_t keyboard_manager::stock_keymap[] = {
  /* examples */
  /*        keysym,                state, range,        handler,             str */
//{XK_ISO_Left_Tab,                    0,     1,         NORMAL,           "\033[Z"},
//{            'a',                    0,    26,    RANGE_META8,           "a" "%c"},
//{            'a',          ControlMask,    26,    RANGE_META8,          "" "%c"},
//{        XK_Left,                    0,     4,           LIST,   "DACBZ" "\033[Z"},
//{        XK_Left,            ShiftMask,     4,           LIST,   "dacbZ" "\033[Z"},
//{        XK_Left,          ControlMask,     4,           LIST,   "dacbZ" "\033OZ"},
//{         XK_Tab,          ControlMask,     1,         NORMAL,      "\033<C-Tab>"},
//{  XK_apostrophe,          ControlMask,     1,         NORMAL,        "\033<C-'>"},
//{       XK_slash,          ControlMask,     1,         NORMAL,        "\033<C-/>"},
//{   XK_semicolon,          ControlMask,     1,         NORMAL,        "\033<C-;>"},
//{       XK_grave,          ControlMask,     1,         NORMAL,        "\033<C-`>"},
//{       XK_comma,          ControlMask,     1,         NORMAL,     "\033<C-\054>"},
//{      XK_Return,          ControlMask,     1,         NORMAL,    "\033<C-Return>"},
//{      XK_Return,            ShiftMask,     1,         NORMAL,    "\033<S-Return>"},
//{            ' ',            ShiftMask,     1,         NORMAL,    "\033<S-Space>"},
//{            '.',          ControlMask,     1,         NORMAL,        "\033<C-.>"},
//{            '0',          ControlMask,    10,          RANGE,   "0" "\033<C-%c>"},
//{            '0', MetaMask|ControlMask,    10,          RANGE, "0" "\033<M-C-%c>"},
//{            'a', MetaMask|ControlMask,    26,          RANGE, "a" "\033<M-C-%c>"},
};

static void
output_string (rxvt_term *rt, const char *str)
{
  assert (rt && str);

  if (strncmp (str, "proto:", 6) == 0)
    rt->cmd_write ((unsigned char *)str + 6, strlen (str) - 6);
  else
    rt->tt_write ((unsigned char *)str, strlen (str));
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
          const unsigned char
            ch = C0_ESC;
          rt->tt_write (&ch, 1);
        }
    }

  rt->tt_write ((unsigned char *) buf, buflen);
}

static int
format_keyrange_string (const char *str, int keysym_offset, char *buf, int bufsize)
{
  int len = snprintf (buf, bufsize, str + 1, keysym_offset + str [0]);

  if (len >= bufsize)
    {
      fprintf (stderr, "buffer overflowed!\n");
      buf[bufsize - 1] = '\0';
    }
  else if (len < 0)
    {
      perror ("keyrange_translator()");
    }

  return len;
}

////////////////////////////////////////////////////////////////////////////////
// return: #bits of '1'
static int
bitcount (unsigned int n)
{
  int i;

  for (i = 0; n; ++i, n &= (n - 1))
    ;

  return i;
}

// return: priority_of_a - priority_of_b
static int
compare_priority (keysym_t *a, keysym_t *b)
{
  assert (a && b);

  // (the more '1's in state; the less range): the greater priority
  int ca = bitcount (a->state /* & OtherModMask */);
  int cb = bitcount (b->state /* & OtherModMask */);

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
  assert (trans);

  keysym_t *key = new keysym_t;
  wchar_t *wc = rxvt_mbstowcs (trans);
  const char *translation = rxvt_wcstoutf8 (wc);
  free (wc);

  if (key && translation)
    {
      key->keysym = keysym;
      key->state  = state;
      key->range  = 1;
      key->str    = translation;
      key->type   = keysym_t::NORMAL;

      if (strncmp (translation, "list", 4) == 0 && translation [4])
        {
          char *middle = strchr  (translation + 5, translation [4]);
          char *suffix = strrchr (translation + 5, translation [4]);
          
          if (suffix && middle && suffix > middle + 1)
            {
              key->type  = keysym_t::LIST;
              key->range = suffix - middle - 1;

              strcpy (translation, translation + 4);
            }
          else
            {
              key->range = 1;
              rxvt_warn ("cannot parse list-type keysym '%s', treating as normal keysym.\n", translation);
            }
        }
      else

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
  assert (key);
  assert (key->range >= 1);

  if (keymap.size () == keymap.capacity ())
    keymap.reserve (keymap.size () * 2);

  keymap.push_back (key);
  hash[0] = 3;
}

void
keyboard_manager::register_done ()
{
  unsigned int i, n = sizeof (stock_keymap) / sizeof (keysym_t);

  if (keymap.back () != &stock_keymap[n - 1])
    for (i = 0; i < n; ++i)
      register_keymap (&stock_keymap[i]);

  purge_duplicate_keymap ();

  setup_hash ();
}

bool
keyboard_manager::dispatch (rxvt_term *term, KeySym keysym, unsigned int state)
{
  assert (hash[0] == 0 && "register_done() need to be called");

  if (state & term->ModMetaMask)
    state |= MetaMask;

  if (state & term->ModNumLockMask)
    state |= NumLockMask;

  if (!!(term->priv_modes & PrivMode_aplKP) != !!(state & ShiftMask))
    state |= AppKeypadMask;

  int index = find_keysym (keysym, state);

  if (index >= 0)
    {
      assert (term && keymap [index]);
      const keysym_t &key = *keymap [index];

      int keysym_offset = keysym - key.keysym;

      wchar_t *wc = rxvt_utf8towcs (key.str);
      char *str = rxvt_wcstombs (wc);
      // TODO: do translations, unescaping etc, here (allow \u escape etc.)
      free (wc);

      switch (key.type)
        {
         case keysym_t::NORMAL:
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
  else
    {
      // fprintf(stderr,"[%x:%x]",state,keysym);
      return false;
    }
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

  // count keysyms for corresponding hash budgets
  for (i = 0; i < keymap.size (); ++i)
    {
      assert (keymap [i]);
      hashkey = (keymap [i]->keysym & KEYSYM_HASH_MASK);
      ++hash_budget_size [hashkey];
    }

  // keysym A with range>1 is counted one more time for
  // every keysym B lies in its range
  for (i = 0; i < keymap.size (); ++i)
    {
      if (keymap[i]->range > 1)
        {
          for (int j = min (keymap [i]->range, KEYSYM_HASH_BUDGETS) - 1; j > 0; --j)
            {
              hashkey = ((keymap [i]->keysym + j) & KEYSYM_HASH_MASK);
              if (hash_budget_size [hashkey])
                ++hash_budget_size [hashkey];
            }
        }
    }

  // now we know the size of each budget
  // compute the index of each budget
  hash [0] = 0;
  for (index = 0, i = 1; i < KEYSYM_HASH_BUDGETS; ++i)
    {
      index += hash_budget_size [i - 1];
      hash[i] = (hash_budget_size [i] ? index : hash [i - 1]);
    }

  // and allocate just enough space
  //sorted_keymap.reserve (hash[i - 1] + hash_budget_size[i - 1]);
  sorted_keymap.insert (sorted_keymap.begin (), index + hash_budget_size [i - 1], 0);

  // fill in sorted_keymap
  // it is sorted in each budget
  for (i = 0; i < keymap.size (); ++i)
    {
      for (int j = min (keymap [i]->range, KEYSYM_HASH_BUDGETS) - 1; j >= 0; --j)
        {
          hashkey = ((keymap [i]->keysym + j) & KEYSYM_HASH_MASK);

          if (hash_budget_size [hashkey])
            {
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
        }
    }

  keymap.swap (sorted_keymap);

#if defined (DEBUG_STRICT) || defined (DEBUG_KEYBOARD)
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
          int index = find_keysym (a->keysym + j, a->state & OtherModMask);
          assert (index >= 0);
          keysym_t *b = keymap [index];
          assert (i == (signed) index ||	// the normally expected result
            (a->keysym + j) >= b->keysym && (a->keysym + j) <= (b->keysym + b->range) && compare_priority (a, b) <= 0);	// is effectively the same
        }
    }
#endif
}

int
keyboard_manager::find_keysym (KeySym keysym, unsigned int state)
{
  int hashkey = keysym & KEYSYM_HASH_MASK;
  unsigned int index = hash [hashkey];

  for (; index < keymap.size (); ++index)
    {
      keysym_t *key = keymap [index];
      assert (key);

      if (key->keysym <= keysym && key->keysym + key->range > keysym
          // match only the specified bits in state and ignore others
          && (key->state & OtherModMask) == (key->state & state))
        return index;
      else if (key->keysym > keysym && key->range == 1)
        return -1;
    }

  return -1;
}

#endif /* KEYSYM_RESOURCE */
// vim:et:ts=2:sw=2
