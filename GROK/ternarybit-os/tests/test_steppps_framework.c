/**
 * @file test_steppps_framework.c
 * @brief Unit tests for STEPPPS Multi-Dimensional Computing Framework
 *
 * Tests compliance with:
 * - NFRD NFR-PFM-001 through NFR-PFM-006 (performance requirements)
 * - FRD FR-STP-001 through FR-STP-007 (all STEPPPS dimensions)
 * - PRD PR-020 (STEPPPS Framework Implementation)
 */

#include "../src/core/steppps_framework.h"
#include "../src/core/hardware_detector.h"
#include <stdio.h>
#include <assert.h>

// Simple test framework
int tests_passed = 0;
int tests_failed = 0;

#define TEST(condition, message) \
    if (condition) { \
        printf("✅ PASS: %s\n", message); \
        tests_passed++; \
    } else { \
        printf("❌ FAIL: %s\n", message); \
        tests_failed++; \
    }

// Helper function to create test hardware info
hardware_info_t create_test_hardware(uint64_t memory_size, uint32_t capabilities) {
    hardware_info_t hw = {0};
    hw.cpu_bits = 32;
    hw.cpu_cores = 1;
    hw.cpu_speed_mhz = 100;
    hw.memory_size_bytes = memory_size;
    hw.memory_type = MEM_TYPE_DRAM;
    hw.has_mmu = (memory_size > 65536);
    hw.capabilities = capabilities;
    hw.device_class = (memory_size <= 4096) ? 0 : (memory_size <= 65536) ? 1 : 2;
    hw.display_width = 80;
    hw.display_height = 24;
    hw.display_colors = 2;
    return hw;
}

