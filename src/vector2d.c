/*
 * See LICENSE for copyright information.
 */

#include "vector2d.h"

#include <math.h>
#include <assert.h>

#include "utils.h"

VECTOR2D vadd (const VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    return (VECTOR2D) { .x = v1->x + v2->x, .y = v1->y + v2->y };
}

VECTOR2D vsub (const VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    return (VECTOR2D) { .x = v1->x - v2->x, .y = v1->y - v2->y };
}

VECTOR2D vmul (const VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    return (VECTOR2D) { .x = v1->x * v2->x, .y = v1->y * v2->y };
}

VECTOR2D vmulf (const VECTOR2D* v1, float f)
{
    assert (v1);
    return (VECTOR2D) { .x = v1->x * f, .y = v1->y * f };
}

VECTOR2D vdiv (const VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    return (VECTOR2D) { .x = v1->x / v2->x, .y = v1->y / v2->y };
}

VECTOR2D vdivf (const VECTOR2D* v1, float f)
{
    assert (v1);
    float m = 1.0f / f;
    return (VECTOR2D) { .x = v1->x * m, .y = v1->y * m };
}

void vatadd (VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    v1->x += v2->x;
    v1->y += v2->y;
}

void vatsub (VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    v1->x -= v2->x;
    v1->y -= v2->y;
}

void vatmul (VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    v1->x *= v2->x;
    v1->y *= v2->y;
}

void vatmulf (VECTOR2D* v1, float f)
{
    assert (v1);
    v1->x *= f;
    v1->y *= f;
}

void vatdiv (VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    v1->x /= v2->x;
    v1->y /= v2->y;
}

void vatdivf (VECTOR2D* v1, float f)
{
    assert (v1);
    float m = 1.0f / f;
    v1->x *= m;
    v1->y *= m;
}

float vsqlen (const VECTOR2D* v)
{
    assert (v);
    return v->x * v->x + v->y * v->y;
}

float vlen (const VECTOR2D* v)
{
    assert (v);
    return (float) sqrt (vsqlen (v));
}

float vdot (const VECTOR2D* v1, const VECTOR2D* v2)
{
    assert (v1 && v2);
    return v1->x * v2->x + v1->y * v2->y;
}

void vnormalize (VECTOR2D* v)
{
    assert (v);
    float len = vlen (v);
    if (!len) return;

    float div = 1.0f / len;
    v->x *= div;
    v->y *= div;
}

void vabs (VECTOR2D* v)
{
    assert (v);
    v->x = fabs (v->x);
    v->y = fabs (v->y);
}

void vdebug (VECTOR2D* v)
{
    assert (v);
    debug ("Vector2D x: %f, y: %f", v->x, v->y);
}


