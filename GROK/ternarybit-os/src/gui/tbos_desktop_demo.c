/**
 * @file tbos_desktop_demo.c
 * @brief TBOS Desktop Environment Demo
 *
 * A complete desktop shell demo showcasing:
 * - Menubar with system tray
 * - Dock with app icons and animations
 * - Window management (drag, resize, minimize, maximize)
 * - Desktop icons
 * - Notifications
 * - Themes (Dark, Light, Saffron)
 *
 * @version 1.0
 * @date 2026-07-16
 */

#include "tbos_pixel.h"
#include "tbos_gui.h"
#include "tbos_desktop.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * DEMO APPS (Window content renderers)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void render_files_app(tbos_window_t* win, tbos_pixbuf_t* buf, void* userdata) {
    (void)userdata;

    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_desktop_theme_t* theme = &desktop->theme;

    int content_y = win->y + TBOS_WINDOW_TITLE_HEIGHT + 8;

    /* Sidebar */
    tbos_draw_rect_fill(buf, win->x + 1, content_y - 8,
                        140, win->height - TBOS_WINDOW_TITLE_HEIGHT - 2,
                        theme->bg_secondary);

    /* Sidebar items */
    const char* sidebar_items[] = {"Favorites", "Home", "Desktop", "Documents", "Downloads"};
    tbos_icon_type_t sidebar_icons[] = {TBOS_ICON_KARMA, TBOS_ICON_HOME, TBOS_ICON_FOLDER,
                                        TBOS_ICON_FOLDER, TBOS_ICON_FOLDER};

    for (int i = 0; i < 5; i++) {
        int iy = content_y + i * 28;
        tbos_icon_draw(buf, win->x + 12, iy, 20, sidebar_icons[i], theme->text_secondary);
        tbos_draw_text(buf, win->x + 40, iy + 2, sidebar_items[i],
                       &tbos_font_8x16, theme->text_primary);
    }

    /* Main content area - file grid */
    int grid_x = win->x + 160;
    int grid_y = content_y;

    /* Toolbar */
    tbos_draw_text(buf, grid_x, grid_y, "Home", &tbos_font_8x16, theme->text_primary);
    grid_y += 32;

    /* Files */
    const char* files[] = {"Documents", "Pictures", "Music", "Videos", "Projects", "STEPPPS"};
    tbos_icon_type_t file_icons[] = {TBOS_ICON_FOLDER, TBOS_ICON_FILE_IMAGE, TBOS_ICON_FILE_AUDIO,
                                     TBOS_ICON_FILE_VIDEO, TBOS_ICON_FOLDER, TBOS_ICON_KARMA};

    int col = 0, row = 0;
    for (int i = 0; i < 6; i++) {
        int fx = grid_x + col * 100;
        int fy = grid_y + row * 100;

        tbos_icon_draw(buf, fx + 18, fy, 64, file_icons[i], theme->accent);

        int label_w = tbos_text_width(files[i], &tbos_font_8x16);
        tbos_draw_text(buf, fx + 50 - label_w/2, fy + 68, files[i],
                       &tbos_font_8x16, theme->text_primary);

        col++;
        if (col >= 3) {
            col = 0;
            row++;
        }
    }
}

static void render_terminal_app(tbos_window_t* win, tbos_pixbuf_t* buf, void* userdata) {
    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_desktop_theme_t* theme = &desktop->theme;

    int content_y = win->y + TBOS_WINDOW_TITLE_HEIGHT;

    /* Black terminal background */
    tbos_rgba_t term_bg = {15, 15, 18, 255};
    tbos_draw_rect_fill(buf, win->x + 1, content_y, win->width - 2,
                        win->height - TBOS_WINDOW_TITLE_HEIGHT - 1, term_bg);

    /* Terminal content */
    tbos_rgba_t green = {0, 255, 128, 255};
    tbos_rgba_t white = {220, 220, 230, 255};
    tbos_rgba_t cyan = theme->accent;

    int ty = content_y + 12;
    int tx = win->x + 12;

    tbos_draw_text(buf, tx, ty, "TernaryBit OS Shell v1.0", &tbos_font_8x16, green);
    ty += 20;
    tbos_draw_text(buf, tx, ty, "Karma: 5000 | Consciousness: Aware", &tbos_font_8x16, cyan);
    ty += 24;

    tbos_draw_text(buf, tx, ty, "tbos@dharma:~$", &tbos_font_8x16, green);
    tbos_draw_text(buf, tx + 120, ty, " ls -la", &tbos_font_8x16, white);
    ty += 20;

    tbos_draw_text(buf, tx, ty, "drwxr-xr-x  user  4096  Jul 16 17:00  .", &tbos_font_8x16, white);
    ty += 16;
    tbos_draw_text(buf, tx, ty, "drwxr-xr-x  user  4096  Jul 16 17:00  ..", &tbos_font_8x16, white);
    ty += 16;
    tbos_draw_text(buf, tx, ty, "-rw-r--r--  user  1234  Jul 16 16:30  README.md", &tbos_font_8x16, white);
    ty += 16;
    tbos_draw_text(buf, tx, ty, "drwxr-xr-x  user  4096  Jul 16 15:00  steppps/", &tbos_font_8x16, cyan);
    ty += 24;

    /* Blinking cursor */
    tbos_draw_text(buf, tx, ty, "tbos@dharma:~$", &tbos_font_8x16, green);
    tbos_draw_rect_fill(buf, tx + 120, ty, 8, 16, green);
}

