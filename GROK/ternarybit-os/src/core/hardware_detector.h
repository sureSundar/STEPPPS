/**
 * @file hardware_detector.h
 * @brief Universal Hardware Detection System for TernaryBit OS
 *
 * Traceability:
 * - PRD: PR-019 (Universal Hardware Support)
 * - FRD: FR-UHS-001 (CPU Architecture Support), FR-UHS-002 (Memory Adaptation), FR-UHS-003 (Storage Support)
 * - NFRD: NFR-PFM-007 (Minimum System Requirements)
 * - HLD: Universal HAL Component
 * - LLD: hardware_info_t structure
 * - PSC: DetectHardwareCapabilities() algorithm
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef HARDWARE_DETECTOR_H
#define HARDWARE_DETECTOR_H

#include <stdint.h>
#include <stdbool.h>

// Hardware capability flags
#define HW_CAP_MMU           0x00000001  // Memory Management Unit
#define HW_CAP_FPU           0x00000002  // Floating Point Unit
#define HW_CAP_CACHE         0x00000004  // Cache memory
#define HW_CAP_MULTICORE     0x00000008  // Multiple CPU cores
#define HW_CAP_INTERRUPTS    0x00000010  // Interrupt support
#define HW_CAP_DMA           0x00000020  // Direct Memory Access
#define HW_CAP_TIMER         0x00000040  // Hardware timer
#define HW_CAP_RTC           0x00000080  // Real-Time Clock
#define HW_CAP_CRYPTO        0x00000100  // Hardware crypto acceleration
#define HW_CAP_NETWORK       0x00000200  // Network interface
#define HW_CAP_STORAGE       0x00000400  // Storage device
#define HW_CAP_DISPLAY       0x00000800  // Display capability
#define HW_CAP_INPUT         0x00001000  // Input device
#define HW_CAP_SOUND         0x00002000  // Audio capability

// CPU Architecture types
typedef enum {
    CPU_ARCH_UNKNOWN = 0,
    CPU_ARCH_4BIT    = 4,
    CPU_ARCH_8BIT    = 8,
    CPU_ARCH_16BIT   = 16,
    CPU_ARCH_32BIT   = 32,
    CPU_ARCH_64BIT   = 64,
    CPU_ARCH_128BIT  = 128,
    CPU_ARCH_QUANTUM = 256
} cpu_architecture_t;

// CPU Instruction Set types
typedef enum {
    ISA_UNKNOWN = 0,
    ISA_RISC,
    ISA_CISC,
    ISA_VLIW,
    ISA_QUANTUM,
    ISA_NEURAL,
    ISA_HYBRID
} instruction_set_t;

// Memory types
typedef enum {
    MEM_TYPE_UNKNOWN = 0,
    MEM_TYPE_SRAM,
    MEM_TYPE_DRAM,
    MEM_TYPE_NVRAM,
    MEM_TYPE_FLASH,
    MEM_TYPE_QUANTUM,
    MEM_TYPE_HYBRID
} memory_type_t;

// Storage types
typedef enum {
    STORAGE_NONE = 0,
    STORAGE_ROM,
    STORAGE_EEPROM,
    STORAGE_FLASH,
    STORAGE_MAGNETIC,
    STORAGE_OPTICAL,
    STORAGE_QUANTUM,
    STORAGE_BIOLOGICAL
} storage_type_t;

// Display types
typedef enum {
    DISPLAY_NONE = 0,
    DISPLAY_TEXT,
    DISPLAY_MONOCHROME,
    DISPLAY_COLOR,
    DISPLAY_3D,
    DISPLAY_HOLOGRAPHIC,
    DISPLAY_NEURAL
} display_type_t;

// Core hardware information structure
typedef struct {
    // CPU Information
    cpu_architecture_t cpu_bits;
    instruction_set_t instruction_set;
    uint32_t cpu_speed_mhz;
    uint8_t cpu_cores;
    uint32_t cpu_cache_kb;

    // Memory Information
    uint64_t memory_size_bytes;
    memory_type_t memory_type;
    uint32_t memory_speed_mhz;
    bool has_mmu;

    // Storage Information
    storage_type_t storage_type;
    uint64_t storage_size_bytes;
    uint32_t storage_speed_mbps;

    // I/O Information
    display_type_t display_type;
    uint16_t display_width;
    uint16_t display_height;
    uint8_t display_colors;

    // Network Information
    bool has_network;
    uint32_t network_speed_mbps;

    // Hardware Capabilities
    uint32_t capabilities;

    // Device Classification
    uint8_t device_class;  // 0=Calculator, 1=Embedded, 2=Retro, 3=Desktop, 4=Server, 5=Supercomputer

    // Performance Metrics
    uint32_t performance_rating;  // Relative performance score
    uint32_t power_consumption_mw;

    // Timing Information
    uint32_t boot_time_target_ms;
    uint32_t response_time_target_us;

} hardware_info_t;

// Hardware detection results
typedef struct {
    hardware_info_t info;
    bool detection_success;
    uint32_t detection_time_ms;
    char error_message[256];
    uint8_t confidence_level;  // 0-100% confidence in detection
} hardware_detection_result_t;

// Function prototypes
/**
 * @brief Main hardware detection function
 * @return hardware_detection_result_t Complete hardware detection results
 *
 * Implements PSC DetectHardwareCapabilities() algorithm
 * Target: <1000ms detection time (NFRD NFR-PFM-007)
 */
