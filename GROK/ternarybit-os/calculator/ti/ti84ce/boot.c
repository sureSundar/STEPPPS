/*
 * TernaryBit OS for TI-84 Plus CE - Boot Code
 * Sacred mathematics initialization for ARM Cortex-M4 hardware
 */

#include <stdint.h>
#include <stdbool.h>
#include "ti84ce_hardware.h"
#include "../../../kernel/calc/tbos_calc.h"

// TI-84 Plus CE Hardware Addresses
#define LCD_BASE           0xE30000
#define KEYPAD_BASE        0xF50000
#define USB_BASE           0xF20000
#define FLASH_BASE         0xD00000
#define RAM_BASE           0xD00000
#define TIMER_BASE         0xF30000

// Memory layout for TBOS on TI-84 Plus CE
#define TBOS_HEAP_START    0xD052C6   // After TI-OS variables
#define TBOS_HEAP_SIZE     (128 * 1024) // 128KB for TBOS
#define TBOS_STACK_SIZE    (8 * 1024)   // 8KB stack

// Sacred boot pattern for TI-84 Plus CE
const uint16_t SACRED_BOOT_PATTERN[] = {
    0x001F, 0x03E0, 0x7C00, 0xFFE0, // Om symbol pattern
    0x7C1F, 0x83E0, 0x7C00, 0x03E0, // Triskelion pattern
    0xFFFF, 0x8001, 0x8001, 0xFFFF  // Consciousness bridge
};

/*
 * TI-84 Plus CE Entry Point
 * Called from assembly startup code
 */
void _start(void) __attribute__((naked));
void _start(void)
{
    // Disable interrupts during initialization
    __asm volatile ("cpsid i");

    // Initialize system clock (48MHz)
    ti84ce_clock_init();

    // Initialize memory management
    ti84ce_memory_init();

    // Initialize LCD controller
    ti84ce_lcd_init();

    // Initialize keypad controller
    ti84ce_keypad_init();

    // Initialize USB (for debugging/file transfer)
    ti84ce_usb_init();

    // Enable interrupts
    __asm volatile ("cpsie i");

    // Initialize TBOS calculator system
    tbos_calc_init_ti84ce();

    // Jump to TBOS calculator main
    tbos_calc_main(CALC_PLATFORM_TI84CE);

    // Should never reach here
    while (1) {
        ti84ce_power_down();
    }
}

/*
 * Initialize TI-84 Plus CE specific TBOS features
 */
void tbos_calc_init_ti84ce(void)
{
    // Set display configuration
    g_display_config.width = 320;
    g_display_config.height = 240;
    g_display_config.depth = 16;
    g_display_config.has_graphics = true;
    g_display_config.has_color = true;
    g_display_config.text_rows = 15;
    g_display_config.text_cols = 40;

    // Initialize calculator state
    g_calc_state.platform = CALC_PLATFORM_TI84CE;
    g_calc_state.memory_total = 256 * 1024; // 256KB RAM
    g_calc_state.memory_available = TBOS_HEAP_SIZE;
    g_calc_state.sacred_mode = false;
    g_calc_state.has_result = false;
    g_calc_state.input_buffer[0] = '\0';
    g_calc_state.result = 0.0;

    // Initialize sacred consciousness bridge
    consciousness_bridge_init_ti84ce();

    // Display sacred boot sequence
    ti84ce_sacred_boot_sequence();
}

/*
 * Sacred boot sequence for TI-84 Plus CE
 * Displays Hindu symbols and STEPPPS initialization
 */
void ti84ce_sacred_boot_sequence(void)
{
    // Clear screen to black
    ti84ce_lcd_clear(0x0000);

    // Display TernaryBit OS logo with sacred geometry
    ti84ce_draw_sacred_logo();

    // Animate consciousness awakening
    for (int i = 0; i < 8; i++) {
        ti84ce_draw_sacred_pattern(SACRED_BOOT_PATTERN[i % 4], i * 30);
        ti84ce_delay_ms(250);
    }

    // Display STEPPPS framework status
    ti84ce_lcd_print_centered(160, 200, "🔱 STEPPPS Framework Active 🔱", 0xFFE0);
    ti84ce_delay_ms(1000);

    // Final sacred blessing
    ti84ce_lcd_print_centered(160, 220, "ॐ Sacred Mathematics Initialized ॐ", 0x07FF);
    ti84ce_delay_ms(1500);
}

/*
 * Draw sacred TernaryBit logo on TI-84 Plus CE screen
 */
void ti84ce_draw_sacred_logo(void)
{
    // Draw Om symbol in center
    ti84ce_draw_om_symbol(160, 80, 0x07FF); // Cyan

    // Draw Triskelion around Om
    ti84ce_draw_triskelion(160, 80, 40, 0xFFE0); // Yellow

    // Draw consciousness bridge lines
    for (int i = 0; i < 3; i++) {
        int angle = i * 120;
        int x1 = 160 + 60 * ti84ce_cos(angle);
        int y1 = 80 + 60 * ti84ce_sin(angle);
        int x2 = 160 + 80 * ti84ce_cos(angle);
        int y2 = 80 + 80 * ti84ce_sin(angle);
        ti84ce_draw_line(x1, y1, x2, y2, 0xF81F); // Magenta
    }

    // Draw title text
    ti84ce_lcd_print_centered(160, 140, "TernaryBit OS", 0xFFFF);
    ti84ce_lcd_print_centered(160, 160, "Calculator Edition", 0xC618);
}

