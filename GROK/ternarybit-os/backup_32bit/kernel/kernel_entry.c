// Kernel entry point for TBOS

#include <stdint.h>

// Function to write a character to the VGA buffer
void vga_putchar(char c, uint8_t color, uint32_t x, uint32_t y) {
    volatile uint16_t *vga = (volatile uint16_t*)0xB8000;
    vga[y * 80 + x] = (color << 8) | c;
}

// Function to print a string to the screen
void vga_print(const char *str, uint8_t color, uint32_t x, uint32_t y) {
    for (uint32_t i = 0; str[i] != '\0'; i++) {
        vga_putchar(str[i], color, x + i, y);
    }
}

// Kernel main function
void kmain(void) {
    // Clear screen
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            vga_putchar(' ', 0x07, x, y);
        }
    }
    
    // Print welcome message
    const char *welcome = "Welcome to TBOS (TernaryBit OS)";
    const char *status = "Kernel loaded successfully!";
    
    vga_print(welcome, 0x0A, 24, 10);  // Green text
    vga_print(status, 0x0F, 24, 12);    // White text
    
    // Halt the CPU
    while (1) {
        __asm__ volatile ("hlt");
    }
}