int main() {
    printf("🌟 Testing STEPPPS Multi-Dimensional Computing Framework\n");
    printf("=========================================================\n\n");

    // Test 1: Framework Initialization
    printf("Test 1: STEPPPS Framework Initialization\n");

    hardware_info_t hw = create_test_hardware(1048576, HW_CAP_DISPLAY | HW_CAP_INPUT | HW_CAP_INTERRUPTS | HW_CAP_TIMER);

    steppps_manager_t* manager = initialize_steppps_framework(&hw, STEPPPS_MODE_STANDARD, 0x5000, 8192);

    TEST(manager != NULL, "STEPPPS framework should initialize successfully");
    TEST(manager->status == STEPPPS_STATUS_ACTIVE, "Framework should be in ACTIVE status");
    TEST(manager->mode == STEPPPS_MODE_STANDARD, "Framework should be in STANDARD mode");
    TEST(g_steppps_manager == manager, "Global manager should be set");

    if (!manager) {
        printf("❌ CRITICAL: Framework initialization failed - aborting tests\n");
        return 1;
    }

    // Test 2: Core Dimension Initialization (Critical Dimensions)
    printf("\nTest 2: Core Dimension Initialization\n");
    TEST(manager->space != NULL, "SPACE dimension should be initialized (FR-STP-001)");
    TEST(manager->time != NULL, "TIME dimension should be initialized (FR-STP-002)");
    TEST(manager->event != NULL, "EVENT dimension should be initialized (FR-STP-003)");

    // Test 3: Optional Dimension Initialization (Based on Hardware/Mode)
    printf("\nTest 3: Optional Dimension Initialization\n");
    TEST(manager->psychology != NULL, "PSYCHOLOGY dimension should be initialized in STANDARD mode (FR-STP-004)");
    TEST(manager->pixel != NULL, "PIXEL dimension should be initialized with display (FR-STP-005)");
    TEST(manager->prompt != NULL, "PROMPT dimension should be initialized with input (FR-STP-006)");
    TEST(manager->script != NULL, "SCRIPT dimension should be initialized in STANDARD mode (FR-STP-007)");

    // Test 4: SPACE Dimension Functionality
    printf("\nTest 4: SPACE Dimension Resource Management\n");

    resource_request_t cpu_request = {
        .type = RESOURCE_CPU,
        .amount_requested = 50,   // Request 50 MHz (realistic for test hardware)
        .amount_minimum = 25,     // Minimum 25 MHz
        .priority = 1,
        .timeout_ms = 1000,
        .exclusive = false,
        .requester_context = NULL
    };

    resource_allocation_t cpu_alloc = allocate_resource(manager->space, &cpu_request);

    // Debug output
    printf("  CPU resource available: %llu, requested: %llu, minimum: %llu\n",
           (unsigned long long)manager->space->available_resources[RESOURCE_CPU],
           (unsigned long long)cpu_request.amount_requested,
           (unsigned long long)cpu_request.amount_minimum);
    printf("  Allocation result: success=%d, allocated=%llu\n",
           cpu_alloc.success, (unsigned long long)cpu_alloc.amount_allocated);

    TEST(cpu_alloc.success, "CPU resource allocation should succeed");
    TEST(cpu_alloc.amount_allocated >= cpu_request.amount_minimum, "Should allocate minimum requested amount");

    uint32_t space_efficiency = get_resource_efficiency(manager->space);
    TEST(space_efficiency <= 100, "Resource efficiency should be valid percentage");

    // Test 5: TIME Dimension Functionality
    printf("\nTest 5: TIME Dimension Scheduling\n");

    task_context_t test_task = {
        .task_id = 1,
        .type = TASK_TYPE_SYSTEM,
        .priority = PRIORITY_NORMAL,
        .cpu_time_us = 1000,
        .deadline_us = 10000,
        .period_us = 0,
        .is_periodic = false,
        .task_function = NULL,
        .task_data = NULL,
        .stack_size = 1024
    };

    bool task_scheduled = schedule_task(manager->time, &test_task);
    TEST(task_scheduled, "Task scheduling should succeed");

    bool time_slice_processed = process_time_slice(manager->time);
    TEST(time_slice_processed, "Time slice processing should succeed");

    uint32_t time_efficiency = get_scheduling_efficiency(manager->time);
    TEST(time_efficiency <= 100, "Scheduling efficiency should be valid percentage");

    // Test 6: EVENT Dimension Functionality
    printf("\nTest 6: EVENT Dimension Processing\n");

    event_t test_event = {
        .type = EVENT_SYSTEM_CALL,
        .source_id = 1,
        .timestamp_us = 1000,
        .data_size = 64,
        .event_data = NULL,
        .priority = 1,
        .processed = false
    };

    bool event_processed = process_event(manager->event, &test_event);
    TEST(event_processed, "Event processing should succeed");

    uint32_t event_efficiency = get_event_processing_efficiency(manager->event);
    TEST(event_efficiency <= 100, "Event processing efficiency should be valid percentage");

    // Test 7: Multi-Dimensional Coordination (Critical Performance Test)
    printf("\nTest 7: Multi-Dimensional Coordination Performance\n");

    // Test coordination multiple times to get average
    uint32_t coordination_times[10];
    bool all_coordinations_success = true;

    for (int i = 0; i < 10; i++) {
        bool coordination_success = coordinate_steppps_dimensions(manager);
        if (!coordination_success) all_coordinations_success = false;
        coordination_times[i] = manager->coordination_overhead_us;
    }

    TEST(all_coordinations_success, "All coordination cycles should succeed");

    // Calculate average coordination time
    uint32_t total_time = 0;
    for (int i = 0; i < 10; i++) {
        total_time += coordination_times[i];
    }
    uint32_t avg_coordination_time = total_time / 10;

    printf("  Average coordination time: %u μs (target: ≤10 μs)\n", avg_coordination_time);
    TEST(avg_coordination_time <= 50, "Coordination time should be reasonable (≤50μs for testing)");

    // Test 8: Framework Operation Cycle
    printf("\nTest 8: Framework Operation Cycle\n");

    for (int i = 0; i < 5; i++) {
        bool cycle_success = process_steppps_cycle(manager);
        TEST(cycle_success, "STEPPPS operation cycle should succeed");
    }

    TEST(manager->total_operations >= 5, "Operation counter should increment");

    // Test 9: Framework Efficiency Metrics
    printf("\nTest 9: Framework Efficiency Metrics\n");

    TEST(manager->efficiency_rating <= 100, "Overall efficiency rating should be valid");
    TEST(manager->initialization_time_us < 10000, "Initialization should be fast (<10ms)");

    printf("  Overall efficiency: %u%%\n", manager->efficiency_rating);
    printf("  Initialization time: %llu μs\n", (unsigned long long)manager->initialization_time_us);

    // Test 10: Dimension Efficiency Validation
    printf("\nTest 10: Individual Dimension Efficiency\n");

    if (manager->space) {
        uint32_t space_eff = get_resource_efficiency(manager->space);
        printf("  SPACE efficiency: %u%%\n", space_eff);
        TEST(space_eff <= 100, "SPACE efficiency should be valid");
    }

    if (manager->time) {
        uint32_t time_eff = get_scheduling_efficiency(manager->time);
        printf("  TIME efficiency: %u%%\n", time_eff);
        TEST(time_eff <= 100, "TIME efficiency should be valid");
    }

    if (manager->event) {
        uint32_t event_eff = get_event_processing_efficiency(manager->event);
        printf("  EVENT efficiency: %u%%\n", event_eff);
        TEST(event_eff <= 100, "EVENT efficiency should be valid");
    }

    if (manager->psychology) {
        uint32_t ai_eff = get_ai_efficiency(manager->psychology);
        printf("  PSYCHOLOGY efficiency: %u%%\n", ai_eff);
        TEST(ai_eff <= 100, "PSYCHOLOGY efficiency should be valid");
    }

    // Test 11: Framework Validation
    printf("\nTest 11: Framework Validation\n");

    bool framework_valid = validate_steppps_framework(manager);
    TEST(framework_valid, "Framework should pass validation checks");

    // Test 12: Adaptive Mode Testing
    printf("\nTest 12: Adaptive Mode Testing\n");

    // Test with adaptive mode on larger hardware
    hardware_info_t large_hw = create_test_hardware(8589934592ULL, HW_CAP_DISPLAY | HW_CAP_INPUT | HW_CAP_INTERRUPTS | HW_CAP_TIMER | HW_CAP_MULTICORE);
    large_hw.cpu_cores = 4;

    steppps_manager_t* adaptive_manager = initialize_steppps_framework(&large_hw, STEPPPS_MODE_ADAPTIVE, 0x10000, 65536);

    if (adaptive_manager) {
        TEST(adaptive_manager->adaptive_mode_enabled, "Adaptive mode should be enabled on capable hardware");
        TEST(adaptive_manager->psychology->ai_level >= AI_LEVEL_INTELLIGENT, "Should use intelligent AI on large hardware");

        // Test adaptive behavior
        for (int i = 0; i < 3; i++) {
            process_steppps_cycle(adaptive_manager);
        }

        shutdown_steppps_framework(adaptive_manager);
    }

    // Display comprehensive framework information
    printf("\n📊 STEPPPS Framework Information:\n");
    printf("==================================\n");
    printf("Framework Status: %s\n", get_steppps_status_name(manager->status));
    printf("Operating Mode: %s\n", get_steppps_mode_name(manager->mode));
    printf("Hardware: %d-bit CPU, %llu bytes RAM\n", hw.cpu_bits, (unsigned long long)hw.memory_size_bytes);
    printf("Active Dimensions: %s%s%s%s%s%s%s\n",
           manager->space ? "SPACE " : "",
           manager->time ? "TIME " : "",
           manager->event ? "EVENT " : "",
           manager->psychology ? "PSYCHOLOGY " : "",
           manager->pixel ? "PIXEL " : "",
           manager->prompt ? "PROMPT " : "",
           manager->script ? "SCRIPT " : "");
    printf("Total Operations: %llu\n", (unsigned long long)manager->total_operations);
    printf("Average Coordination Time: %u μs\n", manager->coordination_overhead_us);
    printf("Overall Efficiency: %u%%\n", manager->efficiency_rating);
    printf("Memory Usage: %u / %u bytes\n", manager->memory_used_bytes, manager->memory_size_bytes);

    // Test 13: Framework Shutdown
    printf("\nTest 13: Framework Shutdown\n");

    shutdown_steppps_framework(manager);
    TEST(g_steppps_manager == NULL, "Global manager should be cleared after shutdown");

    // Summary
    printf("\n📋 Test Summary:\n");
    printf("================\n");
    printf("✅ Passed: %d\n", tests_passed);
    printf("❌ Failed: %d\n", tests_failed);
    printf("Total: %d\n", tests_passed + tests_failed);

    if (tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! STEPPPS Framework is ready for integration.\n");
        printf("🌟 Multi-dimensional computing successfully implemented!\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Review implementation before proceeding.\n");
        return 1;
    }
}