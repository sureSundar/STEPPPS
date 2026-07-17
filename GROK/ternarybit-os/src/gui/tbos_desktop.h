/**
 * @file tbos_desktop.h
 * @brief TBOS Desktop Environment
 *
 * A world-class desktop shell with:
 * - Top menu bar with system tray
 * - Dock/taskbar with app launcher
 * - Window management (drag, resize, minimize, maximize)
 * - Desktop icons and wallpaper
 * - Notification center
 * - Spotlight-like search
 * - Workspaces/virtual desktops
 *
 * Design Philosophy:
 * - Clean, modern aesthetics (inspired by macOS + Material Design)
 * - Dharmic visual elements (subtle, not overwhelming)
 * - Smooth animations and transitions
 * - Accessibility-first design
 *
 * @version 1.0
 * @date 2026-07-16
 */

#ifndef TBOS_DESKTOP_H
#define TBOS_DESKTOP_H

#include "tbos_pixel.h"
#include "tbos_gui.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSTANTS
 * ═══════════════════════════════════════════════════════════════════════════ */

#define TBOS_MENUBAR_HEIGHT     28
#define TBOS_DOCK_HEIGHT        64
#define TBOS_DOCK_ICON_SIZE     48
#define TBOS_DOCK_MARGIN        8
#define TBOS_WINDOW_TITLE_HEIGHT 32
#define TBOS_WINDOW_BORDER      1
#define TBOS_WINDOW_SHADOW      8
#define TBOS_WINDOW_RADIUS      10
#define TBOS_DESKTOP_ICON_SIZE  64
#define TBOS_DESKTOP_ICON_SPACING 96
#define TBOS_MAX_WINDOWS        32
#define TBOS_MAX_DOCK_ITEMS     16
#define TBOS_MAX_NOTIFICATIONS  10
#define TBOS_MAX_WORKSPACES     4

/* ═══════════════════════════════════════════════════════════════════════════
 * THEME
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_THEME_DARK,
    TBOS_THEME_LIGHT,
    TBOS_THEME_SAFFRON,
    TBOS_THEME_AUTO,      /* Based on time of day */
} tbos_theme_mode_t;

