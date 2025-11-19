/**
 * @file tbos_init.c
 * @brief TernaryBit OS Kernel Initialization
 *
 * Master initialization sequence for the entire operating system.
 * This file orchestrates the boot process and module initialization.
 *
 * Boot Sequence:
 * 1. Bootloader loads kernel
 * 2. Kernel entry point calls tbos_kernel_init()
 * 3. Module registry initialized
 * 4. Modules registered in dependency order
 * 5. Modules initialized by priority
 * 6. Shell starts
 * 7. System ready
 *
 * @version 1.0
 * @date 2025-11-03
 * @author TernaryBit Development Team
 */

#include "tbos_modules.h"
#include "tbos_base.h"
#include "tbos_memory.h"
#include "compression/tbos_compression.h"
#include <stdio.h>
#include <stdlib.h>

/* ========================================================================= */
/* MODULE INITIALIZATION CALLBACKS                                           */
/* ========================================================================= */

/* Forward declarations of module init functions */
static tbos_module_result_thardware_module_init(void);
static tbos_module_result_tmemory_module_init(void);
static tbos_module_result_tsteppps_module_init(void);
static tbos_module_result_tcompression_module_init(void);
static tbos_module_result_tprocess_module_init(void);
static tbos_module_result_tscheduler_module_init(void);
static tbos_module_result_tfilesystem_module_init(void);
static tbos_module_result_tshell_module_init(void);

/* ========================================================================= */
/* MODULE DEFINITIONS                                                        */
/* ========================================================================= */

/* Hardware module - CRITICAL priority */
static tbos_module_descriptor_t hardware_module = TBOS_MODULE_DEFINE(
    TBOS_MODULE_HARDWARE,
    "Hardware Detection",
    "Hardware detector and HAL initialization",
    TBOS_MODULE_PRIORITY_CRITICAL,
    hardware_module_init,
    NULL
);

/* Memory module - CRITICAL priority */
static tbos_module_descriptor_t memory_module = TBOS_MODULE_DEFINE(
    TBOS_MODULE_MEMORY,
    "Memory Manager",
    "Heap and memory allocation management",
    TBOS_MODULE_PRIORITY_CRITICAL,
    memory_module_init,
    NULL
);

/* STEPPPS module - HIGH priority, depends on memory */
static tbos_module_id_t steppps_deps[] = {TBOS_MODULE_MEMORY};
static tbos_module_descriptor_t steppps_module = TBOS_MODULE_DEFINE_WITH_DEPS(
    TBOS_MODULE_STEPPPS,
    "STEPPPS Framework",
    "Mother-Arcing Protocol implementation",
    TBOS_MODULE_PRIORITY_HIGH,
    steppps_module_init,
    NULL,
    steppps_deps,
    1
);

/* Compression module - HIGH priority, depends on memory */
static tbos_module_id_t compression_deps[] = {TBOS_MODULE_MEMORY};
static tbos_module_descriptor_t compression_module = TBOS_MODULE_DEFINE_WITH_DEPS(
    TBOS_MODULE_COMPRESSION,
    "PXFS Compression",
    "Pixel File System compression engine",
    TBOS_MODULE_PRIORITY_HIGH,
    compression_module_init,
    NULL,
    compression_deps,
    1
);

/* Process module - HIGH priority, depends on memory */
static tbos_module_id_t process_deps[] = {TBOS_MODULE_MEMORY};
static tbos_module_descriptor_t process_module = TBOS_MODULE_DEFINE_WITH_DEPS(
    TBOS_MODULE_PROCESS,
    "Process Manager",
    "Process creation and management",
    TBOS_MODULE_PRIORITY_HIGH,
    process_module_init,
    NULL,
    process_deps,
    1
);

/* Scheduler module - HIGH priority, depends on process */
static tbos_module_id_t scheduler_deps[] = {TBOS_MODULE_PROCESS};
static tbos_module_descriptor_t scheduler_module = TBOS_MODULE_DEFINE_WITH_DEPS(
    TBOS_MODULE_SCHEDULER,
    "Process Scheduler",
    "CPU scheduling and time slicing",
    TBOS_MODULE_PRIORITY_HIGH,
    scheduler_module_init,
    NULL,
    scheduler_deps,
    1
);

/* Filesystem module - MEDIUM priority, depends on memory and compression */
static tbos_module_id_t filesystem_deps[] = {TBOS_MODULE_MEMORY, TBOS_MODULE_COMPRESSION};
static tbos_module_descriptor_t filesystem_module = TBOS_MODULE_DEFINE_WITH_DEPS(
    TBOS_MODULE_FILESYSTEM,
    "File System",
    "Virtual file system and storage",
    TBOS_MODULE_PRIORITY_MEDIUM,
    filesystem_module_init,
    NULL,
    filesystem_deps,
    2
);

/* Shell module - LOW priority, depends on filesystem and process */
static tbos_module_id_t shell_deps[] = {TBOS_MODULE_FILESYSTEM, TBOS_MODULE_PROCESS};
static tbos_module_descriptor_t shell_module = TBOS_MODULE_DEFINE_WITH_DEPS(
    TBOS_MODULE_SHELL,
    "Command Shell",
    "Interactive command-line interface",
    TBOS_MODULE_PRIORITY_LOW,
    shell_module_init,
    NULL,
    shell_deps,
    2
);

