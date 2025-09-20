#ifndef KERNEL_H
#define KERNEL_H

// Kernel entry point
void kmain(void);

// VGA text mode functions
void vga_clear_screen(uint8_t color);
void vga_putchar(char c, uint8_t color, uint32_t x, uint32_t y);
void vga_print(const char *str, uint8_t color, uint32_t x, uint32_t y);

// Standard types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;

#endif // KERNEL_H
