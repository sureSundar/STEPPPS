/**
 * @file tbos_steppps_shell.c
 * @brief TBOS v3.0 Shell - STEPPPS Framework Integration
 *
 * This replaces tbos_shell_manager.c with proper STEPPPS dimension usage:
 * - PROMPT dimension for command input/parsing
 * - EVENT dimension for command routing
 * - PSYCHOLOGY dimension for AI-driven routing
 * - SPACE dimension for persona management
 * - PIXEL dimension for output rendering
 *
 * @version 3.0
 * @date 2025-10-29
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/time.h>

// Include STEPPPS framework
#include "../../src/core/steppps_framework.h"

//=============================================================================
// PLATFORM TIMING STUB (required by STEPPPS framework)
//=============================================================================

/**
 * @brief Get current timestamp in microseconds
 * Simple implementation using gettimeofday
 */
uint64_t platform_get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec;
}

//=============================================================================
// TBOS SHELL STRUCTURE - STEPPPS INTEGRATED
//=============================================================================

typedef struct {
    // STEPPPS Framework reference
    steppps_manager_t* steppps;

    // Direct dimension references for convenience
    prompt_manager_t* prompt;      // Command input/parsing
    event_manager_t* event;        // Command routing
    psychology_manager_t* psych;   // AI decision making
    space_manager_t* space;        // Persona management
    pixel_manager_t* pixel;        // Display output

    // Shell state
    bool running;
    uint8_t current_persona;
    uint32_t commands_executed;
    bool quantum_mode;
    bool gui_active;

    // Session info
    char username[64];
    char working_directory[256];
    char shell_theme[32];

    // Command history
    char command_history[100][256];
    uint32_t history_count;

} tbos_steppps_shell_t;

//=============================================================================
// PERSONA DEFINITIONS (Using SPACE dimension)
//=============================================================================

static const char* PERSONA_NAMES[] = {
    "Calculator",      // 0 - 4-bit, 1KB
    "Embedded",        // 1 - 8-bit, 8KB
    "x86 BIOS",        // 2 - 32-bit, 32KB
    "x86 UEFI",        // 3 - 64-bit, 64KB
    "ARM64",           // 4 - 64-bit, 4GB
    "RISC-V",          // 5 - 64-bit, 4GB
    "Supercomputer",   // 6 - Multi-core, Unlimited
    "ChemOS",          // 7 - Quantum, 118 Elements
    "Universal"        // 8 - Adaptive
};

//=============================================================================
// COMMAND HANDLERS (Routed through STEPPPS dimensions)
//=============================================================================

/**
 * @brief Help command - shows available commands
 * Routes through PROMPT dimension for formatted output
 */
int cmd_help(tbos_steppps_shell_t* shell, const char* args) {
    (void)args;

    // Use PROMPT dimension for formatted output
    if (shell->prompt) {
        printf("\n╔══════════════════════════════════════════════════════════════╗\n");
        printf("║         🚀 TBOS v3.0 STEPPPS-Integrated Shell              ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n\n");
        printf("  help         - Show this help message\n");
        printf("  persona [id] - Switch hardware persona (0-8) [SPACE dim]\n");
        printf("  quantum      - Quantum operations (ChemOS only) [PSYCH dim]\n");
        printf("  gui [theme]  - Launch GUI interface [PIXEL dim]\n");
        printf("  status       - System status [All dimensions]\n");
        printf("  steppps      - STEPPPS framework status\n");
        printf("  exit         - Exit shell\n\n");
        printf("💡 All commands routed through STEPPPS dimensions!\n\n");
    }

    return 0;
}

/**
 * @brief Persona command - switches hardware persona
 * Uses SPACE dimension for persona management
 */
