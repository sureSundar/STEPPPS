/**
 * @file hardware_detector.c
 * @brief Universal Hardware Detection Implementation for TernaryBit OS
 *
 * Implements the universal hardware detection algorithms from PSC specifications
 * with complete traceability to requirements.
 *
 * @version 1.0
 * @date 2025-01-XX
 */

#include "hardware_detector.h"
#include <string.h>

// Global hardware information
hardware_info_t g_hardware_info;
bool g_hardware_detected = false;

// Performance thresholds for device classification
#define CALCULATOR_MAX_MEMORY    4096      // 4KB
#define EMBEDDED_MAX_MEMORY      65536     // 64KB
#define RETRO_MAX_MEMORY         1048576   // 1MB
#define DESKTOP_MIN_MEMORY       1048576   // 1MB
#define SERVER_MIN_MEMORY        8589934592ULL // 8GB

/**
 * @brief Main hardware detection function
 * Implements PSC DetectHardwareCapabilities() algorithm
 */
hardware_detection_result_t detect_hardware_capabilities(void) {
    hardware_detection_result_t result;
    uint64_t start_time, end_time;

    // Initialize result structure
    memset(&result, 0, sizeof(hardware_detection_result_t));
    start_time = platform_get_timestamp_us();

    // Step 1: Initialize hardware info structure
    memset(&result.info, 0, sizeof(hardware_info_t));
    result.confidence_level = 0;

    // Step 2: Detect CPU information (Critical for classification)
    if (!detect_cpu_info(&result.info)) {
        strcpy(result.error_message, "CPU detection failed");
        result.detection_success = false;
        return result;
    }
    result.confidence_level += 20;  // CPU detection successful

    // Step 3: Detect memory information (Critical for operation)
    if (!detect_memory_info(&result.info)) {
        strcpy(result.error_message, "Memory detection failed");
        result.detection_success = false;
        return result;
    }
    result.confidence_level += 25;  // Memory detection successful

    // Step 4: Detect storage information (Optional)
    if (detect_storage_info(&result.info)) {
        result.confidence_level += 15;  // Storage detected
    }

    // Step 5: Detect display information (Optional)
    if (detect_display_info(&result.info)) {
        result.confidence_level += 15;  // Display detected
    }

    // Step 6: Detect input information (Optional)
    if (detect_input_info(&result.info)) {
        result.confidence_level += 10;  // Input detected
    }

    // Step 7: Detect network information (Optional)
    if (detect_network_info(&result.info)) {
        result.confidence_level += 15;  // Network detected
    }

    // Step 8: Classify device type
    result.info.device_class = classify_device_type(&result.info);

    // Step 9: Calculate performance rating
    result.info.performance_rating = calculate_performance_rating(&result.info);

    // Step 10: Set performance targets based on device class
    switch (result.info.device_class) {
        case 0: // Calculator
            result.info.boot_time_target_ms = 100;
            result.info.response_time_target_us = 50000;  // 50ms
            break;
        case 1: // Embedded
            result.info.boot_time_target_ms = 500;
            result.info.response_time_target_us = 10000;  // 10ms
            break;
        case 2: // Retro
            result.info.boot_time_target_ms = 1000;
            result.info.response_time_target_us = 20000;  // 20ms
            break;
        case 3: // Desktop
            result.info.boot_time_target_ms = 2000;
            result.info.response_time_target_us = 1000;   // 1ms
            break;
        case 4: // Server
            result.info.boot_time_target_ms = 5000;
            result.info.response_time_target_us = 100;    // 0.1ms
            break;
        case 5: // Supercomputer
            result.info.boot_time_target_ms = 10000;
            result.info.response_time_target_us = 10;     // 0.01ms
            break;
    }

    // Step 11: Validate detection results
    if (!validate_hardware_detection(&result.info)) {
        strcpy(result.error_message, "Hardware validation failed");
        result.detection_success = false;
        return result;
    }

    // Step 12: Calculate detection time
    end_time = platform_get_timestamp_us();
    result.detection_time_ms = (uint32_t)((end_time - start_time) / 1000);

    // Step 13: Check performance target (NFR-PFM-007: <1s detection)
    if (result.detection_time_ms > 1000) {
        strcpy(result.error_message, "Detection time exceeded 1000ms target");
        // Don't fail completely, but note the performance issue
    }

    // Success!
    result.detection_success = true;
    g_hardware_info = result.info;
    g_hardware_detected = true;

    return result;
}

