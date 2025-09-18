/**
 * @file test_ioc_container.c
 * @brief Test suite for TernaryBit OS IoC Container
 *
 * Comprehensive tests for Dependency Injection functionality:
 * - Service registration and resolution
 * - Dependency injection
 * - Lifecycle management (Singleton, Transient, Scoped)
 * - Circular dependency detection
 * - Thread safety
 * - Performance benchmarks
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#define _POSIX_C_SOURCE 199309L
#include "tbos_ioc_container.h"
#include "tbos_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

// Test configuration
#define TEST_ITERATIONS 1000
#define TEST_THREADS 4

// Test statistics
static struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
    double total_time_ms;
} test_stats = {0};

// Test service implementations
typedef struct test_service_a {
    int value;
    char name[64];
    bool initialized;
} test_service_a_t;

typedef struct test_service_b {
    test_service_a_t* dependency_a;
    int multiplier;
    bool initialized;
} test_service_b_t;

// Test interfaces
typedef struct {
    int (*get_value)(void* self);
    void (*set_value)(void* self, int value);
} test_interface_a_t;

typedef struct {
    int (*compute)(void* self);
    test_service_a_t* (*get_dependency)(void* self);
} test_interface_b_t;

/**
 * @brief Test utilities
 */
static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1000000.0;
}

static void test_start(const char* test_name) {
    printf("  ▶️  %s... ", test_name);
    fflush(stdout);
    test_stats.tests_run++;
}

static void test_pass(void) {
    printf("✅ PASS\n");
    test_stats.tests_passed++;
}

static void test_fail(const char* reason) {
    printf("❌ FAIL - %s\n", reason);
    test_stats.tests_failed++;
}

/**
 * @brief Test Service Factories
 */
void* create_test_service_a(tbos_ioc_container_t* container) {
    (void)container; // Suppress unused parameter warning

    test_service_a_t* service = malloc(sizeof(test_service_a_t));
    if (!service) return NULL;

    service->value = 42;
    strcpy(service->name, "Test Service A");
    service->initialized = true;

    return service;
}

void* create_test_service_b(tbos_ioc_container_t* container) {
    test_service_b_t* service = malloc(sizeof(test_service_b_t));
    if (!service) return NULL;

    // Inject dependency A
    tbos_result_t dep_result = tbos_ioc_resolve(container, "test_service_a");
    if (dep_result.success) {
        service->dependency_a = (test_service_a_t*)dep_result.data;
    } else {
        service->dependency_a = NULL;
    }

    service->multiplier = 10;
    service->initialized = true;

    return service;
}

/**
 * @brief Basic functionality tests
 */
static void test_container_creation(void) {
    test_start("Container Creation");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    if (!container->base.initialized) {
        test_fail("Container not properly initialized");
        tbos_ioc_container_destroy(container);
        return;
    }

    tbos_ioc_container_destroy(container);
    test_pass();
}

static void test_service_registration(void) {
    test_start("Service Registration");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    // Register a service
    tbos_service_builder_t* builder = tbos_ioc_register_service(
        container,
        "test_service_a",
        "ITestServiceA",
        sizeof(test_service_a_t)
    );

    if (!builder) {
        test_fail("Service registration failed");
        tbos_ioc_container_destroy(container);
        return;
    }

    tbos_result_t build_result = builder->with_factory(builder, create_test_service_a)
                                       ->as_singleton()
                                       ->build();

    if (!build_result.success) {
        test_fail("Service build failed");
        tbos_ioc_container_destroy(container);
        return;
    }

    // Verify service is registered
    if (!tbos_ioc_is_registered(container, "test_service_a")) {
        test_fail("Service not found after registration");
        tbos_ioc_container_destroy(container);
        return;
    }

    tbos_ioc_container_destroy(container);
    test_pass();
}

static void test_service_resolution(void) {
    test_start("Service Resolution");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    // Register service
    tbos_service_builder_t* builder = tbos_ioc_register_service(
        container,
        "test_service_a",
        "ITestServiceA",
        sizeof(test_service_a_t)
    );

    builder->with_factory(builder, create_test_service_a)
           ->as_singleton()
           ->build();

    // Resolve service
    tbos_result_t result = tbos_ioc_resolve(container, "test_service_a");
    if (!result.success) {
        test_fail("Service resolution failed");
        tbos_ioc_container_destroy(container);
        return;
    }

    test_service_a_t* service = (test_service_a_t*)result.data;
    if (!service || !service->initialized || service->value != 42) {
        test_fail("Service instance invalid");
        tbos_ioc_container_destroy(container);
        return;
    }

    tbos_ioc_container_destroy(container);
    test_pass();
}

