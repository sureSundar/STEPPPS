/*
 * TBOS v3.0 GUI Implementation - Missing Helper Functions
 * Implements critical GUI functions for theme management and utilities
 */

#include "tbos_gui_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//=============================================================================
// COLOR AND THEME UTILITIES
//=============================================================================

gui_color_t tbos_gui_get_element_color(uint32_t element_number) {
    // Color mapping for chemical elements based on periodic table groups
    gui_color_t colors[] = {
        {255, 100, 100, 255},  // Group 1 - Alkali metals (red)
        {255, 200, 100, 255},  // Group 2 - Alkaline earth (orange)
        {200, 200, 255, 255},  // Transition metals (blue)
        {150, 255, 150, 255},  // Post-transition metals (green)
        {255, 255, 150, 255},  // Metalloids (yellow)
        {200, 150, 255, 255},  // Nonmetals (purple)
        {100, 255, 255, 255},  // Halogens (cyan)
        {180, 180, 180, 255},  // Noble gases (gray)
    };

    // Simplified element group classification
    if (element_number <= 2) return colors[0];  // H, He
    if (element_number <= 10) return colors[5]; // Li-Ne
    if (element_number <= 18) return colors[1]; // Na-Ar
    if (element_number <= 36) return colors[2]; // Transition metals
    if (element_number <= 54) return colors[3]; // Post-transition
    if (element_number <= 86) return colors[2]; // More transition metals

    return colors[7]; // Default for heavy elements
}

void tbos_gui_apply_theme_colors(gui_framework_t* gui, gui_widget_t* widget, gui_theme_t theme) {
    if (!gui || !widget) return;

    gui_color_scheme_t scheme;

    switch (theme) {
        case GUI_THEME_CALCULATOR:
            scheme.primary = (gui_color_t){240, 240, 240, 255};
            scheme.secondary = (gui_color_t){200, 200, 200, 255};
            scheme.background = (gui_color_t){255, 255, 255, 255};
            scheme.text = (gui_color_t){0, 0, 0, 255};
            scheme.border = (gui_color_t){128, 128, 128, 255};
            scheme.accent = (gui_color_t){0, 120, 215, 255};
            break;

        case GUI_THEME_EMBEDDED:
            scheme.primary = (gui_color_t){50, 50, 50, 255};
            scheme.secondary = (gui_color_t){80, 80, 80, 255};
            scheme.background = (gui_color_t){30, 30, 30, 255};
            scheme.text = (gui_color_t){0, 255, 0, 255};
            scheme.border = (gui_color_t){100, 100, 100, 255};
            scheme.accent = (gui_color_t){0, 200, 0, 255};
            break;

        case GUI_THEME_DESKTOP:
            scheme.primary = (gui_color_t){0, 120, 215, 255};
            scheme.secondary = (gui_color_t){70, 130, 180, 255};
            scheme.background = (gui_color_t){240, 240, 240, 255};
            scheme.text = (gui_color_t){0, 0, 0, 255};
            scheme.border = (gui_color_t){128, 128, 128, 255};
            scheme.accent = (gui_color_t){0, 120, 215, 255};
            break;

        case GUI_THEME_MOBILE:
            scheme.primary = (gui_color_t){33, 150, 243, 255};
            scheme.secondary = (gui_color_t){100, 181, 246, 255};
            scheme.background = (gui_color_t){250, 250, 250, 255};
            scheme.text = (gui_color_t){33, 33, 33, 255};
            scheme.border = (gui_color_t){189, 189, 189, 255};
            scheme.accent = (gui_color_t){255, 64, 129, 255};
            break;

        case GUI_THEME_QUANTUM:
            scheme.primary = (gui_color_t){138, 43, 226, 255};
            scheme.secondary = (gui_color_t){75, 0, 130, 255};
            scheme.background = (gui_color_t){10, 10, 40, 255};
            scheme.text = (gui_color_t){255, 255, 255, 255};
            scheme.border = (gui_color_t){138, 43, 226, 255};
            scheme.accent = (gui_color_t){255, 0, 255, 255};
            break;

        case GUI_THEME_VR:
            scheme.primary = (gui_color_t){0, 200, 255, 255};
            scheme.secondary = (gui_color_t){0, 150, 200, 255};
            scheme.background = (gui_color_t){20, 20, 50, 255};
            scheme.text = (gui_color_t){255, 255, 255, 255};
            scheme.border = (gui_color_t){0, 150, 200, 255};
            scheme.accent = (gui_color_t){255, 100, 0, 255};
            break;

        case GUI_THEME_NEURAL:
            scheme.primary = (gui_color_t){255, 100, 150, 255};
            scheme.secondary = (gui_color_t){200, 50, 100, 255};
            scheme.background = (gui_color_t){30, 0, 30, 255};
            scheme.text = (gui_color_t){255, 255, 255, 255};
            scheme.border = (gui_color_t){255, 100, 150, 255};
            scheme.accent = (gui_color_t){255, 200, 0, 255};
            break;

        default: // GUI_THEME_UNIVERSAL
            scheme.primary = (gui_color_t){103, 126, 234, 255};
            scheme.secondary = (gui_color_t){118, 75, 162, 255};
            scheme.background = (gui_color_t){248, 248, 255, 255};
            scheme.text = (gui_color_t){32, 32, 32, 255};
            scheme.border = (gui_color_t){150, 150, 180, 255};
            scheme.accent = (gui_color_t){255, 107, 107, 255};
            break;
    }

    widget->colors = scheme;
}

