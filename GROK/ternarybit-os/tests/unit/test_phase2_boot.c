/**
 * @file test_phase2_boot.c
 * @brief Phase 2 - Test complete kernel boot sequence
 */

#include <stdio.h>
#include <stdlib.h>

/* Include module system */
#include "../../src/core/tbos_modules.c"
#include "../../src/core/compression/tbos_compression.c"
#include "../../src/core/compression/pxfs_codec.c"

/* Module init functions */
static tbos_module_result_t test_hardware_init(void) {
    printf("  [HW] Hardware detection complete\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_t test_memory_init(void) {
    printf("  [MEM] Memory manager initialized (test stub)\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_t test_steppps_init(void) {
    printf("  [STEPPPS] Framework initialized (test stub)\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_t test_compression_init(void) {
    printf("  [COMPRESS] Initializing PXFS...\n");
    int result = tbos_compression_init();
    if (result == 0) {
        printf("  [COMPRESS] PXFS ready\n");
        return TBOS_MODULE_SUCCESS;
    }
    return TBOS_MODULE_ERROR;
}

static tbos_module_result_t test_process_init(void) {
    printf("  [PROC] Process manager initialized (test stub)\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_t test_scheduler_init(void) {
    printf("  [SCHED] Scheduler initialized (test stub)\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_t test_filesystem_init(void) {
    printf("  [FS] Filesystem initialized (test stub)\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_t test_shell_init(void) {
    printf("  [SHELL] Shell initialized (test stub)\n");
    return TBOS_MODULE_SUCCESS;
}

int main(void) {
    printf("\n\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║         TernaryBit OS - Phase 2 Boot Test                   ║\n");
    printf("║         Module Integration Verification                     ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* Step 1: Initialize module registry */
    printf("[BOOT] Initializing module registry...\n");
    tbos_module_result_t result = tbos_modules_registry_init();
    if (result != TBOS_MODULE_SUCCESS) {
        printf("❌ [BOOT] Module registry init FAILED\n\n");
        return 1;
    }
    printf("✅ [BOOT] Module registry initialized\n\n");

    /* Step 2: Register modules */
    printf("[BOOT] Registering modules...\n");

    tbos_module_descriptor_t hardware_mod = {
        .id = TBOS_MODULE_HARDWARE,
        .name = "Hardware Detection",
        .description = "Hardware detector and HAL",
        .priority = TBOS_MODULE_PRIORITY_CRITICAL,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_hardware_init,
        .shutdown = NULL,
        .dependencies = NULL,
        .dependency_count = 0
    };

    tbos_module_descriptor_t memory_mod = {
        .id = TBOS_MODULE_MEMORY,
        .name = "Memory Manager",
        .description = "Heap and memory allocation",
        .priority = TBOS_MODULE_PRIORITY_CRITICAL,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_memory_init,
        .shutdown = NULL,
        .dependencies = NULL,
        .dependency_count = 0
    };

    tbos_module_id_t steppps_deps[] = {TBOS_MODULE_MEMORY};
    tbos_module_descriptor_t steppps_mod = {
        .id = TBOS_MODULE_STEPPPS,
        .name = "STEPPPS Framework",
        .description = "Mother-Arcing Protocol",
        .priority = TBOS_MODULE_PRIORITY_HIGH,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_steppps_init,
        .shutdown = NULL,
        .dependencies = steppps_deps,
        .dependency_count = 1
    };

    tbos_module_id_t compress_deps[] = {TBOS_MODULE_MEMORY};
    tbos_module_descriptor_t compress_mod = {
        .id = TBOS_MODULE_COMPRESSION,
        .name = "PXFS Compression",
        .description = "Pixel File System compression",
        .priority = TBOS_MODULE_PRIORITY_HIGH,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_compression_init,
        .shutdown = NULL,
        .dependencies = compress_deps,
        .dependency_count = 1
    };

    tbos_module_id_t proc_deps[] = {TBOS_MODULE_MEMORY};
    tbos_module_descriptor_t proc_mod = {
        .id = TBOS_MODULE_PROCESS,
        .name = "Process Manager",
        .description = "Process management",
        .priority = TBOS_MODULE_PRIORITY_HIGH,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_process_init,
        .shutdown = NULL,
        .dependencies = proc_deps,
        .dependency_count = 1
    };

    tbos_module_id_t sched_deps[] = {TBOS_MODULE_PROCESS};
    tbos_module_descriptor_t sched_mod = {
        .id = TBOS_MODULE_SCHEDULER,
        .name = "Scheduler",
        .description = "Process scheduler",
        .priority = TBOS_MODULE_PRIORITY_HIGH,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_scheduler_init,
        .shutdown = NULL,
        .dependencies = sched_deps,
        .dependency_count = 1
    };

    tbos_module_id_t fs_deps[] = {TBOS_MODULE_MEMORY, TBOS_MODULE_COMPRESSION};
    tbos_module_descriptor_t fs_mod = {
        .id = TBOS_MODULE_FILESYSTEM,
        .name = "Filesystem",
        .description = "Virtual filesystem",
        .priority = TBOS_MODULE_PRIORITY_MEDIUM,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_filesystem_init,
        .shutdown = NULL,
        .dependencies = fs_deps,
        .dependency_count = 2
    };

    tbos_module_id_t shell_deps[] = {TBOS_MODULE_FILESYSTEM, TBOS_MODULE_PROCESS};
    tbos_module_descriptor_t shell_mod = {
        .id = TBOS_MODULE_SHELL,
        .name = "Shell",
        .description = "Command shell",
        .priority = TBOS_MODULE_PRIORITY_LOW,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_shell_init,
        .shutdown = NULL,
        .dependencies = shell_deps,
        .dependency_count = 2
    };

    tbos_module_register(&hardware_mod);
    tbos_module_register(&memory_mod);
    tbos_module_register(&steppps_mod);
    tbos_module_register(&compress_mod);
    tbos_module_register(&proc_mod);
    tbos_module_register(&sched_mod);
    tbos_module_register(&fs_mod);
    tbos_module_register(&shell_mod);

    printf("✅ [BOOT] 8 modules registered\n\n");

    /* Step 3: Initialize all modules */
    result = tbos_modules_init_all();
    if (result != TBOS_MODULE_SUCCESS) {
        printf("\n❌ [BOOT] Module initialization had errors\n");
    }

    /* Step 4: Print status */
    tbos_modules_print_status();

    /* Step 5: Check results */
    uint32_t total, initialized, failed;
    tbos_modules_get_stats(&total, &initialized, &failed);

    printf("[BOOT] Initialization Complete\n");
    printf("[BOOT] Total: %u, Ready: %u, Failed: %u\n\n", total, initialized, failed);

    if (failed == 0 && initialized == total) {
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║                                                              ║\n");
        printf("║           ✅ PHASE 2 BOOT TEST: SUCCESS                      ║\n");
        printf("║        All modules initialized successfully!                ║\n");
        printf("║                                                              ║\n");
        printf("║  Ready for Phase 3: Shell Implementation                    ║\n");
        printf("║                                                              ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        printf("\n");
        printf("Simulating shell prompt:\n");
        printf("tbos> _\n\n");
        return 0;
    } else {
        printf("❌ PHASE 2 BOOT TEST: FAILED\n");
        printf("   Some modules did not initialize\n\n");
        return 1;
    }
}
