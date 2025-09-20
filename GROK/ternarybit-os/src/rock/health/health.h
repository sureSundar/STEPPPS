#ifndef TBOS_ROCK_HEALTH_H
#define TBOS_ROCK_HEALTH_H

#include <stdint.h>
#include <stdbool.h>

// Health check result codes
typedef enum {
    HEALTH_CHECK_OK = 0,        // System is healthy
    HEALTH_CHECK_WARNING = 1,   // Warning condition
    HEALTH_CHECK_ERROR = 2,     // Error condition
    HEALTH_CHECK_CRITICAL = 3   // Critical error
} health_status_t;

// Health check categories
typedef enum {
    HEALTH_CATEGORY_CPU,
    HEALTH_CATEGORY_MEMORY,
    HEALTH_CATEGORY_STORAGE,
    HEALTH_CATEGORY_NETWORK,
    HEALTH_CATEGORY_TEMPERATURE,
    HEALTH_CATEGORY_POWER,
    HEALTH_CATEGORY_SYSTEM,
    HEALTH_CATEGORY_CUSTOM
} health_category_t;

// Health check result structure
typedef struct {
    health_status_t status;     // Overall status
    health_category_t category; // Category of the check
    const char *component;      // Component being checked
    const char *message;        // Status message
    int32_t value;             // Current value (if applicable)
    int32_t threshold;         // Threshold value (if applicable)
    uint64_t timestamp;        // Timestamp of the check
} health_check_result_t;

// Health check callback function type
typedef health_check_result_t (*health_check_func_t)(void *context);

// Health check registration structure
typedef struct health_check {
    const char *name;           // Name of the health check
    health_category_t category; // Category
    health_check_func_t check;  // Check function
    void *context;              // Context for the check function
    uint32_t interval_ms;       // How often to run (0 = manual)
    uint64_t last_run;          // Timestamp of last run
    struct health_check *next;  // Next check in the list
} health_check_t;

// Initialize the health monitoring system
int health_init(void);

// Register a new health check
int health_register_check(health_check_t *check);

// Unregister a health check
int health_unregister_check(const char *name);

// Run all health checks
health_check_result_t *health_run_checks(size_t *count);

// Run a specific health check by name
health_check_result_t health_run_check(const char *name);

// Get overall system health status
health_status_t health_get_system_status(void);

// Health check macros
#define HEALTH_CHECK_DEFINE(name, cat, func, ctx, intv) \
    static health_check_t name##_check = { \
        .name = #name, \
        .category = (cat), \
        .check = (func), \
        .context = (ctx), \
        .interval_ms = (intv), \
        .last_run = 0, \
        .next = NULL \
    }; \
    static void __attribute__((constructor)) name##_register(void) { \
        health_register_check(&name##_check); \
    }

// Helper macro for creating result structures
#define HEALTH_RESULT(s, c, m, v, t) \
    (health_check_result_t){ \
        .status = (s), \
        .category = (c), \
        .component = __func__, \
        .message = (m), \
        .value = (v), \
        .threshold = (t), \
        .timestamp = 0 /* Will be set by the health system */ \
    }

#endif // TBOS_ROCK_HEALTH_H
