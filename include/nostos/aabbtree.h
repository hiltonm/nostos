/*
 * See LICENSE for copyright information.
 */

#ifndef _aabbtree_h
#define _aabbtree_h

#include "box.h"
#include "tiled.h"
#include "utils.h"

typedef struct AABB_TREE AABB_TREE;
typedef struct AABB_LEAF AABB_LEAF;
typedef struct AABB_NODE AABB_NODE;
typedef struct AABB_COLLISIONS AABB_COLLISIONS;

struct AABB_NODE
{
    BOX aabb;
    AABB_NODE *left;
    AABB_NODE *right;
};

struct AABB_LEAF
{
    AABB_NODE node;
    BOX *boxes;
    int num_boxes;
};

struct AABB_COLLISIONS
{
    LIST *boxes;
    BOX query_box;
};

struct AABB_TREE
{
    AABB_NODE *root;
    AABB_LEAF *leafs;
    int num_nodes;
    int num_leafs;
    int max_depth;
    AABB_COLLISIONS *collisions;
    int num_collisions;
    bool use_cache;
};

AABB_TREE *aabb_build_tree (BOX *boxes, int num_boxes, int max_depth);
AABB_TREE *aabb_load_tree (TILED_MAP *map, const char *layer_name);
bool aabb_collide (AABB_TREE *tree, BOX *box);
bool aabb_collide_with_cache (AABB_TREE *tree, BOX *box, AABB_COLLISIONS *collision);
void aabb_collide_fill_cache (AABB_TREE *tree, BOX *box, AABB_COLLISIONS *collision);
void aabb_init_collisions (AABB_COLLISIONS *col);
void aabb_free (AABB_TREE *tree);
void aabb_free_collisions (AABB_COLLISIONS *col);
void aabb_draw (AABB_TREE *tree, SCREEN *s, ALLEGRO_COLOR color);

#endif

