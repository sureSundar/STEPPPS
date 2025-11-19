/*
 * TBOS v3.0 GUI Framework - Advanced Graphical Shell Layers
 * Provides persona-aware graphical interfaces with VR and neural support
 */

#ifndef TBOS_GUI_FRAMEWORK_H
#define TBOS_GUI_FRAMEWORK_H

#include "tbos_shell_architecture.h"
#include "../../src/core/steppps_framework.h"
#include <stdint.h>
#include <stdbool.h>

//=============================================================================
// GUI COMPONENT DEFINITIONS
//=============================================================================

typedef enum {
    GUI_WIDGET_WINDOW = 0,
    GUI_WIDGET_BUTTON = 1,
    GUI_WIDGET_LABEL = 2,
    GUI_WIDGET_TEXTBOX = 3,
    GUI_WIDGET_LISTBOX = 4,
    GUI_WIDGET_PANEL = 5,
    GUI_WIDGET_MENU = 6,
    GUI_WIDGET_TOOLBAR = 7,
    GUI_WIDGET_STATUSBAR = 8,
    GUI_WIDGET_CANVAS = 9,
    GUI_WIDGET_GRAPH = 10,
    GUI_WIDGET_3D_VIEW = 11,
    GUI_WIDGET_QUANTUM_VIZ = 12,
    GUI_WIDGET_PERSONA_SWITCHER = 13,
    GUI_WIDGET_COUNT = 14
} gui_widget_type_t;

typedef enum {
    GUI_THEME_CALCULATOR = 0,    // Minimal monochrome
    GUI_THEME_EMBEDDED = 1,      // Compact industrial
    GUI_THEME_DESKTOP = 2,       // Classic desktop
    GUI_THEME_MOBILE = 3,        // Touch-optimized
    GUI_THEME_QUANTUM = 4,       // Scientific visualization
    GUI_THEME_VR = 5,           // Virtual reality
    GUI_THEME_NEURAL = 6,        // Brain-computer interface
    GUI_THEME_UNIVERSAL = 7,     // Adaptive theme
    GUI_THEME_COUNT = 8
} gui_theme_t;

typedef enum {
    GUI_RENDER_SOFTWARE = 0,     // Software rendering
    GUI_RENDER_HARDWARE = 1,     // GPU acceleration
    GUI_RENDER_QUANTUM = 2,      // Quantum visualization
    GUI_RENDER_NEURAL = 3,       // Neural interface
    GUI_RENDER_HOLOGRAPHIC = 4,  // Holographic display
    GUI_RENDER_COUNT = 5
} gui_render_mode_t;

//=============================================================================
// GUI GEOMETRY AND GRAPHICS
//=============================================================================

typedef struct {
    int32_t x, y;
} gui_point_t;

typedef struct {
    int32_t width, height;
} gui_size_t;

typedef struct {
    gui_point_t position;
    gui_size_t size;
} gui_rect_t;

typedef struct {
    uint8_t r, g, b, a;
} gui_color_t;

typedef struct {
    gui_color_t primary;
    gui_color_t secondary;
    gui_color_t accent;
    gui_color_t background;
    gui_color_t text;
    gui_color_t border;
    gui_color_t highlight;
    gui_color_t error;
} gui_color_scheme_t;

//=============================================================================
// GUI WIDGET STRUCTURE
//=============================================================================

typedef struct gui_widget gui_widget_t;

typedef struct {
    int (*on_click)(gui_widget_t* widget, gui_point_t point);
    int (*on_key)(gui_widget_t* widget, uint32_t key);
    int (*on_focus)(gui_widget_t* widget, bool focused);
    int (*on_resize)(gui_widget_t* widget, gui_size_t new_size);
    int (*on_update)(gui_widget_t* widget, uint64_t timestamp);
    int (*on_render)(gui_widget_t* widget, void* render_context);
} gui_event_handlers_t;