static void render_settings_app(tbos_window_t* win, tbos_pixbuf_t* buf, void* userdata) {
    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_desktop_theme_t* theme = &desktop->theme;

    int content_y = win->y + TBOS_WINDOW_TITLE_HEIGHT + 16;
    int content_x = win->x + 16;

    /* Settings categories */
    tbos_draw_text(buf, content_x, content_y, "Appearance", &tbos_font_8x16, theme->accent);
    content_y += 32;

    /* Theme selector */
    tbos_draw_text(buf, content_x + 16, content_y, "Theme:", &tbos_font_8x16, theme->text_primary);

    /* Theme buttons */
    const char* themes[] = {"Dark", "Light", "Saffron"};
    tbos_rgba_t theme_colors[] = {
        {30, 30, 36, 255},
        {250, 250, 252, 255},
        {255, 153, 51, 255}
    };

    for (int i = 0; i < 3; i++) {
        int bx = content_x + 100 + i * 90;
        tbos_draw_rect_rounded_fill(buf, bx, content_y - 4, 80, 24, 6, theme_colors[i]);
        tbos_draw_rect_rounded(buf, bx, content_y - 4, 80, 24, 6, theme->border);

        tbos_rgba_t text_color = (i == 1) ? (tbos_rgba_t){0,0,0,255} : theme->text_primary;
        int tw = tbos_text_width(themes[i], &tbos_font_8x16);
        tbos_draw_text(buf, bx + 40 - tw/2, content_y, themes[i], &tbos_font_8x16, text_color);
    }
    content_y += 48;

    /* Dock settings */
    tbos_draw_text(buf, content_x, content_y, "Dock", &tbos_font_8x16, theme->accent);
    content_y += 32;

    tbos_draw_text(buf, content_x + 16, content_y, "Position:", &tbos_font_8x16, theme->text_primary);
    tbos_draw_text(buf, content_x + 120, content_y, "Bottom", &tbos_font_8x16, theme->text_secondary);
    content_y += 24;

    tbos_draw_text(buf, content_x + 16, content_y, "Magnification:", &tbos_font_8x16, theme->text_primary);
    tbos_draw_text(buf, content_x + 160, content_y, "On", &tbos_font_8x16, theme->success);
    content_y += 48;

    /* Karma settings */
    tbos_draw_text(buf, content_x, content_y, "Karma & Consciousness", &tbos_font_8x16, theme->accent);
    content_y += 32;

    char karma_str[64];
    snprintf(karma_str, sizeof(karma_str), "Current Karma: %d / %d", desktop->karma, desktop->max_karma);
    tbos_draw_text(buf, content_x + 16, content_y, karma_str, &tbos_font_8x16, theme->text_primary);
    content_y += 24;

    /* Karma bar */
    int bar_width = 250;
    int bar_height = 12;
    tbos_draw_rect_rounded_fill(buf, content_x + 16, content_y, bar_width, bar_height, 6, theme->bg_tertiary);
    int fill_width = (int)((float)desktop->karma / desktop->max_karma * bar_width);
    tbos_draw_rect_rounded_fill(buf, content_x + 16, content_y, fill_width, bar_height, 6, theme->accent);
}

