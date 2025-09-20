#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "../error/error.h"
#include "../panic/panic.h"
#include "../watchdog/watchdog.h"
#include "../health/health.h"

// Global variables
static volatile sig_atomic_t running = 1;
static watchdog_config_t wd_config = WATCHDOG_DEFAULT_CONFIG;
static pthread_mutex_t health_mutex = PTHREAD_MUTEX_INITIALIZER;

// Signal handler for graceful shutdown
static void handle_signal(int sig) {
    (void)sig;  // Unused parameter
    running = 0;
}

// Initialize signal handlers
static int init_signals(void) {
    struct sigaction sa = {
        .sa_handler = handle_signal,
        .sa_flags = 0
    };
    
    sigemptyset(&sa.sa_mask);
    
    if (sigaction(SIGINT, &sa, NULL) != 0 ||
        sigaction(SIGTERM, &sa, NULL) != 0) {
        ERROR_CRITICAL(ERROR_DOMAIN_SYSTEM, 0x3001, "Failed to set up signal handlers");
        return -1;
    }
    
    // Ignore SIGPIPE to handle broken pipes gracefully
    signal(SIGPIPE, SIG_IGN);
    
    return 0;
}

// Initialize the watchdog
static int init_watchdog(void) {
    wd_config.timeout_ms = 10000;  // 10 second timeout
    wd_config.reset_on_timeout = true;
    wd_config.debug_mode = false;
    wd_config.callback = NULL;
    
    if (watchdog_init(&wd_config) != 0) {
        ERROR_CRITICAL(ERROR_DOMAIN_HARDWARE, 0x3002, "Failed to initialize watchdog");
        return -1;
    }
    
    if (watchdog_start() != 0) {
        ERROR_CRITICAL(ERROR_DOMAIN_HARDWARE, 0x3003, "Failed to start watchdog");
        return -1;
    }
    
    return 0;
}

// Health monitoring thread
static void* health_monitor_thread(void *arg) {
    (void)arg;  // Unused parameter
    
    while (running) {
        pthread_mutex_lock(&health_mutex);
        
        // Run health checks
        size_t num_results = 0;
        health_check_result_t *results = health_run_checks(&num_results);
        
        // Process results
        for (size_t i = 0; i < num_results; i++) {
            health_check_result_t *r = &results[i];
            
            // Log based on severity
            switch (r->status) {
                case HEALTH_CHECK_OK:
                    // Only log OK status in verbose mode
                    break;
                    
                case HEALTH_CHECK_WARNING:
                    ERROR_WARNING(r->category, r->value, r->message);
                    break;
                    
                case HEALTH_CHECK_ERROR:
                    ERROR_ERROR(r->category, r->value, r->message);
                    break;
                    
                case HEALTH_CHECK_CRITICAL:
                    ERROR_CRITICAL(r->category, r->value, r->message);
                    
                    // For critical errors, we might want to take action
                    if (r->category == HEALTH_CATEGORY_MEMORY && 
                        r->value > 90) {  // >90% memory usage
                        // Trigger memory cleanup
                        // TODO: Implement memory reclamation
                    }
                    break;
            }
        }
        
        free(results);
        pthread_mutex_unlock(&health_mutex);
        
        // Sleep for 1 second between checks
        sleep(1);
    }
    
    return NULL;
}

// Main daemon loop
static void daemon_loop(void) {
    pthread_t health_thread;
    
    // Create health monitoring thread
    if (pthread_create(&health_thread, NULL, health_monitor_thread, NULL) != 0) {
        PANIC(PANIC_GENERAL, "Failed to create health monitoring thread");
    }
    
    // Main loop
    while (running) {
        // Feed the watchdog
        if (watchdog_feed() != 0) {
            ERROR_ERROR(ERROR_DOMAIN_HARDWARE, 0x3004, "Failed to feed watchdog");
        }
        
        // Sleep for a short time (e.g., 100ms)
        usleep(100000);
    }
    
    // Cleanup
    void *retval;
    pthread_join(health_thread, &retval);
}

// Daemon entry point
int main(int argc, char *argv[]) {
    (void)argc;  // Unused parameters
    (void)argv;
    
    printf("TBOS Daemon (tbosd) starting...\n");
    
    // Initialize components
    if (init_signals() != 0) {
        return EXIT_FAILURE;
    }
    
    if (error_init() != 0) {
        fprintf(stderr, "Failed to initialize error handling\n");
        return EXIT_FAILURE;
    }
    
    panic_init();
    
    if (health_init() != 0) {
        ERROR_CRITICAL(ERROR_DOMAIN_SYSTEM, 0x3005, "Failed to initialize health monitoring");
        return EXIT_FAILURE;
    }
    
    if (init_watchdog() != 0) {
        return EXIT_FAILURE;
    }
    
    // Run the daemon
    daemon_loop();
    
    // Cleanup
    watchdog_stop();
    
    printf("TBOS Daemon stopped\n");
    return EXIT_SUCCESS;
}
