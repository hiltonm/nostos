/*
 * See LICENSE for copyright information.
 */

#ifndef _screen_h_
#define _screen_h_

#include "vector2d.h"
#include "tiled.h"

typedef struct SCREEN {
    VECTOR2D position;
    int width, height;
    VECTOR2D focus_ul;
    VECTOR2D focus_lr;
    VECTOR2D movement;
    float accel, deaccel;
    float movement_max;
    float focus_width;
    float focus_height;
} SCREEN;

SCREEN screen_new ();
void screen_update_size (SCREEN *screen, int width, int height);
void screen_update (SCREEN *screen, VECTOR2D focus, TILED_MAP *map, float dt);

#endif
