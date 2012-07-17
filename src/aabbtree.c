/*
 * See LICENSE for copyright information.
 */


#include "nostos/aabbtree.h"
#include "nostos/utils.h"

#include <float.h>

typedef struct AUX_NODE {
    BOX aabb;
    int left, right;
    VECTOR boxes;
} AUX_NODE;

static inline AUX_NODE *new_aux_node ()
{
    AUX_NODE *auxnode = al_malloc (sizeof (AUX_NODE));
    _al_vector_init (&auxnode->boxes, sizeof (BOX));
    auxnode->left = -1;
    auxnode->right = -1;

    return auxnode;
}

static inline bool vector_add (VECTOR *v, const void *item)
{
    return _al_vector_append_array (v, 1, item);
}

static inline int classify_aabb (BOX *box, int axis, int middle)
{
    if ((axis == 0 && box->center.x <= middle) || (axis == 1 && box->center.y <= middle))
        return 0;
    else
        return 1;
}

static inline void aabb_debug_auxnode (AUX_NODE *node)
{
    debug ("AuxNode");
    box_debug (node->aabb);
    debug ("Left: %d, Right: %d", node->left, node->right);
}

static inline void process_node (AUX_NODE *node, VECTOR *auxnodes, int max_depth)
{
    int size = _al_vector_size (&node->boxes);

    if (size <= max_depth) {
        node->left = -1;
        node->right = -1;
        return;
    }

    int axis;
    float middle;

    VECTOR2D bmin = (VECTOR2D){FLT_MAX, FLT_MAX};
    VECTOR2D bmax = (VECTOR2D){-FLT_MAX, -FLT_MAX};

    for (int i = 0; i < size; i++) {
        BOX *box = _al_vector_ref (&node->boxes, i);
        VECTOR2D left = vsub (box->center, box->extent);
        VECTOR2D right = vadd (box->center, box->extent);
        if (left.x < bmin.x) bmin.x = left.x;
        if (left.y < bmin.y) bmin.y = left.y;

        if (right.x > bmax.x) bmax.x = right.x;
        if (right.y > bmax.y) bmax.y = right.y;
    }

    node->aabb = box_from_points (bmin, bmax);
    box_debug (node->aabb);

    VECTOR2D center = node->aabb.center;
    VECTOR2D ext = node->aabb.extent;

    if (ext.x > ext.y) {
        axis = 0;
        middle = center.x;
    }
    else {
        axis = 1;
        middle = center.y;
    }

    VECTOR2D bboxmin = vsub (center, ext);
    VECTOR2D bboxmax = vadd (center, ext);

    debug ("BBox Min");
    vdebug (bboxmin);
    debug ("BBox Max");
    vdebug (bboxmax);

    AUX_NODE *left = NULL;
    AUX_NODE *right = NULL;

    for (int i = 0; i < size; i++) {
        BOX *box = _al_vector_ref (&node->boxes, i);

        float min, max;
        int res = classify_aabb (box, axis, middle);
        if (axis == 0) {
            min = box->center.x - box->extent.x;
            max = box->center.x + box->extent.x;
        } else {
            min = box->center.y - box->extent.y;
            max = box->center.y + box->extent.y;
        }

        if (res == 0) {
            debug ("Left");
            if (!left) {
                left = new_aux_node ();
                debug ("Dividing");
                if (axis == 0)
                    left->aabb = box_from_points (bboxmin, (VECTOR2D){(bboxmin.x + bboxmax.x) / 2.0f, bboxmax.y});
                else
                    left->aabb = box_from_points (bboxmin, (VECTOR2D){bboxmax.x, (bboxmin.y + bboxmax.y) / 2.0f});
                box_debug (left->aabb);
            }
            BOX aabb = left->aabb;
            if (axis == 0 && max > (aabb.center.x + aabb.extent.x))
                left->aabb = box_from_points (bboxmin, (VECTOR2D){max, bboxmax.y});
            else if (axis == 1 && max > (aabb.center.y + aabb.extent.y))
                left->aabb = box_from_points (bboxmin, (VECTOR2D){bboxmax.x, max});
            debug ("Final");
            box_debug (left->aabb);
            vector_add (&left->boxes, box);
        }
        else {
            debug ("Right");
            if (!right) {
                right = new_aux_node ();
                debug ("Dividing");
                if (axis == 0)
                    right->aabb = box_from_points ((VECTOR2D){(bboxmin.x + bboxmax.x) / 2.0f, bboxmin.y}, bboxmax);
                else
                    right->aabb = box_from_points ((VECTOR2D){bboxmin.x, (bboxmin.y + bboxmax.y) / 2.0f}, bboxmax);
                box_debug (right->aabb);
            }
            BOX aabb = right->aabb;
            if (axis == 0 && min < (aabb.center.x - aabb.extent.x))
                right->aabb = box_from_points ((VECTOR2D){min, bboxmin.y}, bboxmax);
            else if (axis == 1 && min < (aabb.center.y - aabb.extent.y))
                right->aabb = box_from_points ((VECTOR2D){bboxmin.x, min}, bboxmax);
            debug ("Final");
            box_debug (right->aabb);
            vector_add (&right->boxes, box);
        }
    }

    if (left && !right) {
        right = new_aux_node ();
        right->aabb = left->aabb;
        int num_boxes = _al_vector_size (&left->boxes);
        int le = num_boxes / 2;
        int start = num_boxes - le;
        for (int i = start; i < num_boxes; i++) {
            BOX *box = _al_vector_ref (&left->boxes, i);
            vector_add (&right->boxes, box);
        }
        vector_shrink (&left->boxes, le);
    }
    else if (!left && right) {
        left = new_aux_node ();
        left->aabb = right->aabb;
        int num_boxes = _al_vector_size (&right->boxes);
        int le = num_boxes / 2;
        int start = num_boxes - le;
        for (int i = start; i < num_boxes; i++) {
            BOX *box = _al_vector_ref (&right->boxes, i);
            vector_add (&left->boxes, box);
        }
        vector_shrink (&right->boxes, le);
    }

    if (left)
        node->left = _al_vector_size (auxnodes);
    else
        node->left = -1;

    if (right) {
        if (node->left != -1)
            node->right = node->left + 1;
        else
            node->right = _al_vector_size (auxnodes);
    }
    else
        node->right = -1;

    if (left) {
        vector_add (auxnodes, left);
        al_free (left);
    }

    if (right) {
        vector_add (auxnodes, right);
        al_free (right);
    }
}