/**
 * @brief Detect CPU architecture and capabilities
 */
bool detect_cpu_info(hardware_info_t* info) {
    if (!info) return false;

    // Call platform-specific CPU detection
    if (!platform_detect_cpu(info)) {
        return false;
    }

    // Set capabilities based on CPU features
    if (info->cpu_bits >= 32) {
        info->capabilities |= HW_CAP_MMU;
    }

    if (info->cpu_cores > 1) {
        info->capabilities |= HW_CAP_MULTICORE;
    }

    if (info->cpu_cache_kb > 0) {
        info->capabilities |= HW_CAP_CACHE;
    }

    // Assume basic capabilities for all CPUs
    info->capabilities |= (HW_CAP_INTERRUPTS | HW_CAP_TIMER);

    return true;
}

/**
 * @brief Detect memory configuration
 */
bool detect_memory_info(hardware_info_t* info) {
    if (!info) return false;

    // Call platform-specific memory detection
    if (!platform_detect_memory(info)) {
        // Set minimum defaults if detection fails
        info->memory_size_bytes = 512;  // Minimum 512 bytes
        info->memory_type = MEM_TYPE_UNKNOWN;
        info->memory_speed_mhz = 1;
        info->has_mmu = false;
    }

    // Validate memory size is within acceptable range
    if (info->memory_size_bytes < 512) {
        return false;  // Below minimum requirement
    }

    return true;
}

/**
 * @brief Detect storage devices
 */
bool detect_storage_info(hardware_info_t* info) {
    if (!info) return false;

    // Call platform-specific storage detection
    if (platform_detect_storage(info)) {
        info->capabilities |= HW_CAP_STORAGE;
        return true;
    }

    // No storage is acceptable (RAM-only systems)
    info->storage_type = STORAGE_NONE;
    info->storage_size_bytes = 0;
    info->storage_speed_mbps = 0;

    return true;  // Not having storage is not a failure
}

/**
 * @brief Detect display capabilities
 */
bool detect_display_info(hardware_info_t* info) {
    if (!info) return false;

    // Call platform-specific display detection
    if (platform_detect_display(info)) {
        info->capabilities |= HW_CAP_DISPLAY;
        return true;
    }

    // No display is acceptable (headless systems)
    info->display_type = DISPLAY_NONE;
    info->display_width = 0;
    info->display_height = 0;
    info->display_colors = 0;

    return true;  // Not having display is not a failure
}

/**
 * @brief Detect input devices
 */
bool detect_input_info(hardware_info_t* info) {
    if (!info) return false;

    // Platform-specific input detection would go here
    // For now, assume basic input capability exists
    info->capabilities |= HW_CAP_INPUT;

    return true;
}

/**
 * @brief Detect network interfaces
 */
bool detect_network_info(hardware_info_t* info) {
    if (!info) return false;

    // Call platform-specific network detection
    if (platform_detect_network(info)) {
        info->capabilities |= HW_CAP_NETWORK;
        return true;
    }

    // No network is acceptable
    info->has_network = false;
    info->network_speed_mbps = 0;

    return true;  // Not having network is not a failure
}

/**
 * @brief Classify device type based on hardware
 */
uint8_t classify_device_type(const hardware_info_t* info) {
    if (!info) return 0;

    // Classification based on memory size and capabilities
    if (info->memory_size_bytes <= CALCULATOR_MAX_MEMORY) {
        return 0;  // Calculator
    } else if (info->memory_size_bytes <= EMBEDDED_MAX_MEMORY) {
        return 1;  // Embedded
    } else if (info->memory_size_bytes <= RETRO_MAX_MEMORY) {
        return 2;  // Retro
    } else if (info->memory_size_bytes < SERVER_MIN_MEMORY) {
        return 3;  // Desktop
    } else {
        // Distinguish between server and supercomputer based on cores and network
        if (info->cpu_cores >= 64 || (info->capabilities & HW_CAP_NETWORK && info->network_speed_mbps >= 10000)) {
            return 5;  // Supercomputer
        } else {
            return 4;  // Server
        }
    }
}

