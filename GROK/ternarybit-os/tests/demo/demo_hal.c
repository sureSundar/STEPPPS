/**
 * @file demo_hal.c
 * @brief Demonstration of Universal Hardware Abstraction Layer (US-1.2)
 *
 * Epic 1: Calculator + Radio Universal Computer
 * US-1.2: Universal Hardware Abstraction
 *
 * Shows:
 * - Automatic hardware detection
 * - Capability-based feature enabling
 * - Adaptive module loading
 * - Graceful degradation
 * - Same code, different platforms
 */

#include "../../src/hal/tbos_hal.h"
#include <stdio.h>
#include <string.h>

/* ========================================================================= */
/* EXAMPLE MODULES                                                           */
/* ========================================================================= */

int module_shell_init(void) {
    printf("    [Module] Shell initialized\n");
    return 0;
}

int module_network_init(void) {
    printf("    [Module] Network stack initialized\n");
    return 0;
}

int module_filesystem_init(void) {
    printf("    [Module] Filesystem initialized\n");
    return 0;
}

int module_crypto_init(void) {
    printf("    [Module] Cryptography module initialized\n");
    return 0;
}

int module_ml_init(void) {
    printf("    [Module] Machine learning engine initialized\n");
    return 0;
}

/* Module descriptors */
static const hal_module_descriptor_t g_modules[] = {
    {
        .module_name = "shell",
        .required_caps = HAL_CAP_COMPUTATION | HAL_CAP_SERIAL,
        .required_ram_kb = 4,
        .init_func = module_shell_init
    },
    {
        .module_name = "network",
        .required_caps = HAL_CAP_COMPUTATION | HAL_CAP_RADIO,
        .required_ram_kb = 8,
        .init_func = module_network_init
    },
    {
        .module_name = "filesystem",
        .required_caps = HAL_CAP_STORAGE,
        .required_ram_kb = 16,
        .init_func = module_filesystem_init
    },
    {
        .module_name = "crypto",
        .required_caps = HAL_CAP_CRYPTO,
        .required_ram_kb = 32,
        .init_func = module_crypto_init
    },
    {
        .module_name = "ml_engine",
        .required_caps = HAL_CAP_FLOAT | HAL_CAP_MULTIPLY,
        .required_ram_kb = 512,
        .init_func = module_ml_init
    }
};

#define MODULE_COUNT (sizeof(g_modules) / sizeof(g_modules[0]))

/* ========================================================================= */
/* DEMO FUNCTIONS                                                            */
/* ========================================================================= */

void print_hardware_info(const hal_hardware_info_t* info) {
    const char* profile_names[] = {
        "Calculator", "Radio Computer", "Minimal System",
        "Standard System", "Advanced System", "Custom"
    };

    printf("\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  Hardware Profile: %s\n", profile_names[info->profile]);
    printf("══════════════════════════════════════════════════\n\n");

    printf("Platform: %s\n", info->platform_name);
    printf("CPU: %u-bit, %u Hz, %u core(s)\n",
           info->cpu_bits, info->cpu_freq_hz, info->cpu_cores);
    printf("RAM: %u bytes (%u KB)\n",
           info->ram_bytes, info->ram_bytes / 1024);
    printf("ROM: %u bytes (%u KB)\n",
           info->rom_bytes, info->rom_bytes / 1024);
    printf("Storage: %u bytes\n", info->storage_bytes);
    printf("Power Budget: %u mW\n", info->power_budget_mw);
    printf("Fasting Support: %s\n\n",
           info->supports_fasting ? "Yes" : "No");

    printf("Capabilities:\n");
    if (info->capabilities & HAL_CAP_COMPUTATION)
        printf("  ✓ Computation\n");
    if (info->capabilities & HAL_CAP_STORAGE)
        printf("  ✓ Storage\n");
    if (info->capabilities & HAL_CAP_SERIAL)
        printf("  ✓ Serial I/O\n");
    if (info->capabilities & HAL_CAP_DISPLAY)
        printf("  ✓ Display\n");
    if (info->capabilities & HAL_CAP_RADIO)
        printf("  ✓ Radio\n");
    if (info->capabilities & HAL_CAP_BLUETOOTH)
        printf("  ✓ Bluetooth\n");
    if (info->capabilities & HAL_CAP_WIFI)
        printf("  ✓ WiFi\n");
    if (info->capabilities & HAL_CAP_ETHERNET)
        printf("  ✓ Ethernet\n");
    if (info->capabilities & HAL_CAP_FLOAT)
        printf("  ✓ Hardware FPU\n");
    if (info->capabilities & HAL_CAP_MULTIPLY)
        printf("  ✓ Hardware Multiply\n");
    if (info->capabilities & HAL_CAP_DIVIDE)
        printf("  ✓ Hardware Divide\n");
    if (info->capabilities & HAL_CAP_CRYPTO)
        printf("  ✓ Hardware Crypto\n");
    if (info->capabilities & HAL_CAP_RTC)
        printf("  ✓ Real-Time Clock\n");
    if (info->capabilities & HAL_CAP_SENSORS)
        printf("  ✓ Sensors\n");
    if (info->capabilities & HAL_CAP_ACTUATORS)
        printf("  ✓ Actuators\n");
    if (info->capabilities & HAL_CAP_POWER_MGMT)
        printf("  ✓ Power Management\n");

    printf("\n");
}

