/**
 * @file tbos_core_impl.c
 * @brief TernaryBit OS Core Implementation - Design Patterns in Action
 *
 * Implements core TBOS functionality using proper design patterns:
 * - Singleton Pattern: Configuration and system instances
 * - Factory Pattern: Component creation
 * - Observer Pattern: Event notification
 * - Strategy Pattern: Algorithm selection
 * - Adapter Pattern: Interface adaptation
 * - Facade Pattern: Simplified high-level interface
 * - Command Pattern: Operation encapsulation
 * - State Pattern: System state management
 * - Template Method: Common algorithms
 * - Decorator Pattern: Feature enhancement
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#include "tbos_interfaces.h"
#include "tbos_base.h"
#include "tbos_filesystem_v2.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

// Global instances (Singleton Pattern)
static tbos_context_t* g_system_context = NULL;
static tbos_factory_interface_t* g_factory = NULL;
static tbos_logger_t* g_logger = NULL;
// Global logger used by TBOS_LOG_* macros declared in tbos_base.h
tbos_logger_t* g_tbos_logger = NULL;
static tbos_config_t* g_config = NULL;
static pthread_mutex_t g_init_mutex = PTHREAD_MUTEX_INITIALIZER;

// Observer list for system events
static tbos_observer_interface_t* g_observers[32];
static size_t g_observer_count = 0;
static pthread_mutex_t g_observer_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Result utility functions (DRY Principle)
 */
tbos_result_t tbos_create_success_result(void* data, size_t size) {
    return (tbos_result_t) {
        .success = true,
        .error_code = 0,
        .error_message = NULL,
        .data = data,
        .data_size = size
    };
}

tbos_result_t tbos_create_error_result(int code, const char* message) {
    return (tbos_result_t) {
        .success = false,
        .error_code = code,
        .error_message = message,
        .data = NULL,
        .data_size = 0
    };
}

void tbos_free_result(tbos_result_t* result) {
    if (result && result->data) {
        free(result->data);
        result->data = NULL;
        result->data_size = 0;
    }
}

/**
 * @brief Memory Manager Implementation (Strategy Pattern)
 */
typedef struct tbos_memory_manager_impl {
    tbos_memory_interface_t interface;
    tbos_resource_manager_t* resource_manager;
    tbos_memory_pool_t* small_pool;
    tbos_memory_pool_t* large_pool;
    tbos_cache_t* allocation_cache;

    // Statistics
    size_t total_allocated;
    size_t peak_usage;
    uint64_t allocation_count;
} tbos_memory_manager_impl_t;

static tbos_result_t memory_allocate(size_t size, uint32_t flags) {
    tbos_memory_manager_impl_t* manager =
        (tbos_memory_manager_impl_t*)g_system_context->memory;

    if (!manager) {
        return tbos_create_error_result(EINVAL, "Memory manager not initialized");
    }

    void* ptr = NULL;

    // Strategy: Use memory pool for small allocations, direct allocation for large ones
    if (size <= 1024) {
        ptr = manager->small_pool->allocate_block(manager->small_pool);
    } else if (size <= 65536) {
        ptr = manager->large_pool->allocate_block(manager->large_pool);
    } else {
        ptr = malloc(size);
    }

    if (!ptr) {
        TBOS_LOG_ERROR("Memory allocation failed for size %zu", size);
        return tbos_create_error_result(ENOMEM, "Out of memory");
    }

    // Update statistics
    manager->total_allocated += size;
    manager->allocation_count++;
    if (manager->total_allocated > manager->peak_usage) {
        manager->peak_usage = manager->total_allocated;
    }

    TBOS_LOG_DEBUG("Allocated %zu bytes at %p", size, ptr);
    return tbos_create_success_result(ptr, size);
}

