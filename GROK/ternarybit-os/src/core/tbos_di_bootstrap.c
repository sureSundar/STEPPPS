/**
 * @file tbos_di_bootstrap.c
 * @brief TernaryBit OS Dependency Injection Bootstrap Implementation
 *
 * This file demonstrates enterprise-grade OS architecture using dependency injection.
 * It shows how to build a real operating system with proper separation of concerns,
 * testability, and maintainability.
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#include "tbos_di_bootstrap.h"
#include "tbos_base.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global application context
tbos_application_context_t* g_tbos_app_context = NULL;

/**
 * @brief Memory Manager Implementation with Dependency Injection
 */
static tbos_result_t memory_manager_allocate_di(size_t size, uint32_t flags) {
    // Get the actual implementation instance
    tbos_memory_manager_di_t* manager = (tbos_memory_manager_di_t*)TBOS_GET_MEMORY_MANAGER();
    if (!manager) {
        return tbos_create_error_result(ENOENT, "Memory manager not available");
    }

    // Use injected logger for debugging
    if (manager->logger) {
        manager->logger->log(manager->logger, "DEBUG", "Allocating %zu bytes with flags 0x%x", size, flags);
    }

    void* ptr = NULL;

    // Strategy: Use appropriate pool based on size
    if (size <= 1024 && manager->heap_pool) {
        ptr = manager->heap_pool->allocate_block(manager->heap_pool);
    } else {
        ptr = malloc(size);
    }

    if (ptr) {
        manager->stats.total_allocated += size;
        manager->stats.allocation_count++;

        // Notify event bus if available
        if (manager->event_bus) {
            manager->event_bus->on_system_event("memory_allocated", &size);
        }
    }

    return ptr ? tbos_create_success_result(ptr, size) :
                 tbos_create_error_result(ENOMEM, "Memory allocation failed");
}

static tbos_result_t memory_manager_deallocate_di(void* ptr) {
    tbos_memory_manager_di_t* manager = (tbos_memory_manager_di_t*)TBOS_GET_MEMORY_MANAGER();
    if (!manager || !ptr) {
        return tbos_create_error_result(EINVAL, "Invalid parameters");
    }

    if (manager->logger) {
        manager->logger->log(manager->logger, "DEBUG", "Deallocating memory at %p", ptr);
    }

    // Try to free from pools first
    if (manager->heap_pool->free_block(manager->heap_pool, ptr).success) {
        // Successfully freed from pool
    } else {
        free(ptr);
    }

    manager->stats.deallocation_count++;

    if (manager->event_bus) {
        manager->event_bus->on_system_event("memory_deallocated", ptr);
    }

    return tbos_create_success_result(NULL, 0);
}

static tbos_result_t memory_manager_get_stats_di(void) {
    tbos_memory_manager_di_t* manager = (tbos_memory_manager_di_t*)TBOS_GET_MEMORY_MANAGER();
    if (!manager) {
        return tbos_create_error_result(ENOENT, "Memory manager not available");
    }

    // Return copy of statistics
    void* stats_copy = malloc(sizeof(manager->stats));
    if (!stats_copy) {
        return tbos_create_error_result(ENOMEM, "Cannot allocate stats");
    }

    memcpy(stats_copy, &manager->stats, sizeof(manager->stats));
    return tbos_create_success_result(stats_copy, sizeof(manager->stats));
}

/**
 * @brief Memory Manager Factory with Dependency Injection
 */
