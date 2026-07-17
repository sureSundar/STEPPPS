/**
 * @file tbos_pixel.h
 * @brief TBOS Pixel-Native GUI Core
 *
 * Custom PXFS-aligned graphics system that renders directly to pixel buffers.
 * Designed to scale from 4-bit calculators to 4K displays.
 *
 * Architecture:
 *   UI Components → PXFS Renderer → Platform Adapter → Display
 *
 * @version 1.0
 * @date 2026-07-16
 */

#ifndef TBOS_PIXEL_H
#define TBOS_PIXEL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * PIXEL TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief RGBA pixel (32-bit)
 */
typedef struct {
    uint8_t r, g, b, a;
} tbos_rgba_t;

/**
 * @brief RGB pixel (24-bit, PXFS native)
 */
typedef struct {
    uint8_t r, g, b;
} tbos_rgb_t;

/**
 * @brief Pixel format for different hardware tiers
 */
typedef enum {
    TBOS_PIXEL_1BIT,      /* Calculator: 1-bit monochrome */
    TBOS_PIXEL_4BIT,      /* Embedded: 16 colors */
    TBOS_PIXEL_8BIT,      /* Retro: 256 colors */
    TBOS_PIXEL_RGB565,    /* Embedded LCD: 16-bit */
    TBOS_PIXEL_RGB24,     /* Standard: 24-bit (PXFS native) */
    TBOS_PIXEL_RGBA32,    /* Desktop: 32-bit with alpha */
} tbos_pixel_format_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COLOR PALETTE - TBOS Theme (Sacred/Dharmic)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Primary colors */
#define TBOS_COLOR_SAFFRON    ((tbos_rgba_t){255, 153, 51, 255})   /* Hindu saffron */
#define TBOS_COLOR_WHITE      ((tbos_rgba_t){255, 255, 255, 255})
#define TBOS_COLOR_GREEN      ((tbos_rgba_t){19, 136, 8, 255})     /* Dharmic green */
#define TBOS_COLOR_BLUE       ((tbos_rgba_t){0, 0, 128, 255})      /* Ashoka Chakra */

/* UI colors */
#define TBOS_COLOR_BG_DARK    ((tbos_rgba_t){13, 17, 23, 255})     /* Dark background */
#define TBOS_COLOR_BG_PANEL   ((tbos_rgba_t){22, 27, 34, 255})     /* Panel background */
#define TBOS_COLOR_BORDER     ((tbos_rgba_t){48, 54, 61, 255})     /* Borders */
#define TBOS_COLOR_TEXT       ((tbos_rgba_t){201, 209, 217, 255})  /* Text */
#define TBOS_COLOR_TEXT_DIM   ((tbos_rgba_t){139, 148, 158, 255})  /* Dim text */
#define TBOS_COLOR_CYAN       ((tbos_rgba_t){57, 197, 207, 255})   /* Accent */
#define TBOS_COLOR_RED        ((tbos_rgba_t){248, 81, 73, 255})    /* Error */
#define TBOS_COLOR_YELLOW     ((tbos_rgba_t){210, 153, 34, 255})   /* Warning */
#define TBOS_COLOR_GREEN_UI   ((tbos_rgba_t){63, 185, 80, 255})    /* Success */

/* Chakra colors (for consciousness visualization) */
#define TBOS_COLOR_MULADHARA  ((tbos_rgba_t){255, 0, 0, 255})      /* Root - Red */
#define TBOS_COLOR_SVADHI     ((tbos_rgba_t){255, 127, 0, 255})    /* Sacral - Orange */
#define TBOS_COLOR_MANIPURA   ((tbos_rgba_t){255, 255, 0, 255})    /* Solar - Yellow */
#define TBOS_COLOR_ANAHATA    ((tbos_rgba_t){0, 255, 0, 255})      /* Heart - Green */
#define TBOS_COLOR_VISHUDDHA  ((tbos_rgba_t){0, 127, 255, 255})    /* Throat - Blue */
#define TBOS_COLOR_AJNA       ((tbos_rgba_t){75, 0, 130, 255})     /* Third Eye - Indigo */
#define TBOS_COLOR_SAHASRARA  ((tbos_rgba_t){148, 0, 211, 255})    /* Crown - Violet */

