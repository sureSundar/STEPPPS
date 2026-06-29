#ifndef TBOS_ROCK_ERROR_H
#define TBOS_ROCK_ERROR_H

/**
 * @file error.h
 * @brief TernaryBit OS - Production Error Handling System
 *
 * Comprehensive error handling with:
 * - Severity levels (DEBUG to PANIC)
 * - Domain categorization
 * - Watchdog integration
 * - Kernel panic support
 * - Error propagation macros
 * - Recovery mechanisms
 *
 * @version 2.0 (Production)
 * @date 2025-11-03
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* ERROR CODES                                                               */
/* ========================================================================= */

/* General error codes */
#define TBOS_SUCCESS            0
#define TBOS_ERROR             -1
#define TBOS_ENOMEM            -2   /* Out of memory */
#define TBOS_EINVAL            -3   /* Invalid argument */
#define TBOS_ENOENT            -4   /* No such entity */
#define TBOS_EBUSY             -5   /* Resource busy */
#define TBOS_ETIMEOUT          -6   /* Operation timed out */
#define TBOS_EIO               -7   /* I/O error */
#define TBOS_EPERM             -8   /* Permission denied */
#define TBOS_ENOSYS            -9   /* Function not implemented */
#define TBOS_EAGAIN            -10  /* Try again */
#define TBOS_EEXIST            -11  /* Already exists */
#define TBOS_EOVERFLOW         -12  /* Value overflow */
#define TBOS_ENOTREADY         -13  /* Not ready */
#define TBOS_ECORRUPT          -14  /* Data corruption */
#define TBOS_EKARMA            -15  /* Insufficient karma */

/* Domain-specific error codes */
#define TBOS_ERR_MEM_ALLOC_FAILED    0x1001
#define TBOS_ERR_MEM_DOUBLE_FREE     0x1002
#define TBOS_ERR_MEM_CORRUPTION      0x1003
#define TBOS_ERR_MEM_LEAK_DETECTED   0x1004
#define TBOS_ERR_MEM_OUT_OF_BOUNDS   0x1005

#define TBOS_ERR_FS_NOT_FOUND        0x2001
#define TBOS_ERR_FS_FULL             0x2002
#define TBOS_ERR_FS_CORRUPT          0x2003
#define TBOS_ERR_FS_ACCESS_DENIED    0x2004
#define TBOS_ERR_FS_READ_FAILED      0x2005
#define TBOS_ERR_FS_WRITE_FAILED     0x2006

#define TBOS_ERR_NET_TIMEOUT         0x3001
#define TBOS_ERR_NET_REFUSED         0x3002
#define TBOS_ERR_NET_UNREACHABLE     0x3003
#define TBOS_ERR_NET_DISCONNECTED    0x3004
#define TBOS_ERR_NET_RIGHT_SPEECH    0x3005

#define TBOS_ERR_HW_NOT_PRESENT      0x4001
#define TBOS_ERR_HW_MALFUNCTION      0x4002
#define TBOS_ERR_HW_UNSUPPORTED      0x4003

#define TBOS_ERR_SEC_AUTH_FAILED     0x5001
#define TBOS_ERR_SEC_VIOLATION       0x5002
#define TBOS_ERR_SEC_INTEGRITY       0x5003

#define TBOS_ERR_SYS_PANIC           0xDEAD0001
#define TBOS_ERR_SYS_WATCHDOG        0xDEAD0002
#define TBOS_ERR_SYS_ASSERT          0xDEADBEEF

/* ========================================================================= */
/* ERROR TYPES                                                               */
/* ========================================================================= */

/**
 * @brief Error severity levels
 */
typedef enum {
    ERROR_SEVERITY_DEBUG = 0,    /* Debug information only */
    ERROR_SEVERITY_INFO,         /* Informational message */
    ERROR_SEVERITY_WARNING,      /* Recoverable, non-critical issue */
    ERROR_SEVERITY_ERROR,        /* Non-fatal error, operation failed */
    ERROR_SEVERITY_CRITICAL,     /* Fatal error, module compromised */
    ERROR_SEVERITY_PANIC         /* System panic, halt required */
} error_severity_t;

/**
 * @brief Error domains for categorization
 */
