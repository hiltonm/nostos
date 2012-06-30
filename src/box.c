/*
 * See LICENSE for copyright information.
 */

#include "box.h"

#include <math.h>
#include <assert.h>

#include <allegro5/allegro_primitives.h>

#include "utils.h"

BOX box_from_points (VECTOR2D v1, VECTOR2D v2)
{
    BOX box;
    box.center = vadd (&v2, &v1);
    box.extent = vsub (&v2, &v1);
    vatmulf (&box.center, 0.5f);
    vatmulf (&box.extent, 0.5f);
    vabs (&box.extent);
    return box;
}

bool box_overlap (BOX *b1, BOX *b2)
{
    assert (b1);
    assert (b2);
    VECTOR2D t = vsub (&b1->center, &b2->center);
    vabs (&t);

    VECTOR2D ext = vadd (&b1->extent, &b2->extent);
    return t.x <= ext.x && t.y <= ext.y;
}

bool box_inside_vector2d (BOX *b, VECTOR2D *v)
{
    assert (b);
    assert (v);
    VECTOR2D min = box_get_min (b);
    VECTOR2D max = box_get_max (b);

    if (v->x < min.x || v->x > max.x || v->y < min.y || v->y > max.y)
        return false;

    return true;
}

bool box_inside_box (BOX *b1, BOX *b2)
{
    assert (b1);
    assert (b2);
    return b1->center.x - b1->extent.x > b2->center.x - b2->extent.x &&
           b1->center.x + b1->extent.x < b2->center.x + b2->extent.x &&
           b1->center.y - b1->extent.y > b2->center.y - b2->extent.y &&
           b1->center.y + b1->extent.y < b2->center.y + b2->extent.y;
}

VECTOR2D box_get_min (BOX *b)
{
    assert (b);
    return vsub (&b->center, &b->extent);
}

VECTOR2D box_get_max (BOX *b)
{
    assert (b);
    return vadd (&b->center, &b->extent);
}

void box_debug (BOX *b)
{
    assert (b);
    debug ("Box center (%f, %f), extent (%f, %f)", b->center.x, b->center.y, b->extent.x, b->extent.y);
}

void box_draw (BOX *b, SCREEN *s, ALLEGRO_COLOR color)
{
    assert (b);
    assert (s);
    VECTOR2D bmin = box_get_min (b);
    VECTOR2D bmax = box_get_max (b);
    vatsub (&bmin, &s->position);
    vatsub (&bmax, &s->position);
    al_draw_rectangle (bmin.x, bmin.y, bmax.x, bmax.y, color, 2);
}

bool box_lateral (BOX *b1, BOX *b2)
{
    if (b2->center.x - b2->extent.x > b1->center.x + b1->extent.x ||
        b2->center.x + b2->extent.x < b1->center.x - b1->extent.x)
        return true;
    else
        return false;
}