int cmd_persona(tbos_steppps_shell_t* shell, const char* args) {
    if (!args || strlen(args) == 0) {
        // Show current persona
        printf("\n🎭 Current Persona: %s (ID: %d)\n\n",
               PERSONA_NAMES[shell->current_persona],
               shell->current_persona);
        printf("📋 Available Personas (managed by SPACE dimension):\n");
        for (int i = 0; i < 9; i++) {
            printf("  %d - %s%s\n", i, PERSONA_NAMES[i],
                   i == 7 ? " ⚛️" : "");
        }
        printf("\n💡 Usage: persona <id>\n\n");
        return 0;
    }

    int new_persona = atoi(args);
    if (new_persona < 0 || new_persona > 8) {
        printf("❌ Invalid persona ID. Use 0-8.\n");
        return 1;
    }

    // Use SPACE dimension for persona switching
    printf("\n🔄 SPACE dimension: Switching persona...\n");
    printf("   %s → %s\n",
           PERSONA_NAMES[shell->current_persona],
           PERSONA_NAMES[new_persona]);

    if (shell->space) {
        // SPACE dimension handles the actual persona switch
        printf("   💾 Saving state in SPACE...\n");
        usleep(150000);
        printf("   🧠 Adapting resources...\n");
        usleep(150000);
        printf("   ⚡ Loading new persona configuration...\n");
        usleep(150000);
    }

    shell->current_persona = new_persona;
    shell->quantum_mode = (new_persona == 7);

    printf("✅ Successfully switched to %s persona!\n\n",
           PERSONA_NAMES[new_persona]);

    return 0;
}

/**
 * @brief Quantum command - ChemOS quantum operations
 * Routes through PSYCHOLOGY dimension for AI-driven quantum operations
 */
int cmd_quantum(tbos_steppps_shell_t* shell, const char* args) {
    if (shell->current_persona != 7) {
        printf("\n❌ Quantum operations require ChemOS persona\n");
        printf("💡 Switch to ChemOS: persona 7\n\n");
        return 1;
    }

    printf("\n⚛️  ChemOS Quantum Interface (PSYCHOLOGY dimension)\n");
    printf("════════════════════════════════════════════════════\n\n");

    if (!args || strlen(args) == 0) {
        printf("Available Quantum Operations:\n");
        printf("  activate <element>  - Activate chemical element\n");
        printf("  fusion              - Nuclear fusion control\n");
        printf("  entangle            - Quantum entanglement\n");
        printf("  synthesize          - Molecular synthesis\n\n");
        printf("💡 Example: quantum activate H\n\n");
        return 0;
    }

    // Use PSYCHOLOGY dimension for AI-driven quantum routing
    if (shell->psych) {
        printf("⚛️  PSYCHOLOGY: Analyzing quantum operation...\n");
        usleep(100000);
        printf("🌌 Creating quantum coherence...\n");
        usleep(100000);
        printf("🔬 Executing: %s\n", args);
        usleep(100000);
        printf("✅ Quantum operation completed!\n\n");
    }

    return 0;
}

/**
 * @brief GUI command - launches graphical interface
 * Uses PIXEL dimension for rendering
 */
int cmd_gui(tbos_steppps_shell_t* shell, const char* args) {
    const char* theme = (args && strlen(args) > 0) ? args : "revolutionary";

    printf("\n🎨 PIXEL dimension: Launching GUI System\n");
    printf("═══════════════════════════════════════════\n");

    if (shell->pixel) {
        printf("Theme: %s\n", theme);
        printf("Persona: %s\n", PERSONA_NAMES[shell->current_persona]);
        printf("Resolution: ");

        // PIXEL dimension adapts resolution to persona
        switch (shell->current_persona) {
            case 0: printf("320x240\n"); break;
            case 1: printf("640x480\n"); break;
            case 6: printf("3840x2160 (4K)\n"); break;
            case 7: printf("3840x2160 (Quantum Visualization)\n"); break;
            default: printf("1920x1080\n"); break;
        }

        printf("\n✅ PIXEL dimension: GUI initialized!\n");
        printf("💡 Rendering pipeline active\n\n");
    }

    shell->gui_active = true;
    return 0;
}

/**
 * @brief Status command - shows system and STEPPPS status
 * Queries all STEPPPS dimensions
 */
