#include "../config.h"
#include "rxvt.h"
#include "keyboard.h"
#include "command.h"
#include <string.h>
#include <X11/X.h>

#ifdef KEYSYM_RESOURCE

////////////////////////////////////////////////////////////////////////////////
// default keycode translation map and keyevent handlers
keyevent_handler keysym_translator;
keyevent_handler keyrange_translator;
keyevent_handler keyrange_translator_meta8;
keyevent_handler keylist_translator;

keysym_t keyboard_manager::stock_keymap_[] =
{
  /* examples */
  /*        keysym,                state, range,                   handler,             str*/
//{XK_ISO_Left_Tab,                    0,     1,         keysym_translator,           "\033[Z"},
//{            'a',                    0,    26, keyrange_translator_meta8,           "a" "%c"},
//{            'a',          ControlMask,    26, keyrange_translator_meta8,          "" "%c"},
//{        XK_Left,                    0,     4,        keylist_translator,   "DACBZ" "\033[Z"},
//{        XK_Left,            ShiftMask,     4,        keylist_translator,   "dacbZ" "\033[Z"},
//{        XK_Left,          ControlMask,     4,        keylist_translator,   "dacbZ" "\033OZ"},
//{         XK_Tab,          ControlMask,     1,         keysym_translator,      "\033<C-Tab>"},
//{  XK_apostrophe,          ControlMask,     1,         keysym_translator,        "\033<C-'>"},
//{       XK_slash,          ControlMask,     1,         keysym_translator,        "\033<C-/>"},
//{   XK_semicolon,          ControlMask,     1,         keysym_translator,        "\033<C-;>"},
//{       XK_grave,          ControlMask,     1,         keysym_translator,        "\033<C-`>"},
//{       XK_comma,          ControlMask,     1,         keysym_translator,     "\033<C-\054>"},
//{      XK_Return,          ControlMask,     1,         keysym_translator,    "\033<C-Return>"},
//{      XK_Return,            ShiftMask,     1,         keysym_translator,    "\033<S-Return>"},
//{            ' ',            ShiftMask,     1,         keysym_translator,    "\033<S-Space>"},
//{            '.',          ControlMask,     1,         keysym_translator,        "\033<C-.>"},
//{            '0',          ControlMask,    10,       keyrange_translator,   "0" "\033<C-%c>"},
//{            '0', MetaMask|ControlMask,    10,       keyrange_translator, "0" "\033<M-C-%c>"},
//{            'a', MetaMask|ControlMask,    26,       keyrange_translator, "a" "\033<M-C-%c>"},
};

void output_string (rxvt_term *rt,
                    const char *str)
{
  assert (rt && str);
  if (strncmp (str, "proto:", 6) == 0)
    rt->cmd_write ((unsigned char*)str + 6, strlen (str) - 6);
  else
    rt->tt_write ((unsigned char*)str, strlen (str));
}

void output_string_meta8 (rxvt_term *rt,
                          unsigned int state,
                          char buf[],
                          int buflen)
{
  if (state & rt->ModMetaMask)
    {
#ifdef META8_OPTION
      if(rt->meta_char == 0x80) /* set 8-bit on */
        {
          for (char *ch = buf; ch < buf + buflen; ch++)
            *ch |= 0x80;
        }
      else if(rt->meta_char == C0_ESC) /* escape prefix */
#endif
        {
          const unsigned char ch = C0_ESC;
          rt->tt_write (&ch, 1);
        }
    }

  rt->tt_write ((unsigned char*)buf, buflen);
}

int format_keyrange_string (keysym_t *key,
                            KeySym keysym,
                            char buf[],
                            int bufsize)
{
  int len;

  assert (key->str);
  len = snprintf (buf, bufsize, key->str + 1,
                  keysym - key->keysym + (int)(key->str[0]));
  if (len >= bufsize)
    {
      fprintf (stderr, "buffer overflowed!\n");
      buf[bufsize-1] = '\0';
    }
  else if (len < 0)
    {
      perror("keyrange_translator()");
    }

  return len;
}

bool format_keylist_string (keysym_t *key,
                            KeySym keysym,
                            char buf[],
                            int bufsize)
{
  char *p;

  assert (key->str);
  strncpy (buf, key->str + key->range + 1, bufsize);
  buf[bufsize-1] = '\0';

  p = strchr (buf, key->str[key->range]);
  if (p)
    {
      *p = key->str[keysym - key->keysym];
      return true;
    }
  else
    {
      fprintf (stderr, "invalid str for keylist_translator()!\n");
      return false;
    }
}

/////////////////////////////////////////////////////////////////
void keysym_translator (rxvt_term *rt,
                        keysym_t *key,
                        KeySym keysym,
                        unsigned int state)
{
  output_string (rt, key->str);
}

void keyrange_translator (rxvt_term *rt,
                          keysym_t *key,
                          KeySym keysym,
                          unsigned int state)
{
  char buf[STRING_MAX];

  if (format_keyrange_string (key, keysym, buf, sizeof(buf)) > 0)
    output_string (rt, buf);
}