typedef enum {
    ERROR_DOMAIN_GENERAL = 0,    /* General/uncategorized errors */
    ERROR_DOMAIN_MEMORY,         /* Memory management errors */
    ERROR_DOMAIN_DRIVER,         /* Device driver errors */
    ERROR_DOMAIN_FILESYSTEM,     /* Filesystem errors */
    ERROR_DOMAIN_NETWORK,        /* Network stack errors */
    ERROR_DOMAIN_SECURITY,       /* Security/access errors */
    ERROR_DOMAIN_HARDWARE,       /* Hardware/device errors */
    ERROR_DOMAIN_SYSTEM,         /* System service errors */
    ERROR_DOMAIN_PROCESS,        /* Process management errors */
    ERROR_DOMAIN_SCHEDULER,      /* Scheduler errors */
    ERROR_DOMAIN_IPC,            /* Inter-process communication */
    ERROR_DOMAIN_CONSCIOUSNESS   /* Karma/consciousness errors */
} error_domain_t;

/**
 * @brief Recovery action to take after error
 */
typedef enum {
    ERROR_RECOVERY_NONE = 0,     /* No recovery possible */
    ERROR_RECOVERY_RETRY,        /* Retry the operation */
    ERROR_RECOVERY_SKIP,         /* Skip and continue */
    ERROR_RECOVERY_FALLBACK,     /* Use fallback mechanism */
    ERROR_RECOVERY_RESET,        /* Reset the subsystem */
    ERROR_RECOVERY_RESTART,      /* Restart the service */
    ERROR_RECOVERY_PANIC         /* System panic required */
} error_recovery_t;

/**
 * @brief Watchdog action type
 */
typedef enum {
    WATCHDOG_ACTION_LOG = 0,     /* Log the event */
    WATCHDOG_ACTION_ALERT,       /* Generate alert */
    WATCHDOG_ACTION_RESTART,     /* Restart the module */
    WATCHDOG_ACTION_RESET,       /* Full system reset */
    WATCHDOG_ACTION_PANIC        /* Trigger panic */
} watchdog_action_t;

/**
 * @brief Error context structure
 */
typedef struct {
    uint32_t code;                  /* Error code */
    error_severity_t severity;      /* Error severity */
    error_domain_t domain;          /* Error domain */
    error_recovery_t recovery;      /* Suggested recovery */
    const char *message;            /* Human-readable message */
    const char *file;               /* Source file */
    const char *function;           /* Function name */
    int line;                       /* Line number */
    uint64_t timestamp;             /* Timestamp (ms since boot) */
    void *context;                  /* Additional context data */
    uint32_t context_size;          /* Size of context data */
} error_t;

/**
 * @brief Error statistics
 */
typedef struct {
    uint64_t total_errors;          /* Total errors reported */
    uint64_t errors_by_severity[6]; /* Count per severity */
    uint64_t errors_by_domain[12];  /* Count per domain */
    uint64_t recoveries_attempted;  /* Recovery attempts */
    uint64_t recoveries_succeeded;  /* Successful recoveries */
    uint64_t panics_triggered;      /* Total panics */
    uint64_t watchdog_triggers;     /* Watchdog activations */
    uint64_t last_error_time;       /* Last error timestamp */
    uint32_t last_error_code;       /* Last error code */
} error_stats_t;

/**
 * @brief Panic information for post-mortem
 */
typedef struct {
    uint32_t panic_code;            /* Panic error code */
    const char *panic_message;      /* Panic message */
    const char *file;               /* Source file */
    const char *function;           /* Function name */
    int line;                       /* Line number */
    uint64_t timestamp;             /* Panic timestamp */
    uint32_t cpu_id;                /* CPU that panicked */
    void *stack_trace[32];          /* Stack trace */
    uint32_t stack_depth;           /* Stack trace depth */
    uint32_t registers[16];         /* CPU registers at panic */
} panic_info_t;

/* ========================================================================= */
/* CALLBACK TYPES                                                            */
/* ========================================================================= */

/**
 * @brief Error handler callback type
 */
typedef void (*error_handler_t)(const error_t *error);

/**
 * @brief Watchdog callback type
 */
typedef void (*watchdog_callback_t)(uint32_t module_id, uint64_t timeout_ms);

/**
 * @brief Panic handler callback type
 */
