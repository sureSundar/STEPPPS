/*
 * TernaryBit OS - Support Functions
 * Basic kernel utilities and hardware abstraction
 */

#include "kernel.h"
#include <stdarg.h>

// VGA text mode constants
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

// Terminal state
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

// VGA color enum
enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

// Terminal functions
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = (uint16_t*) VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_scroll(void) {
    // Move all lines up
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }

    // Clear last line
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    } else {
        terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
            }
        }
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Simple printf implementation with basic format specifiers
void kernel_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    for (size_t i = 0; format[i]; ++i) {
        if (format[i] != '%') {
            terminal_putchar(format[i]);
            continue;
        }

        ++i; // Skip '%'
        char spec = format[i];
        switch (spec) {
            case 'd':
            case 'i': {
                int value = va_arg(args, int);
                char buf[16];
                int_to_string(value, buf);
                terminal_writestring(buf);
                break;
            }
            case 'x': {
                u32 value = va_arg(args, u32);
                char buf[16];
                hex32_to_string(value, buf);
                terminal_writestring(buf);
                break;
            }
            case 's': {
                const char* str = va_arg(args, const char*);
                terminal_writestring(str ? str : "(null)");
                break;
            }
            case 'f': {
                double value = va_arg(args, double); // default promotion
                int whole = (int)value;
                int frac = (int)((value - whole) * 10.0);
                char buf[16];
                int_to_string(whole, buf);
                terminal_writestring(buf);
                terminal_putchar('.');
                if (frac < 0) frac = -frac;
                char frac_buf[4];
                int_to_string(frac, frac_buf);
                terminal_writestring(frac_buf);
                break;
            }
            case '%': {
                terminal_putchar('%');
                break;
            }
            default: {
                terminal_putchar('%');
                terminal_putchar(spec);
                break;
            }
        }
    }

    va_end(args);
}

// String functions
size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

int strcmp(const char* str1, const char* str2) {
    while (*str1 && (*str1 == *str2)) {
        str1++;
        str2++;
    }
    return *(unsigned char*)str1 - *(unsigned char*)str2;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;

    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (!*n) return (char*)haystack;
    }

    return NULL;
}

void int_to_string(int value, char* str) {
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }

    int is_negative = 0;
    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    int i = 0;
    while (value > 0) {
        str[i++] = '0' + (value % 10);
        value /= 10;
    }

    if (is_negative) {
        str[i++] = '-';
    }

    str[i] = '\0';

    // Reverse string
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

void hex32_to_string(u32 value, char* str) {
    static const char digits[] = "0123456789ABCDEF";
    str[0] = '0';
    str[1] = 'x';
    for (int i = 0; i < 8; ++i) {
        int shift = (7 - i) * 4;
        str[2 + i] = digits[(value >> shift) & 0xF];
    }
    str[10] = '\0';
}

void hex64_to_string(u64 value, char* str) {
    static const char digits[] = "0123456789ABCDEF";
    str[0] = '0';
    str[1] = 'x';
    for (int i = 0; i < 16; ++i) {
        int shift = (15 - i) * 4;
        str[2 + i] = digits[(value >> shift) & 0xF];
    }
    str[18] = '\0';
}

// Hardware detection functions
int detect_cpu_count(void) {
    // Simplified CPU detection
    return 1;  // Single CPU for now
}

u32 detect_memory_size(void) {
    if (g_boot_descriptor.valid && g_boot_descriptor.total_memory_kb > 0) {
        return g_boot_descriptor.total_memory_kb;
    }
    return 128 * 1024;  // Fallback
}

int detect_hardware_devices(void) {
    // Simplified device detection
    return 5;  // Keyboard, VGA, Timer, PIC, Disk
}

// Input functions
void read_command(char* buffer) {
    // Simplified command reading
    // In real implementation, would read from keyboard buffer

    // For now, simulate some commands
    static int command_counter = 0;
    static const char* demo_commands[] = {
        "help",
        "steppps",
        "bootinfo",
        "stats",
        "ai",
        "hello world",
        "optimize system",
        "reboot"
    };

    const char* cmd = demo_commands[command_counter % 7];
    command_counter++;

    // Copy command to buffer
    int i = 0;
    while (cmd[i] && i < 255) {
        buffer[i] = cmd[i];
        i++;
    }
    buffer[i] = '\0';

    // Display the command as if typed
    terminal_writestring(buffer);
    terminal_putchar('\n');
}

// System initialization functions
void init_memory_management(void) {
    // Initialize basic memory management
    // In real implementation, would set up paging, heap, etc.
}

void init_interrupt_handling(void) {
    // Initialize interrupt descriptor table
    // In real implementation, would set up IDT, PIC, etc.
}

void init_timer(void) {
    // Initialize system timer
    // In real implementation, would program PIT or APIC timer
}
