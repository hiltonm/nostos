/*
 * See LICENSE for copyright information.
 */

#include "sprite.h"
#include "utils.h"

#include <allegro5/allegro_primitives.h>

#define NUM_ACTIONS 4

const char * config_animation_names[ANI_MAX] = {
    "ani_standfront",
    "ani_standback",
    "ani_standleft",
    "ani_standright",
    "ani_walkfront",
    "ani_walkback",
    "ani_walkleft",
    "ani_walkright",
};

const char * str_npc_actions[] = {
    "stand",
    "move",
    "moveonce",
    "moverandom",
};

EVENTS default_sprite_events = {
    sprite_move_down,
    sprite_move_up,
    sprite_move_left,
    sprite_move_right
};

void sprite_free_sprites (SPRITES *sprites)
{
    _al_list_destroy(sprites->strings);
    _al_aa_free (sprites->chars);
    _al_aa_free (sprites->tilesets);
    _al_list_destroy (sprites->strings);
    al_free (sprites);
}

static void dtor_tileset (void *value, void *user_data)
{
    SPRITE_TILESET *tileset = value;
    al_free (tileset->name);
    al_free (tileset->image_source);
    al_destroy_bitmap (tileset->bitmap);

    for (int i=0; i<tileset->num_tiles; i++) {
        al_destroy_bitmap (tileset->tiles[i]);
    }

    al_free (tileset->tiles);
    al_free (tileset);
}

static void dtor_char (void *value, void *user_data)
{
    SPRITE_CHAR *schar = value;
    al_free (schar->name);

    for (int i = 0; i < ANI_MAX; i++) {
        al_free (schar->animations[i].frames);
    }

    al_free (schar);
}

static void dtor_actor (void *value, void *user_data)
{
    SPRITE_ACTOR *actor = value;
    al_free (actor);
}

static inline int * get_int_array (const char *str, int *num)
{
    assert (str);
    assert (num);

    int i = 0;
    int *array = NULL;
    LIST *nums_str = split (str, ",");

    if (!_al_list_is_empty (nums_str)) {
        *num = _al_list_size (nums_str);
        array = al_malloc (*num * sizeof (int));

        LIST_ITEM *item = _al_list_front (nums_str);
        while (item) {
            array[i++] = atoi (_al_list_item_data (item));
            item = _al_list_next (nums_str, item);
        }
    }

    _al_list_destroy (nums_str);
    return array;
}