void demo_adaptive_loading(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  Adaptive Module Loading\n");
    printf("══════════════════════════════════════════════════\n\n");

    printf("Attempting to load %zu modules...\n\n", MODULE_COUNT);

    int loaded = 0;
    int skipped = 0;

    for (size_t i = 0; i < MODULE_COUNT; i++) {
        int result = hal_load_module_adaptive(&g_modules[i]);
        if (result == 0) {
            loaded++;
        } else if (result == 1) {
            skipped++;
        }
    }

    printf("\n");
    printf("Results:\n");
    printf("  Loaded: %d modules\n", loaded);
    printf("  Skipped: %d modules (insufficient capabilities/RAM)\n", skipped);
    printf("\n");

    /* List loaded modules */
    const char* loaded_modules[32];
    int count = hal_get_loaded_modules(loaded_modules, 32);

    printf("Loaded modules (%d):\n", count);
    for (int i = 0; i < count; i++) {
        printf("  • %s\n", loaded_modules[i]);
    }
}

void demo_capability_checks(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  Runtime Capability Checks\n");
    printf("══════════════════════════════════════════════════\n\n");

    printf("Checking capabilities at runtime...\n\n");

    /* Example: Network initialization */
    if (hal_has_capability(HAL_CAP_WIFI)) {
        printf("✓ WiFi available - initializing network stack\n");
        hal_network_init(HAL_NET_WIFI);
    } else if (hal_has_capability(HAL_CAP_RADIO)) {
        printf("✓ Radio available - initializing basic networking\n");
        hal_network_init(HAL_NET_RADIO);
    } else {
        printf("✗ No network capabilities - running offline\n");
    }

    /* Example: Crypto */
    if (hal_has_capability(HAL_CAP_CRYPTO)) {
        printf("✓ Hardware crypto - using AES acceleration\n");
    } else {
        printf("✗ No hardware crypto - using software implementation\n");
    }

    /* Example: Storage */
    if (hal_has_capability(HAL_CAP_STORAGE)) {
        printf("✓ Persistent storage - enabling file system\n");
    } else {
        printf("✗ No storage - running in RAM-only mode\n");
    }

    printf("\n");
}

void demo_time_functions(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  Time Abstraction\n");
    printf("══════════════════════════════════════════════════\n\n");

    printf("Testing time functions...\n\n");

    uint64_t uptime_us = hal_get_uptime_us();
    uint32_t uptime_ms = hal_get_uptime_ms();

    printf("Uptime: %llu μs (%u ms)\n",
           (unsigned long long)uptime_us, uptime_ms);

    printf("Delaying 100ms...\n");
    hal_delay_ms(100);

    uint64_t uptime_us2 = hal_get_uptime_us();
    printf("Uptime after delay: %llu μs\n", (unsigned long long)uptime_us2);
    printf("Actual delay: %llu μs\n",
           (unsigned long long)(uptime_us2 - uptime_us));

    printf("\n");
}

void demo_power_management(void) {
    printf("\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  Power Management\n");
    printf("══════════════════════════════════════════════════\n\n");

    if (!hal_has_capability(HAL_CAP_POWER_MGMT)) {
        printf("✗ Power management not supported on this hardware\n\n");
        return;
    }

    printf("✓ Power management supported\n\n");

    printf("Current power consumption: %u mW\n",
           hal_get_power_consumption_mw());

    printf("\nTesting power modes:\n");

    hal_set_power_mode(HAL_POWER_LOW);
    printf("  • Low power mode (50%%)\n");

    hal_set_power_mode(HAL_POWER_MINIMAL);
    printf("  • Minimal power mode (25%%)\n");

    hal_set_power_mode(HAL_POWER_FULL);
    printf("  • Back to full power\n");

    printf("\n");
}

/* ========================================================================= */
/* MAIN DEMO                                                                 */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Hardware Abstraction Layer Demo           ║\n");
    printf("║   US-1.2: Universal Hardware Abstraction                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");

    /* Initialize HAL */
    printf("\nInitializing Hardware Abstraction Layer...\n");
    int result = hal_init();
    if (result < 0) {
        printf("✗ HAL initialization failed!\n");
        return 1;
    }
    printf("✓ HAL initialized successfully\n");

    /* Get hardware info */
    hal_hardware_info_t hw_info;
    hal_detect_hardware(&hw_info);

    /* Show hardware information */
    print_hardware_info(&hw_info);

    /* Demo adaptive module loading */
    demo_adaptive_loading();

    /* Demo capability checks */
    demo_capability_checks();

    /* Demo time functions */
    demo_time_functions();

    /* Demo power management */
    demo_power_management();

    /* Summary */
    printf("\n");
    printf("══════════════════════════════════════════════════\n");
    printf("  Demo Summary\n");
    printf("══════════════════════════════════════════════════\n\n");

    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   US-1.2: Universal Hardware Abstraction - COMPLETE         ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ ✅ Automatic hardware detection                             ║\n");
    printf("║ ✅ 5 hardware profiles (Calculator → Advanced)              ║\n");
    printf("║ ✅ 16 capability flags                                      ║\n");
    printf("║ ✅ Adaptive module loading                                  ║\n");
    printf("║ ✅ Graceful degradation on limited hardware                 ║\n");
    printf("║ ✅ Unified time abstraction                                 ║\n");
    printf("║ ✅ Power management integration                             ║\n");
    printf("║ ✅ Network/Storage/I/O abstraction                          ║\n");
    printf("║                                                              ║\n");
    printf("║ Revolutionary Achievement:                                  ║\n");
    printf("║   Same code adapts from calculator to supercomputer!        ║\n");
    printf("║                                                              ║\n");
    printf("║ 🕉️  Universal Abstraction: Write Once, Run Anywhere 🕉️      ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("🕉️  TernaryBit OS: One Codebase for All Hardware! 🕉️\n\n");

    return 0;
}