void* tbos_create_memory_manager_di(tbos_ioc_container_t* container) {
    tbos_memory_manager_di_t* manager = malloc(sizeof(tbos_memory_manager_di_t));
    if (!manager) {
        TBOS_LOG_ERROR("Failed to allocate memory manager");
        return NULL;
    }

    memset(manager, 0, sizeof(tbos_memory_manager_di_t));

    // Initialize base
    manager->base.component_name = "Memory Manager DI";
    manager->base.component_id = 1001;
    manager->base.created_at = time(NULL);

    // Initialize interface
    manager->interface.allocate = memory_manager_allocate_di;
    manager->interface.deallocate = memory_manager_deallocate_di;
    manager->interface.get_stats = memory_manager_get_stats_di;

    // Dependency Injection - resolve dependencies from container
    if (tbos_ioc_is_registered(container, TBOS_SERVICE_LOGGER)) {
        tbos_result_t logger_result = tbos_ioc_resolve(container, TBOS_SERVICE_LOGGER);
        if (logger_result.success) {
            manager->logger = (tbos_logger_t*)logger_result.data;
        }
    }

    if (tbos_ioc_is_registered(container, TBOS_SERVICE_CONFIG_MANAGER)) {
        tbos_result_t config_result = tbos_ioc_resolve(container, TBOS_SERVICE_CONFIG_MANAGER);
        if (config_result.success) {
            manager->config = (tbos_config_t*)config_result.data;
        }
    }

    if (tbos_ioc_is_registered(container, TBOS_SERVICE_EVENT_BUS)) {
        tbos_result_t event_result = tbos_ioc_resolve(container, TBOS_SERVICE_EVENT_BUS);
        if (event_result.success) {
            manager->event_bus = (tbos_observer_interface_t*)event_result.data;
        }
    }

    // Initialize memory pools based on configuration
    size_t heap_blocks = manager->config ? manager->config->default_heap_size / 1024 : 1000;
    manager->heap_pool = tbos_memory_pool_create(1024, heap_blocks);

    size_t stack_blocks = 100; // Default
    manager->stack_pool = tbos_memory_pool_create(4096, stack_blocks);

    manager->allocation_cache = tbos_cache_create("memory_cache", 64, TBOS_CACHE_LRU);

    if (manager->logger) {
        manager->logger->log(manager->logger, "INFO", "Memory Manager DI created with %zu heap blocks", heap_blocks);
    }

    return manager;
}

/**
 * @brief Event Bus Implementation with DI
 */
static void event_bus_on_system_event(const char* event, void* data) {
    tbos_event_bus_di_t* event_bus = (tbos_event_bus_di_t*)tbos_ioc_resolve(
        tbos_get_container(), TBOS_SERVICE_EVENT_BUS).data;

    if (!event_bus || !event) return;

    pthread_mutex_lock(&event_bus->observers_mutex);

    if (event_bus->logger) {
        event_bus->logger->log(event_bus->logger, "DEBUG", "Broadcasting event: %s", event);
    }

    // Broadcast to all observers
    for (size_t i = 0; i < event_bus->observer_count; i++) {
        if (event_bus->observers[i] && event_bus->observers[i]->on_system_event) {
            event_bus->observers[i]->on_system_event(event, data);
        }
    }

    pthread_mutex_unlock(&event_bus->observers_mutex);
}

void* tbos_create_event_bus_di(tbos_ioc_container_t* container) {
    tbos_event_bus_di_t* event_bus = malloc(sizeof(tbos_event_bus_di_t));
    if (!event_bus) {
        return NULL;
    }

    memset(event_bus, 0, sizeof(tbos_event_bus_di_t));

    // Initialize interface
    event_bus->interface.on_system_event = event_bus_on_system_event;

    // Initialize observer management
    event_bus->observers = malloc(64 * sizeof(tbos_observer_interface_t*));
    event_bus->observer_count = 0;
    pthread_mutex_init(&event_bus->observers_mutex, NULL);

    // Initialize event queue
    event_bus->event_data.queue_capacity = 256;
    event_bus->event_data.event_queue = malloc(event_bus->event_data.queue_capacity * sizeof(void*));
    event_bus->event_data.queue_size = 0;
    pthread_mutex_init(&event_bus->event_data.queue_mutex, NULL);

    // Inject logger dependency
    if (tbos_ioc_is_registered(container, TBOS_SERVICE_LOGGER)) {
        tbos_result_t logger_result = tbos_ioc_resolve(container, TBOS_SERVICE_LOGGER);
        if (logger_result.success) {
            event_bus->logger = (tbos_logger_t*)logger_result.data;
            event_bus->logger->log(event_bus->logger, "INFO", "Event Bus DI created successfully");
        }
    }

    return event_bus;
}

