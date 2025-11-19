/**
 * @file tbos_hal.c
 * @brief TernaryBit OS - Hardware Abstraction Layer Implementation
 *
 * US-1.2: Universal Hardware Abstraction
 *
 * @version 1.0
 * @date 2025-11-04
 */

#include "tbos_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static hal_hardware_info_t g_hardware_info;
static bool g_hal_initialized = false;
static const char* g_loaded_modules[32];
static uint32_t g_loaded_module_count = 0;

/* System start time for uptime calculation */
static uint64_t g_system_start_time_us = 0;

/* ========================================================================= */
/* HARDWARE DETECTION                                                        */
/* ========================================================================= */

int hal_detect_hardware(hal_hardware_info_t* info) {
    if (!info) return -1;

    memset(info, 0, sizeof(hal_hardware_info_t));

    /* Detect RAM size */
    #ifdef RAM_SIZE
        info->ram_bytes = RAM_SIZE;
    #else
        /* Try to detect dynamically or use sensible default */
        info->ram_bytes = 64 * 1024;  /* 64KB default */
    #endif

    /* Detect ROM/Flash size */
    #ifdef ROM_SIZE
        info->rom_bytes = ROM_SIZE;
    #else
        info->rom_bytes = 256 * 1024;  /* 256KB default */
    #endif

    /* Detect storage */
    #ifdef STORAGE_SIZE
        info->storage_bytes = STORAGE_SIZE;
    #else
        info->storage_bytes = 0;  /* No persistent storage by default */
    #endif

    /* Detect CPU architecture */
    #if defined(__AVR__)
        info->cpu_bits = 8;
        strncpy(info->platform_name, "AVR", sizeof(info->platform_name) - 1);
    #elif defined(__arm__)
        info->cpu_bits = 32;
        strncpy(info->platform_name, "ARM", sizeof(info->platform_name) - 1);
    #elif defined(__x86_64__)
        info->cpu_bits = 64;
        strncpy(info->platform_name, "x86-64", sizeof(info->platform_name) - 1);
    #elif defined(__i386__)
        info->cpu_bits = 32;
        strncpy(info->platform_name, "x86", sizeof(info->platform_name) - 1);
    #elif defined(__riscv)
        info->cpu_bits = 32;
        strncpy(info->platform_name, "RISC-V", sizeof(info->platform_name) - 1);
    #else
        info->cpu_bits = 8;
        strncpy(info->platform_name, "Generic", sizeof(info->platform_name) - 1);
    #endif

    /* Detect CPU frequency */
    #ifdef F_CPU
        info->cpu_freq_hz = F_CPU;
    #else
        info->cpu_freq_hz = 16000000;  /* 16MHz default */
    #endif

    info->cpu_cores = 1;  /* Single core default */

    /* Determine hardware profile based on RAM */
    if (info->ram_bytes < 4 * 1024) {
        info->profile = HAL_PROFILE_CALCULATOR;
    } else if (info->ram_bytes < 16 * 1024) {
        info->profile = HAL_PROFILE_RADIO;
    } else if (info->ram_bytes < 64 * 1024) {
        info->profile = HAL_PROFILE_MINIMAL;
    } else if (info->ram_bytes < 1024 * 1024) {
        info->profile = HAL_PROFILE_STANDARD;
    } else {
        info->profile = HAL_PROFILE_ADVANCED;
    }

    /* Detect capabilities based on profile */
    info->capabilities = HAL_CAP_COMPUTATION;  /* Always have this */

    if (info->profile >= HAL_PROFILE_CALCULATOR) {
        /* Even calculators can do basic I/O */
        info->capabilities |= HAL_CAP_SERIAL;
    }

    if (info->profile >= HAL_PROFILE_RADIO) {
        info->capabilities |= HAL_CAP_RADIO;
        info->capabilities |= HAL_CAP_STORAGE;
    }

    if (info->profile >= HAL_PROFILE_MINIMAL) {
        info->capabilities |= HAL_CAP_DISPLAY;
        info->capabilities |= HAL_CAP_RTC;
    }

    if (info->profile >= HAL_PROFILE_STANDARD) {
        info->capabilities |= HAL_CAP_FLOAT;
        info->capabilities |= HAL_CAP_MULTIPLY;
        info->capabilities |= HAL_CAP_DIVIDE;
        info->capabilities |= HAL_CAP_POWER_MGMT;
    }

    if (info->profile >= HAL_PROFILE_ADVANCED) {
        info->capabilities |= HAL_CAP_WIFI;
        info->capabilities |= HAL_CAP_ETHERNET;
        info->capabilities |= HAL_CAP_BLUETOOTH;
        info->capabilities |= HAL_CAP_CRYPTO;
        info->capabilities |= HAL_CAP_SENSORS;
        info->capabilities |= HAL_CAP_ACTUATORS;
    }

    /* Set default serial baud rate */
    info->serial_baud = 115200;

    /* Power budget based on profile */
    switch (info->profile) {
        case HAL_PROFILE_CALCULATOR:
            info->power_budget_mw = 100;  /* 100mW */
            break;
        case HAL_PROFILE_RADIO:
            info->power_budget_mw = 500;  /* 500mW */
            break;
        case HAL_PROFILE_MINIMAL:
            info->power_budget_mw = 1000;  /* 1W */
            break;
        case HAL_PROFILE_STANDARD:
            info->power_budget_mw = 5000;  /* 5W */
            break;
        case HAL_PROFILE_ADVANCED:
            info->power_budget_mw = 20000;  /* 20W */
            break;
        default:
            info->power_budget_mw = 1000;
            break;
    }

    info->supports_fasting = (info->profile >= HAL_PROFILE_STANDARD);

    return 0;
}

