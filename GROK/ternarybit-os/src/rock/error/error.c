/**
 * @file error.c
 * @brief TernaryBit OS - Production Error Handling Implementation
 *
 * Complete implementation of error handling including:
 * - Error reporting with severity and domain
 * - Watchdog system for module health monitoring
 * - Kernel panic with post-mortem info
 * - Recovery mechanisms
 * - Statistics tracking
 *
 * @version 2.0 (Production)
 * @date 2025-11-03
 */

#include "error.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ========================================================================= */
/* CONFIGURATION                                                             */
/* ========================================================================= */

#define MAX_WATCHDOG_MODULES    64
#define MAX_RECOVERY_HANDLERS   16
#define ERROR_LOG_BUFFER_SIZE   4096
#define PANIC_STACK_DEPTH       32
#define DEFAULT_RECOVERY_RETRIES 3

/* ========================================================================= */
/* INTERNAL STRUCTURES                                                       */
/* ========================================================================= */

/**
 * @brief Watchdog entry for a module
 */
typedef struct {
    uint32_t module_id;
    uint64_t timeout_ms;
    uint64_t last_heartbeat;
    watchdog_action_t action;
    bool active;
    const char *module_name;
} watchdog_entry_t;

/**
 * @brief Recovery handler entry
 */
typedef struct {
    error_domain_t domain;
    recovery_handler_t handler;
    bool active;
} recovery_entry_t;

/* ========================================================================= */
/* INTERNAL STATE                                                            */
/* ========================================================================= */

/* Error handling state */
static bool g_error_initialized = false;
static error_handler_t g_error_handler = NULL;
static panic_handler_t g_panic_handler = NULL;
static error_stats_t g_error_stats = {0};

/* Watchdog state */
static bool g_watchdog_initialized = false;
static watchdog_entry_t g_watchdog_modules[MAX_WATCHDOG_MODULES];
static uint32_t g_watchdog_count = 0;
static watchdog_callback_t g_watchdog_callback = NULL;

/* Recovery state */
static recovery_entry_t g_recovery_handlers[MAX_RECOVERY_HANDLERS];
static uint32_t g_recovery_handler_count = 0;
static uint32_t g_max_recovery_retries = DEFAULT_RECOVERY_RETRIES;

/* Panic state */
static bool g_panic_active = false;
static panic_info_t g_last_panic = {0};

/* Error log ring buffer */
static char g_error_log[ERROR_LOG_BUFFER_SIZE];
static uint32_t g_error_log_pos = 0;

/* Boot time for timestamps */
static uint64_t g_boot_time_ms = 0;

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

/**
 * @brief Get current timestamp in milliseconds
 */
static uint64_t get_timestamp_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000) - g_boot_time_ms;
}

/**
 * @brief Get current CPU ID
 * Cross-platform implementation
 */
static uint32_t get_cpu_id(void) {
#if defined(__linux__)
    /* Linux: use sched_getcpu() */
    #ifndef HOST_BUILD
    extern int sched_getcpu(void);
    int cpu = sched_getcpu();
    return (cpu >= 0) ? (uint32_t)cpu : 0;
    #else
    return 0;  /* Hosted build - single CPU assumed */
    #endif
#elif defined(__APPLE__)
    /* macOS: no direct API, use pthread_self() hash as approximation */
    #include <pthread.h>
    return (uint32_t)((uintptr_t)pthread_self() % 16);
#elif defined(__x86_64__) || defined(__i386__)
    /* x86: use CPUID or rdtscp */
    #ifndef HOST_BUILD
    uint32_t cpu_id;
    __asm__ volatile ("mov $1, %%eax; cpuid; mov %%ebx, %0"
                      : "=r" (cpu_id) : : "eax", "ebx", "ecx", "edx");
    return (cpu_id >> 24) & 0xFF;
    #else
    return 0;
    #endif
#else
    return 0;  /* Unknown platform */
#endif
}

/**
 * @brief Get severity string
 */
