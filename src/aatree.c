/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      AA tree, a type of self-balancing search tree.
 *
 *      By Peter Wang.
 */


#include <allegro5/allegro.h>
#include "nostos/aatree.h"


static AATREE nil = { 0, &nil, &nil, NULL, NULL };

static AATREE *skew(AATREE *T)
{
   if (T == &nil)
      return T;
   if (T->left->level == T->level) {
      AATREE *L = T->left;
      T->left = L->right;
      L->right = T;
      return L;
   }
   return T;
}

static AATREE *split(AATREE *T)
{
   if (T == &nil)
      return T;
   if (T->level == T->right->right->level) {
      AATREE *R = T->right;
      T->right = R->left;
      R->left = T;
      R->level = R->level + 1;
      return R;
   }
   return T;
}

static AATREE *singleton(const void *key, void *value)
{
   AATREE *T = al_malloc(sizeof(AATREE));
   T->level = 1;
   T->left = &nil;
   T->right = &nil;
   T->key = key;
   T->value = value;
   return T;
}

static AATREE *doinsert(AATREE *T, const void *key, void *value,
   cmp_t compare)
{
   int cmp;
   assert (key);
   if (T == &nil) {
      return singleton(key, value);
   }
   cmp = compare(key, T->key);
   if (cmp < 0) {
      T->left = doinsert(T->left, key, value, compare);
   }
   else if (cmp > 0) {
      T->right = doinsert(T->right, key, value, compare);
   }
   else {
      /* Already exists. We don't yet return any indication of this. */
      return T;
   }
   T = skew(T);
   T = split(T);
   return T;
}

AATREE *aa_insert(AATREE *T, const void *key, void *value,
   cmp_t compare)
{
   if (T == NULL)
      T = &nil;
   return doinsert(T, key, value, compare);
}

void *aa_search(const AATREE *T, const void *key, cmp_t compare)
{
   if (T == NULL)
      return NULL;
   while (T != &nil) {
      int cmp = compare(key, T->key);
      if (cmp == 0)
         return T->value;
      T = (cmp < 0) ? T->left : T->right;
   }
   return NULL;
}

void aa_free(AATREE *T)
{
   if (T && T != &nil) {
      aa_free(T->left);
      aa_free(T->right);
      al_free(T);
   }
}

/* vim: set sts=3 sw=3 et: */
