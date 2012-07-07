/*
 * See LICENSE for copyright information.
 */

#include "game.h"

#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "vector2d.h"
#include "tiled.h"
#include "sprite.h"
#include "aabbtree.h"
#include "screen.h"
#include "utils.h"

#include <stdio.h>
#include <math.h>
#include <time.h>

#define FPS 80
#define NTIMES 10

GAME * game_init ()
{
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize allegro.\n");
        return NULL;
    }

    if (!al_init_image_addon ()) {
        fprintf(stderr, "Failed to initialize image addon.\n");
        return NULL;
    }

    if (!al_install_keyboard ()) {
        fprintf(stderr, "Failed to install keyboard.\n");
        return NULL;
    }

    al_set_org_name ("nostos");
    al_set_app_name ("demo1");

    al_init_font_addon ();
    al_init_primitives_addon ();

    GAME *game = al_malloc (sizeof (GAME));
    if (!game)
        return NULL;

    srand (time (NULL));

    game->running = true;

    game->fullscreen = 1;
    game->windowed = 1;
    game->rrate = 60;
    game->suggest_vsync = 1;
    game->force_vsync = 0;

    game->screen = screen_new ();

    ALLEGRO_PATH *respath = al_get_standard_path (ALLEGRO_RESOURCES_PATH);
    ALLEGRO_PATH *settpath = al_get_standard_path (ALLEGRO_USER_SETTINGS_PATH);
    ALLEGRO_PATH *gcpath = al_clone_path (settpath);

    al_set_path_filename (gcpath, "general.ini");
    const char * gcpath_str = al_path_cstr (gcpath, ALLEGRO_NATIVE_PATH_SEP);

    ALLEGRO_CONFIG *gconfig = al_load_config_file (gcpath_str);

    if (!gconfig) {
        gconfig = al_create_config ();
        al_make_directory (al_path_cstr (settpath, ALLEGRO_NATIVE_PATH_SEP));

        set_config_i (gconfig, "display", "width", game->screen.width);
        set_config_i (gconfig, "display", "height", game->screen.height);
        set_config_i (gconfig, "display", "fullscreen", game->fullscreen);
        set_config_i (gconfig, "display", "windowed", game->windowed);
        set_config_i (gconfig, "display", "refreshrate", game->rrate);
        set_config_i (gconfig, "display", "suggest_vsync", game->suggest_vsync);
        set_config_i (gconfig, "display", "force_vsync", game->force_vsync);
    } else {
        get_config_i (gconfig, "display", "width", &game->screen.width);
        get_config_i (gconfig, "display", "height", &game->screen.height);
        get_config_i (gconfig, "display", "fullscreen", &game->fullscreen);
        get_config_i (gconfig, "display", "windowed", &game->windowed);
        get_config_i (gconfig, "display", "refreshrate", &game->rrate);
        get_config_i (gconfig, "display", "suggest_vsync", &game->suggest_vsync);
        get_config_i (gconfig, "display", "force_vsync", &game->force_vsync);
    }

    al_save_config_file (gcpath_str, gconfig);

    al_destroy_path (respath);
    al_destroy_path (settpath);
    al_destroy_path (gcpath);
    al_destroy_config (gconfig);

    int flags = 0;

    if (game->fullscreen == game->windowed)
        flags |= ALLEGRO_FULLSCREEN_WINDOW;
    else if (game->fullscreen)
        flags |= ALLEGRO_FULLSCREEN;
    else
        flags |= ALLEGRO_WINDOWED;

    al_set_new_display_option (ALLEGRO_VSYNC, game->suggest_vsync, ALLEGRO_SUGGEST);
    al_set_new_display_option (ALLEGRO_DEPTH_SIZE, 8, ALLEGRO_SUGGEST);

    al_set_new_display_flags (flags);
    al_set_new_display_refresh_rate (game->rrate);
    game->display = al_create_display (game->screen.width, game->screen.height);
    if (!game->display) {
        fprintf (stderr, "Failed to create display.\n");
        al_free (game);
        return NULL;
    }

    al_set_new_bitmap_flags (ALLEGRO_VIDEO_BITMAP);

    game->timer = al_create_timer (1.0 / FPS);
    if (!game->timer) {
        fprintf (stderr, "Failed to create timer.\n");
        al_free (game);
        return NULL;
    }

    game->screen.width = al_get_display_width (game->display);
    game->screen.height = al_get_display_height (game->display);
    screen_update_size (&game->screen, game->screen.width, game->screen.height);

    game->rrate = al_get_display_refresh_rate (game->display);

    game->event_queue = al_create_event_queue ();
    if (!game->event_queue) {
        fprintf (stderr, "Failed to create event queue.\n");
        al_free (game);
        return NULL;
    }

    al_register_event_source (game->event_queue, al_get_display_event_source (game->display));
    al_register_event_source (game->event_queue, al_get_timer_event_source (game->timer));

    al_set_render_state (ALLEGRO_ALPHA_FUNCTION, ALLEGRO_RENDER_EQUAL);
    al_set_render_state (ALLEGRO_ALPHA_TEST_VALUE, 1);

    char *filename = get_resource_path_str ("data/chars.ini");
    SPRITES *sprites = sprite_load_sprites (filename);
    al_free (filename);

    filename = get_resource_path_str ("data/scenes.ini");
    SCENES *scenes = scene_load_file (filename);
    scene_load_scenes (scenes, sprites);
    al_free (filename);

    filename = get_resource_path_str ("data/entry.ini");
    ALLEGRO_CONFIG *config = al_load_config_file (filename);

    const char *str = al_get_config_value (config, "", "scene");
    game->current_scene = scene_get (scenes, str);

    str = al_get_config_value (config, "", "actor");
    game->current_actor = sprite_new_actor (sprites, str);
    str = al_get_config_value (config, "", "portal");
    SCENE_PORTAL *portal = _al_aa_search (scenes->portals, str, charcmp);

    al_free (filename);
    al_destroy_config (config);

    sprite_center (game->current_actor, &portal->position);
    screen_center (&game->screen, portal->position, game->current_scene->map);

    return game;
}

