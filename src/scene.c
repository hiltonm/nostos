/*
 * See LICENSE for copyright information.
 */

#include "nostos/scene.h"

static void dtor_scene (void *value, void *user_data)
{
    SCENE *scene = value;
    al_free (scene->name);
    al_free (scene->map_filename);
    al_free (scene->npc_layer_name);
    al_free (scene->collision_layer_name);
    al_free (scene->portal_layer_name);
    tiled_free_map (scene->map);
    _al_list_destroy (scene->npcs);
    _al_list_destroy (scene->portals);
    aabb_free (scene->collision_tree);
    aabb_free (scene->portal_tree);
    al_free (scene);
}

static void dtor_portal (void *value, void *user_data)
{
    SCENE_PORTAL *portal = value;
    al_free (portal->name);
    al_free (portal->destiny_portal);
    al_free (portal);
}

SCENES *scene_load_file (const char *filename)
{
    ALLEGRO_CONFIG *config = al_load_config_file (filename);

    if (!config)
        return NULL;

    ALLEGRO_CONFIG_SECTION *it = NULL;
    const char *section = al_get_first_config_section (config, &it);

    SCENES *scenes = al_calloc (1, sizeof (SCENES));
    scenes->tree = NULL;
    scenes->portals = NULL;
    scenes->scenes = _al_list_create ();

    while (section) {
        SCENE *scene = al_calloc (1, sizeof (SCENE));

        scene->name = strdup (section);
        scene->map_filename = strdup (al_get_config_value (config, section, "map"));
        scene->npc_layer_name = strdup (al_get_config_value (config, section, "npc_layer"));
        scene->collision_layer_name = strdup (al_get_config_value (config, section, "collision_layer"));
        scene->portal_layer_name = strdup (al_get_config_value (config, section, "portal_layer"));

        scenes->tree = aa_insert (scenes->tree, scene->name, scene, charcmp);
        _al_list_push_back_ex (scenes->scenes, scene, dtor_scene);
        section = al_get_next_config_section (&it);
    }

    al_destroy_config (config);
    return scenes;
}

SCENE *scene_get (SCENES *scenes, const char *scene_name)
{
    return aa_search (scenes->tree, scene_name, charcmp);
}

SCENE *scene_load (SCENE *scene, SCENES *scenes, SPRITES *sprites)
{
    assert (scene);
    assert (scenes);
    assert (sprites);

    char *filename = get_resource_path_str (scene->map_filename);
    scene->map = tiled_load_tmx_file (filename);
    al_free (filename);

    char *layer_name = scene->npc_layer_name ? scene->npc_layer_name : "npc";
    scene->npcs = sprite_load_npcs (sprites, scene->map, layer_name);

    layer_name = scene->collision_layer_name ? scene->collision_layer_name : "collision";
    scene->collision_tree = aabb_load_tree (scene->map, layer_name);

    layer_name = scene->portal_layer_name ? scene->portal_layer_name : "portal";
    scene->portal_tree = aabb_load_tree (scene->map, layer_name);

    scene_load_portals (scene, scenes, layer_name);

    return scene;
}

void scene_load_scenes (SCENES *scenes, SPRITES *sprites)
{
    assert (scenes);

    LIST_ITEM *item = _al_list_front (scenes->scenes);
    while (item) {
        SCENE *scene = _al_list_item_data (item);
        scene_load (scene, scenes, sprites);
        item = _al_list_next (scenes->scenes, item);
    }
}

SCENE_PORTAL *scene_get_portal (SCENES *scenes, const char *portal_name)
{
    assert (scenes);
    if (!portal_name)
        return NULL;

    return aa_search (scenes->portals, portal_name, charcmp);
}

void scene_load_portals (SCENE *scene, SCENES *scenes, const char *layer_name)
{
    TILED_LAYER_OBJECT *layer = (TILED_LAYER_OBJECT *)tiled_layer_by_name (scene->map, layer_name);

    if (layer && layer->objects) {
        SCENE_PORTAL *portal;
        LIST_ITEM *item = _al_list_front (layer->objects);
        scene->portals = _al_list_create_static (_al_list_size (layer->objects));

        while (item) {
            TILED_OBJECT *object = _al_list_item_data (item);
            switch (object->type) {
                TILED_OBJECT_RECT *object_rect;
                case OBJECT_TYPE_RECT:
                    object_rect = (TILED_OBJECT_RECT *)object;
                    portal = al_malloc (sizeof (SCENE_PORTAL));

                    portal->name = strdup (object->name);
                    portal->scene = scene;
                    portal->destiny_portal = strdup (aa_search (object->properties, "portal", charcmp));
                    portal->position = (VECTOR2D){object_rect->width / 2.0 + object->x,
                                                  object_rect->height / 2.0 + object->y};
                    debug ("New portal %s", portal->name);
                    scenes->portals = aa_insert (scenes->portals, portal->name, portal, charcmp);
                    _al_list_push_back_ex (scene->portals, portal, dtor_portal);

                    break;
                default:
                    debug ("FIX: Found unsupported object in box layer. Only rectangles are supported.");
                    break;
            }
            item = _al_list_next (layer->objects, item);
        }
    }
}

SCENE *scene_unload (SCENE *scene)
{
    assert (scene);

    tiled_free_map (scene->map);
    _al_list_destroy (scene->npcs);
    aabb_free (scene->collision_tree);
    aabb_free (scene->portal_tree);
    scene->map = NULL;
    scene->npcs = NULL;
    scene->collision_tree = NULL;
    scene->portal_tree = NULL;

    return scene;
}

void scene_free (SCENES *scenes)
{
    aa_free (scenes->tree);
    aa_free (scenes->portals);
    _al_list_destroy (scenes->scenes);
    al_free (scenes);
}