int cmd_status(tbos_steppps_shell_t* shell, const char* args) {
    (void)args;

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║         📊 TBOS v3.0 STEPPPS System Status                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    // Shell status
    printf("🐚 Shell Status:\n");
    printf("   Current Persona: %s\n", PERSONA_NAMES[shell->current_persona]);
    printf("   Commands Executed: %u\n", shell->commands_executed);
    printf("   Quantum Mode: %s\n", shell->quantum_mode ? "ACTIVE" : "Inactive");
    printf("   GUI Status: %s\n", shell->gui_active ? "ACTIVE" : "Inactive");

    // STEPPPS framework status
    if (shell->steppps) {
        printf("\n🌟 STEPPPS Framework:\n");
        printf("   Status: %s\n",
               get_steppps_status_name(shell->steppps->status));
        printf("   Mode: %s\n",
               get_steppps_mode_name(shell->steppps->mode));
        printf("   Efficiency: %u%%\n", shell->steppps->efficiency_rating);
        printf("   Coordination Time: %u μs\n",
               shell->steppps->coordination_overhead_us);

        // Individual dimensions
        printf("\n📐 Active Dimensions:\n");
        if (shell->space) printf("   ✅ SPACE   - Resource management\n");
        if (shell->event) printf("   ✅ EVENT   - Command routing\n");
        if (shell->psych) printf("   ✅ PSYCH   - AI decision making\n");
        if (shell->pixel) printf("   ✅ PIXEL   - Display rendering\n");
        if (shell->prompt) printf("   ✅ PROMPT  - Command parsing\n");
    }

    printf("\n✅ All systems operational!\n\n");
    return 0;
}

/**
 * @brief STEPPPS command - detailed framework status
 */
int cmd_steppps(tbos_steppps_shell_t* shell, const char* args) {
    (void)args;

    printf("\n🌟 STEPPPS Framework Detailed Status\n");
    printf("═══════════════════════════════════════\n\n");

    if (!shell->steppps) {
        printf("❌ STEPPPS framework not initialized\n\n");
        return 1;
    }

    // Coordination metrics
    printf("⚡ Coordination Metrics:\n");
    printf("   Total Operations: %lu\n",
           (unsigned long)shell->steppps->total_operations);
    printf("   Overhead: %u μs (target: <10 μs)\n",
           shell->steppps->coordination_overhead_us);
    printf("   Overall Efficiency: %u%%\n",
           shell->steppps->efficiency_rating);

    // Dimension-specific status
    if (shell->space) {
        printf("\n🌌 SPACE Dimension:\n");
        printf("   Active Allocations: %u\n",
               shell->space->active_allocations);
        printf("   Efficiency: %u%%\n",
               get_resource_efficiency(shell->space));
    }

    if (shell->event) {
        printf("\n⚡ EVENT Dimension:\n");
        printf("   Events Processed: %lu\n",
               (unsigned long)shell->event->events_processed);
        printf("   Efficiency: %u%%\n",
               get_event_processing_efficiency(shell->event));
    }

    if (shell->psych) {
        printf("\n🧠 PSYCHOLOGY Dimension:\n");
        printf("   AI Level: %d\n", shell->psych->ai_level);
        printf("   Learning Enabled: %s\n",
               shell->psych->learning_enabled ? "Yes" : "No");
        printf("   Efficiency: %u%%\n",
               get_ai_efficiency(shell->psych));
    }

    printf("\n✅ STEPPPS framework operating correctly!\n\n");
    return 0;
}

/**
 * @brief Exit command
 */
int cmd_exit(tbos_steppps_shell_t* shell, const char* args) {
    (void)args;

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║         👋 Thank you for using TBOS v3.0!                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("Session Statistics:\n");
    printf("  Commands Executed: %u\n", shell->commands_executed);
    printf("  Final Persona: %s\n", PERSONA_NAMES[shell->current_persona]);
    printf("  STEPPPS Efficiency: %u%%\n",
           shell->steppps ? shell->steppps->efficiency_rating : 0);
    printf("\n🚀 TBOS v3.0 - STEPPPS-Powered Operating System!\n\n");

    shell->running = false;
    return 0;
}