SPRITES* sprite_load_sprites (const char *filename)
{
    ALLEGRO_CONFIG *sprite_config = al_load_config_file (filename);

    if (!sprite_config)
        return NULL;

    ALLEGRO_CONFIG_SECTION *it = NULL;
    const char *section = al_get_first_config_section (sprite_config, &it);
    ALLEGRO_PATH *respath = al_get_standard_path (ALLEGRO_RESOURCES_PATH);

    SPRITES *sprites = al_calloc (1, sizeof (SPRITES));

    do {
        LIST *tokens = split (section, " ");
        LIST_ITEM *item = _al_list_front (tokens);
        const char *type = _al_list_item_data (item);

        if (!strcmp (type, "tileset")) {
            item = _al_list_next (tokens, item);
            SPRITE_TILESET *tileset = al_malloc (sizeof (SPRITE_TILESET));

            char *name = _al_list_item_data (item);
            tileset->name = strdup (name);
            tileset->image_source = strdup (al_get_config_value (sprite_config, section, "image"));
            get_config_i (sprite_config, section, "width", &tileset->tile_width);
            get_config_i (sprite_config, section, "height", &tileset->tile_height);

            ALLEGRO_PATH *image_path = al_create_path (tileset->image_source);
            al_rebase_path (respath, image_path);
            tileset->bitmap = al_load_bitmap (al_path_cstr (image_path, ALLEGRO_NATIVE_PATH_SEP));
            al_destroy_path (image_path);

            int tiles_per_row = al_get_bitmap_width (tileset->bitmap) / tileset->tile_width;
            tileset->num_tiles = (al_get_bitmap_width (tileset->bitmap) * al_get_bitmap_height (tileset->bitmap)) /
                                 (tileset->tile_width * tileset->tile_height);
            tileset->tiles = al_malloc (tileset->num_tiles * sizeof (ALLEGRO_BITMAP *));
            for (int i = 0; i < tileset->num_tiles; i++) {
                ALLEGRO_BITMAP **tile = &tileset->tiles[i];
                int x = (i % tiles_per_row) * tileset->tile_width;
                int y = (i / tiles_per_row) * tileset->tile_height;
                *tile = al_create_sub_bitmap(tileset->bitmap, x, y,
                                             tileset->tile_width,
                                             tileset->tile_height);
            }

            sprites->tilesets = _al_aa_insert (sprites->tilesets, tileset->name, tileset, charcmp);
        } else if (!strcmp (type, "char")) {
            item = _al_list_next (tokens, item);
            SPRITE_CHAR *schar = al_malloc (sizeof (SPRITE_CHAR));

            const char *str = _al_list_item_data (item);
            schar->name = strdup (str);
            str = al_get_config_value (sprite_config, section, "tileset");
            schar->tileset = _al_aa_search (sprites->tilesets, str, charcmp);

            int duration = 100;
            get_config_i (sprite_config, section, "duration", &duration);
            schar->duration = ALLEGRO_MSECS_TO_SECS (duration);

            for (int i = 0; i < ANI_MAX; i++) {
                str = al_get_config_value (sprite_config, section, config_animation_names[i]);
                if (str) {
                    SPRITE_ANIMATION *anim = &schar->animations[i];
                    anim->frames = get_int_array (str, &anim->num_frames);
                }
            }

            str = al_get_config_value (sprite_config, section, "bb");
            if (str) {
                int num = 0;
                int *values = get_int_array (str, &num);
                if (num == 4) {
                    schar->box.center.x = values[0];
                    schar->box.center.y = values[1];
                    schar->box.extent.x = values[2];
                    schar->box.extent.y = values[3];
                }
                al_free (values);
            }

            sprites->chars = _al_aa_insert (sprites->chars, schar->name, schar, charcmp);
        }

        section = al_get_next_config_section (&it);
    } while (section);

    al_destroy_path (respath);
    al_destroy_config (sprite_config);
    return sprites;
}

SPRITE_ACTOR sprite_init_copy ()
{
    return (SPRITE_ACTOR) {
        &default_sprite_events,
        NULL,
        ACTOR_TYPE_MAIN,
        ANI_STAND_FRONT,
        0,
        0.0,
        {20.0, 20.0},
        {0.0, 0.0},
        {{0, 0}, {0, 0}},
        1.0,
        0.3,
        3.0,
    };
}

SPRITE_ACTOR *sprite_init ()
{
    SPRITE_ACTOR *actor = al_malloc (sizeof (SPRITE_ACTOR));
    *actor = sprite_init_copy ();
    return actor;
}

SPRITE_ACTOR *sprite_init_actor (SPRITES *sprites, SPRITE_ACTOR *actor, const char *schar)
{
    assert (actor);
    actor->schar = _al_aa_search (sprites->chars, schar, charcmp);
    actor->box.extent.x = actor->schar->box.extent.x / 2.0;
    actor->box.extent.y = actor->schar->box.extent.y / 2.0;
    actor->box.center.x = actor->position.x + actor->schar->box.center.x;
    actor->box.center.y = actor->position.y + actor->schar->box.center.y;
    return actor;
}

SPRITE_ACTOR *sprite_new_actor (SPRITES *sprites, const char *schar)
{
    SPRITE_ACTOR *actor = sprite_init ();
    actor = sprite_init_actor (sprites, actor, schar);
    return actor;
}

void sprite_move_down (void *sprite, float dt)
{
    SPRITE_ACTOR *actor = sprite;
    actor->movement.y = actor->movement_max;
}

void sprite_move_up (void *sprite, float dt)
{
    SPRITE_ACTOR *actor = sprite;
    actor->movement.y = -actor->movement_max;
}

void sprite_move_left (void *sprite, float dt)
{
    SPRITE_ACTOR *actor = sprite;
    actor->movement.x = -actor->movement_max;
}

