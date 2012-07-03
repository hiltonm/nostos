/*
 * See LICENSE for copyright information.
 */

#include "tiled.h"

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include <ctype.h>
#include <string.h>
#include <math.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "utils.h"


static LIST *get_children_for_name(xmlNode *parent, int nargs, ...)
{
    va_list ap;
    LIST *list = _al_list_create();
    xmlNode *child = parent->children->next;

    while (child != NULL) {
        va_start (ap, nargs);
        for (int i = 0; i < nargs; i++) {
            const char* name = va_arg (ap, const char*);
            if (!strcmp ((const char*)child->name, name))
                _al_list_push_back (list, child);
        }
        va_end (ap);

        child = child->next;
    }

    return list;
}

static xmlNode *get_first_child_for_name(xmlNode *parent, const char *name)
{
    assert (parent);

    if (!parent->children)
        return NULL;

	xmlNode *child = parent->children->next;

	while  (child != NULL) {
		if (!strcmp((const char*)child->name, name))
			return child;

		child = child->next;
	}

	return NULL;
}

static char *get_xml_attribute (xmlNode *node, const char *name)
{
	xmlAttr *attrs = node->properties;

	while (attrs != NULL) {
		if (!strcmp ((const char*)attrs->name, name))
			return (char *)attrs->children->content;

		attrs = attrs->next;
	}

	return NULL;
}

void tiled_free_map (TILED_MAP *map)
{
    _al_list_destroy(map->tilesets);
    _al_list_destroy(map->layers);
    _al_aa_free (map->properties);
    _al_list_destroy(map->strings);
    _al_aa_free (map->tiles);
    al_free(map);
}

static void dtor_tileset(void *value, void *user_data)
{
    TILED_TILESET *tileset = value;
    al_free (tileset->name);
    al_free (tileset->image_source);
    _al_aa_free (tileset->properties);
    al_destroy_bitmap (tileset->bitmap);

    for (int i=0; i<tileset->num_tiles; i++) {
        _al_aa_free (tileset->tiles[i].properties);
        al_destroy_bitmap (tileset->tiles[i].bitmap);
    }
    al_free (tileset->tiles);
    al_free (tileset);
}

static void dtor_layer(void *value, void *user_data)
{
    TILED_LAYER *layer = (TILED_LAYER*)value;
    al_free (layer->name);
    _al_aa_free (layer->properties);

    switch (layer->type) {
        TILED_LAYER_TILE *tile_layer;
        TILED_LAYER_OBJECT *object_layer;
        case LAYER_TYPE_TILE:
            tile_layer = (TILED_LAYER_TILE *) layer;
            for (int i = 0; i < layer->height; i++)
                al_free (tile_layer->tiles[i]);
            al_free (tile_layer->tiles);
            break;
        case LAYER_TYPE_OBJECT:
            object_layer = (TILED_LAYER_OBJECT *) layer;
            _al_list_destroy (object_layer->objects);
            break;
    }
    al_free (layer);
}

static void dtor_object (void *value, void *user_data)
{
    TILED_OBJECT *cobject = (TILED_OBJECT *) value;
    al_free (cobject->type_str);
    _al_aa_free (cobject->properties);

    switch (cobject->type) {
        TILED_OBJECT_GEOM *object_geom;
        case OBJECT_TYPE_GEOM:
            object_geom = (TILED_OBJECT_GEOM *) cobject;
            al_free (object_geom->points);
            //al_free (object_geom);
            break;
    }

    al_free (cobject);
}


static inline int get_int (xmlNode *node, const char *name, int def)
{
    char *attr = get_xml_attribute (node, name);
    return attr ? atoi (attr) : def;
}

static inline float get_float (xmlNode *node, const char *name, float def)
{
    char *attr = get_xml_attribute (node, name);
    return attr ? atof (attr) : def;
}

static inline void draw_polyline (float *points, int num_points)
{
    if (!points)
        return;

    al_draw_polyline (points, num_points,
                      ALLEGRO_LINE_JOIN_NONE,
                      ALLEGRO_LINE_CAP_NONE,
                      al_map_rgba_f (1, 1, 1, 1), 2, 0);
}

static void offset_points (int px, int py, float *points, int num_points)
{
    for (int i = 0; i < num_points * 2; i += 2) {
        points[i] += px;
        points[i + 1] += py;
    }
}

static inline void draw_polygon (float *points, int num_points)
{
    if (!points)
        return;

    al_draw_polygon (points, num_points,
                     ALLEGRO_LINE_JOIN_NONE,
                     al_map_rgba_f (1, 1, 1, 1), 2, 0);
}

