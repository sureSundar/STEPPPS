/**
 * @file tbos_di_bootstrap.h
 * @brief TernaryBit OS Dependency Injection Bootstrap
 *
 * Demonstrates comprehensive IoC container usage with all TBOS components.
 * Shows how to properly structure an OS using Dependency Injection principles:
 *
 * - Loose Coupling: Components depend on interfaces, not implementations
 * - Testability: Easy to inject mock dependencies for unit testing
 * - Configuration: External configuration of component relationships
 * - Lifecycle Management: Automatic handling of component lifecycles
 * - Aspect-Oriented Programming: Cross-cutting concerns (logging, security)
 *
 * This is how a real enterprise-grade OS should be architected!
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef TBOS_DI_BOOTSTRAP_H
#define TBOS_DI_BOOTSTRAP_H

#include "tbos_ioc_container.h"
#include "tbos_interfaces.h"
#include "tbos_base.h"
#include "tbos_filesystem.h"
#include "tbos_process.h"

/**
 * @brief TBOS Service Names (centralized configuration)
 * Following DRY principle - service names defined once
 */
#define TBOS_SERVICE_MEMORY_MANAGER     "memory_manager"
#define TBOS_SERVICE_FILESYSTEM         "filesystem"
#define TBOS_SERVICE_PROCESS_MANAGER    "process_manager"
#define TBOS_SERVICE_SCHEDULER          "scheduler"
#define TBOS_SERVICE_HARDWARE_MANAGER   "hardware_manager"
#define TBOS_SERVICE_NETWORK_STACK      "network_stack"
#define TBOS_SERVICE_SECURITY_MANAGER   "security_manager"
#define TBOS_SERVICE_LOGGER            "logger"
#define TBOS_SERVICE_CONFIG_MANAGER    "config_manager"
#define TBOS_SERVICE_EVENT_BUS         "event_bus"

/**
 * @brief TBOS Interface Names
 */
#define TBOS_INTERFACE_MEMORY          "IMemoryManager"
#define TBOS_INTERFACE_FILESYSTEM      "IFilesystem"
#define TBOS_INTERFACE_PROCESS         "IProcessManager"
#define TBOS_INTERFACE_SCHEDULER       "IScheduler"
#define TBOS_INTERFACE_HARDWARE        "IHardwareManager"
#define TBOS_INTERFACE_NETWORK         "INetworkStack"
#define TBOS_INTERFACE_SECURITY        "ISecurityManager"
#define TBOS_INTERFACE_LOGGER          "ILogger"
#define TBOS_INTERFACE_CONFIG          "IConfigManager"
#define TBOS_INTERFACE_EVENT_BUS       "IEventBus"

/**
 * @brief Memory Manager Implementation with DI
 */
typedef struct tbos_memory_manager_di {
    tbos_memory_interface_t interface;  // Implements the interface
    tbos_base_t base;                   // Inherits base functionality

    // Dependencies (injected)
    tbos_logger_t* logger;              // @Inject(TBOS_SERVICE_LOGGER)
    tbos_config_t* config;              // @Inject(TBOS_SERVICE_CONFIG_MANAGER)
    tbos_observer_interface_t* event_bus; // @Inject(TBOS_SERVICE_EVENT_BUS)

    // Implementation-specific data
    tbos_memory_pool_t* heap_pool;
    tbos_memory_pool_t* stack_pool;
    tbos_cache_t* allocation_cache;

    struct {
        size_t total_allocated;
        size_t peak_usage;
        uint64_t allocation_count;
        uint64_t deallocation_count;
    } stats;

} tbos_memory_manager_di_t;

/**
 * @brief Filesystem Implementation with DI
 */
typedef struct tbos_filesystem_di {
    tbos_filesystem_interface_t interface;

    // Dependencies
    tbos_memory_interface_t* memory_manager;    // @Inject(TBOS_INTERFACE_MEMORY)
    tbos_logger_t* logger;                      // @Inject(TBOS_SERVICE_LOGGER)
    tbos_observer_interface_t* event_bus;       // @Inject(TBOS_SERVICE_EVENT_BUS)

    // Storage layer dependencies
    tbos_block_device_interface_t* block_device;
    tbos_inode_manager_interface_t* inode_manager;
    tbos_block_allocator_interface_t* block_allocator;

    // Implementation data
    tbos_superblock_t* superblock;
    tbos_file_cache_interface_t* cache;
    bool mounted;

} tbos_filesystem_di_t;

/**
 * @brief Process Manager Implementation with DI
 */
