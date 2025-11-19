/**
 * @file tbos_process_v2.h
 * @brief TernaryBit OS Process Management - SOLID Principles Implementation
 *
 * Refactored process management following SOLID principles and design patterns:
 * - Single Responsibility: Separate concerns into focused classes
 * - Open/Closed: Extensible without modification
 * - Liskov Substitution: Interchangeable implementations
 * - Interface Segregation: Minimal, focused interfaces
 * - Dependency Inversion: Depend on abstractions
 *
 * Design Patterns Used:
 * - Strategy: Different scheduling algorithms
 * - State: Process state management
 * - Observer: Process event notification
 * - Command: Process operations
 * - Factory: Process creation
 * - Template Method: Common process operations
 * - Object Pool: Process and thread pooling
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef TBOS_PROCESS_V2_H
#define TBOS_PROCESS_V2_H

#include "tbos_interfaces.h"
#include "tbos_base.h"
#include <signal.h>

/**
 * @brief Process State Interface (State Pattern)
 * Single responsibility: Process state management
 */
typedef struct tbos_process_state {
    const char* name;
    int state_id;

    // State behavior methods
    tbos_result_t (*enter)(struct tbos_process_state* self, void* process);
    tbos_result_t (*exit)(struct tbos_process_state* self, void* process);
    tbos_result_t (*handle_event)(struct tbos_process_state* self, void* process, int event);
    bool (*can_transition_to)(struct tbos_process_state* self, struct tbos_process_state* new_state);
} tbos_process_state_t;

// Process states (Singleton instances)
extern tbos_process_state_t* tbos_state_created;
extern tbos_process_state_t* tbos_state_ready;
extern tbos_process_state_t* tbos_state_running;
extern tbos_process_state_t* tbos_state_blocked;
extern tbos_process_state_t* tbos_state_sleeping;
extern tbos_process_state_t* tbos_state_zombie;
extern tbos_process_state_t* tbos_state_terminated;

/**
 * @brief Scheduling Strategy Interface (Strategy Pattern)
 * Single responsibility: Process scheduling algorithms
 */
typedef struct {
    const char* algorithm_name;

    tbos_result_t (*schedule_next)(void* scheduler_data);
    tbos_result_t (*add_process)(void* scheduler_data, void* process);
    tbos_result_t (*remove_process)(void* scheduler_data, void* process);
    tbos_result_t (*update_priority)(void* scheduler_data, void* process, int new_priority);
    tbos_result_t (*handle_time_quantum_expired)(void* scheduler_data, void* process);
} tbos_scheduling_strategy_t;

/**
 * @brief CPU Context Interface (SRP)
 * Single responsibility: CPU register management
 */
typedef struct {
    tbos_result_t (*save_context)(void* self, void* registers);
    tbos_result_t (*restore_context)(void* self, void* registers);
    tbos_result_t (*initialize_context)(void* self, void* entry_point, void* stack);
    size_t (*get_context_size)(void* self);
} tbos_cpu_context_interface_t;

/**
 * @brief Memory Space Interface (SRP)
 * Single responsibility: Process memory management
 */
typedef struct {
    tbos_result_t (*allocate_virtual)(void* self, size_t size, uint32_t permissions);
    tbos_result_t (*deallocate_virtual)(void* self, void* address, size_t size);
    tbos_result_t (*map_physical)(void* self, void* virtual_addr, void* physical_addr, size_t size);
    tbos_result_t (*protect_memory)(void* self, void* address, size_t size, uint32_t permissions);
    tbos_result_t (*get_memory_stats)(void* self);
} tbos_memory_space_interface_t;

/**
 * @brief IPC Manager Interface (SRP)
 * Single responsibility: Inter-process communication
 */
typedef struct {
    tbos_result_t (*send_message)(void* self, pid_t from, pid_t to, const void* data, size_t size);
    tbos_result_t (*receive_message)(void* self, pid_t process, void* buffer, size_t buffer_size);
    tbos_result_t (*create_shared_memory)(void* self, size_t size, uint32_t permissions);
    tbos_result_t (*attach_shared_memory)(void* self, pid_t process, void* shared_memory);
    tbos_result_t (*detach_shared_memory)(void* self, pid_t process, void* shared_memory);
} tbos_ipc_interface_t;

