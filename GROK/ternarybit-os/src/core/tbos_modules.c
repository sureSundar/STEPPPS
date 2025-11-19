/**
 * @file tbos_modules.c
 * @brief TernaryBit OS Module Registry Implementation
 */

#include "tbos_modules.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static tbos_module_descriptor_t g_modules[TBOS_MODULE_COUNT];
static bool g_registry_initialized = false;
static uint32_t g_registered_count = 0;

/* ========================================================================= */
/* REGISTRY INITIALIZATION                                                   */
/* ========================================================================= */

tbos_module_result_t tbos_modules_registry_init(void) {
    if (g_registry_initialized) {
        return TBOS_MODULE_SUCCESS;  /* Already initialized */
    }

    /* Clear all module descriptors */
    memset(g_modules, 0, sizeof(g_modules));

    /* Set all modules to UNINITIALIZED */
    for (int i = 0; i < TBOS_MODULE_COUNT; i++) {
        g_modules[i].id = i;
        g_modules[i].status = TBOS_MODULE_STATUS_UNINITIALIZED;
    }

    g_registry_initialized = true;
    g_registered_count = 0;

    return TBOS_MODULE_SUCCESS;
}

/* ========================================================================= */
/* MODULE REGISTRATION                                                       */
/* ========================================================================= */

tbos_module_result_t tbos_module_register(const tbos_module_descriptor_t* descriptor) {
    if (!g_registry_initialized) {
        return TBOS_MODULE_ERROR;
    }

    if (!descriptor || descriptor->id >= TBOS_MODULE_COUNT) {
        return TBOS_MODULE_ERROR_INVALID_PARAM;
    }

    /* Copy descriptor to registry */
    memcpy(&g_modules[descriptor->id], descriptor, sizeof(tbos_module_descriptor_t));
    g_registered_count++;

    return TBOS_MODULE_SUCCESS;
}

/* ========================================================================= */
/* MODULE INITIALIZATION                                                     */
/* ========================================================================= */

static bool check_dependencies(tbos_module_id_t id) {
    tbos_module_descriptor_t* module = &g_modules[id];

    if (!module->dependencies || module->dependency_count == 0) {
        return true;  /* No dependencies */
    }

    /* Check all dependencies are ready */
    for (uint8_t i = 0; i < module->dependency_count; i++) {
        tbos_module_id_t dep_id = module->dependencies[i];
        if (g_modules[dep_id].status != TBOS_MODULE_STATUS_READY &&
            g_modules[dep_id].status != TBOS_MODULE_STATUS_RUNNING) {
            return false;  /* Dependency not ready */
        }
    }

    return true;
}

tbos_module_result_t tbos_module_init(tbos_module_id_t id) {
    if (!g_registry_initialized || id >= TBOS_MODULE_COUNT) {
        return TBOS_MODULE_ERROR_INVALID_PARAM;
    }

    tbos_module_descriptor_t* module = &g_modules[id];

    /* Check if already initialized */
    if (module->status == TBOS_MODULE_STATUS_READY ||
        module->status == TBOS_MODULE_STATUS_RUNNING) {
        return TBOS_MODULE_SUCCESS;  /* Already ready */
    }

    /* Check dependencies */
    if (!check_dependencies(id)) {
        printf("[MODULE] %s: Dependencies not met\n", module->name);
        return TBOS_MODULE_ERROR;
    }

    /* Set initializing status */
    module->status = TBOS_MODULE_STATUS_INITIALIZING;

    /* Call init callback */
    if (module->init) {
        tbos_module_result_t result = module->init();
        if (result != TBOS_MODULE_SUCCESS) {
            printf("[MODULE] %s: Initialization FAILED (code: %d)\n",
                   module->name, result);
            module->status = TBOS_MODULE_STATUS_ERROR;
            module->error_count++;
            return result;
        }
    }

    /* Mark as ready */
    module->status = TBOS_MODULE_STATUS_READY;
    printf("[MODULE] %s: Initialized successfully\n", module->name);

    return TBOS_MODULE_SUCCESS;
}

tbos_module_result_t tbos_modules_init_all(void) {
    if (!g_registry_initialized) {
        return TBOS_MODULE_ERROR;
    }

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║       TernaryBit OS - Module Initialization              ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    /* Initialize in priority order */
    for (int priority = TBOS_MODULE_PRIORITY_CRITICAL;
         priority <= TBOS_MODULE_PRIORITY_LOW;
         priority++) {

        printf("[INIT] Priority %d modules:\n", priority);

        for (tbos_module_id_t id = 0; id < TBOS_MODULE_COUNT; id++) {
            tbos_module_descriptor_t* module = &g_modules[id];

            /* Skip if different priority or no name */
            if (module->priority != priority || !module->name) {
                continue;
            }

            /* Try to initialize */
            tbos_module_result_t result = tbos_module_init(id);
            if (result != TBOS_MODULE_SUCCESS) {
                printf("[INIT] FAILED to initialize %s\n", module->name);
                /* Continue with other modules */
            }
        }
        printf("\n");
    }

    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║       Module Initialization Complete                     ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    return TBOS_MODULE_SUCCESS;
}

/* ========================================================================= */
/* MODULE STATUS                                                             */
/* ========================================================================= */