/**
 * @brief Calculate performance rating
 */
uint32_t calculate_performance_rating(const hardware_info_t* info) {
    if (!info) return 0;

    uint32_t rating = 0;

    // CPU contribution (40% of rating)
    rating += (info->cpu_speed_mhz / 10) * (info->cpu_cores);
    rating += (info->cpu_bits * 10);

    // Memory contribution (30% of rating)
    rating += (uint32_t)(info->memory_size_bytes / 1024);  // KB of memory
    if (info->has_mmu) rating += 1000;

    // Storage contribution (15% of rating)
    rating += (uint32_t)(info->storage_size_bytes / (1024 * 1024));  // MB of storage

    // I/O contribution (15% of rating)
    if (info->capabilities & HW_CAP_DISPLAY) rating += 500;
    if (info->capabilities & HW_CAP_NETWORK) rating += (info->network_speed_mbps / 10);

    return rating;
}

/**
 * @brief Validate hardware detection results
 */
bool validate_hardware_detection(const hardware_info_t* info) {
    if (!info) return false;

    // Validate CPU information
    if (info->cpu_bits < 4 || info->cpu_bits > 256) return false;
    if (info->cpu_cores == 0) return false;

    // Validate memory information
    if (info->memory_size_bytes < 512) return false;  // Minimum 512 bytes

    // Validate device class
    if (info->device_class > 5) return false;

    // Validate performance targets are reasonable
    if (info->boot_time_target_ms == 0 || info->boot_time_target_ms > 60000) return false;
    if (info->response_time_target_us == 0) return false;

    return true;
}

/**
 * @brief Print hardware information for debugging
 */
void print_hardware_info(const hardware_info_t* info) {
    if (!info) return;

    // This would normally print to debug output
    // Implementation depends on available output methods
    // For now, this is a placeholder
}

// Weak implementations of platform-specific functions
// These should be overridden by platform-specific implementations

__attribute__((weak)) bool platform_detect_cpu(hardware_info_t* info) {
    // Default implementation - detect generic 32-bit CPU
    info->cpu_bits = 32;
    info->instruction_set = ISA_RISC;
    info->cpu_speed_mhz = 100;  // Default 100MHz
    info->cpu_cores = 1;
    info->cpu_cache_kb = 0;
    return true;
}

__attribute__((weak)) bool platform_detect_memory(hardware_info_t* info) {
    // Default implementation - assume 1MB RAM
    info->memory_size_bytes = 1048576;  // 1MB
    info->memory_type = MEM_TYPE_DRAM;
    info->memory_speed_mhz = 100;
    info->has_mmu = (info->cpu_bits >= 32);
    return true;
}

__attribute__((weak)) bool platform_detect_storage(hardware_info_t* info) {
    // Default - no storage detected
    info->storage_type = STORAGE_NONE;
    info->storage_size_bytes = 0;
    info->storage_speed_mbps = 0;
    return false;
}

__attribute__((weak)) bool platform_detect_display(hardware_info_t* info) {
    // Default - basic text display
    info->display_type = DISPLAY_TEXT;
    info->display_width = 80;
    info->display_height = 24;
    info->display_colors = 2;  // Monochrome
    return true;
}

__attribute__((weak)) bool platform_detect_network(hardware_info_t* info) {
    // Default - no network
    info->has_network = false;
    info->network_speed_mbps = 0;
    return false;
}

__attribute__((weak)) uint32_t platform_get_timer_frequency(void) {
    return 1000000;  // Default 1MHz timer
}

__attribute__((weak)) uint64_t platform_get_timestamp_us(void) {
    // Default implementation returns 0 (no timing available)
    return 0;
}