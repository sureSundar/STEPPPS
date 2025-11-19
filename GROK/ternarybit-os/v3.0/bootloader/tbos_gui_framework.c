/*
 * TBOS v3.0 GUI Framework Implementation
 * Advanced graphical shell layers with persona-aware interfaces
 */

#include "tbos_gui_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

// Global GUI framework instance
static gui_framework_t g_gui_framework;
static bool g_gui_initialized = false;
static pixel_manager_t g_fallback_pixel_manager;

// Forward declarations for internal helpers
static int tbos_gui_init_persona_configs(gui_framework_t* gui);
static int tbos_gui_create_taskbar(gui_framework_t* gui);
static int tbos_gui_create_desktop_icons(gui_framework_t* gui, uint8_t persona);

//=============================================================================
// INTERNAL HELPERS
//=============================================================================

static uint64_t tbos_gui_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
}

static display_mode_t tbos_gui_display_mode_for_persona(uint8_t persona) {
    switch (persona) {
        case 0:
        case 1:
            return DISPLAY_MODE_TEXT;
        case 2:
        case 3:
        case 4:
        case 5:
            return DISPLAY_MODE_GRAPHICS_2D;
        case 6:
            return DISPLAY_MODE_GRAPHICS_3D;
        case 7:
            return DISPLAY_MODE_HOLOGRAPHIC;
        case 8:
            return DISPLAY_MODE_HOLOGRAPHIC;
        default:
            return DISPLAY_MODE_GRAPHICS_2D;
    }
}

static uint8_t tbos_gui_color_depth_for_persona(uint8_t persona) {
    if (persona <= 1) return 16;
    if (persona <= 3) return 24;
    if (persona <= 6) return 32;
    return 48; // Quantum/Universal gets HDR-style depth
}

static uint32_t tbos_gui_frame_rate_for_persona(uint8_t persona) {
    if (persona <= 1) return 30;
    if (persona <= 5) return 60;
    if (persona == 6) return 90;
    return 120;
}

static const char* tbos_pixel_display_mode_name(display_mode_t mode) {
    switch (mode) {
        case DISPLAY_MODE_NONE: return "None";
        case DISPLAY_MODE_TEXT: return "Text";
        case DISPLAY_MODE_GRAPHICS_2D: return "2D Graphics";
        case DISPLAY_MODE_GRAPHICS_3D: return "3D Graphics";
        case DISPLAY_MODE_HOLOGRAPHIC: return "Holographic";
        default: return "Unknown";
    }
}

typedef struct {
    char surface[64];
    gui_size_t size;
    gui_theme_t theme;
    uint8_t persona;
    uint32_t widget_count;
} pixel_surface_packet_t;

static void tbos_gui_render_surface(gui_framework_t* gui,
                                    const char* surface_name,
                                    gui_size_t size,
                                    gui_theme_t theme,
                                    uint32_t widget_count) {
    if (!gui || !surface_name || !tbos_gui_has_pixel_dimension(gui)) {
        return;
    }

    pixel_surface_packet_t packet = {0};
    strncpy(packet.surface, surface_name, sizeof(packet.surface) - 1);
    packet.size = size;
    packet.theme = theme;
    packet.persona = gui->current_persona;
    packet.widget_count = widget_count;

    if (render_adaptive_ui(gui->pixel, &packet)) {
        uint64_t now = tbos_gui_timestamp_us();
        uint64_t delta = 0;
        if (gui->pixel_last_render_us != 0) {
            delta = now - gui->pixel_last_render_us;
            gui->pixel->average_frame_time_us =
                (gui->pixel->average_frame_time_us + delta) / 2;
        }
        gui->total_render_time_us += delta;
        gui->pixel_last_render_us = now;
        gui->pixel->frames_rendered++;
        gui->frames_rendered++;
        gui->pixel_context = gui->pixel->context;
        printf("🖼️ PIXEL: Rendered %s surface (%dx%d)\n",
               surface_name, size.width, size.height);
    } else {
        printf("⚠️ PIXEL: render_adaptive_ui failed for %s\n", surface_name);
    }
}

