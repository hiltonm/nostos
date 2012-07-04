
#include <stdio.h>
#include <math.h>
#include <time.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "vector2d.h"
#include "tiled.h"
#include "sprite.h"
#include "aabbtree.h"
#include "screen.h"
#include "utils.h"

#define FPS 80
#define NTIMES 10

int main (int argc, char *argv[])
{
    ALLEGRO_DISPLAY	*display = NULL;
    ALLEGRO_EVENT_QUEUE	*event_queue = NULL;
    ALLEGRO_TIMER *timer = NULL;
    ALLEGRO_KEYBOARD_STATE keyboard_state;
    TILED_MAP *map;

    srand (time (NULL));

    bool running = true;
    bool redraw = true;

    int fullscreen = 1;
    int windowed = 1;
    int rrate = 60;
    int suggest_vsync = 1;
    int force_vsync = 0;

    float times[NTIMES] = {0};

    SCREEN screen = screen_new ();

    if (!al_init()) {
        fprintf(stderr, "Failed to initialize allegro.\n");
        return 1;
    }

    if (!al_init_image_addon()) {
        fprintf(stderr, "Failed to initialize image addon.\n");
        return 1;
    }

    if (!al_install_keyboard()) {
        fprintf(stderr, "Failed to install keyboard.\n");
        return 1;
    }

    al_set_org_name ("nostos");
    al_set_app_name ("demo1");

    al_init_font_addon ();
    al_init_primitives_addon ();

    ALLEGRO_PATH *respath = al_get_standard_path (ALLEGRO_RESOURCES_PATH);
    ALLEGRO_PATH *settpath = al_get_standard_path (ALLEGRO_USER_SETTINGS_PATH);
    ALLEGRO_PATH *gcpath = al_clone_path (settpath);

    al_set_path_filename (gcpath, "general.ini");
    const char * gcpath_str = al_path_cstr (gcpath, ALLEGRO_NATIVE_PATH_SEP);

    ALLEGRO_CONFIG *gconfig = al_load_config_file (gcpath_str);

    if (!gconfig) {
        gconfig = al_create_config ();
        al_make_directory (al_path_cstr (settpath, ALLEGRO_NATIVE_PATH_SEP));

        set_config_i (gconfig, "display", "width", screen.width);
        set_config_i (gconfig, "display", "height", screen.height);
        set_config_i (gconfig, "display", "fullscreen", fullscreen);
        set_config_i (gconfig, "display", "windowed", windowed);
        set_config_i (gconfig, "display", "refreshrate", rrate);
        set_config_i (gconfig, "display", "suggest_vsync", suggest_vsync);
        set_config_i (gconfig, "display", "force_vsync", force_vsync);
    } else {
        get_config_i (gconfig, "display", "width", &screen.width);
        get_config_i (gconfig, "display", "height", &screen.height);
        get_config_i (gconfig, "display", "fullscreen", &fullscreen);
        get_config_i (gconfig, "display", "windowed", &windowed);
        get_config_i (gconfig, "display", "refreshrate", &rrate);
        get_config_i (gconfig, "display", "suggest_vsync", &suggest_vsync);
        get_config_i (gconfig, "display", "force_vsync", &force_vsync);
    }

    al_save_config_file (gcpath_str, gconfig);

    int flags = 0;

    if (fullscreen == windowed)
        flags |= ALLEGRO_FULLSCREEN_WINDOW;
    else if (fullscreen)
        flags |= ALLEGRO_FULLSCREEN;
    else
        flags |= ALLEGRO_WINDOWED;

    al_set_new_display_option (ALLEGRO_VSYNC, suggest_vsync, ALLEGRO_SUGGEST);
    al_set_new_display_option (ALLEGRO_DEPTH_SIZE, 8, ALLEGRO_SUGGEST);

    al_set_new_display_flags (flags);
    al_set_new_display_refresh_rate (rrate);
    display = al_create_display(screen.width, screen.height);
    if (!display) {
        fprintf(stderr, "Failed to create display.\n");
        return 1;
    }

    al_set_new_bitmap_flags (ALLEGRO_VIDEO_BITMAP);
    ALLEGRO_FONT *font = al_load_font ("data/fixed_font.tga", 0, 0);

    ALLEGRO_PATH *mappath = al_clone_path (respath);
    ALLEGRO_PATH *mapfilepath = al_create_path ("data/maps/teste.tmx");
    al_join_paths (mappath, mapfilepath);
    map = tiled_load_tmx_file (al_path_cstr (mappath, ALLEGRO_NATIVE_PATH_SEP));

    timer = al_create_timer (1.0 / FPS);
    if (!timer) {
        fprintf(stderr, "Failed to create timer.\n");
        return 1;
    }

    screen.width = al_get_display_width(display);
    screen.height = al_get_display_height(display);
    screen_update_size (&screen, screen.width, screen.height);

    rrate = al_get_display_refresh_rate (display);

    event_queue = al_create_event_queue();
    if (!event_queue) {
        fprintf(stderr, "Failed to create event queue.\n");
        return 1;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));

    al_start_timer(timer);

    double frame_time, current_time, new_time, mean_frame_time;
    double fixed_dt = 1.0 / FPS, dt;
    double curfps = 0.0;

    SPRITES *sprites = sprite_load_sprites ("data/chars.ini");
    SPRITE_ACTOR *actor = sprite_new_actor (sprites, "female");
    actor->position.x += 130;
    actor->position.y += 240;
    actor->box.center.x += 130;
    actor->box.center.y += 240;

    LIST *npcs = sprite_load_npcs (sprites, map, "npcs");
    AABB_TREE *collision_tree = aabb_load_tree (map, "collisions");
    //collision_tree->use_cache = true;
    AABB_COLLISIONS collisions;

    aabb_init_collisions (&collisions);

    int i = 0;

    current_time = al_get_time ();

    al_set_render_state (ALLEGRO_ALPHA_FUNCTION, ALLEGRO_RENDER_EQUAL);
    al_set_render_state (ALLEGRO_ALPHA_TEST_VALUE, 1);

    while (running) {
        ALLEGRO_EVENT event;
        if (redraw) {
            al_clear_depth_buffer (0);
            tiled_draw_map_back (map, screen.position.x, screen.position.y, screen.width, screen.height, 0, 0, 0);

            al_draw_textf (font, al_map_rgba_f (0.9, 0, 0, 1), 5, 5, 0, "FPS: %.2f", curfps);

            al_set_render_state (ALLEGRO_ALPHA_TEST, true);
            al_set_render_state (ALLEGRO_DEPTH_TEST, true);
            al_set_render_state (ALLEGRO_DEPTH_FUNCTION, ALLEGRO_RENDER_GREATER);
            al_hold_bitmap_drawing (true);
            sprite_draw (actor, &screen);
            LIST_ITEM *item = _al_list_front (npcs);
            while (item) {
                SPRITE_ACTOR *npc_actor = (SPRITE_ACTOR *)_al_list_item_data (item);
                sprite_draw (npc_actor, &screen);
                item = _al_list_next (npcs, item);
            }
            al_hold_bitmap_drawing (false);
            al_set_render_state (ALLEGRO_DEPTH_TEST, false);
            al_set_render_state (ALLEGRO_ALPHA_TEST, false);

            if (false) {
                item = _al_list_front (collisions.boxes);
                while (item) {
                    BOX *box = _al_list_item_data (item);
                    box_draw (box, &screen, al_map_rgb_f (1, 0, 0));
                    item = _al_list_next (collisions.boxes, item);
                }

                aabb_draw (collision_tree, &screen, al_map_rgb_f (0, 0, 1));
            }

            tiled_draw_map_fore (map, screen.position.x, screen.position.y, screen.width, screen.height, 0, 0, 0);

            if (force_vsync)
                al_wait_for_vsync ();

            al_flip_display ();
            redraw = false;
        }

        al_wait_for_event(event_queue, &event);

        switch (event.type) {
            case ALLEGRO_EVENT_TIMER:
                new_time = al_get_time ();
                frame_time = new_time - current_time;
                current_time = new_time;

                times[i++ % NTIMES] = frame_time;

                mean_frame_time = 0.0;
                for (int j = 0; j < NTIMES; j++)
                    mean_frame_time += times[j];

                mean_frame_time /= NTIMES;
                curfps = 1.0 / mean_frame_time;

                dt = mean_frame_time / fixed_dt;

                al_get_keyboard_state (&keyboard_state);

                if (al_key_down (&keyboard_state, ALLEGRO_KEY_ESCAPE)) {
                    running = false;
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
                aabb_collide_fill_cache (collision_tree, &box, &collisions);
                if (collision_tree->num_collisions > 0) {
                    LIST_ITEM *item = _al_list_front (collisions.boxes);
                    while (item) {
                        if (box_lateral ((BOX *)_al_list_item_data (item), &actor->box))
                            actor->movement.x = 0;
                        else
                            actor->movement.y = 0;
                        item = _al_list_next (collisions.boxes, item);
                    }
                }

                screen_update (&screen, actor->position, map, dt);
                sprite_update (actor, dt, mean_frame_time);

                LIST_ITEM *item = _al_list_front (npcs);
                while (item) {
                    SPRITE_ACTOR *npc_actor = (SPRITE_ACTOR *)_al_list_item_data (item);
                    sprite_update (npc_actor, dt, mean_frame_time);
                    item = _al_list_next (npcs, item);
                }

                redraw = true;
                break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                running = false;
                break;
            default:
                fprintf(stderr, "Unsupported event received: %d\n", event.type);
                break;
        }

    }

    al_destroy_path (respath);
    al_destroy_path (settpath);
    al_destroy_path (gcpath);
    al_destroy_path (mappath);
    al_destroy_path (mapfilepath);

    al_destroy_config (gconfig);
    _al_list_destroy (npcs);
    aabb_free_collisions (&collisions);
    aabb_free (collision_tree);

    tiled_free_map (map);
    al_destroy_display (display);
    al_destroy_event_queue (event_queue);
    return 0;
}