static inline float * get_float_points (xmlNode *node, const char *name, int *num_points)
{
    assert (node);
    assert (name);
    assert (num_points);

    char *attr = get_xml_attribute (node, name);

    if (attr) {
        int i = 0;
        float *points = NULL;
        LIST *nums_str = split (attr, " ,");

        if (!_al_list_is_empty (nums_str)) {
            *num_points = _al_list_size (nums_str) / 2;
            points = al_malloc (*num_points * 2 * sizeof (float));

            LIST_ITEM *item = _al_list_front (nums_str);
            while (item) {
                points[i++] = atof ((char *)_al_list_item_data (item));
                item = _al_list_next (nums_str, item);
            }
        }
        _al_list_destroy (nums_str);
        return points;
    }

    return NULL;
}

static inline char *get_str (xmlNode *node, const char *name)
{
    char *str = get_xml_attribute (node, name);
    return str ? strdup (str) : NULL;
}

static AATREE *get_properties (xmlNode *node, TILED_MAP *map)
{
    assert (node);

    xmlNode *props_node = get_first_child_for_name (node, "properties");

    if (!props_node)
        return NULL;

    LIST *property_nodes = get_children_for_name (props_node, 1, "property");

    if (_al_list_is_empty (property_nodes))
        return NULL;

    AATREE *props = NULL;

    LIST_ITEM *property_item = _al_list_front (property_nodes);
    while (property_item) {
        xmlNode *prop_node = (xmlNode*)_al_list_item_data (property_item);
        char *name = get_str (prop_node, "name");
        char *value = get_str (prop_node, "value");
        props = _al_aa_insert (props, (void *)name, (void *)value, charcmp);
        _al_list_push_back_ex (map->strings, name, dtor_string);
        _al_list_push_back_ex (map->strings, value, dtor_string);
        property_item = _al_list_next (property_nodes, property_item);
    }

    _al_list_destroy (property_nodes);

    return props;
}