static tbos_result_t memory_deallocate(void* ptr) {
    if (!ptr) {
        return tbos_create_error_result(EINVAL, "Null pointer");
    }

    tbos_memory_manager_impl_t* manager =
        (tbos_memory_manager_impl_t*)g_system_context->memory;

    // Try to free from pools first, fallback to regular free
    if (manager->small_pool->free_block(manager->small_pool, ptr).success ||
        manager->large_pool->free_block(manager->large_pool, ptr).success) {
        TBOS_LOG_DEBUG("Freed memory from pool: %p", ptr);
    } else {
        free(ptr);
        TBOS_LOG_DEBUG("Freed memory directly: %p", ptr);
    }

    return tbos_create_success_result(NULL, 0);
}

static tbos_result_t memory_get_stats(void) {
    tbos_memory_manager_impl_t* manager =
        (tbos_memory_manager_impl_t*)g_system_context->memory;

    typedef struct {
        size_t total_allocated;
        size_t peak_usage;
        uint64_t allocation_count;
        size_t small_pool_free;
        size_t large_pool_free;
    } memory_stats_t;

    memory_stats_t* stats = malloc(sizeof(memory_stats_t));
    if (!stats) {
        return tbos_create_error_result(ENOMEM, "Cannot allocate stats");
    }

    stats->total_allocated = manager->total_allocated;
    stats->peak_usage = manager->peak_usage;
    stats->allocation_count = manager->allocation_count;
    stats->small_pool_free = manager->small_pool->free_blocks;
    stats->large_pool_free = manager->large_pool->free_blocks;

    return tbos_create_success_result(stats, sizeof(memory_stats_t));
}

/**
 * @brief Factory Implementation (Factory Pattern + Abstract Factory)
 */
static tbos_memory_interface_t* create_memory_manager(const char* type) {
    tbos_memory_manager_impl_t* manager = malloc(sizeof(tbos_memory_manager_impl_t));
    if (!manager) {
        TBOS_LOG_ERROR("Failed to allocate memory manager");
        return NULL;
    }

    // Initialize interface (Strategy Pattern)
    manager->interface.allocate = memory_allocate;
    manager->interface.deallocate = memory_deallocate;
    manager->interface.reallocate = NULL; // TODO: Implement
    manager->interface.get_stats = memory_get_stats;
    manager->interface.cleanup = NULL; // TODO: Implement

    // Create memory pools based on type
    if (strcmp(type, "standard") == 0) {
        manager->small_pool = tbos_memory_pool_create(1024, 1000);
        manager->large_pool = tbos_memory_pool_create(65536, 100);
    } else if (strcmp(type, "embedded") == 0) {
        manager->small_pool = tbos_memory_pool_create(512, 100);
        manager->large_pool = tbos_memory_pool_create(4096, 10);
    } else {
        // Default configuration
        manager->small_pool = tbos_memory_pool_create(1024, 500);
        manager->large_pool = tbos_memory_pool_create(32768, 50);
    }

    // Initialize statistics
    manager->total_allocated = 0;
    manager->peak_usage = 0;
    manager->allocation_count = 0;

    TBOS_LOG_INFO("Created %s memory manager", type);
    return &manager->interface;
}

static tbos_filesystem_interface_t* create_filesystem(const char* type) {
    if (strcmp(type, "tbos_v2") == 0) {
        tbos_filesystem_v2_t* fs = tbos_create_simple_filesystem("/dev/tbos_disk");
        if (!fs) {
            TBOS_LOG_ERROR("Failed to create TBOS v2 filesystem");
            return NULL;
        }

        // Use Adapter Pattern to convert to interface
        tbos_filesystem_adapter_t* adapter = tbos_filesystem_adapter_create(fs);
        if (!adapter) {
            TBOS_LOG_ERROR("Failed to create filesystem adapter");
            return NULL;
        }

        TBOS_LOG_INFO("Created TBOS v2 filesystem");
        return &adapter->interface;
    }

    TBOS_LOG_WARN("Unknown filesystem type: %s", type);
    return NULL;
}

