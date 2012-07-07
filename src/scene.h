/*
 * See LICENSE for copyright information.
 */

#ifndef _scene_h_
#define _scene_h_

#include "aabbtree.h"
#include "tiled.h"
#include "sprite.h"
#include "utils.h"

typedef struct SCENE {
    char *name;
    char *map_filename;
    char *npc_layer_name;
    char *collision_layer_name;
    char *portal_layer_name;
    TILED_MAP *map;
    LIST *npcs;
    AABB_TREE *collision_tree;
    AABB_TREE *portal_tree;
} SCENE;

typedef struct SCENES {
    AATREE *tree;
    LIST *scenes;
} SCENES;

SCENES *scene_load_file (const char *filename);
SCENE *scene_get (SCENES *scenes, const char *scene_name);
SCENE *scene_load (SCENE *scene, SPRITES *sprites);
SCENE *scene_unload (SCENE *scene);
void scene_free (SCENES *scenes);

#endif
