#include "error.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// Default error handler
static error_handler_t current_handler = NULL;

// Default error handler implementation
static void default_error_handler(const error_t *error) {
    const char *severity_str;
    const char *domain_str;
    
    // Map severity to string
    switch (error->severity) {
        case ERROR_SEVERITY_DEBUG:    severity_str = "DEBUG"; break;
        case ERROR_SEVERITY_INFO:     severity_str = "INFO"; break;
        case ERROR_SEVERITY_WARNING:  severity_str = "WARNING"; break;
        case ERROR_SEVERITY_ERROR:    severity_str = "ERROR"; break;
        case ERROR_SEVERITY_CRITICAL: severity_str = "CRITICAL"; break;
        case ERROR_SEVERITY_PANIC:    severity_str = "PANIC"; break;
        default:                      severity_str = "UNKNOWN";
    }
    
    // Map domain to string
    switch (error->domain) {
        case ERROR_DOMAIN_GENERAL:    domain_str = "GENERAL"; break;
        case ERROR_DOMAIN_MEMORY:     domain_str = "MEMORY"; break;
        case ERROR_DOMAIN_DRIVER:     domain_str = "DRIVER"; break;
        case ERROR_DOMAIN_FILESYSTEM: domain_str = "FILESYSTEM"; break;
        case ERROR_DOMAIN_NETWORK:    domain_str = "NETWORK"; break;
        case ERROR_DOMAIN_SECURITY:   domain_str = "SECURITY"; break;
        case ERROR_DOMAIN_HARDWARE:   domain_str = "HARDWARE"; break;
        case ERROR_DOMAIN_SYSTEM:     domain_str = "SYSTEM"; break;
        default:                      domain_str = "UNKNOWN";
    }
    
    // Format: [TIMESTAMP] [SEVERITY] [DOMAIN] [CODE] [FILE:LINE] - MESSAGE
    printf("[%llu] [%s] [%s] [0x%08X] [%s:%d] - %s\n",
           (unsigned long long)error->timestamp,
           severity_str,
           domain_str,
           error->code,
           error->file,
           error->line,
           error->message);
    
    // If this is a critical error or panic, we might want to take additional actions
    if (error->severity >= ERROR_SEVERITY_CRITICAL) {
        // TODO: Trigger system watchdog or other recovery mechanisms
    }
}

void error_init(void) {
    // Set default error handler if none is set
    if (current_handler == NULL) {
        current_handler = default_error_handler;
    }
}

void error_report(uint32_t code, error_severity_t severity, error_domain_t domain,
                 const char *message, const char *file, int line) {
    error_t error = {
        .code = code,
        .severity = severity,
        .domain = domain,
        .message = message ? message : "No error message provided",
        .file = file ? file : "unknown",
        .line = line,
        .timestamp = (uint64_t)time(NULL)
    };
    
    // Call the current error handler
    if (current_handler != NULL) {
        current_handler(&error);
    }
    
    // If this is a panic, we should not continue execution
    if (severity == ERROR_SEVERITY_PANIC) {
        // TODO: Trigger kernel panic
        while (1) { /* Halt */ }
    }
}

void error_set_handler(error_handler_t handler) {
    current_handler = (handler != NULL) ? handler : default_error_handler;
}

const char *error_to_string(const error_t *error) {
    static char buffer[512];
    if (error == NULL) {
        return "NULL error";
    }
    
    snprintf(buffer, sizeof(buffer),
             "[%llu] [%s] [%s] [0x%08X] [%s:%d] - %s",
             (unsigned long long)error->timestamp,
             (error->severity == ERROR_SEVERITY_DEBUG) ? "DEBUG" :
             (error->severity == ERROR_SEVERITY_INFO) ? "INFO" :
             (error->severity == ERROR_SEVERITY_WARNING) ? "WARNING" :
             (error->severity == ERROR_SEVERITY_ERROR) ? "ERROR" :
             (error->severity == ERROR_SEVERITY_CRITICAL) ? "CRITICAL" : "UNKNOWN",
             (error->domain == ERROR_DOMAIN_GENERAL) ? "GENERAL" :
             (error->domain == ERROR_DOMAIN_MEMORY) ? "MEMORY" :
             (error->domain == ERROR_DOMAIN_DRIVER) ? "DRIVER" :
             (error->domain == ERROR_DOMAIN_FILESYSTEM) ? "FILESYSTEM" :
             (error->domain == ERROR_DOMAIN_NETWORK) ? "NETWORK" :
             (error->domain == ERROR_DOMAIN_SECURITY) ? "SECURITY" :
             (error->domain == ERROR_DOMAIN_HARDWARE) ? "HARDWARE" :
             (error->domain == ERROR_DOMAIN_SYSTEM) ? "SYSTEM" : "UNKNOWN",
             error->code,
             error->file,
             error->line,
             error->message);
             
    return buffer;
}
