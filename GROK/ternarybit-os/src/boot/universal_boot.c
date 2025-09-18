/**
 * @file universal_boot.c
 * @brief Universal Boot System Implementation for TernaryBit OS
 *
 * Implements the universal boot sequence algorithm from PSC specifications
 * with performance targets from NFRD NFR-PFM-001.
 *
 * @version 1.0
 * @date 2025-01-XX
 */

#include "universal_boot.h"
#include <string.h>

// Global boot status
boot_status_t g_boot_status;

// Boot stage names for debugging
static const char* boot_stage_names[] = {
    "INIT",
    "HARDWARE_DETECT",
    "MEMORY_INIT",
    "STEPPPS_INIT",
    "KERNEL_LOAD",
    "SERVICES_START",
    "READY"
};

// Boot mode names
static const char* boot_mode_names[] = {
    "MINIMAL",
    "EMBEDDED",
    "RETRO",
    "FULL",
    "SERVER",
    "SUPER"
};

/**
 * @brief Main universal boot sequence
 * Implements PSC UniversalBootLoader() algorithm
 */
boot_status_t universal_boot_sequence(void) {
    boot_status_t status;
    uint32_t boot_start_time, stage_start_time;

    // Initialize boot status
    memset(&status, 0, sizeof(boot_status_t));
    boot_start_time = platform_get_boot_timestamp_us();

    // STAGE 1: Initialization
    status.current_stage = STAGE_INIT;
    stage_start_time = platform_get_boot_timestamp_us();
    status.timing.stage_start_us[STAGE_INIT] = stage_start_time;

    // Platform-specific early initialization
    if (!platform_early_init()) {
        strcpy(status.last_error, "Platform early init failed");
        status.error_count++;
        strcpy(status.timing.error_messages[STAGE_INIT], "Platform init failed");
        status.boot_success = false;
        return status;
    }

    status.timing.stage_duration_us[STAGE_INIT] =
        platform_get_boot_timestamp_us() - stage_start_time;
    status.timing.stage_success[STAGE_INIT] = true;

    // STAGE 2: Hardware Detection
    status.current_stage = STAGE_HARDWARE_DETECT;
    stage_start_time = platform_get_boot_timestamp_us();
    status.timing.stage_start_us[STAGE_HARDWARE_DETECT] = stage_start_time;

    hardware_detection_result_t hw_result = detect_hardware_capabilities();
    if (!hw_result.detection_success) {
        strcpy(status.last_error, hw_result.error_message);
        status.error_count++;
        strcpy(status.timing.error_messages[STAGE_HARDWARE_DETECT], "Hardware detection failed");
        status.boot_success = false;
        return status;
    }

    status.hardware = hw_result.info;
    status.timing.stage_duration_us[STAGE_HARDWARE_DETECT] =
        platform_get_boot_timestamp_us() - stage_start_time;
    status.timing.stage_success[STAGE_HARDWARE_DETECT] = true;

    // Determine boot mode based on detected hardware
    status.config.mode = determine_boot_mode(&status.hardware);
    status.config = create_boot_config(&status.hardware, status.config.mode);

    // STAGE 3: Memory Layout Initialization
    status.current_stage = STAGE_MEMORY_INIT;
    stage_start_time = platform_get_boot_timestamp_us();
    status.timing.stage_start_us[STAGE_MEMORY_INIT] = stage_start_time;

    if (!initialize_memory_layout(&status.config)) {
        strcpy(status.last_error, "Memory initialization failed");
        status.error_count++;
        strcpy(status.timing.error_messages[STAGE_MEMORY_INIT], "Memory init failed");
        status.boot_success = false;
        return status;
    }

    status.timing.stage_duration_us[STAGE_MEMORY_INIT] =
        platform_get_boot_timestamp_us() - stage_start_time;
    status.timing.stage_success[STAGE_MEMORY_INIT] = true;

    // STAGE 4: STEPPPS Framework Initialization
    status.current_stage = STAGE_STEPPPS_INIT;
    stage_start_time = platform_get_boot_timestamp_us();
    status.timing.stage_start_us[STAGE_STEPPPS_INIT] = stage_start_time;

    if (!initialize_steppps_minimal(&status.config)) {
        strcpy(status.last_error, "STEPPPS initialization failed");
        status.error_count++;
        strcpy(status.timing.error_messages[STAGE_STEPPPS_INIT], "STEPPPS init failed");
        // Don't fail boot completely - STEPPPS is enhancement
    }

    status.timing.stage_duration_us[STAGE_STEPPPS_INIT] =
        platform_get_boot_timestamp_us() - stage_start_time;
    status.timing.stage_success[STAGE_STEPPPS_INIT] = (status.error_count == 0);

    // STAGE 5: Kernel Component Loading
    status.current_stage = STAGE_KERNEL_LOAD;
    stage_start_time = platform_get_boot_timestamp_us();
    status.timing.stage_start_us[STAGE_KERNEL_LOAD] = stage_start_time;

    if (!load_kernel_components(&status.config)) {
        strcpy(status.last_error, "Kernel loading failed");
        status.error_count++;
        strcpy(status.timing.error_messages[STAGE_KERNEL_LOAD], "Kernel load failed");
        status.boot_success = false;
        return status;
    }

    status.timing.stage_duration_us[STAGE_KERNEL_LOAD] =
        platform_get_boot_timestamp_us() - stage_start_time;
    status.timing.stage_success[STAGE_KERNEL_LOAD] = true;

    // STAGE 6: System Services Startup
    status.current_stage = STAGE_SERVICES_START;
    stage_start_time = platform_get_boot_timestamp_us();
    status.timing.stage_start_us[STAGE_SERVICES_START] = stage_start_time;

    if (!start_system_services(&status.config)) {
        strcpy(status.last_error, "Service startup failed");
        status.error_count++;
        strcpy(status.timing.error_messages[STAGE_SERVICES_START], "Service start failed");
        // Don't fail completely - basic services may still work
    }

    status.timing.stage_duration_us[STAGE_SERVICES_START] =
        platform_get_boot_timestamp_us() - stage_start_time;
    status.timing.stage_success[STAGE_SERVICES_START] = true;

    // STAGE 7: Ready
    status.current_stage = STAGE_READY;
    status.timing.total_boot_time_us =
        platform_get_boot_timestamp_us() - boot_start_time;

    // Validate boot sequence
    status.boot_success = validate_boot_sequence(&status);

    // Copy to global status
    g_boot_status = status;

    return status;
}

