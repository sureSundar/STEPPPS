// TernaryBit OS - Simple Kernel Test
// Testing function calls with -O0
// 🕉️ Swamiye Saranam Aiyappa 🕉️

#include <stdint.h>

#define VGA_MEMORY 0xB8000
volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;

// Stub implementations for libc dependencies
void kernel_putchar(char c) {
    (void)c;
}

void kernel_print(const char* str) {
    (void)str;
}

int vfs_write_file(const char* path, const void* data, int size) {
    (void)path; (void)data; (void)size;
    return -1;
}

char* vfs_read_file_cstr(const char* path) {
    (void)path;
    return 0;
}

int vfs_exists(const char* path) {
    (void)path;
    return 0;
}

int vfs_type(const char* path) {
    (void)path;
    return 0;
}

int vfs_list_dir(const char* path, char*** out_files) {
    (void)path; (void)out_files;
    return 0;
}

static inline void serial_outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void test_function() {
    serial_outb(0x3F8, 'T');
    serial_outb(0x3F8, 'E');
    serial_outb(0x3F8, 'S');
    serial_outb(0x3F8, 'T');
    serial_outb(0x3F8, '\n');
}

void kernel_main(void) {
    // Test serial
    serial_outb(0x3F8, '[');
    serial_outb(0x3F8, 'M');
    serial_outb(0x3F8, 'A');
    serial_outb(0x3F8, 'I');
    serial_outb(0x3F8, 'N');
    serial_outb(0x3F8, ']');
    serial_outb(0x3F8, '\n');

    // Test function call
    test_function();

    // Test VGA
    const char* msg = "SIMPLE KERNEL - FUNCTION CALLS WORK!";
    for (int i = 0; msg[i] != '\0'; i++) {
        vga[i] = 0x0A00 | msg[i];  // Green text
    }

    serial_outb(0x3F8, '[');
    serial_outb(0x3F8, 'D');
    serial_outb(0x3F8, 'O');
    serial_outb(0x3F8, 'N');
    serial_outb(0x3F8, 'E');
    serial_outb(0x3F8, ']');
    serial_outb(0x3F8, '\n');

    // Halt
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}
