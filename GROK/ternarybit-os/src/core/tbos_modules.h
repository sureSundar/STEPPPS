/**
 * @file tbos_modules.h
 * @brief TernaryBit OS Module Registry System
 *
 * Central module management system for TBOS initialization, status tracking,
 * and lifecycle management. All core subsystems register here.
 *
 * @version 1.0
 * @date 2025-11-03
 * @author TernaryBit Development Team
 */

#ifndef TBOS_MODULES_H
#define TBOS_MODULES_H

#include <stdint.h>
#include <stdbool.h>

/* Module system uses simple integer result codes */
typedef int tbos_module_result_t;
#define TBOS_MODULE_SUCCESS              0
#define TBOS_MODULE_ERROR               -1
#define TBOS_MODULE_ERROR_INVALID_PARAM -2

/* ========================================================================= */
/* MODULE DEFINITIONS                                                        */
/* ========================================================================= */

/**
 * @brief TBOS Core Modules
 * Each module represents a major subsystem of the operating system
 */
typedef enum {
    TBOS_MODULE_HARDWARE,      /**< Hardware detection and HAL */
    TBOS_MODULE_MEMORY,        /**< Memory management */
    TBOS_MODULE_STEPPPS,       /**< STEPPPS framework */
    TBOS_MODULE_COMPRESSION,   /**< PXFS compression */
    TBOS_MODULE_PROCESS,       /**< Process management */
    TBOS_MODULE_SCHEDULER,     /**< Process scheduler */
    TBOS_MODULE_FILESYSTEM,    /**< File system */
    TBOS_MODULE_NETWORK,       /**< Network stack */
    TBOS_MODULE_IPC,           /**< Inter-process communication */
    TBOS_MODULE_SHELL,         /**< Command shell */
    TBOS_MODULE_COUNT          /**< Total number of modules */
} tbos_module_id_t;

/**
 * @brief Module Status
 */
typedef enum {
    TBOS_MODULE_STATUS_UNINITIALIZED,  /**< Not yet initialized */
    TBOS_MODULE_STATUS_INITIALIZING,   /**< Currently initializing */
    TBOS_MODULE_STATUS_READY,          /**< Initialized and ready */
    TBOS_MODULE_STATUS_RUNNING,        /**< Active and processing */
    TBOS_MODULE_STATUS_PAUSED,         /**< Temporarily paused */
    TBOS_MODULE_STATUS_ERROR,          /**< Error state */
    TBOS_MODULE_STATUS_SHUTDOWN        /**< Shut down */
} tbos_module_status_t;

/**
 * @brief Module Priority (for initialization order)
 */
typedef enum {
    TBOS_MODULE_PRIORITY_CRITICAL = 0,  /**< Must init first (hardware, memory) */
    TBOS_MODULE_PRIORITY_HIGH = 1,      /**< Core services (steppps, process) */
    TBOS_MODULE_PRIORITY_MEDIUM = 2,    /**< I/O subsystems (fs, network) */
    TBOS_MODULE_PRIORITY_LOW = 3        /**< User-facing (shell, ui) */
} tbos_module_priority_t;

/**
 * @brief Module Descriptor
 * Contains all information about a registered module
 */
typedef struct {
    tbos_module_id_t id;                    /**< Module identifier */
    const char* name;                       /**< Human-readable name */
    const char* description;                /**< Module description */
    tbos_module_priority_t priority;        /**< Initialization priority */
    tbos_module_status_t status;            /**< Current status */

    /* Lifecycle callbacks */
    tbos_module_result_t (*init)(void);            /**< Initialization function */
    tbos_module_result_t (*shutdown)(void);        /**< Shutdown function */
    tbos_module_result_t (*suspend)(void);         /**< Suspend/pause function */
    tbos_module_result_t (*resume)(void);          /**< Resume function */

    /* Dependencies */
    tbos_module_id_t* dependencies;         /**< Array of required modules */
    uint8_t dependency_count;               /**< Number of dependencies */

    /* Statistics */
    uint64_t init_timestamp;                /**< When initialized */
    uint64_t error_count;                   /**< Number of errors */
    void* module_data;                      /**< Module-specific data */
} tbos_module_descriptor_t;

