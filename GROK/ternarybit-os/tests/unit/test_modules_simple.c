/**
 * @file test_modules_simple.c
 * @brief Simple test of module registry system
 */

#include "../../src/core/tbos_modules.c"
#include "../../src/core/compression/tbos_compression.h"
#include <stdio.h>

/* Simple test module init functions */
static tbos_result_t test_module1_init(void) {
    printf("  [TEST1] Initializing...\n");
    return TBOS_SUCCESS;
}

static tbos_result_t test_module2_init(void) {
    printf("  [TEST2] Initializing...\n");
    return TBOS_SUCCESS;
}

static tbos_result_t compression_init_wrapper(void) {
    printf("  [COMPRESS] Initializing...\n");
    return tbos_compression_init();
}

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Module Registry Test                  ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    /* Initialize registry */
    printf("[TEST] Initializing module registry...\n");
    tbos_result_t result = tbos_modules_registry_init();
    if (result != TBOS_SUCCESS) {
        printf("❌ Registry init failed\n");
        return 1;
    }
    printf("✅ Registry initialized\n\n");

    /* Register test modules */
    printf("[TEST] Registering modules...\n");

    tbos_module_descriptor_t mod1 = {
        .id = TBOS_MODULE_HARDWARE,
        .name = "Test Module 1",
        .description = "First test module",
        .priority = TBOS_MODULE_PRIORITY_CRITICAL,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_module1_init,
        .shutdown = NULL,
        .dependencies = NULL,
        .dependency_count = 0
    };

    tbos_module_descriptor_t mod2 = {
        .id = TBOS_MODULE_MEMORY,
        .name = "Test Module 2",
        .description = "Second test module",
        .priority = TBOS_MODULE_PRIORITY_HIGH,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = test_module2_init,
        .shutdown = NULL,
        .dependencies = NULL,
        .dependency_count = 0
    };

    tbos_module_descriptor_t compression = {
        .id = TBOS_MODULE_COMPRESSION,
        .name = "PXFS Compression",
        .description = "Compression subsystem",
        .priority = TBOS_MODULE_PRIORITY_HIGH,
        .status = TBOS_MODULE_STATUS_UNINITIALIZED,
        .init = compression_init_wrapper,
        .shutdown = NULL,
        .dependencies = NULL,
        .dependency_count = 0
    };

    tbos_module_register(&mod1);
    tbos_module_register(&mod2);
    tbos_module_register(&compression);
    printf("✅ 3 modules registered\n\n");

    /* Initialize all */
    printf("[TEST] Initializing all modules...\n\n");
    result = tbos_modules_init_all();

    if (result != TBOS_SUCCESS) {
        printf("\n❌ Module initialization failed\n");
        return 1;
    }

    /* Print status */
    tbos_modules_print_status();

    /* Get stats */
    uint32_t total, initialized, failed;
    tbos_modules_get_stats(&total, &initialized, &failed);

    printf("[TEST] Statistics:\n");
    printf("  Total: %u\n", total);
    printf("  Initialized: %u\n", initialized);
    printf("  Failed: %u\n\n", failed);

    if (failed == 0 && initialized == total) {
        printf("✅ ALL TESTS PASSED\n\n");
        return 0;
    } else {
        printf("❌ SOME TESTS FAILED\n\n");
        return 1;
    }
}
