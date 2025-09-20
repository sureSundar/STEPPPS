#include "health.h"
#include "../error/error.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Static variables
static health_check_t *health_checks = NULL;
static health_status_t last_system_status = HEALTH_CHECK_OK;

// Initialize the health monitoring system
int health_init(void) {
    // Nothing to do here for now, but we keep this for future initialization
    return 0;
}

// Register a new health check
int health_register_check(health_check_t *check) {
    if (check == NULL || check->name == NULL || check->check == NULL) {
        ERROR_REPORT(0x2001, ERROR_SEVERITY_ERROR, ERROR_DOMAIN_SYSTEM,
                    "Invalid health check parameters");
        return -1;
    }
    
    // Check if already registered
    health_check_t *current = health_checks;
    while (current != NULL) {
        if (strcmp(current->name, check->name) == 0) {
            ERROR_REPORT(0x2002, ERROR_SEVERITY_WARNING, ERROR_DOMAIN_SYSTEM,
                        "Health check already registered");
            return -1;
        }
        if (current->next == NULL) {
            break;
        }
        current = current->next;
    }
    
    // Add to the end of the list
    if (health_checks == NULL) {
        health_checks = check;
    } else {
        current->next = check;
    }
    
    check->next = NULL;
    check->last_run = 0;
    
    return 0;
}

// Unregister a health check
int health_unregister_check(const char *name) {
    if (name == NULL) {
        return -1;
    }
    
    health_check_t *current = health_checks;
    health_check_t *prev = NULL;
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            if (prev == NULL) {
                health_checks = current->next;
            } else {
                prev->next = current->next;
            }
            return 0;
        }
        prev = current;
        current = current->next;
    }
    
    return -1;
}

// Run all health checks
health_check_result_t *health_run_checks(size_t *count) {
    // First, count the number of checks
    size_t num_checks = 0;
    health_check_t *current = health_checks;
    while (current != NULL) {
        num_checks++;
        current = current->next;
    }
    
    if (num_checks == 0) {
        if (count) *count = 0;
        return NULL;
    }
    
    // Allocate memory for results
    health_check_result_t *results = (health_check_result_t *)malloc(
        num_checks * sizeof(health_check_result_t));
    if (results == NULL) {
        ERROR_REPORT(0x2003, ERROR_SEVERITY_ERROR, ERROR_DOMAIN_MEMORY,
                    "Failed to allocate memory for health check results");
        if (count) *count = 0;
        return NULL;
    }
    
    // Run each check
    current = health_checks;
    size_t i = 0;
    health_status_t max_status = HEALTH_CHECK_OK;
    uint64_t now = (uint64_t)time(NULL);
    
    while (current != NULL && i < num_checks) {
        // Skip if not time to run yet
        if (current->interval_ms > 0 && 
            (now * 1000 - current->last_run) < current->interval_ms) {
            current = current->next;
            continue;
        }
        
        // Run the check
        health_check_result_t result = current->check(current->context);
        result.timestamp = now;
        
        // Update the worst status
        if (result.status > max_status) {
            max_status = result.status;
        }
        
        // Store the result
        results[i++] = result;
        current->last_run = now * 1000;  // Convert to milliseconds
        current = current->next;
    }
    
    // Update the system status
    last_system_status = max_status;
    
    if (count) *count = i;
    return results;
}

// Run a specific health check by name
health_check_result_t health_run_check(const char *name) {
    health_check_t *current = health_checks;
    uint64_t now = (uint64_t)time(NULL);
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            health_check_result_t result = current->check(current->context);
            result.timestamp = now;
            current->last_run = now * 1000;  // Convert to milliseconds
            return result;
        }
        current = current->next;
    }
    
    // Not found
    return (health_check_result_t){
        .status = HEALTH_CHECK_ERROR,
        .category = HEALTH_CATEGORY_SYSTEM,
        .component = "health",
        .message = "Check not found",
        .value = 0,
        .threshold = 0,
        .timestamp = now
    };
}

// Get overall system health status
health_status_t health_get_system_status(void) {
    return last_system_status;
}

// Example health checks (can be moved to separate files)

// CPU usage check
static health_check_result_t check_cpu_usage(void *context) {
    // In a real implementation, this would read actual CPU usage
    (void)context;  // Unused parameter
    
    // Simulated CPU usage (0-100%)
    int cpu_usage = 25;  // This would be read from the system
    int threshold = 90;  // Warning threshold
    
    if (cpu_usage >= threshold) {
        return HEALTH_RESULT(
            HEALTH_CHECK_WARNING,
            HEALTH_CATEGORY_CPU,
            "High CPU usage",
            cpu_usage,
            threshold
        );
    }
    
    return HEALTH_RESULT(
        HEALTH_CHECK_OK,
        HEALTH_CATEGORY_CPU,
        "CPU usage normal",
        cpu_usage,
        threshold
    );
}

// Memory usage check
static health_check_result_t check_memory_usage(void *context) {
    // In a real implementation, this would read actual memory usage
    (void)context;  // Unused parameter
    
    // Simulated memory usage (0-100%)
    int mem_usage = 65;  // This would be read from the system
    int threshold = 85;  // Warning threshold
    
    if (mem_usage >= threshold) {
        return HEALTH_RESULT(
            HEALTH_CHECK_WARNING,
            HEALTH_CATEGORY_MEMORY,
            "High memory usage",
            mem_usage,
            threshold
        );
    }
    
    return HEALTH_RESULT(
        HEALTH_CHECK_OK,
        HEALTH_CATEGORY_MEMORY,
        "Memory usage normal",
        mem_usage,
        threshold
    );
}

// Register example health checks
HEALTH_CHECK_DEFINE(cpu_check, HEALTH_CATEGORY_CPU, check_cpu_usage, NULL, 5000);
HEALTH_CHECK_DEFINE(mem_check, HEALTH_CATEGORY_MEMORY, check_memory_usage, NULL, 5000);
