// TEST_DEPS: drivers/vga/vga.c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../../drivers/vga/vga.h"

#define TEST_BUFFER_SIZE (VGA_WIDTH * VGA_HEIGHT)

static uint16_t test_buffer[TEST_BUFFER_SIZE];
static uint16_t bound_width = VGA_WIDTH;
static uint16_t bound_height = VGA_HEIGHT;

static inline size_t cell_index(uint16_t row, uint16_t col) {
    return (size_t)row * bound_width + col;
}

static inline char cell_char(uint16_t row, uint16_t col) {
    return (char)(test_buffer[cell_index(row, col)] & 0xFF);
}

static inline uint8_t cell_color(uint16_t row, uint16_t col) {
    return (uint8_t)(test_buffer[cell_index(row, col)] >> 8);
}

static void configure_buffer(uint16_t width, uint16_t height) {
    memset(test_buffer, 0xCD, sizeof(test_buffer));
    bound_width = width;
    bound_height = height;
    vga_bind_memory(test_buffer, width, height);
    vga_set_color(0x0F, 0x00);
    vga_clear();
}

static int test_clear_blanks_buffer(void) {
    configure_buffer(5, 3);
    for (uint16_t row = 0; row < bound_height; ++row) {
        for (uint16_t col = 0; col < bound_width; ++col) {
            if (cell_char(row, col) != ' ') {
                fprintf(stderr, "[test_vga] clear failed at (%u,%u): expected space, saw '%c'\n",
                        row, col, cell_char(row, col));
                return 1;
            }
            if (cell_color(row, col) != 0x0F) {
                fprintf(stderr, "[test_vga] clear color mismatch at (%u,%u): got 0x%02X\n",
                        row, col, cell_color(row, col));
                return 1;
            }
        }
    }
    return 0;
}

static int test_newline_moves_cursor(void) {
    configure_buffer(4, 4);
    vga_putc('A');
    vga_putc('\n');
    vga_putc('B');

    if (cell_char(0, 0) != 'A' || cell_char(1, 0) != 'B') {
        fprintf(stderr, "[test_vga] newline write mismatch\n");
        return 1;
    }

    uint16_t row = 0, col = 0;
    vga_get_cursor(&row, &col);
    if (row != 1 || col != 1) {
        fprintf(stderr, "[test_vga] newline cursor unexpected row=%u col=%u\n", row, col);
        return 1;
    }
    return 0;
}

static int test_scroll_discard_top_row(void) {
    configure_buffer(3, 2);
    const char* text = "ABCDEFG";
    for (size_t i = 0; text[i] != '\0'; ++i) {
        vga_putc(text[i]);
    }

    const char expected_row0[3] = {'D', 'E', 'F'};
    const char expected_row1[3] = {'G', ' ', ' '};

    for (uint16_t col = 0; col < bound_width; ++col) {
        if (cell_char(0, col) != expected_row0[col]) {
            fprintf(stderr, "[test_vga] scroll row0 mismatch at col %u: got '%c'\n",
                    col, cell_char(0, col));
            return 1;
        }
        if (cell_char(1, col) != expected_row1[col]) {
            fprintf(stderr, "[test_vga] scroll row1 mismatch at col %u: got '%c'\n",
                    col, cell_char(1, col));
            return 1;
        }
    }

    uint16_t row = 0, col = 0;
    vga_get_cursor(&row, &col);
    if (row != 1 || col != 1) {
        fprintf(stderr, "[test_vga] scroll cursor unexpected row=%u col=%u\n", row, col);
        return 1;
    }

    return 0;
}

int main(void) {
    int failed = 0;
    failed += test_clear_blanks_buffer();
    failed += test_newline_moves_cursor();
    failed += test_scroll_discard_top_row();

    if (failed == 0) {
        printf("[TEST] test_vga: OK\n");
    }
    return failed;
}