static const char *severity_to_string(error_severity_t severity) {
    switch (severity) {
        case ERROR_SEVERITY_DEBUG:    return "DEBUG";
        case ERROR_SEVERITY_INFO:     return "INFO";
        case ERROR_SEVERITY_WARNING:  return "WARNING";
        case ERROR_SEVERITY_ERROR:    return "ERROR";
        case ERROR_SEVERITY_CRITICAL: return "CRITICAL";
        case ERROR_SEVERITY_PANIC:    return "PANIC";
        default:                      return "UNKNOWN";
    }
}

/**
 * @brief Get domain string
 */
static const char *domain_to_string(error_domain_t domain) {
    switch (domain) {
        case ERROR_DOMAIN_GENERAL:       return "GENERAL";
        case ERROR_DOMAIN_MEMORY:        return "MEMORY";
        case ERROR_DOMAIN_DRIVER:        return "DRIVER";
        case ERROR_DOMAIN_FILESYSTEM:    return "FILESYSTEM";
        case ERROR_DOMAIN_NETWORK:       return "NETWORK";
        case ERROR_DOMAIN_SECURITY:      return "SECURITY";
        case ERROR_DOMAIN_HARDWARE:      return "HARDWARE";
        case ERROR_DOMAIN_SYSTEM:        return "SYSTEM";
        case ERROR_DOMAIN_PROCESS:       return "PROCESS";
        case ERROR_DOMAIN_SCHEDULER:     return "SCHEDULER";
        case ERROR_DOMAIN_IPC:           return "IPC";
        case ERROR_DOMAIN_CONSCIOUSNESS: return "CONSCIOUSNESS";
        default:                         return "UNKNOWN";
    }
}

/**
 * @brief Append to error log ring buffer
 */
static void log_error(const char *msg) {
    size_t len = strlen(msg);
    if (len >= ERROR_LOG_BUFFER_SIZE) {
        len = ERROR_LOG_BUFFER_SIZE - 1;
    }

    /* Simple ring buffer append */
    for (size_t i = 0; i < len; i++) {
        g_error_log[g_error_log_pos] = msg[i];
        g_error_log_pos = (g_error_log_pos + 1) % ERROR_LOG_BUFFER_SIZE;
    }
    g_error_log[g_error_log_pos] = '\n';
    g_error_log_pos = (g_error_log_pos + 1) % ERROR_LOG_BUFFER_SIZE;
}

/* ========================================================================= */
/* DEFAULT HANDLERS                                                          */
/* ========================================================================= */

/**
 * @brief Default error handler - prints to console
 */
static void default_error_handler(const error_t *error) {
    if (!error) return;

    char buffer[512];
    snprintf(buffer, sizeof(buffer),
             "[%llu] [%s] [%s] [0x%08X] [%s:%d] - %s",
             (unsigned long long)error->timestamp,
             severity_to_string(error->severity),
             domain_to_string(error->domain),
             error->code,
             error->file ? error->file : "unknown",
             error->line,
             error->message ? error->message : "No message");

    /* Print to console */
    fprintf(stderr, "%s\n", buffer);

    /* Log to ring buffer */
    log_error(buffer);
}

/**
 * @brief Default panic handler
 */
static void default_panic_handler(const panic_info_t *info) {
    if (!info) return;

    fprintf(stderr, "\n");
    fprintf(stderr, "╔══════════════════════════════════════════════════════════════╗\n");
    fprintf(stderr, "║                    KERNEL PANIC                              ║\n");
    fprintf(stderr, "╠══════════════════════════════════════════════════════════════╣\n");
    fprintf(stderr, "║ Code:      0x%08X                                       ║\n", info->panic_code);
    fprintf(stderr, "║ Message:   %-50s ║\n", info->panic_message ? info->panic_message : "Unknown");
    fprintf(stderr, "║ Location:  %s:%d                                        \n",
            info->file ? info->file : "unknown", info->line);
    fprintf(stderr, "║ Function:  %-50s ║\n", info->function ? info->function : "unknown");
    fprintf(stderr, "║ Timestamp: %llu ms                                         \n",
            (unsigned long long)info->timestamp);
    fprintf(stderr, "╠══════════════════════════════════════════════════════════════╣\n");
    fprintf(stderr, "║ System halted. Please reboot.                                ║\n");
    fprintf(stderr, "╚══════════════════════════════════════════════════════════════╝\n\n");
}

/**
 * @brief Default watchdog callback
 */
