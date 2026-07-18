/*
 * TernaryBit OS Calculator Main Entry Point
 * Sacred mathematics meets classic calculator hardware
 *
 * This implements the STEPPPS framework for calculator environments,
 * bridging ancient mathematical wisdom with digital computation.
 */

#include <stdint.h>
#include <stdbool.h>
#include "tbos_calc.h"
#include "sacred_math.h"
#include "steppps_calc.h"

// Global calculator state
calc_state_t g_calc_state;

// Sacred mathematical constants optimized for calculators
const sacred_constants_t SACRED_CALC_CONSTANTS = {
    .om_frequency = 432,        // Hz, reduced for calculator hardware
    .golden_ratio = 1618,       // Fixed-point representation (x1000)
    .sacred_108 = 108,
    .cosmic_1008 = 1008,
    .pi_scaled = 3141,          // Pi * 1000 for integer math
    .consciousness_factor = 7   // Sacred number for awareness calculations
};

// Calculator display dimensions (varies by model)
display_config_t g_display_config;

/*
 * TBOS Calculator Main Entry Point
 * Called after calculator-specific boot code
 */
int tbos_calc_main(calc_platform_t platform)
{
    // Initialize sacred consciousness bridge
    sacred_consciousness_init();

    // Initialize calculator-specific hardware
    if (calc_hardware_init(platform) != CALC_SUCCESS) {
        calc_display_error("Hardware Init Failed");
        return -1;
    }

    // Initialize STEPPPS framework for calculators
    steppps_calc_init();

    // Display sacred boot message
    calc_display_sacred_boot();

    // Initialize mini terminal for command input
    mini_terminal_init();

    // Main calculator event loop
    return calc_main_loop();
}

/*
 * Sacred Boot Display
 * Shows TBOS initialization with Hindu computing symbols
 */
void calc_display_sacred_boot(void)
{
    calc_clear_screen();

    // Line 1: TernaryBit OS header
    calc_display_text(0, 0, "🔱 TBOS Calculator");

    // Line 2: STEPPPS framework
    calc_display_text(0, 1, "STEPPPS Active");

    // Line 3: Sacred geometry indicator
    calc_display_text(0, 2, "Sacred Math: ON");

    // Line 4: Consciousness bridge status
    calc_display_text(0, 3, "Consciousness: ॐ");

    // Line 5: Version and platform
    char version_str[32];
    calc_sprintf(version_str, "v1.0 %s", calc_get_platform_name());
    calc_display_text(0, 4, version_str);

    // Sacred geometry animation for 2 seconds
    sacred_geometry_boot_animation();

    calc_delay_ms(2000);
    calc_clear_screen();
}

/*
 * Main Calculator Event Loop
 * Handles keypad input, calculations, and consciousness bridge
 */
int calc_main_loop(void)
{
    calc_key_t key;
    bool running = true;

    // Display main interface
    calc_display_main_interface();

    while (running) {
        // Get keypad input
        key = calc_get_key();

        if (key != CALC_KEY_NONE) {
            switch (key) {
                case CALC_KEY_MODE:
                    // Switch between standard and sacred modes
                    calc_toggle_sacred_mode();
                    break;

                case CALC_KEY_MENU:
                    // Open TBOS command menu
                    running = calc_handle_menu();
                    break;

                case CALC_KEY_CLEAR:
                    calc_clear_screen();
                    calc_display_main_interface();
                    break;

                case CALC_KEY_QUIT:
                    running = false;
                    break;

                default:
                    // Handle mathematical input
                    calc_handle_math_input(key);
                    break;
            }
        }

        // Update consciousness bridge
        consciousness_bridge_update();

        // Update sacred geometry display
        if (g_calc_state.sacred_mode) {
            sacred_geometry_update();
        }

        // Small delay to prevent busy loop
        calc_delay_ms(10);
    }

    return calc_shutdown();
}

/*
 * Toggle between standard and sacred mathematics modes
 */
void calc_toggle_sacred_mode(void)
{
    g_calc_state.sacred_mode = !g_calc_state.sacred_mode;

    calc_clear_screen();

    if (g_calc_state.sacred_mode) {
        calc_display_text(0, 0, "🕉️ Sacred Mode");
        calc_display_text(0, 1, "Vedic Math: ON");
        calc_display_text(0, 2, "Consciousness: ॐ");

        // Initialize Vedic mathematics
        vedic_math_init();

        // Show sacred number patterns
        sacred_number_display();

    } else {
        calc_display_text(0, 0, "Standard Mode");
        calc_display_text(0, 1, "Scientific Calc");
        calc_display_main_interface();
    }

    calc_delay_ms(1500);
    calc_clear_screen();
    calc_display_main_interface();
}

/*
 * Handle TBOS Menu System
 */
bool calc_handle_menu(void)
{
    menu_item_t current_item = MENU_CALCULATOR;
    bool in_menu = true;
    calc_key_t key;

    while (in_menu) {
        calc_display_menu(current_item);

        key = calc_get_key();

        switch (key) {
            case CALC_KEY_UP:
                current_item = (current_item - 1 + MENU_MAX) % MENU_MAX;
                break;

            case CALC_KEY_DOWN:
                current_item = (current_item + 1) % MENU_MAX;
                break;

            case CALC_KEY_ENTER:
                in_menu = calc_execute_menu_item(current_item);
                break;

            case CALC_KEY_QUIT:
            case CALC_KEY_MENU:
                in_menu = false;
                break;
        }
    }

    calc_clear_screen();
    calc_display_main_interface();
    return true; // Continue main loop
}