/**
 * @brief Signal Manager Interface (SRP)
 * Single responsibility: Signal handling
 */
typedef struct {
    tbos_result_t (*send_signal)(void* self, pid_t from, pid_t to, int signal);
    tbos_result_t (*register_handler)(void* self, pid_t process, int signal, void (*handler)(int));
    tbos_result_t (*block_signals)(void* self, pid_t process, const sigset_t* signals);
    tbos_result_t (*unblock_signals)(void* self, pid_t process, const sigset_t* signals);
    tbos_result_t (*deliver_pending_signals)(void* self, pid_t process);
} tbos_signal_interface_t;

/**
 * @brief Thread Management Interface (SRP)
 * Single responsibility: Thread operations within a process
 */
typedef struct {
    tbos_result_t (*create_thread)(void* self, pid_t process, void (*entry)(void*), void* args);
    tbos_result_t (*terminate_thread)(void* self, pid_t process, uint32_t thread_id);
    tbos_result_t (*join_thread)(void* self, pid_t process, uint32_t thread_id);
    tbos_result_t (*yield_thread)(void* self, pid_t process, uint32_t thread_id);
    tbos_result_t (*get_thread_info)(void* self, pid_t process, uint32_t thread_id);
} tbos_thread_interface_t;

/**
 * @brief Process Control Block v2 (Clean, focused design)
 */
typedef struct tbos_process_v2 {
    tbos_base_t base;           // Inherits from base class

    // Core identification
    pid_t pid;
    pid_t parent_pid;
    char name[256];

    // State management (State Pattern)
    tbos_process_state_t* current_state;
    tbos_state_machine_t* state_machine;

    // Timing information
    uint64_t created_time;
    uint64_t start_time;
    uint64_t cpu_time_used;
    uint64_t last_scheduled;

    // Priority and scheduling
    int priority;
    int nice_value;
    uint32_t time_quantum;
    uint64_t quantum_remaining;

    // Dependencies (Dependency Injection)
    tbos_cpu_context_interface_t* cpu_context;
    tbos_memory_space_interface_t* memory_space;
    tbos_thread_interface_t* thread_manager;

    // Resource limits
    struct {
        size_t max_memory;
        uint32_t max_cpu_time;
        uint32_t max_files;
        uint32_t max_threads;
    } limits;

    // Security context
    struct {
        uint32_t uid;
        uint32_t gid;
        uint32_t effective_uid;
        uint32_t effective_gid;
        uint32_t capabilities;
    } security;

    // Parent/child relationships
    struct tbos_process_v2* parent;
    struct tbos_process_v2** children;
    size_t children_count;
    size_t children_capacity;

    // Exit information
    int exit_status;
    bool has_exited;

    // Private data for extensions
    void* private_data;

} tbos_process_t;

/**
 * @brief Process Manager Implementation (Facade Pattern)
 */
typedef struct tbos_process_manager_v2 {
    tbos_base_t base;

    // Dependencies
    tbos_scheduling_strategy_t* scheduler_strategy;
    tbos_ipc_interface_t* ipc_manager;
    tbos_signal_interface_t* signal_manager;
    tbos_memory_interface_t* memory_manager;

    // Process table (using hash table for efficiency)
    tbos_process_t** process_table;
    size_t process_table_size;
    size_t process_count;

    // Process pools (Object Pool Pattern)
    tbos_memory_pool_t* process_pool;
    tbos_memory_pool_t* thread_pool;

    // Global scheduling data
    struct {
        tbos_process_t* current_process;
        tbos_process_t** ready_queues[10]; // Multi-level priority queues
        tbos_process_t* blocked_queue;
        tbos_process_t* zombie_queue;

        uint64_t total_context_switches;
        uint64_t scheduler_ticks;
        uint32_t time_slice_ms;
        bool preemptive;
    } scheduler_data;

    // Statistics
    struct {
        uint32_t processes_created;
        uint32_t processes_terminated;
        uint64_t total_cpu_time;
        double average_cpu_utilization;
    } stats;

    // Configuration
    struct {
        uint32_t max_processes;
        uint32_t default_time_slice;
        bool enable_preemption;
        bool enable_priority_inheritance;
    } config;

} tbos_process_manager_t;

