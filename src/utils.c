/*
 * See LICENSE for copyright information.
 */

#include "nostos/utils.h"

#include <stdio.h>

void debug(const char *format, ...)
{
#ifndef NDEBUG
    va_list argptr;
    va_start (argptr, format);
    vprintf (format, argptr);
    printf ("\n");
    fflush (stdout);
    va_end (argptr);
#endif
}

void set_config_i (ALLEGRO_CONFIG *config, const char *section, const char *key, int value)
{
    char val[9];

    sprintf (val, "%d", value);
    al_set_config_value (config, section, key, val);
}

void get_config_i (ALLEGRO_CONFIG *config, const char *section, const char *key, int *value)
{
    const char *val;
    val = al_get_config_value (config, section, key);
    if (val)
        *value = atoi (val);
    else
        set_config_i (config, section, key, *value);
}

char *strdup (const char *s)
{
    if (!s) return NULL;
    char *d = malloc (strlen (s) + 1);
    if (d != NULL)
        strcpy (d,s);
    return d;
}

LIST *create_list (size_t capacity)
{
    if (capacity == 0)
        return _al_list_create ();
    else
        return _al_list_create_static (capacity);
}

void vector_shrink (VECTOR *v, size_t size)
{
    assert (v);
    assert (size <= v->_size);
    v->_size -= size;
    v->_unused += size;
}

int intcmp (const void *a, const void *b)
{
    return *(int*)a - *(int*)b;
}

int charcmp (const void *a, const void *b)
{
    return strcmp ((char *)a, (char *)b);
}

void dtor_string (void *value, void *user_data)
{
    char *str = value;
    free (str);
}

void dtor_ustr (void *value, void *user_data)
{
    ALLEGRO_USTR *ustr = value;
    al_ustr_free (ustr);
}

LIST * split (const char *str, const char *delimiters)
{
    char *cstr = strdup (str);
    LIST *tokens = _al_list_create ();

    char *tok = strtok (cstr, delimiters);
    while (tok) {
        _al_list_push_back_ex (tokens, strdup (tok), dtor_string);
        tok = strtok (NULL, delimiters);
    }

    free (cstr);
    return tokens;
}

LIST * split_line (const ALLEGRO_USTR* ustr, int max_chars, int32_t c)
{
    int length = al_ustr_length (ustr);
    LIST *list = _al_list_create ();
    int start_pos = 0;

    if (length < max_chars) {
        _al_list_push_back_ex (list, al_ustr_dup (ustr), dtor_ustr);
        return list;
    }

    int end_pos = max_chars;

    while (length > end_pos) {
        end_pos = al_ustr_rfind_chr (ustr, al_ustr_offset (ustr, end_pos), c);
        _al_list_push_back_ex (list, al_ustr_dup_substr (ustr,
                                                         al_ustr_offset (ustr, start_pos),
                                                         al_ustr_offset (ustr, end_pos)),
                                                         dtor_ustr);
        start_pos = end_pos + 1;
        end_pos += max_chars;
    }

    if (start_pos < length - 1)
        _al_list_push_back_ex (list, al_ustr_dup_substr (ustr,
                                                         al_ustr_offset (ustr, start_pos),
                                                         al_ustr_offset (ustr, length - 1)),
                                                         dtor_ustr);

    return list;
}

ALLEGRO_PATH* get_resource_path (const char *filename)
{
    ALLEGRO_PATH *respath = al_get_standard_path (ALLEGRO_RESOURCES_PATH);
    ALLEGRO_PATH *path = al_create_path (filename);
    al_rebase_path (respath, path);
    al_destroy_path (respath);
    return path;
}

char* get_resource_path_str (const char *filename)
{
    ALLEGRO_PATH *path = get_resource_path (filename);
    char *path_str = strdup (al_path_cstr (path, ALLEGRO_NATIVE_PATH_SEP));
    al_destroy_path (path);
    debug (path_str);
    return path_str;
}

