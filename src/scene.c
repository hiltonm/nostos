/*
 * See LICENSE for copyright information.
 */

#include "scene.h"

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
    aabb_free (scene->collision_tree);
    aabb_free (scene->portal_tree);
    al_free (scene);
}

SCENES *scene_load_file (const char *filename)
{
    ALLEGRO_CONFIG *config = al_load_config_file (filename);

    if (!config)
        return NULL;

    ALLEGRO_CONFIG_SECTION *it = NULL;
    const char *section = al_get_first_config_section (config, &it);

    SCENES *scenes = al_calloc (1, sizeof (SCENES));
    scenes->scenes = _al_list_create ();

    while (section) {
        SCENE *scene = al_calloc (1, sizeof (SCENE));

        scene->name = strdup (section);
        scene->map_filename = strdup (al_get_config_value (config, section, "map"));
        scene->npc_layer_name = strdup (al_get_config_value (config, section, "npc_layer"));
        scene->collision_layer_name = strdup (al_get_config_value (config, section, "collision_layer"));
        scene->portal_layer_name = strdup (al_get_config_value (config, section, "portal_layer"));

        scenes->tree = _al_aa_insert (scenes->tree, scene->name, scene, charcmp);
        _al_list_push_back_ex (scenes->scenes, scene, dtor_scene);
        section = al_get_next_config_section (&it);
    }

    al_destroy_config (config);
    return scenes;
}

SCENE *scene_get (SCENES *scenes, const char *scene_name)
{
    return _al_aa_search (scenes->tree, scene_name, charcmp);
}

SCENE *scene_load (SCENE *scene, SPRITES *sprites)
{
    assert (scene);
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

    return scene;
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
    _al_aa_free (scenes->tree);
    _al_list_destroy (scenes->scenes);
    al_free (scenes);
}