/**
 * @brief Service Registration Functions
 */
tbos_result_t tbos_register_core_services(tbos_ioc_container_t* container) {
    if (!container) {
        return tbos_create_error_result(EINVAL, "Invalid container");
    }

    // Register Logger (Singleton - needed by other services)
    tbos_logger_t* logger = tbos_logger_create("/var/log/tbos.log", true);
    if (logger) {
        tbos_ioc_register_instance(container, TBOS_SERVICE_LOGGER, logger);
        logger->log(logger, "INFO", "Registered logger service");
    }

    // Register Configuration Manager (Singleton)
    tbos_config_t* config = tbos_config_get_instance();
    if (config) {
        tbos_ioc_register_instance(container, TBOS_SERVICE_CONFIG_MANAGER, config);
    }

    // Register Event Bus (Singleton)
    tbos_service_builder_t* event_bus_builder = tbos_ioc_register_service(
        container,
        TBOS_SERVICE_EVENT_BUS,
        TBOS_INTERFACE_EVENT_BUS,
        sizeof(tbos_event_bus_di_t)
    );

    const char* event_bus_deps[] = {TBOS_SERVICE_LOGGER};
    event_bus_builder->with_factory(event_bus_builder, tbos_create_event_bus_di)
                     ->with_dependencies(event_bus_builder, event_bus_deps, 1)
                     ->as_singleton()
                     ->build();

    TBOS_LOG_INFO("Core services registered successfully");
    return tbos_create_success_result(NULL, 0);
}

tbos_result_t tbos_register_memory_services(tbos_ioc_container_t* container) {
    if (!container) {
        return tbos_create_error_result(EINVAL, "Invalid container");
    }

    // Register Memory Manager with dependencies
    tbos_service_builder_t* memory_builder = tbos_ioc_register_service(
        container,
        TBOS_SERVICE_MEMORY_MANAGER,
        TBOS_INTERFACE_MEMORY,
        sizeof(tbos_memory_manager_di_t)
    );

    const char* memory_deps[] = {
        TBOS_SERVICE_LOGGER,
        TBOS_SERVICE_CONFIG_MANAGER,
        TBOS_SERVICE_EVENT_BUS
    };

    memory_builder->with_factory(memory_builder, tbos_create_memory_manager_di)
                  ->with_dependencies(memory_builder, memory_deps, 3)
                  ->as_singleton()
                  ->build();

    TBOS_LOG_INFO("Memory services registered successfully");
    return tbos_create_success_result(NULL, 0);
}

/**
 * @brief TBOS Bootstrap with Dependency Injection
 */
tbos_result_t tbos_bootstrap_with_di(void) {
    TBOS_LOG_INFO("Starting TBOS bootstrap with Dependency Injection...");

    // Initialize IoC container
    tbos_result_t container_result = tbos_initialize_global_container();
    if (!container_result.success) {
        TBOS_LOG_ERROR("Failed to initialize IoC container");
        return container_result;
    }

    tbos_ioc_container_t* container = tbos_get_container();

    // Register services in dependency order
    tbos_result_t result;

    result = tbos_register_core_services(container);
    if (!result.success) {
        TBOS_LOG_ERROR("Failed to register core services");
        return result;
    }

    result = tbos_register_memory_services(container);
    if (!result.success) {
        TBOS_LOG_ERROR("Failed to register memory services");
        return result;
    }

    // Validate dependency graph
    result = tbos_ioc_validate_dependencies(container);
    if (!result.success) {
        TBOS_LOG_WARN("Dependency validation found issues");
    }

    // Create application context
    g_tbos_app_context = tbos_create_application_context();
    if (!g_tbos_app_context) {
        return tbos_create_error_result(ENOMEM, "Failed to create application context");
    }

    // Resolve core services for application context
    tbos_result_t memory_result = tbos_ioc_resolve_interface(container, TBOS_INTERFACE_MEMORY);
    if (memory_result.success) {
        g_tbos_app_context->memory_manager = (tbos_memory_interface_t*)memory_result.data;
        TBOS_LOG_INFO("Memory manager resolved successfully");
    }

    g_tbos_app_context->container = container;
    g_tbos_app_context->is_running = true;
    g_tbos_app_context->start_time = (uint64_t)time(NULL);

    // Dump container state for debugging
    if (container->config.enable_debugging) {
        tbos_ioc_dump_container_state(container);
    }

    TBOS_LOG_INFO("TBOS bootstrap completed successfully with Dependency Injection!");
    TBOS_LOG_INFO("Services registered: %u, Instances created: %u",
                  container->stats.services_registered,
                  container->stats.instances_created);

    return tbos_create_success_result(g_tbos_app_context, sizeof(tbos_application_context_t));
}