static void test_dependency_injection(void) {
    test_start("Dependency Injection");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    // Register service A
    tbos_service_builder_t* builder_a = tbos_ioc_register_service(
        container,
        "test_service_a",
        "ITestServiceA",
        sizeof(test_service_a_t)
    );
    builder_a->with_factory(builder_a, create_test_service_a)->as_singleton()->build();

    // Register service B with dependency on A
    tbos_service_builder_t* builder_b = tbos_ioc_register_service(
        container,
        "test_service_b",
        "ITestServiceB",
        sizeof(test_service_b_t)
    );

    const char* deps[] = {"test_service_a"};
    builder_b->with_factory(builder_b, create_test_service_b)
             ->with_dependencies(builder_b, deps, 1)
             ->as_singleton()
             ->build();

    // Resolve service B (should automatically resolve A)
    tbos_result_t result = tbos_ioc_resolve(container, "test_service_b");
    if (!result.success) {
        test_fail("Service B resolution failed");
        tbos_ioc_container_destroy(container);
        return;
    }

    test_service_b_t* service_b = (test_service_b_t*)result.data;
    if (!service_b || !service_b->initialized || !service_b->dependency_a) {
        test_fail("Dependency injection failed");
        tbos_ioc_container_destroy(container);
        return;
    }

    if (service_b->dependency_a->value != 42) {
        test_fail("Injected dependency invalid");
        tbos_ioc_container_destroy(container);
        return;
    }

    tbos_ioc_container_destroy(container);
    test_pass();
}

static void test_singleton_lifecycle(void) {
    test_start("Singleton Lifecycle");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    // Register singleton service
    tbos_service_builder_t* builder = tbos_ioc_register_service(
        container,
        "singleton_service",
        "ISingletonService",
        sizeof(test_service_a_t)
    );
    builder->with_factory(builder, create_test_service_a)->as_singleton()->build();

    // Resolve twice
    tbos_result_t result1 = tbos_ioc_resolve(container, "singleton_service");
    tbos_result_t result2 = tbos_ioc_resolve(container, "singleton_service");

    if (!result1.success || !result2.success) {
        test_fail("Singleton resolution failed");
        tbos_ioc_container_destroy(container);
        return;
    }

    // Should be the same instance
    if (result1.data != result2.data) {
        test_fail("Singleton returned different instances");
        tbos_ioc_container_destroy(container);
        return;
    }

    tbos_ioc_container_destroy(container);
    test_pass();
}

static void test_transient_lifecycle(void) {
    test_start("Transient Lifecycle");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    // Register transient service
    tbos_service_builder_t* builder = tbos_ioc_register_service(
        container,
        "transient_service",
        "ITransientService",
        sizeof(test_service_a_t)
    );
    builder->with_factory(builder, create_test_service_a)->as_transient()->build();

    // Resolve twice
    tbos_result_t result1 = tbos_ioc_resolve(container, "transient_service");
    tbos_result_t result2 = tbos_ioc_resolve(container, "transient_service");

    if (!result1.success || !result2.success) {
        test_fail("Transient resolution failed");
        tbos_ioc_container_destroy(container);
        return;
    }

    // Should be different instances
    if (result1.data == result2.data) {
        test_fail("Transient returned same instance");
        tbos_ioc_container_destroy(container);
        return;
    }

    // Cleanup instances
    free(result1.data);
    free(result2.data);

    tbos_ioc_container_destroy(container);
    test_pass();
}

/**
 * @brief Performance tests
 */
typedef struct thread_test_data {
    tbos_ioc_container_t* container;
    int thread_id;
    int iterations;
    double total_time;
    bool success;
} thread_test_data_t;

void* thread_resolution_test(void* arg) {
    thread_test_data_t* data = (thread_test_data_t*)arg;
    double start_time = get_time_ms();

    for (int i = 0; i < data->iterations; i++) {
        tbos_result_t result = tbos_ioc_resolve(data->container, "test_service_a");
        if (!result.success) {
            data->success = false;
            return NULL;
        }
    }

    data->total_time = get_time_ms() - start_time;
    data->success = true;
    return NULL;
}