void game_loop (GAME *game)
{
    if (!game)
        return;

    ALLEGRO_KEYBOARD_STATE keyboard_state;
    ALLEGRO_EVENT event;
    ALLEGRO_FONT *font = al_load_font ("data/fixed_font.tga", 0, 0);
    SCENE *scene;
    SPRITE_ACTOR *actor;

    AABB_COLLISIONS collisions;
    aabb_init_collisions (&collisions);

    int i = 0;
    bool redraw = true;
    float times[NTIMES] = {0};

    double frame_time, current_time, new_time, mean_frame_time;
    double fixed_dt = 1.0 / FPS, dt;
    double curfps = 0.0;

    current_time = al_get_time ();
    al_start_timer (game->timer);

    while (game->running) {
        scene = game->current_scene;
        actor = game->current_actor;

        if (redraw) {
            al_clear_depth_buffer (0);
            tiled_draw_map_back (scene->map, game->screen.position.x, game->screen.position.y,
                                             game->screen.width, game->screen.height, 0, 0, 0);

            al_draw_textf (font, al_map_rgba_f (0.9, 0, 0, 1), 5, 5, 0, "FPS: %.2f", curfps);

            al_set_render_state (ALLEGRO_ALPHA_TEST, true);
            al_set_render_state (ALLEGRO_DEPTH_TEST, true);
            al_set_render_state (ALLEGRO_DEPTH_FUNCTION, ALLEGRO_RENDER_GREATER);

            al_hold_bitmap_drawing (true);
            sprite_draw (actor, &game->screen);
            LIST_ITEM *item = _al_list_front (scene->npcs);
            while (item) {
                SPRITE_ACTOR *npc_actor = (SPRITE_ACTOR *)_al_list_item_data (item);
                sprite_draw (npc_actor, &game->screen);
                item = _al_list_next (scene->npcs, item);
            }
            al_hold_bitmap_drawing (false);

            al_set_render_state (ALLEGRO_DEPTH_TEST, false);
            al_set_render_state (ALLEGRO_ALPHA_TEST, false);

            if (false) {
                item = _al_list_front (collisions.boxes);
                while (item) {
                    BOX *box = _al_list_item_data (item);
                    box_draw (box, &game->screen, al_map_rgb_f (1, 0, 0));
                    item = _al_list_next (collisions.boxes, item);
                }

                aabb_draw (scene->collision_tree, &game->screen, al_map_rgb_f (0, 0, 1));
            }

            tiled_draw_map_fore (scene->map, game->screen.position.x, game->screen.position.y,
                                             game->screen.width, game->screen.height, 0, 0, 0);

            if (game->force_vsync)
                al_wait_for_vsync ();

            al_flip_display ();
            redraw = false;
        }

        al_wait_for_event (game->event_queue, &event);

        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                new_time = al_get_time ();
                frame_time = new_time - current_time;
                current_time = new_time;

                times[i] = frame_time;
                i = (i + 1) % NTIMES;

                mean_frame_time = 0.0;
                for (int j = 0; j < NTIMES; j++)
                    mean_frame_time += times[j];

                mean_frame_time /= NTIMES;
                curfps = 1.0 / mean_frame_time;

                dt = mean_frame_time / fixed_dt;

                al_get_keyboard_state (&keyboard_state);

                if (al_key_down (&keyboard_state, ALLEGRO_KEY_ESCAPE)) {
                    game->running = false;
                    continue;
                }

                if (al_key_down (&keyboard_state, ALLEGRO_KEY_RIGHT)) {
                    actor->event->move_right (actor, dt);
                }
                if (al_key_down (&keyboard_state, ALLEGRO_KEY_LEFT)) {
                    actor->event->move_left (actor, dt);
                }
                if (al_key_down (&keyboard_state, ALLEGRO_KEY_UP)) {
                    actor->event->move_up (actor, dt);
                }
                if (al_key_down (&keyboard_state, ALLEGRO_KEY_DOWN)) {
                    actor->event->move_down (actor, dt);
                }

                BOX box = actor->box;
                box.center.x += actor->movement.x * dt;
                box.center.y += actor->movement.y * dt;

                aabb_collide_fill_cache (scene->collision_tree, &box, &collisions);
                if (scene->collision_tree->num_collisions > 0) {
                    LIST_ITEM *item = _al_list_front (collisions.boxes);
                    while (item) {
                        if (box_lateral ((BOX *)_al_list_item_data (item), &actor->box))
                            actor->movement.x = 0;
                        else
                            actor->movement.y = 0;
                        item = _al_list_next (collisions.boxes, item);
                    }
                }

                screen_update (&game->screen, actor->position, scene->map, dt);
                sprite_update (actor, dt, mean_frame_time);

                LIST_ITEM *item = _al_list_front (scene->npcs);
                while (item) {
                    SPRITE_ACTOR *npc_actor = (SPRITE_ACTOR *)_al_list_item_data (item);
                    sprite_update (npc_actor, dt, mean_frame_time);
                    item = _al_list_next (scene->npcs, item);
                }

                redraw = true;
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                game->running = false;
                break;
            default:
                fprintf (stderr, "Unsupported event received: %d\n", event.type);
                break;
        }
    }

    aabb_free_collisions (&collisions);
}

void game_destroy (GAME *game)
{
    if (game) {
        al_destroy_display (game->display);
        al_destroy_event_queue (game->event_queue);
        al_free (game);
    }
}

