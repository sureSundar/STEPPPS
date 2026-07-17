/**
 * @file tbos_gui.h
 * @brief TBOS GUI Widget System
 *
 * Immediate-mode GUI widgets rendered to PXFS pixel buffers.
 * Inspired by Dear ImGui but designed for TBOS philosophy.
 *
 * @version 1.0
 * @date 2026-07-16
 */

#ifndef TBOS_GUI_H
#define TBOS_GUI_H

#include "tbos_pixel.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * KEY CODES
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Special key codes (above ASCII range) */
#define KEY_UNKNOWN     0
#define KEY_UP          1001
#define KEY_DOWN        1002
#define KEY_LEFT        1003
#define KEY_RIGHT       1004
#define KEY_HOME        1005
#define KEY_END         1006
#define KEY_PAGE_UP     1007
#define KEY_PAGE_DOWN   1008
#define KEY_INSERT      1009
#define KEY_DELETE      1010
#define KEY_BACKSPACE   127
#define KEY_TAB         9
#define KEY_ENTER       10
#define KEY_ESCAPE      27

/* Control key combinations */
#define KEY_CTRL_A      1
#define KEY_CTRL_C      3
#define KEY_CTRL_D      4
#define KEY_CTRL_E      5
#define KEY_CTRL_K      11
#define KEY_CTRL_L      12
#define KEY_CTRL_R      18
#define KEY_CTRL_U      21
#define KEY_CTRL_W      23

/* ═══════════════════════════════════════════════════════════════════════════
 * GUI CONTEXT
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Mouse state
 */
typedef struct {
    int x, y;               /* Current position */
    bool left_down;         /* Left button pressed */
    bool right_down;        /* Right button pressed */
    bool left_clicked;      /* Left button just clicked */
    bool right_clicked;     /* Right button just clicked */
    int scroll_delta;       /* Mouse wheel delta */
} tbos_mouse_t;

/**
 * @brief Keyboard state
 */
typedef struct {
    int key;                /* Current key code */
    bool key_down;          /* Key is pressed */
    bool key_pressed;       /* Key just pressed */
    char text_input[32];    /* Text input buffer */
    size_t text_len;        /* Text input length */
    bool ctrl;              /* Ctrl held */
    bool shift;             /* Shift held */
    bool alt;               /* Alt held */
} tbos_keyboard_t;

/**
 * @brief GUI style/theme
 */
typedef struct {
    /* Colors */
    tbos_rgba_t bg;
    tbos_rgba_t bg_panel;
    tbos_rgba_t bg_hover;
    tbos_rgba_t bg_active;
    tbos_rgba_t border;
    tbos_rgba_t text;
    tbos_rgba_t text_dim;
    tbos_rgba_t accent;
    tbos_rgba_t error;
    tbos_rgba_t success;
    tbos_rgba_t warning;

    /* Sizes */
    int padding;
    int spacing;
    int border_radius;
    int border_width;

    /* Fonts */
    const tbos_font_t* font;
    const tbos_font_t* font_small;
    const tbos_font_t* font_large;
} tbos_style_t;

/**
 * @brief Widget ID (for tracking hot/active state)
 */
typedef uint32_t tbos_id_t;

/**
 * @brief GUI context
 */
typedef struct {
    /* Rendering target */
    tbos_pixbuf_t* target;

    /* Input state */
    tbos_mouse_t mouse;
    tbos_keyboard_t keyboard;

    /* Widget state */
    tbos_id_t hot;          /* Widget under mouse */
    tbos_id_t active;       /* Widget being interacted with */
    tbos_id_t focus;        /* Widget with keyboard focus */

    /* Layout state */
    int cursor_x, cursor_y; /* Current layout position */
    int row_height;         /* Current row height */
    int indent;             /* Current indentation */

    /* Clipping */
    int clip_x, clip_y;
    int clip_w, clip_h;

    /* Style */
    tbos_style_t style;

    /* ID generation */
    tbos_id_t next_id;
} tbos_gui_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * INITIALIZATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize GUI context
 */
int tbos_gui_init(tbos_gui_t* gui, tbos_pixbuf_t* target);

/**
 * @brief Free GUI resources
 */
void tbos_gui_free(tbos_gui_t* gui);

/**
 * @brief Begin new frame
 */
void tbos_gui_begin(tbos_gui_t* gui);

/**
 * @brief End frame
 */
void tbos_gui_end(tbos_gui_t* gui);

/**
 * @brief Update input state
 */
void tbos_gui_input_mouse(tbos_gui_t* gui, int x, int y,
                          bool left, bool right, int scroll);
void tbos_gui_input_key(tbos_gui_t* gui, int key, bool down,
                        bool ctrl, bool shift, bool alt);
void tbos_gui_input_text(tbos_gui_t* gui, const char* text);

/**
 * @brief Set default style (dark dharmic theme)
 */
void tbos_gui_style_default(tbos_gui_t* gui);

/**
 * @brief Set saffron theme
 */
void tbos_gui_style_saffron(tbos_gui_t* gui);

/* ═══════════════════════════════════════════════════════════════════════════
 * LAYOUT
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Set cursor position
 */
void tbos_gui_set_pos(tbos_gui_t* gui, int x, int y);

/**
 * @brief Move to next row
 */
void tbos_gui_newline(tbos_gui_t* gui);

/**
 * @brief Add spacing
 */
void tbos_gui_space(tbos_gui_t* gui, int pixels);

/**
 * @brief Indent
 */
void tbos_gui_indent(tbos_gui_t* gui);
void tbos_gui_unindent(tbos_gui_t* gui);