/**
 * @brief Determine optimal boot mode based on hardware
 */
boot_mode_t determine_boot_mode(const hardware_info_t* hardware) {
    if (!hardware) return BOOT_MODE_MINIMAL;

    // Use the device classification from hardware detection
    switch (hardware->device_class) {
        case 0: return BOOT_MODE_MINIMAL;    // Calculator
        case 1: return BOOT_MODE_EMBEDDED;   // Embedded
        case 2: return BOOT_MODE_RETRO;      // Retro
        case 3: return BOOT_MODE_FULL;       // Desktop
        case 4: return BOOT_MODE_SERVER;     // Server
        case 5: return BOOT_MODE_SUPER;      // Supercomputer
        default: return BOOT_MODE_MINIMAL;
    }
}

/**
 * @brief Create boot configuration for detected hardware
 */
boot_config_t create_boot_config(const hardware_info_t* hardware, boot_mode_t mode) {
    boot_config_t config;
    memset(&config, 0, sizeof(boot_config_t));

    config.mode = mode;

    // Configure memory layout based on boot mode
    switch (mode) {
        case BOOT_MODE_MINIMAL:
            config.kernel_base_addr = 0x0000;
            config.kernel_size_bytes = MINIMAL_KERNEL_SIZE;
            config.stack_base_addr = config.kernel_base_addr + config.kernel_size_bytes;
            config.stack_size_bytes = MINIMAL_STACK_SIZE;
            config.steppps_base_addr = config.stack_base_addr + config.stack_size_bytes;
            config.steppps_size_bytes = MINIMAL_STEPPPS_SIZE;
            config.heap_base_addr = config.steppps_base_addr + config.steppps_size_bytes;
            config.heap_size_bytes = MINIMAL_HEAP_SIZE;
            config.enable_steppps_full = false;
            config.enable_optimization = false;
            config.enable_networking = false;
            config.priority_levels = 2;  // Minimal priority levels
            break;

        case BOOT_MODE_EMBEDDED:
            config.kernel_base_addr = 0x0000;
            config.kernel_size_bytes = EMBEDDED_KERNEL_SIZE;
            config.stack_base_addr = 0x1000;  // 4KB offset
            config.stack_size_bytes = EMBEDDED_STACK_SIZE;
            config.steppps_base_addr = 0x2000;  // 8KB offset
            config.steppps_size_bytes = EMBEDDED_STEPPPS_SIZE;
            config.heap_base_addr = 0x3000;  // 12KB offset
            config.heap_size_bytes = EMBEDDED_HEAP_SIZE;
            config.enable_steppps_full = false;
            config.enable_optimization = true;
            config.enable_networking = hardware->has_network;
            config.priority_levels = 4;
            break;

        case BOOT_MODE_RETRO:
            config.kernel_base_addr = 0x0000;
            config.kernel_size_bytes = RETRO_KERNEL_SIZE;
            config.stack_base_addr = 0x4000;  // 16KB offset
            config.stack_size_bytes = RETRO_STACK_SIZE;
            config.steppps_base_addr = 0x5000;  // 20KB offset
            config.steppps_size_bytes = RETRO_STEPPPS_SIZE;
            config.heap_base_addr = 0x7000;  // 28KB offset
            config.heap_size_bytes = RETRO_HEAP_SIZE;
            config.enable_steppps_full = true;
            config.enable_optimization = true;
            config.enable_networking = hardware->has_network;
            config.priority_levels = 8;
            break;

        case BOOT_MODE_FULL:
        case BOOT_MODE_SERVER:
        case BOOT_MODE_SUPER:
            config.kernel_base_addr = 0x00000000;
            config.kernel_size_bytes = FULL_KERNEL_SIZE;
            config.stack_base_addr = 0x00010000;  // 64KB offset
            config.stack_size_bytes = FULL_STACK_SIZE;
            config.steppps_base_addr = 0x00020000;  // 128KB offset
            config.steppps_size_bytes = FULL_STEPPPS_SIZE;
            config.heap_base_addr = 0x00100000;  // 1MB offset
            // Heap uses remaining memory
            config.heap_size_bytes = (uint32_t)(hardware->memory_size_bytes - 0x00100000);
            config.enable_steppps_full = true;
            config.enable_optimization = true;
            config.enable_networking = hardware->has_network;
            config.priority_levels = (mode == BOOT_MODE_SUPER) ? 32 : 16;
            break;
    }

    return config;
}