/* ═══════════════════════════════════════════════════════════════════════════
 * PIXEL BUFFER
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Pixel buffer - the fundamental rendering surface
 */
typedef struct {
    uint8_t* data;              /* Raw pixel data */
    uint32_t width;             /* Width in pixels */
    uint32_t height;            /* Height in pixels */
    uint32_t stride;            /* Bytes per row */
    tbos_pixel_format_t format; /* Pixel format */
    bool owned;                 /* If true, data is owned and should be freed */
} tbos_pixbuf_t;

/**
 * @brief Create a new pixel buffer
 */
tbos_pixbuf_t* tbos_pixbuf_create(uint32_t width, uint32_t height,
                                   tbos_pixel_format_t format);

/**
 * @brief Create pixel buffer from existing memory
 */
tbos_pixbuf_t* tbos_pixbuf_from_data(uint8_t* data, uint32_t width,
                                      uint32_t height, uint32_t stride,
                                      tbos_pixel_format_t format);

/**
 * @brief Free pixel buffer
 */
void tbos_pixbuf_free(tbos_pixbuf_t* buf);

/**
 * @brief Clear buffer to a color
 */
void tbos_pixbuf_clear(tbos_pixbuf_t* buf, tbos_rgba_t color);

/**
 * @brief Get pixel at position
 */
tbos_rgba_t tbos_pixbuf_get(const tbos_pixbuf_t* buf, int x, int y);

/**
 * @brief Set pixel at position
 */
void tbos_pixbuf_set(tbos_pixbuf_t* buf, int x, int y, tbos_rgba_t color);

/**
 * @brief Blend pixel with alpha
 */
void tbos_pixbuf_blend(tbos_pixbuf_t* buf, int x, int y, tbos_rgba_t color);

/* ═══════════════════════════════════════════════════════════════════════════
 * DRAWING PRIMITIVES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Draw horizontal line
 */
void tbos_draw_hline(tbos_pixbuf_t* buf, int x1, int x2, int y, tbos_rgba_t color);

/**
 * @brief Draw vertical line
 */
void tbos_draw_vline(tbos_pixbuf_t* buf, int x, int y1, int y2, tbos_rgba_t color);

/**
 * @brief Draw line (Bresenham)
 */
void tbos_draw_line(tbos_pixbuf_t* buf, int x1, int y1, int x2, int y2,
                    tbos_rgba_t color);

/**
 * @brief Draw rectangle outline
 */
void tbos_draw_rect(tbos_pixbuf_t* buf, int x, int y, int w, int h,
                    tbos_rgba_t color);

/**
 * @brief Draw filled rectangle
 */
void tbos_draw_rect_fill(tbos_pixbuf_t* buf, int x, int y, int w, int h,
                         tbos_rgba_t color);

/**
 * @brief Draw rounded rectangle
 */
void tbos_draw_rect_rounded(tbos_pixbuf_t* buf, int x, int y, int w, int h,
                            int radius, tbos_rgba_t color);

/**
 * @brief Draw filled rounded rectangle
 */
void tbos_draw_rect_rounded_fill(tbos_pixbuf_t* buf, int x, int y, int w, int h,
                                  int radius, tbos_rgba_t color);

/**
 * @brief Draw circle outline
 */
void tbos_draw_circle(tbos_pixbuf_t* buf, int cx, int cy, int r, tbos_rgba_t color);

/**
 * @brief Draw filled circle
 */
void tbos_draw_circle_fill(tbos_pixbuf_t* buf, int cx, int cy, int r,
                           tbos_rgba_t color);

/**
 * @brief Draw arc
 */
void tbos_draw_arc(tbos_pixbuf_t* buf, int cx, int cy, int r,
                   float start_angle, float end_angle, tbos_rgba_t color);

/* ═══════════════════════════════════════════════════════════════════════════
 * SACRED GEOMETRY (TBOS Special)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Draw Om symbol (ॐ)
 */
void tbos_draw_om(tbos_pixbuf_t* buf, int cx, int cy, int size, tbos_rgba_t color);