// Factory interface implementation
static tbos_factory_interface_t g_factory_impl = {
    .create_memory_manager = create_memory_manager,
    .create_storage_driver = NULL, // TODO: Implement
    .create_filesystem = create_filesystem,
    .create_process_manager = NULL, // TODO: Implement
    .create_scheduler = NULL, // TODO: Implement
    .create_hardware_abstraction = NULL, // TODO: Implement
    .create_network_stack = NULL // TODO: Implement
};

/**
 * @brief Observer Pattern Implementation
 */
tbos_result_t tbos_register_observer(tbos_observer_interface_t* observer) {
    if (!observer) {
        return tbos_create_error_result(EINVAL, "Null observer");
    }

    pthread_mutex_lock(&g_observer_mutex);

    if (g_observer_count >= sizeof(g_observers) / sizeof(g_observers[0])) {
        pthread_mutex_unlock(&g_observer_mutex);
        return tbos_create_error_result(ENOSPC, "Observer list full");
    }

    g_observers[g_observer_count++] = observer;
    pthread_mutex_unlock(&g_observer_mutex);

    TBOS_LOG_DEBUG("Registered observer (total: %zu)", g_observer_count);
    return tbos_create_success_result(NULL, 0);
}

tbos_result_t tbos_unregister_observer(tbos_observer_interface_t* observer) {
    if (!observer) {
        return tbos_create_error_result(EINVAL, "Null observer");
    }

    pthread_mutex_lock(&g_observer_mutex);

    for (size_t i = 0; i < g_observer_count; i++) {
        if (g_observers[i] == observer) {
            // Shift remaining observers
            memmove(&g_observers[i], &g_observers[i + 1],
                   (g_observer_count - i - 1) * sizeof(tbos_observer_interface_t*));
            g_observer_count--;
            pthread_mutex_unlock(&g_observer_mutex);

            TBOS_LOG_DEBUG("Unregistered observer (remaining: %zu)", g_observer_count);
            return tbos_create_success_result(NULL, 0);
        }
    }

    pthread_mutex_unlock(&g_observer_mutex);
    return tbos_create_error_result(ENOENT, "Observer not found");
}

void tbos_notify_observers(const char* event, void* data) {
    pthread_mutex_lock(&g_observer_mutex);

    for (size_t i = 0; i < g_observer_count; i++) {
        if (g_observers[i] && g_observers[i]->on_system_event) {
            g_observers[i]->on_system_event(event, data);
        }
    }

    pthread_mutex_unlock(&g_observer_mutex);
}

/**
 * @brief Configuration Management (Singleton Pattern)
 */
static tbos_config_t g_config_instance = {
    .default_heap_size = 1024 * 1024,  // 1MB
    .max_heap_size = 64 * 1024 * 1024, // 64MB
    .enable_memory_protection = true,
    .block_size = 4096,
    .max_open_files = 1024,
    .enable_journaling = true,
    .max_processes = 1024,
    .default_time_slice = 10, // 10ms
    .enable_preemption = true,
    .debug_mode = false,
    .log_level = "INFO",
    .log_file = "/var/log/tbos.log"
};

tbos_config_t* tbos_config_get_instance(void) {
    return &g_config_instance;
}

/**
 * @brief Logger Implementation (Decorator Pattern)
 */
static void logger_log(tbos_logger_t* self, const char* level, const char* format, ...) {
    if (!self || !format) return;

    pthread_mutex_lock(&self->mutex);

    // Get current time
    time_t now;
    time(&now);
    struct tm* tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Format message
    va_list args;
    va_start(args, format);

    char message[1024];
    vsnprintf(message, sizeof(message), format, args);

    va_end(args);

    // Write to file if available
    if (self->log_file) {
        fprintf(self->log_file, "[%s] %s: %s\n", timestamp, level, message);
        fflush(self->log_file);
    }

    // Write to console if enabled
    if (self->console_output) {
        printf("[%s] %s: %s\n", timestamp, level, message);
        fflush(stdout);
    }

    pthread_mutex_unlock(&self->mutex);
}

static void logger_flush(tbos_logger_t* self) {
    if (!self) return;

    pthread_mutex_lock(&self->mutex);
    if (self->log_file) {
        fflush(self->log_file);
    }
    pthread_mutex_unlock(&self->mutex);
}

