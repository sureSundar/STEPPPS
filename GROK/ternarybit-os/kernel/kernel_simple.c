// TBOS Simple Kernel - Sprint 11
// 🕉️ Swamiye Saranam Aiyappa 🕉️
// Minimal working kernel

#include <stdint.h>
#include <stddef.h>

// VGA buffer
#define VGA_BUFFER 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint16_t* vga = (uint16_t*)VGA_BUFFER;
static int cursor_x = 0;
static int cursor_y = 0;

// Print functions
void kernel_print(const char* str);
void kernel_print_hex(uint32_t value);
void kernel_clear(void);

// Module functions
void memory_init(void);
void interrupt_init(void);
void timer_init(uint32_t freq);
void steppps_init(void);
void steppps_status(void);

// Clear screen
void kernel_clear(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = 0x0720; // Space with gray on black
    }
    cursor_x = 0;
    cursor_y = 0;
}

// Print string
void kernel_print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            vga[cursor_y * VGA_WIDTH + cursor_x] = (0x07 << 8) | *str;
            cursor_x++;
            if (cursor_x >= VGA_WIDTH) {
                cursor_x = 0;
                cursor_y++;
            }
        }
        if (cursor_y >= VGA_HEIGHT) {
            // Simple scroll
            for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
                vga[i] = vga[i + VGA_WIDTH];
            }
            for (int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
                vga[i] = 0x0720;
            }
            cursor_y = VGA_HEIGHT - 1;
        }
        str++;
    }
}

// Print hex
void kernel_print_hex(uint32_t value) {
    char hex[] = "0x00000000";
    const char* digits = "0123456789ABCDEF";

    for (int i = 9; i >= 2; i--) {
        hex[i] = digits[value & 0xF];
        value >>= 4;
    }

    kernel_print(hex);
}

// Kernel entry point
void kernel_main(void) {
    // Clear screen
    kernel_clear();

    // Print header
    kernel_print("=====================================\n");
    kernel_print("       TBOS Kernel v1.0\n");
    kernel_print("    Swamiye Saranam Aiyappa\n");
    kernel_print("=====================================\n\n");

    // Initialize STEPPPS framework
    steppps_init();

    // Initialize memory
    kernel_print("[KERNEL] Memory management...\n");
    memory_init();

    // Initialize interrupts
    kernel_print("[KERNEL] Interrupt system...\n");
    interrupt_init();

    // Initialize timer
    kernel_print("[KERNEL] Timer configuration...\n");
    timer_init(100); // 100 Hz

    // Show STEPPPS status
    steppps_status();

    // Success
    kernel_print("\n[KERNEL] System ready!\n");
    kernel_print("[KERNEL] Sprint 12 Complete!\n");
    kernel_print("\nTBOS> ");

    // Halt
    while(1) {
        __asm__ volatile("hlt");
    }
}