/* ========================================================================= */
/* HAL INITIALIZATION                                                        */
/* ========================================================================= */

int hal_init(void) {
    if (g_hal_initialized) {
        return 0;  /* Already initialized */
    }

    /* Detect hardware */
    int result = hal_detect_hardware(&g_hardware_info);
    if (result < 0) {
        return result;
    }

    /* Initialize platform-specific code */
    result = hal_platform_init();
    if (result < 0) {
        return result;
    }

    /* Initialize time tracking */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    g_system_start_time_us = (uint64_t)ts.tv_sec * 1000000ULL +
                             (uint64_t)ts.tv_nsec / 1000ULL;

    g_hal_initialized = true;

    return 0;
}

hal_profile_t hal_get_profile(void) {
    return g_hardware_info.profile;
}

bool hal_has_capability(hal_capability_t cap) {
    return (g_hardware_info.capabilities & cap) != 0;
}

/* ========================================================================= */
/* MEMORY ABSTRACTION                                                        */
/* ========================================================================= */

void* hal_malloc(size_t size) {
    return malloc(size);
}

void hal_free(void* ptr) {
    free(ptr);
}

uint32_t hal_get_free_memory(void) {
    /* Platform-specific implementation would go here */
    /* For simulation, return a reasonable value */
    return g_hardware_info.ram_bytes / 2;
}

/* ========================================================================= */
/* I/O ABSTRACTION                                                           */
/* ========================================================================= */

int hal_putc(char c) {
    return putchar(c);
}

int hal_puts(const char* str) {
    return puts(str);
}

int hal_getc(void) {
    return getchar();
}

int hal_input_available(void) {
    /* Platform-specific */
    return 0;
}

/* ========================================================================= */
/* TIME ABSTRACTION                                                          */
/* ========================================================================= */

uint64_t hal_get_uptime_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t current_us = (uint64_t)ts.tv_sec * 1000000ULL +
                          (uint64_t)ts.tv_nsec / 1000ULL;
    return current_us - g_system_start_time_us;
}

uint32_t hal_get_uptime_ms(void) {
    return (uint32_t)(hal_get_uptime_us() / 1000);
}

