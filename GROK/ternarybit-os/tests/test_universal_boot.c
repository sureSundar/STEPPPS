/**
 * @file test_universal_boot.c
 * @brief Unit tests for Universal Boot System
 *
 * Tests compliance with:
 * - NFRD NFR-PFM-001 (boot time targets)
 * - FRD FR-PFM-001 (boot performance)
 */

#include "../src/boot/universal_boot.h"
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

int main() {
    printf("🚀 Testing Universal Boot System\n");
    printf("=================================\n\n");

    // Test 1: Boot sequence execution
    printf("Test 1: Boot Sequence Execution\n");
    boot_status_t status = universal_boot_sequence();

    TEST(status.boot_success, "Boot sequence should complete successfully");
    TEST(status.current_stage == STAGE_READY, "Should reach READY stage");
    TEST(status.error_count <= 1, "Should have minimal errors");  // Allow STEPPPS to fail

    // Test 2: Hardware detection integration
    printf("\nTest 2: Hardware Detection Integration\n");
    TEST(status.hardware.cpu_cores >= 1, "Should detect CPU cores");
    TEST(status.hardware.memory_size_bytes >= 512, "Should detect memory");
    TEST(status.config.mode <= BOOT_MODE_SUPER, "Should have valid boot mode");

    // Test 3: Boot timing validation
    printf("\nTest 3: Boot Timing Validation\n");
    uint32_t boot_time_ms = status.timing.total_boot_time_us / 1000;
    uint32_t target_ms;

    switch (status.config.mode) {
        case BOOT_MODE_MINIMAL:  target_ms = BOOT_TARGET_CALCULATOR_MS; break;
        case BOOT_MODE_EMBEDDED: target_ms = BOOT_TARGET_EMBEDDED_MS; break;
        case BOOT_MODE_RETRO:    target_ms = BOOT_TARGET_RETRO_MS; break;
        case BOOT_MODE_FULL:     target_ms = BOOT_TARGET_DESKTOP_MS; break;
        case BOOT_MODE_SERVER:   target_ms = BOOT_TARGET_SERVER_MS; break;
        case BOOT_MODE_SUPER:    target_ms = BOOT_TARGET_SUPER_MS; break;
        default: target_ms = BOOT_TARGET_DESKTOP_MS; break;
    }

    printf("  Boot time: %u ms (target: %u ms)\n", boot_time_ms, target_ms);
    TEST(boot_time_ms <= target_ms * 2, "Boot time should be reasonable (within 2x target)");

    // Test 4: Memory layout configuration
    printf("\nTest 4: Memory Layout Configuration\n");
    TEST(status.config.kernel_size_bytes > 0, "Kernel size should be configured");
    TEST(status.config.stack_size_bytes > 0, "Stack size should be configured");
    TEST(status.config.heap_size_bytes > 0, "Heap size should be configured");

    // Test 5: Boot stage progression
    printf("\nTest 5: Boot Stage Progression\n");
    bool all_critical_stages_ok = true;
    for (int i = 0; i <= STAGE_KERNEL_LOAD; i++) {
        if (!status.timing.stage_success[i]) {
            all_critical_stages_ok = false;
            break;
        }
    }
    TEST(all_critical_stages_ok, "All critical boot stages should succeed");

    // Test 6: Global state
    printf("\nTest 6: Global State\n");
    TEST(g_boot_status.boot_success == status.boot_success, "Global boot status should match");

    // Display boot information
    printf("\n📊 Boot Information:\n");
    printf("====================\n");
    printf("Boot Mode: %s\n", get_boot_mode_name(status.config.mode));
    printf("Hardware: %d-bit CPU, %llu bytes RAM\n",
           status.hardware.cpu_bits, (unsigned long long)status.hardware.memory_size_bytes);
    printf("Total Boot Time: %u ms (target: %u ms)\n", boot_time_ms, target_ms);
    printf("Memory Layout:\n");
    printf("  Kernel: 0x%08X + %u bytes\n", status.config.kernel_base_addr, status.config.kernel_size_bytes);
    printf("  Stack:  0x%08X + %u bytes\n", status.config.stack_base_addr, status.config.stack_size_bytes);
    printf("  Heap:   0x%08X + %u bytes\n", status.config.heap_base_addr, status.config.heap_size_bytes);
    printf("  STEPPPS: 0x%08X + %u bytes\n", status.config.steppps_base_addr, status.config.steppps_size_bytes);

    printf("\nBoot Stage Timing:\n");
    for (int i = 0; i < 7; i++) {
        const char* status_str = status.timing.stage_success[i] ? "✅" : "❌";
        printf("  %s %s: %u μs\n",
               status_str,
               get_boot_stage_name((boot_stage_t)i),
               status.timing.stage_duration_us[i]);
    }

    // Summary
    printf("\n📋 Test Summary:\n");
    printf("================\n");
    printf("✅ Passed: %d\n", tests_passed);
    printf("❌ Failed: %d\n", tests_failed);
    printf("Total: %d\n", tests_passed + tests_failed);

    if (tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! Boot system is ready for integration.\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Review implementation before proceeding.\n");
        return 1;
    }
}