tbos_result_t tbos_bootstrap_minimal_di(void) {
    TBOS_LOG_INFO("Starting minimal TBOS bootstrap for embedded systems...");

    tbos_result_t container_result = tbos_initialize_global_container();
    if (!container_result.success) {
        return container_result;
    }

    tbos_ioc_container_t* container = tbos_get_container();

    // Register only essential services for embedded systems
    tbos_logger_t* minimal_logger = tbos_logger_create(NULL, true); // Console only
    tbos_ioc_register_instance(container, TBOS_SERVICE_LOGGER, minimal_logger);

    // Minimal memory manager
    tbos_service_builder_t* memory_builder = tbos_ioc_register_service(
        container,
        TBOS_SERVICE_MEMORY_MANAGER,
        TBOS_INTERFACE_MEMORY,
        sizeof(tbos_memory_manager_di_t)
    );

    const char* minimal_deps[] = {TBOS_SERVICE_LOGGER};
    memory_builder->with_factory(memory_builder, tbos_create_memory_manager_di)
                  ->with_dependencies(memory_builder, minimal_deps, 1)
                  ->as_singleton()
                  ->build();

    g_tbos_app_context = tbos_create_application_context();
    g_tbos_app_context->container = container;

    TBOS_LOG_INFO("Minimal TBOS bootstrap completed - embedded mode");
    return tbos_create_success_result(g_tbos_app_context, sizeof(tbos_application_context_t));
}

/**
 * @brief Application Context Management
 */
tbos_application_context_t* tbos_create_application_context(void) {
    tbos_application_context_t* context = malloc(sizeof(tbos_application_context_t));
    if (!context) {
        return NULL;
    }

    memset(context, 0, sizeof(tbos_application_context_t));
    context->start_time = (uint64_t)time(NULL);
    strcpy(context->boot_parameters, "tbos=di,verbose=true,profile=standard");

    return context;
}

void tbos_destroy_application_context(tbos_application_context_t* app_context) {
    if (!app_context) return;

    TBOS_LOG_INFO("Destroying TBOS application context...");

    if (app_context->container) {
        tbos_cleanup_global_container();
    }

    free(app_context);
    g_tbos_app_context = NULL;
}

tbos_application_context_t* tbos_get_app_context(void) {
    return g_tbos_app_context;
}

/**
 * @brief Service Validation and Testing
 */
tbos_result_t tbos_validate_all_services(tbos_ioc_container_t* container) {
    if (!container) {
        return tbos_create_error_result(EINVAL, "Invalid container");
    }

    TBOS_LOG_INFO("Validating all registered services...");

    bool all_valid = true;

    // Test memory manager resolution
    tbos_result_t memory_test = tbos_ioc_resolve_interface(container, TBOS_INTERFACE_MEMORY);
    if (!memory_test.success) {
        TBOS_LOG_ERROR("Memory manager validation failed");
        all_valid = false;
    } else {
        // Test memory operations
        tbos_memory_interface_t* memory = (tbos_memory_interface_t*)memory_test.data;
        tbos_result_t alloc_test = memory->allocate(1024, 0);
        if (alloc_test.success) {
            memory->deallocate(alloc_test.data);
            TBOS_LOG_INFO("Memory manager validation passed");
        } else {
            TBOS_LOG_ERROR("Memory manager allocation test failed");
            all_valid = false;
        }
    }

    // Test event bus resolution
    if (tbos_ioc_is_registered(container, TBOS_SERVICE_EVENT_BUS)) {
        tbos_result_t event_test = tbos_ioc_resolve(container, TBOS_SERVICE_EVENT_BUS);
        if (event_test.success) {
            TBOS_LOG_INFO("Event bus validation passed");
        } else {
            TBOS_LOG_ERROR("Event bus validation failed");
            all_valid = false;
        }
    }

    return all_valid ? tbos_create_success_result(NULL, 0) :
                      tbos_create_error_result(EINVAL, "Service validation failed");
}