typedef struct {
    /* Base colors */
    tbos_rgba_t bg_primary;
    tbos_rgba_t bg_secondary;
    tbos_rgba_t bg_tertiary;
    tbos_rgba_t bg_elevated;      /* For floating elements */

    /* Text colors */
    tbos_rgba_t text_primary;
    tbos_rgba_t text_secondary;
    tbos_rgba_t text_disabled;

    /* Accent colors */
    tbos_rgba_t accent;
    tbos_rgba_t accent_hover;
    tbos_rgba_t accent_pressed;

    /* Semantic colors */
    tbos_rgba_t success;
    tbos_rgba_t warning;
    tbos_rgba_t error;
    tbos_rgba_t info;

    /* UI element colors */
    tbos_rgba_t border;
    tbos_rgba_t divider;
    tbos_rgba_t shadow;
    tbos_rgba_t overlay;          /* For modals/dialogs */

    /* Window colors */
    tbos_rgba_t window_bg;
    tbos_rgba_t window_title_bg;
    tbos_rgba_t window_title_text;
    tbos_rgba_t window_border;

    /* Dock/menubar */
    tbos_rgba_t menubar_bg;
    tbos_rgba_t dock_bg;

    /* Special: Glassmorphism */
    uint8_t blur_radius;
    uint8_t glass_opacity;

} tbos_desktop_theme_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * ICONS
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_ICON_NONE = 0,

    /* System icons */
    TBOS_ICON_TBOS,           /* TBOS logo (Om-inspired) */
    TBOS_ICON_FOLDER,
    TBOS_ICON_FILE,
    TBOS_ICON_TERMINAL,
    TBOS_ICON_SETTINGS,
    TBOS_ICON_SEARCH,
    TBOS_ICON_TRASH,
    TBOS_ICON_HOME,

    /* App icons */
    TBOS_ICON_APP_FILES,
    TBOS_ICON_APP_TERMINAL,
    TBOS_ICON_APP_EDITOR,
    TBOS_ICON_APP_SETTINGS,
    TBOS_ICON_APP_MONITOR,
    TBOS_ICON_APP_BROWSER,
    TBOS_ICON_APP_CALENDAR,
    TBOS_ICON_APP_MUSIC,

    /* Status icons */
    TBOS_ICON_WIFI,
    TBOS_ICON_WIFI_OFF,
    TBOS_ICON_BATTERY_FULL,
    TBOS_ICON_BATTERY_LOW,
    TBOS_ICON_BATTERY_CHARGING,
    TBOS_ICON_VOLUME,
    TBOS_ICON_VOLUME_MUTE,
    TBOS_ICON_BRIGHTNESS,
    TBOS_ICON_NOTIFICATION,
    TBOS_ICON_KARMA,

    /* Action icons */
    TBOS_ICON_CLOSE,
    TBOS_ICON_MINIMIZE,
    TBOS_ICON_MAXIMIZE,
    TBOS_ICON_RESTORE,
    TBOS_ICON_MENU,
    TBOS_ICON_BACK,
    TBOS_ICON_FORWARD,
    TBOS_ICON_REFRESH,
    TBOS_ICON_ADD,
    TBOS_ICON_REMOVE,
    TBOS_ICON_EDIT,
    TBOS_ICON_CHECK,
    TBOS_ICON_ARROW_UP,
    TBOS_ICON_ARROW_DOWN,
    TBOS_ICON_ARROW_LEFT,
    TBOS_ICON_ARROW_RIGHT,

    /* File type icons */
    TBOS_ICON_FILE_TEXT,
    TBOS_ICON_FILE_IMAGE,
    TBOS_ICON_FILE_AUDIO,
    TBOS_ICON_FILE_VIDEO,
    TBOS_ICON_FILE_CODE,
    TBOS_ICON_FILE_ARCHIVE,
    TBOS_ICON_FILE_PDF,

    TBOS_ICON_COUNT
} tbos_icon_type_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * WINDOW MANAGEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_WIN_NORMAL,
    TBOS_WIN_MINIMIZED,
    TBOS_WIN_MAXIMIZED,
    TBOS_WIN_FULLSCREEN,
} tbos_window_state_t;

typedef enum {
    TBOS_WIN_NONE = 0,
    TBOS_WIN_RESIZABLE    = (1 << 0),
    TBOS_WIN_CLOSABLE     = (1 << 1),
    TBOS_WIN_MINIMIZABLE  = (1 << 2),
    TBOS_WIN_MAXIMIZABLE  = (1 << 3),
    TBOS_WIN_MODAL        = (1 << 4),
    TBOS_WIN_BORDERLESS   = (1 << 5),
    TBOS_WIN_ALWAYS_ON_TOP = (1 << 6),
    TBOS_WIN_DEFAULT      = TBOS_WIN_RESIZABLE | TBOS_WIN_CLOSABLE |
                            TBOS_WIN_MINIMIZABLE | TBOS_WIN_MAXIMIZABLE,
} tbos_window_flags_t;

typedef struct tbos_window {
    uint32_t id;
    char title[128];
    tbos_icon_type_t icon;

    /* Geometry */
    int x, y;
    int width, height;
    int min_width, min_height;
    int max_width, max_height;

    /* State */
    tbos_window_state_t state;
    tbos_window_flags_t flags;
    bool focused;
    bool visible;
    int z_order;

    /* Restore state (for maximize/minimize) */
    int restore_x, restore_y;
    int restore_w, restore_h;

    /* Content rendering callback */
    void (*render)(struct tbos_window* win, tbos_pixbuf_t* buf, void* userdata);
    void* userdata;

    /* Animation state */
    float anim_progress;
    bool animating;

} tbos_window_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * DOCK
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    char name[64];
    tbos_icon_type_t icon;
    bool running;           /* Has windows open */
    bool pinned;            /* Stays in dock */
    int badge_count;        /* Notification badge */
    tbos_window_t* windows[8];  /* Associated windows */
    int window_count;

    /* Animation */
    float hover_scale;      /* 1.0 to 1.3 for bounce effect */
    float bounce_offset;    /* Bouncing animation */

    /* Launch callback */
    void (*launch)(void* userdata);
    void* userdata;

} tbos_dock_item_t;

