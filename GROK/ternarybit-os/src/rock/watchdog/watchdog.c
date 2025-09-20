#include "watchdog.h"
#include "../error/error.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>

// Static variables
static atomic_bool watchdog_initialized = false;
static atomic_bool watchdog_running = false;
static atomic_bool watchdog_enabled = true;
static watchdog_config_t current_config;

// Platform-specific implementations
#if defined(ARCH_X86_64)
#include "arch/x86_64/watchdog.h"
#elif defined(ARCH_ARM64)
#include "arch/arm64/watchdog.h"
#elif defined(ARCH_RISCV64)
#include "arch/riscv64/watchdog.h"
#else
#error "No watchdog implementation for this architecture"
#endif

int watchdog_init(const watchdog_config_t *config) {
    if (watchdog_initialized) {
        return WATCHDOG_ERR_ALREADY_RUN;
    }
    
    if (config == NULL) {
        // Use default configuration
        current_config = (watchdog_config_t)WATCHDOG_DEFAULT_CONFIG;
    } else {
        // Validate configuration
        if (config->timeout_ms == 0) {
            ERROR_REPORT(0x1001, ERROR_SEVERITY_ERROR, ERROR_DOMAIN_HARDWARE, 
                        "Invalid watchdog timeout value");
            return WATCHDOG_ERR_INVALID_ARG;
        }
        current_config = *config;
    }
    
    // Initialize platform-specific watchdog
    int result = watchdog_platform_init(&current_config);
    if (result != WATCHDOG_ERR_NONE) {
        ERROR_REPORT(0x1002, ERROR_SEVERITY_ERROR, ERROR_DOMAIN_HARDWARE,
                    "Failed to initialize watchdog hardware");
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    watchdog_initialized = true;
    return WATCHDOG_ERR_NONE;
}

int watchdog_start(void) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    if (watchdog_running) {
        return WATCHDOG_ERR_ALREADY_RUN;
    }
    
    // Enable the watchdog
    int result = watchdog_platform_enable(true);
    if (result != WATCHDOG_ERR_NONE) {
        ERROR_REPORT(0x1003, ERROR_SEVERITY_ERROR, ERROR_DOMAIN_HARDWARE,
                    "Failed to start watchdog");
        return result;
    }
    
    watchdog_running = true;
    watchdog_enabled = true;
    
    // Initial feed
    watchdog_feed();
    
    return WATCHDOG_ERR_NONE;
}

int watchdog_stop(void) {
    if (!watchdog_initialized || !watchdog_running) {
        return WATCHDOG_ERR_NOT_RUNNING;
    }
    
    int result = watchdog_platform_enable(false);
    if (result != WATCHDOG_ERR_NONE) {
        ERROR_REPORT(0x1004, ERROR_SEVERITY_WARNING, ERROR_DOMAIN_HARDWARE,
                    "Failed to stop watchdog");
        return result;
    }
    
    watchdog_running = false;
    return WATCHDOG_ERR_NONE;
}

int watchdog_feed(void) {
    if (!watchdog_initialized || !watchdog_running || !watchdog_enabled) {
        return WATCHDOG_ERR_NOT_RUNNING;
    }
    
    return watchdog_platform_feed();
}

uint32_t watchdog_get_remaining_time(void) {
    if (!watchdog_initialized || !watchdog_running) {
        return 0;
    }
    
    return watchdog_platform_get_remaining_time();
}

bool watchdog_is_running(void) {
    return watchdog_initialized && watchdog_running;
}

int watchdog_set_timeout(uint32_t timeout_ms) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    if (timeout_ms == 0) {
        return WATCHDOG_ERR_INVALID_ARG;
    }
    
    bool was_running = watchdog_running;
    
    // Stop the watchdog if it's running
    if (was_running) {
        watchdog_stop();
    }
    
    // Update the timeout
    current_config.timeout_ms = timeout_ms;
    
    // Reinitialize with new timeout
    int result = watchdog_platform_init(&current_config);
    if (result != WATCHDOG_ERR_NONE) {
        ERROR_REPORT(0x1005, ERROR_SEVERITY_ERROR, ERROR_DOMAIN_HARDWARE,
                    "Failed to set watchdog timeout");
        return result;
    }
    
    // Restart if it was running
    if (was_running) {
        return watchdog_start();
    }
    
    return WATCHDOG_ERR_NONE;
}

int watchdog_enable(bool enable) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    watchdog_enabled = enable;
    
    if (watchdog_running) {
        return watchdog_platform_enable(enable);
    }
    
    return WATCHDOG_ERR_NONE;
}
