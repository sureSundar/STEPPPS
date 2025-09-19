/**
 * @file tbos_interfaces.h
 * @brief TernaryBit OS Core Interfaces - SOLID Principles Implementation
 *
 * Defines abstract interfaces following Interface Segregation Principle (ISP)
 * and Dependency Inversion Principle (DIP). Each interface has a single
 * responsibility (SRP) and is open for extension but closed for modification (OCP).
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef TBOS_INTERFACES_H
#define TBOS_INTERFACES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/**
 * @brief Common result type following DRY principle
 * Used across all TBOS operations for consistent error handling
 */
typedef struct tbos_result {
    bool success;
    int error_code;
    const char* error_message;
    void* data;
    size_t data_size;
} tbos_result_t;

/**
 * @brief Memory Management Interface (ISP)
 * Single responsibility: Memory allocation and management
 */
typedef struct {
    tbos_result_t (*allocate)(size_t size, uint32_t flags);
    tbos_result_t (*deallocate)(void* ptr);
    tbos_result_t (*reallocate)(void* ptr, size_t new_size);
    tbos_result_t (*get_stats)(void);
    tbos_result_t (*cleanup)(void);
} tbos_memory_interface_t;

/**
 * @brief Storage Interface (ISP)
 * Single responsibility: Block-level storage operations
 */
typedef struct {
    tbos_result_t (*read_block)(uint32_t block_num, void* buffer, size_t size);
    tbos_result_t (*write_block)(uint32_t block_num, const void* buffer, size_t size);
    tbos_result_t (*sync)(void);
    tbos_result_t (*get_capacity)(void);
} tbos_storage_interface_t;

/**
 * @brief File System Interface (ISP)
 * Single responsibility: High-level file operations
 */
typedef struct {
    tbos_result_t (*open)(const char* path, int flags, int mode);
    tbos_result_t (*close)(int fd);
    tbos_result_t (*read)(int fd, void* buffer, size_t size);
    tbos_result_t (*write)(int fd, const void* buffer, size_t size);
    tbos_result_t (*seek)(int fd, off_t offset, int whence);
    tbos_result_t (*stat)(const char* path);
    tbos_result_t (*mkdir)(const char* path, int mode);
    tbos_result_t (*rmdir)(const char* path);
    tbos_result_t (*unlink)(const char* path);
} tbos_filesystem_interface_t;

/**
 * @brief Process Management Interface (ISP)
 * Single responsibility: Process lifecycle management
 */
typedef struct {
    tbos_result_t (*create)(const char* name, void (*entry)(void*), void* args);
    tbos_result_t (*terminate)(pid_t pid, int exit_status);
    tbos_result_t (*wait)(pid_t pid, int* status, int options);
    tbos_result_t (*get_info)(pid_t pid);
    tbos_result_t (*set_priority)(pid_t pid, int priority);
    tbos_result_t (*send_signal)(pid_t pid, int signal);
} tbos_process_interface_t;

/**
 * @brief Scheduler Interface (ISP)
 * Single responsibility: Task scheduling
 */
typedef struct {
    tbos_result_t (*schedule_next)(void);
    tbos_result_t (*add_task)(void* task);
    tbos_result_t (*remove_task)(void* task);
    tbos_result_t (*yield)(void);
    tbos_result_t (*block)(void* task, const char* reason);
    tbos_result_t (*unblock)(void* task);
} tbos_scheduler_interface_t;

/**
 * @brief Hardware Abstraction Interface (ISP)
 * Single responsibility: Hardware interaction
 */
typedef struct {
    tbos_result_t (*detect_hardware)(void);
    tbos_result_t (*initialize_hardware)(void);
    tbos_result_t (*get_capabilities)(void);
    tbos_result_t (*power_management)(int state);
} tbos_hardware_interface_t;

/**
 * @brief Network Interface (ISP)
 * Single responsibility: Network operations
 */
typedef struct {
    tbos_result_t (*initialize)(void);
    tbos_result_t (*send)(const void* data, size_t size, const char* destination);
    tbos_result_t (*receive)(void* buffer, size_t buffer_size, char* source);
    tbos_result_t (*get_status)(void);
    tbos_result_t (*shutdown)(void);
} tbos_network_interface_t;

/**
 * @brief TBOS Core Context (Dependency Injection Container)
 * Implements Dependency Inversion Principle - high-level modules
 * depend on abstractions, not concretions
 */
typedef struct tbos_context {
    tbos_memory_interface_t* memory;
    tbos_storage_interface_t* storage;
    tbos_filesystem_interface_t* filesystem;
    tbos_process_interface_t* process;
    tbos_scheduler_interface_t* scheduler;
    tbos_hardware_interface_t* hardware;
    tbos_network_interface_t* network;

    // System state
    bool initialized;
    uint64_t boot_time;
    char system_info[256];
} tbos_context_t;

/**
 * @brief Factory Interface for creating implementations
 * Factory Method Pattern implementation
 */
typedef struct {
    tbos_memory_interface_t* (*create_memory_manager)(const char* type);
    tbos_storage_interface_t* (*create_storage_driver)(const char* type);
    tbos_filesystem_interface_t* (*create_filesystem)(const char* type);
    tbos_process_interface_t* (*create_process_manager)(const char* type);
    tbos_scheduler_interface_t* (*create_scheduler)(const char* type);
    tbos_hardware_interface_t* (*create_hardware_abstraction)(const char* type);
    tbos_network_interface_t* (*create_network_stack)(const char* type);
} tbos_factory_interface_t;

/**
 * @brief Observer Interface for system events
 * Observer Pattern implementation
 */
typedef struct {
    void (*on_system_event)(const char* event, void* data);
    void (*on_error)(int error_code, const char* message);
    void (*on_state_change)(const char* component, int old_state, int new_state);
} tbos_observer_interface_t;

// Global factory instance (Singleton Pattern)
extern tbos_factory_interface_t* g_tbos_factory;

// Core system context (Dependency Injection)
extern tbos_context_t* g_tbos_context;

// Utility functions following DRY principle
tbos_result_t tbos_create_success_result(void* data, size_t size);
tbos_result_t tbos_create_error_result(int code, const char* message);
void tbos_free_result(tbos_result_t* result);

// Context management (RAII pattern)
tbos_context_t* tbos_context_create(void);
void tbos_context_destroy(tbos_context_t* ctx);
tbos_result_t tbos_context_initialize(tbos_context_t* ctx);

// Observer pattern functions
tbos_result_t tbos_register_observer(tbos_observer_interface_t* observer);
tbos_result_t tbos_unregister_observer(tbos_observer_interface_t* observer);
void tbos_notify_observers(const char* event, void* data);

#endif // TBOS_INTERFACES_H