static void render_monitor_app(tbos_window_t* win, tbos_pixbuf_t* buf, void* userdata) {
    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_desktop_theme_t* theme = &desktop->theme;

    int content_y = win->y + TBOS_WINDOW_TITLE_HEIGHT + 16;
    int content_x = win->x + 16;

    /* CPU Usage */
    tbos_draw_text(buf, content_x, content_y, "System Monitor", &tbos_font_8x16, theme->accent);
    content_y += 32;

    /* Fake CPU meters */
    const char* cpu_labels[] = {"CPU 1:", "CPU 2:", "CPU 3:", "CPU 4:"};
    float cpu_values[] = {0.45f, 0.72f, 0.33f, 0.58f};

    for (int i = 0; i < 4; i++) {
        tbos_draw_text(buf, content_x, content_y, cpu_labels[i], &tbos_font_8x16, theme->text_primary);

        int bar_x = content_x + 60;
        int bar_width = 200;
        tbos_draw_rect_rounded_fill(buf, bar_x, content_y, bar_width, 16, 4, theme->bg_tertiary);

        int fill = (int)(cpu_values[i] * bar_width);
        tbos_rgba_t fill_color = cpu_values[i] > 0.8f ? theme->error :
                                 cpu_values[i] > 0.5f ? theme->warning : theme->success;
        tbos_draw_rect_rounded_fill(buf, bar_x, content_y, fill, 16, 4, fill_color);

        char pct[16];
        snprintf(pct, sizeof(pct), "%d%%", (int)(cpu_values[i] * 100));
        tbos_draw_text(buf, bar_x + bar_width + 8, content_y, pct, &tbos_font_8x16, theme->text_secondary);

        content_y += 24;
    }

    content_y += 16;

    /* Memory */
    tbos_draw_text(buf, content_x, content_y, "Memory:", &tbos_font_8x16, theme->text_primary);
    tbos_draw_text(buf, content_x + 80, content_y, "4.2 GB / 16 GB", &tbos_font_8x16, theme->text_secondary);
    content_y += 24;

    /* Processes */
    tbos_draw_text(buf, content_x, content_y, "Processes:", &tbos_font_8x16, theme->text_primary);
    tbos_draw_text(buf, content_x + 100, content_y, "142", &tbos_font_8x16, theme->text_secondary);
    content_y += 24;

    /* STEPPPS */
    tbos_draw_text(buf, content_x, content_y, "Active STEPPPS:", &tbos_font_8x16, theme->text_primary);
    tbos_draw_text(buf, content_x + 140, content_y, "7 dimensions", &tbos_font_8x16, theme->accent);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DOCK ITEM LAUNCHERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void launch_files(void* userdata) {
    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_window_t* win = tbos_desktop_window_create(desktop, "Files", 100, 80, 500, 400, TBOS_WIN_DEFAULT);
    if (win) {
        win->icon = TBOS_ICON_APP_FILES;
        win->render = render_files_app;
        win->userdata = desktop;
    }
}

static void launch_terminal(void* userdata) {
    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_window_t* win = tbos_desktop_window_create(desktop, "Terminal", 150, 120, 600, 400, TBOS_WIN_DEFAULT);
    if (win) {
        win->icon = TBOS_ICON_APP_TERMINAL;
        win->render = render_terminal_app;
        win->userdata = desktop;
    }
}

static void launch_settings(void* userdata) {
    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_window_t* win = tbos_desktop_window_create(desktop, "Settings", 200, 100, 450, 380, TBOS_WIN_DEFAULT);
    if (win) {
        win->icon = TBOS_ICON_APP_SETTINGS;
        win->render = render_settings_app;
        win->userdata = desktop;
    }
}

static void launch_monitor(void* userdata) {
    tbos_desktop_t* desktop = (tbos_desktop_t*)userdata;
    tbos_window_t* win = tbos_desktop_window_create(desktop, "System Monitor", 250, 140, 400, 320, TBOS_WIN_DEFAULT);
    if (win) {
        win->icon = TBOS_ICON_APP_MONITOR;
        win->render = render_monitor_app;
        win->userdata = desktop;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * STANDALONE DEMO (NO SDL - OUTPUTS TO FILE)
 * ═══════════════════════════════════════════════════════════════════════════ */

#ifndef TBOS_USE_SDL2

static void save_ppm(tbos_pixbuf_t* buf, const char* filename) {
    FILE* f = fopen(filename, "wb");
    if (!f) return;

    fprintf(f, "P6\n%d %d\n255\n", buf->width, buf->height);

    for (uint32_t y = 0; y < buf->height; y++) {
        for (uint32_t x = 0; x < buf->width; x++) {
            tbos_rgba_t c = tbos_pixbuf_get(buf, x, y);
            fputc(c.r, f);
            fputc(c.g, f);
            fputc(c.b, f);
        }
    }

    fclose(f);
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     TernaryBit OS - Desktop Environment Demo                 ║\n");
    printf("║     World-Class GUI with Sacred Geometry                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    /* Create pixel buffer */
    int width = 1920;
    int height = 1080;
    tbos_pixbuf_t* buf = tbos_pixbuf_create(width, height, TBOS_PIXEL_RGBA32);
    if (!buf) {
        fprintf(stderr, "Failed to create pixel buffer\n");
        return 1;
    }

    printf("Created %dx%d display\n", width, height);

    /* Initialize desktop */
    tbos_desktop_t desktop;
    tbos_desktop_init(&desktop, width, height);

    /* Add dock items */
    tbos_desktop_dock_add(&desktop, "Files", TBOS_ICON_APP_FILES, launch_files, &desktop);
    tbos_desktop_dock_add(&desktop, "Terminal", TBOS_ICON_APP_TERMINAL, launch_terminal, &desktop);
    tbos_desktop_dock_add(&desktop, "Settings", TBOS_ICON_APP_SETTINGS, launch_settings, &desktop);
    tbos_desktop_dock_add(&desktop, "Monitor", TBOS_ICON_APP_MONITOR, launch_monitor, &desktop);
    tbos_desktop_dock_add(&desktop, "Browser", TBOS_ICON_APP_BROWSER, NULL, &desktop);
    tbos_desktop_dock_add(&desktop, "Editor", TBOS_ICON_APP_EDITOR, NULL, &desktop);

    /* Mark some as running */
    desktop.dock.items[0].running = true;  /* Files */
    desktop.dock.items[1].running = true;  /* Terminal */
    desktop.dock.items[3].badge_count = 3; /* Monitor has 3 notifications */

    /* Add desktop icons */
    tbos_desktop_icon_add(&desktop, "Home", "/home", TBOS_ICON_HOME);
    tbos_desktop_icon_add(&desktop, "STEPPPS", "/steppps", TBOS_ICON_KARMA);
    tbos_desktop_icon_add(&desktop, "Trash", "/trash", TBOS_ICON_TRASH);

    /* Create some windows */
    launch_files(&desktop);
    launch_terminal(&desktop);

    /* Add notifications */
    tbos_desktop_notify(&desktop, "Welcome to TBOS",
        "Your consciousness level is: Aware", TBOS_NOTIFY_INFO);
    tbos_desktop_notify(&desktop, "Karma Update",
        "+100 karma for mindful computing", TBOS_NOTIFY_SUCCESS);

    /* Simulate some update cycles for animations */
    for (int i = 0; i < 30; i++) {
        tbos_desktop_update(&desktop, 0.016f);  /* 60fps */
    }

    /* Render desktop */
    printf("Rendering desktop...\n");
    tbos_desktop_render(&desktop, buf);

    /* Save outputs */
    const char* output = "tbos_desktop_dark.ppm";
    save_ppm(buf, output);
    printf("Saved: %s (Dark Theme)\n", output);

    /* Switch to light theme and render */
    tbos_desktop_set_theme(&desktop, TBOS_THEME_LIGHT);
    tbos_desktop_render(&desktop, buf);
    save_ppm(buf, "tbos_desktop_light.ppm");
    printf("Saved: tbos_desktop_light.ppm (Light Theme)\n");

    /* Switch to saffron theme and render */
    tbos_desktop_set_theme(&desktop, TBOS_THEME_SAFFRON);
    tbos_desktop_render(&desktop, buf);
    save_ppm(buf, "tbos_desktop_saffron.ppm");
    printf("Saved: tbos_desktop_saffron.ppm (Saffron Theme)\n");

    /* Cleanup */
    tbos_desktop_free(&desktop);
    tbos_pixbuf_free(buf);

    printf("\n✨ Desktop demo complete!\n");
    printf("View with: open tbos_desktop_dark.ppm\n");

    return 0;
}

#else /* TBOS_USE_SDL2 */

/* ═══════════════════════════════════════════════════════════════════════════
 * SDL2 INTERACTIVE DEMO
 * ═══════════════════════════════════════════════════════════════════════════ */

#include "tbos_platform.h"

static tbos_desktop_t g_desktop;

static void demo_init(tbos_gui_t* gui, void* userdata) {
    (void)gui;
    (void)userdata;

    int width = 1280;
    int height = 720;

    tbos_desktop_init(&g_desktop, width, height);

    /* Add dock items */
    tbos_desktop_dock_add(&g_desktop, "Files", TBOS_ICON_APP_FILES, launch_files, &g_desktop);
    tbos_desktop_dock_add(&g_desktop, "Terminal", TBOS_ICON_APP_TERMINAL, launch_terminal, &g_desktop);
    tbos_desktop_dock_add(&g_desktop, "Settings", TBOS_ICON_APP_SETTINGS, launch_settings, &g_desktop);
    tbos_desktop_dock_add(&g_desktop, "Monitor", TBOS_ICON_APP_MONITOR, launch_monitor, &g_desktop);
    tbos_desktop_dock_add(&g_desktop, "Browser", TBOS_ICON_APP_BROWSER, NULL, &g_desktop);
    tbos_desktop_dock_add(&g_desktop, "Editor", TBOS_ICON_APP_EDITOR, NULL, &g_desktop);

    /* Add desktop icons */
    tbos_desktop_icon_add(&g_desktop, "Home", "/home", TBOS_ICON_HOME);
    tbos_desktop_icon_add(&g_desktop, "STEPPPS", "/steppps", TBOS_ICON_KARMA);
    tbos_desktop_icon_add(&g_desktop, "Trash", "/trash", TBOS_ICON_TRASH);

    /* Create initial windows */
    launch_files(&g_desktop);

    /* Welcome notification */
    tbos_desktop_notify(&g_desktop, "Welcome to TBOS",
        "Click dock icons to open apps", TBOS_NOTIFY_INFO);

    printf("Desktop initialized\n");
}

static void demo_update(tbos_gui_t* gui, float dt, void* userdata) {
    (void)userdata;

    /* Update input state */
    tbos_desktop_mouse_move(&g_desktop, gui->mouse.x, gui->mouse.y);

    if (gui->mouse.left_clicked || gui->mouse.left_down != g_desktop.mouse_left) {
        tbos_desktop_mouse_button(&g_desktop, gui->mouse.left_down, gui->mouse.right_down);
    }

    /* Theme switching with number keys */
    if (gui->keyboard.key_pressed) {
        switch (gui->keyboard.key) {
            case '1':
                tbos_desktop_set_theme(&g_desktop, TBOS_THEME_DARK);
                tbos_desktop_notify(&g_desktop, "Theme Changed", "Dark theme activated", TBOS_NOTIFY_INFO);
                break;
            case '2':
                tbos_desktop_set_theme(&g_desktop, TBOS_THEME_LIGHT);
                tbos_desktop_notify(&g_desktop, "Theme Changed", "Light theme activated", TBOS_NOTIFY_INFO);
                break;
            case '3':
                tbos_desktop_set_theme(&g_desktop, TBOS_THEME_SAFFRON);
                tbos_desktop_notify(&g_desktop, "Theme Changed", "Saffron theme activated", TBOS_NOTIFY_INFO);
                break;
            case 'n':
            case 'N':
                tbos_desktop_notify(&g_desktop, "Test Notification",
                    "This is a test notification message", TBOS_NOTIFY_SUCCESS);
                break;
        }
    }

    tbos_desktop_update(&g_desktop, dt);
}

static void demo_render(tbos_gui_t* gui, void* userdata) {
    (void)userdata;
    tbos_desktop_render(&g_desktop, gui->target);
}

static void demo_shutdown(void* userdata) {
    (void)userdata;
    tbos_desktop_free(&g_desktop);
    printf("Desktop shutdown\n");
}

int main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║     TernaryBit OS - Desktop Environment (Interactive)        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Controls:                                                   ║\n");
    printf("║    Click dock icons  - Open apps                             ║\n");
    printf("║    Drag window title - Move window                           ║\n");
    printf("║    Traffic lights    - Close/Min/Max                         ║\n");
    printf("║    1/2/3            - Switch themes (Dark/Light/Saffron)     ║\n");
    printf("║    N                - Send test notification                 ║\n");
    printf("║    ESC              - Exit                                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    tbos_window_config_t config = TBOS_WINDOW_DEFAULT;
    config.title = "TernaryBit OS Desktop";
    config.width = 1280;
    config.height = 720;

    tbos_app_callbacks_t callbacks = {
        .init = demo_init,
        .update = demo_update,
        .render = demo_render,
        .shutdown = demo_shutdown,
        .userdata = NULL
    };

    return tbos_app_run(&config, &callbacks);
}

#endif /* TBOS_USE_SDL2 */