struct gui_widget {
    // Widget identification
    uint32_t widget_id;
    char name[64];
    gui_widget_type_t type;
    uint32_t persona_mask;  // Which personas support this widget

    // Widget hierarchy
    gui_widget_t* parent;
    gui_widget_t* children[32];
    uint32_t child_count;

    // Widget geometry
    gui_rect_t bounds;
    gui_rect_t client_rect;
    bool visible;
    bool enabled;
    bool focused;

    // Widget appearance
    gui_color_scheme_t colors;
    char font_name[32];
    uint32_t font_size;
    uint32_t style_flags;

    // Widget data
    char text[256];
    void* widget_data;
    size_t data_size;

    // Widget behavior
    gui_event_handlers_t handlers;
    uint32_t update_frequency; // Hz
    uint64_t last_update;

    // Persona-specific properties
    uint32_t quantum_state;     // For ChemOS quantum widgets
    float neural_activity;      // For neural interface widgets
    bool vr_enabled;           // For VR-compatible widgets
};

//=============================================================================
// GUI WINDOW AND DESKTOP
//=============================================================================

typedef enum {
    GUI_WINDOW_NORMAL = 0,
    GUI_WINDOW_MODAL = 1,
    GUI_WINDOW_POPUP = 2,
    GUI_WINDOW_TOOLTIP = 3,
    GUI_WINDOW_SPLASH = 4,
    GUI_WINDOW_OVERLAY = 5,
    GUI_WINDOW_QUANTUM = 6,     // ChemOS quantum workspace
    GUI_WINDOW_VR = 7,          // VR environment
    GUI_WINDOW_COUNT = 8
} gui_window_type_t;

typedef struct {
    uint32_t window_id;
    char title[128];
    gui_window_type_t type;
    gui_rect_t bounds;

    // Window state
    bool visible;
    bool minimized;
    bool maximized;
    bool resizable;
    bool closable;

    // Window content
    gui_widget_t* root_widget;
    gui_widget_t* focused_widget;

    // Window theme
    gui_theme_t theme;
    gui_color_scheme_t color_scheme;

    // Persona-specific properties
    uint8_t target_persona;
    bool quantum_workspace;
    bool vr_enabled;
    float neural_sync_level;

} gui_window_t;

typedef struct {
    // Desktop properties
    gui_size_t screen_size;
    gui_color_t background_color;
    char wallpaper_path[512];
    gui_theme_t current_theme;

    // Desktop widgets
    gui_widget_t taskbar;
    gui_widget_t desktop_icons[64];
    uint32_t icon_count;

    // Window management
    gui_window_t windows[32];
    uint32_t window_count;
    uint32_t active_window_id;

    // Persona adaptation
    uint8_t current_persona;
    gui_render_mode_t render_mode;
    bool quantum_overlay_active;
    bool vr_mode_active;
    bool neural_interface_active;

} gui_desktop_t;

//=============================================================================
// GUI FRAMEWORK MANAGER
//=============================================================================

#define MAX_GUI_SESSIONS 16
#define MAX_GUI_THEMES 16
#define MAX_GUI_WIDGETS 1024

typedef struct {
    // Framework state
    bool initialized;
    gui_render_mode_t render_mode;
    uint8_t current_persona;

    // STEPPPS / PIXEL integration
    steppps_manager_t* steppps;
    pixel_manager_t* pixel;
    render_context_t pixel_context;
    display_mode_t pixel_display_mode;
    bool pixel_dimension_active;
    uint64_t pixel_last_sync_us;
    uint64_t pixel_last_render_us;

    // Desktop management
    gui_desktop_t desktop;

    // Session management
    uint32_t active_sessions[MAX_GUI_SESSIONS];
    uint32_t session_count;

    // Theme management
    gui_color_scheme_t themes[MAX_GUI_THEMES];
    uint32_t theme_count;

    // Widget registry
    gui_widget_t* widget_registry[MAX_GUI_WIDGETS];
    uint32_t widget_count;
    uint32_t next_widget_id;

    // Persona-specific configurations
    gui_size_t persona_resolutions[8];
    gui_theme_t persona_themes[8];
    bool persona_quantum_support[8];
    bool persona_vr_support[8];

    // Performance metrics
    uint64_t frames_rendered;
    uint64_t total_render_time_us;
    uint32_t current_fps;

    // Advanced features
    bool quantum_visualization_enabled;
    bool vr_environment_active;
    bool neural_interface_connected;
    bool holographic_display_available;

} gui_framework_t;

