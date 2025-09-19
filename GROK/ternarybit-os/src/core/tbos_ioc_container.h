/**
 * @file tbos_ioc_container.h
 * @brief TernaryBit OS IoC Container and Dependency Injection Framework
 *
 * Implements comprehensive Inversion of Control (IoC) and Dependency Injection (DI):
 * - Constructor Injection: Dependencies provided during object creation
 * - Setter Injection: Dependencies set after object creation
 * - Interface Injection: Dependencies injected through interfaces
 * - Service Locator: Central registry for service discovery
 * - Dependency Resolution: Automatic resolution of dependency graphs
 * - Lifecycle Management: Singleton, Transient, Scoped lifetimes
 * - Circular Dependency Detection: Prevents dependency cycles
 * - Configuration-based DI: XML/JSON configuration support
 *
 * Benefits:
 * - Loose Coupling: Components depend on abstractions, not implementations
 * - Testability: Easy to inject mock dependencies for testing
 * - Maintainability: Changes to implementations don't affect dependents
 * - Extensibility: New implementations can be plugged in easily
 * - Single Responsibility: Each component focuses on its core logic
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef TBOS_IOC_CONTAINER_H
#define TBOS_IOC_CONTAINER_H

#include "tbos_interfaces.h"
#include "tbos_base.h"
#include <stdbool.h>
#include <stddef.h>
#include <pthread.h>

/**
 * @brief Service Lifetime Management
 */
typedef enum {
    TBOS_LIFETIME_SINGLETON,    // One instance for the entire application
    TBOS_LIFETIME_TRANSIENT,    // New instance every time
    TBOS_LIFETIME_SCOPED,       // One instance per scope (e.g., per request)
    TBOS_LIFETIME_THREAD        // One instance per thread
} tbos_service_lifetime_t;

/**
 * @brief Dependency Injection Types
 */
typedef enum {
    TBOS_DI_CONSTRUCTOR,        // Inject through constructor
    TBOS_DI_SETTER,             // Inject through setter methods
    TBOS_DI_INTERFACE,          // Inject through interface methods
    TBOS_DI_PROPERTY            // Direct property injection
} tbos_injection_type_t;

/**
 * @brief Service Descriptor
 * Describes how to create and manage a service
 */
typedef struct tbos_service_descriptor {
    const char* service_name;           // Unique service identifier
    const char* interface_name;         // Interface this service implements
    size_t service_size;                // Size of service structure
    tbos_service_lifetime_t lifetime;   // Service lifetime

    // Factory methods
    void* (*factory_method)(struct tbos_ioc_container* container);
    void (*destructor_method)(void* instance);
    tbos_result_t (*initializer_method)(void* instance, struct tbos_ioc_container* container);

    // Dependency information
    const char** dependencies;          // Array of dependency names
    size_t dependency_count;
    tbos_injection_type_t injection_type;

    // Configuration
    void* configuration_data;
    size_t configuration_size;

    // Metadata
    bool is_abstract;                   // True for interfaces/abstract classes
    uint32_t version;
    const char* author;
    const char* description;

} tbos_service_descriptor_t;

/**
 * @brief Service Instance
 * Represents a created service instance
 */
typedef struct tbos_service_instance {
    void* instance;                     // The actual service object
    tbos_service_descriptor_t* descriptor; // Service descriptor
    bool is_initialized;                // Initialization status
    uint32_t reference_count;           // For lifetime management
    uint64_t creation_time;             // When instance was created
    pthread_mutex_t instance_mutex;     // Thread safety

    // For scoped/thread lifetime management
    pthread_t owner_thread;
    void* scope_context;

} tbos_service_instance_t;

/**
 * @brief Dependency Graph Node
 * Used for dependency resolution and circular dependency detection
 */
typedef struct tbos_dependency_node {
    const char* service_name;
    struct tbos_dependency_node** dependencies;
    size_t dependency_count;
    bool visited;                       // For cycle detection
    bool in_resolution;                 // Currently being resolved
} tbos_dependency_node_t;

/**
 * @brief IoC Container
 * Central container for managing services and dependencies
 */
