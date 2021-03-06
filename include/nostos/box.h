/*
 * See LICENSE for copyright information.
 */

#ifndef _box_h_
#define _box_h_

#include "vector2d.h"

#include <allegro5/allegro.h>
#include <stdbool.h>

typedef struct BOX {
    VECTOR2D center;
    VECTOR2D extent;
    void *data;
} BOX;

BOX box_from_points (VECTOR2D v1, VECTOR2D v2);
BOX box_scale (BOX b, float f);
bool box_overlap (BOX b1, BOX b2);
bool box_inside_vector2d (BOX b, VECTOR2D v);
bool box_inside_box (BOX b1, BOX b2);
VECTOR2D box_get_min (BOX b);
VECTOR2D box_get_max (BOX b);
void box_debug (BOX b);
void box_draw (BOX b, VECTOR2D offset, ALLEGRO_COLOR color);
bool box_lateral (BOX b1, BOX b2);

#endif