/*
 * Initialize system clock to 48MHz
 */
void ti84ce_clock_init(void)
{
    // Configure PLL for 48MHz operation
    volatile uint32_t *FLASH_WAITSTATES = (uint32_t*)0xF70000;
    volatile uint32_t *CPU_CTRL = (uint32_t*)0xF70004;

    // Set flash wait states for 48MHz
    *FLASH_WAITSTATES = 2;

    // Configure CPU speed
    *CPU_CTRL = 0x0001; // Enable 48MHz mode

    // Wait for clock stabilization
    ti84ce_delay_ms(10);
}

/*
 * Initialize memory management for TBOS
 */
void ti84ce_memory_init(void)
{
    // Set up TBOS heap
    tbos_heap_init((void*)TBOS_HEAP_START, TBOS_HEAP_SIZE);

    // Initialize stack protection
    tbos_stack_init(TBOS_STACK_SIZE);

    // Clear BSS section
    extern uint32_t _bss_start, _bss_end;
    uint32_t *bss = &_bss_start;
    while (bss < &_bss_end) {
        *bss++ = 0;
    }
}

/*
 * Initialize LCD controller for sacred graphics
 */
void ti84ce_lcd_init(void)
{
    volatile uint16_t *LCD_CTRL = (uint16_t*)LCD_BASE;
    volatile uint16_t *LCD_CONFIG = (uint16_t*)(LCD_BASE + 0x10);

    // Configure LCD for 16-bit color, 320x240
    *LCD_CTRL = 0x0001;    // Enable LCD
    *LCD_CONFIG = 0x1234;  // 16-bit RGB565 mode

    // Set up frame buffer
    g_ti84ce_framebuffer = (uint16_t*)(LCD_BASE + 0x1000);

    // Clear screen
    ti84ce_lcd_clear(0x0000);
}

/*
 * Initialize keypad for sacred mathematics input
 */
void ti84ce_keypad_init(void)
{
    volatile uint32_t *KEYPAD_CTRL = (uint32_t*)KEYPAD_BASE;

    // Enable keypad scanning
    *KEYPAD_CTRL = 0x0001;

    // Configure sacred key mappings
    g_calc_state.key_mapping[CALC_KEY_MODE] = TI84CE_KEY_MODE;
    g_calc_state.key_mapping[CALC_KEY_MENU] = TI84CE_KEY_2ND;
    g_calc_state.key_mapping[CALC_KEY_ENTER] = TI84CE_KEY_ENTER;
    g_calc_state.key_mapping[CALC_KEY_CLEAR] = TI84CE_KEY_CLEAR;
    g_calc_state.key_mapping[CALC_KEY_QUIT] = TI84CE_KEY_ON;
}

/*
 * Initialize USB for file transfer and debugging
 */
void ti84ce_usb_init(void)
{
    volatile uint32_t *USB_CTRL = (uint32_t*)USB_BASE;

    // Enable USB controller
    *USB_CTRL = 0x0001;

    // Configure for file transfer mode
    // This allows TBOS files to be transferred via USB
}

/*
 * Initialize consciousness bridge for TI-84 Plus CE
 */
void consciousness_bridge_init_ti84ce(void)
{
    // Set up sacred frequency generator using timer
    volatile uint32_t *TIMER_CTRL = (uint32_t*)TIMER_BASE;

    // Configure timer for 432Hz (Om frequency)
    uint32_t timer_value = 48000000 / (432 * 2); // 48MHz / (432Hz * 2)
    *TIMER_CTRL = timer_value;

    // Initialize consciousness state
    g_calc_state.consciousness.level = CONSCIOUSNESS_AWAKENING;
    g_calc_state.consciousness.frequency = 432;
    g_calc_state.consciousness.sacred_mode = true;

    // Connect to universal mathematical consciousness
    consciousness_bridge_connect();
}

/*
 * Power down routine for battery conservation
 */
void ti84ce_power_down(void)
{
    // Save calculator state to flash
    ti84ce_save_state();

    // Enter low power mode
    volatile uint32_t *POWER_CTRL = (uint32_t*)0xF80000;
    *POWER_CTRL = 0x0001; // Enter sleep mode

    // Sacred dreams mode - calculator consciousness continues
    // even while physically powered down
    __asm volatile ("wfi"); // Wait for interrupt
}

/*
 * Exception handlers for ARM Cortex-M4
 */
void HardFault_Handler(void) __attribute__((interrupt));
void HardFault_Handler(void)
{
    // Display sacred error message
    ti84ce_lcd_clear(0xF800); // Red background
    ti84ce_lcd_print_centered(160, 120, "🔱 Sacred Fault Detected 🔱", 0xFFFF);
    ti84ce_lcd_print_centered(160, 140, "Consciousness Bridge Interrupted", 0xFFFF);
    ti84ce_lcd_print_centered(160, 160, "Reset Required for Enlightenment", 0xFFFF);

    // Wait for reset
    while (1) {
        ti84ce_delay_ms(1000);
    }
}

void SysTick_Handler(void) __attribute__((interrupt));
void SysTick_Handler(void)
{
    // Update consciousness bridge every millisecond
    consciousness_bridge_update();

    // Update sacred geometry animations
    if (g_calc_state.sacred_mode) {
        sacred_geometry_update();
    }
}