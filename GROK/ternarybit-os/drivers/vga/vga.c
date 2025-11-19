// WINDSURF: Text-mode VGA driver with host-test hooks
#include "vga.h"
#include <stddef.h>

#define VGA_TAB_WIDTH 4
#define VGA_CURSOR_PORT_CMD 0x3D4
#define VGA_CURSOR_PORT_DATA 0x3D5

static uint16_t* g_vga_memory = (uint16_t*)0xB8000;
static uint16_t g_vga_width = VGA_WIDTH;
static uint16_t g_vga_height = VGA_HEIGHT;
static uint8_t g_current_color = 0x0F; // white on black
static uint16_t g_cursor_row = 0;
static uint16_t g_cursor_col = 0;
static bool g_hw_cursor_enabled = false;

#if defined(__i386__) || defined(__x86_64__)
static inline void vga_outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}
#else
static inline void vga_outb(uint16_t port, uint8_t value) {
    (void)port;
    (void)value;
}
#endif

static inline size_t vga_cell_count(void) {
    return (size_t)g_vga_width * (size_t)g_vga_height;
}

static inline uint16_t vga_entry(char c) {
    return ((uint16_t)g_current_color << 8) | (uint8_t)c;
}

static void vga_sync_hw_cursor(void) {
    if (!g_hw_cursor_enabled) {
        return;
    }
    uint16_t position = (uint16_t)(g_cursor_row * g_vga_width + g_cursor_col);
    vga_outb(VGA_CURSOR_PORT_CMD, 0x0F);
    vga_outb(VGA_CURSOR_PORT_DATA, (uint8_t)(position & 0xFF));
    vga_outb(VGA_CURSOR_PORT_CMD, 0x0E);
    vga_outb(VGA_CURSOR_PORT_DATA, (uint8_t)((position >> 8) & 0xFF));
}

static void vga_ensure_dimensions(void) {
    if (g_vga_width == 0) {
        g_vga_width = 1;
    }
    if (g_vga_height == 0) {
        g_vga_height = 1;
    }
}

void vga_bind_memory(uint16_t* buffer, uint16_t width, uint16_t height) {
    g_vga_memory = buffer ? buffer : (uint16_t*)0xB8000;
    g_vga_width = width ? width : VGA_WIDTH;
    g_vga_height = height ? height : VGA_HEIGHT;
    g_cursor_row = 0;
    g_cursor_col = 0;
    vga_ensure_dimensions();
    vga_sync_hw_cursor();
}

void vga_enable_hw_cursor(bool enable) {
    g_hw_cursor_enabled = enable;
    vga_sync_hw_cursor();
}

void vga_get_cursor(uint16_t* row, uint16_t* col) {
    if (row) *row = g_cursor_row;
    if (col) *col = g_cursor_col;
}

void vga_set_cursor(uint16_t row, uint16_t col) {
    if (row >= g_vga_height) {
        row = (g_vga_height > 0) ? (g_vga_height - 1) : 0;
    }
    if (col >= g_vga_width) {
        col = (g_vga_width > 0) ? (g_vga_width - 1) : 0;
    }
    g_cursor_row = row;
    g_cursor_col = col;
    vga_sync_hw_cursor();
}

static void vga_scroll(void) {
    if (!g_vga_memory) {
        return;
    }

    vga_ensure_dimensions();

    if (g_vga_height <= 1 || g_vga_width == 0) {
        g_cursor_row = 0;
        g_cursor_col = 0;
        vga_sync_hw_cursor();
        return;
    }

    size_t row_cells = g_vga_width;
    for (uint16_t row = 1; row < g_vga_height; ++row) {
        size_t dst = (size_t)(row - 1) * row_cells;
        size_t src = (size_t)row * row_cells;
        for (size_t col = 0; col < row_cells; ++col) {
            g_vga_memory[dst + col] = g_vga_memory[src + col];
        }
    }

    size_t last_row = (size_t)(g_vga_height - 1) * row_cells;
    for (size_t col = 0; col < row_cells; ++col) {
        g_vga_memory[last_row + col] = vga_entry(' ');
    }

    g_cursor_row = g_vga_height - 1;
    g_cursor_col = 0;
    vga_sync_hw_cursor();
}

static void vga_advance_cursor(void) {
    if (++g_cursor_col >= g_vga_width) {
        g_cursor_col = 0;
        if (g_cursor_row + 1 >= g_vga_height) {
            vga_scroll();
            return;
        }
        g_cursor_row++;
    }
    if (g_cursor_row >= g_vga_height) {
        vga_scroll();
        return;
    }
    vga_sync_hw_cursor();
}

static void vga_write_char(char c) {
    if (!g_vga_memory) {
        return;
    }
    size_t idx = (size_t)g_cursor_row * g_vga_width + g_cursor_col;
    if (idx < vga_cell_count()) {
        g_vga_memory[idx] = vga_entry(c);
    }
    vga_advance_cursor();
}

static void vga_newline(void) {
    g_cursor_col = 0;
    if (g_cursor_row + 1 >= g_vga_height) {
        vga_scroll();
    } else {
        g_cursor_row++;
        vga_sync_hw_cursor();
    }
}

static void vga_backspace(void) {
    if (g_cursor_col == 0 && g_cursor_row == 0) {
        return;
    }
    if (g_cursor_col == 0) {
        g_cursor_row--;
        g_cursor_col = g_vga_width - 1;
    } else {
        g_cursor_col--;
    }
    if (g_vga_memory) {
        size_t idx = (size_t)g_cursor_row * g_vga_width + g_cursor_col;
        if (idx < vga_cell_count()) {
            g_vga_memory[idx] = vga_entry(' ');
        }
    }
    vga_sync_hw_cursor();
}

void vga_init(void) {
    vga_bind_memory(NULL, VGA_WIDTH, VGA_HEIGHT);
    vga_set_color(0x0F, 0x00);
    vga_clear();
}

void vga_clear(void) {
    if (!g_vga_memory) {
        return;
    }

    size_t total = vga_cell_count();
    for (size_t i = 0; i < total; ++i) {
        g_vga_memory[i] = vga_entry(' ');
    }
    g_cursor_row = 0;
    g_cursor_col = 0;
    vga_sync_hw_cursor();
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    g_current_color = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void vga_putc(char c) {
    switch (c) {
        case '\n':
            vga_newline();
            break;
        case '\r':
            g_cursor_col = 0;
            vga_sync_hw_cursor();
            break;
        case '\t': {
            uint16_t next_tab = (uint16_t)(((g_cursor_col / VGA_TAB_WIDTH) + 1) * VGA_TAB_WIDTH);
            while (g_cursor_col < next_tab) {
                vga_write_char(' ');
            }
            break;
        }
        case '\b':
            vga_backspace();
            break;
        default:
            vga_write_char(c);
            break;
    }
}

void vga_write(const char* s) {
    if (!s) {
        return;
    }
    while (*s) {
        vga_putc(*s++);
    }
}