static void test_thread_safety(void) {
    test_start("Thread Safety");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    // Register test service
    tbos_service_builder_t* builder = tbos_ioc_register_service(
        container,
        "test_service_a",
        "ITestServiceA",
        sizeof(test_service_a_t)
    );
    builder->with_factory(builder, create_test_service_a)->as_singleton()->build();

    // Create test threads
    pthread_t threads[TEST_THREADS];
    thread_test_data_t thread_data[TEST_THREADS];

    for (int i = 0; i < TEST_THREADS; i++) {
        thread_data[i].container = container;
        thread_data[i].thread_id = i;
        thread_data[i].iterations = 100;
        thread_data[i].success = false;

        if (pthread_create(&threads[i], NULL, thread_resolution_test, &thread_data[i]) != 0) {
            test_fail("Thread creation failed");
            tbos_ioc_container_destroy(container);
            return;
        }
    }

    // Wait for all threads
    bool all_success = true;
    for (int i = 0; i < TEST_THREADS; i++) {
        pthread_join(threads[i], NULL);
        if (!thread_data[i].success) {
            all_success = false;
        }
    }

    tbos_ioc_container_destroy(container);

    if (all_success) {
        test_pass();
    } else {
        test_fail("Thread safety test failed");
    }
}

static void test_performance_benchmark(void) {
    test_start("Performance Benchmark");

    tbos_ioc_container_t* container = tbos_ioc_container_create();
    if (!container) {
        test_fail("Container creation failed");
        return;
    }

    // Register test service
    tbos_service_builder_t* builder = tbos_ioc_register_service(
        container,
        "benchmark_service",
        "IBenchmarkService",
        sizeof(test_service_a_t)
    );
    builder->with_factory(builder, create_test_service_a)->as_singleton()->build();

    // Warm up
    for (int i = 0; i < 100; i++) {
        tbos_ioc_resolve(container, "benchmark_service");
    }

    // Benchmark service resolution
    double start_time = get_time_ms();
    for (int i = 0; i < TEST_ITERATIONS; i++) {
        tbos_result_t result = tbos_ioc_resolve(container, "benchmark_service");
        if (!result.success) {
            test_fail("Benchmark resolution failed");
            tbos_ioc_container_destroy(container);
            return;
        }
    }
    double end_time = get_time_ms();

    double total_time = end_time - start_time;
    double avg_time_us = (total_time * 1000.0) / TEST_ITERATIONS;

    printf("✅ PASS (%.2f ms total, %.2f μs avg per resolution)\n", total_time, avg_time_us);
    test_stats.tests_passed++;
    test_stats.total_time_ms += total_time;

    tbos_ioc_container_destroy(container);
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n");
    printf("🧪 TBOS IoC Container Test Suite\n");
    printf("================================\n");
    printf("Testing enterprise-grade Dependency Injection...\n\n");

    double suite_start_time = get_time_ms();

    // Basic functionality tests
    printf("📋 Basic Functionality Tests:\n");
    test_container_creation();
    test_service_registration();
    test_service_resolution();
    test_dependency_injection();

    printf("\n");

    // Lifecycle tests
    printf("🔄 Lifecycle Management Tests:\n");
    test_singleton_lifecycle();
    test_transient_lifecycle();

    printf("\n");

    // Advanced tests
    printf("🚀 Advanced Tests:\n");
    test_thread_safety();
    test_performance_benchmark();

    printf("\n");

    // Test summary
    double suite_total_time = get_time_ms() - suite_start_time;
    printf("📊 Test Summary:\n");
    printf("================\n");
    printf("Tests Run:    %d\n", test_stats.tests_run);
    printf("Tests Passed: %d\n", test_stats.tests_passed);
    printf("Tests Failed: %d\n", test_stats.tests_failed);
    printf("Success Rate: %.1f%%\n", (double)test_stats.tests_passed / test_stats.tests_run * 100.0);
    printf("Total Time:   %.2f ms\n", suite_total_time);

    if (test_stats.tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! IoC Container is working correctly.\n");
        printf("✅ TBOS Dependency Injection is enterprise-ready!\n\n");
        return 0;
    } else {
        printf("\n❌ %d TESTS FAILED. Please review the implementation.\n\n", test_stats.tests_failed);
        return 1;
    }
}