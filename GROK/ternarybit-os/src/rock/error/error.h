#ifndef TBOS_ROCK_ERROR_H
#define TBOS_ROCK_ERROR_H

#include <stdint.h>
#include <stdbool.h>

// Error severity levels
typedef enum {
    ERROR_SEVERITY_DEBUG,    // Debug information
    ERROR_SEVERITY_INFO,     // Informational
    ERROR_SEVERITY_WARNING,  // Recoverable error
    ERROR_SEVERITY_ERROR,    // Non-fatal error
    ERROR_SEVERITY_CRITICAL, // Fatal error
    ERROR_SEVERITY_PANIC     // System panic
} error_severity_t;

// Error domains
typedef enum {
    ERROR_DOMAIN_GENERAL,    // General errors
    ERROR_DOMAIN_MEMORY,     // Memory management
    ERROR_DOMAIN_DRIVER,     // Device drivers
    ERROR_DOMAIN_FILESYSTEM, // Filesystem
    ERROR_DOMAIN_NETWORK,    // Networking
    ERROR_DOMAIN_SECURITY,   // Security
    ERROR_DOMAIN_HARDWARE,   // Hardware
    ERROR_DOMAIN_SYSTEM      // System services
} error_domain_t;

// Error structure
typedef struct {
    uint32_t code;              // Error code
    error_severity_t severity;  // Error severity
    error_domain_t domain;      // Error domain
    const char *message;        // Human-readable message
    const char *file;           // Source file where error occurred
    int line;                   // Line number where error occurred
    uint64_t timestamp;         // Timestamp when error occurred
} error_t;

// Error callback function type
typedef void (*error_handler_t)(const error_t *error);

// Initialize error handling system
void error_init(void);

// Report an error
void error_report(uint32_t code, error_severity_t severity, error_domain_t domain,
                 const char *message, const char *file, int line);

// Set custom error handler
void error_set_handler(error_handler_t handler);

// Get error string
const char *error_to_string(const error_t *error);

// Error reporting macros
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

// Assertion macro
#define TBOS_ASSERT(expr) \
    do { \
        if (!(expr)) { \
            error_report(0xDEADBEEF, ERROR_SEVERITY_CRITICAL, ERROR_DOMAIN_GENERAL, \
                       "Assertion failed: " #expr, __FILE__, __LINE__); \
        } \
    } while(0)

#endif // TBOS_ROCK_ERROR_H