void keyrange_translator_meta8 (rxvt_term *rt,
                                keysym_t *key,
                                KeySym keysym,
                                unsigned int state)
{
  int len;
  char buf[STRING_MAX];

  len = format_keyrange_string (key, keysym, buf, sizeof(buf));
  if (len > 0)
    output_string_meta8 (rt, state, buf, len);
}

void keylist_translator (rxvt_term *rt,
                         keysym_t *key,
                         KeySym keysym,
                         unsigned int state)
{
  char buf[STRING_MAX];

  format_keylist_string (key, keysym, buf, sizeof(buf));
  output_string (rt, buf);
}

////////////////////////////////////////////////////////////////////////////////
// return: #bits of '1'
int
bitcount (unsigned int n)
{
  int i;
  for (i = 0; n; ++i, n &= (n - 1));
  return i;
}

// return: priority_of_a - priority_of_b
int
compare_priority (keysym_t *a, keysym_t *b)
{
  assert (a && b);

  // (the more '1's in state; the less range): the greater priority
  int ca = bitcount (a->state/* & AllModMask*/);
  int cb = bitcount (b->state/* & AllModMask*/);
  if (ca != cb)
    return ca - cb;
//else if (a->state != b->state) // this behavior is to be disscussed
//  return b->state - a->state;
  else
    return b->range - a->range;
}

////////////////////////////////////////////////////////////////////////////////
keyboard_manager::keyboard_manager (rxvt_term *rt)
  :term_(rt)
{
  keymap_.reserve (256);
  hash_[0] = 1; // hash_[0] != 0 indicates uninitialized data
}

keyboard_manager::~keyboard_manager ()
{
  clear ();
}

void
keyboard_manager::clear ()
{
  keymap_.clear ();
  hash_[0] = 2;

  for(unsigned int i = 0;i < user_translations_.size();++i)
    {
      delete[] user_translations_[i];
      user_translations_[i] = 0;
    }

  for(unsigned int i = 0;i < user_keymap_.size();++i)
    {
      delete user_keymap_[i];
      user_keymap_[i] = 0;
    }

  user_keymap_.clear();
  user_translations_.clear();
}

// a wrapper for register_keymap,
// so that outside codes don't have to know so much details.
//
// the string 'trans' is copied to an internal managed buffer,
// so the caller can free memory of 'trans' at any time.
void
keyboard_manager::register_user_translation (KeySym keysym,
                                             unsigned int state,
                                             const char *trans)
{
  assert(trans);

  keysym_t *key = new keysym_t;
  const char *translation = new char[1+strlen(trans)];

  if(key && translation)
    {
      key->keysym = keysym;
      key->state = state;
      key->range = 1;
      key->str = translation;

      if (strncmp (trans, "list", 4) == 0)
        {
          const char *p = &trans[4];
          if (*p && (p = strchr (p+1, *p)))
            if ((p - trans - 5 > 1) && strchr (p+1, *p))
              {
                strcpy (translation, trans+5);
                key->range = p - trans - 5;
                key->handler = keylist_translator;
              }
        }
      if (key->range == 1)
        {
          strcpy (translation, trans);
          key->handler = keysym_translator;
        }

      user_keymap_.push_back (key);
      user_translations_.push_back (translation);
      register_keymap (key);
    }
  else
    {
      delete key;
      delete[] translation;
      rxvt_fatal ("out of memory, aborting.\n");
    }
}

void
keyboard_manager::register_keymap (keysym_t *key)
{
  assert (key);
  assert (key->handler);
  assert (key->range >= 1);

  if (keymap_.size () == keymap_.capacity ())
    keymap_.reserve (keymap_.size () * 2);

  keymap_.push_back (key);
  hash_[0] = 3;
}

void
keyboard_manager::register_done ()
{
  unsigned int i, n = sizeof(stock_keymap_)/sizeof(keysym_t);

  if(keymap_.back() != &stock_keymap_[n-1])
    for(i = 0;i < n;++i)
      register_keymap(&stock_keymap_[i]);

  purge_duplicate_keymap ();

  for (i = 0; i < keymap_.size(); ++i)
    {
      keysym_t *key = keymap_[i];

      assert (bitcount (term_->ModMetaMask) == 1
              && "call me after ModMetaMask was set!");
      if (key->state & MetaMask)
        {
          //key->state &= ~MetaMask;
          key->state |= term_->ModMetaMask;
        }

      assert (bitcount (term_->ModNumLockMask) == 1
              && "call me after ModNumLockMask was set!");
      if (key->state & NumLockMask)
        {
          //key->state &= ~NumLockMask;
          key->state |= term_->ModNumLockMask;
        }
    }

  setup_hash ();
}

bool keyboard_manager::dispatch (KeySym keysym, unsigned int state)
{
  assert(hash_[0] == 0 && "register_done() need to be called");

  int index = find_keysym (keysym, state);

  if (index >= 0)
    {
      assert (term_ && keymap_[index] && keymap_[index]->handler);
      keymap_[index]->handler (term_, keymap_[index], keysym, state);
      return true;
    }
  else
  {
    // fprintf(stderr,"[%x:%x]",state,keysym);
    return false;
  }
}

