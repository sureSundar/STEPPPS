#ifndef TBOS_ROCK_WATCHDOG_H
#define TBOS_ROCK_WATCHDOG_H

#include <stdint.h>
#include <stdbool.h>

// Watchdog configuration structure
typedef struct {
    uint32_t timeout_ms;       // Timeout in milliseconds
    bool reset_on_timeout;     // Whether to reset the system on timeout
    bool debug_mode;           // If true, don't reset on timeout (for debugging)
    void (*callback)(void);    // Callback function when watchdog is about to reset
} watchdog_config_t;

// Initialize the watchdog timer
int watchdog_init(const watchdog_config_t *config);

// Start the watchdog timer
int watchdog_start(void);

// Stop the watchdog timer
int watchdog_stop(void);

// Feed the watchdog (reset the timer)
int watchdog_feed(void);

// Get remaining time until timeout (in milliseconds)
uint32_t watchdog_get_remaining_time(void);

// Check if watchdog is running
bool watchdog_is_running(void);

// Set a new timeout value (in milliseconds)
int watchdog_set_timeout(uint32_t timeout_ms);

// Enable/disable watchdog
int watchdog_enable(bool enable);

// Platform-specific initialization (implemented in platform code)
int watchdog_platform_init(const watchdog_config_t *config);

// Platform-specific feed implementation (implemented in platform code)
int watchdog_platform_feed(void);

// Platform-specific enable/disable (implemented in platform code)
int watchdog_platform_enable(bool enable);

// Platform-specific get remaining time (implemented in platform code)
uint32_t watchdog_platform_get_remaining_time(void);

// Default configuration
#define WATCHDOG_DEFAULT_CONFIG { \
    .timeout_ms = 5000,    \
    .reset_on_timeout = true, \
    .debug_mode = false,   \
    .callback = NULL       \
}

// Watchdog error codes
#define WATCHDOG_ERR_NONE          0
#define WATCHDOG_ERR_INIT_FAILED  -1
#define WATCHDOG_ERR_ALREADY_RUN  -2
#define WATCHDOG_ERR_NOT_RUNNING  -3
#define WATCHDOG_ERR_INVALID_ARG  -4

#endif // TBOS_ROCK_WATCHDOG_H
