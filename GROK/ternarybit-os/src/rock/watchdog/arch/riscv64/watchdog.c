#include "watchdog.h"
#include <stdint.h>
#include <stdbool.h>
#include "../../../core/tbos_hal.h"

// RISC-V CLINT (Core Local Interruptor) registers
#define CLINT_MSIP     0x0000  // Machine software interrupt pending
#define CLINT_MTIMECMP 0x4000  // Machine timer compare
#define CLINT_MTIME    0xBFF8  // Machine timer

// RISC-V PLIC (Platform-Level Interrupt Controller) registers
#define PLIC_PRIORITY  0x0000  // Interrupt priority
#define PLIC_PENDING   0x1000  // Pending interrupts
#define PLIC_ENABLE    0x2000  // Interrupt enable
#define PLIC_THRESHOLD 0x200000  // Priority threshold
#define PLIC_CLAIM     0x200004  // Claim/complete

// Memory-mapped I/O access
static volatile uint64_t *clint = (volatile uint64_t *)0x2000000;
static volatile uint32_t *plic = (volatile uint32_t *)0xC000000;

// Current watchdog configuration
static watchdog_config_t current_config;
static bool watchdog_initialized = false;

// Platform-specific initialization
int watchdog_platform_init(const watchdog_config_t *config) {
    if (config == NULL) {
        return WATCHDOG_ERR_INVALID_ARG;
    }
    
    // Save the configuration
    current_config = *config;
    
    // In a real implementation, we would:
    // 1. Configure the timer frequency
    // 2. Set up interrupts if needed
    
    // For now, just mark as initialized
    watchdog_initialized = true;
    
    // Set the timeout
    return watchdog_set_timeout(config->timeout_ms);
}

// Platform-specific feed implementation
int watchdog_platform_feed(void) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    // Get current time and add timeout
    uint64_t now = clint[CLINT_MTIME / 8];
    uint64_t timeout = (current_config.timeout_ms * 1000) / 100;  // Assuming 100KHz clock
    
    // Set the new compare value
    clint[CLINT_MTIMECMP / 8] = now + timeout;
    
    return WATCHDOG_ERR_NONE;
}

// Platform-specific enable/disable
int watchdog_platform_enable(bool enable) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    // In RISC-V, we don't explicitly enable/disable the timer
    // Instead, we set the compare register to a very large value to disable
    if (!enable) {
        clint[CLINT_MTIMECMP / 8] = UINT64_MAX;
    } else {
        // Re-arm the timer
        return watchdog_platform_feed();
    }
    
    return WATCHDOG_ERR_NONE;
}

// Platform-specific get remaining time
uint32_t watchdog_platform_get_remaining_time(void) {
    if (!watchdog_initialized) {
        return 0;
    }
    
    uint64_t now = clint[CLINT_MTIME / 8];
    uint64_t cmp = clint[CLINT_MTIMECMP / 8];
    
    if (cmp <= now) {
        return 0;  // Already expired
    }
    
    // Convert to milliseconds (assuming 100KHz clock)
    return (uint32_t)((cmp - now) * 1000 / 100000);
}

// Set a new timeout value (in milliseconds)
int watchdog_set_timeout(uint32_t timeout_ms) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    if (timeout_ms < 100 || timeout_ms > 60000) {
        return WATCHDOG_ERR_INVALID_ARG;
    }
    
    // Save the new timeout
    current_config.timeout_ms = timeout_ms;
    
    // Update the compare register
    return watchdog_platform_feed();
}

// Check if watchdog caused the last reset
bool watchdog_was_reset(void) {
    if (!watchdog_initialized) {
        return false;
    }
    
    // In RISC-V, we would check the reset reason register
    // For now, return false as we can't determine this easily
    return false;
}