/**
 * @brief Draw Sri Yantra (sacred geometry)
 */
void tbos_draw_sri_yantra(tbos_pixbuf_t* buf, int cx, int cy, int size,
                          tbos_rgba_t color);

/**
 * @brief Draw Chakra symbol
 * @param chakra 0-6 (Muladhara to Sahasrara)
 */
void tbos_draw_chakra(tbos_pixbuf_t* buf, int cx, int cy, int size, int chakra);

/**
 * @brief Draw Ashoka Chakra (24-spoke wheel)
 */
void tbos_draw_ashoka_chakra(tbos_pixbuf_t* buf, int cx, int cy, int size,
                              tbos_rgba_t color);

/**
 * @brief Draw karma meter (arc with value)
 */
void tbos_draw_karma_meter(tbos_pixbuf_t* buf, int x, int y, int w, int h,
                           int karma, int max_karma);

/**
 * @brief Draw consciousness level indicator
 * @param level 0=dormant, 1=awakening, 2=aware, 3=compassionate, 4=enlightened
 */
void tbos_draw_consciousness(tbos_pixbuf_t* buf, int cx, int cy, int size,
                             int level);

/* ═══════════════════════════════════════════════════════════════════════════
 * TEXT RENDERING
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Built-in font (8x16 bitmap)
 */
typedef struct {
    const uint8_t* data;    /* Bitmap data */
    uint8_t width;          /* Character width */
    uint8_t height;         /* Character height */
    uint8_t first_char;     /* First character code */
    uint8_t last_char;      /* Last character code */
} tbos_font_t;

/* Default fonts */
extern const tbos_font_t tbos_font_8x16;
extern const tbos_font_t tbos_font_6x12;
extern const tbos_font_t tbos_font_5x8;   /* Tiny for embedded */

/**
 * @brief Draw character
 */
void tbos_draw_char(tbos_pixbuf_t* buf, int x, int y, char c,
                    const tbos_font_t* font, tbos_rgba_t color);

/**
 * @brief Draw string
 */
void tbos_draw_text(tbos_pixbuf_t* buf, int x, int y, const char* text,
                    const tbos_font_t* font, tbos_rgba_t color);

/**
 * @brief Measure text width
 */
int tbos_text_width(const char* text, const tbos_font_t* font);

/**
 * @brief Draw text centered
 */
void tbos_draw_text_centered(tbos_pixbuf_t* buf, int x, int y, int w,
                              const char* text, const tbos_font_t* font,
                              tbos_rgba_t color);

/* ═══════════════════════════════════════════════════════════════════════════
 * IMAGE OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Blit (copy) one buffer to another
 */
void tbos_pixbuf_blit(tbos_pixbuf_t* dst, int dx, int dy,
                      const tbos_pixbuf_t* src, int sx, int sy,
                      int w, int h);

/**
 * @brief Blit with alpha blending
 */
void tbos_pixbuf_blit_blend(tbos_pixbuf_t* dst, int dx, int dy,
                            const tbos_pixbuf_t* src, int sx, int sy,
                            int w, int h);

/**
 * @brief Scale buffer (nearest neighbor for speed)
 */
tbos_pixbuf_t* tbos_pixbuf_scale(const tbos_pixbuf_t* src,
                                  uint32_t new_width, uint32_t new_height);

/* ═══════════════════════════════════════════════════════════════════════════
 * PXFS INTEGRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Save pixel buffer to PXFS file format
 */
int tbos_pixbuf_save_pxfs(const tbos_pixbuf_t* buf, const char* path);

/**
 * @brief Load pixel buffer from PXFS file
 */
tbos_pixbuf_t* tbos_pixbuf_load_pxfs(const char* path);

/**
 * @brief Convert PXFS compressed data to pixel buffer
 */
tbos_pixbuf_t* tbos_pixbuf_from_pxfs(const uint8_t* pxfs_data, size_t len);

/**
 * @brief Convert pixel buffer to PXFS compressed data
 */
int tbos_pixbuf_to_pxfs(const tbos_pixbuf_t* buf, uint8_t* output,
                        size_t max_len, size_t* out_len);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_PIXEL_H */
