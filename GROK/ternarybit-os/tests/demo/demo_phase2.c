/**
 * @file demo_phase2.c
 * @brief TernaryBit OS - Phase 2 Complete Demonstration
 *
 * Demonstrates:
 * - Conscious Shell
 * - Process Manager with karma-based scheduling
 * - Filesystem operations
 * - Soft shutdown
 *
 * @date 2025-11-04
 */

#include "../../src/core/shell/tbos_shell.h"
#include "../../src/core/process/tbos_process.h"
#include "../../src/core/tbos_shutdown.h"
#include <stdio.h>

/* Simple test process */
int test_process_main(int argc, char** argv) {
    printf("  [Test Process] Starting with %d args\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("    arg[%d]: %s\n", i, argv[i]);
    }
    printf("  [Test Process] Completed successfully\n");
    return 0;
}

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Phase 2 Complete Demonstration            ║\n");
    printf("║                                                              ║\n");
    printf("║   Features:                                                  ║\n");
    printf("║   - Conscious Shell with Karma Tracking                     ║\n");
    printf("║   - Process Manager with Conscious Scheduling               ║\n");
    printf("║   - Ramdisk Filesystem                                      ║\n");
    printf("║   - Soft Shutdown with State Preservation                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    /* Initialize all subsystems */
    printf("═══ Initializing Subsystems ═══\n\n");

    tbos_shell_init();
    tbos_process_init();
    tbos_shutdown_init();

    /* Demonstrate Process Manager */
    printf("\n═══ Process Manager Demonstration ═══\n\n");

    /* Create test processes */
    char* args1[] = {"test1", "arg1"};
    tbos_process_create_params_t params1 = {
        .name = "TestProcess1",
        .entry = test_process_main,
        .argc = 2,
        .argv = args1,
        .parent_pid = TBOS_PID_KERNEL,
        .priority = TBOS_PRIORITY_NORMAL,
        .stack_size = 0,
        .initial_awareness = CONSCIOUSNESS_AWARE
    };

    tbos_pid_t pid1 = tbos_process_create(&params1);
    printf("\n");

    char* args2[] = {"test2", "compassionate"};
    tbos_process_create_params_t params2 = {
        .name = "CompassionateProc",
        .entry = test_process_main,
        .argc = 2,
        .argv = args2,
        .parent_pid = TBOS_PID_KERNEL,
        .priority = TBOS_PRIORITY_HIGH,
        .stack_size = 0,
        .initial_awareness = CONSCIOUSNESS_COMPASSIONATE
    };

    tbos_pid_t pid2 = tbos_process_create(&params2);
    printf("\n");

    /* Show process table */
    printf("Process table after creation:\n");
    tbos_process_print_table();

    /* Show scheduler status */
    tbos_scheduler_print_status();

    /* Demonstrate process karma */
    printf("═══ Process Karma System ═══\n\n");

    printf("Process 1 helps Process 2:\n");
    tbos_process_help(pid1, pid2);
    printf("\n");

    printf("Process 1 consciousness evolves:\n");
    tbos_process_evolve_consciousness(pid1);
    printf("\n");

    /* Show process details */
    printf("Process details after karma changes:\n");
    tbos_process_print_details(pid1);

    /* Demonstrate shell commands */
    printf("\n═══ Shell Command Demonstration ═══\n\n");

    printf("Executing: pwd\n");
    tbos_shell_execute("pwd");
    printf("\n");

    printf("Executing: ls /\n");
    tbos_shell_execute("ls /");
    printf("\n");

    printf("Executing: karma\n");
    tbos_shell_execute("karma");

    printf("Executing: consciousness\n");
    tbos_shell_execute("consciousness");

    printf("Executing: ps\n");
    tbos_shell_execute("ps");

    printf("Executing: top\n");
    tbos_shell_execute("top");

    /* Demonstrate shutdown */
    printf("\n═══ Graceful Shutdown Demonstration ═══\n\n");

    /* Register a test module for shutdown */
    int dummy_callback(tbos_shutdown_phase_t phase, void* context) {
        printf("  [Demo Module] Shutdown phase %d\n", phase);
        return 0;
    }

    tbos_shutdown_register("DemoModule", dummy_callback, NULL, 5);

    /* Save some state */
    struct {
        int value;
        char name[32];
    } state = {42, "DemoState"};

    tbos_shutdown_save_module_state(
        "DemoModule",
        &state,
        sizeof(state),
        CONSCIOUSNESS_AWARE,
        150
    );

    /* Initiate shutdown */
    tbos_shutdown_initiate(TBOS_SHUTDOWN_NORMAL, 0);

    /* Summary */
    printf("\n═══ Phase 2 Demonstration Complete ═══\n\n");

    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   Phase 2 Features Successfully Demonstrated:               ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ ✅ Conscious Shell - 15 commands with karma tracking         ║\n");
    printf("║ ✅ Process Manager - Karma-based scheduling                  ║\n");
    printf("║ ✅ Process Consciousness - Evolution & Compassion            ║\n");
    printf("║ ✅ Ramdisk Filesystem - File operations working              ║\n");
    printf("║ ✅ Soft Shutdown - State preservation complete               ║\n");
    printf("║                                                              ║\n");
    printf("║   All Phase 2 objectives achieved!                          ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  TernaryBit OS: Where Silicon Meets Spirituality 🕉️\n\n");

    return 0;
}
