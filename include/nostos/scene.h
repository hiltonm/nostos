/*
 * See LICENSE for copyright information.
 */

#ifndef _scene_h_
#define _scene_h_

#include "aabbtree.h"
#include "tiled.h"
#include "sprite.h"
#include "utils.h"

typedef struct SCENE SCENE;
typedef struct SCENES SCENES;
typedef struct SCENE_PORTAL SCENE_PORTAL;

struct SCENE {
    char *name;
    char *map_filename;
    char *npc_layer_name;
    char *collision_layer_name;
    char *portal_layer_name;
    TILED_MAP *map;
    LIST *npcs;
    LIST *portals;
    AABB_TREE *collision_tree;
    AABB_TREE *portal_tree;
    AABB_TREE *npc_tree;
};

struct SCENES {
    AATREE *tree;
    AATREE *portals;
    LIST *scenes;
};

struct SCENE_PORTAL {
    char *name;
    char *destiny_portal;
    SCENE *scene;
    VECTOR2D position;
};

SCENES *scene_load_file (const char *filename);
SCENE *scene_get (SCENES *scenes, const char *scene_name);
SCENE *scene_load (SCENE *scene, SCENES *scenes, SPRITES *sprites);
void scene_load_scenes (SCENES *scenes, SPRITES *sprites);
SCENE *scene_unload (SCENE *scene);
void scene_load_portals (SCENE *scene, SCENES *scenes, const char *layer_name);
SCENE_PORTAL *scene_get_portal (SCENES *scenes, const char *portal_name);
void scene_free (SCENES *scenes);

#endif
