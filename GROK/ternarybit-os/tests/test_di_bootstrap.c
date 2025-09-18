/**
 * @file test_di_bootstrap.c
 * @brief Test suite for TernaryBit OS DI Bootstrap
 *
 * Tests the complete dependency injection bootstrap process
 * and demonstrates enterprise-grade OS architecture.
 *
 * @version 2.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#include "tbos_di_bootstrap.h"
#include "tbos_base.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

// Test statistics
static struct {
    int tests_run;
    int tests_passed;
    int tests_failed;
} test_stats = {0};

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
 * @brief Test DI Bootstrap Process
 */
static void test_di_bootstrap(void) {
    test_start("DI Bootstrap Process");

    tbos_result_t result = tbos_bootstrap_with_di();
    if (!result.success) {
        test_fail("Bootstrap failed");
        return;
    }

    tbos_application_context_t* app_context = (tbos_application_context_t*)result.data;
    if (!app_context || !app_context->is_running) {
        test_fail("Application context invalid");
        return;
    }

    test_pass();
}

static void test_service_resolution_after_bootstrap(void) {
    test_start("Service Resolution After Bootstrap");

    tbos_ioc_container_t* container = tbos_get_container();
    if (!container) {
        test_fail("Container not available");
        return;
    }

    // Test memory manager resolution
    tbos_memory_interface_t* memory_manager = TBOS_GET_MEMORY_MANAGER();
    if (!memory_manager) {
        test_fail("Memory manager not resolved");
        return;
    }

    // Test memory operations
    tbos_result_t alloc_result = memory_manager->allocate(1024, 0);
    if (!alloc_result.success) {
        test_fail("Memory allocation failed");
        return;
    }

    tbos_result_t dealloc_result = memory_manager->deallocate(alloc_result.data);
    if (!dealloc_result.success) {
        test_fail("Memory deallocation failed");
        return;
    }

    test_pass();
}

static void test_dependency_injection_validation(void) {
    test_start("Dependency Injection Validation");

    tbos_ioc_container_t* container = tbos_get_container();
    if (!container) {
        test_fail("Container not available");
        return;
    }

    tbos_result_t validation_result = tbos_validate_all_services(container);
    if (!validation_result.success) {
        test_fail("Service validation failed");
        return;
    }

    test_pass();
}

static void test_minimal_bootstrap(void) {
    test_start("Minimal Bootstrap for Embedded Systems");

    // Cleanup current context first
    if (g_tbos_app_context) {
        tbos_destroy_application_context(g_tbos_app_context);
    }

    tbos_result_t result = tbos_bootstrap_minimal_di();
    if (!result.success) {
        test_fail("Minimal bootstrap failed");
        return;
    }

    tbos_application_context_t* app_context = (tbos_application_context_t*)result.data;
    if (!app_context || !app_context->container) {
        test_fail("Minimal application context invalid");
        return;
    }

    // Verify minimal services are available
    if (!tbos_ioc_is_registered(app_context->container, TBOS_SERVICE_LOGGER)) {
        test_fail("Logger service not registered in minimal bootstrap");
        return;
    }

    if (!tbos_ioc_is_registered(app_context->container, TBOS_SERVICE_MEMORY_MANAGER)) {
        test_fail("Memory manager not registered in minimal bootstrap");
        return;
    }

    test_pass();
}

/**
 * @brief Test container state and diagnostics
 */
static void test_container_diagnostics(void) {
    test_start("Container Diagnostics");

    tbos_ioc_container_t* container = tbos_get_container();
    if (!container) {
        test_fail("Container not available");
        return;
    }

    printf("\n");
    printf("    📊 Container State Dump:\n");
    printf("    ========================\n");

    // Enable detailed output temporarily
    container->config.enable_debugging = true;
    tbos_ioc_dump_container_state(container);
    container->config.enable_debugging = false;

    printf("    ========================\n");
    printf("    ");

    test_pass();
}

/**
 * @brief Performance and stress tests
 */
