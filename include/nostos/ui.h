/*
 * See LICENSE for copyright information.
 */

#ifndef _ui_h_
#define _ui_h_

#include "box.h"
#include "screen.h"

#include <allegro5/allegro_font.h>

typedef struct UI UI;
typedef struct UI_DIALOG UI_DIALOG;

struct UI {
    char *font_filename;
    int font_size;
    ALLEGRO_FONT *font;
    UI_DIALOG *dialog;
};

struct UI_DIALOG {
    ALLEGRO_FONT *font;
    int font_size;
    ALLEGRO_BITMAP *image;
    ALLEGRO_USTR *speaker;
    ALLEGRO_COLOR speaker_color;
    ALLEGRO_USTR *text;
    LIST *text_lines;
    ALLEGRO_COLOR text_color;
    bool visible;
};

UI* ui_load_file (const char *filename);
void ui_draw (UI *ui, SCREEN *screen);
void ui_show_dialog_cstr (UI *ui, const char *speaker, const char *text);
void ui_show_dialog (UI *ui, const ALLEGRO_USTR *speaker, const ALLEGRO_USTR *text);

#endif
