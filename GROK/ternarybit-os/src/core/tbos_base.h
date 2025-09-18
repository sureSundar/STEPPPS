/**
 * @file tbos_base.h
 * @brief TernaryBit OS Base Classes - Template Method Pattern
 *
 * Provides base implementations and common functionality following
 * the Template Method pattern. Implements DRY principle by providing
 * reusable components and KISS principle by keeping implementations simple.
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef TBOS_BASE_H
#define TBOS_BASE_H

#include "tbos_interfaces.h"
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

// Base class for all TBOS components (OOP in C)
typedef struct tbos_base {
    const char* component_name;
    uint32_t component_id;
    bool initialized;
    pthread_mutex_t mutex;
    time_t created_at;
    uint64_t operation_count;

    // Template methods (virtual functions in C)
    tbos_result_t (*initialize)(struct tbos_base* self);
    tbos_result_t (*cleanup)(struct tbos_base* self);
    tbos_result_t (*get_status)(struct tbos_base* self);
    void (*log_operation)(struct tbos_base* self, const char* operation);
} tbos_base_t;

/**
 * @brief Resource Manager Base Class
 * Template Method Pattern for resource management
 */
typedef struct tbos_resource_manager {
    tbos_base_t base;

    size_t resource_count;
    size_t max_resources;
    void** resources;

    // Template methods for resource management
    tbos_result_t (*acquire_resource)(struct tbos_resource_manager* self);
    tbos_result_t (*release_resource)(struct tbos_resource_manager* self, void* resource);
    tbos_result_t (*validate_resource)(struct tbos_resource_manager* self, void* resource);
} tbos_resource_manager_t;

/**
 * @brief Cache Base Class (Strategy Pattern for caching algorithms)
 */
typedef enum {
    TBOS_CACHE_LRU,
    TBOS_CACHE_LFU,
    TBOS_CACHE_FIFO
} tbos_cache_strategy_t;

typedef struct tbos_cache {
    tbos_base_t base;

    size_t cache_size;
    size_t max_size;
    tbos_cache_strategy_t strategy;

    // Strategy interface
    tbos_result_t (*put)(struct tbos_cache* self, const char* key, void* value, size_t size);
    tbos_result_t (*get)(struct tbos_cache* self, const char* key);
    tbos_result_t (*remove)(struct tbos_cache* self, const char* key);
    tbos_result_t (*clear)(struct tbos_cache* self);
} tbos_cache_t;

/**
 * @brief State Machine Base Class
 * State Pattern implementation for component states
 */
typedef enum {
    TBOS_STATE_UNINITIALIZED,
    TBOS_STATE_INITIALIZING,
    TBOS_STATE_RUNNING,
    TBOS_STATE_PAUSED,
    TBOS_STATE_ERROR,
    TBOS_STATE_SHUTDOWN
} tbos_state_t;

typedef struct tbos_state_machine {
    tbos_base_t base;

    tbos_state_t current_state;
    tbos_state_t previous_state;

    // State transition methods
    tbos_result_t (*transition_to)(struct tbos_state_machine* self, tbos_state_t new_state);
    bool (*can_transition)(struct tbos_state_machine* self, tbos_state_t from, tbos_state_t to);
    void (*on_state_enter)(struct tbos_state_machine* self, tbos_state_t state);
    void (*on_state_exit)(struct tbos_state_machine* self, tbos_state_t state);
} tbos_state_machine_t;

/**
 * @brief Configuration Manager (Singleton Pattern)
 * Manages system-wide configuration with type safety
 */
typedef struct tbos_config {
    // Memory configuration
    size_t default_heap_size;
    size_t max_heap_size;
    bool enable_memory_protection;

    // File system configuration
    size_t block_size;
    size_t max_open_files;
    bool enable_journaling;

    // Process configuration
    uint32_t max_processes;
    uint32_t default_time_slice;
    bool enable_preemption;

    // Network configuration
    char default_interface[64];
    uint16_t default_port;
    bool enable_encryption;

    // Debug configuration
    bool debug_mode;
    const char* log_level;
    const char* log_file;
} tbos_config_t;

// Base class constructors (Factory functions)
tbos_base_t* tbos_base_create(const char* name, uint32_t id);
void tbos_base_destroy(tbos_base_t* base);

tbos_resource_manager_t* tbos_resource_manager_create(const char* name, size_t max_resources);
void tbos_resource_manager_destroy(tbos_resource_manager_t* manager);

tbos_cache_t* tbos_cache_create(const char* name, size_t max_size, tbos_cache_strategy_t strategy);
void tbos_cache_destroy(tbos_cache_t* cache);

tbos_state_machine_t* tbos_state_machine_create(const char* name);
void tbos_state_machine_destroy(tbos_state_machine_t* machine);

// Configuration management (Singleton)
tbos_config_t* tbos_config_get_instance(void);
tbos_result_t tbos_config_load_from_file(const char* config_file);
tbos_result_t tbos_config_save_to_file(const char* config_file);

// Utility functions following DRY principle
tbos_result_t tbos_base_safe_operation(tbos_base_t* base,
                                       tbos_result_t (*operation)(tbos_base_t*));

// Thread-safe logging (Decorator Pattern)
typedef struct tbos_logger {
    pthread_mutex_t mutex;
    FILE* log_file;
    bool console_output;
    const char* log_level;

    void (*log)(struct tbos_logger* self, const char* level, const char* format, ...);
    void (*flush)(struct tbos_logger* self);
} tbos_logger_t;

// Global logger instance
extern tbos_logger_t* g_tbos_logger;

// Logger functions
tbos_logger_t* tbos_logger_create(const char* log_file, bool console_output);
void tbos_logger_destroy(tbos_logger_t* logger);

// Logging macros for convenience (DRY)
#define TBOS_LOG_ERROR(fmt, ...) \
    if (g_tbos_logger) g_tbos_logger->log(g_tbos_logger, "ERROR", fmt, ##__VA_ARGS__)
#define TBOS_LOG_WARN(fmt, ...) \
    if (g_tbos_logger) g_tbos_logger->log(g_tbos_logger, "WARN", fmt, ##__VA_ARGS__)
#define TBOS_LOG_INFO(fmt, ...) \
    if (g_tbos_logger) g_tbos_logger->log(g_tbos_logger, "INFO", fmt, ##__VA_ARGS__)
#define TBOS_LOG_DEBUG(fmt, ...) \
    if (g_tbos_logger) g_tbos_logger->log(g_tbos_logger, "DEBUG", fmt, ##__VA_ARGS__)

// Error handling utilities (Command Pattern)
typedef struct tbos_error_handler {
    tbos_result_t (*handle_error)(int error_code, const char* context);
    void (*register_error_callback)(int error_code, void (*callback)(const char*));
} tbos_error_handler_t;

extern tbos_error_handler_t* g_tbos_error_handler;

// Memory pool allocator (Object Pool Pattern)
typedef struct tbos_memory_pool {
    tbos_base_t base;
    void* pool_memory;
    size_t block_size;
    size_t total_blocks;
    size_t free_blocks;
    uint8_t* free_bitmap;

    void* (*allocate_block)(struct tbos_memory_pool* self);
    tbos_result_t (*free_block)(struct tbos_memory_pool* self, void* block);
} tbos_memory_pool_t;

tbos_memory_pool_t* tbos_memory_pool_create(size_t block_size, size_t num_blocks);
void tbos_memory_pool_destroy(tbos_memory_pool_t* pool);

#endif // TBOS_BASE_H