tbos_logger_t* tbos_logger_create(const char* log_file, bool console_output) {
    tbos_logger_t* logger = malloc(sizeof(tbos_logger_t));
    if (!logger) return NULL;

    pthread_mutex_init(&logger->mutex, NULL);
    logger->console_output = console_output;
    logger->log_level = "INFO";
    logger->log = logger_log;
    logger->flush = logger_flush;

    if (log_file) {
        logger->log_file = fopen(log_file, "a");
        if (!logger->log_file) {
            printf("Warning: Could not open log file %s\n", log_file);
        }
    } else {
        logger->log_file = NULL;
    }

    return logger;
}

void tbos_logger_destroy(tbos_logger_t* logger) {
    if (!logger) return;

    pthread_mutex_lock(&logger->mutex);
    if (logger->log_file) {
        fclose(logger->log_file);
    }
    pthread_mutex_unlock(&logger->mutex);

    pthread_mutex_destroy(&logger->mutex);
    free(logger);
}

/**
 * @brief Context Management (RAII Pattern)
 */
tbos_context_t* tbos_context_create(void) {
    pthread_mutex_lock(&g_init_mutex);

    if (g_system_context) {
        pthread_mutex_unlock(&g_init_mutex);
        return g_system_context; // Singleton
    }

    tbos_context_t* ctx = malloc(sizeof(tbos_context_t));
    if (!ctx) {
        pthread_mutex_unlock(&g_init_mutex);
        return NULL;
    }

    memset(ctx, 0, sizeof(tbos_context_t));

    // Initialize timestamp
    ctx->boot_time = (uint64_t)time(NULL);
    strcpy(ctx->system_info, "TernaryBit OS v2.0 - Standards Compliant");

    g_system_context = ctx;
    pthread_mutex_unlock(&g_init_mutex);

    return ctx;
}

tbos_result_t tbos_context_initialize(tbos_context_t* ctx) {
    if (!ctx) {
        return tbos_create_error_result(EINVAL, "Null context");
    }

    // Initialize global logger
    g_logger = tbos_logger_create(g_config_instance.log_file, true);
    if (!g_logger) {
        return tbos_create_error_result(ENOMEM, "Failed to create logger");
    }
    // Wire global logger for macros
    g_tbos_logger = g_logger;

    TBOS_LOG_INFO("Initializing TernaryBit OS Core...");

    // Initialize factory
    g_factory = &g_factory_impl;

    // Create core components using factory
    ctx->memory = g_factory->create_memory_manager("standard");
    if (!ctx->memory) {
        TBOS_LOG_ERROR("Failed to create memory manager");
        return tbos_create_error_result(ENOMEM, "Memory manager creation failed");
    }

    ctx->filesystem = g_factory->create_filesystem("tbos_v2");
    if (!ctx->filesystem) {
        TBOS_LOG_WARN("Failed to create filesystem, using fallback");
    }

    ctx->initialized = true;

    // Notify observers
    tbos_notify_observers("system_initialized", ctx);

    TBOS_LOG_INFO("TernaryBit OS Core initialized successfully");
    return tbos_create_success_result(ctx, sizeof(tbos_context_t));
}

void tbos_context_destroy(tbos_context_t* ctx) {
    if (!ctx) return;

    TBOS_LOG_INFO("Shutting down TernaryBit OS Core...");

    // Notify observers
    tbos_notify_observers("system_shutdown", ctx);

    // Cleanup components
    if (ctx->memory && ctx->memory->cleanup) {
        ctx->memory->cleanup();
    }

    // Cleanup logger
    if (g_logger) {
        tbos_logger_destroy(g_logger);
        g_logger = NULL;
    }

    free(ctx);
    g_system_context = NULL;
}

// Global accessor functions
tbos_factory_interface_t* tbos_get_factory(void) {
    return g_factory;
}

tbos_context_t* tbos_get_context(void) {
    return g_system_context;
}