hardware_detection_result_t detect_hardware_capabilities(void);

/**
 * @brief Detect CPU architecture and capabilities
 * @param info Pointer to hardware_info_t to populate
 * @return bool Success/failure status
 */
bool detect_cpu_info(hardware_info_t* info);

/**
 * @brief Detect memory configuration
 * @param info Pointer to hardware_info_t to populate
 * @return bool Success/failure status
 */
bool detect_memory_info(hardware_info_t* info);

/**
 * @brief Detect storage devices
 * @param info Pointer to hardware_info_t to populate
 * @return bool Success/failure status
 */
bool detect_storage_info(hardware_info_t* info);

/**
 * @brief Detect display capabilities
 * @param info Pointer to hardware_info_t to populate
 * @return bool Success/failure status
 */
bool detect_display_info(hardware_info_t* info);

/**
 * @brief Detect input devices
 * @param info Pointer to hardware_info_t to populate
 * @return bool Success/failure status
 */
bool detect_input_info(hardware_info_t* info);

/**
 * @brief Detect network interfaces
 * @param info Pointer to hardware_info_t to populate
 * @return bool Success/failure status
 */
bool detect_network_info(hardware_info_t* info);

/**
 * @brief Classify device type based on hardware
 * @param info Pointer to hardware_info_t
 * @return uint8_t Device class (0-5)
 */
uint8_t classify_device_type(const hardware_info_t* info);

/**
 * @brief Calculate performance rating
 * @param info Pointer to hardware_info_t
 * @return uint32_t Performance rating score
 */
uint32_t calculate_performance_rating(const hardware_info_t* info);

/**
 * @brief Validate hardware detection results
 * @param info Pointer to hardware_info_t
 * @return bool Validation success/failure
 */
bool validate_hardware_detection(const hardware_info_t* info);

/**
 * @brief Print hardware information for debugging
 * @param info Pointer to hardware_info_t to display
 */
void print_hardware_info(const hardware_info_t* info);

// Hardware-specific detection functions (platform dependent)
extern bool platform_detect_cpu(hardware_info_t* info);
extern bool platform_detect_memory(hardware_info_t* info);
extern bool platform_detect_storage(hardware_info_t* info);
extern bool platform_detect_display(hardware_info_t* info);
extern bool platform_detect_network(hardware_info_t* info);
extern uint32_t platform_get_timer_frequency(void);
extern uint64_t platform_get_timestamp_us(void);

// Global hardware information (populated after detection)
extern hardware_info_t g_hardware_info;
extern bool g_hardware_detected;

#endif // HARDWARE_DETECTOR_H