static void default_watchdog_callback(uint32_t module_id, uint64_t timeout_ms) {
    fprintf(stderr, "[WATCHDOG] Module %u timed out after %llu ms\n",
            module_id, (unsigned long long)timeout_ms);
}

/* ========================================================================= */
/* CORE ERROR API                                                            */
/* ========================================================================= */

void error_init(void) {
    if (g_error_initialized) return;

    /* Initialize boot time */
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    g_boot_time_ms = (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);

    /* Set default handlers */
    g_error_handler = default_error_handler;
    g_panic_handler = default_panic_handler;

    /* Clear statistics */
    memset(&g_error_stats, 0, sizeof(g_error_stats));

    /* Clear log buffer */
    memset(g_error_log, 0, sizeof(g_error_log));
    g_error_log_pos = 0;

    g_error_initialized = true;

    printf("  [ERROR] Error handling system initialized\n");
}

void error_shutdown(void) {
    if (!g_error_initialized) return;

    /* Print final statistics */
    printf("\n  [ERROR] Final Error Statistics:\n");
    printf("          Total errors:     %llu\n", (unsigned long long)g_error_stats.total_errors);
    printf("          Panics:           %llu\n", (unsigned long long)g_error_stats.panics_triggered);
    printf("          Watchdog triggers: %llu\n", (unsigned long long)g_error_stats.watchdog_triggers);
    printf("          Recovery success:  %llu/%llu\n",
           (unsigned long long)g_error_stats.recoveries_succeeded,
           (unsigned long long)g_error_stats.recoveries_attempted);

    g_error_initialized = false;
}

void error_report(uint32_t code, error_severity_t severity, error_domain_t domain,
                  const char *message, const char *file, int line) {
    if (!g_error_initialized) {
        error_init();
    }

    error_t error = {
        .code = code,
        .severity = severity,
        .domain = domain,
        .recovery = ERROR_RECOVERY_NONE,
        .message = message ? message : "No error message provided",
        .file = file ? file : "unknown",
        .function = NULL,
        .line = line,
        .timestamp = get_timestamp_ms(),
        .context = NULL,
        .context_size = 0
    };

    error_report_full(&error);
}

void error_report_full(const error_t *error) {
    if (!error) return;

    if (!g_error_initialized) {
        error_init();
    }

    /* Update statistics */
    g_error_stats.total_errors++;
    if (error->severity < 6) {
        g_error_stats.errors_by_severity[error->severity]++;
    }
    if (error->domain < 12) {
        g_error_stats.errors_by_domain[error->domain]++;
    }
    g_error_stats.last_error_time = error->timestamp;
    g_error_stats.last_error_code = error->code;

    /* Call error handler */
    if (g_error_handler) {
        g_error_handler(error);
    }

    /* Handle critical errors - trigger watchdog */
    if (error->severity >= ERROR_SEVERITY_CRITICAL) {
        g_error_stats.watchdog_triggers++;

        /* Attempt recovery if available */
        if (error->severity == ERROR_SEVERITY_CRITICAL) {
            g_error_stats.recoveries_attempted++;
            if (recovery_attempt(error)) {
                g_error_stats.recoveries_succeeded++;
                return;  /* Recovery successful */
            }
        }
    }

    /* Handle panic */
    if (error->severity == ERROR_SEVERITY_PANIC) {
        kernel_panic(error->code, error->message, error->file, error->line);
    }
}

void error_set_handler(error_handler_t handler) {
    g_error_handler = handler ? handler : default_error_handler;
}

void error_set_panic_handler(panic_handler_t handler) {
    g_panic_handler = handler ? handler : default_panic_handler;
}

const char *error_to_string(const error_t *error) {
    static char buffer[512];

    if (!error) {
        return "NULL error";
    }

    snprintf(buffer, sizeof(buffer),
             "[%llu] [%s] [%s] [0x%08X] [%s:%d] - %s",
             (unsigned long long)error->timestamp,
             severity_to_string(error->severity),
             domain_to_string(error->domain),
             error->code,
             error->file ? error->file : "unknown",
             error->line,
             error->message ? error->message : "No message");

    return buffer;
}

void error_get_stats(error_stats_t *stats) {
    if (stats) {
        *stats = g_error_stats;
    }
}

