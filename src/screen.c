/*
 * See LICENSE for copyright information.
 */

#include "nostos/screen.h"
#include "nostos/utils.h"

#include <assert.h>
#include <math.h>

static ALLEGRO_COLOR solid_white = {1, 1, 1, 1};

SCREEN screen_new ()
{
    return (SCREEN){
        {0, 0},
        0, 0,
        {640, 480},
        {0, 0},
        {0, 0},
        0.3, 0.1,
        3,
        30.0 / 100.0,
        30.0 / 100.0,
        solid_white
    };
}

void screen_update_size (SCREEN *screen, int width, int height)
{
    assert (screen);

    screen->width = width;
    screen->height = height;

    screen->focus_ul.x = screen->focus_width * width;
    screen->focus_ul.y = screen->focus_height * height;
    screen->focus_lr.x = width - screen->focus_ul.x;
    screen->focus_lr.y = height - screen->focus_ul.y;
}

void screen_update (SCREEN *screen, VECTOR2D focus, TILED_MAP *map, float dt)
{
    assert (screen);
    float deaccel = screen->deaccel;
    if (screen->movement.y > 0) screen->movement.y -= deaccel;
    if (screen->movement.y < 0) screen->movement.y += deaccel;
    if (screen->movement.y <= deaccel && screen->movement.y >= -deaccel) screen->movement.y = 0;

    if (screen->movement.x > 0) screen->movement.x -= deaccel;
    if (screen->movement.x < 0) screen->movement.x += deaccel;
    if (screen->movement.x <= deaccel && screen->movement.x >= -deaccel) screen->movement.x = 0;

    if (focus.x < screen->position.x + screen->focus_ul.x) screen->movement.x -= screen->accel;
    if (focus.y < screen->position.y + screen->focus_ul.y) screen->movement.y -= screen->accel;
    if (focus.x > screen->position.x + screen->focus_lr.x) screen->movement.x += screen->accel;
    if (focus.y > screen->position.y + screen->focus_lr.y) screen->movement.y += screen->accel;

    screen->movement.x = CLAMP (-screen->movement_max, screen->movement.x, screen->movement_max);
    screen->movement.y = CLAMP (-screen->movement_max, screen->movement.y, screen->movement_max);

    screen->position.x += screen->movement.x * dt;
    screen->position.y += screen->movement.y * dt;

    screen->position.x = CLAMP (0, round (screen->position.x), map->width * map->tile_width - screen->width);
    screen->position.y = CLAMP (0, round (screen->position.y), map->height * map->tile_height - screen->height);
}

void screen_center (SCREEN *screen, VECTOR2D focus, TILED_MAP *map)
{
    screen->position.x = focus.x - screen->width * 0.5f;
    screen->position.y = focus.y - screen->height * 0.5f;
    screen->position.x = CLAMP (0, round (screen->position.x), map->width * map->tile_width - screen->width);
    screen->position.y = CLAMP (0, round (screen->position.y), map->height * map->tile_height - screen->height);
    screen->movement = (VECTOR2D){0.0f, 0.0f};
}