void sprite_move_right (void *sprite, float dt)
{
    SPRITE_ACTOR *actor = sprite;
    actor->movement.x = actor->movement_max;
}

void sprite_move_to (void *sprite, float x, float y, float dt)
{
    SPRITE_ACTOR *actor = sprite;
    VECTOR2D delta;
    delta.x = x - actor->position.x;
    delta.y = y - actor->position.y;
    vnormalize (&delta);
    actor->movement.x = delta.x * actor->movement_max;
    actor->movement.y = delta.y * actor->movement_max;
}

bool sprite_is_paused (SPRITE_NPC *npc, float t)
{
    if (npc->paused) {
        npc->current_pause_duration += t;
        if (npc->current_pause_duration >= npc->pause_duration) {
            npc->paused = false;
            npc->current_pause_duration = 0;
        }
    }
    return npc->paused;
}

static inline VECTOR2D sprite_next_point (SPRITE_NPC *npc)
{
    return (VECTOR2D) {npc->points[npc->next_point], npc->points[npc->next_point + 1]};
}

static inline bool sprite_point_is_equal (SPRITE_ACTOR *actor, VECTOR2D *v1, VECTOR2D *v2, float dt)
{
    VECTOR2D diff = vsub (v1, v2);
    float limit = dt * actor->movement_max;
    vabs (&diff);

    return (diff.x < limit && diff.y < limit);
}

void sprite_update (SPRITE_ACTOR *actor, float dt, float t)
{
    assert (actor);

    bool down = actor->movement.y > 0;
    bool right = actor->movement.x > 0;
    bool more_vertical = abs (actor->movement.x) < abs (actor->movement.y);

    int previous_animation = actor->current_animation;

    if (!actor->movement.x && !actor->movement.y) {
        switch (actor->current_animation) {
            case ANI_WALK_FRONT: actor->current_animation = ANI_STAND_FRONT; break;
            case ANI_WALK_BACK: actor->current_animation = ANI_STAND_BACK; break;
            case ANI_WALK_LEFT: actor->current_animation = ANI_STAND_LEFT; break;
            case ANI_WALK_RIGHT: actor->current_animation = ANI_STAND_RIGHT; break;
        }
    }
    else if (right && down)
        actor->current_animation = more_vertical ? ANI_WALK_FRONT : ANI_WALK_RIGHT;
    else if (right)
        actor->current_animation = more_vertical ? ANI_WALK_BACK : ANI_WALK_RIGHT;
    else if (down)
        actor->current_animation = more_vertical ? ANI_WALK_FRONT : ANI_WALK_LEFT;
    else
        actor->current_animation = more_vertical ? ANI_WALK_BACK : ANI_WALK_LEFT;

    actor->position.x += actor->movement.x * dt;
    actor->position.y += actor->movement.y * dt;
    actor->box.center.x += actor->movement.x * dt;
    actor->box.center.y += actor->movement.y * dt;

    if (actor->type == ACTOR_TYPE_MAIN) {
        float deaccel = actor->movement_deaccel * dt;
        if (actor->movement.y) {
            actor->movement.y += down ? -deaccel : deaccel;

            if (abs (actor->movement.y) <= actor->movement_deaccel)
                actor->movement.y = 0;
        }

        if (actor->movement.x) {
            actor->movement.x += right ? -deaccel : deaccel;

            if (abs (actor->movement.x) <= actor->movement_deaccel)
                actor->movement.x = 0;
        }

    }

    if (previous_animation == actor->current_animation) {
        actor->current_duration += t;
        if (actor->current_duration > actor->schar->duration) {
            actor->current_duration = 0.0;
            actor->current_frame = (actor->current_frame + 1) %
                                    actor->schar->animations[actor->current_animation].num_frames;
        }
    } else {
        actor->current_duration = actor->current_frame = 0;
    }

    if (actor->type == ACTOR_TYPE_NPC) {
        SPRITE_NPC *npc = (SPRITE_NPC *)actor;
        VECTOR2D next_point = sprite_next_point (npc);
        switch (npc->action) {
            case NPC_ACTION_MOVE:
                if (sprite_is_paused (npc, t))
                    break;

                if (sprite_point_is_equal (actor, &actor->position, &next_point, dt)) {
                    npc->current_point = npc->next_point;
                    actor->movement = (VECTOR2D){0, 0};

                    npc->next_point += npc->direction;
                    if ((npc->next_point + npc->direction) == npc->num_points || npc->next_point == 0)
                        npc->direction = -npc->direction;

                    if (npc->current_point == 0 || npc->current_point + 2 == npc->num_points)
                        npc->paused = true;
                } else {
                    sprite_move_to (actor, next_point.x, next_point.y, dt);
                }

                break;
            case NPC_ACTION_MOVE_RANDOM:
                if (sprite_is_paused (npc, t))
                    break;

                if (sprite_point_is_equal (actor, &actor->position, &next_point, dt)) {
                    npc->current_point = npc->next_point;
                    npc->next_point = 2 * (rand () % ((npc->num_points / 2) - 1));
                    actor->movement = (VECTOR2D){0, 0};
                    npc->paused = true;
                } else {
                    sprite_move_to (actor, next_point.x, next_point.y, dt);
                }

                break;
            default:
                break;
        }
    }
}

