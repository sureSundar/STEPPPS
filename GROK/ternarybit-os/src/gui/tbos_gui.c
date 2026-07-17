/**
 * @file tbos_gui.c
 * @brief TBOS GUI Widget Implementation
 *
 * @version 1.0
 * @date 2026-07-16
 */

#include "tbos_gui.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * INITIALIZATION
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_gui_init(tbos_gui_t* gui, tbos_pixbuf_t* target) {
    if (!gui || !target) return -1;

    memset(gui, 0, sizeof(tbos_gui_t));
    gui->target = target;
    gui->clip_w = target->width;
    gui->clip_h = target->height;

    tbos_gui_style_default(gui);

    return 0;
}

void tbos_gui_free(tbos_gui_t* gui) {
    if (!gui) return;
    /* Nothing to free currently */
}

void tbos_gui_begin(tbos_gui_t* gui) {
    if (!gui) return;

    gui->cursor_x = gui->style.padding;
    gui->cursor_y = gui->style.padding;
    gui->row_height = 0;
    gui->indent = 0;
    gui->next_id = 1;
}

void tbos_gui_end(tbos_gui_t* gui) {
    if (!gui) return;
    /* Reset per-frame state */
}

void tbos_gui_input_mouse(tbos_gui_t* gui, int x, int y,
                          bool left, bool right, int scroll) {
    if (!gui) return;

    gui->mouse.left_clicked = left && !gui->mouse.left_down;
    gui->mouse.right_clicked = right && !gui->mouse.right_down;
    gui->mouse.x = x;
    gui->mouse.y = y;
    gui->mouse.left_down = left;
    gui->mouse.right_down = right;
    gui->mouse.scroll_delta = scroll;
}

void tbos_gui_input_key(tbos_gui_t* gui, int key, bool down,
                        bool ctrl, bool shift, bool alt) {
    if (!gui) return;

    gui->keyboard.key_pressed = down && !gui->keyboard.key_down;
    gui->keyboard.key = key;
    gui->keyboard.key_down = down;
    gui->keyboard.ctrl = ctrl;
    gui->keyboard.shift = shift;
    gui->keyboard.alt = alt;
}

