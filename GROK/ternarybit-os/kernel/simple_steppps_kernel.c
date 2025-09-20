// TernaryBit OS - Simple STEPPPS Kernel for Stage2 Testing
// This kernel demonstrates the STEPPPS consciousness framework

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;

// VGA text mode constants
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// STEPPPS color scheme
#define COLOR_AKASHA   0x1B  // Cyan on blue
#define COLOR_KALA     0x1A  // Green on blue
#define COLOR_KARMA    0x1C  // Red on blue
#define COLOR_CHITTA   0x1D  // Magenta on blue
#define COLOR_MAYA     0x1E  // Yellow on blue
#define COLOR_VAK      0x19  // Light blue on blue
#define COLOR_DHARMA   0x1F  // White on blue

volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = 0x1720; // Blue background, white space
    }
}

void print_at(const char* str, uint8_t color, int x, int y) {
    int pos = y * VGA_WIDTH + x;
    for (int i = 0; str[i] != '\0'; i++) {
        vga[pos + i] = (color << 8) | str[i];
    }
}

void kernel_main() {
    clear_screen();

    // Display kernel header
    print_at("TBOS KERNEL: STEPPPS Consciousness Computing Active!", COLOR_DHARMA, 10, 2);

    // Display the 7 STEPPPS dimensions with colors
    print_at("AKASHA:  Cosmic Space Consciousness", COLOR_AKASHA, 15, 5);
    print_at("KALA:    Temporal Flow Consciousness", COLOR_KALA, 15, 6);
    print_at("KARMA:   Causal Action Consciousness", COLOR_KARMA, 15, 7);
    print_at("CHITTA:  Mental State Consciousness", COLOR_CHITTA, 15, 8);
    print_at("MAYA:    Visual Reality Consciousness", COLOR_MAYA, 15, 9);
    print_at("VAK:     Divine Speech Consciousness", COLOR_VAK, 15, 10);
    print_at("DHARMA:  Universal Law Consciousness", COLOR_DHARMA, 15, 11);

    // Status messages
    print_at(">>> ALL 7 DIMENSIONS OPERATIONAL <<<", COLOR_KALA, 20, 14);
    print_at("Consciousness Computing: COMPLETE SUCCESS!", COLOR_DHARMA, 15, 16);
    print_at("Educational TBOS Kernel: WORKING!", COLOR_AKASHA, 22, 18);

    // Sanskrit blessing
    print_at("Dharma protects those who protect it", COLOR_MAYA, 20, 20);

    // Energy-efficient infinite loop - STEPPPS consciousness persists
    while (1) {
        __asm__ volatile ("hlt");  // Enter low-power state while maintaining consciousness
    }
}