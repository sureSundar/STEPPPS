/**
 * @file tbos_hal.h
 * @brief TernaryBit OS - Hardware Abstraction Layer (HAL)
 *
 * US-1.2: Universal Hardware Abstraction
 * Epic 1: Calculator + Radio Universal Computer
 *
 * Provides unified interface across all hardware platforms:
 * - Calculator (2KB RAM) to Cloud (unlimited RAM)
 * - 8-bit MCU to 64-bit servers
 * - Serial, Radio, Network I/O
 * - Automatic capability detection
 *
 * Philosophy: "Same kernel, different capabilities"
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_HAL_H
#define TBOS_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* HARDWARE CAPABILITY FLAGS                                                 */
/* ========================================================================= */

typedef enum {
    /* Basic capabilities */
    HAL_CAP_COMPUTATION     = (1 << 0),  /* Can compute */
    HAL_CAP_STORAGE         = (1 << 1),  /* Has persistent storage */
    HAL_CAP_SERIAL          = (1 << 2),  /* Serial I/O */
    HAL_CAP_DISPLAY         = (1 << 3),  /* Visual display */

    /* Network capabilities */
    HAL_CAP_RADIO           = (1 << 4),  /* Radio communication */
    HAL_CAP_BLUETOOTH       = (1 << 5),  /* Bluetooth */
    HAL_CAP_WIFI            = (1 << 6),  /* WiFi */
    HAL_CAP_ETHERNET        = (1 << 7),  /* Wired network */

    /* Advanced capabilities */
    HAL_CAP_FLOAT           = (1 << 8),  /* Hardware floating point */
    HAL_CAP_MULTIPLY        = (1 << 9),  /* Hardware multiply */
    HAL_CAP_DIVIDE          = (1 << 10), /* Hardware divide */
    HAL_CAP_CRYPTO          = (1 << 11), /* Hardware crypto */

    /* Consciousness capabilities */
    HAL_CAP_RTC             = (1 << 12), /* Real-time clock */
    HAL_CAP_SENSORS         = (1 << 13), /* Environmental sensors */
    HAL_CAP_ACTUATORS       = (1 << 14), /* Can affect environment */
    HAL_CAP_POWER_MGMT      = (1 << 15)  /* Power management/fasting */
} hal_capability_t;

/* ========================================================================= */
/* HARDWARE PROFILES                                                         */
/* ========================================================================= */

typedef enum {
    HAL_PROFILE_CALCULATOR,   /* Minimal: 2KB RAM, computation only */
    HAL_PROFILE_RADIO,        /* Basic: 8KB RAM, + I/O */
    HAL_PROFILE_MINIMAL,      /* Standard: 16KB RAM, + shell */
    HAL_PROFILE_STANDARD,     /* Full: 64KB RAM, all features */
    HAL_PROFILE_ADVANCED,     /* Extended: 1MB+ RAM, extensions */
    HAL_PROFILE_CUSTOM        /* User-defined profile */
} hal_profile_t;

typedef struct {
    hal_profile_t profile;

    /* Memory resources */
    uint32_t ram_bytes;
    uint32_t rom_bytes;
    uint32_t storage_bytes;

    /* CPU */
    uint8_t cpu_bits;         /* 8, 16, 32, 64 */
    uint32_t cpu_freq_hz;
    uint8_t cpu_cores;

    /* Capabilities (bitmask of hal_capability_t) */
    uint32_t capabilities;

    /* I/O */
    uint32_t serial_baud;
    uint8_t radio_channels;

    /* Power */
    uint32_t power_budget_mw;
    bool supports_fasting;

    /* Identification */
    char platform_name[32];
    char vendor[32];
    uint32_t device_id;
} hal_hardware_info_t;

/* ========================================================================= */
/* HAL INITIALIZATION                                                        */
/* ========================================================================= */

/**
 * @brief Initialize Hardware Abstraction Layer
 * @return 0 on success, negative on error
 */
int hal_init(void);

/**
 * @brief Detect hardware capabilities automatically
 * @param info Output hardware information
 * @return 0 on success, negative on error
 */
int hal_detect_hardware(hal_hardware_info_t* info);

/**
 * @brief Get current hardware profile
 * @return Current hardware profile
 */
hal_profile_t hal_get_profile(void);

/**
 * @brief Check if capability is supported
 * @param cap Capability to check
 * @return true if supported, false otherwise
 */
bool hal_has_capability(hal_capability_t cap);

/* ========================================================================= */
/* MEMORY ABSTRACTION                                                        */
/* ========================================================================= */

/**
 * @brief Allocate memory (platform-specific)
 * @param size Size in bytes
 * @return Pointer to allocated memory, NULL on failure
 */
void* hal_malloc(size_t size);

/**
 * @brief Free allocated memory
 * @param ptr Pointer to free
 */
void hal_free(void* ptr);

/**
 * @brief Get available free memory
 * @return Free bytes available
 */
uint32_t hal_get_free_memory(void);

/* ========================================================================= */
/* I/O ABSTRACTION                                                           */
/* ========================================================================= */

/**
 * @brief Output single character
 * @param c Character to output
 * @return 0 on success, negative on error
 */
