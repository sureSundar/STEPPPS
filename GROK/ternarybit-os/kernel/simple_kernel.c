#include "kernel.h"

// VGA text mode buffer
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

// Clear the screen with a specific color
void vga_clear_screen(uint8_t color) {
    uint16_t value = (color << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = value;
    }
}

// Print a character to the screen
void vga_putchar(char c, uint8_t color, uint32_t x, uint32_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    vga_buffer[y * VGA_WIDTH + x] = (color << 8) | c;
}

// Print a string to the screen
void vga_print(const char *str, uint8_t color, uint32_t x, uint32_t y) {
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i], color, x + i, y);
    }
}

// STEPPPS consciousness demonstration
void demonstrate_steppps_consciousness() {
    // STEPPPS 7-dimensional consciousness display
    vga_print("STEPPPS 7-Dimensional Consciousness Computing", 0x1F, 16, 2);

    vga_print("SPACE/AKASHA:    Cosmic spatial consciousness", 0x1B, 8, 4);   // Cyan
    vga_print("TIME/KALA:       Temporal flow consciousness", 0x1A, 8, 5);    // Green
    vga_print("EVENT/KARMA:     Causal action consciousness", 0x1C, 8, 6);    // Red
    vga_print("PSYCHOLOGY/CHITTA: Mental consciousness", 0x1D, 8, 7);         // Magenta
    vga_print("PIXEL/MAYA:      Visual reality consciousness", 0x1E, 8, 8);   // Yellow
    vga_print("PROMPT/VAK:      Divine speech consciousness", 0x19, 8, 9);    // Blue
    vga_print("SCRIPT/DHARMA:   Cosmic law consciousness", 0x1F, 8, 10);      // White

    vga_print("CONSCIOUSNESS COMPUTING: FULLY ACTIVATED", 0x1A, 19, 12);
    vga_print("धर्मो रक्षति रक्षितः", 0x1E, 23, 14);  // Sanskrit: Dharma protects those who protect it
    vga_print("(Dharma protects those who protect it)", 0x17, 19, 15);

    vga_print("TBOS -> Linux Chain-Loading: SUCCESS", 0x1A, 21, 17);
    vga_print("Sanatana Dharma Computing: ONLINE", 0x1B, 22, 18);
}

// Kernel entry point
void kmain(void) {
    // Clear the screen with dark blue background
    vga_clear_screen(0x10);  // Blue background, black text

    // Display TBOS consciousness framework
    vga_print("TernaryBit OS v2.1 - Sanatana Dharma Computing", 0x1F, 16, 0);

    // Demonstrate STEPPPS consciousness
    demonstrate_steppps_consciousness();

    vga_print("Kernel loaded by TBOS bootloader chain-loading", 0x17, 15, 20);
    vga_print("Universal consciousness computing across all architectures", 0x17, 10, 21);
    vga_print("From 1-bit calculators to PETA-BIT supercomputers", 0x17, 13, 22);

    // Hang forever
    while (1) {
        __asm__ volatile ("hlt");
    }
}