tbos_module_status_t tbos_module_get_status(tbos_module_id_t id) {
    if (!g_registry_initialized || id >= TBOS_MODULE_COUNT) {
        return TBOS_MODULE_STATUS_ERROR;
    }

    return g_modules[id].status;
}

const tbos_module_descriptor_t* tbos_module_get_descriptor(tbos_module_id_t id) {
    if (!g_registry_initialized || id >= TBOS_MODULE_COUNT) {
        return NULL;
    }

    return &g_modules[id];
}

const char* tbos_module_get_name(tbos_module_id_t id) {
    if (!g_registry_initialized || id >= TBOS_MODULE_COUNT) {
        return "UNKNOWN";
    }

    return g_modules[id].name ? g_modules[id].name : "UNNAMED";
}

bool tbos_module_is_ready(tbos_module_id_t id) {
    if (!g_registry_initialized || id >= TBOS_MODULE_COUNT) {
        return false;
    }

    tbos_module_status_t status = g_modules[id].status;
    return (status == TBOS_MODULE_STATUS_READY ||
            status == TBOS_MODULE_STATUS_RUNNING);
}

/* ========================================================================= */
/* MODULE SHUTDOWN                                                           */
/* ========================================================================= */

tbos_module_result_t tbos_module_shutdown(tbos_module_id_t id) {
    if (!g_registry_initialized || id >= TBOS_MODULE_COUNT) {
        return TBOS_MODULE_ERROR_INVALID_PARAM;
    }

    tbos_module_descriptor_t* module = &g_modules[id];

    /* Call shutdown callback */
    if (module->shutdown) {
        tbos_module_result_t result = module->shutdown();
        if (result != TBOS_MODULE_SUCCESS) {
            printf("[MODULE] %s: Shutdown FAILED\n", module->name);
            return result;
        }
    }

    module->status = TBOS_MODULE_STATUS_SHUTDOWN;
    printf("[MODULE] %s: Shut down\n", module->name);

    return TBOS_MODULE_SUCCESS;
}

tbos_module_result_t tbos_modules_shutdown_all(void) {
    if (!g_registry_initialized) {
        return TBOS_MODULE_ERROR;
    }

    printf("\n[SHUTDOWN] Shutting down all modules...\n");

    /* Shutdown in reverse priority order */
    for (int priority = TBOS_MODULE_PRIORITY_LOW;
         priority >= TBOS_MODULE_PRIORITY_CRITICAL;
         priority--) {

        for (tbos_module_id_t id = 0; id < TBOS_MODULE_COUNT; id++) {
            if (g_modules[id].priority == priority &&
                g_modules[id].name != NULL) {
                tbos_module_shutdown(id);
            }
        }
    }

    printf("[SHUTDOWN] All modules shut down\n\n");
    return TBOS_MODULE_SUCCESS;
}

/* ========================================================================= */
/* MODULE DIAGNOSTICS                                                        */
/* ========================================================================= */

static const char* status_to_string(tbos_module_status_t status) {
    switch (status) {
        case TBOS_MODULE_STATUS_UNINITIALIZED: return "UNINIT";
        case TBOS_MODULE_STATUS_INITIALIZING:  return "INIT..";
        case TBOS_MODULE_STATUS_READY:         return "READY ";
        case TBOS_MODULE_STATUS_RUNNING:       return "RUNNING";
        case TBOS_MODULE_STATUS_PAUSED:        return "PAUSED";
        case TBOS_MODULE_STATUS_ERROR:         return "ERROR ";
        case TBOS_MODULE_STATUS_SHUTDOWN:      return "SHUTDOWN";
        default:                               return "UNKNOWN";
    }
}

void tbos_modules_print_status(void) {
    if (!g_registry_initialized) {
        printf("Module registry not initialized\n");
        return;
    }

    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║              TernaryBit OS - Module Status                     ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║ ID │ Module Name         │ Priority │ Status   │ Errors      ║\n");
    printf("╠════╪═════════════════════╪══════════╪══════════╪═════════════╣\n");

    for (tbos_module_id_t id = 0; id < TBOS_MODULE_COUNT; id++) {
        tbos_module_descriptor_t* module = &g_modules[id];

        if (module->name) {
            printf("║ %2d │ %-19s │    %d     │ %-8s │ %6lu      ║\n",
                   id,
                   module->name,
                   module->priority,
                   status_to_string(module->status),
                   (unsigned long)module->error_count);
        }
    }

    printf("╚════╧═════════════════════╧══════════╧══════════╧═════════════╝\n");
    printf("\n");
}

void tbos_modules_get_stats(uint32_t* total_modules,
                            uint32_t* initialized_modules,
                            uint32_t* failed_modules) {
    *total_modules = 0;
    *initialized_modules = 0;
    *failed_modules = 0;

    if (!g_registry_initialized) {
        return;
    }

    for (tbos_module_id_t id = 0; id < TBOS_MODULE_COUNT; id++) {
        if (g_modules[id].name) {
            (*total_modules)++;

            if (g_modules[id].status == TBOS_MODULE_STATUS_READY ||
                g_modules[id].status == TBOS_MODULE_STATUS_RUNNING) {
                (*initialized_modules)++;
            }

            if (g_modules[id].status == TBOS_MODULE_STATUS_ERROR) {
                (*failed_modules)++;
            }
        }
    }
}