/**
 * @brief Same line (don't advance to next row)
 */
void tbos_gui_same_line(tbos_gui_t* gui);

/* ═══════════════════════════════════════════════════════════════════════════
 * BASIC WIDGETS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Static text label
 */
void tbos_gui_label(tbos_gui_t* gui, const char* text);

/**
 * @brief Formatted text label
 */
void tbos_gui_labelf(tbos_gui_t* gui, const char* fmt, ...);

/**
 * @brief Clickable button
 * @return true if clicked
 */
bool tbos_gui_button(tbos_gui_t* gui, const char* label);

/**
 * @brief Button with specific size
 */
bool tbos_gui_button_sized(tbos_gui_t* gui, const char* label, int w, int h);

/**
 * @brief Checkbox
 * @return true if value changed
 */
bool tbos_gui_checkbox(tbos_gui_t* gui, const char* label, bool* value);

/**
 * @brief Radio button
 */
bool tbos_gui_radio(tbos_gui_t* gui, const char* label, int* value, int option);

/**
 * @brief Horizontal slider
 * @return true if value changed
 */
bool tbos_gui_slider(tbos_gui_t* gui, const char* label, float* value,
                     float min, float max);

/**
 * @brief Integer slider
 */
bool tbos_gui_slider_int(tbos_gui_t* gui, const char* label, int* value,
                         int min, int max);

/**
 * @brief Progress bar
 */
void tbos_gui_progress(tbos_gui_t* gui, float value, const char* label);

/**
 * @brief Text input
 * @return true if value changed
 */
bool tbos_gui_input(tbos_gui_t* gui, const char* label, char* buf, size_t buf_size);

/**
 * @brief Multi-line text input
 */
bool tbos_gui_textarea(tbos_gui_t* gui, char* buf, size_t buf_size, int height);

/* ═══════════════════════════════════════════════════════════════════════════
 * CONTAINERS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Begin panel (bordered container)
 */
void tbos_gui_panel_begin(tbos_gui_t* gui, const char* title, int x, int y,
                          int w, int h);
void tbos_gui_panel_end(tbos_gui_t* gui);

/**
 * @brief Begin window (draggable)
 */
bool tbos_gui_window_begin(tbos_gui_t* gui, const char* title, int* x, int* y,
                           int w, int h, bool* open);
void tbos_gui_window_end(tbos_gui_t* gui);

/**
 * @brief Begin collapsible section
 */
bool tbos_gui_collapse_begin(tbos_gui_t* gui, const char* label, bool* open);
void tbos_gui_collapse_end(tbos_gui_t* gui);

/**
 * @brief Begin tab bar
 */
void tbos_gui_tabs_begin(tbos_gui_t* gui);
bool tbos_gui_tab(tbos_gui_t* gui, const char* label, int* active, int index);
void tbos_gui_tabs_end(tbos_gui_t* gui);

/**
 * @brief Begin scrollable area
 */
void tbos_gui_scroll_begin(tbos_gui_t* gui, int height, int* scroll_y);
void tbos_gui_scroll_end(tbos_gui_t* gui);

/* ═══════════════════════════════════════════════════════════════════════════
 * TBOS-SPECIFIC WIDGETS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Display karma with meter
 */
void tbos_gui_karma(tbos_gui_t* gui, int karma, int max_karma);

/**
 * @brief Display consciousness level with chakra visualization
 */
void tbos_gui_consciousness(tbos_gui_t* gui, const char* level, int karma);

/**
 * @brief Display STEPPPS dimensions summary
 */
void tbos_gui_steppps(tbos_gui_t* gui, const char* space, const char* time,
                      const char* event, const char* psychology,
                      const char* pixel, const char* prompt, const char* script);

/**
 * @brief File browser with STEPPPS metadata
 */
bool tbos_gui_file_browser(tbos_gui_t* gui, const char* path, char* selected,
                           size_t selected_size);

/**
 * @brief Process list with soul info
 */
void tbos_gui_process_list(tbos_gui_t* gui);

/**
 * @brief Terminal emulator widget
 */
void tbos_gui_terminal(tbos_gui_t* gui, int x, int y, int w, int h);

/**
 * @brief Om animation (sacred loading indicator)
 */
void tbos_gui_om_spinner(tbos_gui_t* gui, int cx, int cy, int size);

/* ═══════════════════════════════════════════════════════════════════════════
 * DIALOGS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Message box
 */
int tbos_gui_msgbox(tbos_gui_t* gui, const char* title, const char* message,
                    const char* buttons);  /* "OK|Cancel" format */

/**
 * @brief Confirmation dialog
 */
bool tbos_gui_confirm(tbos_gui_t* gui, const char* title, const char* message);

/**
 * @brief Input dialog
 */
bool tbos_gui_prompt(tbos_gui_t* gui, const char* title, const char* label,
                     char* buf, size_t buf_size);

/* ═══════════════════════════════════════════════════════════════════════════
 * MENUS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Begin menu bar
 */
void tbos_gui_menubar_begin(tbos_gui_t* gui);
void tbos_gui_menubar_end(tbos_gui_t* gui);

/**
 * @brief Begin menu
 */
bool tbos_gui_menu_begin(tbos_gui_t* gui, const char* label);
void tbos_gui_menu_end(tbos_gui_t* gui);

/**
 * @brief Menu item
 */
bool tbos_gui_menu_item(tbos_gui_t* gui, const char* label, const char* shortcut);

/**
 * @brief Menu separator
 */
void tbos_gui_menu_separator(tbos_gui_t* gui);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_GUI_H */
