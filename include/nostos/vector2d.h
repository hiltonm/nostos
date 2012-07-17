/*
 * See LICENSE for copyright information.
 */

#ifndef _vector2d_h_
#define _vector2d_h_

typedef struct VECTOR2D {
    float x, y;
} VECTOR2D;

VECTOR2D vadd (VECTOR2D v1, VECTOR2D v2);
VECTOR2D vsub (VECTOR2D v1, VECTOR2D v2);
VECTOR2D vmul (VECTOR2D v1, VECTOR2D v2);
VECTOR2D vmulf (VECTOR2D v1, float f);
VECTOR2D vdiv (VECTOR2D v1, VECTOR2D v2);
VECTOR2D vdivf (VECTOR2D v1, float f);
float vsqlen (VECTOR2D v);
float vlen (VECTOR2D v);
float vdot (VECTOR2D v1, VECTOR2D v2);
VECTOR2D vnormalize (VECTOR2D v);
VECTOR2D vabs (VECTOR2D v);
float vdistance (VECTOR2D v1, VECTOR2D v2);
float vsqdistance (VECTOR2D v1, VECTOR2D v2);
void vdebug (VECTOR2D v);

#endif