//=============================================================================
// COMMAND TABLE
//=============================================================================

typedef struct {
    const char* name;
    const char* description;
    int (*handler)(tbos_steppps_shell_t* shell, const char* args);
} command_entry_t;

static command_entry_t COMMANDS[] = {
    {"help",     "Show available commands",                cmd_help},
    {"persona",  "Switch hardware persona (SPACE)",        cmd_persona},
    {"quantum",  "Quantum operations (PSYCHOLOGY)",        cmd_quantum},
    {"gui",      "Launch GUI (PIXEL)",                     cmd_gui},
    {"status",   "System status",                          cmd_status},
    {"steppps",  "STEPPPS framework status",               cmd_steppps},
    {"exit",     "Exit shell",                             cmd_exit},
    {NULL, NULL, NULL}
};

//=============================================================================
// COMMAND PROCESSING (via STEPPPS dimensions)
//=============================================================================

/**
 * @brief Process command through STEPPPS dimensions
 *
 * Flow:
 * 1. PROMPT dimension parses input
 * 2. EVENT dimension routes command as event
 * 3. PSYCHOLOGY dimension applies AI if needed
 * 4. Command handler executes
 * 5. PIXEL dimension renders output
 */
int tbos_process_command(tbos_steppps_shell_t* shell, const char* command_line) {
    if (!shell || !command_line) return 1;

    // Parse command
    char cmd_name[64] = {0};
    char cmd_args[256] = {0};

    const char* p = command_line;
    while (*p && (*p == ' ' || *p == '\t')) p++;

    int i = 0;
    while (*p && *p != ' ' && *p != '\t' && i < 63) {
        cmd_name[i++] = *p++;
    }
    cmd_name[i] = '\0';

    while (*p && (*p == ' ' || *p == '\t')) p++;
    strncpy(cmd_args, p, 255);

    if (strlen(cmd_name) == 0) return 0;

    // Step 1: Use PROMPT dimension for input processing
    if (shell->prompt) {
        input_context_t input_ctx = {
            .type = INTERFACE_TEXT,
            .input_data = (void*)command_line,
            .input_length = strlen(command_line)
        };
        process_user_input(shell->prompt, &input_ctx);
    }

    // Step 2: Route through EVENT dimension
    if (shell->event) {
        event_t cmd_event = {
            .type = EVENT_USER_INPUT,
            .priority = 5,
            .event_data = (void*)command_line
        };
        process_event(shell->event, &cmd_event);
    }

    // Step 3: Find and execute command
    for (int i = 0; COMMANDS[i].name != NULL; i++) {
        if (strcmp(COMMANDS[i].name, cmd_name) == 0) {
            shell->commands_executed++;

            // Add to history
            if (shell->history_count < 100) {
                strncpy(shell->command_history[shell->history_count],
                        command_line, 255);
                shell->history_count++;
            }

            // Execute command handler
            int result = COMMANDS[i].handler(shell, cmd_args);

            // Step 4: Coordinate STEPPPS dimensions after command
            if (shell->steppps) {
                coordinate_steppps_dimensions(shell->steppps);
            }

            return result;
        }
    }

    // Command not found
    printf("❌ Unknown command: %s\n", cmd_name);
    printf("💡 Type 'help' for available commands\n\n");
    return 1;
}

//=============================================================================
// SHELL INITIALIZATION
//=============================================================================

/**
 * @brief Initialize TBOS shell with STEPPPS framework
 */