typedef struct tbos_process_manager_di {
    tbos_process_interface_t interface;

    // Dependencies
    tbos_memory_interface_t* memory_manager;    // @Inject(TBOS_INTERFACE_MEMORY)
    tbos_scheduler_interface_t* scheduler;      // @Inject(TBOS_INTERFACE_SCHEDULER)
    tbos_logger_t* logger;                      // @Inject(TBOS_SERVICE_LOGGER)
    tbos_observer_interface_t* event_bus;       // @Inject(TBOS_SERVICE_EVENT_BUS)

    // Process management data
    tbos_process_t** process_table;
    size_t process_count;
    pid_t next_pid;

} tbos_process_manager_di_t;

/**
 * @brief Scheduler Implementation with DI
 */
typedef struct tbos_scheduler_di {
    tbos_scheduler_interface_t interface;

    // Dependencies
    tbos_process_interface_t* process_manager;  // @Inject(TBOS_INTERFACE_PROCESS)
    tbos_logger_t* logger;                      // @Inject(TBOS_SERVICE_LOGGER)
    tbos_config_t* config;                      // @Inject(TBOS_SERVICE_CONFIG_MANAGER)

    // Scheduling strategy (Strategy Pattern + DI)
    tbos_scheduling_strategy_t* strategy;       // @Inject(configured strategy)

    // Scheduling data
    tbos_process_t* current_process;
    tbos_process_t** ready_queues[10];

} tbos_scheduler_di_t;

/**
 * @brief Network Stack Implementation with DI
 */
typedef struct tbos_network_stack_di {
    tbos_network_interface_t interface;

    // Dependencies
    tbos_memory_interface_t* memory_manager;    // @Inject(TBOS_INTERFACE_MEMORY)
    tbos_logger_t* logger;                      // @Inject(TBOS_SERVICE_LOGGER)
    tbos_hardware_interface_t* hardware;        // @Inject(TBOS_INTERFACE_HARDWARE)

    // Network implementation data
    struct {
        bool initialized;
        char interface_name[64];
        uint8_t mac_address[6];
        uint32_t ip_address;
    } network_state;

} tbos_network_stack_di_t;

/**
 * @brief Security Manager Implementation with DI
 */
typedef struct tbos_security_manager_di {
    // Security interface (to be defined)
    void* security_interface;

    // Dependencies
    tbos_process_interface_t* process_manager;  // @Inject(TBOS_INTERFACE_PROCESS)
    tbos_filesystem_interface_t* filesystem;    // @Inject(TBOS_INTERFACE_FILESYSTEM)
    tbos_logger_t* logger;                      // @Inject(TBOS_SERVICE_LOGGER)

    // Security policies
    struct {
        bool enforce_access_control;
        bool audit_file_access;
        bool enforce_memory_protection;
    } policies;

} tbos_security_manager_di_t;

/**
 * @brief Event Bus Implementation (Observer Pattern + DI)
 */
typedef struct tbos_event_bus_di {
    tbos_observer_interface_t interface;

    // Dependencies
    tbos_logger_t* logger;                      // @Inject(TBOS_SERVICE_LOGGER)

    // Event management
    tbos_observer_interface_t** observers;
    size_t observer_count;
    pthread_mutex_t observers_mutex;

    // Event queuing
    struct {
        void** event_queue;
        size_t queue_size;
        size_t queue_capacity;
        pthread_mutex_t queue_mutex;
    } event_data;

} tbos_event_bus_di_t;

/**
 * @brief Factory Functions for Each Service
 * These are the actual factory methods that create instances
 */

// Memory Manager Factory
void* tbos_create_memory_manager_di(tbos_ioc_container_t* container);

// Filesystem Factory
void* tbos_create_filesystem_di(tbos_ioc_container_t* container);

// Process Manager Factory
void* tbos_create_process_manager_di(tbos_ioc_container_t* container);

// Scheduler Factory
void* tbos_create_scheduler_di(tbos_ioc_container_t* container);

// Network Stack Factory
void* tbos_create_network_stack_di(tbos_ioc_container_t* container);

// Security Manager Factory
void* tbos_create_security_manager_di(tbos_ioc_container_t* container);

// Event Bus Factory
void* tbos_create_event_bus_di(tbos_ioc_container_t* container);

/**
 * @brief Service Registration Functions
 * These functions register services with proper dependencies
 */
