/**
 * @file test_hardware_detector.c
 * @brief Unit tests for Universal Hardware Detection System
 *
 * Tests compliance with:
 * - NFRD NFR-PFM-007 (detection time <1s)
 * - FRD FR-UHS-001 (CPU architecture support)
 * - FRD FR-UHS-002 (memory adaptation)
 */

#include "../src/core/hardware_detector.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

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
    printf("🧪 Testing Universal Hardware Detection System\n");
    printf("================================================\n\n");

    // Test 1: Basic hardware detection
    printf("Test 1: Basic Hardware Detection\n");
    hardware_detection_result_t result = detect_hardware_capabilities();

    TEST(result.detection_success, "Hardware detection should succeed");
    TEST(result.info.cpu_cores >= 1, "Should detect at least 1 CPU core");
    TEST(result.info.memory_size_bytes >= 512, "Should detect minimum 512 bytes memory");
    TEST(result.info.device_class <= 5, "Device class should be valid (0-5)");

    // Test 2: Performance requirements (NFR-PFM-007)
    printf("\nTest 2: Performance Requirements\n");
    TEST(result.detection_time_ms <= 1000, "Detection time should be ≤1000ms (NFR-PFM-007)");
    TEST(result.confidence_level >= 50, "Confidence level should be reasonable");

    // Test 3: Device classification
    printf("\nTest 3: Device Classification\n");
    const char* device_names[] = {"Calculator", "Embedded", "Retro", "Desktop", "Server", "Supercomputer"};
    printf("  Detected device type: %s\n", device_names[result.info.device_class]);

    // Test 4: Hardware validation
    printf("\nTest 4: Hardware Validation\n");
    TEST(validate_hardware_detection(&result.info), "Hardware detection should validate successfully");

    // Test 5: Performance targets
    printf("\nTest 5: Performance Targets\n");
    TEST(result.info.boot_time_target_ms > 0, "Boot time target should be set");
    TEST(result.info.response_time_target_us > 0, "Response time target should be set");

    // Test 6: Global state
    printf("\nTest 6: Global State\n");
    TEST(g_hardware_detected == true, "Global hardware detected flag should be set");
    TEST(memcmp(&g_hardware_info, &result.info, sizeof(hardware_info_t)) == 0, "Global info should match result");

    // Display hardware information
    printf("\n📊 Detected Hardware Information:\n");
    printf("================================\n");
    printf("CPU: %d-bit, %d cores, %d MHz\n",
           result.info.cpu_bits, result.info.cpu_cores, result.info.cpu_speed_mhz);
    printf("Memory: %llu bytes (%s)\n",
           (unsigned long long)result.info.memory_size_bytes,
           result.info.has_mmu ? "with MMU" : "no MMU");
    printf("Device Class: %s\n", device_names[result.info.device_class]);
    printf("Performance Rating: %u\n", result.info.performance_rating);
    printf("Boot Target: %u ms\n", result.info.boot_time_target_ms);
    printf("Response Target: %u μs\n", result.info.response_time_target_us);
    printf("Detection Time: %u ms\n", result.detection_time_ms);
    printf("Confidence: %u%%\n", result.confidence_level);

    // Summary
    printf("\n📋 Test Summary:\n");
    printf("================\n");
    printf("✅ Passed: %d\n", tests_passed);
    printf("❌ Failed: %d\n", tests_failed);
    printf("Total: %d\n", tests_passed + tests_failed);

    if (tests_failed == 0) {
        printf("\n🎉 ALL TESTS PASSED! Hardware detection is ready for integration.\n");
        return 0;
    } else {
        printf("\n⚠️  Some tests failed. Review implementation before proceeding.\n");
        return 1;
    }
}