typedef enum {
    TBOS_DOCK_BOTTOM,
    TBOS_DOCK_LEFT,
    TBOS_DOCK_RIGHT,
} tbos_dock_position_t;

typedef struct {
    tbos_dock_item_t items[TBOS_MAX_DOCK_ITEMS];
    int count;
    tbos_dock_position_t position;
    bool auto_hide;
    bool magnification;
    float magnification_scale;
    int hovered_index;
    bool visible;
    float show_progress;    /* For auto-hide animation */
} tbos_dock_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * MENU BAR
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct tbos_menu_item {
    char label[64];
    char shortcut[16];
    tbos_icon_type_t icon;
    bool enabled;
    bool checked;
    bool separator;
    struct tbos_menu_item* submenu;
    int submenu_count;
    void (*action)(void* userdata);
    void* userdata;
} tbos_menu_item_t;

typedef struct {
    char label[32];
    tbos_menu_item_t* items;
    int item_count;
    bool open;
} tbos_menu_t;

typedef struct {
    /* Left side: Apple menu, app menus */
    tbos_menu_t menus[8];
    int menu_count;

    /* Right side: System tray */
    struct {
        tbos_icon_type_t icon;
        char tooltip[64];
        void (*click)(void* userdata);
        void* userdata;
    } tray_icons[16];
    int tray_count;

    /* Clock */
    bool show_clock;
    bool show_date;
    bool show_seconds;

    /* Active menu */
    int active_menu;

} tbos_menubar_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * NOTIFICATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    TBOS_NOTIFY_INFO,
    TBOS_NOTIFY_SUCCESS,
    TBOS_NOTIFY_WARNING,
    TBOS_NOTIFY_ERROR,
} tbos_notification_type_t;

typedef struct {
    uint32_t id;
    char title[64];
    char message[256];
    char app_name[32];
    tbos_icon_type_t icon;
    tbos_notification_type_t type;
    uint64_t timestamp;
    uint64_t duration_ms;    /* 0 = persistent */
    bool read;
    bool dismissable;

    /* Actions */
    char action_labels[3][32];
    void (*actions[3])(void* userdata);
    void* userdata;
    int action_count;

    /* Animation */
    float show_progress;
    bool dismissing;

} tbos_notification_t;

typedef struct {
    tbos_notification_t items[TBOS_MAX_NOTIFICATIONS];
    int count;
    bool panel_open;
    int unread_count;
} tbos_notification_center_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * DESKTOP
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    char name[64];
    char path[256];
    tbos_icon_type_t icon;
    int x, y;
    bool selected;
} tbos_desktop_icon_t;

typedef struct {
    /* Display info */
    int screen_width;
    int screen_height;

    /* Theme */
    tbos_theme_mode_t theme_mode;
    tbos_desktop_theme_t theme;

    /* Components */
    tbos_menubar_t menubar;
    tbos_dock_t dock;
    tbos_notification_center_t notifications;

    /* Windows */
    tbos_window_t* windows[TBOS_MAX_WINDOWS];
    int window_count;
    tbos_window_t* focused_window;

    /* Desktop icons */
    tbos_desktop_icon_t icons[64];
    int icon_count;

    /* Wallpaper */
    tbos_pixbuf_t* wallpaper;
    tbos_rgba_t wallpaper_color;

    /* Workspaces */
    int current_workspace;
    int workspace_count;

    /* Spotlight/Search */
    bool spotlight_open;
    char spotlight_query[256];

    /* Mouse state */
    int mouse_x, mouse_y;
    bool mouse_left, mouse_right;
    bool dragging_window;
    bool resizing_window;
    int drag_offset_x, drag_offset_y;
    int resize_edge;  /* 1=top, 2=right, 4=bottom, 8=left */

    /* Time */
    uint64_t current_time;
    float delta_time;

    /* Karma integration */
    int karma;
    int max_karma;

} tbos_desktop_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * DESKTOP API
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Lifecycle */
int tbos_desktop_init(tbos_desktop_t* desktop, int width, int height);
void tbos_desktop_free(tbos_desktop_t* desktop);