/**
 * @brief Process Command Interface (Command Pattern)
 * Encapsulates process operations for undo/redo, queuing, logging
 */
typedef struct tbos_process_command {
    const char* command_name;

    tbos_result_t (*execute)(struct tbos_process_command* self, void* params);
    tbos_result_t (*undo)(struct tbos_process_command* self);
    tbos_result_t (*redo)(struct tbos_process_command* self);
    bool (*can_undo)(struct tbos_process_command* self);

    void* command_data;
    uint64_t timestamp;
} tbos_process_command_t;

/**
 * @brief Process Event Types (Observer Pattern)
 */
typedef enum {
    TBOS_PROCESS_EVENT_CREATED,
    TBOS_PROCESS_EVENT_STARTED,
    TBOS_PROCESS_EVENT_TERMINATED,
    TBOS_PROCESS_EVENT_STATE_CHANGED,
    TBOS_PROCESS_EVENT_PRIORITY_CHANGED,
    TBOS_PROCESS_EVENT_MEMORY_ALLOCATED,
    TBOS_PROCESS_EVENT_CPU_QUOTA_EXCEEDED,
    TBOS_PROCESS_EVENT_ERROR
} tbos_process_event_type_t;

typedef struct {
    void (*on_process_event)(tbos_process_event_type_t event, pid_t pid, void* data);
} tbos_process_observer_t;

/**
 * @brief Process Factory Interface (Abstract Factory Pattern)
 */
typedef struct {
    tbos_process_t* (*create_process)(const char* name, void (*entry)(void*), void* args);
    tbos_cpu_context_interface_t* (*create_cpu_context)(const char* architecture);
    tbos_memory_space_interface_t* (*create_memory_space)(size_t initial_size);
    tbos_scheduling_strategy_t* (*create_scheduler)(const char* algorithm);
    tbos_thread_interface_t* (*create_thread_manager)(void);
} tbos_process_factory_t;

// Factory functions for different scheduling strategies
tbos_scheduling_strategy_t* tbos_create_round_robin_scheduler(uint32_t time_quantum);
tbos_scheduling_strategy_t* tbos_create_priority_scheduler(bool preemptive);
tbos_scheduling_strategy_t* tbos_create_cfs_scheduler(void); // Completely Fair Scheduler
tbos_scheduling_strategy_t* tbos_create_realtime_scheduler(void);

// Process manager creation (Factory)
tbos_process_manager_t* tbos_process_manager_create(const char* scheduler_type);
void tbos_process_manager_destroy(tbos_process_manager_t* manager);

// High-level process operations (Facade Pattern)
tbos_result_t tbos_process_create_v2(const char* name, void (*entry)(void*), void* args, int priority);
tbos_result_t tbos_process_terminate_v2(pid_t pid, int exit_status);
tbos_result_t tbos_process_wait_v2(pid_t pid, int* status, int options);
tbos_result_t tbos_process_kill_v2(pid_t pid, int signal);
tbos_result_t tbos_process_get_info_v2(pid_t pid);

// Process state queries
tbos_result_t tbos_process_is_running(pid_t pid);
tbos_result_t tbos_process_get_children(pid_t pid);
tbos_result_t tbos_process_get_parent(pid_t pid);

// Process resource management
tbos_result_t tbos_process_set_memory_limit(pid_t pid, size_t limit);
tbos_result_t tbos_process_set_cpu_limit(pid_t pid, uint32_t limit_ms);
tbos_result_t tbos_process_get_resource_usage(pid_t pid);

// Observer pattern for process events
tbos_result_t tbos_register_process_observer(tbos_process_observer_t* observer);
tbos_result_t tbos_unregister_process_observer(tbos_process_observer_t* observer);

// Global process manager access
extern tbos_process_manager_t* g_tbos_process_manager;

// Utility functions
const char* tbos_process_state_to_string(tbos_process_state_t* state);
tbos_result_t tbos_validate_process_params(const char* name, void (*entry)(void*));

#endif // TBOS_PROCESS_V2_H