AABB_TREE *aabb_build_tree (BOX *boxes, int num_boxes, int max_depth)
{
    AABB_TREE *tree = al_malloc (sizeof (AABB_TREE));
    tree->max_depth = max_depth;

    debug ("num boxes: %d, max depth: %d", num_boxes, max_depth);

    VECTOR auxnodes;
    _al_vector_init (&auxnodes, sizeof (AUX_NODE));

    AUX_NODE *auxnode = new_aux_node ();
    _al_vector_append_array (&auxnode->boxes, num_boxes, boxes);

    vector_add (&auxnodes, auxnode);

    al_free (auxnode);
    auxnode = NULL;
    int size = _al_vector_size (&auxnodes);

    int off = 0;
    while(true) {
        AUX_NODE *node = _al_vector_ref (&auxnodes, off++);
        process_node (node, &auxnodes, max_depth);
        if (off == _al_vector_size (&auxnodes))
            break;
    }

    size = _al_vector_size (&auxnodes);
    debug ("Number of auxnodes: %d", size);

    int *ln = al_malloc (size * sizeof (int));
    int num_nodes = 0;
    int num_leafs = 0;

    for (int i = 0; i < size; i++) {
        AUX_NODE *node = _al_vector_ref (&auxnodes, i);
        aabb_debug_auxnode (node);
        if (node->left == -1 && node->right == -1)
            ln[i] = num_leafs++;
        else
            ln[i] = num_nodes++;
    }

    tree->root = al_malloc (num_nodes * sizeof (AABB_NODE));
    tree->leafs = al_malloc (num_leafs * sizeof (AABB_LEAF));
    tree->num_nodes = num_nodes;
    tree->num_leafs = num_leafs;
    debug ("Number of nodes: %d", num_nodes);
    debug ("Number of leafs: %d", num_leafs);

    for (int i = 0; i < size; i++) {
        auxnode = _al_vector_ref (&auxnodes, i);

        if (auxnode->left == -1 && auxnode->right == -1) {
            int i_leaf = ln[i];
            AABB_LEAF *leaf = &tree->leafs[i_leaf];
            debug ("New leaf");
            leaf->node.aabb = auxnode->aabb;
            debug ("AABB");
            box_debug (leaf->node.aabb);
            leaf->node.left = NULL;
            leaf->node.right = NULL;
            leaf->num_boxes = _al_vector_size (&auxnode->boxes);
            leaf->boxes = al_malloc (leaf->num_boxes * sizeof (BOX));
            debug ("Boxes");
            for (int j = 0; j < leaf->num_boxes; j++) {
                BOX *box = &leaf->boxes[j];
                *box = *(BOX *)_al_vector_ref (&auxnode->boxes, j);
                box_debug (leaf->boxes[j]);
            }
        } else {
            int i_node = ln[i];
            AABB_NODE *node = &tree->root[i_node];
            debug ("New node");
            node->aabb = auxnode->aabb;
            debug ("AABB");
            box_debug (node->aabb);

            if (auxnode->left != -1) {
                debug ("Left OK");
                AUX_NODE *left_node = _al_vector_ref (&auxnodes, auxnode->left);
                if (left_node->left == -1 && left_node->right == -1) {
                    node->left = (AABB_NODE*) &tree->leafs[ln[auxnode->left]];
                } else {
                    node->left = &tree->root[ln[auxnode->left]];
                }
            } else {
                debug ("Left NULL");
                node->left = NULL;
            }

            if (auxnode->right != -1) {
                debug ("Right OK");
                AUX_NODE *right_node = _al_vector_ref (&auxnodes, auxnode->right);
                if (right_node->left == -1 && right_node->right == -1) {
                    node->right = (AABB_NODE*) &tree->leafs[ln[auxnode->right]];
                } else {
                    node->right = &tree->root[ln[auxnode->right]];
                }
            } else {
                debug ("Right NULL");
                node->right = NULL;
            }
        }
    }

    for (int i = 0; i < size; i++) {
        auxnode = _al_vector_ref (&auxnodes, i);
        _al_vector_free (&auxnode->boxes);
    }

    al_free (ln);
    _al_vector_free (&auxnodes);

    return tree;
}