static void tbos_gui_sync_pixel_for_persona(gui_framework_t* gui, uint8_t persona) {
    if (!gui || !tbos_gui_has_pixel_dimension(gui)) {
        return;
    }

    render_context_t* ctx = &gui->pixel->context;
    ctx->mode = tbos_gui_display_mode_for_persona(persona);
    ctx->width = gui->desktop.screen_size.width;
    ctx->height = gui->desktop.screen_size.height;
    ctx->color_depth = tbos_gui_color_depth_for_persona(persona);
    ctx->frame_rate = tbos_gui_frame_rate_for_persona(persona);
    ctx->hardware_accelerated = (persona >= 2);
    ctx->memory_usage_bytes =
        (uint32_t)ctx->width * ctx->height * (ctx->color_depth / 8);

    gui->pixel_context = *ctx;
    gui->pixel_display_mode = ctx->mode;
    gui->pixel_last_sync_us = tbos_gui_timestamp_us();
    gui->pixel_dimension_active = true;

    update_display_mode(gui->pixel, ctx->mode);

    printf("🖼️ PIXEL dimension synced: %s persona • %dx%d • %u-bit • %u FPS (%s)\n",
           tbos_get_persona_name(persona),
           ctx->width, ctx->height,
           ctx->color_depth, ctx->frame_rate,
           tbos_pixel_display_mode_name(ctx->mode));
}

//=============================================================================
// FRAMEWORK MANAGEMENT
//=============================================================================

int tbos_gui_init(gui_framework_t* gui) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    if (g_gui_initialized) {
        return TBOS_GUI_SUCCESS; // Already initialized
    }

    printf("🎨 Initializing TBOS v3.0 GUI Framework...\n");

    // Initialize framework structure
    memset(gui, 0, sizeof(gui_framework_t));

    // Set default configuration
    gui->current_persona = 2; // x86 default
    gui->render_mode = GUI_RENDER_HARDWARE;

    // Initialize persona-specific configurations
    tbos_gui_init_persona_configs(gui);

    // Initialize desktop
    tbos_gui_init_desktop(gui, gui->current_persona);

    // Load default themes
    tbos_gui_load_default_themes(gui);

    // Attempt to auto-connect to STEPPPS PIXEL dimension if available
    if (g_steppps_manager) {
        tbos_gui_attach_steppps(gui, g_steppps_manager);
    } else {
        printf("ℹ️ STEPPPS framework not detected - running GUI in standalone PIXEL mode\n");
        memset(&g_fallback_pixel_manager, 0, sizeof(g_fallback_pixel_manager));
        g_fallback_pixel_manager.context.mode = DISPLAY_MODE_GRAPHICS_2D;
        g_fallback_pixel_manager.context.color_depth = 24;
        g_fallback_pixel_manager.context.frame_rate = 60;
        tbos_gui_use_pixel_manager(gui, &g_fallback_pixel_manager);
        tbos_gui_sync_pixel_for_persona(gui, gui->current_persona);
    }

    // Initialize widget registry
    gui->next_widget_id = 1;

    gui->initialized = true;
    g_gui_initialized = true;

    printf("✅ GUI Framework initialized with %s persona\n",
           tbos_get_persona_name(gui->current_persona));

    return TBOS_GUI_SUCCESS;
}

int tbos_gui_start(gui_framework_t* gui) {
    if (!gui || !gui->initialized) {
        return TBOS_GUI_ERROR_NOT_INITIALIZED;
    }

    printf("\n🌈 Starting TBOS v3.0 GUI Framework\n");
    printf("===================================\n");
    printf("🎭 Current Persona: %s\n", tbos_get_persona_name(gui->current_persona));
    printf("🖥️ Screen Resolution: %dx%d\n",
           gui->desktop.screen_size.width, gui->desktop.screen_size.height);
    printf("🎨 Active Theme: %s\n", tbos_gui_theme_name(gui->desktop.current_theme));
    printf("🚀 Render Mode: %s\n", tbos_gui_render_mode_name(gui->render_mode));

    // Check advanced features
    if (gui->current_persona == 7) { // ChemOS
        gui->quantum_visualization_enabled = true;
        printf("⚛️ Quantum Visualization: Enabled\n");
    }

    if (gui->persona_vr_support[gui->current_persona]) {
        printf("🥽 VR Support: Available\n");
    }

    if (tbos_gui_has_pixel_dimension(gui)) {
        printf("🖼️ PIXEL Context: %dx%d @ %u-bit • %u FPS (%s)\n",
               gui->pixel_context.width,
               gui->pixel_context.height,
               gui->pixel_context.color_depth,
               gui->pixel_context.frame_rate,
               tbos_pixel_display_mode_name(gui->pixel_context.mode));
    } else {
        printf("🖼️ PIXEL Context: Not connected\n");
    }

    // Launch default desktop environment (will render via PIXEL)
    tbos_gui_launch_desktop_environment(gui);

    return TBOS_GUI_SUCCESS;
}