tbos_result_t tbos_register_core_services(tbos_ioc_container_t* container);
tbos_result_t tbos_register_memory_services(tbos_ioc_container_t* container);
tbos_result_t tbos_register_filesystem_services(tbos_ioc_container_t* container);
tbos_result_t tbos_register_process_services(tbos_ioc_container_t* container);
tbos_result_t tbos_register_network_services(tbos_ioc_container_t* container);
tbos_result_t tbos_register_security_services(tbos_ioc_container_t* container);

/**
 * @brief TBOS Bootstrap Functions
 * Main entry points for initializing TBOS with DI
 */
tbos_result_t tbos_bootstrap_with_di(void);
tbos_result_t tbos_bootstrap_from_config(const char* config_file);
tbos_result_t tbos_bootstrap_minimal_di(void);  // For embedded systems
tbos_result_t tbos_bootstrap_testing_di(void);  // For unit testing

/**
 * @brief Main TBOS Application Context (using DI)
 */
typedef struct tbos_application_context {
    tbos_ioc_container_t* container;

    // Core services (resolved from container)
    tbos_memory_interface_t* memory_manager;
    tbos_filesystem_interface_t* filesystem;
    tbos_process_interface_t* process_manager;
    tbos_scheduler_interface_t* scheduler;
    tbos_network_interface_t* network_stack;

    // Application state
    bool is_running;
    uint64_t start_time;
    char boot_parameters[1024];

} tbos_application_context_t;

// Application context management
tbos_application_context_t* tbos_create_application_context(void);
tbos_result_t tbos_start_application(tbos_application_context_t* app_context);
tbos_result_t tbos_stop_application(tbos_application_context_t* app_context);
void tbos_destroy_application_context(tbos_application_context_t* app_context);

/**
 * @brief Configuration-driven Service Registration
 * Allows external configuration of component relationships
 */
typedef struct tbos_service_configuration {
    char services[32][128];     // Service implementations
    char interfaces[32][128];   // Interface mappings
    char lifetimes[32][32];     // Lifecycle management
    char dependencies[32][512]; // Dependency declarations
    size_t service_count;
} tbos_service_configuration_t;

tbos_result_t tbos_load_service_configuration(const char* config_file,
                                              tbos_service_configuration_t* config);
tbos_result_t tbos_register_services_from_config(tbos_ioc_container_t* container,
                                                  tbos_service_configuration_t* config);

/**
 * @brief Aspect Registration (Cross-cutting concerns)
 */
tbos_result_t tbos_register_logging_aspect(tbos_ioc_container_t* container);
tbos_result_t tbos_register_security_aspect(tbos_ioc_container_t* container);
tbos_result_t tbos_register_performance_aspect(tbos_ioc_container_t* container);
tbos_result_t tbos_register_caching_aspect(tbos_ioc_container_t* container);

/**
 * @brief Health Check Registration
 */
tbos_result_t tbos_register_all_health_checks(tbos_ioc_container_t* container);

/**
 * @brief Service Validation and Testing
 */
tbos_result_t tbos_validate_all_services(tbos_ioc_container_t* container);
tbos_result_t tbos_test_dependency_injection(tbos_ioc_container_t* container);
tbos_result_t tbos_benchmark_service_resolution(tbos_ioc_container_t* container);

/**
 * @brief Convenience Macros for Service Resolution
 */
#define TBOS_GET_MEMORY_MANAGER() \
    TBOS_RESOLVE_INTERFACE(tbos_get_container(), tbos_memory_interface_t, TBOS_INTERFACE_MEMORY)

#define TBOS_GET_FILESYSTEM() \
    TBOS_RESOLVE_INTERFACE(tbos_get_container(), tbos_filesystem_interface_t, TBOS_INTERFACE_FILESYSTEM)

#define TBOS_GET_PROCESS_MANAGER() \
    TBOS_RESOLVE_INTERFACE(tbos_get_container(), tbos_process_interface_t, TBOS_INTERFACE_PROCESS)

#define TBOS_GET_SCHEDULER() \
    TBOS_RESOLVE_INTERFACE(tbos_get_container(), tbos_scheduler_interface_t, TBOS_INTERFACE_SCHEDULER)

#define TBOS_GET_NETWORK_STACK() \
    TBOS_RESOLVE_INTERFACE(tbos_get_container(), tbos_network_interface_t, TBOS_INTERFACE_NETWORK)

/**
 * @brief Global Application Context Access
 */
extern tbos_application_context_t* g_tbos_app_context;
tbos_application_context_t* tbos_get_app_context(void);

#endif // TBOS_DI_BOOTSTRAP_H