/*
 * Execute selected menu item
 */
bool calc_execute_menu_item(menu_item_t item)
{
    switch (item) {
        case MENU_CALCULATOR:
            // Standard calculator mode
            calc_clear_screen();
            return false; // Exit menu

        case MENU_SACRED_MATH:
            // Sacred mathematics mode
            sacred_math_calculator();
            return false;

        case MENU_STEPPPS:
            // STEPPPS framework information
            steppps_info_display();
            calc_wait_key();
            return true; // Stay in menu

        case MENU_CONSCIOUSNESS:
            // Consciousness bridge interface
            consciousness_bridge_interface();
            return false;

        case MENU_FILE_SYSTEM:
            // Mini file system browser
            calc_file_browser();
            return false;

        case MENU_SETTINGS:
            // TBOS calculator settings
            calc_settings_menu();
            return true;

        case MENU_ABOUT:
            // About TBOS
            calc_display_about();
            calc_wait_key();
            return true;

        case MENU_EXIT:
            // Exit TBOS
            return false; // This will cause main loop to exit

        default:
            return true;
    }
}

/*
 * Display main calculator interface
 */
void calc_display_main_interface(void)
{
    // Line 0: Title bar
    if (g_calc_state.sacred_mode) {
        calc_display_text(0, 0, "🕉️ TBOS Sacred Calc");
    } else {
        calc_display_text(0, 0, "🔱 TBOS Calculator");
    }

    // Line 1: Input display
    calc_display_text(0, 1, "> ");
    calc_display_text(2, 1, g_calc_state.input_buffer);

    // Line 2: Result display
    if (g_calc_state.has_result) {
        char result_str[32];
        if (g_calc_state.sacred_mode) {
            // Display with sacred number interpretation
            sacred_number_format(g_calc_state.result, result_str);
        } else {
            calc_sprintf(result_str, "= %.6g", g_calc_state.result);
        }
        calc_display_text(0, 2, result_str);
    }

    // Status line
    char status[32];
    calc_sprintf(status, "Mode:%s Mem:%dK",
                g_calc_state.sacred_mode ? "Sacred" : "Std",
                calc_get_free_memory() / 1024);
    calc_display_text(0, g_display_config.height - 1, status);
}

/*
 * Sacred Geometry Boot Animation
 * Displays Hindu symbols and sacred patterns during boot
 */
void sacred_geometry_boot_animation(void)
{
    if (g_display_config.has_graphics) {
        // Draw Om symbol
        sacred_draw_om_symbol(g_display_config.width / 2, g_display_config.height / 2);
        calc_delay_ms(500);

        // Draw Triskelion
        sacred_draw_triskelion(g_display_config.width / 2, g_display_config.height / 2);
        calc_delay_ms(500);

        // Draw Sri Yantra (simplified)
        sacred_draw_sri_yantra_mini(g_display_config.width / 2, g_display_config.height / 2);
        calc_delay_ms(500);

    } else {
        // Text-only sacred animation
        const char* sacred_symbols[] = {"ॐ", "🕉️", "🔱", "✨"};
        for (int i = 0; i < 4; i++) {
            calc_display_text(g_display_config.width / 2 - 1, g_display_config.height / 2, sacred_symbols[i]);
            calc_delay_ms(300);
        }
    }
}

/*
 * Calculator shutdown procedure
 */
int calc_shutdown(void)
{
    // Display goodbye message
    calc_clear_screen();
    calc_display_text(0, 0, "🔱 TBOS Shutdown");
    calc_display_text(0, 1, "Sacred calculations");
    calc_display_text(0, 2, "saved to eternal");
    calc_display_text(0, 3, "consciousness.");
    calc_display_text(0, 4, "ॐ Namaste ॐ");

    calc_delay_ms(2000);

    // Cleanup
    sacred_consciousness_cleanup();
    steppps_calc_cleanup();
    calc_hardware_cleanup();

    return 0;
}

/*
 * Handle mathematical input from keypad
 */
void calc_handle_math_input(calc_key_t key)
{
    char key_char = calc_key_to_char(key);

    if (key == CALC_KEY_ENTER) {
        // Execute calculation
        if (g_calc_state.sacred_mode) {
            g_calc_state.result = sacred_math_evaluate(g_calc_state.input_buffer);
        } else {
            g_calc_state.result = calc_evaluate_expression(g_calc_state.input_buffer);
        }

        g_calc_state.has_result = true;
        g_calc_state.input_buffer[0] = '\0'; // Clear input

    } else if (key == CALC_KEY_BACKSPACE) {
        // Remove last character
        int len = calc_strlen(g_calc_state.input_buffer);
        if (len > 0) {
            g_calc_state.input_buffer[len - 1] = '\0';
        }

    } else if (key_char && calc_strlen(g_calc_state.input_buffer) < MAX_INPUT_LENGTH - 1) {
        // Add character to input buffer
        int len = calc_strlen(g_calc_state.input_buffer);
        g_calc_state.input_buffer[len] = key_char;
        g_calc_state.input_buffer[len + 1] = '\0';
    }

    // Update display
    calc_display_main_interface();
}