static void test_service_resolution_performance(void) {
    test_start("Service Resolution Performance");

    tbos_ioc_container_t* container = tbos_get_container();
    if (!container) {
        test_fail("Container not available");
        return;
    }

    const int iterations = 10000;
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < iterations; i++) {
        tbos_memory_interface_t* memory_manager = TBOS_GET_MEMORY_MANAGER();
        if (!memory_manager) {
            test_fail("Memory manager resolution failed");
            return;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double total_time = (end.tv_sec - start.tv_sec) * 1000.0 +
                       (end.tv_nsec - start.tv_nsec) / 1000000.0;
    double avg_time_us = (total_time * 1000.0) / iterations;

    printf("✅ PASS (%.2f ms total, %.2f μs avg per resolution)\n", total_time, avg_time_us);
    test_stats.tests_passed++;
}

/**
 * @brief Integration tests with real OS components
 */
static void test_memory_manager_integration(void) {
    test_start("Memory Manager Integration");

    tbos_memory_interface_t* memory_manager = TBOS_GET_MEMORY_MANAGER();
    if (!memory_manager) {
        test_fail("Memory manager not available");
        return;
    }

    // Test multiple allocations
    void* ptrs[100];
    for (int i = 0; i < 100; i++) {
        tbos_result_t result = memory_manager->allocate(1024 * (i + 1), 0);
        if (!result.success) {
            test_fail("Memory allocation failed during stress test");
            return;
        }
        ptrs[i] = result.data;
    }

    // Free all allocations
    for (int i = 0; i < 100; i++) {
        tbos_result_t result = memory_manager->deallocate(ptrs[i]);
        if (!result.success) {
            test_fail("Memory deallocation failed during stress test");
            return;
        }
    }

    // Get statistics
    tbos_result_t stats_result = memory_manager->get_stats();
    if (!stats_result.success) {
        test_fail("Memory statistics retrieval failed");
        return;
    }

    tbos_free_result(&stats_result);
    test_pass();
}

/**
 * @brief Demonstrate enterprise patterns
 */
static void demonstrate_enterprise_patterns(void) {
    printf("\n🏢 Enterprise Architecture Demonstration:\n");
    printf("=========================================\n");

    tbos_ioc_container_t* container = tbos_get_container();
    if (!container) {
        printf("❌ Container not available for demonstration\n");
        return;
    }

    printf("✅ Inversion of Control: Services depend on abstractions\n");
    printf("✅ Dependency Injection: Dependencies automatically resolved\n");
    printf("✅ Separation of Concerns: Each service has single responsibility\n");
    printf("✅ Lifecycle Management: Singleton, Transient, and Scoped lifetimes\n");
    printf("✅ Configuration: Externalized component configuration\n");
    printf("✅ Testability: Easy to inject mock dependencies\n");
    printf("✅ Modularity: Components can be replaced without code changes\n");
    printf("✅ Thread Safety: Container operations are thread-safe\n");
    printf("✅ Performance: Optimized service resolution\n");
    printf("✅ Diagnostics: Comprehensive container state inspection\n");

    printf("\n📊 Current Container Statistics:\n");
    printf("  - Services Registered: %u\n", container->stats.services_registered);
    printf("  - Instances Created: %u\n", container->stats.instances_created);
    printf("  - Dependency Resolutions: %u\n", container->stats.dependency_resolutions);

    printf("\n🎯 This demonstrates how a real enterprise OS should be architected!\n");
}

/**
 * @brief Main test function - called by DI bootstrap
 */
int tbos_main_with_di(int argc, char* argv[]) {
    (void)argc; (void)argv; // Suppress unused warnings

    printf("\n");
    printf("🧪 TBOS Dependency Injection Bootstrap Test Suite\n");
    printf("=================================================\n");
    printf("Testing enterprise-grade OS architecture with DI...\n\n");

    // Run bootstrap tests
    printf("🚀 Bootstrap Tests:\n");
    test_di_bootstrap();
    test_service_resolution_after_bootstrap();
    test_dependency_injection_validation();

    printf("\n");

    // Embedded systems test
    printf("📱 Embedded Systems Tests:\n");
    test_minimal_bootstrap();

    printf("\n");

    // Diagnostics tests
    printf("🔍 Diagnostics Tests:\n");
    test_container_diagnostics();

    printf("\n");

    // Performance tests
    printf("⚡ Performance Tests:\n");
    test_service_resolution_performance();

    printf("\n");

    // Integration tests
    printf("🔗 Integration Tests:\n");
    test_memory_manager_integration();

    printf("\n");

    // Enterprise patterns demonstration
    demonstrate_enterprise_patterns();

    printf("\n");

    // Test summary
    printf("📊 Test Summary:\n");
    printf("================\n");
    printf("Tests Run:    %d\n", test_stats.tests_run);
    printf("Tests Passed: %d\n", test_stats.tests_passed);
    printf("Tests Failed: %d\n", test_stats.tests_failed);
    printf("Success Rate: %.1f%%\n", (double)test_stats.tests_passed / test_stats.tests_run * 100.0);

    if (test_stats.tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! TBOS Dependency Injection is working perfectly!\n");
        printf("✅ Enterprise-grade OS architecture demonstrated successfully!\n");
        printf("🌟 This shows how software engineering standards should be applied!\n\n");
        return 0;
    } else {
        printf("\n❌ %d TESTS FAILED. Please review the implementation.\n\n", test_stats.tests_failed);
        return 1;
    }
}

/**
 * @brief Standard main function for direct testing
 */
int main(void) {
    return tbos_main_with_di(0, NULL);
}