void error_reset_stats(void) {
    memset(&g_error_stats, 0, sizeof(g_error_stats));
}

/* ========================================================================= */
/* WATCHDOG API                                                              */
/* ========================================================================= */

void watchdog_init(void) {
    if (g_watchdog_initialized) return;

    memset(g_watchdog_modules, 0, sizeof(g_watchdog_modules));
    g_watchdog_count = 0;
    g_watchdog_callback = default_watchdog_callback;
    g_watchdog_initialized = true;

    printf("  [WATCHDOG] Watchdog system initialized\n");
}

int watchdog_register(uint32_t module_id, uint64_t timeout_ms, watchdog_action_t action) {
    if (!g_watchdog_initialized) {
        watchdog_init();
    }

    if (g_watchdog_count >= MAX_WATCHDOG_MODULES) {
        return TBOS_ENOMEM;
    }

    /* Check for duplicate */
    for (uint32_t i = 0; i < g_watchdog_count; i++) {
        if (g_watchdog_modules[i].module_id == module_id) {
            return TBOS_EEXIST;
        }
    }

    /* Add new entry */
    watchdog_entry_t *entry = &g_watchdog_modules[g_watchdog_count++];
    entry->module_id = module_id;
    entry->timeout_ms = timeout_ms;
    entry->last_heartbeat = get_timestamp_ms();
    entry->action = action;
    entry->active = true;
    entry->module_name = NULL;

    printf("  [WATCHDOG] Registered module %u (timeout: %llu ms)\n",
           module_id, (unsigned long long)timeout_ms);

    return TBOS_SUCCESS;
}

int watchdog_unregister(uint32_t module_id) {
    for (uint32_t i = 0; i < g_watchdog_count; i++) {
        if (g_watchdog_modules[i].module_id == module_id) {
            g_watchdog_modules[i].active = false;
            printf("  [WATCHDOG] Unregistered module %u\n", module_id);
            return TBOS_SUCCESS;
        }
    }
    return TBOS_ENOENT;
}

void watchdog_heartbeat(uint32_t module_id) {
    for (uint32_t i = 0; i < g_watchdog_count; i++) {
        if (g_watchdog_modules[i].module_id == module_id &&
            g_watchdog_modules[i].active) {
            g_watchdog_modules[i].last_heartbeat = get_timestamp_ms();
            return;
        }
    }
}

void watchdog_set_callback(watchdog_callback_t callback) {
    g_watchdog_callback = callback ? callback : default_watchdog_callback;
}

void watchdog_check(void) {
    if (!g_watchdog_initialized) return;

    uint64_t now = get_timestamp_ms();

    for (uint32_t i = 0; i < g_watchdog_count; i++) {
        watchdog_entry_t *entry = &g_watchdog_modules[i];

        if (!entry->active) continue;

        uint64_t elapsed = now - entry->last_heartbeat;

        if (elapsed > entry->timeout_ms) {
            /* Timeout detected */
            g_error_stats.watchdog_triggers++;

            if (g_watchdog_callback) {
                g_watchdog_callback(entry->module_id, elapsed);
            }

            /* Take action based on configured action */
            switch (entry->action) {
                case WATCHDOG_ACTION_LOG:
                    ERROR_WARNING(ERROR_DOMAIN_SYSTEM, TBOS_ERR_SYS_WATCHDOG,
                                 "Watchdog timeout (log only)");
                    break;

                case WATCHDOG_ACTION_ALERT:
                    ERROR_ERROR(ERROR_DOMAIN_SYSTEM, TBOS_ERR_SYS_WATCHDOG,
                               "Watchdog timeout - alert");
                    break;

                case WATCHDOG_ACTION_RESTART:
                    ERROR_CRITICAL(ERROR_DOMAIN_SYSTEM, TBOS_ERR_SYS_WATCHDOG,
                                  "Watchdog timeout - restart required");
                    /* Module should handle restart */
                    break;

                case WATCHDOG_ACTION_RESET:
                    ERROR_CRITICAL(ERROR_DOMAIN_SYSTEM, TBOS_ERR_SYS_WATCHDOG,
                                  "Watchdog timeout - system reset");
                    /* Trigger system reset */
                    break;

                case WATCHDOG_ACTION_PANIC:
                    kernel_panic(TBOS_ERR_SYS_WATCHDOG,
                               "Watchdog timeout - panic triggered",
                               __FILE__, __LINE__);
                    break;
            }

            /* Reset heartbeat to avoid repeated triggers */
            entry->last_heartbeat = now;
        }
    }
}