AABB_TREE *aabb_load_tree (TILED_MAP *map, const char *layer_name)
{
    TILED_LAYER_OBJECT *layer = (TILED_LAYER_OBJECT *)tiled_layer_by_name (map, layer_name);
    BOX *boxes = NULL;
    int num_boxes = 0;

    if (layer && layer->objects) {
        LIST_ITEM *item = _al_list_front (layer->objects);
        num_boxes = _al_list_size (layer->objects);
        boxes = al_malloc (num_boxes * sizeof (BOX));
        int i = 0;

        while (item) {
            TILED_OBJECT *object = _al_list_item_data (item);
            switch (object->type) {
                TILED_OBJECT_RECT *object_rect;
                case OBJECT_TYPE_RECT:
                    object_rect = (TILED_OBJECT_RECT *)object;
                    boxes[i].extent = (VECTOR2D){object_rect->width / 2.0, object_rect->height / 2.0};
                    boxes[i].center = (VECTOR2D){boxes[i].extent.x + object->x, boxes[i].extent.y + object->y};
                    boxes[i].data = object_rect;
                    i++;
                    break;
                default:
                    debug ("FIX: Found unsupported object in box layer. Only rectangles are supported.");
                    break;
            }
            item = _al_list_next (layer->objects, item);
        }
    }

    if (boxes) {
        AABB_TREE *tree = aabb_build_tree (boxes, num_boxes, MIN (num_boxes - 1, 4));
        tree->use_cache = false;
        tree->collisions = NULL;
        free (boxes);
        return tree;
    }

    return NULL;
}

