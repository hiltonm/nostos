/*
 * See LICENSE for copyright information.
 */

#include "nostos/vector2d.h"

#include <math.h>
#include <assert.h>

#include "nostos/utils.h"

VECTOR2D vadd (VECTOR2D v1, VECTOR2D v2)
{
    return (VECTOR2D) { .x = v1.x + v2.x, .y = v1.y + v2.y };
}

VECTOR2D vsub (VECTOR2D v1, VECTOR2D v2)
{
    return (VECTOR2D) { .x = v1.x - v2.x, .y = v1.y - v2.y };
}

VECTOR2D vmul (VECTOR2D v1, VECTOR2D v2)
{
    return (VECTOR2D) { .x = v1.x * v2.x, .y = v1.y * v2.y };
}

VECTOR2D vmulf (VECTOR2D v1, float f)
{
    return (VECTOR2D) { .x = v1.x * f, .y = v1.y * f };
}

VECTOR2D vdiv (VECTOR2D v1, VECTOR2D v2)
{
    return (VECTOR2D) { .x = v1.x / v2.x, .y = v1.y / v2.y };
}

VECTOR2D vdivf (VECTOR2D v1, float f)
{
    float m = 1.0f / f;
    return (VECTOR2D) { .x = v1.x * m, .y = v1.y * m };
}

float vsqlen (VECTOR2D v)
{
    return v.x * v.x + v.y * v.y;
}

float vlen (VECTOR2D v)
{
    return (float) sqrt (vsqlen (v));
}

float vdot (VECTOR2D v1, VECTOR2D v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

VECTOR2D vnormalize (VECTOR2D v)
{
    float len = vlen (v);
    if (!len) return v;

    float div = 1.0f / len;
    return (VECTOR2D) { .x = v.x * div, .y = v.y * div };
}

VECTOR2D vabs (VECTOR2D v)
{
    return (VECTOR2D) { .x = fabs (v.x), .y = fabs (v.y) };
}

void vdebug (VECTOR2D v)
{
    debug ("Vector2D x: %f, y: %f", v.x, v.y);
}