typedef void (*panic_handler_t)(const panic_info_t *info);

/**
 * @brief Recovery callback type
 * @return true if recovery succeeded
 */
typedef bool (*recovery_handler_t)(const error_t *error);

/* ========================================================================= */
/* CORE API                                                                  */
/* ========================================================================= */

/**
 * @brief Initialize error handling system
 */
void error_init(void);

/**
 * @brief Shutdown error handling system
 */
void error_shutdown(void);

/**
 * @brief Report an error
 */
void error_report(uint32_t code, error_severity_t severity, error_domain_t domain,
                  const char *message, const char *file, int line);

/**
 * @brief Report an error with full context
 */
void error_report_full(const error_t *error);

/**
 * @brief Set custom error handler
 */
void error_set_handler(error_handler_t handler);

/**
 * @brief Set panic handler
 */
void error_set_panic_handler(panic_handler_t handler);

/**
 * @brief Get error string representation
 */
const char *error_to_string(const error_t *error);

/**
 * @brief Get error statistics
 */
void error_get_stats(error_stats_t *stats);

/**
 * @brief Reset error statistics
 */
void error_reset_stats(void);

/* ========================================================================= */
/* WATCHDOG API                                                              */
/* ========================================================================= */

/**
 * @brief Initialize watchdog system
 */
void watchdog_init(void);

/**
 * @brief Register a module with the watchdog
 * @param module_id Unique module identifier
 * @param timeout_ms Maximum time between heartbeats
 * @param action Action to take on timeout
 * @return 0 on success
 */
int watchdog_register(uint32_t module_id, uint64_t timeout_ms, watchdog_action_t action);

/**
 * @brief Unregister a module from watchdog
 */
int watchdog_unregister(uint32_t module_id);

/**
 * @brief Send heartbeat from a module
 */
void watchdog_heartbeat(uint32_t module_id);

/**
 * @brief Set watchdog callback
 */
void watchdog_set_callback(watchdog_callback_t callback);

/**
 * @brief Check all watchdogs (called periodically)
 */
void watchdog_check(void);

/**
 * @brief Trigger watchdog manually
 */
void watchdog_trigger(uint32_t module_id, const char *reason);

/* ========================================================================= */
/* PANIC API                                                                 */
/* ========================================================================= */

/**
 * @brief Trigger kernel panic
 * @param code Panic error code
 * @param message Panic message
 * @param file Source file
 * @param line Line number
 */
void kernel_panic(uint32_t code, const char *message, const char *file, int line);

/**
 * @brief Trigger kernel panic with full info
 */
void kernel_panic_full(const panic_info_t *info);

/**
 * @brief Get last panic info (for debugging after reset)
 */
const panic_info_t *panic_get_last(void);

/**
 * @brief Check if system is in panic state
 */
bool panic_is_active(void);

/* ========================================================================= */
/* RECOVERY API                                                              */
/* ========================================================================= */

/**
 * @brief Register recovery handler for a domain
 */
int recovery_register(error_domain_t domain, recovery_handler_t handler);

/**
 * @brief Attempt recovery for an error
 */
bool recovery_attempt(const error_t *error);

/**
 * @brief Set maximum retry count for recovery
 */
void recovery_set_max_retries(uint32_t count);

/* ========================================================================= */
/* ERROR REPORTING MACROS                                                    */
/* ========================================================================= */

#define ERROR_REPORT(code, severity, domain, msg) \
    error_report((code), (severity), (domain), (msg), __FILE__, __LINE__)

#define ERROR_DEBUG(domain, code, msg) \
    error_report((code), ERROR_SEVERITY_DEBUG, (domain), (msg), __FILE__, __LINE__)

#define ERROR_INFO(domain, code, msg) \
    error_report((code), ERROR_SEVERITY_INFO, (domain), (msg), __FILE__, __LINE__)

#define ERROR_WARNING(domain, code, msg) \
    error_report((code), ERROR_SEVERITY_WARNING, (domain), (msg), __FILE__, __LINE__)

#define ERROR_ERROR(domain, code, msg) \
    error_report((code), ERROR_SEVERITY_ERROR, (domain), (msg), __FILE__, __LINE__)

