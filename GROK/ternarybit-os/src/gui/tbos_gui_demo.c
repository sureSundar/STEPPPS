/**
 * @file tbos_gui_demo.c
 * @brief TBOS Native GUI Demo Application
 *
 * Demonstrates the PXFS-native pixel renderer with sacred geometry,
 * dharmic colors, and STEPPPS visualization.
 *
 * Build: cc -o tbos_gui_demo tbos_gui_demo.c tbos_pixel.c tbos_platform_sdl.c \
 *        -DTBOS_USE_SDL2 $(sdl2-config --cflags --libs) -lm
 *
 * @version 1.0
 * @date 2026-07-16
 */

#include "tbos_pixel.h"
#include "tbos_gui.h"
#include "tbos_platform.h"
#include <stdio.h>
#include <math.h>

/* Demo state */
typedef struct {
    int karma;
    int consciousness_level;
    float time;
    bool show_chakras;
    char command_buf[256];
} demo_state_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * DEMO RENDERING (NO SDL DEPENDENCY - PURE PIXEL OPS)
 * ═══════════════════════════════════════════════════════════════════════════ */

static void render_header(tbos_pixbuf_t* buf) {
    /* Header background */
    tbos_draw_rect_fill(buf, 0, 0, buf->width, 48, TBOS_COLOR_BG_PANEL);
    tbos_draw_hline(buf, 0, buf->width - 1, 47, TBOS_COLOR_BORDER);

    /* Om symbol (simplified) */
    int cx = 28, cy = 24;
    tbos_draw_circle(buf, cx, cy, 12, TBOS_COLOR_SAFFRON);
    tbos_draw_circle(buf, cx, cy, 8, TBOS_COLOR_SAFFRON);
    tbos_draw_arc(buf, cx - 4, cy - 4, 6, 0, M_PI, TBOS_COLOR_SAFFRON);

    /* Title */
    tbos_draw_text(buf, 50, 8, "TernaryBit OS", &tbos_font_8x16, TBOS_COLOR_CYAN);
    tbos_draw_text(buf, 50, 28, "Native GUI Demo", &tbos_font_8x16, TBOS_COLOR_TEXT_DIM);
}

static void render_sidebar(tbos_pixbuf_t* buf, demo_state_t* state) {
    int x = 0, y = 56;
    int w = 200, h = buf->height - 56;

    /* Sidebar background */
    tbos_draw_rect_fill(buf, x, y, w, h, TBOS_COLOR_BG_PANEL);
    tbos_draw_vline(buf, w - 1, y, y + h - 1, TBOS_COLOR_BORDER);

    int py = y + 16;

    /* Karma section */
    tbos_draw_text(buf, x + 16, py, "KARMA", &tbos_font_8x16, TBOS_COLOR_CYAN);
    py += 24;

    tbos_draw_karma_meter(buf, x + 16, py, w - 32, 20, state->karma, 10000);
    py += 32;

    char karma_str[32];
    snprintf(karma_str, sizeof(karma_str), "%d / 10000", state->karma);
    tbos_draw_text(buf, x + 16, py, karma_str, &tbos_font_8x16, TBOS_COLOR_TEXT);
    py += 32;

    /* Consciousness section */
    tbos_draw_text(buf, x + 16, py, "CONSCIOUSNESS", &tbos_font_8x16, TBOS_COLOR_CYAN);
    py += 24;

    tbos_draw_consciousness(buf, x + w / 2, py + 40, 60, state->consciousness_level);
    py += 100;

    const char* levels[] = {"Dormant", "Awakening", "Aware", "Compassionate", "Enlightened"};
    tbos_draw_text_centered(buf, x, py, w, levels[state->consciousness_level],
                            &tbos_font_8x16, TBOS_COLOR_TEXT);
    py += 32;

    /* Ashoka Chakra */
    if (state->show_chakras) {
        tbos_draw_text(buf, x + 16, py, "DHARMA CHAKRA", &tbos_font_8x16, TBOS_COLOR_CYAN);
        py += 24;
        tbos_draw_ashoka_chakra(buf, x + w / 2, py + 40, 60, TBOS_COLOR_BLUE);
    }
}