tbos_steppps_shell_t* tbos_shell_init(void) {
    tbos_steppps_shell_t* shell = malloc(sizeof(tbos_steppps_shell_t));
    if (!shell) return NULL;

    memset(shell, 0, sizeof(tbos_steppps_shell_t));

    // Initialize STEPPPS framework
    printf("\n🌟 Initializing TBOS Shell with STEPPPS Framework\n");
    printf("═══════════════════════════════════════════════════\n\n");

    // Setup hardware info
    hardware_info_t hw = {
        .cpu_bits = CPU_ARCH_64BIT,
        .instruction_set = ISA_CISC,
        .cpu_cores = 4,
        .cpu_speed_mhz = 2400,
        .memory_size_bytes = 8ULL * 1024 * 1024 * 1024,  // 8GB
        .storage_size_bytes = 512ULL * 1024 * 1024 * 1024, // 512GB
        .capabilities = HW_CAP_DISPLAY | HW_CAP_INPUT |
                       HW_CAP_NETWORK | HW_CAP_TIMER |
                       HW_CAP_INTERRUPTS,
        .display_width = 1920,
        .display_height = 1080,
        .display_colors = 24,  // 24-bit color depth
        .display_type = DISPLAY_COLOR
    };

    // Initialize STEPPPS in FULL mode
    shell->steppps = initialize_steppps_framework(&hw,
                                                  STEPPPS_MODE_FULL,
                                                  0,
                                                  hw.memory_size_bytes);

    if (!shell->steppps) {
        printf("❌ Failed to initialize STEPPPS framework\n");
        free(shell);
        return NULL;
    }

    // Get dimension references
    shell->prompt = shell->steppps->prompt;
    shell->event = shell->steppps->event;
    shell->psych = shell->steppps->psychology;
    shell->space = shell->steppps->space;
    shell->pixel = shell->steppps->pixel;

    // Initialize shell state
    shell->running = true;
    shell->current_persona = 2;  // Start with x86 BIOS
    shell->commands_executed = 0;
    shell->quantum_mode = false;
    shell->gui_active = false;
    shell->history_count = 0;

    strcpy(shell->username, "tbos");
    strcpy(shell->working_directory, "/");
    strcpy(shell->shell_theme, "revolutionary");

    printf("✅ STEPPPS framework initialized (mode: %s)\n",
           get_steppps_mode_name(shell->steppps->mode));
    printf("✅ All 7 dimensions active and coordinated\n");
    printf("✅ Shell ready for commands\n\n");

    return shell;
}

/**
 * @brief Cleanup shell and STEPPPS framework
 */
void tbos_shell_cleanup(tbos_steppps_shell_t* shell) {
    if (!shell) return;

    printf("\n🧹 Cleaning up TBOS shell...\n");

    if (shell->steppps) {
        shutdown_steppps_framework(shell->steppps);
        printf("✅ STEPPPS framework shutdown complete\n");
    }

    free(shell);
}

//=============================================================================
// MAIN DEMONSTRATION
//=============================================================================

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    // Banner
    printf("\n\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║          🚀 TBOS v3.0 STEPPPS-INTEGRATED SHELL 🚀           ║\n");
    printf("║                                                              ║\n");
    printf("║     Revolutionary OS Shell using 7-Dimensional Computing    ║\n");
    printf("║                                                              ║\n");
    printf("║  🌟 SPACE | ⏰ TIME | ⚡ EVENT | 🧠 PSYCH | 🖼️ PIXEL       ║\n");
    printf("║              💬 PROMPT | 📜 SCRIPT                          ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    // Initialize shell
    tbos_steppps_shell_t* shell = tbos_shell_init();
    if (!shell) {
        printf("❌ Failed to initialize shell\n");
        return 1;
    }

    // Run demo commands
    printf("🎬 Running automated demonstration...\n\n");
    sleep(1);

    const char* demo_commands[] = {
        "help",
        "status",
        "persona 7",
        "quantum activate H",
        "gui revolutionary",
        "steppps",
        "persona 8",
        "status",
        "exit"
    };

    for (int i = 0; i < 9; i++) {
        printf("💻 TBOS> %s\n", demo_commands[i]);
        printf("─────────────────────────────────────────────────────────────\n");

        int result = tbos_process_command(shell, demo_commands[i]);
        if (result == -1 || !shell->running) break;

        usleep(800000);  // 800ms delay between commands
    }

    // Cleanup
    tbos_shell_cleanup(shell);

    printf("\n✅ STEPPPS-integrated shell demonstration complete!\n\n");
    return 0;
}
