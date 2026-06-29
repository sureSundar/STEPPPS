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
#include "../memory/memory_reclaim.h"

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
                        // Trigger memory reclamation
                        printf("[TBOSD] Critical memory pressure detected (%d%%), initiating reclamation\n", r->value);

                        // First try quick reclamation
                        size_t freed = mem_reclaim_quick(50 * 1024 * 1024);  // Try to free 50MB
                        printf("[TBOSD] Quick reclaim freed %zu bytes\n", freed);

                        // If still critical, trigger emergency
                        if (mem_reclaim_get_usage_percent() > 95) {
                            printf("[TBOSD] Still critical, triggering emergency reclamation\n");
                            freed = mem_reclaim_emergency();
                            printf("[TBOSD] Emergency reclaim freed %zu bytes\n", freed);
                        }
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
    static uint32_t loop_counter = 0;
    while (running) {
        // Feed the watchdog
        if (watchdog_feed() != 0) {
            ERROR_ERROR(ERROR_DOMAIN_HARDWARE, 0x3004, "Failed to feed watchdog");
        }

        // Periodic memory pressure check (every 10 iterations = 1 second)
        if (++loop_counter % 10 == 0) {
            mem_reclaim_trigger_check();
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

    // Initialize memory reclamation system
    if (mem_reclaim_init() != 0) {
        ERROR_WARNING(ERROR_DOMAIN_MEMORY, 0x3006, "Failed to initialize memory reclamation");
        // Continue anyway - not critical for startup
    }

    if (init_watchdog() != 0) {
        return EXIT_FAILURE;
    }
    
    // Run the daemon
    daemon_loop();
    
    // Cleanup
    watchdog_stop();
    mem_reclaim_shutdown();

    printf("TBOS Daemon stopped\n");
    return EXIT_SUCCESS;
}
