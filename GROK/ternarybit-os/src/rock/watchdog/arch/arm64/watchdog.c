#include "watchdog.h"
#include <stdint.h>
#include <stdbool.h>
#include "../../../core/tbos_hal.h"

// ARM Generic Timer registers (memory-mapped)
#define ARM_TIMER_CTL    0x00   // Control register
#define ARM_TIMER_CNT    0x04   // Current count
#define ARM_TIMER_RLD    0x08   // Reload value
#define ARM_TIMER_STAT   0x0C   // Status register

// Control register bits
#define ARM_TIMER_EN     (1 << 0)    // Enable timer
#define ARM_TIMER_IMASK  (1 << 1)    // Interrupt mask
#define ARM_TIMER_INT    (1 << 2)    // Interrupt status
#define ARM_TIMER_RST    (1 << 3)    // Reset on timeout

// Memory-mapped I/O access
static volatile uint32_t *timer_regs = (volatile uint32_t *)0x200B000;

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
    // 1. Map the timer registers if not already mapped
    // 2. Configure the timer frequency
    // 3. Set up interrupts if needed
    
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
    
    // Reload the timer with the current timeout value
    uint32_t reload = (current_config.timeout_ms * 1000) / 100;  // Assuming 100KHz clock
    timer_regs[ARM_TIMER_RLD / 4] = reload;
    
    return WATCHDOG_ERR_NONE;
}

// Platform-specific enable/disable
int watchdog_platform_enable(bool enable) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    uint32_t ctl = timer_regs[ARM_TIMER_CTL / 4];
    
    if (enable) {
        // Enable watchdog and set reset on timeout if configured
        ctl |= ARM_TIMER_EN;
        if (current_config.reset_on_timeout) {
            ctl |= ARM_TIMER_RST;
        } else {
            ctl &= ~ARM_TIMER_RST;
        }
    } else {
        // Disable watchdog
        ctl &= ~(ARM_TIMER_EN | ARM_TIMER_RST);
    }
    
    timer_regs[ARM_TIMER_CTL / 4] = ctl;
    
    return WATCHDOG_ERR_NONE;
}

// Platform-specific get remaining time
uint32_t watchdog_platform_get_remaining_time(void) {
    if (!watchdog_initialized) {
        return 0;
    }
    
    // Read the current count
    uint32_t count = timer_regs[ARM_TIMER_CNT / 4];
    
    // Convert to milliseconds (assuming 100KHz clock)
    return (count * 10) / 1000;  // count * (1000/1000000) * 1000
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
    
    // Calculate the reload value (assuming 100KHz clock)
    uint32_t reload = (timeout_ms * 1000) / 10;  // Convert to microseconds
    
    // Write the reload value
    timer_regs[ARM_TIMER_RLD / 4] = reload;
    
    return WATCHDOG_ERR_NONE;
}

// Check if watchdog caused the last reset
bool watchdog_was_reset(void) {
    if (!watchdog_initialized) {
        return false;
    }
    
    // Read the status register
    uint32_t status = timer_regs[ARM_TIMER_STAT / 4];
    
    // Clear the interrupt status
    if (status & ARM_TIMER_INT) {
        timer_regs[ARM_TIMER_STAT / 4] = ARM_TIMER_INT;
        return true;
    }
    
    return false;
}
