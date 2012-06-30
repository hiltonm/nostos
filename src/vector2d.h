/*
 * See LICENSE for copyright information.
 */

#ifndef _vector2d_h_
#define _vector2d_h_

typedef struct VECTOR2D {
    float x, y;
} VECTOR2D;

typedef float (*VECTOR2D_APPLY)(float f);

VECTOR2D vadd (const VECTOR2D* v1, const VECTOR2D* v2);
VECTOR2D vsub (const VECTOR2D* v1, const VECTOR2D* v2);
VECTOR2D vmul (const VECTOR2D* v1, const VECTOR2D* v2);
VECTOR2D vmulf (const VECTOR2D* v1, float f);
VECTOR2D vdiv (const VECTOR2D* v1, const VECTOR2D* v2);
VECTOR2D vdivf (const VECTOR2D* v1, float f);
void vatadd (VECTOR2D* v1, const VECTOR2D* v2);
void vatsub (VECTOR2D* v1, const VECTOR2D* v2);
void vatmul (VECTOR2D* v1, const VECTOR2D* v2);
void vatmulf (VECTOR2D* v1, float f);
void vatdiv (VECTOR2D* v1, const VECTOR2D* v2);
void vatdivf (VECTOR2D* v1, float f);
float vsqlen (const VECTOR2D* v);
float vlen (const VECTOR2D* v);
float vdot (const VECTOR2D* v1, const VECTOR2D* v2);
void vnormalize (VECTOR2D* v);
void vabs (VECTOR2D* v);
void vdebug (VECTOR2D* v);

#endif
