/*
 * See LICENSE for copyright information.
 */

#include "nostos/box.h"
#include "nostos/utils.h"

#include <math.h>
#include <assert.h>

#include <allegro5/allegro_primitives.h>


BOX box_from_points (VECTOR2D v1, VECTOR2D v2)
{
    BOX box;
    box.center = vabs (vmulf (vadd (v2, v1), 0.5f));
    box.extent = vabs (vmulf (vsub (v2, v1), 0.5f));
    return box;
}

bool box_overlap (BOX b1, BOX b2)
{
    VECTOR2D t = vabs (vsub (b1.center, b2.center));
    VECTOR2D ext = vadd (b1.extent, b2.extent);
    return t.x <= ext.x && t.y <= ext.y;
}

bool box_inside_vector2d (BOX b, VECTOR2D v)
{
    VECTOR2D min = box_get_min (b);
    VECTOR2D max = box_get_max (b);

    if (v.x < min.x || v.x > max.x || v.y < min.y || v.y > max.y)
        return false;

    return true;
}

bool box_inside_box (BOX b1, BOX b2)
{
    return b1.center.x - b1.extent.x > b2.center.x - b2.extent.x &&
           b1.center.x + b1.extent.x < b2.center.x + b2.extent.x &&
           b1.center.y - b1.extent.y > b2.center.y - b2.extent.y &&
           b1.center.y + b1.extent.y < b2.center.y + b2.extent.y;
}

VECTOR2D box_get_min (BOX b)
{
    return vsub (b.center, b.extent);
}

VECTOR2D box_get_max (BOX b)
{
    return vadd (b.center, b.extent);
}

void box_debug (BOX b)
{
    debug ("Box center (%f, %f), extent (%f, %f)", b.center.x, b.center.y, b.extent.x, b.extent.y);
}

void box_draw (BOX b, VECTOR2D offset, ALLEGRO_COLOR color)
{
    VECTOR2D bmin = vsub (box_get_min (b), offset);
    VECTOR2D bmax = vsub (box_get_max (b), offset);
    al_draw_rectangle (bmin.x, bmin.y, bmax.x, bmax.y, color, 2);
}

bool box_lateral (BOX b1, BOX b2)
{
    if (b2.center.x - b2.extent.x > b1.center.x + b1.extent.x ||
        b2.center.x + b2.extent.x < b1.center.x - b1.extent.x)
        return true;
    else
        return false;
}

