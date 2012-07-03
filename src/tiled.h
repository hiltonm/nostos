/*
 * See LICENSE for copyright information.
 */

#ifndef _tiled_h_
#define _tiled_h_

#include "utils.h"

typedef struct TILED_MAP TILED_MAP;
typedef struct TILED_LAYER TILED_LAYER;
typedef struct TILED_LAYER_TILE TILED_LAYER_TILE;
typedef struct TILED_LAYER_OBJECT TILED_LAYER_OBJECT;
typedef struct TILED_TILESET TILED_TILESET;
typedef struct TILED_TILE TILED_TILE;
typedef struct TILED_OBJECT TILED_OBJECT;
typedef struct TILED_OBJECT_RECT TILED_OBJECT_RECT;
typedef struct TILED_OBJECT_TILE TILED_OBJECT_TILE;
typedef struct TILED_OBJECT_GEOM TILED_OBJECT_GEOM;

enum TILED_ORIENTATION {
    ORIENTATION_UNKNOWN,
    ORIENTATION_ORTHOGONAL,
    ORIENTATION_ISOMETRIC,
    ORIENTATION_STAGGERED
};

enum TILED_LAYER_TYPE {
    LAYER_TYPE_TILE   = 1,
    LAYER_TYPE_OBJECT = 2,
    LAYER_TYPE_IMAGE  = 4,
    LAYER_TYPE_ANY    = 0xFF
};

enum TILED_OBJECT_GEOM_TYPE {
    GEOM_TYPE_POLYLINE,
    GEOM_TYPE_POLYGON
};

enum TILED_OBJECT_TYPE {
    OBJECT_TYPE_RECT,
    OBJECT_TYPE_TILE,
    OBJECT_TYPE_GEOM,
};


struct TILED_MAP {
    int width;
    int height;
    int tile_width;
    int tile_height;
    int orientation;
    LIST *tilesets;
    LIST *layers;
    AATREE *tiles;
    AATREE *properties;
    LIST *layers_back;
    LIST *layers_fore;
    LIST *strings;
};

struct TILED_OBJECT {
    int x, y;
    int type;
    char *type_str;
    AATREE *properties;
};

struct TILED_OBJECT_RECT {
    TILED_OBJECT object;
    int width, height;
};

struct TILED_OBJECT_TILE {
    TILED_OBJECT object;
    TILED_TILE *tile;
};

struct TILED_OBJECT_GEOM {
    TILED_OBJECT object;
    int type;
    float *points;
    int num_points;
};

struct TILED_LAYER {
    char *name;
    int type;
    int x, y;
    int width;
    int height;
    float opacity;
    bool visible;
    TILED_MAP *map;
    AATREE *properties;
};

struct TILED_LAYER_TILE {
    TILED_LAYER layer;
    TILED_TILE ***tiles;
};

struct TILED_LAYER_OBJECT {
    TILED_LAYER layer;
    LIST *objects;
};

struct TILED_TILESET {
    char *name;
    char *image_source;
    ALLEGRO_COLOR transparent_color;
    int tile_width;
    int tile_height;
    int spacing;
    int margin;
    int tile_offset_x;
    int tile_offset_y;
    int first_gid;
    int num_tiles;
    int image_width;
    int image_height;
    ALLEGRO_BITMAP *bitmap;
    TILED_TILE *tiles;
    AATREE *properties;
};

struct TILED_TILE {
    int id;
    int gid;
    TILED_TILESET *tileset;
    ALLEGRO_BITMAP *bitmap;
    _AL_AATREE *properties;
};

#define NULL_TILE {0, 0, NULL, NULL, NULL}


TILED_MAP* tiled_load_tmx_file (const char *filename);
TILED_LAYER* tiled_layer_by_name (TILED_MAP *map, const char *name);
void tiled_draw_map (TILED_MAP *map, float sx, float sy, float sw, float sh, float dx, float dy, int flags);
void tiled_draw_layers (LIST *layers, float sx, float sy, float sw, float sh, float dx, float dy, int flags);
void tiled_draw_map_back (TILED_MAP *map, float sx, float sy, float sw, float sh, float dx, float dy, int flags);
void tiled_draw_map_fore (TILED_MAP *map, float sx, float sy, float sw, float sh, float dx, float dy, int flags);
void tiled_free_map (TILED_MAP *map);

#endif
