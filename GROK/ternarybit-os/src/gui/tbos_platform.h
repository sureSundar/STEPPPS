/**
 * @file tbos_platform.h
 * @brief TBOS Platform Adapters for GUI
 *
 * Abstracts the display/input backend:
 * - SDL2 for desktop (macOS, Linux, Windows)
 * - Framebuffer for embedded Linux
 * - Terminal for ASCII fallback
 * - Future: Metal, Vulkan, Direct3D
 *
 * @version 1.0
 * @date 2026-07-16
 */

#ifndef TBOS_PLATFORM_H
#define TBOS_PLATFORM_H

#include "tbos_pixel.h"
#include "tbos_gui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * PLATFORM TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_PLATFORM_AUTO,       /* Auto-detect best platform */
    TBOS_PLATFORM_SDL2,       /* SDL2 (desktop) */
    TBOS_PLATFORM_FRAMEBUFFER,/* Linux framebuffer */
    TBOS_PLATFORM_TERMINAL,   /* ASCII/ANSI terminal */
    TBOS_PLATFORM_HEADLESS,   /* No display (testing) */
} tbos_platform_type_t;

/**
 * @brief Window configuration
 */
typedef struct {
    const char* title;        /* Window title */
    int width;                /* Window width */
    int height;               /* Window height */
    bool fullscreen;          /* Fullscreen mode */
    bool resizable;           /* Allow resize */
    bool borderless;          /* No window decorations */
    int scale;                /* Pixel scale (1, 2, 3...) */
    tbos_platform_type_t type;/* Platform type */
} tbos_window_config_t;

/**
 * @brief Platform context (opaque)
 */
typedef struct tbos_platform tbos_platform_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * LIFECYCLE
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Initialize platform
 */
tbos_platform_t* tbos_platform_init(const tbos_window_config_t* config);

/**
 * @brief Shutdown platform
 */
void tbos_platform_shutdown(tbos_platform_t* platform);

/**
 * @brief Get pixel buffer for rendering
 */
tbos_pixbuf_t* tbos_platform_get_buffer(tbos_platform_t* platform);

/**
 * @brief Present rendered frame to screen
 */
void tbos_platform_present(tbos_platform_t* platform);

/**
 * @brief Check if window should close
 */
bool tbos_platform_should_close(tbos_platform_t* platform);

/* ═══════════════════════════════════════════════════════════════════════════
 * INPUT
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Poll events and update input state
 */
void tbos_platform_poll_events(tbos_platform_t* platform);

/**
 * @brief Get mouse state
 */
void tbos_platform_get_mouse(tbos_platform_t* platform, tbos_mouse_t* mouse);

/**
 * @brief Get keyboard state
 */
void tbos_platform_get_keyboard(tbos_platform_t* platform, tbos_keyboard_t* keyboard);

/**
 * @brief Update GUI context with platform input
 */
void tbos_platform_update_gui(tbos_platform_t* platform, tbos_gui_t* gui);

/* ═══════════════════════════════════════════════════════════════════════════
 * WINDOW CONTROL
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Set window title
 */
void tbos_platform_set_title(tbos_platform_t* platform, const char* title);

/**
 * @brief Set window size
 */
void tbos_platform_set_size(tbos_platform_t* platform, int width, int height);

/**
 * @brief Get window size
 */
void tbos_platform_get_size(tbos_platform_t* platform, int* width, int* height);

/**
 * @brief Set fullscreen mode
 */
void tbos_platform_set_fullscreen(tbos_platform_t* platform, bool fullscreen);

/**
 * @brief Minimize window
 */
void tbos_platform_minimize(tbos_platform_t* platform);

/**
 * @brief Maximize window
 */
void tbos_platform_maximize(tbos_platform_t* platform);

/* ═══════════════════════════════════════════════════════════════════════════
 * TIMING
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Get time in milliseconds since init
 */
uint64_t tbos_platform_get_time(tbos_platform_t* platform);

/**
 * @brief Sleep for milliseconds
 */
void tbos_platform_sleep(tbos_platform_t* platform, uint32_t ms);

/**
 * @brief Target FPS (limits frame rate)
 */
void tbos_platform_set_target_fps(tbos_platform_t* platform, int fps);

/* ═══════════════════════════════════════════════════════════════════════════
 * CLIPBOARD
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Set clipboard text
 */
void tbos_platform_set_clipboard(tbos_platform_t* platform, const char* text);

/**
 * @brief Get clipboard text (must free result)
 */
char* tbos_platform_get_clipboard(tbos_platform_t* platform);

/* ═══════════════════════════════════════════════════════════════════════════
 * CURSOR
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_CURSOR_ARROW,
    TBOS_CURSOR_IBEAM,
    TBOS_CURSOR_HAND,
    TBOS_CURSOR_RESIZE_H,
    TBOS_CURSOR_RESIZE_V,
    TBOS_CURSOR_RESIZE_DIAG,
    TBOS_CURSOR_MOVE,
    TBOS_CURSOR_WAIT,
    TBOS_CURSOR_HIDDEN,
} tbos_cursor_type_t;

/**
 * @brief Set cursor type
 */
void tbos_platform_set_cursor(tbos_platform_t* platform, tbos_cursor_type_t cursor);

/* ═══════════════════════════════════════════════════════════════════════════
 * HIGH-LEVEL APP LOOP
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Application callbacks
 */
typedef struct {
    void (*init)(tbos_gui_t* gui, void* userdata);
    void (*update)(tbos_gui_t* gui, float dt, void* userdata);
    void (*render)(tbos_gui_t* gui, void* userdata);
    void (*shutdown)(void* userdata);
    void* userdata;
} tbos_app_callbacks_t;

/**
 * @brief Run application main loop
 *
 * This handles:
 * - Platform initialization
 * - Event loop
 * - Frame timing
 * - GUI context management
 */
int tbos_app_run(const tbos_window_config_t* config,
                 const tbos_app_callbacks_t* callbacks);

/* ═══════════════════════════════════════════════════════════════════════════
 * CONVENIENCE MACROS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Default window configuration
 */
#define TBOS_WINDOW_DEFAULT { \
    .title = "TernaryBit OS", \
    .width = 1280, \
    .height = 720, \
    .fullscreen = false, \
    .resizable = true, \
    .borderless = false, \
    .scale = 1, \
    .type = TBOS_PLATFORM_AUTO \
}

/**
 * @brief Embedded configuration (small screen)
 */
#define TBOS_WINDOW_EMBEDDED { \
    .title = "TBOS", \
    .width = 320, \
    .height = 240, \
    .fullscreen = true, \
    .resizable = false, \
    .borderless = true, \
    .scale = 1, \
    .type = TBOS_PLATFORM_FRAMEBUFFER \
}

/**
 * @brief Terminal configuration
 */
#define TBOS_WINDOW_TERMINAL { \
    .title = "TBOS Terminal", \
    .width = 80, \
    .height = 24, \
    .fullscreen = false, \
    .resizable = false, \
    .borderless = false, \
    .scale = 1, \
    .type = TBOS_PLATFORM_TERMINAL \
}

#ifdef __cplusplus
}
#endif

#endif /* TBOS_PLATFORM_H */