static void render_main_content(tbos_pixbuf_t* buf, demo_state_t* state) {
    int x = 208, y = 56;
    int w = buf->width - x - 8;
    int h = buf->height - 56 - 8;

    /* Main panel background */
    tbos_draw_rect_rounded_fill(buf, x, y, w, h, 8, TBOS_COLOR_BG_PANEL);
    tbos_draw_rect_rounded(buf, x, y, w, h, 8, TBOS_COLOR_BORDER);

    int px = x + 16;
    int py = y + 16;

    /* STEPPPS Dimensions */
    tbos_draw_text(buf, px, py, "STEPPPS Dimensions", &tbos_font_8x16, TBOS_COLOR_CYAN);
    py += 28;

    const char* dims[] = {
        "S - Space:      bhulok (Earth realm)",
        "T - Time:       2026-07-16T17:00:00Z",
        "E - Event:      GUI Demo Started",
        "P - Psychology: Karma = %d",
        "P - Pixel:      %dx%d RGBA32",
        "P - Prompt:     Demo the PXFS renderer",
        "S - Script:     Read-only display"
    };

    for (int i = 0; i < 7; i++) {
        char line[128];
        if (i == 3) {
            snprintf(line, sizeof(line), dims[i], state->karma);
        } else if (i == 4) {
            snprintf(line, sizeof(line), dims[i], buf->width, buf->height);
        } else {
            snprintf(line, sizeof(line), "%s", dims[i]);
        }
        tbos_draw_text(buf, px, py, line, &tbos_font_8x16, TBOS_COLOR_TEXT);
        py += 20;
    }

    py += 20;

    /* Animated element - pulsing circle */
    tbos_draw_text(buf, px, py, "Animation (Sacred Pulse)", &tbos_font_8x16, TBOS_COLOR_CYAN);
    py += 28;

    int pulse_r = 20 + (int)(10 * sin(state->time * 2));
    tbos_rgba_t pulse_color = TBOS_COLOR_SAFFRON;
    pulse_color.a = 128 + (int)(127 * sin(state->time * 3));
    tbos_draw_circle_fill(buf, px + 100, py + 30, pulse_r, pulse_color);
    tbos_draw_circle(buf, px + 100, py + 30, pulse_r + 5, TBOS_COLOR_CYAN);

    /* Draw chakra colors as a horizontal bar */
    py += 80;
    tbos_draw_text(buf, px, py, "Chakra Colors", &tbos_font_8x16, TBOS_COLOR_CYAN);
    py += 24;

    tbos_rgba_t chakras[] = {
        TBOS_COLOR_MULADHARA, TBOS_COLOR_SVADHI, TBOS_COLOR_MANIPURA,
        TBOS_COLOR_ANAHATA, TBOS_COLOR_VISHUDDHA, TBOS_COLOR_AJNA,
        TBOS_COLOR_SAHASRARA
    };

    int bar_w = (w - 48) / 7;
    for (int i = 0; i < 7; i++) {
        tbos_draw_rect_fill(buf, px + i * bar_w, py, bar_w - 2, 24, chakras[i]);
    }
}

