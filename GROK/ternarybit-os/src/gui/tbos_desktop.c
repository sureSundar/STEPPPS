/**
 * @file tbos_desktop.c
 * @brief TBOS Desktop Environment Implementation
 *
 * @version 1.0
 * @date 2026-07-16
 */

#include "tbos_desktop.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * THEME DEFINITIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_desktop_theme_dark(tbos_desktop_theme_t* theme) {
    if (!theme) return;

    /* Base colors - Modern dark with depth */
    theme->bg_primary   = (tbos_rgba_t){18, 18, 20, 255};
    theme->bg_secondary = (tbos_rgba_t){28, 28, 32, 255};
    theme->bg_tertiary  = (tbos_rgba_t){38, 38, 44, 255};
    theme->bg_elevated  = (tbos_rgba_t){48, 48, 56, 255};

    /* Text */
    theme->text_primary   = (tbos_rgba_t){255, 255, 255, 255};
    theme->text_secondary = (tbos_rgba_t){170, 170, 180, 255};
    theme->text_disabled  = (tbos_rgba_t){100, 100, 110, 255};

    /* Accent - Vibrant cyan/teal */
    theme->accent         = (tbos_rgba_t){0, 200, 220, 255};
    theme->accent_hover   = (tbos_rgba_t){0, 220, 240, 255};
    theme->accent_pressed = (tbos_rgba_t){0, 160, 180, 255};

    /* Semantic */
    theme->success = (tbos_rgba_t){76, 217, 100, 255};
    theme->warning = (tbos_rgba_t){255, 204, 0, 255};
    theme->error   = (tbos_rgba_t){255, 69, 58, 255};
    theme->info    = (tbos_rgba_t){90, 200, 250, 255};

    /* UI elements */
    theme->border  = (tbos_rgba_t){60, 60, 70, 255};
    theme->divider = (tbos_rgba_t){50, 50, 58, 255};
    theme->shadow  = (tbos_rgba_t){0, 0, 0, 80};
    theme->overlay = (tbos_rgba_t){0, 0, 0, 160};

    /* Window */
    theme->window_bg         = (tbos_rgba_t){30, 30, 36, 245};
    theme->window_title_bg   = (tbos_rgba_t){40, 40, 48, 255};
    theme->window_title_text = (tbos_rgba_t){220, 220, 230, 255};
    theme->window_border     = (tbos_rgba_t){55, 55, 65, 255};

    /* Dock/menubar - Glassmorphism */
    theme->menubar_bg = (tbos_rgba_t){28, 28, 32, 220};
    theme->dock_bg    = (tbos_rgba_t){40, 40, 48, 200};

    /* Glass effect */
    theme->blur_radius   = 20;
    theme->glass_opacity = 200;
}

void tbos_desktop_theme_light(tbos_desktop_theme_t* theme) {
    if (!theme) return;

    /* Base colors - Clean white with subtle warmth */
    theme->bg_primary   = (tbos_rgba_t){250, 250, 252, 255};
    theme->bg_secondary = (tbos_rgba_t){242, 242, 247, 255};
    theme->bg_tertiary  = (tbos_rgba_t){230, 230, 235, 255};
    theme->bg_elevated  = (tbos_rgba_t){255, 255, 255, 255};

    /* Text */
    theme->text_primary   = (tbos_rgba_t){28, 28, 30, 255};
    theme->text_secondary = (tbos_rgba_t){100, 100, 105, 255};
    theme->text_disabled  = (tbos_rgba_t){160, 160, 165, 255};

    /* Accent - Vibrant blue */
    theme->accent         = (tbos_rgba_t){0, 122, 255, 255};
    theme->accent_hover   = (tbos_rgba_t){0, 100, 220, 255};
    theme->accent_pressed = (tbos_rgba_t){0, 80, 180, 255};

    /* Semantic */
    theme->success = (tbos_rgba_t){52, 199, 89, 255};
    theme->warning = (tbos_rgba_t){255, 149, 0, 255};
    theme->error   = (tbos_rgba_t){255, 59, 48, 255};
    theme->info    = (tbos_rgba_t){50, 173, 230, 255};

    /* UI elements */
    theme->border  = (tbos_rgba_t){210, 210, 215, 255};
    theme->divider = (tbos_rgba_t){220, 220, 225, 255};
    theme->shadow  = (tbos_rgba_t){0, 0, 0, 30};
    theme->overlay = (tbos_rgba_t){0, 0, 0, 100};

    /* Window */
    theme->window_bg         = (tbos_rgba_t){255, 255, 255, 250};
    theme->window_title_bg   = (tbos_rgba_t){245, 245, 248, 255};
    theme->window_title_text = (tbos_rgba_t){50, 50, 55, 255};
    theme->window_border     = (tbos_rgba_t){200, 200, 205, 255};

    /* Dock/menubar */
    theme->menubar_bg = (tbos_rgba_t){245, 245, 248, 230};
    theme->dock_bg    = (tbos_rgba_t){250, 250, 252, 210};

    /* Glass effect */
    theme->blur_radius   = 15;
    theme->glass_opacity = 230;
}