void sprite_draw (SPRITE_ACTOR *actor, SCREEN *screen)
{
    float sx = screen->position.x, sy = screen->position.y;
    SPRITE_ANIMATION *anim = &actor->schar->animations[actor->current_animation];
    ALLEGRO_BITMAP *bitmap = actor->schar->tileset->tiles[anim->frames[actor->current_frame]];
    al_draw_bitmap (bitmap, actor->position.x - sx, actor->position.y - sy, 0);
    box_draw (&actor->box, screen, al_map_rgb_f (1, 1, 1));
}

LIST *sprite_load_npcs (SPRITES *sprites, TILED_MAP *map, const char *layer_name)
{
    TILED_LAYER_OBJECT *layer = (TILED_LAYER_OBJECT *)tiled_layer_by_name (map, layer_name);
    LIST *npcs = _al_list_create ();

    if (layer && layer->objects) {
        LIST_ITEM *item = _al_list_front (layer->objects);

        while (item) {
            TILED_OBJECT *object = _al_list_item_data (item);
            SPRITE_NPC *npc = al_malloc (sizeof (SPRITE_NPC));
            npc->actor = sprite_init_copy ();
            npc->actor.type = ACTOR_TYPE_NPC;

            switch (object->type) {
                TILED_OBJECT_GEOM *object_geom;
                case OBJECT_TYPE_GEOM:
                    object_geom = (TILED_OBJECT_GEOM *)object;

                    const char *actionstr = _al_aa_search (object_geom->object.properties, "action", charcmp);
                    const char *charstr = _al_aa_search (object_geom->object.properties, "char", charcmp);

                    for (int i = 0; i < NUM_ACTIONS; i++) {
                        if (!strcmp (actionstr, str_npc_actions[i])) {
                            npc->action = i;
                            npc->points = object_geom->points;
                            npc->num_points = object_geom->num_points * 2;
                            break;
                        }
                    }

                    sprite_init_actor (sprites, &npc->actor, charstr);
                    npc->actor.position.x = npc->points[0];
                    npc->actor.position.y = npc->points[1];
                    npc->actor.box.center.x = npc->points[0] + npc->actor.schar->box.center.x;
                    npc->actor.box.center.y = npc->points[1] + npc->actor.schar->box.center.y;
                    npc->current_point = 0;
                    npc->paused = false;
                    npc->pause_duration = 3;
                    npc->current_pause_duration = 0;
                    npc->direction = 2;
                    if (npc->num_points > 2) {
                        npc->next_point = npc->direction;
                        if (npc->next_point == npc->num_points - 2)
                            npc->direction = -npc->direction;
                    } else {
                        npc->next_point = 0;
                        npc->action = NPC_ACTION_STAND;
                    }
                    break;
                default:
                    debug ("FIX: Found unsupported object in sprites layer. Only geometries (lines and polygons) are supported.");
                    break;
            }

            _al_list_push_back_ex (npcs, npc, dtor_actor);
            item = _al_list_next (layer->objects, item);
        }
    }

    return npcs;
}

