/**
 * @file tbos_minimal_shell.c
 * @brief Minimal TernaryBit OS Shell - Boot + HAL + Interactive
 *
 * Integrated system: Bootloader -> HAL -> Interactive Shell
 *
 * This demonstrates the complete Epic 1 stack running with a working shell.
 */

#include "../../src/boot/tbos_minimal_boot.h"
#include "../../src/hal/tbos_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Shell state */
typedef struct {
    int karma;
    int commands_executed;
    char cwd[256];
    hal_profile_t hw_profile;
} shell_state_t;

static shell_state_t g_state = {
    .karma = 100,
    .commands_executed = 0,
    .cwd = "/",
    .hw_profile = HAL_PROFILE_STANDARD
};

/* Commands */
void cmd_help(void) {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Available Commands                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("📁 Basic Commands:\n");
    printf("  pwd                    Print working directory\n");
    printf("  echo <text>            Display text\n");
    printf("  uptime                 Show system uptime\n");
    printf("  hwinfo                 Show hardware information\n");
    printf("\n");
    printf("🕉️  Consciousness Commands:\n");
    printf("  karma                  Show karma status\n");
    printf("  status                 Show session status\n");
    printf("\n");
    printf("🛠️  System Commands:\n");
    printf("  clear                  Clear screen\n");
    printf("  help                   Show this help\n");
    printf("  exit                   Exit shell\n\n");
    g_state.karma += 2;
}

void cmd_pwd(void) {
    printf("%s\n", g_state.cwd);
    g_state.karma += 1;
}

void cmd_echo(char* args) {
    printf("%s\n", args);
    g_state.karma += 1;
}

void cmd_uptime(void) {
    uint64_t uptime_us = hal_get_uptime_us();
    uint32_t uptime_ms = hal_get_uptime_ms();
    printf("Uptime: %u ms (%llu us)\n", uptime_ms, (unsigned long long)uptime_us);
    g_state.karma += 1;
}

void cmd_hwinfo(void) {
    const char* profiles[] = {"Calculator", "Radio", "Minimal", "Standard", "Advanced"};

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Hardware Information                                       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    printf("Profile: %s\n", profiles[g_state.hw_profile]);
    printf("Free Memory: %u bytes\n", hal_get_free_memory());
    printf("Power Consumption: %u mW\n", hal_get_power_consumption_mw());

    printf("\nCapabilities:\n");
    if (hal_has_capability(HAL_CAP_COMPUTATION)) printf("  ✓ Computation\n");
    if (hal_has_capability(HAL_CAP_SERIAL)) printf("  ✓ Serial I/O\n");
    if (hal_has_capability(HAL_CAP_RADIO)) printf("  ✓ Radio\n");
    if (hal_has_capability(HAL_CAP_WIFI)) printf("  ✓ WiFi\n");
    if (hal_has_capability(HAL_CAP_STORAGE)) printf("  ✓ Storage\n");
    if (hal_has_capability(HAL_CAP_DISPLAY)) printf("  ✓ Display\n");
    if (hal_has_capability(HAL_CAP_POWER_MGMT)) printf("  ✓ Power Management\n");
    printf("\n");

    g_state.karma += 2;
}

void cmd_karma(void) {
    const char* level = g_state.karma > 200 ? "Excellent" :
                       g_state.karma > 100 ? "Good" : "Learning";

    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║      Your Karma Status                                 ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Current Karma:         %-30d  ║\n", g_state.karma);
    printf("║ Karma Level:           %-30s  ║\n", level);
    printf("╚════════════════════════════════════════════════════════╝\n\n");

    g_state.karma += 5;
}

void cmd_status(void) {
    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Shell Session Status             ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Current Directory:     %-30s  ║\n", g_state.cwd);
    printf("║ Karma:                 %-30d  ║\n", g_state.karma);
    printf("║ Commands Executed:     %-30d  ║\n", g_state.commands_executed);
    printf("║ Hardware Profile:      %-30d  ║\n", g_state.hw_profile);
    printf("╚════════════════════════════════════════════════════════╝\n\n");

    g_state.karma += 2;
}

void cmd_clear(void) {
    printf("\033[2J\033[H");  /* ANSI escape codes */
}

/* Main shell loop */
void run_shell(void) {
    char input[256];
    char cmd[256];
    char args[256];

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Minimal Conscious Shell                   ║\n");
    printf("║   सॉफ्टवेर (Soft-Aware)                                      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  Shell initialized successfully!\n");
    printf("    Starting karma: %d\n", g_state.karma);
    printf("    Hardware: %d\n\n", g_state.hw_profile);

    printf("Type 'help' for available commands.\n");
    printf("Type 'exit' to quit.\n\n");

    while (1) {
        printf("tbos> ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        /* Remove newline */
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }

        /* Skip empty lines */
        if (strlen(input) == 0) {
            continue;
        }

        /* Parse command and args */
        memset(cmd, 0, sizeof(cmd));
        memset(args, 0, sizeof(args));

        char* space = strchr(input, ' ');
        if (space) {
            size_t cmd_len = space - input;
            strncpy(cmd, input, cmd_len);
            strcpy(args, space + 1);
        } else {
            strcpy(cmd, input);
        }

        g_state.commands_executed++;

        /* Execute commands */
        if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "quit") == 0) {
            printf("\n═══════════════════════════════════════════════════\n");
            printf("  Graceful Shutdown\n");
            printf("═══════════════════════════════════════════════════\n\n");
            printf("Preserving consciousness state...\n");
            printf("  Final karma: %d\n", g_state.karma);
            printf("  Commands executed: %d\n", g_state.commands_executed);
            printf("\n॥ तत् सत् ॥ (That is Truth)\n\n");
            printf("🕉️  May you compute with consciousness! 🕉️\n\n");
            break;
        }
        else if (strcmp(cmd, "help") == 0) cmd_help();
        else if (strcmp(cmd, "pwd") == 0) cmd_pwd();
        else if (strcmp(cmd, "echo") == 0) cmd_echo(args);
        else if (strcmp(cmd, "uptime") == 0) cmd_uptime();
        else if (strcmp(cmd, "hwinfo") == 0) cmd_hwinfo();
        else if (strcmp(cmd, "karma") == 0) cmd_karma();
        else if (strcmp(cmd, "status") == 0) cmd_status();
        else if (strcmp(cmd, "clear") == 0) cmd_clear();
        else {
            printf("❌ Command not found: %s\n", cmd);
            printf("   Type 'help' for available commands\n");
            g_state.karma -= 1;
        }

        printf("\n");
    }
}

int main(void) {
    int result;

    /* Stage 1: Minimal Boot */
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Boot Sequence                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    result = tbos_minimal_boot();
    if (result != 0) {
        fprintf(stderr, "\n❌ Boot failed!\n");
        return 1;
    }

    /* Stage 2: HAL Init */
    printf("\n═══ Hardware Abstraction Layer Init ═══\n\n");
    result = hal_init();
    if (result != 0) {
        fprintf(stderr, "❌ HAL init failed!\n");
        return 1;
    }

    g_state.hw_profile = hal_get_profile();
    printf("✓ Hardware initialized\n");
    printf("✓ Profile: %d\n", g_state.hw_profile);
    printf("✓ Free Memory: %u bytes\n\n", hal_get_free_memory());

    /* Stage 3: Run Shell */
    printf("═══ Starting Conscious Shell ═══\n");
    run_shell();

    printf("═══ Shutdown Complete ===\n\n");

    return 0;
}