//=============================================================================
// GUI FRAMEWORK API
//=============================================================================

// Framework management
int tbos_gui_init(gui_framework_t* gui);
int tbos_gui_cleanup(gui_framework_t* gui);
int tbos_gui_start(gui_framework_t* gui);
int tbos_gui_stop(gui_framework_t* gui);
int tbos_gui_update(gui_framework_t* gui);
int tbos_gui_render(gui_framework_t* gui);
int tbos_gui_attach_steppps(gui_framework_t* gui, steppps_manager_t* steppps);
int tbos_gui_use_pixel_manager(gui_framework_t* gui, pixel_manager_t* pixel);
bool tbos_gui_has_pixel_dimension(const gui_framework_t* gui);

// Desktop management
int tbos_gui_init_desktop(gui_framework_t* gui, uint8_t persona);
int tbos_gui_switch_desktop_persona(gui_framework_t* gui, uint8_t persona);
int tbos_gui_set_wallpaper(gui_framework_t* gui, const char* wallpaper_path);
int tbos_gui_set_theme(gui_framework_t* gui, gui_theme_t theme);

// Window management
uint32_t tbos_gui_create_window(gui_framework_t* gui, const char* title, gui_window_type_t type, gui_rect_t bounds);
int tbos_gui_destroy_window(gui_framework_t* gui, uint32_t window_id);
int tbos_gui_show_window(gui_framework_t* gui, uint32_t window_id);
int tbos_gui_hide_window(gui_framework_t* gui, uint32_t window_id);
int tbos_gui_focus_window(gui_framework_t* gui, uint32_t window_id);
gui_window_t* tbos_gui_get_window(gui_framework_t* gui, uint32_t window_id);

// Widget management
uint32_t tbos_gui_create_widget(gui_framework_t* gui, gui_widget_type_t type, gui_widget_t* parent);
int tbos_gui_destroy_widget(gui_framework_t* gui, uint32_t widget_id);
int tbos_gui_set_widget_bounds(gui_framework_t* gui, uint32_t widget_id, gui_rect_t bounds);
int tbos_gui_set_widget_text(gui_framework_t* gui, uint32_t widget_id, const char* text);
int tbos_gui_set_widget_color(gui_framework_t* gui, uint32_t widget_id, gui_color_scheme_t colors);
gui_widget_t* tbos_gui_get_widget(gui_framework_t* gui, uint32_t widget_id);

// Event handling
int tbos_gui_handle_mouse_event(gui_framework_t* gui, gui_point_t position, uint32_t buttons);
int tbos_gui_handle_keyboard_event(gui_framework_t* gui, uint32_t key, bool pressed);
int tbos_gui_handle_touch_event(gui_framework_t* gui, gui_point_t touches[], uint32_t touch_count);
int tbos_gui_handle_vr_event(gui_framework_t* gui, void* vr_data);
int tbos_gui_handle_neural_event(gui_framework_t* gui, float neural_signals[], uint32_t signal_count);

// Persona integration
int tbos_gui_configure_persona(gui_framework_t* gui, uint8_t persona);
int tbos_gui_switch_persona(gui_framework_t* gui, uint8_t new_persona);
int tbos_gui_get_persona_capabilities(gui_framework_t* gui, uint8_t persona, uint32_t* capabilities);