int hal_putc(char c);

/**
 * @brief Output string
 * @param str String to output
 * @return Number of characters written
 */
int hal_puts(const char* str);

/**
 * @brief Read single character (blocking)
 * @return Character read, or -1 on error
 */
int hal_getc(void);

/**
 * @brief Check if input is available
 * @return Number of bytes available to read
 */
int hal_input_available(void);

/* ========================================================================= */
/* TIME ABSTRACTION                                                          */
/* ========================================================================= */

/**
 * @brief Get system uptime in microseconds
 * @return Uptime in microseconds
 */
uint64_t hal_get_uptime_us(void);

/**
 * @brief Get system uptime in milliseconds
 * @return Uptime in milliseconds
 */
uint32_t hal_get_uptime_ms(void);

/**
 * @brief Delay for specified microseconds
 * @param us Microseconds to delay
 */
void hal_delay_us(uint32_t us);

/**
 * @brief Delay for specified milliseconds
 * @param ms Milliseconds to delay
 */
void hal_delay_ms(uint32_t ms);

/* ========================================================================= */
/* POWER MANAGEMENT ABSTRACTION                                              */
/* ========================================================================= */

typedef enum {
    HAL_POWER_FULL,           /* Full power */
    HAL_POWER_REDUCED,        /* Reduced power (75%) */
    HAL_POWER_LOW,            /* Low power (50%) */
    HAL_POWER_MINIMAL,        /* Minimal power (25%) */
    HAL_POWER_SLEEP,          /* Sleep mode (10%) */
    HAL_POWER_DEEP_SLEEP      /* Deep sleep (1%) */
} hal_power_mode_t;

/**
 * @brief Set power mode
 * @param mode Power mode to enter
 * @return 0 on success, negative on error
 */
int hal_set_power_mode(hal_power_mode_t mode);

/**
 * @brief Get current power consumption estimate
 * @return Power consumption in milliwatts
 */
uint32_t hal_get_power_consumption_mw(void);

/* ========================================================================= */
/* STORAGE ABSTRACTION                                                       */
/* ========================================================================= */

/**
 * @brief Read from persistent storage
 * @param offset Offset in bytes
 * @param buffer Buffer to read into
 * @param length Number of bytes to read
 * @return Number of bytes read, negative on error
 */
int hal_storage_read(uint32_t offset, void* buffer, size_t length);

/**
 * @brief Write to persistent storage
 * @param offset Offset in bytes
 * @param buffer Buffer to write from
 * @param length Number of bytes to write
 * @return Number of bytes written, negative on error
 */
int hal_storage_write(uint32_t offset, const void* buffer, size_t length);

/**
 * @brief Sync storage (flush buffers)
 * @return 0 on success, negative on error
 */
int hal_storage_sync(void);

/* ========================================================================= */
/* NETWORK ABSTRACTION (Radio/WiFi/Ethernet)                                */
/* ========================================================================= */

typedef enum {
    HAL_NET_RADIO,            /* Simple radio (RF24, etc.) */
    HAL_NET_BLUETOOTH,        /* Bluetooth */
    HAL_NET_WIFI,             /* WiFi */
    HAL_NET_ETHERNET          /* Wired Ethernet */
} hal_network_type_t;

/**
 * @brief Initialize network interface
 * @param type Network type to initialize
 * @return 0 on success, negative on error
 */
int hal_network_init(hal_network_type_t type);

/**
 * @brief Send data over network
 * @param type Network type
 * @param data Data to send
 * @param length Length of data
 * @return Number of bytes sent, negative on error
 */
int hal_network_send(hal_network_type_t type, const void* data, size_t length);

/**
 * @brief Receive data from network
 * @param type Network type
 * @param buffer Buffer to receive into
 * @param max_length Maximum bytes to receive
 * @return Number of bytes received, negative on error
 */
int hal_network_receive(hal_network_type_t type, void* buffer, size_t max_length);

/* ========================================================================= */
/* PLATFORM-SPECIFIC INITIALIZATION                                          */
/* ========================================================================= */

/**
 * @brief Platform-specific initialization (implemented per platform)
 * @return 0 on success, negative on error
 */
int hal_platform_init(void);

/**
 * @brief Platform-specific shutdown
 * @return 0 on success, negative on error
 */
int hal_platform_shutdown(void);

/* ========================================================================= */
/* ADAPTIVE MODULE LOADING                                                   */
/* ========================================================================= */

typedef struct {
    const char* module_name;
    hal_capability_t required_caps;
    uint32_t required_ram_kb;
    int (*init_func)(void);
} hal_module_descriptor_t;

/**
 * @brief Load module if hardware supports it
 * @param module Module descriptor
 * @return 0 if loaded, 1 if skipped, negative on error
 */
int hal_load_module_adaptive(const hal_module_descriptor_t* module);

/**
 * @brief Get list of loaded modules
 * @param modules Output array of module names
 * @param max_count Maximum number of modules to return
 * @return Number of modules loaded
 */
int hal_get_loaded_modules(const char** modules, uint32_t max_count);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_HAL_H */