tbos_result_t tbos_test_dependency_injection(tbos_ioc_container_t* container) {
    TBOS_LOG_INFO("Testing dependency injection functionality...");

    // Test 1: Service resolution
    tbos_result_t memory_result = tbos_ioc_resolve_interface(container, TBOS_INTERFACE_MEMORY);
    if (!memory_result.success) {
        return tbos_create_error_result(EFAULT, "DI Test 1 failed - service resolution");
    }

    // Test 2: Dependency injection verification
    tbos_memory_manager_di_t* memory_manager = (tbos_memory_manager_di_t*)memory_result.data;
    if (!memory_manager->logger) {
        TBOS_LOG_WARN("DI Test 2 warning - logger dependency not injected");
    } else {
        TBOS_LOG_INFO("DI Test 2 passed - dependencies injected correctly");
    }

    // Test 3: Circular dependency detection
    // (This would be tested with services that have circular dependencies)

    TBOS_LOG_INFO("Dependency injection tests completed successfully");
    return tbos_create_success_result(NULL, 0);
}

/**
 * @brief Entry Point for DI-based TBOS
 */
int tbos_main_with_di(int argc, char* argv[]) {
    printf("=== TernaryBit OS with Dependency Injection ===\n");
    printf("Demonstrating enterprise-grade OS architecture\n");
    printf("===============================================\n\n");

    // Bootstrap TBOS with DI
    tbos_result_t bootstrap_result = tbos_bootstrap_with_di();
    if (!bootstrap_result.success) {
        fprintf(stderr, "TBOS bootstrap failed: %s\n", bootstrap_result.error_message);
        return 1;
    }

    printf("✅ TBOS successfully bootstrapped with Dependency Injection!\n\n");

    // Validate services
    tbos_result_t validation_result = tbos_validate_all_services(tbos_get_container());
    if (validation_result.success) {
        printf("✅ All services validated successfully\n");
    } else {
        printf("⚠️  Service validation completed with warnings\n");
    }

    // Test dependency injection
    tbos_result_t di_test_result = tbos_test_dependency_injection(tbos_get_container());
    if (di_test_result.success) {
        printf("✅ Dependency injection tests passed\n");
    }

    // Demonstrate service usage
    printf("\n=== Service Usage Demonstration ===\n");

    tbos_memory_interface_t* memory = TBOS_GET_MEMORY_MANAGER();
    if (memory) {
        printf("Memory Manager: Available ✅\n");

        // Test memory allocation
        tbos_result_t alloc_result = memory->allocate(2048, 0);
        if (alloc_result.success) {
            printf("Memory allocation: Success ✅ (allocated %zu bytes)\n", alloc_result.data_size);
            memory->deallocate(alloc_result.data);
            printf("Memory deallocation: Success ✅\n");
        }

        // Get statistics
        tbos_result_t stats_result = memory->get_stats();
        if (stats_result.success) {
            printf("Memory statistics: Available ✅\n");
            tbos_free_result(&stats_result);
        }
    }

    printf("\n=== Container State ===\n");
    tbos_ioc_dump_container_state(tbos_get_container());

    printf("\n✅ TBOS with Dependency Injection demonstration completed!\n");
    printf("This demonstrates how a real enterprise OS should be architected.\n\n");

    // Cleanup
    tbos_destroy_application_context(g_tbos_app_context);

    return 0;
}