static void render_footer(tbos_pixbuf_t* buf) {
    int y = buf->height - 32;

    /* Footer background */
    tbos_draw_rect_fill(buf, 0, y, buf->width, 32, TBOS_COLOR_BG_PANEL);
    tbos_draw_hline(buf, 0, buf->width - 1, y, TBOS_COLOR_BORDER);

    /* Footer text */
    tbos_draw_text(buf, 16, y + 8, "TBOS Native GUI | Press ESC to exit | Arrow keys change karma",
                   &tbos_font_8x16, TBOS_COLOR_TEXT_DIM);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * STANDALONE DEMO (NO SDL - OUTPUTS TO FILE)
 * ═══════════════════════════════════════════════════════════════════════════ */

#ifndef TBOS_USE_SDL2

/* Simple PPM output for testing without SDL */
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
    printf("TBOS Native GUI Demo (Standalone)\n");
    printf("==================================\n\n");

    /* Create pixel buffer */
    tbos_pixbuf_t* buf = tbos_pixbuf_create(1280, 720, TBOS_PIXEL_RGBA32);
    if (!buf) {
        fprintf(stderr, "Failed to create pixel buffer\n");
        return 1;
    }

    printf("Created %dx%d pixel buffer\n", buf->width, buf->height);

    /* Demo state */
    demo_state_t state = {
        .karma = 4200,
        .consciousness_level = 2,
        .time = 0,
        .show_chakras = true
    };

    /* Clear to background */
    tbos_pixbuf_clear(buf, TBOS_COLOR_BG_DARK);

    /* Render UI */
    render_header(buf);
    render_sidebar(buf, &state);
    render_main_content(buf, &state);
    render_footer(buf);

    /* Save to PPM file */
    const char* output = "tbos_gui_demo.ppm";
    save_ppm(buf, output);
    printf("Saved output to: %s\n", output);
    printf("\nView with: open %s (macOS) or display %s (Linux)\n", output, output);

    /* Cleanup */
    tbos_pixbuf_free(buf);

    printf("\nDemo complete!\n");
    return 0;
}

#else /* TBOS_USE_SDL2 */

/* ═══════════════════════════════════════════════════════════════════════════
 * SDL2 INTERACTIVE DEMO
 * ═══════════════════════════════════════════════════════════════════════════ */

static demo_state_t g_state = {
    .karma = 4200,
    .consciousness_level = 2,
    .time = 0,
    .show_chakras = true
};

static void demo_init(tbos_gui_t* gui, void* userdata) {
    (void)gui;
    (void)userdata;
    printf("TBOS GUI Demo initialized\n");
}

static void demo_update(tbos_gui_t* gui, float dt, void* userdata) {
    (void)userdata;

    g_state.time += dt;

    /* Handle keyboard */
    if (gui->keyboard.key_pressed) {
        switch (gui->keyboard.key) {
            case KEY_UP:
                g_state.karma += 100;
                if (g_state.karma > 10000) g_state.karma = 10000;
                break;
            case KEY_DOWN:
                g_state.karma -= 100;
                if (g_state.karma < 0) g_state.karma = 0;
                break;
            case KEY_LEFT:
                if (g_state.consciousness_level > 0) g_state.consciousness_level--;
                break;
            case KEY_RIGHT:
                if (g_state.consciousness_level < 4) g_state.consciousness_level++;
                break;
            case 'c':
            case 'C':
                g_state.show_chakras = !g_state.show_chakras;
                break;
        }
    }
}

static void demo_render(tbos_gui_t* gui, void* userdata) {
    (void)userdata;

    tbos_pixbuf_t* buf = gui->target;

    /* Clear */
    tbos_pixbuf_clear(buf, TBOS_COLOR_BG_DARK);

    /* Render UI */
    render_header(buf);
    render_sidebar(buf, &g_state);
    render_main_content(buf, &g_state);
    render_footer(buf);
}

static void demo_shutdown(void* userdata) {
    (void)userdata;
    printf("TBOS GUI Demo shutdown\n");
}

int main(void) {
    printf("TBOS Native GUI Demo (SDL2)\n");
    printf("===========================\n\n");
    printf("Controls:\n");
    printf("  Up/Down:    Change karma\n");
    printf("  Left/Right: Change consciousness\n");
    printf("  C:          Toggle chakra display\n");
    printf("  ESC:        Exit\n\n");

    tbos_window_config_t config = TBOS_WINDOW_DEFAULT;
    config.title = "TernaryBit OS - Native GUI Demo";

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