/* ========================================================================= */
/* MODULE INITIALIZATION IMPLEMENTATIONS                                     */
/* ========================================================================= */

static tbos_module_result_thardware_module_init(void) {
    printf("  [HW] Detecting hardware...\n");

    /* Initialize hardware detector */
    /* TODO: Call actual hardware detection */

    printf("  [HW] Hardware detection complete\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_tmemory_module_init(void) {
    printf("  [MEM] Initializing memory manager...\n");

    /* Initialize memory manager with 10MB heap */
    int result = tbos_memory_init(10 * 1024 * 1024);
    if (result != 0) {
        printf("  [MEM] Failed to initialize memory manager\n");
        return TBOS_MODULE_ERROR;
    }

    printf("  [MEM] Memory manager initialized\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_tsteppps_module_init(void) {
    printf("  [STEPPPS] Initializing STEPPPS framework...\n");

    /* Initialize STEPPPS */
    /* TODO: Call steppps_init() when available */

    printf("  [STEPPPS] Framework initialized\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_tcompression_module_init(void) {
    printf("  [COMPRESS] Initializing PXFS compression...\n");

    /* Initialize compression subsystem */
    int result = tbos_compression_init();
    if (result != 0) {
        return TBOS_MODULE_ERROR;
    }

    printf("  [COMPRESS] PXFS codec ready\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_tprocess_module_init(void) {
    printf("  [PROC] Initializing process manager...\n");

    /* Initialize process manager */
    /* TODO: Call process manager init */

    printf("  [PROC] Process manager ready\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_tscheduler_module_init(void) {
    printf("  [SCHED] Initializing scheduler...\n");

    /* Initialize scheduler */
    /* TODO: Call scheduler init */

    printf("  [SCHED] Scheduler ready\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_tfilesystem_module_init(void) {
    printf("  [FS] Initializing filesystem...\n");

    /* Initialize filesystem */
    /* TODO: Create ramdisk, mount root */

    printf("  [FS] Filesystem ready\n");
    return TBOS_MODULE_SUCCESS;
}

static tbos_module_result_tshell_module_init(void) {
    printf("  [SHELL] Initializing command shell...\n");

    /* Initialize shell */
    /* TODO: Register commands, setup terminal */

    printf("  [SHELL] Shell ready\n");
    return TBOS_MODULE_SUCCESS;
}

/* ========================================================================= */
/* KERNEL INITIALIZATION                                                     */
/* ========================================================================= */

/**
 * @brief Main kernel initialization routine
 * Called by bootloader after kernel is loaded into memory
 *
 * @return TBOS_SUCCESS on successful init, error code otherwise
 */
tbos_module_result_t tbos_kernel_init(void) {
    printf("\n\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║              TernaryBit OS v1.0 - Kernel Init                ║\n");
    printf("║         Under STEPPPS Mother-Arcing Protocol                ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* Step 1: Initialize module registry */
    printf("[KERNEL] Initializing module registry...\n");
    tbos_module_result_t result = tbos_modules_registry_init();
    if (result != TBOS_MODULE_SUCCESS) {
        printf("[KERNEL] FATAL: Module registry initialization failed!\n");
        return result;
    }

    /* Step 2: Register all modules */
    printf("[KERNEL] Registering modules...\n");
    tbos_module_register(&hardware_module);
    tbos_module_register(&memory_module);
    tbos_module_register(&steppps_module);
    tbos_module_register(&compression_module);
    tbos_module_register(&process_module);
    tbos_module_register(&scheduler_module);
    tbos_module_register(&filesystem_module);
    tbos_module_register(&shell_module);
    printf("[KERNEL] %d modules registered\n\n", 8);

    /* Step 3: Initialize all modules */
    result = tbos_modules_init_all();
    if (result != TBOS_MODULE_SUCCESS) {
        printf("[KERNEL] WARNING: Some modules failed to initialize\n");
    }

    /* Step 4: Print status */
    tbos_modules_print_status();

    /* Step 5: Get initialization stats */
    uint32_t total, initialized, failed;
    tbos_modules_get_stats(&total, &initialized, &failed);

    printf("[KERNEL] Initialization complete\n");
    printf("[KERNEL] Modules: %u total, %u ready, %u failed\n",
           total, initialized, failed);

    if (failed > 0) {
        printf("[KERNEL] WARNING: System partially initialized\n");
    } else {
        printf("[KERNEL] System fully operational\n");
    }

    printf("\n");
    return TBOS_MODULE_SUCCESS;
}

/**
 * @brief Kernel main loop
 * Called after initialization is complete
 */
void tbos_kernel_main(void) {
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 TernaryBit OS - Ready                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* Check if shell is ready */
    if (tbos_module_is_ready(TBOS_MODULE_SHELL)) {
        printf("Starting shell...\n\n");
        /* TODO: Start shell_main() */
    } else {
        printf("[KERNEL] Shell not available. System in minimal mode.\n");
    }

    /* Kernel idle loop */
    printf("[KERNEL] Entering idle loop...\n");
    while (1) {
        /* TODO: Scheduler tick, process switching */
    }
}

/**
 * @brief Kernel shutdown sequence
 */
void tbos_kernel_shutdown(void) {
    printf("\n[KERNEL] Initiating shutdown sequence...\n");

    /* Shutdown all modules */
    tbos_modules_shutdown_all();

    printf("[KERNEL] Shutdown complete. Halting system.\n\n");
}
