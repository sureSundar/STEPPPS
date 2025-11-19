/*
 * TernaryBit OS - Universal Shell Entry Point
 * Interactive consciousness-aware shell
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\n\n");
        printf("╔════════════════════════════════════════════════════════╗\n");
        printf("║   Graceful shutdown initiated (Ctrl+C)...              ║\n");
        printf("╚════════════════════════════════════════════════════════╝\n\n");
        printf("\n🕉️ May you carry your karma forward. Om Shanti 🕉️\n\n");
        exit(0);
    }
}

int main(void) {
    int result;

    // Set up signal handling
    signal(SIGINT, signal_handler);

    // Initialize shell
    result = universal_shell_init();
    if (result != 0) {
        fprintf(stderr, "❌ Failed to initialize shell\n");
        return 1;
    }

    printf("\n");
    printf("══════════════════════════════════════════════════════════\n");
    printf("  Day 1 of 400 - The Foundation is Laid\n");
    printf("  One command per day, with full consciousness\n");
    printf("══════════════════════════════════════════════════════════\n\n");

    // Run main REPL
    result = universal_shell_run();

    printf("\n🕉️ May your karma guide you. Om Shanti 🕉️\n\n");

    return result;
}