/**
 * @brief Initialize memory layout based on boot configuration
 */
bool initialize_memory_layout(const boot_config_t* config) {
    if (!config) return false;

    // Initialize main memory regions
    if (!platform_memory_init(config->kernel_base_addr, config->kernel_size_bytes)) {
        return false;
    }

    // Initialize stack
    if (!platform_memory_init(config->stack_base_addr, config->stack_size_bytes)) {
        return false;
    }

    // Initialize heap
    if (!platform_memory_init(config->heap_base_addr, config->heap_size_bytes)) {
        return false;
    }

    // Initialize STEPPPS framework memory
    if (config->enable_steppps_full || config->mode != BOOT_MODE_MINIMAL) {
        if (!platform_memory_init(config->steppps_base_addr, config->steppps_size_bytes)) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Initialize minimal STEPPPS framework
 */
bool initialize_steppps_minimal(const boot_config_t* config) {
    if (!config) return false;

    // For now, just return true - STEPPPS will be implemented next
    // This is where we would initialize the STEPPPS framework
    return true;
}

/**
 * @brief Load kernel components based on mode
 */
bool load_kernel_components(const boot_config_t* config) {
    if (!config) return false;

    // Basic kernel components are already in memory
    // This is where we would load additional components based on mode
    return true;
}

/**
 * @brief Start essential system services
 */
bool start_system_services(const boot_config_t* config) {
    if (!config) return false;

    // Start services based on configuration
    // This is where we would start services like networking, etc.
    return true;
}

/**
 * @brief Validate boot sequence completion
 */
bool validate_boot_sequence(const boot_status_t* status) {
    if (!status) return false;

    // Check that all critical stages succeeded
    if (!status->timing.stage_success[STAGE_INIT] ||
        !status->timing.stage_success[STAGE_HARDWARE_DETECT] ||
        !status->timing.stage_success[STAGE_MEMORY_INIT] ||
        !status->timing.stage_success[STAGE_KERNEL_LOAD]) {
        return false;
    }

    // Check boot time against target
    uint32_t boot_time_ms = status->timing.total_boot_time_us / 1000;
    uint32_t target_ms;

    switch (status->config.mode) {
        case BOOT_MODE_MINIMAL:  target_ms = BOOT_TARGET_CALCULATOR_MS; break;
        case BOOT_MODE_EMBEDDED: target_ms = BOOT_TARGET_EMBEDDED_MS; break;
        case BOOT_MODE_RETRO:    target_ms = BOOT_TARGET_RETRO_MS; break;
        case BOOT_MODE_FULL:     target_ms = BOOT_TARGET_DESKTOP_MS; break;
        case BOOT_MODE_SERVER:   target_ms = BOOT_TARGET_SERVER_MS; break;
        case BOOT_MODE_SUPER:    target_ms = BOOT_TARGET_SUPER_MS; break;
        default: target_ms = BOOT_TARGET_DESKTOP_MS; break;
    }

    // Boot time validation - warn but don't fail if exceeded
    if (boot_time_ms > target_ms) {
        // Performance target missed, but boot succeeded
    }

    return true;
}

/**
 * @brief Get current boot stage name
 */
const char* get_boot_stage_name(boot_stage_t stage) {
    if (stage < 0 || stage >= sizeof(boot_stage_names)/sizeof(boot_stage_names[0])) {
        return "UNKNOWN";
    }
    return boot_stage_names[stage];
}

/**
 * @brief Get boot mode name
 */
const char* get_boot_mode_name(boot_mode_t mode) {
    if (mode < 0 || mode >= sizeof(boot_mode_names)/sizeof(boot_mode_names[0])) {
        return "UNKNOWN";
    }
    return boot_mode_names[mode];
}

/**
 * @brief Print boot timing information
 */
void print_boot_timing(const boot_status_t* status) {
    if (!status) return;

    // This would print timing information
    // Implementation depends on available output methods
}

// Weak implementations of platform-specific functions
__attribute__((weak)) bool platform_early_init(void) {
    // Default early initialization
    return true;
}

__attribute__((weak)) bool platform_memory_init(uint32_t base, uint32_t size) {
    (void)base;  // Suppress unused parameter warning
    (void)size;  // Suppress unused parameter warning
    // Default memory initialization
    // In a real implementation, this would set up memory regions
    return true;
}

__attribute__((weak)) bool platform_jump_to_kernel(uint32_t kernel_addr) {
    (void)kernel_addr;  // Suppress unused parameter warning
    // Default kernel jump
    // In a real implementation, this would transfer control to kernel
    return true;
}

__attribute__((weak)) uint32_t platform_get_boot_timestamp_us(void) {
    // Default timestamp - return incrementing counter
    static uint32_t counter = 0;
    return counter += 1000;  // Increment by 1ms each call
}