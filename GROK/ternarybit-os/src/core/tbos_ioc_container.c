/**
 * @file tbos_ioc_container.c
 * @brief TernaryBit OS IoC Container Implementation
 *
 * Implements comprehensive IoC and Dependency Injection functionality
 * with focus on performance, thread safety, and robustness.
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#include "tbos_ioc_container.h"
#include "tbos_base.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>  // For uint32_t
#include <stdio.h>   // For debugging

// Global IoC container instance (Singleton)
tbos_ioc_container_t* g_tbos_ioc_container = NULL;
static pthread_mutex_t g_container_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Helper function for string duplication (DRY principle)
 */
static char* tbos_strdup(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

/**
 * @brief Hash function for service name lookup
 */
static uint32_t tbos_hash_service_name(const char* name) {
    if (!name) return 0;

    uint32_t hash = 5381;
    int c;
    while ((c = *name++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

/**
 * @brief Service Builder implementation
 */
typedef struct tbos_service_builder {
    tbos_service_descriptor_t* descriptor;
    tbos_ioc_container_t* container;
    
    // Builder methods
    tbos_service_builder_t* (*as_singleton)(struct tbos_service_builder* self);
    tbos_service_builder_t* (*as_transient)(struct tbos_service_builder* self);
    tbos_service_builder_t* (*as_scoped)(struct tbos_service_builder* self);
    tbos_service_builder_t* (*with_factory)(struct tbos_service_builder* self, 
                                          void* (*factory)(tbos_ioc_container_t*));
    tbos_service_builder_t* (*with_dependencies)(struct tbos_service_builder* self, 
                                               const char** deps, size_t count);
    tbos_service_builder_t* (*with_configuration)(struct tbos_service_builder* self, 
                                                void* config, size_t config_size);
    tbos_result_t (*build)(struct tbos_service_builder* self);
} tbos_service_builder_t;

static tbos_service_builder_t* builder_as_singleton(tbos_service_builder_t* self) {
    if (self && self->descriptor) {
        self->descriptor->lifetime = TBOS_LIFETIME_SINGLETON;
    }
    return self;
}

static tbos_service_builder_t* builder_as_transient(tbos_service_builder_t* self) {
    if (self && self->descriptor) {
        self->descriptor->lifetime = TBOS_LIFETIME_TRANSIENT;
    }
    return self;
}

static tbos_service_builder_t* builder_as_scoped(tbos_service_builder_t* self) {
    if (self && self->descriptor) {
        self->descriptor->lifetime = TBOS_LIFETIME_SCOPED;
    }
    return self;
}

static tbos_service_builder_t* builder_with_factory(tbos_service_builder_t* self,
                                                   void* (*factory)(tbos_ioc_container_t*)) {
    if (self && self->descriptor) {
        self->descriptor->factory_method = factory;
    }
    return self;
}

static tbos_service_builder_t* builder_with_dependencies(tbos_service_builder_t* self,
                                                       const char** deps, size_t count) {
    if (self && self->descriptor && deps && count > 0) {
        // Free existing dependencies if any
        if (self->descriptor->dependencies) {
            for (size_t i = 0; i < self->descriptor->dependency_count; i++) {
                free((void*)self->descriptor->dependencies[i]);
            }
            free(self->descriptor->dependencies);
        }
        
        // Allocate new dependency array
        self->descriptor->dependencies = (const char**)calloc(count, sizeof(char*));
        if (!self->descriptor->dependencies) {
            return self;
        }
        
        // Copy dependencies
        for (size_t i = 0; i < count; i++) {
            self->descriptor->dependencies[i] = strdup(deps[i]);
            if (!self->descriptor->dependencies[i]) {
                // Cleanup on allocation failure
                for (size_t j = 0; j < i; j++) {
                    free((void*)self->descriptor->dependencies[j]);
                }
                free(self->descriptor->dependencies);
                self->descriptor->dependencies = NULL;
                return self;
            }
        }
        
        self->descriptor->dependency_count = count;
    }
    return self;

static tbos_service_builder_t* builder_with_configuration(tbos_service_builder_t* self,
                                                        void* config, size_t config_size) {
    if (self && self->descriptor && config && config_size > 0) {
        // Free existing config if any
        if (self->descriptor->configuration_data) {
            free(self->descriptor->configuration_data);
        }
        
        // Allocate and copy new config
        self->descriptor->configuration_data = malloc(config_size);
        if (self->descriptor->configuration_data) {
            memcpy(self->descriptor->configuration_data, config, config_size);
            self->descriptor->configuration_size = config_size;
        }
    }
    return self;
}

static tbos_result_t builder_build(tbos_service_builder_t* self) {
    tbos_result_t result = {0};
    
    if (!self || !self->container || !self->descriptor) {
        result.error_code = TBOS_ERROR_INVALID_ARGUMENT;
        result.error_message = "Invalid builder state";
        return result;
    }
    
    // Add the service to the container
    result = tbos_ioc_register_service(self->container, self->descriptor);
    
    // Clean up the builder
    free(self);
    
    return result;
}

tbos_service_builder_t* tbos_ioc_builder(tbos_ioc_container_t* container,
                                      const char* service_name,
                                      const char* interface_name) {
    if (!container || !service_name) {
        return NULL;
    }
    
    tbos_service_builder_t* builder = (tbos_service_builder_t*)calloc(1, sizeof(tbos_service_builder_t));
    if (!builder) {
        return NULL;
    }
    
    // Initialize the descriptor
    builder->descriptor = (tbos_service_descriptor_t*)calloc(1, sizeof(tbos_service_descriptor_t));
    if (!builder->descriptor) {
        free(builder);
        return NULL;
    }
    
    // Set up the descriptor
    builder->descriptor->service_name = strdup(service_name);
    if (interface_name) {
        builder->descriptor->interface_name = strdup(interface_name);
    }
    builder->descriptor->lifetime = TBOS_LIFETIME_TRANSIENT; // Default
    
    // Set up the builder methods
    builder->as_singleton = builder_as_singleton;
    builder->as_transient = builder_as_transient;
    builder->as_scoped = builder_as_scoped;
    builder->with_factory = builder_with_factory;
    builder->with_dependencies = builder_with_dependencies;
    builder->with_configuration = builder_with_configuration;
    builder->build = builder_build;
    
    builder->container = container;
    
    return builder;
}

tbos_result_t tbos_ioc_resolve(tbos_ioc_container_t* container, const char* service_name) {
    if (!container || !service_name) {
        return tbos_create_error_result(EINVAL, "Invalid parameters");
    }

    pthread_rwlock_rdlock(&container->container_lock);

    // Find service descriptor
    tbos_service_descriptor_t* descriptor = NULL;
    for (size_t i = 0; i < container->descriptor_count; i++) {
        if (strcmp(container->descriptors[i]->service_name, service_name) == 0) {
            descriptor = container->descriptors[i];
            break;
        }
    }

    if (!descriptor) {
        pthread_rwlock_unlock(&container->container_lock);
        TBOS_LOG_WARN("Service not found: %s", service_name);
        return tbos_create_error_result(ENOENT, "Service not registered");
    }

    // Check for existing singleton instance
    if (descriptor->lifetime == TBOS_LIFETIME_SINGLETON) {
        for (size_t i = 0; i < container->instance_count; i++) {
            tbos_service_instance_t* instance = container->instances[i];
            if (instance->descriptor == descriptor) {
                instance->reference_count++;
                pthread_rwlock_unlock(&container->container_lock);
                return tbos_create_success_result(instance->instance, descriptor->service_size);
            }
        }
    }

    pthread_rwlock_unlock(&container->container_lock);

    // Create new instance
    void* instance = NULL;
    if (descriptor->factory_method) {
        instance = descriptor->factory_method(container);
    } else {
        instance = malloc(descriptor->service_size);
        if (instance) {
            memset(instance, 0, descriptor->service_size);
        }
    }

    if (!instance) {
        TBOS_LOG_ERROR("Failed to create instance of service: %s", service_name);
        return tbos_create_error_result(ENOMEM, "Service instantiation failed");
    }

    // Create service instance wrapper
    tbos_service_instance_t* service_instance = malloc(sizeof(tbos_service_instance_t));
    if (!service_instance) {
        free(instance);
        return tbos_create_error_result(ENOMEM, "Failed to create service instance wrapper");
    }

    service_instance->instance = instance;
    service_instance->descriptor = descriptor;
    service_instance->is_initialized = false;
    service_instance->reference_count = 1;
    service_instance->creation_time = (uint64_t)time(NULL);
    service_instance->owner_thread = pthread_self();
    service_instance->scope_context = NULL;

    if (pthread_mutex_init(&service_instance->instance_mutex, NULL) != 0) {
        free(instance);
        free(service_instance);
        return tbos_create_error_result(errno, "Failed to initialize instance mutex");
    }

    // Add to container
    pthread_rwlock_wrlock(&container->container_lock);

    if (container->instance_count >= container->instance_capacity) {
        size_t new_capacity = container->instance_capacity * 2;
        tbos_service_instance_t** new_instances =
            realloc(container->instances, new_capacity * sizeof(tbos_service_instance_t*));

        if (!new_instances) {
            pthread_mutex_destroy(&service_instance->instance_mutex);
            free(instance);
            free(service_instance);
            pthread_rwlock_unlock(&container->container_lock);
            return tbos_create_error_result(ENOMEM, "Cannot resize instance array");
        }

        container->instances = new_instances;
        container->instance_capacity = new_capacity;
    }

    container->instances[container->instance_count++] = service_instance;
    container->stats.instances_created++;
    container->stats.dependency_resolutions++;

    pthread_rwlock_unlock(&container->container_lock);

    // Initialize service if needed
    if (descriptor->initializer_method) {
        tbos_result_t init_result = descriptor->initializer_method(instance, container);
        if (!init_result.success) {
            TBOS_LOG_ERROR("Service initialization failed: %s", service_name);
            // Don't return the instance if initialization failed
            return init_result;
        }
    }

    service_instance->is_initialized = true;

    TBOS_LOG_DEBUG("Resolved service: %s (instance: %p)", service_name, instance);
    return tbos_create_success_result(instance, descriptor->service_size);
}

bool tbos_ioc_is_registered(tbos_ioc_container_t* container, const char* service_name) {
    if (!container || !service_name) return false;

    pthread_rwlock_rdlock(&container->container_lock);

    for (size_t i = 0; i < container->descriptor_count; i++) {
        if (strcmp(container->descriptors[i]->service_name, service_name) == 0) {
            pthread_rwlock_unlock(&container->container_lock);
            return true;
        }
    }

    pthread_rwlock_unlock(&container->container_lock);
    return false;
}

/**
 * @brief Global container management
 */
tbos_ioc_container_t* tbos_get_container(void) {
    if (!g_tbos_ioc_container) {
        pthread_mutex_lock(&g_container_init_mutex);
        if (!g_tbos_ioc_container) {
            g_tbos_ioc_container = tbos_ioc_container_create();
        }
        pthread_mutex_unlock(&g_container_init_mutex);
    }
    return g_tbos_ioc_container;
}

tbos_result_t tbos_initialize_global_container(void) {
    tbos_ioc_container_t* container = tbos_get_container();
    if (!container) {
        return tbos_create_error_result(ENOMEM, "Failed to create global IoC container");
    }

    TBOS_LOG_INFO("Global IoC container initialized successfully");
    return tbos_create_success_result(container, sizeof(tbos_ioc_container_t));
}

void tbos_cleanup_global_container(void) {
    pthread_mutex_lock(&g_container_init_mutex);
    if (g_tbos_ioc_container) {
        tbos_ioc_container_destroy(g_tbos_ioc_container);
        g_tbos_ioc_container = NULL;
    }
    pthread_mutex_unlock(&g_container_init_mutex);
    TBOS_LOG_INFO("Global IoC container cleaned up");
}

/**
 * @brief Debugging and diagnostics
 */
void tbos_ioc_dump_container_state(tbos_ioc_container_t* container) {
    if (!container) {
        printf("IoC Container: NULL\n");
        return;
    }

    pthread_rwlock_rdlock(&container->container_lock);

    printf("\n=== IoC Container State ===\n");
    printf("Services registered: %u\n", container->stats.services_registered);
    printf("Instances created: %u\n", container->stats.instances_created);
    printf("Dependency resolutions: %u\n", container->stats.dependency_resolutions);
    printf("Current descriptors: %zu/%zu\n", container->descriptor_count, container->descriptor_capacity);
    printf("Current instances: %zu/%zu\n", container->instance_count, container->instance_capacity);
    printf("Scope depth: %zu/%zu\n", container->scope_depth, container->max_scope_depth);

    printf("\nRegistered Services:\n");
    for (size_t i = 0; i < container->descriptor_count; i++) {
        tbos_service_descriptor_t* desc = container->descriptors[i];
        printf("  [%zu] %s -> %s (%s, %zu deps)\n",
               i, desc->service_name, desc->interface_name,
               desc->lifetime == TBOS_LIFETIME_SINGLETON ? "Singleton" :
               desc->lifetime == TBOS_LIFETIME_TRANSIENT ? "Transient" :
               desc->lifetime == TBOS_LIFETIME_SCOPED ? "Scoped" : "Thread",
               desc->dependency_count);
    }

    printf("\nActive Instances:\n");
    for (size_t i = 0; i < container->instance_count; i++) {
        tbos_service_instance_t* instance = container->instances[i];
        printf("  [%zu] %s (refs: %u, initialized: %s, created: %lu)\n",
               i, instance->descriptor->service_name,
               instance->reference_count,
               instance->is_initialized ? "yes" : "no",
               instance->creation_time);
    }

    printf("=== End Container State ===\n\n");

    pthread_rwlock_unlock(&container->container_lock);
}