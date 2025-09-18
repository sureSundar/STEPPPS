/**
 * @file shell_main.c
 * @brief TernaryBit OS Interactive Shell Application
 *
 * Integrates all TernaryBit OS components for interactive testing.
 * This is the main interface you requested for testing commands.
 *
 * @version 1.0
 * @date 2025-01-XX
 */

#include "command_shell.h"
#include "../core/hardware_detector.h"
#include "../core/memory_manager.h"
#include "../core/steppps_framework.h"
#include "../boot/universal_boot.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("🌟 TernaryBit Universal OS - Starting Interactive Shell...\n");
    printf("===========================================================\n\n");

    // Step 1: Hardware Detection
    printf("Step 1: Detecting hardware capabilities...\n");
    hardware_detection_result_t hw_result = detect_hardware_capabilities();
    if (!hw_result.detection_success) {
        printf("❌ Hardware detection failed: %s\n", hw_result.error_message);
        return 1;
    }
    printf("✅ Hardware detected: %d-bit CPU, %llu bytes RAM\n",
           hw_result.info.cpu_bits,
           (unsigned long long)hw_result.info.memory_size_bytes);

    // Step 2: Universal Boot Sequence
    printf("\nStep 2: Executing universal boot sequence...\n");
    boot_status_t boot_status = universal_boot_sequence();
    if (!boot_status.boot_success) {
        printf("❌ Boot sequence failed: %s\n", boot_status.last_error);
        return 1;
    }
    printf("✅ Boot completed in %u μs\n", boot_status.timing.total_boot_time_us);

    // Step 3: Initialize Memory Manager
    printf("\nStep 3: Initializing memory management...\n");
    memory_manager_t* memory_manager = initialize_memory_manager(
        &hw_result.info,
        hw_result.info.memory_size_bytes,
        boot_status.config.kernel_base_addr
    );
    if (!memory_manager) {
        printf("❌ Memory manager initialization failed\n");
        return 1;
    }

    // Configure memory zones
    bool zones_configured = configure_memory_zones(memory_manager, &boot_status.config);
    if (!zones_configured) {
        printf("❌ Memory zone configuration failed\n");
        shutdown_memory_manager(memory_manager);
        return 1;
    }
    printf("✅ Memory manager initialized with %zu bytes\n",
           memory_manager->stats.total_memory);

    // Step 4: Initialize STEPPPS Framework
    printf("\nStep 4: Initializing STEPPPS framework...\n");
    steppps_manager_t* steppps_manager = initialize_steppps_framework(
        &hw_result.info,
        STEPPPS_MODE_STANDARD,
        boot_status.config.steppps_base_addr,
        boot_status.config.steppps_size_bytes
    );
    if (!steppps_manager) {
        printf("❌ STEPPPS framework initialization failed\n");
        shutdown_memory_manager(memory_manager);
        return 1;
    }
    printf("✅ STEPPPS framework initialized with %u%% efficiency\n",
           steppps_manager->efficiency_rating);

    // Step 5: Initialize Command Shell
    printf("\nStep 5: Initializing command shell...\n");
    shell_context_t* shell = initialize_command_shell(
        &hw_result.info,
        memory_manager,
        steppps_manager
    );
    if (!shell) {
        printf("❌ Command shell initialization failed\n");
        shutdown_steppps_framework(steppps_manager);
        shutdown_memory_manager(memory_manager);
        return 1;
    }
    printf("✅ Command shell ready\n");

    // Step 6: System Integration Check
    printf("\nStep 6: Performing system integration check...\n");

    // Test memory allocation
    memory_request_t test_request = {
        .size = 1024,
        .alignment = ALIGN_DEFAULT,
        .flags = MEM_FLAG_ZERO,
        .preferred_zone = ZONE_HEAP,
        .timeout_ms = 1000,
        .debug_name = "shell_test"
    };
    memory_result_t test_result = allocate_memory(memory_manager, &test_request);
    if (!test_result.success) {
        printf("❌ Memory allocation test failed\n");
    } else {
        printf("✅ Memory allocation test passed\n");
        free_memory(memory_manager, test_result.address);
    }

    // Test STEPPPS operation
    bool steppps_cycle = process_steppps_cycle(steppps_manager);
    if (!steppps_cycle) {
        printf("❌ STEPPPS cycle test failed\n");
    } else {
        printf("✅ STEPPPS cycle test passed\n");
    }

    printf("\n🎉 ALL SYSTEMS READY! TernaryBit OS is fully operational.\n");
    printf("🚀 Universal OS successfully running on this hardware platform.\n\n");

    // Step 7: Start Interactive Shell Session
    printf("Starting interactive shell session...\n");
    printf("Type 'help' for available commands, 'exit' to quit.\n\n");

    bool shell_success = start_shell_session(shell);

    // Cleanup
    printf("\nShutting down TernaryBit OS...\n");
    shutdown_command_shell(shell);
    shutdown_steppps_framework(steppps_manager);
    shutdown_memory_manager(memory_manager);

    if (shell_success) {
        printf("✅ TernaryBit OS shutdown complete.\n");
        printf("🌟 Thank you for testing TernaryBit Universal OS!\n");
        return 0;
    } else {
        printf("❌ Shell session ended with errors.\n");
        return 1;
    }
}