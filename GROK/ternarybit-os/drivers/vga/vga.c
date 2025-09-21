// WINDSURF: Minimal VGA text-mode stub implementation
#include "vga.h"

static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static uint8_t current_color = 0x0F; // white on black
static uint16_t cursor = 0;

static uint16_t vga_entry(char c, uint8_t color) {
    return ((uint16_t)color << 8) | (uint16_t)c;
}

void vga_init(void) {
    vga_clear();
}

void vga_clear(void) {
    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        VGA_MEMORY[i] = vga_entry(' ', current_color);
    }
    cursor = 0;
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    current_color = (bg << 4) | (fg & 0x0F);
}

void vga_putc(char c) {
    if (c == '\n') {
        cursor += VGA_WIDTH - (cursor % VGA_WIDTH);
    } else {
        VGA_MEMORY[cursor++] = vga_entry(c, current_color);
    }
    if (cursor >= VGA_WIDTH * VGA_HEIGHT) {
        cursor = 0; // simple wrap for now
    }
}

void vga_write(const char* s) {
    while (*s) vga_putc(*s++);
}