void tbos_gui_input_text(tbos_gui_t* gui, const char* text) {
    if (!gui || !text) return;

    size_t len = strlen(text);
    if (len >= sizeof(gui->keyboard.text_input)) {
        len = sizeof(gui->keyboard.text_input) - 1;
    }
    memcpy(gui->keyboard.text_input, text, len);
    gui->keyboard.text_input[len] = '\0';
    gui->keyboard.text_len = len;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * STYLES
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_gui_style_default(tbos_gui_t* gui) {
    if (!gui) return;

    gui->style.bg = TBOS_COLOR_BG_DARK;
    gui->style.bg_panel = TBOS_COLOR_BG_PANEL;
    gui->style.bg_hover = (tbos_rgba_t){33, 38, 45, 255};
    gui->style.bg_active = (tbos_rgba_t){48, 54, 61, 255};
    gui->style.border = TBOS_COLOR_BORDER;
    gui->style.text = TBOS_COLOR_TEXT;
    gui->style.text_dim = TBOS_COLOR_TEXT_DIM;
    gui->style.accent = TBOS_COLOR_CYAN;
    gui->style.error = TBOS_COLOR_RED;
    gui->style.success = TBOS_COLOR_GREEN_UI;
    gui->style.warning = TBOS_COLOR_YELLOW;

    gui->style.padding = 8;
    gui->style.spacing = 4;
    gui->style.border_radius = 4;
    gui->style.border_width = 1;

    gui->style.font = &tbos_font_8x16;
    gui->style.font_small = &tbos_font_8x16;  /* Would use smaller font */
    gui->style.font_large = &tbos_font_8x16;  /* Would use larger font */
}

void tbos_gui_style_saffron(tbos_gui_t* gui) {
    if (!gui) return;

    tbos_gui_style_default(gui);

    /* Override with saffron theme */
    gui->style.accent = TBOS_COLOR_SAFFRON;
    gui->style.bg = (tbos_rgba_t){25, 15, 5, 255};
    gui->style.bg_panel = (tbos_rgba_t){40, 25, 10, 255};
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LAYOUT
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_gui_set_pos(tbos_gui_t* gui, int x, int y) {
    if (!gui) return;
    gui->cursor_x = x;
    gui->cursor_y = y;
}

void tbos_gui_newline(tbos_gui_t* gui) {
    if (!gui) return;
    gui->cursor_x = gui->style.padding + gui->indent;
    gui->cursor_y += gui->row_height + gui->style.spacing;
    gui->row_height = 0;
}

void tbos_gui_space(tbos_gui_t* gui, int pixels) {
    if (!gui) return;
    gui->cursor_y += pixels;
}

void tbos_gui_indent(tbos_gui_t* gui) {
    if (!gui) return;
    gui->indent += 16;
    gui->cursor_x += 16;
}

void tbos_gui_unindent(tbos_gui_t* gui) {
    if (!gui) return;
    if (gui->indent >= 16) {
        gui->indent -= 16;
    }
}

void tbos_gui_same_line(tbos_gui_t* gui) {
    if (!gui) return;
    gui->cursor_x += gui->style.spacing;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static tbos_id_t gui_get_id(tbos_gui_t* gui) {
    return gui->next_id++;
}

static bool point_in_rect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

static void advance_cursor(tbos_gui_t* gui, int w, int h) {
    gui->cursor_x += w + gui->style.spacing;
    if (h > (int)gui->row_height) {
        gui->row_height = h;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BASIC WIDGETS
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_gui_label(tbos_gui_t* gui, const char* text) {
    if (!gui || !text) return;

    int h = gui->style.font->height;
    tbos_draw_text(gui->target, gui->cursor_x, gui->cursor_y, text,
                   gui->style.font, gui->style.text);

    int w = tbos_text_width(text, gui->style.font);
    advance_cursor(gui, w, h);
}

void tbos_gui_labelf(tbos_gui_t* gui, const char* fmt, ...) {
    if (!gui || !fmt) return;

    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    tbos_gui_label(gui, buf);
}

bool tbos_gui_button(tbos_gui_t* gui, const char* label) {
    if (!gui || !label) return false;

    int text_w = tbos_text_width(label, gui->style.font);
    int w = text_w + gui->style.padding * 2;
    int h = gui->style.font->height + gui->style.padding * 2;

    return tbos_gui_button_sized(gui, label, w, h);
}

bool tbos_gui_button_sized(tbos_gui_t* gui, const char* label, int w, int h) {
    if (!gui) return false;

    tbos_id_t id = gui_get_id(gui);
    int x = gui->cursor_x;
    int y = gui->cursor_y;

    /* Check hover/active state */
    bool hovered = point_in_rect(gui->mouse.x, gui->mouse.y, x, y, w, h);
    bool clicked = false;

    if (hovered) {
        gui->hot = id;
        if (gui->mouse.left_clicked) {
            gui->active = id;
            clicked = true;
        }
    }

    /* Draw button */
    tbos_rgba_t bg;
    if (gui->active == id && gui->mouse.left_down) {
        bg = gui->style.bg_active;
    } else if (hovered) {
        bg = gui->style.bg_hover;
    } else {
        bg = gui->style.bg_panel;
    }

    tbos_draw_rect_rounded_fill(gui->target, x, y, w, h,
                                 gui->style.border_radius, bg);
    tbos_draw_rect_rounded(gui->target, x, y, w, h,
                           gui->style.border_radius, gui->style.border);

    /* Draw label centered */
    if (label) {
        tbos_draw_text_centered(gui->target, x, y + gui->style.padding, w,
                                label, gui->style.font, gui->style.text);
    }

    advance_cursor(gui, w, h);
    return clicked;
}

bool tbos_gui_checkbox(tbos_gui_t* gui, const char* label, bool* value) {
    if (!gui || !value) return false;

    (void)gui_get_id(gui); /* reserve an id slot for future hot/active tracking */
    int x = gui->cursor_x;
    int y = gui->cursor_y;
    int size = gui->style.font->height;

    /* Check click */
    bool hovered = point_in_rect(gui->mouse.x, gui->mouse.y, x, y, size, size);
    bool changed = false;

    if (hovered && gui->mouse.left_clicked) {
        *value = !*value;
        changed = true;
    }

    /* Draw checkbox */
    tbos_rgba_t bg = *value ? gui->style.accent : gui->style.bg_panel;
    tbos_draw_rect_rounded_fill(gui->target, x, y, size, size,
                                 gui->style.border_radius / 2, bg);
    tbos_draw_rect_rounded(gui->target, x, y, size, size,
                           gui->style.border_radius / 2, gui->style.border);

    /* Checkmark */
    if (*value) {
        int cx = x + size / 2;
        int cy = y + size / 2;
        tbos_draw_line(gui->target, cx - 4, cy, cx - 1, cy + 3, gui->style.text);
        tbos_draw_line(gui->target, cx - 1, cy + 3, cx + 4, cy - 3, gui->style.text);
    }

    /* Label */
    if (label) {
        tbos_draw_text(gui->target, x + size + gui->style.spacing, y,
                       label, gui->style.font, gui->style.text);
    }

    int w = size + (label ? tbos_text_width(label, gui->style.font) + gui->style.spacing : 0);
    advance_cursor(gui, w, size);

    return changed;
}

bool tbos_gui_slider(tbos_gui_t* gui, const char* label, float* value,
                     float min, float max) {
    if (!gui || !value) return false;

    tbos_id_t id = gui_get_id(gui);
    int x = gui->cursor_x;
    int y = gui->cursor_y;
    int h = gui->style.font->height;
    int track_w = 150;
    int label_w = label ? tbos_text_width(label, gui->style.font) + gui->style.spacing : 0;

    /* Draw label */
    if (label) {
        tbos_draw_text(gui->target, x, y, label, gui->style.font, gui->style.text);
        x += label_w;
    }

    /* Track */
    int track_h = 4;
    int track_y = y + (h - track_h) / 2;
    tbos_draw_rect_rounded_fill(gui->target, x, track_y, track_w, track_h,
                                 2, gui->style.bg_active);

    /* Handle */
    float ratio = (*value - min) / (max - min);
    if (ratio < 0) ratio = 0;
    if (ratio > 1) ratio = 1;
    int handle_x = x + (int)(ratio * (track_w - 8));
    int handle_w = 8;

    bool hovered = point_in_rect(gui->mouse.x, gui->mouse.y, x, y, track_w, h);
    bool changed = false;

    if (hovered && gui->mouse.left_down) {
        gui->active = id;
        float new_ratio = (float)(gui->mouse.x - x) / (track_w - handle_w);
        if (new_ratio < 0) new_ratio = 0;
        if (new_ratio > 1) new_ratio = 1;
        float new_value = min + new_ratio * (max - min);
        if (new_value != *value) {
            *value = new_value;
            changed = true;
        }
    }

    tbos_draw_rect_rounded_fill(gui->target, handle_x, y, handle_w, h,
                                 4, gui->style.accent);

    advance_cursor(gui, label_w + track_w, h);
    return changed;
}

void tbos_gui_progress(tbos_gui_t* gui, float value, const char* label) {
    if (!gui) return;

    int x = gui->cursor_x;
    int y = gui->cursor_y;
    int w = 200;
    int h = 20;

    if (value < 0) value = 0;
    if (value > 1) value = 1;

    /* Background */
    tbos_draw_rect_rounded_fill(gui->target, x, y, w, h, 4, gui->style.bg_panel);

    /* Fill */
    int fill_w = (int)(value * (w - 4));
    if (fill_w > 0) {
        tbos_rgba_t fill_color = gui->style.accent;
        if (value >= 1.0f) fill_color = gui->style.success;
        tbos_draw_rect_rounded_fill(gui->target, x + 2, y + 2, fill_w, h - 4,
                                     2, fill_color);
    }

    /* Border */
    tbos_draw_rect_rounded(gui->target, x, y, w, h, 4, gui->style.border);

    /* Label */
    if (label) {
        tbos_draw_text_centered(gui->target, x, y + 2, w,
                                label, gui->style.font, gui->style.text);
    }

    advance_cursor(gui, w, h);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CONTAINERS
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_gui_panel_begin(tbos_gui_t* gui, const char* title, int x, int y,
                          int w, int h) {
    if (!gui) return;

    /* Draw panel background */
    tbos_draw_rect_rounded_fill(gui->target, x, y, w, h,
                                 gui->style.border_radius, gui->style.bg_panel);
    tbos_draw_rect_rounded(gui->target, x, y, w, h,
                           gui->style.border_radius, gui->style.border);

    /* Title bar */
    if (title) {
        int title_h = gui->style.font->height + gui->style.padding;
        tbos_draw_rect_fill(gui->target, x + 1, y + 1, w - 2, title_h,
                            gui->style.bg_active);
        tbos_draw_hline(gui->target, x, x + w - 1, y + title_h, gui->style.border);
        tbos_draw_text(gui->target, x + gui->style.padding, y + gui->style.padding / 2,
                       title, gui->style.font, gui->style.accent);

        gui->cursor_y = y + title_h + gui->style.padding;
    } else {
        gui->cursor_y = y + gui->style.padding;
    }

    gui->cursor_x = x + gui->style.padding;

    /* Set clipping (would implement properly with clip stack) */
    gui->clip_x = x;
    gui->clip_y = y;
    gui->clip_w = w;
    gui->clip_h = h;
}

void tbos_gui_panel_end(tbos_gui_t* gui) {
    if (!gui) return;

    /* Reset clipping */
    gui->clip_x = 0;
    gui->clip_y = 0;
    gui->clip_w = gui->target->width;
    gui->clip_h = gui->target->height;
}