typedef struct tbos_ioc_container {
    tbos_base_t base;

    // Service registry
    tbos_service_descriptor_t** descriptors;
    size_t descriptor_count;
    size_t descriptor_capacity;

    // Instance cache
    tbos_service_instance_t** instances;
    size_t instance_count;
    size_t instance_capacity;

    // Dependency resolution
    tbos_dependency_node_t** dependency_graph;
    size_t graph_node_count;

    // Scoping
    void** scope_stack;
    size_t scope_depth;
    size_t max_scope_depth;

    // Configuration
    struct {
        bool auto_resolve_dependencies;
        bool allow_circular_dependencies;
        bool lazy_initialization;
        uint32_t max_resolution_depth;
        bool enable_debugging;
    } config;

    // Statistics
    struct {
        uint32_t services_registered;
        uint32_t instances_created;
        uint32_t dependency_resolutions;
        uint32_t circular_dependencies_detected;
    } stats;

    // Thread safety
    pthread_rwlock_t container_lock;

} tbos_ioc_container_t;

/**
 * @brief Dependency Injection Annotations (using macros for C)
 */
#define TBOS_INJECT_CONSTRUCTOR(...) \
    static const char* _tbos_constructor_dependencies[] = {__VA_ARGS__, NULL}

#define TBOS_INJECT_SERVICE(name) \
    static const char* _tbos_service_name = name

#define TBOS_INJECT_DEPENDENCY(dep_name) \
    const char* _tbos_dependency_##dep_name = #dep_name

/**
 * @brief Service Registration Builder Pattern
 */
typedef struct tbos_service_builder {
    tbos_service_descriptor_t* descriptor;
    tbos_ioc_container_t* container;

    // Builder methods (method chaining)
    struct tbos_service_builder* (*as_singleton)(struct tbos_service_builder* self);
    struct tbos_service_builder* (*as_transient)(struct tbos_service_builder* self);
    struct tbos_service_builder* (*as_scoped)(struct tbos_service_builder* self);
    struct tbos_service_builder* (*with_factory)(struct tbos_service_builder* self,
                                                  void* (*factory)(tbos_ioc_container_t*));
    struct tbos_service_builder* (*with_dependencies)(struct tbos_service_builder* self,
                                                       const char** deps, size_t count);
    struct tbos_service_builder* (*with_configuration)(struct tbos_service_builder* self,
                                                        void* config, size_t config_size);
    tbos_result_t (*build)(struct tbos_service_builder* self);

} tbos_service_builder_t;

/**
 * @brief Core IoC Container Functions
 */

// Container lifecycle
tbos_ioc_container_t* tbos_ioc_container_create(void);
void tbos_ioc_container_destroy(tbos_ioc_container_t* container);
tbos_result_t tbos_ioc_container_configure(tbos_ioc_container_t* container, const char* config_file);

// Service registration
tbos_service_builder_t* tbos_ioc_register_service(tbos_ioc_container_t* container,
                                                   const char* service_name,
                                                   const char* interface_name,
                                                   size_t service_size);

tbos_result_t tbos_ioc_register_instance(tbos_ioc_container_t* container,
                                          const char* service_name,
                                          void* instance);

tbos_result_t tbos_ioc_register_factory(tbos_ioc_container_t* container,
                                         const char* service_name,
                                         void* (*factory)(tbos_ioc_container_t*),
                                         tbos_service_lifetime_t lifetime);

// Service resolution
tbos_result_t tbos_ioc_resolve(tbos_ioc_container_t* container, const char* service_name);
tbos_result_t tbos_ioc_resolve_interface(tbos_ioc_container_t* container, const char* interface_name);
tbos_result_t tbos_ioc_try_resolve(tbos_ioc_container_t* container, const char* service_name);

// Typed resolution (using macros for type safety)
#define TBOS_RESOLVE(container, type, name) \
    ((type*)tbos_ioc_resolve(container, name).data)

#define TBOS_RESOLVE_INTERFACE(container, type, interface) \
    ((type*)tbos_ioc_resolve_interface(container, interface).data)

// Dependency injection
tbos_result_t tbos_ioc_inject_dependencies(tbos_ioc_container_t* container, void* instance);
tbos_result_t tbos_ioc_inject_constructor_dependencies(tbos_ioc_container_t* container,
                                                        void* instance,
                                                        const char** dependencies);

// Scoping
tbos_result_t tbos_ioc_begin_scope(tbos_ioc_container_t* container, const char* scope_name);
tbos_result_t tbos_ioc_end_scope(tbos_ioc_container_t* container);
void* tbos_ioc_get_current_scope(tbos_ioc_container_t* container);

// Service discovery
tbos_result_t tbos_ioc_get_registered_services(tbos_ioc_container_t* container);
tbos_result_t tbos_ioc_get_service_info(tbos_ioc_container_t* container, const char* service_name);
bool tbos_ioc_is_registered(tbos_ioc_container_t* container, const char* service_name);