// Quantum features (ChemOS)
int tbos_gui_enable_quantum_visualization(gui_framework_t* gui);
int tbos_gui_create_quantum_widget(gui_framework_t* gui, uint32_t element_number);
int tbos_gui_update_quantum_state(gui_framework_t* gui, uint32_t widget_id, uint32_t quantum_state);
int tbos_gui_visualize_quantum_coherence(gui_framework_t* gui, float coherence_level);

// VR interface
int tbos_gui_enable_vr_mode(gui_framework_t* gui);
int tbos_gui_disable_vr_mode(gui_framework_t* gui);
int tbos_gui_create_vr_environment(gui_framework_t* gui, const char* environment_name);
int tbos_gui_vr_teleport_user(gui_framework_t* gui, gui_point_t destination);

// Neural interface
int tbos_gui_connect_neural_interface(gui_framework_t* gui);
int tbos_gui_disconnect_neural_interface(gui_framework_t* gui);
int tbos_gui_calibrate_neural_interface(gui_framework_t* gui);
int tbos_gui_process_neural_input(gui_framework_t* gui, float neural_patterns[], uint32_t pattern_count);

// Theme and appearance
int tbos_gui_load_default_themes(gui_framework_t* gui);
int tbos_gui_load_theme(gui_framework_t* gui, const char* theme_path);
int tbos_gui_create_custom_theme(gui_framework_t* gui, gui_color_scheme_t colors, const char* theme_name);
int tbos_gui_apply_persona_theme(gui_framework_t* gui, uint8_t persona);
gui_color_t tbos_gui_get_element_color(uint32_t element_number);
void tbos_gui_apply_theme_colors(gui_framework_t* gui, gui_widget_t* widget, gui_theme_t theme);

// Utility functions
const char* tbos_gui_widget_type_name(gui_widget_type_t type);
const char* tbos_gui_theme_name(gui_theme_t theme);
const char* tbos_gui_render_mode_name(gui_render_mode_t mode);
void tbos_gui_print_status(gui_framework_t* gui);
void tbos_gui_print_widget_tree(gui_widget_t* widget, int depth);

// Error codes
#define TBOS_GUI_SUCCESS                  0
#define TBOS_GUI_ERROR_INVALID_PARAM     -1
#define TBOS_GUI_ERROR_NOT_INITIALIZED   -2
#define TBOS_GUI_ERROR_WIDGET_NOT_FOUND  -3
#define TBOS_GUI_ERROR_WINDOW_NOT_FOUND  -4
#define TBOS_GUI_ERROR_THEME_NOT_FOUND   -5
#define TBOS_GUI_ERROR_RENDER_FAILED     -6
#define TBOS_GUI_ERROR_VR_NOT_AVAILABLE  -7
#define TBOS_GUI_ERROR_NEURAL_FAILED     -8
#define TBOS_GUI_ERROR_QUANTUM_REQUIRED  -9

//=============================================================================
// BUILT-IN GUI APPLICATIONS
//=============================================================================

// Persona-specific applications
int tbos_gui_launch_calculator_app(gui_framework_t* gui);
int tbos_gui_launch_embedded_dashboard(gui_framework_t* gui);
int tbos_gui_launch_desktop_environment(gui_framework_t* gui);
int tbos_gui_launch_mobile_interface(gui_framework_t* gui);
int tbos_gui_launch_quantum_workbench(gui_framework_t* gui);
int tbos_gui_launch_vr_environment(gui_framework_t* gui);
int tbos_gui_launch_neural_interface(gui_framework_t* gui);

// System applications
int tbos_gui_launch_persona_switcher(gui_framework_t* gui);
int tbos_gui_launch_system_monitor(gui_framework_t* gui);
int tbos_gui_launch_file_manager(gui_framework_t* gui);
int tbos_gui_launch_settings_panel(gui_framework_t* gui);

#endif // TBOS_GUI_FRAMEWORK_H
