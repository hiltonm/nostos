/*
 * See LICENSE for copyright information.
 */

#ifndef _utils_h_
#define _utils_h_

#include <allegro5/allegro.h>
#include <allegro5/internal/aintern_list.h>
#include <allegro5/internal/aintern_vector.h>
#include <allegro5/internal/aintern_aatree.h>

#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#define CLAMP(x,y,z) MAX((x), MIN((y), (z)))

#define AATREE _AL_AATREE
#define LIST _AL_LIST
#define VECTOR _AL_VECTOR
#define LIST_ITEM _AL_LIST_ITEM


typedef struct EVENTS {
    void (*move_down) (void *, float dt);
    void (*move_up) (void *, float dt);
    void (*move_left) (void *, float dt);
    void (*move_right) (void *, float dt);
} EVENTS;

void debug(const char *format, ...);

void set_config_i (ALLEGRO_CONFIG *config, const char *section, const char *key, int value);
void get_config_i (ALLEGRO_CONFIG *config, const char *section, const char *key, int *value);

char *strdup (const char *s);

LIST *create_list (size_t capacity);

void vector_shrink (VECTOR *v, size_t size);

int intcmp (const void *a, const void *b);
int charcmp (const void *a, const void *b);

void dtor_string (void *value, void *user_data);

LIST * split (const char *str, const char *delimiters);

#endif