TILED_MAP* tiled_load_tmx_file (const char *filename)
{
    TILED_MAP *map;
    xmlDoc *doc;
    xmlNode *root;
    char *str;

    LIBXML_TEST_VERSION

    doc = xmlReadFile (filename, NULL, 0);
    if (!doc)
        return NULL;

    ALLEGRO_PATH *mapdir = al_create_path (filename);
    al_set_path_filename (mapdir, NULL);

    if (!al_change_directory (al_path_cstr (mapdir, ALLEGRO_NATIVE_PATH_SEP))) {
        printf ("Failed to change directory.");
    }

    al_destroy_path (mapdir);

    root = xmlDocGetRootElement (doc);

    map = al_malloc (sizeof (TILED_MAP));
    map->width = get_int (root, "width", 0);
    map->height = get_int (root, "height", 0);
    map->tile_width = get_int (root, "tilewidth", 0);
    map->tile_height = get_int (root, "tileheight", 0);
    map->strings = _al_list_create ();
    map->properties = get_properties (root, map);
    map->tiles = NULL;

    str = get_xml_attribute (root, "orientation");
    if (!strcmp (str, "orthogonal"))
        map->orientation = ORIENTATION_ORTHOGONAL;
    else if (!strcmp (str, "isometric"))
        map->orientation = ORIENTATION_ISOMETRIC;
    else if (!strcmp (str, "staggered"))
        map->orientation = ORIENTATION_STAGGERED;
    else
        map->orientation = ORIENTATION_UNKNOWN;

    // Tilesets
    LIST *tileset_nodes = get_children_for_name (root, 1, "tileset");
    map->tilesets = create_list (_al_list_size (tileset_nodes));

    LIST_ITEM *tileset_item = _al_list_front (tileset_nodes);
    while (tileset_item) {
        xmlNode *tileset_node = _al_list_item_data (tileset_item);
        TILED_TILESET *tileset = al_malloc (sizeof (TILED_TILESET));
        tileset->first_gid = get_int (tileset_node, "firstgid", 1);
        tileset->tile_width = get_int (tileset_node, "tilewidth", 0);
        tileset->tile_height = get_int (tileset_node, "tileheight", 0);
        tileset->name = get_str (tileset_node, "name");
        tileset->properties = get_properties (tileset_node, map);

        xmlNode *image_node = get_first_child_for_name (tileset_node, "image");
        tileset->image_width = get_int (image_node, "width", 0);
        tileset->image_height = get_int (image_node, "height", 0);
        tileset->image_source = get_str (image_node, "source");
        tileset->bitmap = al_load_bitmap (tileset->image_source);

        LIST *tile_nodes = get_children_for_name (tileset_node, 1, "tile");

        int tiles_per_row = tileset->image_width / tileset->tile_width;
        tileset->num_tiles = (tileset->image_width * tileset->image_height) /
                             (tileset->tile_width * tileset->tile_height);

        TILED_TILE *tile;
        tileset->tiles = al_malloc (tileset->num_tiles * sizeof (TILED_TILE));
        for (int i = 0; i < tileset->num_tiles; i++) {
            tile = &tileset->tiles[i];
            tile->id = i;
            tile->gid = i + tileset->first_gid;
            tile->tileset = tileset;
            tile->properties = NULL;

            int x = (i % tiles_per_row) * tileset->tile_width;
            int y = (i / tiles_per_row) * tileset->tile_height;
            tile->bitmap = al_create_sub_bitmap(tileset->bitmap, x, y,
                                                tileset->tile_width,
                                                tileset->tile_height);

            map->tiles = _al_aa_insert (map->tiles, &tile->gid, tile, intcmp);
        }

		LIST_ITEM *tile_item = _al_list_front (tile_nodes);
		while (tile_item) {
			xmlNode *tile_node = (xmlNode*)_al_list_item_data (tile_item);
            int id = get_int (tile_node, "id", 0);

            tile = &tileset->tiles[id];
            tile->properties = get_properties (tile_node, map);

			tile_item = _al_list_next (tile_nodes, tile_item);
		}

		_al_list_destroy (tile_nodes);
		_al_list_push_back_ex (map->tilesets, tileset, dtor_tileset);
		tileset_item = _al_list_next (tileset_nodes, tileset_item);
    }

    _al_list_destroy (tileset_nodes);

    // Layers
    LIST *layer_nodes = get_children_for_name (root, 2, "layer", "objectgroup");
    map->layers = create_list (_al_list_size (layer_nodes));
    map->layers_fore = _al_list_create ();
    map->layers_back = _al_list_create ();

    LIST_ITEM *layer_item = _al_list_front (layer_nodes);
    while (layer_item) {
        xmlNode *layer_node = _al_list_item_data (layer_item);
        TILED_LAYER *layer = NULL;

        const char* type_str = (const char *)layer_node->name;
        if (!strcmp (type_str, "layer")) {
            layer = al_malloc (sizeof (TILED_LAYER_TILE));
            layer->type = LAYER_TYPE_TILE;
        }
        else if (!strcmp (type_str, "objectgroup")) {
            layer = al_malloc (sizeof (TILED_LAYER_OBJECT));
            layer->type = LAYER_TYPE_OBJECT;
        }

        layer->name = get_str (layer_node, "name");
        layer->width = get_int (layer_node, "width", 0);
        layer->height = get_int (layer_node, "height", 0);
        layer->opacity = get_float (layer_node, "opacity", 1.0);
        layer->map = map;
        layer->properties = get_properties (layer_node, map);

        char *order = _al_aa_search (layer->properties, "order", charcmp);

        if (layer->type == LAYER_TYPE_TILE) {
            TILED_LAYER_TILE *tile_layer = (TILED_LAYER_TILE *)layer;
            tile_layer->tiles = al_malloc (layer->height * sizeof (TILED_TILE*));

            xmlNode *data_node = get_first_child_for_name (layer_node, "data");
            LIST *tile_nodes = get_children_for_name (data_node, 1, "tile");

            LIST_ITEM *tile_item = _al_list_front (tile_nodes);
            for (int j = 0; j < layer->height; j++) {
                tile_layer->tiles[j] = al_malloc (layer->width * sizeof (TILED_TILE*));
                for (int k = 0; k < layer->width; k++) {
                    xmlNode *tile_node = (xmlNode*)_al_list_item_data (tile_item);
                    int id = get_int (tile_node, "gid", 0);

                    if (id == 0) {
                        tile_layer->tiles[j][k] = NULL;
                    } else {
                        TILED_TILE *tile = _al_aa_search (map->tiles, &id, intcmp);
                        tile_layer->tiles[j][k] = tile;
                    }

                    tile_item = _al_list_next (tile_nodes, tile_item);
                }
            }
            _al_list_destroy (tile_nodes);
        } else if (layer->type == LAYER_TYPE_OBJECT) {
            TILED_LAYER_OBJECT *object_layer = (TILED_LAYER_OBJECT *)layer;

            object_layer->objects = _al_list_create ();
            LIST *object_nodes = get_children_for_name (layer_node, 1, "object");

            LIST_ITEM *object_item = _al_list_front (object_nodes);
            while (object_item) {
                xmlNode *object_node = (xmlNode*)_al_list_item_data (object_item);
                TILED_OBJECT *cobj = NULL;

                int width = get_int (object_node, "width", 0);
                int gid = get_int (object_node, "gid", 0);
                int px = get_int (object_node, "x", 0);
                int py = get_int (object_node, "y", 0);

                if (width > 0) {
                    TILED_OBJECT_RECT *obj = al_malloc (sizeof (TILED_OBJECT_RECT));
                    obj->object.type = OBJECT_TYPE_RECT;
                    obj->width = width;
                    obj->height = get_int (object_node, "height", 0);

                    cobj = (TILED_OBJECT *) obj;
                } else if (gid > 0) {
                    TILED_OBJECT_TILE *obj = al_malloc (sizeof (TILED_OBJECT_TILE));
                    obj->object.type = OBJECT_TYPE_TILE;
                    obj->tile = _al_aa_search (map->tiles, &gid, intcmp);

                    cobj = (TILED_OBJECT *) obj;
                } else {
                    TILED_OBJECT_GEOM *obj = al_malloc (sizeof (TILED_OBJECT_GEOM));
                    obj->object.type = OBJECT_TYPE_GEOM;

                    xmlNode *poly_node = get_first_child_for_name (object_node, "polyline");
                    if (poly_node) {
                        obj->points = get_float_points (poly_node, "points", &obj->num_points);
                        offset_points (px, py, obj->points, obj->num_points);
                    } else {
                        poly_node = get_first_child_for_name (object_node, "polygon");
                        if (poly_node)
                            obj->points = get_float_points (poly_node, "points", &obj->num_points);
                            offset_points (px, py, obj->points, obj->num_points);
                    }
                    cobj = (TILED_OBJECT *) obj;
                }

                cobj->x = px;
                cobj->y = py;
                cobj->type_str = get_str (object_node, "type");
                cobj->properties = get_properties (object_node, map);

                _al_list_push_back_ex (object_layer->objects, cobj, dtor_object);
                object_item = _al_list_next (object_nodes, object_item);
            }
            _al_list_destroy (object_nodes);
        }

		layer_item = _al_list_next (layer_nodes, layer_item);
        _al_list_push_back_ex (map->layers, layer, dtor_layer);
        if (order && !strcmp (order, "fore"))
            _al_list_push_back (map->layers_fore, layer);
        else
            _al_list_push_back (map->layers_back, layer);
    }
    _al_list_destroy (layer_nodes);

    xmlFreeDoc (doc);
    xmlCleanupParser ();

    ALLEGRO_PATH *respath = al_get_standard_path (ALLEGRO_RESOURCES_PATH);
    al_change_directory (al_path_cstr (respath, ALLEGRO_NATIVE_PATH_SEP));

    return map;
}