// purge duplicate keymap_ entries
void
keyboard_manager::purge_duplicate_keymap ()
{
  for (unsigned int i = 0; i < keymap_.size (); ++i)
    {
      for (unsigned int j = 0; j < i; ++j)
        {
          if (keymap_[i] == keymap_[j])
            {
              while (keymap_[i] == keymap_.back ())
                keymap_.pop_back ();
              if (i < keymap_.size ())
                {
                  keymap_[i] = keymap_.back ();
                  keymap_.pop_back ();
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
  vector<keysym_t *> sorted_keymap;
  u16 hash_budget_size[KEYSYM_HASH_BUDGETS];     // size of each budget
  u16 hash_budget_counter[KEYSYM_HASH_BUDGETS];  // #elements in each budget

  memset (hash_budget_size, 0, sizeof (hash_budget_size));
  memset (hash_budget_counter, 0, sizeof (hash_budget_counter));

  // count keysyms for corresponding hash budgets
  for (i = 0; i < keymap_.size (); ++i)
    {
      assert (keymap_[i]);
      hashkey = (keymap_[i]->keysym & KEYSYM_HASH_MASK);
      ++hash_budget_size[hashkey];
    }

  // keysym A with range>1 is counted one more time for
  // every keysym B lies in its range
  for (i = 0; i < keymap_.size (); ++i)
    {
      if (keymap_[i]->range > 1)
        {
          for (int j = min (keymap_[i]->range, KEYSYM_HASH_BUDGETS) - 1;j > 0; --j)
            {
              hashkey = ((keymap_[i]->keysym + j) & KEYSYM_HASH_MASK);
              if (hash_budget_size[hashkey])
                ++hash_budget_size[hashkey];
            }
        }
    }

  // now we know the size of each budget
  // compute the index of each budget
  hash_[0] = 0;
  for (index = 0,i = 1; i < KEYSYM_HASH_BUDGETS; ++i)
    {
      index += hash_budget_size[i-1];
      hash_[i] = (hash_budget_size[i] ? index : hash_[i-1]);
    }
  // and allocate just enough space
  //sorted_keymap.reserve (hash_[i - 1] + hash_budget_size[i - 1]);
  sorted_keymap.insert (sorted_keymap.begin(), index + hash_budget_size[i - 1], 0);

  // fill in sorted_keymap
  // it is sorted in each budget
  for (i = 0; i < keymap_.size (); ++i)
    {
      for (int j = min (keymap_[i]->range, KEYSYM_HASH_BUDGETS) - 1;j >= 0; --j)
        {
          hashkey = ((keymap_[i]->keysym + j) & KEYSYM_HASH_MASK);
          if (hash_budget_size[hashkey])
            {
              index = hash_[hashkey] + hash_budget_counter[hashkey];
              while (index > hash_[hashkey] &&
                     compare_priority (keymap_[i],
                                       sorted_keymap[index - 1]) > 0)
                {
                  sorted_keymap[index] = sorted_keymap[index - 1];
                  --index;
                }
              sorted_keymap[index] = keymap_[i];
              ++hash_budget_counter[hashkey];
            }
        }
    }

  keymap_.swap (sorted_keymap);

#if defined (DEBUG_STRICT) || defined (DEBUG_KEYBOARD)
  // check for invariants
  for (i = 0; i < KEYSYM_HASH_BUDGETS; ++i)
    {
      index = hash_[i];
      for (int j = 0; j < hash_budget_size[i]; ++j)
        {
          if (keymap_[index + j]->range == 1)
            assert (i == (keymap_[index + j]->keysym & KEYSYM_HASH_MASK));
          if (j)
            assert (compare_priority (keymap_[index + j - 1],
                                      keymap_[index + j]) >= 0);
        }
    }

  // this should be able to detect most possible bugs
  for (i = 0; i < sorted_keymap.size (); ++i)
    {
      keysym_t *a = sorted_keymap[i];
      for (int j = 0; j < a->range; ++j)
        {
          int index = find_keysym (a->keysym + j, a->state & AllModMask);
          assert (index >= 0);
          keysym_t *b = keymap_[index];
          assert (i == (signed)index || // the normally expected result
              (a->keysym + j) >= b->keysym &&
              (a->keysym + j) <= (b->keysym + b->range) &&
              compare_priority (a, b) <= 0); // is effectively the same
        }
    }
#endif
}

int
keyboard_manager::find_keysym (KeySym keysym, unsigned int state)
{
  int hashkey = (keysym & KEYSYM_HASH_MASK);
  unsigned int index = hash_[hashkey];

  for (;index < keymap_.size(); ++index)
    {
      keysym_t *key = keymap_[index];
      assert (key);
      if (key->keysym <= keysym &&
          key->keysym + key->range > keysym &&
          // match only the specified bits in state and ignore others
          (key->state & (unsigned int)AllModMask) == (key->state & state))
        {
          return index;
        }
      else if (key->keysym > keysym &&
               key->range == 1)
        return -1;
    }

  return -1;
}

#endif /* KEYSYM_RESOURCE */
// vim:et:ts=2:sw=2
