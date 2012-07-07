/*
 * See LICENSE for copyright information.
 */

#ifndef _game_h_
#define _game_h_

#include "sprite.h"
#include "scene.h"
#include "screen.h"

#include <allegro5/allegro.h>

typedef struct GAME {
    SCENES *scenes;
    SCENE *current_scene;
    SPRITES *sprites;
    SPRITE_ACTOR *current_actor;
    SCREEN screen;

    ALLEGRO_DISPLAY	*display;
    ALLEGRO_EVENT_QUEUE	*event_queue;
    ALLEGRO_TIMER *timer;

    bool running;

    int fullscreen;
    int windowed;
    int rrate;
    int suggest_vsync;
    int force_vsync;
} GAME;

GAME * game_init ();
void game_loop (GAME *game);
void game_destroy (GAME *game);

#endif
