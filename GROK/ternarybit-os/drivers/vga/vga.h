#ifndef TBOS_VGA_H
#define TBOS_VGA_H

#include <stdint.h>

// WINDSURF: Minimal VGA interface (text mode first)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_write(const char* str);
void vga_set_color(uint8_t fg, uint8_t bg);

#endif // TBOS_VGA_H
