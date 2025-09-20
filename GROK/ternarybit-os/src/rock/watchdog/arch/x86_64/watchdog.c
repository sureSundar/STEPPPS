#include "watchdog.h"
#include <stdint.h>
#include <stdbool.h>
#include "../../../core/tbos_hal.h"

// i6300ESB watchdog registers
#define WDT_RLD    0x00    // Reload register
#define WDT_VAL    0x01    // Current value
#define WDT_CTRL   0x02    // Control register
#define WDT_TIMEOUT 0x03   // Timeout status

// Control register bits
#define WDT_CTRL_RST    (1 << 1)  // Reset on timeout
#define WDT_CTRL_EN     (1 << 0)  // Enable watchdog

// I/O port base (this is a placeholder - should be probed from ACPI/PCI)
#define WDT_BASE 0x1000

// Current watchdog configuration
static watchdog_config_t current_config;
static bool watchdog_initialized = false;

// I/O port operations
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// Platform-specific initialization
int watchdog_platform_init(const watchdog_config_t *config) {
    if (config == NULL) {
        return WATCHDOG_ERR_INVALID_ARG;
    }
    
    // Save the configuration
    current_config = *config;
    
    // Check if we can access the watchdog
    // In a real implementation, we would probe the hardware here
    
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
    
    // Write to the reload register
    outb(WDT_BASE + WDT_RLD, 1);
    
    return WATCHDOG_ERR_NONE;
}

// Platform-specific enable/disable
int watchdog_platform_enable(bool enable) {
    if (!watchdog_initialized) {
        return WATCHDOG_ERR_INIT_FAILED;
    }
    
    uint8_t ctrl = inb(WDT_BASE + WDT_CTRL);
    
    if (enable) {
        // Enable watchdog and set reset on timeout if configured
        ctrl |= WDT_CTRL_EN;
        if (current_config.reset_on_timeout) {
            ctrl |= WDT_CTRL_RST;
        } else {
            ctrl &= ~WDT_CTRL_RST;
        }
    } else {
        // Disable watchdog
        ctrl &= ~(WDT_CTRL_EN | WDT_CTRL_RST);
    }
    
    outb(WDT_BASE + WDT_CTRL, ctrl);
    
    return WATCHDOG_ERR_NONE;
}

// Platform-specific get remaining time
uint32_t watchdog_platform_get_remaining_time(void) {
    if (!watchdog_initialized) {
        return 0;
    }
    
    // Read the current value register
    uint8_t val = inb(WDT_BASE + WDT_VAL);
    
    // Convert to milliseconds (this is a simplification)
    // In a real implementation, we'd need to know the timer frequency
    return (uint32_t)val * 1000 / 100;  // Assuming 100Hz timer
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
    
    // Calculate the reload value
    // For a 100Hz timer, timeout_ms / 10 = number of ticks
    uint8_t reload = (timeout_ms + 9) / 10;  // Round up
    if (reload < 1) reload = 1;
    
    // Write the reload value
    outb(WDT_BASE + WDT_RLD, reload);
    
    return WATCHDOG_ERR_NONE;
}

// Check if watchdog caused the last reset
bool watchdog_was_reset(void) {
    if (!watchdog_initialized) {
        return false;
    }
    
    // Read the timeout status register
    uint8_t status = inb(WDT_BASE + WDT_TIMEOUT);
    
    // Clear the status
    outb(WDT_BASE + WDT_TIMEOUT, 0);
    
    return (status & 0x01) != 0;
}