#define ERROR_CRITICAL(domain, code, msg) \
    error_report((code), ERROR_SEVERITY_CRITICAL, (domain), (msg), __FILE__, __LINE__)

#define ERROR_PANIC(code, msg) \
    kernel_panic((code), (msg), __FILE__, __LINE__)

/* ========================================================================= */
/* ERROR PROPAGATION MACROS                                                  */
/* ========================================================================= */

/**
 * @brief Check result and return on error
 */
#define TBOS_CHECK(expr) \
    do { \
        int __result = (expr); \
        if (__result != TBOS_SUCCESS) { \
            return __result; \
        } \
    } while(0)

/**
 * @brief Check result and goto cleanup label on error
 */
#define TBOS_CHECK_GOTO(expr, label) \
    do { \
        int __result = (expr); \
        if (__result != TBOS_SUCCESS) { \
            goto label; \
        } \
    } while(0)

/**
 * @brief Check pointer and return ENOMEM if NULL
 */
#define TBOS_CHECK_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            ERROR_ERROR(ERROR_DOMAIN_MEMORY, TBOS_ERR_MEM_ALLOC_FAILED, \
                       "Memory allocation failed"); \
            return TBOS_ENOMEM; \
        } \
    } while(0)

/**
 * @brief Check pointer and goto cleanup if NULL
 */
#define TBOS_CHECK_NULL_GOTO(ptr, label) \
    do { \
        if ((ptr) == NULL) { \
            ERROR_ERROR(ERROR_DOMAIN_MEMORY, TBOS_ERR_MEM_ALLOC_FAILED, \
                       "Memory allocation failed"); \
            goto label; \
        } \
    } while(0)

/**
 * @brief Check condition and return error if false
 */
#define TBOS_REQUIRE(cond, err, msg) \
    do { \
        if (!(cond)) { \
            ERROR_ERROR(ERROR_DOMAIN_GENERAL, (err), (msg)); \
            return (err); \
        } \
    } while(0)

/**
 * @brief Check condition and return EINVAL if false
 */
#define TBOS_REQUIRE_ARG(cond) \
    do { \
        if (!(cond)) { \
            ERROR_WARNING(ERROR_DOMAIN_GENERAL, TBOS_EINVAL, \
                         "Invalid argument: " #cond); \
            return TBOS_EINVAL; \
        } \
    } while(0)

/* ========================================================================= */
/* ASSERTION MACROS                                                          */
/* ========================================================================= */

/**
 * @brief Assert condition, trigger critical error if false
 */
#define TBOS_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            error_report(TBOS_ERR_SYS_ASSERT, ERROR_SEVERITY_CRITICAL, \
                        ERROR_DOMAIN_GENERAL, \
                        "Assertion failed: " #expr, __FILE__, __LINE__); \
        } \
    } while(0)

/**
 * @brief Assert condition, panic if false
 */
#define TBOS_ASSERT_PANIC(expr) \
    do { \
        if (!(expr)) { \
            kernel_panic(TBOS_ERR_SYS_ASSERT, \
                        "Fatal assertion failed: " #expr, \
                        __FILE__, __LINE__); \
        } \
    } while(0)

/**
 * @brief Debug assertion (only in debug builds)
 */
#ifdef DEBUG
#define TBOS_DEBUG_ASSERT(expr) TBOS_ASSERT(expr)
#else
#define TBOS_DEBUG_ASSERT(expr) ((void)0)
#endif

/* ========================================================================= */
/* UTILITY MACROS                                                            */
/* ========================================================================= */

/**
 * @brief Log and return error
 */
#define TBOS_RETURN_ERROR(domain, code, msg) \
    do { \
        ERROR_ERROR((domain), (code), (msg)); \
        return (code); \
    } while(0)

/**
 * @brief Safe cleanup with error propagation
 */
#define TBOS_CLEANUP_BEGIN int __cleanup_result = TBOS_SUCCESS;
#define TBOS_CLEANUP_SET(result) __cleanup_result = (result)
#define TBOS_CLEANUP_END return __cleanup_result

/**
 * @brief Mark unreachable code
 */
#define TBOS_UNREACHABLE() \
    kernel_panic(TBOS_ERR_SYS_PANIC, "Reached unreachable code", __FILE__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif /* TBOS_ROCK_ERROR_H */