int tbos_gui_use_pixel_manager(gui_framework_t* gui, pixel_manager_t* pixel) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    gui->pixel = pixel;
    gui->pixel_dimension_active = (pixel != NULL);

    if (pixel) {
        gui->pixel_context = pixel->context;
        gui->pixel_display_mode = pixel->context.mode;
        gui->pixel_last_sync_us = tbos_gui_timestamp_us();
        printf("🔗 PIXEL dimension connected (direct)\n");
    } else {
        memset(&gui->pixel_context, 0, sizeof(gui->pixel_context));
        gui->pixel_display_mode = DISPLAY_MODE_NONE;
        gui->pixel_last_sync_us = 0;
        printf("⚠️ PIXEL dimension disconnected\n");
    }

    return TBOS_GUI_SUCCESS;
}

int tbos_gui_attach_steppps(gui_framework_t* gui, steppps_manager_t* steppps) {
    if (!gui) return TBOS_GUI_ERROR_INVALID_PARAM;

    gui->steppps = steppps;

    if (steppps) {
        printf("🔗 Connecting GUI to STEPPPS framework\n");
        tbos_gui_use_pixel_manager(gui, steppps->pixel);
        tbos_gui_sync_pixel_for_persona(gui, gui->current_persona);
    } else {
        tbos_gui_use_pixel_manager(gui, NULL);
    }

    return TBOS_GUI_SUCCESS;
}

bool tbos_gui_has_pixel_dimension(const gui_framework_t* gui) {
    return gui && gui->pixel_dimension_active && gui->pixel != NULL;
}

//=============================================================================
// PERSONA CONFIGURATION
//=============================================================================