bool collide (AABB_TREE *tree, AABB_NODE *node, BOX *box)
{
    if (box_overlap (node->aabb, *box)) {
        if (!node->left && !node->right) {
            AABB_LEAF *leaf = (AABB_LEAF*)node;
            for (int i = 0; i < leaf->num_boxes; i++) {
                BOX *leaf_box = &leaf->boxes[i];

                bool overlap = box_overlap (*box, *leaf_box);
                if (overlap) {
                    tree->num_collisions++;
                    if (tree->collisions)
                        _al_list_push_back (tree->collisions->boxes, leaf_box);
                    return overlap;
                }
            }
        } else {
            if (node->left) {
                if (collide (tree, node->left, box))
                    return true;
            }
            if (node->right) {
                if (collide (tree, node->right, box))
                    return true;
            }
        }
    }

    return false;
}

void collide_fill (AABB_TREE *tree, AABB_NODE *node, BOX *box)
{
    if (box_overlap (node->aabb, *box)) {
        if (!node->left && !node->right) {
            AABB_LEAF *leaf = (AABB_LEAF*)node;
            for (int i = 0; i < leaf->num_boxes; i++) {
                BOX *leaf_box = &leaf->boxes[i];

                bool overlap = box_overlap (*box, *leaf_box);
                if (overlap) {
                    tree->num_collisions++;
                    if (tree->collisions)
                        _al_list_push_back (tree->collisions->boxes, leaf_box);
                }
            }
        } else {
            if (node->left) {
                collide_fill (tree, node->left, box);
            }
            if (node->right) {
                collide_fill (tree, node->right, box);
            }
        }
    }
}

bool aabb_collide (AABB_TREE *tree, BOX *box)
{
    assert (tree);
    assert (box);
    tree->collisions = NULL;
    tree->num_collisions = 0;
    return collide (tree, tree->root, box);
}

void aabb_init_collisions (AABB_COLLISIONS *col)
{
    assert (col);
    col->boxes = _al_list_create ();
}

bool aabb_collide_with_cache (AABB_TREE *tree, BOX *box, AABB_COLLISIONS *collisions)
{
    assert (tree);
    assert (box);
    assert (collisions);

    if (tree->use_cache && !_al_list_is_empty (collisions->boxes)) {
        LIST_ITEM *item = _al_list_front (collisions->boxes);
        if (box_overlap (*box, *(BOX*)_al_list_item_data (item))) {
            tree->num_collisions = 1;
            return true;
        }
    }

    collisions->query_box = *box;
    if (collisions->boxes)
        _al_list_clear (collisions->boxes);
    tree->collisions = collisions;
    tree->num_collisions = 0;

    return collide (tree, tree->root, box);
}

void aabb_collide_fill_cache (AABB_TREE *tree, BOX *box, AABB_COLLISIONS *collisions)
{
    assert (tree);
    assert (box);
    assert (collisions);

    if (tree->use_cache && !_al_list_is_empty (collisions->boxes)) {
        LIST_ITEM *item = _al_list_front (collisions->boxes);
        if (box_overlap (*box, *(BOX*)_al_list_item_data (item))) {
            tree->num_collisions = 1;
            return;
        }
    }

    collisions->query_box = *box;
    if (collisions->boxes)
        _al_list_clear (collisions->boxes);
    tree->collisions = collisions;
    tree->num_collisions = 0;

    return collide_fill (tree, tree->root, box);
}

void aabb_free (AABB_TREE *tree)
{
    if (!tree)
        return;

    al_free (tree->root);

    for (int i = 0; i < tree->num_leafs; i++)
        al_free (tree->leafs[i].boxes);

    al_free (tree->leafs);
    al_free (tree);
}

void aabb_free_collisions (AABB_COLLISIONS *col)
{
    assert (col);
    if (col->boxes)
        _al_list_destroy (col->boxes);
}

void aabb_draw_node (AABB_NODE *node, SCREEN *s, ALLEGRO_COLOR color)
{
    if (!node)
        return;

    box_draw (node->aabb, s->position, color);
    aabb_draw_node (node->left, s, color);
    aabb_draw_node (node->right, s, color);
}

void aabb_draw (AABB_TREE *tree, SCREEN *s, ALLEGRO_COLOR color)
{
    aabb_draw_node (tree->root, s, color);
}