// Lifecycle management
tbos_result_t tbos_ioc_initialize_service(tbos_ioc_container_t* container, const char* service_name);
tbos_result_t tbos_ioc_shutdown_service(tbos_ioc_container_t* container, const char* service_name);
tbos_result_t tbos_ioc_restart_service(tbos_ioc_container_t* container, const char* service_name);

// Debugging and diagnostics
tbos_result_t tbos_ioc_validate_dependencies(tbos_ioc_container_t* container);
tbos_result_t tbos_ioc_detect_circular_dependencies(tbos_ioc_container_t* container);
tbos_result_t tbos_ioc_get_dependency_graph(tbos_ioc_container_t* container);
void tbos_ioc_dump_container_state(tbos_ioc_container_t* container);

/**
 * @brief Attribute-based Dependency Injection (Decorator Pattern)
 */
typedef struct tbos_injectable {
    const char* service_name;
    bool required;
    const char* qualifier;  // For multiple implementations
} tbos_injectable_t;

// Injection attributes (using GCC attributes)
#define TBOS_INJECTABLE(name, required) \
    __attribute__((annotate("tbos_injectable:" name ":" #required)))

#define TBOS_QUALIFIER(qualifier) \
    __attribute__((annotate("tbos_qualifier:" qualifier)))

/**
 * @brief Configuration-based Registration
 */
typedef struct tbos_service_config {
    char service_name[128];
    char interface_name[128];
    char implementation[256];
    char lifetime[32];
    char dependencies[512];  // Comma-separated
    char configuration[1024]; // JSON configuration
} tbos_service_config_t;

tbos_result_t tbos_ioc_load_configuration(tbos_ioc_container_t* container, const char* config_file);
tbos_result_t tbos_ioc_register_from_config(tbos_ioc_container_t* container,
                                             tbos_service_config_t* config);

/**
 * @brief Aspect-Oriented Programming Support
 * Allows cross-cutting concerns (logging, security, caching) to be applied to services
 */
typedef struct tbos_aspect {
    const char* aspect_name;
    tbos_result_t (*before)(void* instance, const char* method_name, void** args);
    tbos_result_t (*after)(void* instance, const char* method_name, tbos_result_t result);
    tbos_result_t (*around)(void* instance, const char* method_name, void** args,
                           tbos_result_t (*proceed)(void*, const char*, void**));
    tbos_result_t (*on_exception)(void* instance, const char* method_name, int error_code);
} tbos_aspect_t;

tbos_result_t tbos_ioc_register_aspect(tbos_ioc_container_t* container, tbos_aspect_t* aspect);
tbos_result_t tbos_ioc_apply_aspect(tbos_ioc_container_t* container,
                                    const char* service_name,
                                    const char* aspect_name);

/**
 * @brief Global IoC Container (Application-level Singleton)
 */
extern tbos_ioc_container_t* g_tbos_ioc_container;

// Global convenience functions
tbos_ioc_container_t* tbos_get_container(void);
tbos_result_t tbos_initialize_global_container(void);
void tbos_cleanup_global_container(void);

// Service registration macros for convenience
#define TBOS_REGISTER_SINGLETON(interface, implementation) \
    tbos_ioc_register_service(tbos_get_container(), #implementation, #interface, sizeof(implementation)) \
        ->as_singleton() \
        ->build()

#define TBOS_REGISTER_TRANSIENT(interface, implementation) \
    tbos_ioc_register_service(tbos_get_container(), #implementation, #interface, sizeof(implementation)) \
        ->as_transient() \
        ->build()

#define TBOS_REGISTER_SCOPED(interface, implementation) \
    tbos_ioc_register_service(tbos_get_container(), #implementation, #interface, sizeof(implementation)) \
        ->as_scoped() \
        ->build()

/**
 * @brief Service Health Monitoring
 */
typedef enum {
    TBOS_SERVICE_HEALTH_HEALTHY,
    TBOS_SERVICE_HEALTH_DEGRADED,
    TBOS_SERVICE_HEALTH_UNHEALTHY,
    TBOS_SERVICE_HEALTH_UNKNOWN
} tbos_service_health_t;

typedef struct {
    tbos_service_health_t (*check_health)(void* service_instance);
    tbos_result_t (*get_health_details)(void* service_instance);
} tbos_health_check_t;

tbos_result_t tbos_ioc_register_health_check(tbos_ioc_container_t* container,
                                             const char* service_name,
                                             tbos_health_check_t* health_check);

tbos_result_t tbos_ioc_check_service_health(tbos_ioc_container_t* container, const char* service_name);
tbos_result_t tbos_ioc_check_all_services_health(tbos_ioc_container_t* container);

#endif // TBOS_IOC_CONTAINER_H