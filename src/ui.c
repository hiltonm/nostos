/*
 * See LICENSE for copyright information.
 */

#include "nostos/ui.h"
#include "nostos/utils.h"

#include <assert.h>

#include <allegro5/allegro_ttf.h>


UI* ui_load_file (const char *filename)
{
    ALLEGRO_CONFIG *config = al_load_config_file (filename);

    if (!config)
        return NULL;

    UI *ui = al_malloc (sizeof (UI));

    const char *value = al_get_config_value (config, "", "font");
    ui->font_filename = get_resource_path_str (value);

    get_config_i (config, "", "font_size", &ui->font_size);
    ui->font = al_load_ttf_font (ui->font_filename, ui->font_size, 0);

    int ivalue = ui->font_size;
    ui->dialog = al_malloc (sizeof (UI_DIALOG));
    *ui->dialog = (UI_DIALOG){
        .speaker = NULL,
        .text = NULL,
        .text_lines = NULL,
        .visible = false,
        .speaker_color = al_map_rgba_f (1, 0.7, 0.7, 1),
        .text_color = al_map_rgba_f (1, 1, 1, 1),
    };

    value = al_get_config_value (config, "dialog", "font");
    get_config_i (config, "dialog", "font_size", &ivalue);

    if (!value && ivalue == ui->font_size)
        ui->dialog->font = ui->font;
    else {
        if (value) {
            char *path = get_resource_path_str (value);
            ui->dialog->font = al_load_ttf_font (path, ivalue, 0);
            al_free (path);
        } else {
            ui->dialog->font = al_load_ttf_font (ui->font_filename, ivalue, 0);
        }
    }

    char *path = get_resource_path_str (al_get_config_value (config, "dialog", "image"));
    ui->dialog->image = al_load_bitmap (path);
    al_free (path);
    al_destroy_config (config);

    return ui;
}

void ui_draw (UI *ui, SCREEN *screen)
{
    if (ui->dialog->visible) {
        UI_DIALOG *dialog = ui->dialog;
        int sh = al_get_bitmap_height (dialog->image);
        int dh = screen->height - sh;
        al_draw_scaled_bitmap (dialog->image, 0, 0,
                               al_get_bitmap_width (dialog->image), sh,
                               0, dh,
                               screen->width, sh, 0);
        al_draw_ustr (dialog->font, dialog->speaker_color,
                      60, dh + 20, 0, dialog->speaker);

        LIST_ITEM *item = _al_list_front (dialog->text_lines);
        dh += 20;
        while (item) {
            dh += al_get_font_line_height (dialog->font) + 5;
            const ALLEGRO_USTR *line = _al_list_item_data (item);
            al_draw_justified_ustr (dialog->font, dialog->text_color,
                                    60, screen->width - 60, dh,
                                    40, 0, line);
            item = _al_list_next (dialog->text_lines, item);
        }
    }
}

void ui_show_dialog (UI *ui, const ALLEGRO_USTR *speaker, const ALLEGRO_USTR *text)
{
    if (!text) {
        ui->dialog->visible = false;
        return;
    }

    _al_list_destroy (ui->dialog->text_lines);
    al_ustr_free (ui->dialog->speaker);
    al_ustr_free (ui->dialog->text);
    ui->dialog->speaker = al_ustr_dup (speaker);
    ui->dialog->text = al_ustr_dup (text);
    ui->dialog->text_lines = split_line (ui->dialog->text, 70, ' ');
    ui->dialog->visible = true;
}

void ui_show_dialog_cstr (UI *ui, const char *speaker, const char *text)
{
    ALLEGRO_USTR *u_speaker = al_ustr_new (speaker);
    ALLEGRO_USTR *u_text = al_ustr_new (text);
    ui_show_dialog (ui, u_speaker, u_text);
    al_ustr_free (u_speaker);
    al_ustr_free (u_text);
}