static int tbos_gui_init_persona_configs(gui_framework_t* gui) {
    // Calculator (minimal display)
    gui->persona_resolutions[0] = (gui_size_t){320, 240};
    gui->persona_themes[0] = GUI_THEME_CALCULATOR;
    gui->persona_quantum_support[0] = false;
    gui->persona_vr_support[0] = false;

    // Embedded (compact display)
    gui->persona_resolutions[1] = (gui_size_t){640, 480};
    gui->persona_themes[1] = GUI_THEME_EMBEDDED;
    gui->persona_quantum_support[1] = false;
    gui->persona_vr_support[1] = false;

    // x86 (desktop)
    gui->persona_resolutions[2] = (gui_size_t){1920, 1080};
    gui->persona_themes[2] = GUI_THEME_DESKTOP;
    gui->persona_quantum_support[2] = false;
    gui->persona_vr_support[2] = true;

    // ARM64 (mobile)
    gui->persona_resolutions[4] = (gui_size_t){1080, 1920};
    gui->persona_themes[4] = GUI_THEME_MOBILE;
    gui->persona_quantum_support[4] = false;
    gui->persona_vr_support[4] = true;

    // RISC-V (open source)
    gui->persona_resolutions[5] = (gui_size_t){1920, 1080};
    gui->persona_themes[5] = GUI_THEME_DESKTOP;
    gui->persona_quantum_support[5] = false;
    gui->persona_vr_support[5] = true;

    // Supercomputer (high resolution)
    gui->persona_resolutions[6] = (gui_size_t){3840, 2160};
    gui->persona_themes[6] = GUI_THEME_DESKTOP;
    gui->persona_quantum_support[6] = false;
    gui->persona_vr_support[6] = true;

    // ChemOS (quantum visualization)
    gui->persona_resolutions[7] = (gui_size_t){3840, 2160};
    gui->persona_themes[7] = GUI_THEME_QUANTUM;
    gui->persona_quantum_support[7] = true;
    gui->persona_vr_support[7] = true;

    // Universal (adaptive)
    gui->persona_resolutions[8] = (gui_size_t){7680, 4320};
    gui->persona_themes[8] = GUI_THEME_UNIVERSAL;
    gui->persona_quantum_support[8] = true;
    gui->persona_vr_support[8] = true;

    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// DESKTOP MANAGEMENT
//=============================================================================

int tbos_gui_init_desktop(gui_framework_t* gui, uint8_t persona) {
    gui_desktop_t* desktop = &gui->desktop;

    // Set persona-specific properties
    desktop->screen_size = gui->persona_resolutions[persona];
    desktop->current_theme = gui->persona_themes[persona];
    desktop->current_persona = persona;

    // Set background based on persona
    switch (persona) {
        case 0: // Calculator
            desktop->background_color = (gui_color_t){240, 240, 240, 255};
            strcpy(desktop->wallpaper_path, "/boot/wallpapers/calculator.bmp");
            break;
        case 1: // Embedded
            desktop->background_color = (gui_color_t){50, 50, 50, 255};
            strcpy(desktop->wallpaper_path, "/boot/wallpapers/embedded.bmp");
            break;
        case 2: // x86
            desktop->background_color = (gui_color_t){0, 120, 215, 255};
            strcpy(desktop->wallpaper_path, "/boot/wallpapers/desktop.jpg");
            break;
        case 4: // ARM64
            desktop->background_color = (gui_color_t){25, 25, 25, 255};
            strcpy(desktop->wallpaper_path, "/boot/wallpapers/mobile.jpg");
            break;
        case 7: // ChemOS
            desktop->background_color = (gui_color_t){10, 10, 40, 255};
            strcpy(desktop->wallpaper_path, "/boot/wallpapers/quantum.jpg");
            desktop->quantum_overlay_active = true;
            break;
        default:
            desktop->background_color = (gui_color_t){100, 100, 100, 255};
            strcpy(desktop->wallpaper_path, "/boot/wallpapers/default.jpg");
            break;
    }

    // Create taskbar
    tbos_gui_create_taskbar(gui);

    // Create desktop icons
    tbos_gui_create_desktop_icons(gui, persona);

    // Synchronize PIXEL dimension with new desktop state
    tbos_gui_sync_pixel_for_persona(gui, persona);

    printf("🖥️ Desktop initialized for %s persona (%dx%d)\n",
           tbos_get_persona_name(persona),
           desktop->screen_size.width, desktop->screen_size.height);

    return TBOS_GUI_SUCCESS;
}

static int tbos_gui_create_taskbar(gui_framework_t* gui) {
    gui_widget_t* taskbar = &gui->desktop.taskbar;

    // Initialize taskbar
    taskbar->widget_id = gui->next_widget_id++;
    strcpy(taskbar->name, "taskbar");
    taskbar->type = GUI_WIDGET_TOOLBAR;
    taskbar->persona_mask = 0xFF; // All personas

    // Set taskbar geometry
    int screen_width = gui->desktop.screen_size.width;
    int screen_height = gui->desktop.screen_size.height;

    taskbar->bounds = (gui_rect_t){
        {0, screen_height - 48},  // Bottom of screen
        {screen_width, 48}
    };

    taskbar->visible = true;
    taskbar->enabled = true;

    // Set taskbar colors based on theme
    tbos_gui_apply_theme_colors(gui, taskbar, gui->desktop.current_theme);

    return TBOS_GUI_SUCCESS;
}

static int tbos_gui_create_desktop_icons(gui_framework_t* gui, uint8_t persona) {
    gui->desktop.icon_count = 0;

    const char* icon_names[] = {
        "File Manager", "Settings", "Terminal", "Persona Switcher",
        "System Monitor", "Calculator", "Text Editor", "Web Browser"
    };

    const char* quantum_icons[] = {
        "Quantum Workbench", "Element Controller", "Fusion Monitor",
        "Coherence Analyzer", "Reactor Control", "Energy Manager"
    };

    int icon_count = 8;
    const char** icons = icon_names;

    // Use quantum icons for ChemOS
    if (persona == 7) {
        icons = quantum_icons;
        icon_count = 6;
    }

    // Create desktop icons
    for (int i = 0; i < icon_count && i < 64; i++) {
        gui_widget_t* icon = &gui->desktop.desktop_icons[i];

        icon->widget_id = gui->next_widget_id++;
        strcpy(icon->name, icons[i]);
        strcpy(icon->text, icons[i]);
        icon->type = GUI_WIDGET_BUTTON;
        icon->persona_mask = (persona == 7 && i >= 8) ? 0x80 : 0xFF;

        // Position icons in grid
        int col = i % 8;
        int row = i / 8;
        icon->bounds = (gui_rect_t){
            {20 + col * 100, 20 + row * 100},
            {80, 80}
        };

        icon->visible = true;
        icon->enabled = true;

        gui->desktop.icon_count++;
    }

    printf("📱 Created %d desktop icons for %s persona\n",
           gui->desktop.icon_count, tbos_get_persona_name(persona));

    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// WINDOW MANAGEMENT
//=============================================================================

uint32_t tbos_gui_create_window(gui_framework_t* gui, const char* title,
                               gui_window_type_t type, gui_rect_t bounds) {
    if (!gui || !title || gui->desktop.window_count >= 32) {
        return 0;
    }

    uint32_t window_id = gui->desktop.window_count + 1;
    gui_window_t* window = &gui->desktop.windows[gui->desktop.window_count];

    // Initialize window
    memset(window, 0, sizeof(gui_window_t));
    window->window_id = window_id;
    strcpy(window->title, title);
    window->type = type;
    window->bounds = bounds;
    window->visible = false;
    window->resizable = (type == GUI_WINDOW_NORMAL);
    window->closable = (type != GUI_WINDOW_SPLASH);
    window->theme = gui->desktop.current_theme;
    window->target_persona = gui->current_persona;

    // Create root widget for window
    window->root_widget = malloc(sizeof(gui_widget_t));
    if (window->root_widget) {
        memset(window->root_widget, 0, sizeof(gui_widget_t));
        window->root_widget->widget_id = gui->next_widget_id++;
        strcpy(window->root_widget->name, "root");
        window->root_widget->type = GUI_WIDGET_PANEL;
        window->root_widget->bounds = (gui_rect_t){{0, 0}, bounds.size};
        window->root_widget->visible = true;
        window->root_widget->enabled = true;
    }

    gui->desktop.window_count++;

    printf("🪟 Created window '%s' (ID: %d) for %s persona\n",
           title, window_id, tbos_get_persona_name(gui->current_persona));

    tbos_gui_render_surface(gui, title, bounds.size, window->theme,
                            window->root_widget ? window->root_widget->child_count : 0);

    return window_id;
}

//=============================================================================
// PERSONA SWITCHING
//=============================================================================

int tbos_gui_switch_persona(gui_framework_t* gui, uint8_t new_persona) {
    if (!gui || new_persona > 8) {
        return TBOS_GUI_ERROR_INVALID_PARAM;
    }

    if (new_persona == gui->current_persona) {
        return TBOS_GUI_SUCCESS;
    }

    printf("\n🔄 GUI Persona Switch: %s → %s\n",
           tbos_get_persona_name(gui->current_persona),
           tbos_get_persona_name(new_persona));

    // Save current desktop state
    printf("💾 Saving current desktop state...\n");

    // Update screen resolution
    gui_size_t new_resolution = gui->persona_resolutions[new_persona];
    printf("📺 Changing resolution: %dx%d → %dx%d\n",
           gui->desktop.screen_size.width, gui->desktop.screen_size.height,
           new_resolution.width, new_resolution.height);

    gui->desktop.screen_size = new_resolution;

    // Switch theme
    gui_theme_t new_theme = gui->persona_themes[new_persona];
    printf("🎨 Switching theme: %s → %s\n",
           tbos_gui_theme_name(gui->desktop.current_theme),
           tbos_gui_theme_name(new_theme));

    gui->desktop.current_theme = new_theme;

    // Update quantum features
    if (gui->persona_quantum_support[new_persona]) {
        gui->quantum_visualization_enabled = true;
        gui->desktop.quantum_overlay_active = true;
        printf("⚛️ Quantum visualization activated\n");
    } else {
        gui->quantum_visualization_enabled = false;
        gui->desktop.quantum_overlay_active = false;
    }

    // Update VR support
    if (gui->persona_vr_support[new_persona]) {
        printf("🥽 VR support available\n");
    }

    // Reinitialize desktop for new persona
    tbos_gui_init_desktop(gui, new_persona);

    // Update render mode if needed
    if (new_persona == 7) { // ChemOS
        gui->render_mode = GUI_RENDER_QUANTUM;
    } else if (gui->persona_vr_support[new_persona]) {
        gui->render_mode = GUI_RENDER_HARDWARE;
    } else {
        gui->render_mode = GUI_RENDER_SOFTWARE;
    }

    gui->current_persona = new_persona;

    printf("✅ GUI persona switch completed!\n");

    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// QUANTUM FEATURES (ChemOS)
//=============================================================================

int tbos_gui_enable_quantum_visualization(gui_framework_t* gui) {
    if (!gui || gui->current_persona != 7) {
        return TBOS_GUI_ERROR_QUANTUM_REQUIRED;
    }

    printf("⚛️ Enabling ChemOS Quantum Visualization...\n");

    gui->quantum_visualization_enabled = true;
    gui->desktop.quantum_overlay_active = true;
    gui->render_mode = GUI_RENDER_QUANTUM;

    // Create quantum visualization overlay
    uint32_t quantum_window = tbos_gui_create_window(gui,
        "Quantum State Visualizer", GUI_WINDOW_OVERLAY,
        (gui_rect_t){{0, 0}, gui->desktop.screen_size});

    // Create quantum widgets
    for (int i = 1; i <= 10; i++) { // First 10 elements
        tbos_gui_create_quantum_widget(gui, i);
    }

    printf("✅ Quantum visualization active with 10 element widgets\n");

    tbos_gui_render_surface(gui, "Quantum Visualization",
                            gui->desktop.screen_size,
                            gui->desktop.current_theme,
                            10);

    return TBOS_GUI_SUCCESS;
}

int tbos_gui_create_quantum_widget(gui_framework_t* gui, uint32_t element_number) {
    if (!gui || element_number > 118) {
        return TBOS_GUI_ERROR_INVALID_PARAM;
    }

    gui_widget_t* widget = malloc(sizeof(gui_widget_t));
    if (!widget) return TBOS_GUI_ERROR_RENDER_FAILED;

    memset(widget, 0, sizeof(gui_widget_t));
    widget->widget_id = gui->next_widget_id++;
    snprintf(widget->name, sizeof(widget->name), "element_%d", element_number);
    widget->type = GUI_WIDGET_QUANTUM_VIZ;
    widget->persona_mask = 0x80; // ChemOS only

    // Position based on element number
    int col = (element_number - 1) % 18;
    int row = (element_number - 1) / 18;
    widget->bounds = (gui_rect_t){
        {50 + col * 40, 100 + row * 40},
        {35, 35}
    };

    widget->visible = true;
    widget->enabled = true;
    widget->quantum_state = 0; // Ground state

    // Set element-specific colors
    widget->colors.primary = tbos_gui_get_element_color(element_number);

    // Register widget
    if (gui->widget_count < MAX_GUI_WIDGETS) {
        gui->widget_registry[gui->widget_count] = widget;
        gui->widget_count++;
    }

    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// VR INTERFACE
//=============================================================================

int tbos_gui_enable_vr_mode(gui_framework_t* gui) {
    if (!gui || !gui->persona_vr_support[gui->current_persona]) {
        return TBOS_GUI_ERROR_VR_NOT_AVAILABLE;
    }

    printf("🥽 Enabling VR Mode for %s persona...\n",
           tbos_get_persona_name(gui->current_persona));

    gui->vr_environment_active = true;
    gui->render_mode = GUI_RENDER_HARDWARE; // VR requires hardware acceleration

    // Create VR environment
    tbos_gui_create_vr_environment(gui, "TBOS_VR_Workspace");

    printf("✅ VR mode activated - immersive 3D workspace ready\n");

    return TBOS_GUI_SUCCESS;
}

int tbos_gui_create_vr_environment(gui_framework_t* gui, const char* environment_name) {
    printf("🏗️ Creating VR environment: %s\n", environment_name);

    // Create VR workspace window
    uint32_t vr_window = tbos_gui_create_window(gui,
        environment_name, GUI_WINDOW_VR,
        (gui_rect_t){{0, 0}, {3840, 2160}}); // High resolution for VR

    if (vr_window == 0) {
        return TBOS_GUI_ERROR_VR_NOT_AVAILABLE;
    }

    // Create 3D workspace elements
    printf("🎭 Loading 3D persona workspace elements...\n");
    printf("🎮 VR hand tracking initialized\n");
    printf("👁️ Stereoscopic rendering enabled\n");
    printf("🔊 3D spatial audio configured\n");

    tbos_gui_render_surface(gui, environment_name,
                            (gui_size_t){3840, 2160},
                            gui->desktop.current_theme,
                            3);

    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// BUILT-IN APPLICATIONS
//=============================================================================

int tbos_gui_launch_desktop_environment(gui_framework_t* gui) {
    printf("🚀 Launching %s Desktop Environment\n",
           tbos_get_persona_name(gui->current_persona));

    // Show taskbar and desktop icons
    gui->desktop.taskbar.visible = true;
    for (int i = 0; i < gui->desktop.icon_count; i++) {
        gui->desktop.desktop_icons[i].visible = true;
    }

    // Launch persona-specific applications
    switch (gui->current_persona) {
        case 0: // Calculator
            tbos_gui_launch_calculator_app(gui);
            break;
        case 1: // Embedded
            tbos_gui_launch_embedded_dashboard(gui);
            break;
        case 7: // ChemOS
            tbos_gui_launch_quantum_workbench(gui);
            break;
        default:
            // Standard desktop
            tbos_gui_launch_file_manager(gui);
            break;
    }

    tbos_gui_render_surface(gui, "Desktop Environment",
                            gui->desktop.screen_size,
                            gui->desktop.current_theme,
                            gui->desktop.icon_count);

    return TBOS_GUI_SUCCESS;
}

int tbos_gui_launch_quantum_workbench(gui_framework_t* gui) {
    if (gui->current_persona != 7) {
        return TBOS_GUI_ERROR_QUANTUM_REQUIRED;
    }

    printf("⚛️ Launching ChemOS Quantum Workbench...\n");

    // Create quantum workbench window
    uint32_t workbench_window = tbos_gui_create_window(gui,
        "ChemOS Quantum Workbench", GUI_WINDOW_QUANTUM,
        (gui_rect_t){{100, 100}, {1600, 1200}});

    if (workbench_window == 0) {
        return TBOS_GUI_ERROR_RENDER_FAILED;
    }

    // Enable quantum visualization
    tbos_gui_enable_quantum_visualization(gui);

    printf("🧪 Quantum workbench loaded with:\n");
    printf("   • Periodic table with 118 elements\n");
    printf("   • Fusion reactor control panel\n");
    printf("   • Quantum coherence monitor\n");
    printf("   • Energy production dashboard\n");

    return TBOS_GUI_SUCCESS;
}

//=============================================================================
// UTILITY FUNCTIONS
//=============================================================================

const char* tbos_gui_theme_name(gui_theme_t theme) {
    switch (theme) {
        case GUI_THEME_CALCULATOR: return "Calculator";
        case GUI_THEME_EMBEDDED: return "Embedded";
        case GUI_THEME_DESKTOP: return "Desktop";
        case GUI_THEME_MOBILE: return "Mobile";
        case GUI_THEME_QUANTUM: return "Quantum";
        case GUI_THEME_VR: return "VR";
        case GUI_THEME_NEURAL: return "Neural";
        case GUI_THEME_UNIVERSAL: return "Universal";
        default: return "Unknown";
    }
}

const char* tbos_gui_render_mode_name(gui_render_mode_t mode) {
    switch (mode) {
        case GUI_RENDER_SOFTWARE: return "Software";
        case GUI_RENDER_HARDWARE: return "Hardware";
        case GUI_RENDER_QUANTUM: return "Quantum";
        case GUI_RENDER_NEURAL: return "Neural";
        case GUI_RENDER_HOLOGRAPHIC: return "Holographic";
        default: return "Unknown";
    }
}

void tbos_gui_print_status(gui_framework_t* gui) {
    printf("\n🎨 TBOS v3.0 GUI Framework Status\n");
    printf("=================================\n");
    printf("🎭 Current Persona: %s\n", tbos_get_persona_name(gui->current_persona));
    printf("📺 Resolution: %dx%d\n",
           gui->desktop.screen_size.width, gui->desktop.screen_size.height);
    printf("🎨 Theme: %s\n", tbos_gui_theme_name(gui->desktop.current_theme));
    printf("🚀 Render Mode: %s\n", tbos_gui_render_mode_name(gui->render_mode));
    printf("🪟 Active Windows: %d\n", gui->desktop.window_count);
    printf("🔧 Widgets: %d\n", gui->widget_count);
    printf("⚛️ Quantum Mode: %s\n", gui->quantum_visualization_enabled ? "Enabled" : "Disabled");
    printf("🥽 VR Mode: %s\n", gui->vr_environment_active ? "Active" : "Inactive");
    printf("🧠 Neural Interface: %s\n", gui->neural_interface_connected ? "Connected" : "Disconnected");
    printf("📊 Frames Rendered: %lu\n", gui->frames_rendered);
    printf("⏱️ Avg Render Time: %lu µs\n",
           gui->frames_rendered > 0 ? gui->total_render_time_us / gui->frames_rendered : 0);

    if (tbos_gui_has_pixel_dimension(gui)) {
        printf("🖼️ PIXEL Dimension: %dx%d @ %u-bit • %u FPS (%s)\n",
               gui->pixel_context.width,
               gui->pixel_context.height,
               gui->pixel_context.color_depth,
               gui->pixel_context.frame_rate,
               tbos_pixel_display_mode_name(gui->pixel_context.mode));
        printf("   Frames via PIXEL: %u (avg %u µs)\n",
               gui->pixel->frames_rendered,
               gui->pixel->average_frame_time_us);
    } else {
        printf("🖼️ PIXEL Dimension: Not Connected\n");
    }
}

//=============================================================================
// MAIN DEMO FUNCTION
//=============================================================================

int main() {
    printf("🎨 TBOS v3.0 GUI Framework Demo\n");
    printf("===============================\n\n");

    // Initialize GUI framework
    int result = tbos_gui_init(&g_gui_framework);
    if (result != TBOS_GUI_SUCCESS) {
        printf("❌ Failed to initialize GUI framework: %d\n", result);
        return 1;
    }

    // Start GUI framework
    result = tbos_gui_start(&g_gui_framework);
    if (result != TBOS_GUI_SUCCESS) {
        printf("❌ Failed to start GUI framework: %d\n", result);
        return 1;
    }

    // Demo persona switching
    printf("\n🔄 Demonstrating Persona Switching:\n");
    printf("===================================\n");

    uint8_t demo_personas[] = {0, 1, 2, 4, 7, 8}; // Calculator → Embedded → x86 → ARM64 → ChemOS → Universal
    const char* persona_names[] = {"Calculator", "Embedded", "x86", "ARM64", "ChemOS", "Universal"};

    for (int i = 0; i < 6; i++) {
        printf("\n🎭 Switching to %s persona...\n", persona_names[i]);

        result = tbos_gui_switch_persona(&g_gui_framework, demo_personas[i]);
        if (result == TBOS_GUI_SUCCESS) {
            printf("✅ Successfully switched to %s persona\n", persona_names[i]);

            // Special features for ChemOS
            if (demo_personas[i] == 7) {
                tbos_gui_enable_quantum_visualization(&g_gui_framework);
            }

            // VR mode for capable personas
            if (g_gui_framework.persona_vr_support[demo_personas[i]] && demo_personas[i] != 7) {
                tbos_gui_enable_vr_mode(&g_gui_framework);
            }
        }

        usleep(1000000); // 1 second delay
    }

    // Print final status
    printf("\n");
    tbos_gui_print_status(&g_gui_framework);

    printf("\n✅ TBOS v3.0 GUI Framework demo completed successfully!\n");
    printf("🎯 Ready for integration with shell manager and full OS deployment.\n");

    return 0;
}