void tiled_draw_map (TILED_MAP *map, float sx, float sy, float sw, float sh, float dx, float dy, int flags)
{
    tiled_draw_layers (map->layers, sx, sy, sw, sh, dx, dy, flags);
}

void tiled_draw_map_back (TILED_MAP *map, float sx, float sy, float sw, float sh, float dx, float dy, int flags)
{
    tiled_draw_layers (map->layers_back, sx, sy, sw, sh, dx, dy, flags);
}

void tiled_draw_map_fore (TILED_MAP *map, float sx, float sy, float sw, float sh, float dx, float dy, int flags)
{
    tiled_draw_layers (map->layers_fore, sx, sy, sw, sh, dx, dy, flags);
}

void tiled_draw_layers (LIST *layers, float sx, float sy, float sw, float sh, float dx, float dy, int flags)
{
    LIST_ITEM *layer_item = _al_list_front (layers);
    TILED_LAYER *layer = _al_list_item_data (layer_item);
    TILED_MAP *map = layer->map;
    int ti = floor (sx / map->tile_width);
    int tj = floor (sy / map->tile_height);
    int tw = MIN (ceil (sw / map->tile_width + ti) + 1, map->width);
    int th = MIN (ceil (sh / map->tile_height + tj) + 1, map->height);

    float fx = ti * map->tile_width - sx;
    float fy = tj * map->tile_height - sy;

    float x, y;

    while (layer_item) {
        layer = _al_list_item_data (layer_item);

        if (layer->type == LAYER_TYPE_TILE) {
            TILED_LAYER_TILE *tile_layer = (TILED_LAYER_TILE*) layer;
            x = fx;
            y = fy;
            for (int j = tj; j < th; j++) {
                x = fx;
                for (int i = ti; i < tw; i++) {
                    TILED_TILE *tile = tile_layer->tiles[j][i];

                    if (tile)
                        al_draw_bitmap (tile->bitmap, x, y, 0);

                    x += map->tile_width;
                }
                y += map->tile_height;
            }
        }

        layer_item = _al_list_next (layers, layer_item);
    }
}

TILED_LAYER* tiled_layer_by_name (TILED_MAP *map, const char *name)
{
    LIST_ITEM *item = _al_list_front (map->layers);

    while (item) {
        TILED_LAYER *layer = _al_list_item_data (item);

        if (!strcmp (layer->name, name))
            return layer;

        item = _al_list_next (map->layers, item);
    }

    return NULL;
}