void watchdog_trigger(uint32_t module_id, const char *reason) {
    g_error_stats.watchdog_triggers++;

    char msg[256];
    snprintf(msg, sizeof(msg), "Manual watchdog trigger for module %u: %s",
             module_id, reason ? reason : "no reason given");

    ERROR_WARNING(ERROR_DOMAIN_SYSTEM, TBOS_ERR_SYS_WATCHDOG, msg);

    if (g_watchdog_callback) {
        g_watchdog_callback(module_id, 0);
    }
}

/* ========================================================================= */
/* PANIC API                                                                 */
/* ========================================================================= */

void kernel_panic(uint32_t code, const char *message, const char *file, int line) {
    panic_info_t info = {
        .panic_code = code,
        .panic_message = message,
        .file = file,
        .function = NULL,
        .line = line,
        .timestamp = get_timestamp_ms(),
        .cpu_id = get_cpu_id(),
        .stack_depth = 0
    };

    /* Clear stack trace */
    memset(info.stack_trace, 0, sizeof(info.stack_trace));
    memset(info.registers, 0, sizeof(info.registers));

    kernel_panic_full(&info);
}

void kernel_panic_full(const panic_info_t *info) {
    if (!info) return;

    /* Prevent recursive panic */
    if (g_panic_active) {
        fprintf(stderr, "[PANIC] Recursive panic detected! Halting immediately.\n");
        while (1) { /* Hard halt */ }
    }

    g_panic_active = true;
    g_error_stats.panics_triggered++;

    /* Save panic info for post-mortem */
    g_last_panic = *info;

    /* Call panic handler */
    if (g_panic_handler) {
        g_panic_handler(info);
    }

    /* Halt the system */
    fprintf(stderr, "\n[PANIC] System halted.\n");

#ifdef HOST_BUILD
    /* In hosted build, exit cleanly for testing */
    exit(1);
#else
    /* In bare metal, halt CPU */
    while (1) {
        __asm__ volatile ("hlt");
    }
#endif
}

const panic_info_t *panic_get_last(void) {
    return &g_last_panic;
}

bool panic_is_active(void) {
    return g_panic_active;
}

/* ========================================================================= */
/* RECOVERY API                                                              */
/* ========================================================================= */

int recovery_register(error_domain_t domain, recovery_handler_t handler) {
    if (!handler) {
        return TBOS_EINVAL;
    }

    if (g_recovery_handler_count >= MAX_RECOVERY_HANDLERS) {
        return TBOS_ENOMEM;
    }

    recovery_entry_t *entry = &g_recovery_handlers[g_recovery_handler_count++];
    entry->domain = domain;
    entry->handler = handler;
    entry->active = true;

    printf("  [RECOVERY] Registered handler for domain %s\n",
           domain_to_string(domain));

    return TBOS_SUCCESS;
}

bool recovery_attempt(const error_t *error) {
    if (!error) return false;

    /* Find matching recovery handler */
    for (uint32_t i = 0; i < g_recovery_handler_count; i++) {
        recovery_entry_t *entry = &g_recovery_handlers[i];

        if (!entry->active) continue;

        if (entry->domain == error->domain ||
            entry->domain == ERROR_DOMAIN_GENERAL) {

            /* Try recovery with retries */
            for (uint32_t retry = 0; retry < g_max_recovery_retries; retry++) {
                printf("  [RECOVERY] Attempting recovery (attempt %u/%u)...\n",
                       retry + 1, g_max_recovery_retries);

                if (entry->handler(error)) {
                    printf("  [RECOVERY] Recovery successful!\n");
                    return true;
                }
            }
        }
    }

    printf("  [RECOVERY] Recovery failed after %u attempts\n", g_max_recovery_retries);
    return false;
}

void recovery_set_max_retries(uint32_t count) {
    g_max_recovery_retries = count > 0 ? count : DEFAULT_RECOVERY_RETRIES;
}
