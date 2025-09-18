/**
 * @file universal_boot.h
 * @brief Universal Boot System for TernaryBit OS
 *
 * Traceability:
 * - PRD: PR-021 (Performance Requirements)
 * - FRD: FR-PFM-001 (Boot Performance)
 * - NFRD: NFR-PFM-001 (System Boot Time targets)
 * - HLD: Boot Architecture
 * - LLD: boot_sequence_t structure
 * - PSC: UniversalBootLoader() algorithm
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef UNIVERSAL_BOOT_H
#define UNIVERSAL_BOOT_H

#include "../core/hardware_detector.h"
#include <stdint.h>
#include <stdbool.h>

// Boot modes based on hardware capabilities
typedef enum {
    BOOT_MODE_MINIMAL = 0,    // Calculator mode (512B-4KB RAM)
    BOOT_MODE_EMBEDDED = 1,   // Embedded mode (4KB-64KB RAM)
    BOOT_MODE_RETRO = 2,      // Retro mode (64KB-1MB RAM)
    BOOT_MODE_FULL = 3,       // Desktop mode (>1MB RAM)
    BOOT_MODE_SERVER = 4,     // Server mode (>8GB RAM)
    BOOT_MODE_SUPER = 5       // Supercomputer mode
} boot_mode_t;

// Boot stages
typedef enum {
    STAGE_INIT = 0,
    STAGE_HARDWARE_DETECT = 1,
    STAGE_MEMORY_INIT = 2,
    STAGE_STEPPPS_INIT = 3,
    STAGE_KERNEL_LOAD = 4,
    STAGE_SERVICES_START = 5,
    STAGE_READY = 6
} boot_stage_t;

// Boot sequence timing structure
typedef struct {
    uint32_t stage_start_us[7];
    uint32_t stage_duration_us[7];
    uint32_t total_boot_time_us;
    bool stage_success[7];
    char error_messages[7][64];
} boot_timing_t;

// Boot configuration based on hardware
typedef struct {
    boot_mode_t mode;
    uint32_t kernel_base_addr;
    uint32_t kernel_size_bytes;
    uint32_t stack_base_addr;
    uint32_t stack_size_bytes;
    uint32_t heap_base_addr;
    uint32_t heap_size_bytes;
    uint32_t steppps_base_addr;
    uint32_t steppps_size_bytes;
    bool enable_steppps_full;
    bool enable_optimization;
    bool enable_networking;
    uint8_t priority_levels;
} boot_config_t;

// Boot status structure
typedef struct {
    boot_stage_t current_stage;
    boot_timing_t timing;
    boot_config_t config;
    hardware_info_t hardware;
    bool boot_success;
    uint8_t error_count;
    char last_error[128];
} boot_status_t;

// Function prototypes

/**
 * @brief Main universal boot sequence
 * @return boot_status_t Complete boot status and timing
 *
 * Implements PSC UniversalBootLoader() algorithm
 * Targets: NFR-PFM-001 boot time requirements
 */
boot_status_t universal_boot_sequence(void);

/**
 * @brief Determine optimal boot mode based on hardware
 * @param hardware Hardware detection results
 * @return boot_mode_t Optimal boot mode
 */
boot_mode_t determine_boot_mode(const hardware_info_t* hardware);

/**
 * @brief Create boot configuration for detected hardware
 * @param hardware Hardware information
 * @param mode Boot mode
 * @return boot_config_t Boot configuration
 */
boot_config_t create_boot_config(const hardware_info_t* hardware, boot_mode_t mode);

/**
 * @brief Initialize memory layout based on boot configuration
 * @param config Boot configuration
 * @return bool Success/failure
 */
bool initialize_memory_layout(const boot_config_t* config);

/**
 * @brief Initialize minimal STEPPPS framework
 * @param config Boot configuration
 * @return bool Success/failure
 */
bool initialize_steppps_minimal(const boot_config_t* config);

/**
 * @brief Load kernel components based on mode
 * @param config Boot configuration
 * @return bool Success/failure
 */
bool load_kernel_components(const boot_config_t* config);

/**
 * @brief Start essential system services
 * @param config Boot configuration
 * @return bool Success/failure
 */
bool start_system_services(const boot_config_t* config);

/**
 * @brief Validate boot sequence completion
 * @param status Boot status structure
 * @return bool Validation success
 */
bool validate_boot_sequence(const boot_status_t* status);

/**
 * @brief Print boot timing information
 * @param status Boot status structure
 */
void print_boot_timing(const boot_status_t* status);

/**
 * @brief Get current boot stage name
 * @param stage Boot stage
 * @return const char* Stage name string
 */
const char* get_boot_stage_name(boot_stage_t stage);

/**
 * @brief Get boot mode name
 * @param mode Boot mode
 * @return const char* Mode name string
 */
const char* get_boot_mode_name(boot_mode_t mode);

// Platform-specific boot functions (to be implemented per platform)
extern bool platform_early_init(void);
extern bool platform_memory_init(uint32_t base, uint32_t size);
extern bool platform_jump_to_kernel(uint32_t kernel_addr);
extern uint32_t platform_get_boot_timestamp_us(void);

// Memory layout constants for different modes
#define MINIMAL_KERNEL_SIZE     128   // bytes
#define MINIMAL_STACK_SIZE      64    // bytes
#define MINIMAL_HEAP_SIZE       256   // bytes
#define MINIMAL_STEPPPS_SIZE    64    // bytes

#define EMBEDDED_KERNEL_SIZE    4096   // 4KB
#define EMBEDDED_STACK_SIZE     1024   // 1KB
#define EMBEDDED_HEAP_SIZE      8192   // 8KB
#define EMBEDDED_STEPPPS_SIZE   2048   // 2KB

#define RETRO_KERNEL_SIZE      16384   // 16KB
#define RETRO_STACK_SIZE       4096    // 4KB
#define RETRO_HEAP_SIZE        32768   // 32KB
#define RETRO_STEPPPS_SIZE     8192    // 8KB

#define FULL_KERNEL_SIZE       65536   // 64KB
#define FULL_STACK_SIZE        16384   // 16KB
#define FULL_STEPPPS_SIZE      65536   // 64KB

// Boot timing targets (from NFRD NFR-PFM-001)
#define BOOT_TARGET_CALCULATOR_MS    100
#define BOOT_TARGET_EMBEDDED_MS      500
#define BOOT_TARGET_RETRO_MS         1000
#define BOOT_TARGET_DESKTOP_MS       2000
#define BOOT_TARGET_SERVER_MS        5000
#define BOOT_TARGET_SUPER_MS         10000

// Global boot status
extern boot_status_t g_boot_status;

#endif // UNIVERSAL_BOOT_H