int tbos_gui_load_default_themes(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    // Load default color schemes for all themes
    for (int i = 0; i < 8 && i < MAX_GUI_THEMES; i++) {
        gui_color_scheme_t* theme = &gui->themes[i];

        // Use a dummy widget to generate theme colors
        gui_widget_t dummy;
        tbos_gui_apply_theme_colors(gui, &dummy, (gui_theme_t)i);
        *theme = dummy.colors;
    }

    gui->theme_count = 8;
    printf("✅ Loaded %d default themes\n", gui->theme_count);

    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// WIDGET TYPE UTILITIES
//=============================================================================

const char* tbos_gui_widget_type_name(gui_widget_type_t type) {
    switch (type) {
        case GUI_WIDGET_WINDOW: return "Window";
        case GUI_WIDGET_BUTTON: return "Button";
        case GUI_WIDGET_LABEL: return "Label";
        case GUI_WIDGET_TEXTBOX: return "TextBox";
        case GUI_WIDGET_LISTBOX: return "ListBox";
        case GUI_WIDGET_PANEL: return "Panel";
        case GUI_WIDGET_MENU: return "Menu";
        case GUI_WIDGET_TOOLBAR: return "Toolbar";
        case GUI_WIDGET_STATUSBAR: return "StatusBar";
        case GUI_WIDGET_CANVAS: return "Canvas";
        case GUI_WIDGET_GRAPH: return "Graph";
        case GUI_WIDGET_3D_VIEW: return "3D View";
        case GUI_WIDGET_QUANTUM_VIZ: return "Quantum Visualizer";
        case GUI_WIDGET_PERSONA_SWITCHER: return "Persona Switcher";
        default: return "Unknown";
    }
}

//=============================================================================
// BUILT-IN APPLICATIONS (STUBS)
//=============================================================================

int tbos_gui_launch_calculator_app(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("🔢 Launching Calculator App for Calculator persona\n");

    uint32_t calc_window = tbos_gui_create_window(gui,
        "TBOS Calculator", GUI_WINDOW_NORMAL,
        (gui_rect_t){{100, 100}, {320, 480}});

    if (calc_window == 0) {
        return TBOS_GUI_ERROR_RENDER_FAILED;
    }

    printf("✅ Calculator app launched\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_embedded_dashboard(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("📊 Launching Embedded Dashboard\n");

    uint32_t dash_window = tbos_gui_create_window(gui,
        "TBOS Embedded Dashboard", GUI_WINDOW_NORMAL,
        (gui_rect_t){{50, 50}, {640, 480}});

    if (dash_window == 0) {
        return TBOS_GUI_ERROR_RENDER_FAILED;
    }

    printf("✅ Embedded dashboard launched\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_file_manager(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("📁 Launching File Manager\n");

    uint32_t fm_window = tbos_gui_create_window(gui,
        "TBOS File Manager", GUI_WINDOW_NORMAL,
        (gui_rect_t){{200, 100}, {800, 600}});

    if (fm_window == 0) {
        return TBOS_GUI_ERROR_RENDER_FAILED;
    }

    printf("✅ File manager launched\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_persona_switcher(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("🎭 Launching Persona Switcher\n");

    uint32_t ps_window = tbos_gui_create_window(gui,
        "TBOS Persona Switcher", GUI_WINDOW_NORMAL,
        (gui_rect_t){{300, 200}, {600, 400}});

    if (ps_window == 0) {
        return TBOS_GUI_ERROR_RENDER_FAILED;
    }

    printf("✅ Persona switcher launched\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_system_monitor(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("📈 Launching System Monitor\n");

    uint32_t mon_window = tbos_gui_create_window(gui,
        "TBOS System Monitor", GUI_WINDOW_NORMAL,
        (gui_rect_t){{150, 150}, {700, 500}});

    if (mon_window == 0) {
        return TBOS_GUI_ERROR_RENDER_FAILED;
    }

    printf("✅ System monitor launched\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_settings_panel(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("⚙️ Launching Settings Panel\n");

    uint32_t set_window = tbos_gui_create_window(gui,
        "TBOS Settings", GUI_WINDOW_NORMAL,
        (gui_rect_t){{250, 200}, {500, 600}});

    if (set_window == 0) {
        return TBOS_GUI_ERROR_RENDER_FAILED;
    }

    printf("✅ Settings panel launched\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_mobile_interface(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("📱 Launching Mobile Interface\n");
    printf("✅ Touch-optimized interface active\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_vr_environment(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("🥽 Launching VR Environment\n");

    int result = tbos_gui_enable_vr_mode(gui);
    if (result != TBOS_GUI_SUCCESS) {
        return result;
    }

    printf("✅ VR environment active\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_neural_interface(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("🧠 Launching Neural Interface\n");

    gui->neural_interface_connected = true;

    printf("✅ Neural interface calibrated\n");
    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// WINDOW MANAGEMENT HELPERS
//=============================================================================

int tbos_gui_show_window(gui_framework_t* gui, uint32_t window_id) {
    gui_window_t* window = tbos_gui_get_window(gui, window_id);
    if (!window) return TBOS_GUI_ERROR_WINDOW_NOT_FOUND;

    window->visible = true;
    printf("👁️ Window '%s' shown\n", window->title);
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_hide_window(gui_framework_t* gui, uint32_t window_id) {
    gui_window_t* window = tbos_gui_get_window(gui, window_id);
    if (!window) return TBOS_GUI_ERROR_WINDOW_NOT_FOUND;

    window->visible = false;
    printf("🙈 Window '%s' hidden\n", window->title);
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_focus_window(gui_framework_t* gui, uint32_t window_id) {
    gui_window_t* window = tbos_gui_get_window(gui, window_id);
    if (!window) return TBOS_GUI_ERROR_WINDOW_NOT_FOUND;

    gui->desktop.active_window_id = window_id;
    printf("🎯 Window '%s' focused\n", window->title);
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_destroy_window(gui_framework_t* gui, uint32_t window_id) {
    gui_window_t* window = tbos_gui_get_window(gui, window_id);
    if (!window) return TBOS_GUI_ERROR_WINDOW_NOT_FOUND;

    // Free window resources
    if (window->root_widget) {
        free(window->root_widget);
        window->root_widget = NULL;
    }

    printf("🗑️ Window '%s' destroyed\n", window->title);
    return TBOS_GUI_SUCCESS;
}

gui_window_t* tbos_gui_get_window(gui_framework_t* gui, uint32_t window_id) {
    if (!gui || window_id == 0) return NULL;

    for (uint32_t i = 0; i < gui->desktop.window_count; i++) {
        if (gui->desktop.windows[i].window_id == window_id) {
            return &gui->desktop.windows[i];
        }
    }

    return NULL;
}

//=============================================================================
// WIDGET MANAGEMENT HELPERS
//=============================================================================

uint32_t tbos_gui_create_widget(gui_framework_t* gui, gui_widget_type_t type, gui_widget_t* parent) {
    if (!gui || gui->widget_count >= MAX_GUI_WIDGETS) {
        return 0;
    }

    gui_widget_t* widget = malloc(sizeof(gui_widget_t));
    if (!widget) return 0;

    memset(widget, 0, sizeof(gui_widget_t));
    widget->widget_id = gui->next_widget_id++;
    widget->type = type;
    widget->parent = parent;
    widget->visible = true;
    widget->enabled = true;

    // Register widget
    gui->widget_registry[gui->widget_count] = widget;
    gui->widget_count++;

    return widget->widget_id;
}

gui_widget_t* tbos_gui_get_widget(gui_framework_t* gui, uint32_t widget_id) {
    if (!gui || widget_id == 0) return NULL;

    for (uint32_t i = 0; i < gui->widget_count; i++) {
        if (gui->widget_registry[i] && gui->widget_registry[i]->widget_id == widget_id) {
            return gui->widget_registry[i];
        }
    }

    return NULL;
}

int tbos_gui_destroy_widget(gui_framework_t* gui, uint32_t widget_id) {
    gui_widget_t* widget = tbos_gui_get_widget(gui, widget_id);
    if (!widget) return TBOS_GUI_ERROR_WIDGET_NOT_FOUND;

    // Remove from registry
    for (uint32_t i = 0; i < gui->widget_count; i++) {
        if (gui->widget_registry[i] && gui->widget_registry[i]->widget_id == widget_id) {
            free(gui->widget_registry[i]);
            gui->widget_registry[i] = NULL;
            return TBOS_GUI_SUCCESS;
        }
    }

    return TBOS_GUI_ERROR_WIDGET_NOT_FOUND;
}

int tbos_gui_set_widget_bounds(gui_framework_t* gui, uint32_t widget_id, gui_rect_t bounds) {
    gui_widget_t* widget = tbos_gui_get_widget(gui, widget_id);
    if (!widget) return TBOS_GUI_ERROR_WIDGET_NOT_FOUND;

    widget->bounds = bounds;
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_set_widget_text(gui_framework_t* gui, uint32_t widget_id, const char* text) {
    gui_widget_t* widget = tbos_gui_get_widget(gui, widget_id);
    if (!widget || !text) return TBOS_GUI_ERROR_WIDGET_NOT_FOUND;

    strncpy(widget->text, text, sizeof(widget->text) - 1);
    widget->text[sizeof(widget->text) - 1] = '\0';
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_set_widget_color(gui_framework_t* gui, uint32_t widget_id, gui_color_scheme_t colors) {
    gui_widget_t* widget = tbos_gui_get_widget(gui, widget_id);
    if (!widget) return TBOS_GUI_ERROR_WIDGET_NOT_FOUND;

    widget->colors = colors;
    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// VR AND NEURAL INTERFACE STUBS
//=============================================================================

int tbos_gui_disable_vr_mode(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    gui->vr_environment_active = false;
    printf("🥽 VR mode disabled\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_vr_teleport_user(gui_framework_t* gui, gui_point_t destination) {
    if (!gui || !gui->vr_environment_active) {
        return TBOS_GUI_ERROR_VR_NOT_AVAILABLE;
    }

    printf("🚀 VR teleport to (%d, %d)\n", destination.x, destination.y);
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_connect_neural_interface(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("🧠 Connecting neural interface...\n");
    gui->neural_interface_connected = true;
    printf("✅ Neural interface connected\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_disconnect_neural_interface(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    gui->neural_interface_connected = false;
    printf("🧠 Neural interface disconnected\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_calibrate_neural_interface(gui_framework_t* gui) {
    if (!gui || !gui->neural_interface_connected) {
        return TBOS_GUI_ERROR_NEURAL_FAILED;
    }

    printf("🧠 Calibrating neural interface...\n");
    printf("✅ Neural interface calibrated\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_process_neural_input(gui_framework_t* gui, float neural_patterns[], uint32_t pattern_count) {
    if (!gui || !neural_patterns) return TBOS_GUI_ERROR_INVALID_PARAM;

    // Stub: Process neural patterns
    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// EVENT HANDLING STUBS
//=============================================================================

int tbos_gui_handle_mouse_event(gui_framework_t* gui, gui_point_t position, uint32_t buttons) {
    // Stub: Handle mouse events
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_handle_keyboard_event(gui_framework_t* gui, uint32_t key, bool pressed) {
    // Stub: Handle keyboard events
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_handle_touch_event(gui_framework_t* gui, gui_point_t touches[], uint32_t touch_count) {
    // Stub: Handle touch events
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_handle_vr_event(gui_framework_t* gui, void* vr_data) {
    // Stub: Handle VR events
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_handle_neural_event(gui_framework_t* gui, float neural_signals[], uint32_t signal_count) {
    // Stub: Handle neural events
    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// RENDERING AND UPDATE STUBS
//=============================================================================

int tbos_gui_update(gui_framework_t* gui) {
    if (!gui || !gui->initialized) {
        return TBOS_GUI_ERROR_NOT_INITIALIZED;
    }

    // Update all widgets
    // Stub implementation
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_render(gui_framework_t* gui) {
    if (!gui || !gui->initialized) {
        return TBOS_GUI_ERROR_NOT_INITIALIZED;
    }

    // Render frame
    gui->frames_rendered++;

    return TBOS_GUI_SUCCESS;
}

int tbos_gui_cleanup(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    // Cleanup resources
    for (uint32_t i = 0; i < gui->widget_count; i++) {
        if (gui->widget_registry[i]) {
            free(gui->widget_registry[i]);
            gui->widget_registry[i] = NULL;
        }
    }

    printf("🧹 GUI framework cleaned up\n");
    return TBOS_GUI_SUCCESS;
}

int tbos_gui_stop(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    printf("⏹️ Stopping GUI framework\n");
    gui->initialized = false;
    return TBOS_GUI_SUCCESS;
}