void tbos_desktop_theme_saffron(tbos_desktop_theme_t* theme) {
    if (!theme) return;

    /* Start with dark theme as base */
    tbos_desktop_theme_dark(theme);

    /* Override with saffron/dharmic colors */
    theme->bg_primary   = (tbos_rgba_t){20, 15, 12, 255};
    theme->bg_secondary = (tbos_rgba_t){32, 24, 18, 255};
    theme->bg_tertiary  = (tbos_rgba_t){45, 35, 28, 255};
    theme->bg_elevated  = (tbos_rgba_t){55, 42, 32, 255};

    /* Saffron accent */
    theme->accent         = (tbos_rgba_t){255, 153, 51, 255};  /* Saffron */
    theme->accent_hover   = (tbos_rgba_t){255, 180, 80, 255};
    theme->accent_pressed = (tbos_rgba_t){220, 130, 40, 255};

    /* Sacred colors for semantic */
    theme->success = (tbos_rgba_t){19, 136, 8, 255};   /* India Green */
    theme->info    = (tbos_rgba_t){6, 3, 141, 255};    /* India Blue */

    /* Window with warm tint */
    theme->window_bg       = (tbos_rgba_t){35, 28, 22, 250};
    theme->window_title_bg = (tbos_rgba_t){48, 38, 30, 255};

    /* Dock with saffron glow */
    theme->dock_bg = (tbos_rgba_t){45, 35, 28, 210};
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DESKTOP LIFECYCLE
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_desktop_init(tbos_desktop_t* desktop, int width, int height) {
    if (!desktop) return -1;

    memset(desktop, 0, sizeof(tbos_desktop_t));

    desktop->screen_width = width;
    desktop->screen_height = height;

    /* Default theme */
    desktop->theme_mode = TBOS_THEME_DARK;
    tbos_desktop_theme_dark(&desktop->theme);

    /* Initialize dock with default apps */
    desktop->dock.position = TBOS_DOCK_BOTTOM;
    desktop->dock.magnification = true;
    desktop->dock.magnification_scale = 1.5f;
    desktop->dock.visible = true;
    desktop->dock.show_progress = 1.0f;
    desktop->dock.hovered_index = -1;

    /* Initialize menubar */
    desktop->menubar.show_clock = true;
    desktop->menubar.show_date = true;
    desktop->menubar.active_menu = -1;

    /* Initialize workspaces */
    desktop->workspace_count = TBOS_MAX_WORKSPACES;
    desktop->current_workspace = 0;

    /* Default wallpaper color */
    desktop->wallpaper_color = (tbos_rgba_t){30, 35, 45, 255};

    /* Karma defaults */
    desktop->karma = 5000;
    desktop->max_karma = 10000;

    return 0;
}

void tbos_desktop_free(tbos_desktop_t* desktop) {
    if (!desktop) return;

    /* Free windows */
    for (int i = 0; i < desktop->window_count; i++) {
        free(desktop->windows[i]);
    }

    /* Free wallpaper */
    if (desktop->wallpaper) {
        tbos_pixbuf_free(desktop->wallpaper);
    }
}

void tbos_desktop_set_theme(tbos_desktop_t* desktop, tbos_theme_mode_t mode) {
    if (!desktop) return;

    desktop->theme_mode = mode;

    switch (mode) {
        case TBOS_THEME_LIGHT:
            tbos_desktop_theme_light(&desktop->theme);
            break;
        case TBOS_THEME_SAFFRON:
            tbos_desktop_theme_saffron(&desktop->theme);
            break;
        case TBOS_THEME_DARK:
        default:
            tbos_desktop_theme_dark(&desktop->theme);
            break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WINDOW MANAGEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

static uint32_t next_window_id = 1;

tbos_window_t* tbos_desktop_window_create(tbos_desktop_t* desktop,
    const char* title, int x, int y, int width, int height,
    tbos_window_flags_t flags) {

    if (!desktop || desktop->window_count >= TBOS_MAX_WINDOWS) return NULL;

    tbos_window_t* win = calloc(1, sizeof(tbos_window_t));
    if (!win) return NULL;

    win->id = next_window_id++;
    strncpy(win->title, title ? title : "Untitled", sizeof(win->title) - 1);
    win->icon = TBOS_ICON_APP_FILES;

    win->x = x;
    win->y = y;
    win->width = width;
    win->height = height;
    win->min_width = 200;
    win->min_height = 100;
    win->max_width = desktop->screen_width;
    win->max_height = desktop->screen_height;

    win->state = TBOS_WIN_NORMAL;
    win->flags = flags;
    win->visible = true;
    win->z_order = desktop->window_count;

    /* Save restore position */
    win->restore_x = x;
    win->restore_y = y;
    win->restore_w = width;
    win->restore_h = height;

    desktop->windows[desktop->window_count++] = win;
    tbos_desktop_window_focus(desktop, win);

    return win;
}

void tbos_desktop_window_close(tbos_desktop_t* desktop, tbos_window_t* window) {
    if (!desktop || !window) return;

    /* Find and remove window */
    for (int i = 0; i < desktop->window_count; i++) {
        if (desktop->windows[i] == window) {
            /* Shift remaining windows */
            for (int j = i; j < desktop->window_count - 1; j++) {
                desktop->windows[j] = desktop->windows[j + 1];
            }
            desktop->window_count--;

            /* Update focus */
            if (desktop->focused_window == window) {
                desktop->focused_window = desktop->window_count > 0 ?
                    desktop->windows[desktop->window_count - 1] : NULL;
            }

            free(window);
            break;
        }
    }
}

void tbos_desktop_window_focus(tbos_desktop_t* desktop, tbos_window_t* window) {
    if (!desktop || !window) return;

    /* Remove focus from current */
    if (desktop->focused_window) {
        desktop->focused_window->focused = false;
    }

    /* Set new focus */
    window->focused = true;
    desktop->focused_window = window;

    /* Bring to front */
    for (int i = 0; i < desktop->window_count; i++) {
        if (desktop->windows[i] == window) {
            /* Move to end of array (top of z-order) */
            for (int j = i; j < desktop->window_count - 1; j++) {
                desktop->windows[j] = desktop->windows[j + 1];
            }
            desktop->windows[desktop->window_count - 1] = window;
            break;
        }
    }

    /* Update z-order values */
    for (int i = 0; i < desktop->window_count; i++) {
        desktop->windows[i]->z_order = i;
    }
}

void tbos_desktop_window_minimize(tbos_desktop_t* desktop, tbos_window_t* window) {
    if (!desktop || !window) return;

    if (window->state != TBOS_WIN_MINIMIZED) {
        window->restore_x = window->x;
        window->restore_y = window->y;
        window->restore_w = window->width;
        window->restore_h = window->height;
        window->state = TBOS_WIN_MINIMIZED;
        window->visible = false;

        /* Focus next window */
        if (desktop->focused_window == window && desktop->window_count > 1) {
            for (int i = desktop->window_count - 1; i >= 0; i--) {
                if (desktop->windows[i] != window && desktop->windows[i]->visible) {
                    tbos_desktop_window_focus(desktop, desktop->windows[i]);
                    break;
                }
            }
        }
    }
}

void tbos_desktop_window_maximize(tbos_desktop_t* desktop, tbos_window_t* window) {
    if (!desktop || !window) return;

    if (window->state != TBOS_WIN_MAXIMIZED) {
        window->restore_x = window->x;
        window->restore_y = window->y;
        window->restore_w = window->width;
        window->restore_h = window->height;

        window->x = 0;
        window->y = TBOS_MENUBAR_HEIGHT;
        window->width = desktop->screen_width;
        window->height = desktop->screen_height - TBOS_MENUBAR_HEIGHT - TBOS_DOCK_HEIGHT;
        window->state = TBOS_WIN_MAXIMIZED;
    }
}

void tbos_desktop_window_restore(tbos_desktop_t* desktop, tbos_window_t* window) {
    if (!desktop || !window) return;

    window->x = window->restore_x;
    window->y = window->restore_y;
    window->width = window->restore_w;
    window->height = window->restore_h;
    window->state = TBOS_WIN_NORMAL;
    window->visible = true;

    tbos_desktop_window_focus(desktop, window);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DOCK
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_desktop_dock_add(tbos_desktop_t* desktop, const char* name,
    tbos_icon_type_t icon, void (*launch)(void*), void* userdata) {

    if (!desktop || desktop->dock.count >= TBOS_MAX_DOCK_ITEMS) return;

    tbos_dock_item_t* item = &desktop->dock.items[desktop->dock.count++];
    strncpy(item->name, name ? name : "App", sizeof(item->name) - 1);
    item->icon = icon;
    item->pinned = true;
    item->running = false;
    item->hover_scale = 1.0f;
    item->launch = launch;
    item->userdata = userdata;
}

void tbos_desktop_dock_set_badge(tbos_desktop_t* desktop, int index, int count) {
    if (!desktop || index < 0 || index >= desktop->dock.count) return;
    desktop->dock.items[index].badge_count = count;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * NOTIFICATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static uint32_t next_notification_id = 1;

uint32_t tbos_desktop_notify(tbos_desktop_t* desktop, const char* title,
    const char* message, tbos_notification_type_t type) {

    if (!desktop) return 0;

    /* Make room if needed */
    if (desktop->notifications.count >= TBOS_MAX_NOTIFICATIONS) {
        /* Remove oldest */
        for (int i = 0; i < TBOS_MAX_NOTIFICATIONS - 1; i++) {
            desktop->notifications.items[i] = desktop->notifications.items[i + 1];
        }
        desktop->notifications.count--;
    }

    tbos_notification_t* notif = &desktop->notifications.items[desktop->notifications.count++];
    memset(notif, 0, sizeof(tbos_notification_t));

    notif->id = next_notification_id++;
    strncpy(notif->title, title ? title : "", sizeof(notif->title) - 1);
    strncpy(notif->message, message ? message : "", sizeof(notif->message) - 1);
    notif->type = type;
    notif->timestamp = desktop->current_time;
    notif->duration_ms = 5000;  /* 5 seconds */
    notif->dismissable = true;
    notif->show_progress = 0.0f;

    desktop->notifications.unread_count++;

    return notif->id;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DESKTOP ICONS
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_desktop_icon_add(tbos_desktop_t* desktop, const char* name,
    const char* path, tbos_icon_type_t icon) {

    if (!desktop || desktop->icon_count >= 64) return;

    tbos_desktop_icon_t* di = &desktop->icons[desktop->icon_count];
    strncpy(di->name, name ? name : "Icon", sizeof(di->name) - 1);
    strncpy(di->path, path ? path : "", sizeof(di->path) - 1);
    di->icon = icon;

    /* Position in a column-major grid (fill top-to-bottom, then wrap to a
     * new column), matching how every mainstream desktop lays out icons.
     * Row-major would place the 2nd/3rd icon directly under where app
     * windows open, hiding them behind the first launched window. */
    int usable_h = desktop->screen_height - TBOS_MENUBAR_HEIGHT - TBOS_DOCK_HEIGHT - 48;
    int rows_per_col = usable_h / TBOS_DESKTOP_ICON_SPACING;
    if (rows_per_col < 1) rows_per_col = 1;
    int col = desktop->icon_count / rows_per_col;
    int row = desktop->icon_count % rows_per_col;
    di->x = 24 + col * TBOS_DESKTOP_ICON_SPACING;
    di->y = TBOS_MENUBAR_HEIGHT + 24 + row * TBOS_DESKTOP_ICON_SPACING;

    desktop->icon_count++;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * INPUT HANDLING
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_desktop_mouse_move(tbos_desktop_t* desktop, int x, int y) {
    if (!desktop) return;

    int dx = x - desktop->mouse_x;
    int dy = y - desktop->mouse_y;
    desktop->mouse_x = x;
    desktop->mouse_y = y;

    /* Handle window dragging */
    if (desktop->dragging_window && desktop->focused_window) {
        tbos_window_t* win = desktop->focused_window;
        win->x += dx;
        win->y += dy;

        /* Keep in bounds */
        if (win->y < TBOS_MENUBAR_HEIGHT) win->y = TBOS_MENUBAR_HEIGHT;
    }

    /* Handle window resizing */
    if (desktop->resizing_window && desktop->focused_window) {
        tbos_window_t* win = desktop->focused_window;

        if (desktop->resize_edge & 2) {  /* Right */
            win->width += dx;
            if (win->width < win->min_width) win->width = win->min_width;
        }
        if (desktop->resize_edge & 4) {  /* Bottom */
            win->height += dy;
            if (win->height < win->min_height) win->height = win->min_height;
        }
    }

    /* Update dock hover */
    if (desktop->dock.position == TBOS_DOCK_BOTTOM) {
        int dock_y = desktop->screen_height - TBOS_DOCK_HEIGHT;
        if (y >= dock_y) {
            int dock_width = desktop->dock.count * (TBOS_DOCK_ICON_SIZE + TBOS_DOCK_MARGIN);
            int dock_x = (desktop->screen_width - dock_width) / 2;
            int relative_x = x - dock_x;
            if (relative_x >= 0 && relative_x < dock_width) {
                desktop->dock.hovered_index = relative_x / (TBOS_DOCK_ICON_SIZE + TBOS_DOCK_MARGIN);
            } else {
                desktop->dock.hovered_index = -1;
            }
        } else {
            desktop->dock.hovered_index = -1;
        }
    }
}

void tbos_desktop_mouse_button(tbos_desktop_t* desktop, bool left, bool right) {
    if (!desktop) return;

    bool left_clicked = left && !desktop->mouse_left;
    bool left_released = !left && desktop->mouse_left;

    desktop->mouse_left = left;
    desktop->mouse_right = right;

    if (left_released) {
        desktop->dragging_window = false;
        desktop->resizing_window = false;
    }

    if (!left_clicked) return;

    int mx = desktop->mouse_x;
    int my = desktop->mouse_y;

    /* Check dock click */
    if (desktop->dock.position == TBOS_DOCK_BOTTOM) {
        int dock_y = desktop->screen_height - TBOS_DOCK_HEIGHT;
        if (my >= dock_y && desktop->dock.hovered_index >= 0) {
            tbos_dock_item_t* item = &desktop->dock.items[desktop->dock.hovered_index];
            if (item->launch) {
                item->launch(item->userdata);
            }
            /* Add bounce animation */
            item->bounce_offset = -15.0f;
            return;
        }
    }

    /* Check menubar click */
    if (my < TBOS_MENUBAR_HEIGHT) {
        /* Handle menubar interaction */
        return;
    }

    /* Check window clicks (reverse order for z-order) */
    for (int i = desktop->window_count - 1; i >= 0; i--) {
        tbos_window_t* win = desktop->windows[i];
        if (!win->visible) continue;

        /* Check if click is in window bounds */
        if (mx >= win->x && mx < win->x + win->width &&
            my >= win->y && my < win->y + win->height) {

            tbos_desktop_window_focus(desktop, win);

            /* Check title bar (for dragging) */
            if (my < win->y + TBOS_WINDOW_TITLE_HEIGHT) {
                /* Close button (right side of title) */
                int close_x = win->x + win->width - 24;
                if (mx >= close_x && mx < close_x + 16) {
                    tbos_desktop_window_close(desktop, win);
                    return;
                }

                /* Minimize button */
                int min_x = close_x - 24;
                if (mx >= min_x && mx < min_x + 16) {
                    tbos_desktop_window_minimize(desktop, win);
                    return;
                }

                /* Maximize button */
                int max_x = min_x - 24;
                if (mx >= max_x && mx < max_x + 16) {
                    if (win->state == TBOS_WIN_MAXIMIZED) {
                        tbos_desktop_window_restore(desktop, win);
                    } else {
                        tbos_desktop_window_maximize(desktop, win);
                    }
                    return;
                }

                /* Start dragging */
                desktop->dragging_window = true;
                desktop->drag_offset_x = mx - win->x;
                desktop->drag_offset_y = my - win->y;
            }

            /* Check resize edge (bottom-right corner) */
            if ((win->flags & TBOS_WIN_RESIZABLE) &&
                mx >= win->x + win->width - 16 &&
                my >= win->y + win->height - 16) {
                desktop->resizing_window = true;
                desktop->resize_edge = 2 | 4;  /* Right and bottom */
            }

            return;
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * UPDATE
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_desktop_update(tbos_desktop_t* desktop, float dt) {
    if (!desktop) return;

    desktop->delta_time = dt;
    desktop->current_time += (uint64_t)(dt * 1000);

    /* Update dock animations */
    for (int i = 0; i < desktop->dock.count; i++) {
        tbos_dock_item_t* item = &desktop->dock.items[i];

        /* Hover scale */
        float target_scale = (i == desktop->dock.hovered_index) ?
            desktop->dock.magnification_scale : 1.0f;
        item->hover_scale += (target_scale - item->hover_scale) * dt * 10.0f;

        /* Bounce animation */
        if (item->bounce_offset != 0) {
            item->bounce_offset += item->bounce_offset * -8.0f * dt;
            if (fabsf(item->bounce_offset) < 0.5f) {
                item->bounce_offset = 0;
            }
        }
    }

    /* Update notification animations */
    for (int i = 0; i < desktop->notifications.count; i++) {
        tbos_notification_t* notif = &desktop->notifications.items[i];

        /* Show animation */
        if (notif->show_progress < 1.0f) {
            notif->show_progress += dt * 4.0f;
            if (notif->show_progress > 1.0f) notif->show_progress = 1.0f;
        }

        /* Auto-dismiss */
        if (notif->duration_ms > 0) {
            uint64_t age = desktop->current_time - notif->timestamp;
            if (age > notif->duration_ms && !notif->dismissing) {
                notif->dismissing = true;
            }
        }
    }

    /* Update window animations */
    for (int i = 0; i < desktop->window_count; i++) {
        tbos_window_t* win = desktop->windows[i];
        if (win->animating) {
            win->anim_progress += dt * 5.0f;
            if (win->anim_progress >= 1.0f) {
                win->anim_progress = 1.0f;
                win->animating = false;
            }
        }
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * ICON RENDERING
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_icon_draw(tbos_pixbuf_t* buf, int x, int y, int size,
    tbos_icon_type_t icon, tbos_rgba_t color) {

    if (!buf) return;

    int cx = x + size / 2;
    int cy = y + size / 2;
    int r = size / 2 - 2;

    switch (icon) {
        case TBOS_ICON_TBOS:
            /* Om-inspired TBOS logo */
            tbos_draw_circle(buf, cx, cy, r, color);
            tbos_draw_circle(buf, cx, cy, r * 2 / 3, color);
            tbos_draw_arc(buf, cx - r/3, cy - r/3, r/3, 0, M_PI, color);
            break;

        case TBOS_ICON_FOLDER:
            /* Folder shape */
            tbos_draw_rect_rounded_fill(buf, x + 2, y + size/3, size - 4, size * 2/3 - 2, 3, color);
            tbos_draw_rect_rounded_fill(buf, x + 2, y + size/4, size/2, size/6, 2, color);
            break;

        case TBOS_ICON_FILE:
            /* File with folded corner */
            tbos_draw_rect_fill(buf, x + size/4, y + 2, size/2, size - 4, color);
            tbos_draw_line(buf, x + size*3/4 - 4, y + 2, x + size*3/4, y + 8, color);
            break;

        case TBOS_ICON_TERMINAL:
            /* Terminal prompt */
            tbos_draw_rect_rounded_fill(buf, x + 2, y + 2, size - 4, size - 4, 4, color);
            {
                tbos_rgba_t dark = {0, 0, 0, 200};
                tbos_draw_rect_rounded_fill(buf, x + 4, y + 4, size - 8, size - 8, 2, dark);
            }
            tbos_draw_line(buf, x + 8, cy, x + 12, cy - 4, color);
            tbos_draw_line(buf, x + 8, cy, x + 12, cy + 4, color);
            tbos_draw_hline(buf, x + 14, x + size - 10, cy, color);
            break;

        case TBOS_ICON_SETTINGS:
            /* Gear */
            tbos_draw_circle(buf, cx, cy, r * 2/3, color);
            tbos_draw_circle_fill(buf, cx, cy, r/3, (tbos_rgba_t){0,0,0,0});
            for (int i = 0; i < 8; i++) {
                float angle = i * M_PI / 4;
                int px = cx + (int)(r * 0.8f * cosf(angle));
                int py = cy + (int)(r * 0.8f * sinf(angle));
                tbos_draw_circle_fill(buf, px, py, 3, color);
            }
            break;

        case TBOS_ICON_SEARCH:
            /* Magnifying glass */
            tbos_draw_circle(buf, cx - 2, cy - 2, r * 2/3, color);
            tbos_draw_line(buf, cx + r/3, cy + r/3, cx + r - 2, cy + r - 2, color);
            tbos_draw_line(buf, cx + r/3 + 1, cy + r/3, cx + r - 1, cy + r - 2, color);
            break;

        case TBOS_ICON_CLOSE:
            /* X */
            tbos_draw_line(buf, x + 4, y + 4, x + size - 4, y + size - 4, color);
            tbos_draw_line(buf, x + size - 4, y + 4, x + 4, y + size - 4, color);
            break;

        case TBOS_ICON_MINIMIZE:
            /* Horizontal line */
            tbos_draw_hline(buf, x + 4, x + size - 4, cy, color);
            break;

        case TBOS_ICON_MAXIMIZE:
            /* Square */
            tbos_draw_rect(buf, x + 4, y + 4, size - 8, size - 8, color);
            break;

        case TBOS_ICON_WIFI:
            /* WiFi arcs */
            tbos_draw_arc(buf, cx, cy + r/2, r/3, M_PI, 2*M_PI, color);
            tbos_draw_arc(buf, cx, cy + r/2, r*2/3, M_PI, 2*M_PI, color);
            tbos_draw_arc(buf, cx, cy + r/2, r, M_PI, 2*M_PI, color);
            tbos_draw_circle_fill(buf, cx, cy + r/2, 2, color);
            break;

        case TBOS_ICON_BATTERY_FULL:
            /* Battery */
            tbos_draw_rect(buf, x + 2, cy - 4, size - 6, 8, color);
            tbos_draw_rect_fill(buf, x + size - 4, cy - 2, 2, 4, color);
            tbos_draw_rect_fill(buf, x + 4, cy - 2, size - 12, 4, color);
            break;

        case TBOS_ICON_KARMA:
            /* Om-like karma symbol */
            tbos_draw_circle(buf, cx, cy, r, color);
            tbos_draw_circle(buf, cx, cy, r * 2/3, color);
            tbos_draw_circle(buf, cx, cy, r/3, color);
            break;

        default:
            /* Default: filled circle */
            tbos_draw_circle_fill(buf, cx, cy, r, color);
            break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING
 * ═══════════════════════════════════════════════════════════════════════════ */

static void render_wallpaper(tbos_desktop_t* desktop, tbos_pixbuf_t* buf) {
    if (desktop->wallpaper) {
        /* TODO: Blit wallpaper */
    } else {
        /* Gradient wallpaper */
        tbos_rgba_t top = desktop->theme.bg_primary;
        tbos_rgba_t bottom = desktop->theme.bg_secondary;

        for (uint32_t y = 0; y < buf->height; y++) {
            float t = (float)y / buf->height;
            tbos_rgba_t color = {
                (uint8_t)(top.r + (bottom.r - top.r) * t),
                (uint8_t)(top.g + (bottom.g - top.g) * t),
                (uint8_t)(top.b + (bottom.b - top.b) * t),
                255
            };
            tbos_draw_hline(buf, 0, buf->width - 1, y, color);
        }
    }
}

static void render_menubar(tbos_desktop_t* desktop, tbos_pixbuf_t* buf) {
    /* Menubar background with glass effect */
    tbos_draw_rect_fill(buf, 0, 0, buf->width, TBOS_MENUBAR_HEIGHT,
                        desktop->theme.menubar_bg);
    tbos_draw_hline(buf, 0, buf->width - 1, TBOS_MENUBAR_HEIGHT - 1,
                    desktop->theme.border);

    /* TBOS logo (left) */
    tbos_icon_draw(buf, 8, 4, 20, TBOS_ICON_TBOS, desktop->theme.accent);

    /* App name */
    const char* app_name = desktop->focused_window ?
        desktop->focused_window->title : "TernaryBit OS";
    tbos_draw_text(buf, 36, 6, app_name, &tbos_font_8x16,
                   desktop->theme.text_primary);

    /* System tray (right side) */
    int tray_x = buf->width - 16;

    /* Clock */
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[32];
    if (desktop->menubar.show_date) {
        strftime(time_str, sizeof(time_str), "%a %b %d  %H:%M", tm_info);
    } else {
        strftime(time_str, sizeof(time_str), "%H:%M", tm_info);
    }
    int time_width = tbos_text_width(time_str, &tbos_font_8x16);
    tray_x -= time_width + 8;
    tbos_draw_text(buf, tray_x, 6, time_str, &tbos_font_8x16,
                   desktop->theme.text_primary);

    /* Karma indicator */
    tray_x -= 32;
    tbos_icon_draw(buf, tray_x, 4, 20, TBOS_ICON_KARMA, desktop->theme.accent);

    /* WiFi */
    tray_x -= 28;
    tbos_icon_draw(buf, tray_x, 4, 20, TBOS_ICON_WIFI, desktop->theme.text_secondary);

    /* Battery */
    tray_x -= 28;
    tbos_icon_draw(buf, tray_x, 4, 20, TBOS_ICON_BATTERY_FULL, desktop->theme.text_secondary);
}

static void render_dock(tbos_desktop_t* desktop, tbos_pixbuf_t* buf) {
    if (!desktop->dock.visible) return;

    int dock_y = buf->height - TBOS_DOCK_HEIGHT;
    int dock_width = desktop->dock.count * (TBOS_DOCK_ICON_SIZE + TBOS_DOCK_MARGIN) + TBOS_DOCK_MARGIN * 2;
    int dock_x = (buf->width - dock_width) / 2;

    /* Dock background with rounded corners */
    tbos_draw_rect_rounded_fill(buf, dock_x, dock_y + 8, dock_width, TBOS_DOCK_HEIGHT - 16,
                                 12, desktop->theme.dock_bg);
    tbos_draw_rect_rounded(buf, dock_x, dock_y + 8, dock_width, TBOS_DOCK_HEIGHT - 16,
                           12, desktop->theme.border);

    /* Draw icons */
    int icon_x = dock_x + TBOS_DOCK_MARGIN;
    for (int i = 0; i < desktop->dock.count; i++) {
        tbos_dock_item_t* item = &desktop->dock.items[i];

        int icon_size = (int)(TBOS_DOCK_ICON_SIZE * item->hover_scale);
        int offset_y = (TBOS_DOCK_ICON_SIZE - icon_size) / 2;
        int bounce_y = (int)item->bounce_offset;

        int ix = icon_x + (TBOS_DOCK_ICON_SIZE - icon_size) / 2;
        int iy = dock_y + 12 + offset_y + bounce_y;

        /* Icon background */
        tbos_draw_rect_rounded_fill(buf, ix - 2, iy - 2, icon_size + 4, icon_size + 4,
                                     8, desktop->theme.bg_elevated);

        /* Icon */
        tbos_icon_draw(buf, ix, iy, icon_size, item->icon, desktop->theme.accent);

        /* Running indicator (dot below) */
        if (item->running) {
            tbos_draw_circle_fill(buf, icon_x + TBOS_DOCK_ICON_SIZE / 2,
                                  dock_y + TBOS_DOCK_HEIGHT - 10, 3,
                                  desktop->theme.text_secondary);
        }

        /* Badge */
        if (item->badge_count > 0) {
            tbos_draw_circle_fill(buf, ix + icon_size - 4, iy + 4, 8,
                                  desktop->theme.error);
            char badge[8];
            snprintf(badge, sizeof(badge), "%d", item->badge_count > 9 ? 9 : item->badge_count);
            tbos_draw_text(buf, ix + icon_size - 8, iy, badge,
                          &tbos_font_8x16, desktop->theme.text_primary);
        }

        icon_x += TBOS_DOCK_ICON_SIZE + TBOS_DOCK_MARGIN;
    }
}

static void render_window(tbos_desktop_t* desktop, tbos_pixbuf_t* buf, tbos_window_t* win) {
    if (!win->visible) return;

    tbos_desktop_theme_t* theme = &desktop->theme;

    /* Shadow: layered translucent FILLED rounded rects, largest/faintest
     * first. Filled layers (not outline strokes) give a smooth halo once
     * the opaque window is painted on top; outline strokes overlap far
     * more densely at the rounded corners than along straight edges and
     * previously left visible dark wedges pointing out of each corner. */
    for (int i = TBOS_WINDOW_SHADOW; i >= 1; i--) {
        tbos_rgba_t shadow = theme->shadow;
        shadow.a = (uint8_t)((int)shadow.a * i / (TBOS_WINDOW_SHADOW * 4));
        tbos_draw_rect_rounded_fill(buf, win->x - i, win->y - i + 2,
                               win->width + i * 2, win->height + i * 2,
                               TBOS_WINDOW_RADIUS + i, shadow);
    }

    /* Window background */
    tbos_draw_rect_rounded_fill(buf, win->x, win->y, win->width, win->height,
                                 TBOS_WINDOW_RADIUS, theme->window_bg);

    /* Title bar */
    tbos_draw_rect_rounded_fill(buf, win->x, win->y, win->width, TBOS_WINDOW_TITLE_HEIGHT,
                                 TBOS_WINDOW_RADIUS, theme->window_title_bg);

    /* Only round top corners for title bar - draw rect to cover bottom */
    tbos_draw_rect_fill(buf, win->x, win->y + TBOS_WINDOW_TITLE_HEIGHT - TBOS_WINDOW_RADIUS,
                        win->width, TBOS_WINDOW_RADIUS, theme->window_title_bg);

    /* Title bar divider */
    tbos_draw_hline(buf, win->x, win->x + win->width - 1, win->y + TBOS_WINDOW_TITLE_HEIGHT - 1,
                    theme->border);

    /* Window controls (traffic lights style) */
    int btn_y = win->y + TBOS_WINDOW_TITLE_HEIGHT / 2;
    int btn_x = win->x + 16;

    /* Close (red) */
    tbos_rgba_t close_color = win->focused ?
        (tbos_rgba_t){255, 95, 87, 255} : theme->text_disabled;
    tbos_draw_circle_fill(buf, btn_x, btn_y, 6, close_color);
    btn_x += 20;

    /* Minimize (yellow) */
    tbos_rgba_t min_color = win->focused ?
        (tbos_rgba_t){255, 189, 46, 255} : theme->text_disabled;
    tbos_draw_circle_fill(buf, btn_x, btn_y, 6, min_color);
    btn_x += 20;

    /* Maximize (green) */
    tbos_rgba_t max_color = win->focused ?
        (tbos_rgba_t){40, 200, 64, 255} : theme->text_disabled;
    tbos_draw_circle_fill(buf, btn_x, btn_y, 6, max_color);

    /* Title text (centered) */
    int title_width = tbos_text_width(win->title, &tbos_font_8x16);
    int title_x = win->x + (win->width - title_width) / 2;
    tbos_draw_text(buf, title_x, win->y + 8, win->title, &tbos_font_8x16,
                   theme->window_title_text);

    /* Window border */
    tbos_rgba_t border_color = win->focused ? theme->accent : theme->window_border;
    tbos_draw_rect_rounded(buf, win->x, win->y, win->width, win->height,
                           TBOS_WINDOW_RADIUS, border_color);

    /* Content area */
    if (win->render) {
        /* TODO: Clip to content area and call render callback */
        win->render(win, buf, win->userdata);
    }
}

static void render_desktop_icons(tbos_desktop_t* desktop, tbos_pixbuf_t* buf) {
    for (int i = 0; i < desktop->icon_count; i++) {
        tbos_desktop_icon_t* icon = &desktop->icons[i];

        /* Icon */
        tbos_icon_draw(buf, icon->x, icon->y, TBOS_DESKTOP_ICON_SIZE,
                       icon->icon, desktop->theme.text_primary);

        /* Label */
        int label_width = tbos_text_width(icon->name, &tbos_font_8x16);
        int label_x = icon->x + (TBOS_DESKTOP_ICON_SIZE - label_width) / 2;
        tbos_draw_text(buf, label_x, icon->y + TBOS_DESKTOP_ICON_SIZE + 4,
                       icon->name, &tbos_font_8x16, desktop->theme.text_primary);

        /* Selection highlight */
        if (icon->selected) {
            tbos_draw_rect_rounded(buf, icon->x - 4, icon->y - 4,
                                   TBOS_DESKTOP_ICON_SIZE + 8, TBOS_DESKTOP_ICON_SIZE + 24,
                                   4, desktop->theme.accent);
        }
    }
}

static void render_notifications(tbos_desktop_t* desktop, tbos_pixbuf_t* buf) {
    int notif_x = buf->width - 320 - 16;
    int notif_y = TBOS_MENUBAR_HEIGHT + 16;

    for (int i = 0; i < desktop->notifications.count; i++) {
        tbos_notification_t* notif = &desktop->notifications.items[i];

        /* Slide-in animation */
        float progress = notif->show_progress;
        int offset_x = (int)((1.0f - progress) * 340);

        int nx = notif_x + offset_x;
        int ny = notif_y + i * 90;

        /* Notification card */
        tbos_draw_rect_rounded_fill(buf, nx, ny, 320, 80, 12,
                                     desktop->theme.bg_elevated);
        tbos_draw_rect_rounded(buf, nx, ny, 320, 80, 12,
                               desktop->theme.border);

        /* Icon based on type */
        tbos_rgba_t icon_color;
        switch (notif->type) {
            case TBOS_NOTIFY_SUCCESS: icon_color = desktop->theme.success; break;
            case TBOS_NOTIFY_WARNING: icon_color = desktop->theme.warning; break;
            case TBOS_NOTIFY_ERROR:   icon_color = desktop->theme.error; break;
            default:                  icon_color = desktop->theme.info; break;
        }
        tbos_draw_circle_fill(buf, nx + 24, ny + 24, 12, icon_color);

        /* Title */
        tbos_draw_text(buf, nx + 48, ny + 12, notif->title, &tbos_font_8x16,
                       desktop->theme.text_primary);

        /* Message (truncate if too long) */
        char msg_truncated[48];
        strncpy(msg_truncated, notif->message, sizeof(msg_truncated) - 1);
        msg_truncated[sizeof(msg_truncated) - 1] = '\0';
        if (strlen(notif->message) > sizeof(msg_truncated) - 4) {
            strcpy(msg_truncated + sizeof(msg_truncated) - 4, "...");
        }
        tbos_draw_text(buf, nx + 48, ny + 32, msg_truncated, &tbos_font_8x16,
                       desktop->theme.text_secondary);

        /* Close button */
        tbos_icon_draw(buf, nx + 320 - 28, ny + 8, 16, TBOS_ICON_CLOSE,
                       desktop->theme.text_disabled);
    }
}

void tbos_desktop_render(tbos_desktop_t* desktop, tbos_pixbuf_t* target) {
    if (!desktop || !target) return;

    /* 1. Wallpaper */
    render_wallpaper(desktop, target);

    /* 2. Desktop icons */
    render_desktop_icons(desktop, target);

    /* 3. Windows (in z-order) */
    for (int i = 0; i < desktop->window_count; i++) {
        render_window(desktop, target, desktop->windows[i]);
    }

    /* 4. Dock */
    render_dock(desktop, target);

    /* 5. Menubar (always on top) */
    render_menubar(desktop, target);

    /* 6. Notifications */
    render_notifications(desktop, target);

    /* 7. Spotlight (if open) */
    if (desktop->spotlight_open) {
        /* TODO: Render spotlight overlay */
    }
}