/* ========================================================================= */
/* MODULE REGISTRY API                                                       */
/* ========================================================================= */

/**
 * @brief Initialize the module registry system
 * Must be called before any other module operations
 *
 * @return TBOS_SUCCESS on success, error code otherwise
 */
tbos_module_result_t tbos_modules_registry_init(void);

/**
 * @brief Register a module with the system
 *
 * @param descriptor Module descriptor with callbacks and metadata
 * @return TBOS_SUCCESS on success, error code otherwise
 */
tbos_module_result_t tbos_module_register(const tbos_module_descriptor_t* descriptor);

/**
 * @brief Initialize a specific module
 * Checks dependencies and calls the module's init callback
 *
 * @param id Module to initialize
 * @return TBOS_SUCCESS on success, error code otherwise
 */
tbos_module_result_t tbos_module_init(tbos_module_id_t id);

/**
 * @brief Initialize all registered modules
 * Initializes modules in priority order, respecting dependencies
 *
 * @return TBOS_SUCCESS if all modules initialized, error otherwise
 */
tbos_module_result_t tbos_modules_init_all(void);

/**
 * @brief Get module status
 *
 * @param id Module identifier
 * @return Current module status
 */
tbos_module_status_t tbos_module_get_status(tbos_module_id_t id);

/**
 * @brief Get module descriptor
 *
 * @param id Module identifier
 * @return Pointer to module descriptor, or NULL if not found
 */
const tbos_module_descriptor_t* tbos_module_get_descriptor(tbos_module_id_t id);

/**
 * @brief Get module name
 *
 * @param id Module identifier
 * @return Module name string
 */
const char* tbos_module_get_name(tbos_module_id_t id);

/**
 * @brief Check if module is ready
 *
 * @param id Module identifier
 * @return true if module is in READY or RUNNING state
 */
bool tbos_module_is_ready(tbos_module_id_t id);

/**
 * @brief Shutdown a specific module
 *
 * @param id Module to shutdown
 * @return TBOS_SUCCESS on success, error code otherwise
 */
tbos_module_result_t tbos_module_shutdown(tbos_module_id_t id);

/**
 * @brief Shutdown all modules
 * Shuts down in reverse priority order
 *
 * @return TBOS_SUCCESS if all modules shut down cleanly
 */
tbos_module_result_t tbos_modules_shutdown_all(void);

/**
 * @brief Print module status table
 * Useful for debugging and system monitoring
 */
void tbos_modules_print_status(void);

/**
 * @brief Get initialization statistics
 *
 * @param total_modules Output: total registered modules
 * @param initialized_modules Output: number initialized
 * @param failed_modules Output: number failed
 */
void tbos_modules_get_stats(uint32_t* total_modules,
                            uint32_t* initialized_modules,
                            uint32_t* failed_modules);

/* ========================================================================= */
/* MODULE HELPER MACROS                                                      */
/* ========================================================================= */

/**
 * @brief Define a module with standard callbacks
 */
#define TBOS_MODULE_DEFINE(id, name, desc, priority, init_fn, shutdown_fn) \
    { \
        .id = id, \
        .name = name, \
        .description = desc, \
        .priority = priority, \
        .status = TBOS_MODULE_STATUS_UNINITIALIZED, \
        .init = init_fn, \
        .shutdown = shutdown_fn, \
        .suspend = NULL, \
        .resume = NULL, \
        .dependencies = NULL, \
        .dependency_count = 0, \
        .init_timestamp = 0, \
        .error_count = 0, \
        .module_data = NULL \
    }

/**
 * @brief Define a module with dependencies
 */
#define TBOS_MODULE_DEFINE_WITH_DEPS(id, name, desc, priority, init_fn, shutdown_fn, deps, dep_count) \
    { \
        .id = id, \
        .name = name, \
        .description = desc, \
        .priority = priority, \
        .status = TBOS_MODULE_STATUS_UNINITIALIZED, \
        .init = init_fn, \
        .shutdown = shutdown_fn, \
        .suspend = NULL, \
        .resume = NULL, \
        .dependencies = deps, \
        .dependency_count = dep_count, \
        .init_timestamp = 0, \
        .error_count = 0, \
        .module_data = NULL \
    }

#endif /* TBOS_MODULES_H */
