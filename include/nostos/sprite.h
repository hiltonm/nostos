/*
 * See LICENSE for copyright information.
 */

#ifndef _sprite_h_
#define _sprite_h_

#include "vector2d.h"
#include "screen.h"
#include "tiled.h"
#include "box.h"
#include "utils.h"

enum {
    ANI_STAND_FRONT = 0,
    ANI_STAND_BACK,
    ANI_STAND_LEFT,
    ANI_STAND_RIGHT,
    ANI_WALK_FRONT,
    ANI_WALK_BACK,
    ANI_WALK_LEFT,
    ANI_WALK_RIGHT,
    ANI_MAX
};

enum {
    ACTOR_TYPE_MAIN,
    ACTOR_TYPE_NPC,
};

enum {
    NPC_ACTION_STAND,
    NPC_ACTION_MOVE,
    NPC_ACTION_MOVE_ONCE,
    NPC_ACTION_MOVE_RANDOM,
};

typedef struct SPRITE_TILESET {
    char *name;
    char *image_source;
    ALLEGRO_BITMAP *bitmap;
    int tile_width, tile_height;
    VECTOR2D *tiles;
    int num_tiles;
} SPRITE_TILESET;

typedef struct SPRITE_ANIMATION {
    int *frames;
    int num_frames;
} SPRITE_ANIMATION;

typedef struct SPRITE {
    char *name;
    SPRITE_TILESET *tileset;
    SPRITE_ANIMATION animations[ANI_MAX];
    float duration;
    BOX box;
} SPRITE;

typedef struct SPRITE_ACTOR {
    EVENTS *event;
    SPRITE *sprite;
    int type;
    int current_animation;
    int current_frame;
    float current_duration;
    VECTOR2D position;
    VECTOR2D movement;
    BOX box;
    float movement_accel;
    float movement_deaccel;
    float movement_max;
} SPRITE_ACTOR;

typedef struct SPRITE_NPC {
    SPRITE_ACTOR actor;
    int action;
    float *points;
    int num_points;
    int current_point;
    int next_point;
    int direction;
    bool paused;
    float pause_duration;
    float current_pause_duration;
} SPRITE_NPC;

typedef struct SPRITES {
    AATREE *sprites;
    AATREE *tilesets;
    LIST *sprites_list;
    LIST *tilesets_list;
    LIST *strings;
} SPRITES;

void sprite_move_down (void *sprite, float dt);
void sprite_move_up (void *sprite, float dt);
void sprite_move_left (void *sprite, float dt);
void sprite_move_right (void *sprite, float dt);
void sprite_update (SPRITE_ACTOR *actor, float dt, float t);
void sprite_draw (SPRITE_ACTOR *actor, SCREEN *screen);
SPRITE_ACTOR *sprite_new_actor (SPRITES *sprites, const char *filename);
SPRITES *sprite_load_sprites (const char *filename);
LIST *sprite_load_npcs (SPRITES *sprites, TILED_MAP *map, const char *layer_name);
void sprite_center (SPRITE_ACTOR *actor, VECTOR2D *v);
void sprite_free (SPRITES *sprites);
void sprite_free_actor (void *value, void *user_data);
void sprite_free_npc (void *value, void *user_data);

#endif
