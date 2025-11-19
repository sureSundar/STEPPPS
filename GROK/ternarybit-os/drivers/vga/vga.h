#ifndef TBOS_VGA_H
#define TBOS_VGA_H

#include <stdint.h>
#include <stdbool.h>

// WINDSURF: Minimal VGA interface (text mode first)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_write(const char* str);
void vga_set_color(uint8_t fg, uint8_t bg);

// WINDSURF: Helpers for unit testing / host binding
void vga_bind_memory(uint16_t* buffer, uint16_t width, uint16_t height);
void vga_set_cursor(uint16_t row, uint16_t col);
void vga_get_cursor(uint16_t* row, uint16_t* col);
void vga_enable_hw_cursor(bool enable);

#endif // TBOS_VGA_H
