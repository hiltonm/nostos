#ifndef _aatree_h
#define _aatree_h

typedef struct AATREE AATREE;

struct AATREE
{
   int         level;
   AATREE      *left;
   AATREE      *right;
   const void  *key;
   void        *value;
};

typedef int (*cmp_t)(const void *a, const void *b);

AATREE *aa_insert(AATREE *T, const void *key, void *value, cmp_t compare);
void *aa_search(const AATREE *T, const void *key, cmp_t compare);
void aa_free(AATREE *T);

#endif

/* vim: set sts=3 sw=3 et: */