void hal_delay_us(uint32_t us) {
    struct timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

void hal_delay_ms(uint32_t ms) {
    hal_delay_us(ms * 1000);
}

/* ========================================================================= */
/* POWER MANAGEMENT                                                          */
/* ========================================================================= */

int hal_set_power_mode(hal_power_mode_t mode) {
    if (!g_hardware_info.supports_fasting) {
        return -1;  /* Not supported */
    }

    /* Platform-specific implementation */
    printf("[HAL] Power mode changed to: %d\n", mode);

    return 0;
}

uint32_t hal_get_power_consumption_mw(void) {
    /* Estimate based on current profile */
    return g_hardware_info.power_budget_mw;
}

/* ========================================================================= */
/* STORAGE ABSTRACTION                                                       */
/* ========================================================================= */

int hal_storage_read(uint32_t offset, void* buffer, size_t length) {
    if (!hal_has_capability(HAL_CAP_STORAGE)) {
        return -1;  /* No storage */
    }

    /* Platform-specific implementation */
    (void)offset;
    (void)buffer;
    (void)length;

    return 0;
}

int hal_storage_write(uint32_t offset, const void* buffer, size_t length) {
    if (!hal_has_capability(HAL_CAP_STORAGE)) {
        return -1;
    }

    /* Platform-specific implementation */
    (void)offset;
    (void)buffer;
    (void)length;

    return 0;
}

int hal_storage_sync(void) {
    if (!hal_has_capability(HAL_CAP_STORAGE)) {
        return -1;
    }

    /* Platform-specific implementation */
    return 0;
}

/* ========================================================================= */
/* NETWORK ABSTRACTION                                                       */
/* ========================================================================= */

int hal_network_init(hal_network_type_t type) {
    switch (type) {
        case HAL_NET_RADIO:
            if (!hal_has_capability(HAL_CAP_RADIO)) return -1;
            break;
        case HAL_NET_BLUETOOTH:
            if (!hal_has_capability(HAL_CAP_BLUETOOTH)) return -1;
            break;
        case HAL_NET_WIFI:
            if (!hal_has_capability(HAL_CAP_WIFI)) return -1;
            break;
        case HAL_NET_ETHERNET:
            if (!hal_has_capability(HAL_CAP_ETHERNET)) return -1;
            break;
    }

    /* Platform-specific initialization */
    printf("[HAL] Network initialized: type %d\n", type);

    return 0;
}

int hal_network_send(hal_network_type_t type, const void* data, size_t length) {
    (void)type;
    (void)data;
    (void)length;

    /* Platform-specific implementation */
    return length;
}

int hal_network_receive(hal_network_type_t type, void* buffer, size_t max_length) {
    (void)type;
    (void)buffer;
    (void)max_length;

    /* Platform-specific implementation */
    return 0;
}

/* ========================================================================= */
/* PLATFORM-SPECIFIC INITIALIZATION                                          */
/* ========================================================================= */

int hal_platform_init(void) {
    /* Platform-specific initialization goes here */
    /* For simulation, just return success */
    return 0;
}

int hal_platform_shutdown(void) {
    /* Platform-specific shutdown */
    return 0;
}

/* ========================================================================= */
/* ADAPTIVE MODULE LOADING                                                   */
/* ========================================================================= */

int hal_load_module_adaptive(const hal_module_descriptor_t* module) {
    if (!module) return -1;

    /* Check if required capabilities are met */
    if ((g_hardware_info.capabilities & module->required_caps) != module->required_caps) {
        printf("[HAL] Skipping module '%s': missing required capabilities\n",
               module->module_name);
        return 1;  /* Skipped */
    }

    /* Check if enough RAM */
    uint32_t required_ram_bytes = module->required_ram_kb * 1024;
    if (hal_get_free_memory() < required_ram_bytes) {
        printf("[HAL] Skipping module '%s': insufficient RAM (%u KB required)\n",
               module->module_name, module->required_ram_kb);
        return 1;  /* Skipped */
    }

    /* Initialize module */
    if (module->init_func) {
        int result = module->init_func();
        if (result < 0) {
            printf("[HAL] Module '%s' initialization failed\n",
                   module->module_name);
            return result;
        }
    }

    /* Add to loaded modules list */
    if (g_loaded_module_count < 32) {
        g_loaded_modules[g_loaded_module_count++] = module->module_name;
    }

    printf("[HAL] Module loaded: %s\n", module->module_name);

    return 0;  /* Loaded successfully */
}

int hal_get_loaded_modules(const char** modules, uint32_t max_count) {
    if (!modules) return -1;

    uint32_t count = (g_loaded_module_count < max_count) ?
                     g_loaded_module_count : max_count;

    for (uint32_t i = 0; i < count; i++) {
        modules[i] = g_loaded_modules[i];
    }

    return count;
}