/* Theme */
void tbos_desktop_set_theme(tbos_desktop_t* desktop, tbos_theme_mode_t mode);
void tbos_desktop_theme_dark(tbos_desktop_theme_t* theme);
void tbos_desktop_theme_light(tbos_desktop_theme_t* theme);
void tbos_desktop_theme_saffron(tbos_desktop_theme_t* theme);

/* Window management */
tbos_window_t* tbos_desktop_window_create(tbos_desktop_t* desktop,
    const char* title, int x, int y, int width, int height,
    tbos_window_flags_t flags);
void tbos_desktop_window_close(tbos_desktop_t* desktop, tbos_window_t* window);
void tbos_desktop_window_focus(tbos_desktop_t* desktop, tbos_window_t* window);
void tbos_desktop_window_minimize(tbos_desktop_t* desktop, tbos_window_t* window);
void tbos_desktop_window_maximize(tbos_desktop_t* desktop, tbos_window_t* window);
void tbos_desktop_window_restore(tbos_desktop_t* desktop, tbos_window_t* window);

/* Dock */
void tbos_desktop_dock_add(tbos_desktop_t* desktop, const char* name,
    tbos_icon_type_t icon, void (*launch)(void*), void* userdata);
void tbos_desktop_dock_remove(tbos_desktop_t* desktop, int index);
void tbos_desktop_dock_set_badge(tbos_desktop_t* desktop, int index, int count);

/* Notifications */
uint32_t tbos_desktop_notify(tbos_desktop_t* desktop, const char* title,
    const char* message, tbos_notification_type_t type);
void tbos_desktop_notify_dismiss(tbos_desktop_t* desktop, uint32_t id);

/* Desktop icons */
void tbos_desktop_icon_add(tbos_desktop_t* desktop, const char* name,
    const char* path, tbos_icon_type_t icon);

/* Input handling */
void tbos_desktop_mouse_move(tbos_desktop_t* desktop, int x, int y);
void tbos_desktop_mouse_button(tbos_desktop_t* desktop, bool left, bool right);
void tbos_desktop_mouse_scroll(tbos_desktop_t* desktop, int delta);
void tbos_desktop_key_press(tbos_desktop_t* desktop, int key, bool ctrl, bool shift, bool alt);
void tbos_desktop_key_release(tbos_desktop_t* desktop, int key);

/* Update and render */
void tbos_desktop_update(tbos_desktop_t* desktop, float dt);
void tbos_desktop_render(tbos_desktop_t* desktop, tbos_pixbuf_t* target);

/* ═══════════════════════════════════════════════════════════════════════════
 * ICON RENDERING
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_icon_draw(tbos_pixbuf_t* buf, int x, int y, int size,
    tbos_icon_type_t icon, tbos_rgba_t color);

/* ═══════════════════════════════════════════════════════════════════════════
 * BUILT-IN APPS
 * ═══════════════════════════════════════════════════════════════════════════ */

/* File Manager */
tbos_window_t* tbos_app_files_open(tbos_desktop_t* desktop, const char* path);

/* Terminal */
tbos_window_t* tbos_app_terminal_open(tbos_desktop_t* desktop);

/* Settings */
tbos_window_t* tbos_app_settings_open(tbos_desktop_t* desktop);

/* System Monitor */
tbos_window_t* tbos_app_monitor_open(tbos_desktop_t* desktop);

/* Text Editor */
tbos_window_t* tbos_app_editor_open(tbos_desktop_t* desktop, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_DESKTOP_H */
