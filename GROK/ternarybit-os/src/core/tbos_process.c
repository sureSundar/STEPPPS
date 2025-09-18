/**
 * @file tbos_process.c
 * @brief TernaryBit OS Real Process Management System Implementation
 */

#include "tbos_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <time.h>

// Global process management structures
static tbos_process_t* g_process_table[TBOS_MAX_PROCESSES];
static pthread_mutex_t g_process_table_mutex = PTHREAD_MUTEX_INITIALIZER;
static pid_t g_next_process_id = 1;
tbos_scheduler_t* g_tbos_scheduler = NULL;

// Remove duplicate scheduler definition - use header version

// Result structure
typedef struct {
    bool success;
    void* data;
    const char* message;
} tbos_result_t;

// Initialize process management system
int tbos_process_init(void) {
    if (g_tbos_scheduler) {
        return 0; // Already initialized
    }

    g_tbos_scheduler = malloc(sizeof(tbos_scheduler_t));
    if (!g_tbos_scheduler) {
        return -1;
    }

    memset(g_tbos_scheduler, 0, sizeof(tbos_scheduler_t));
    // g_tbos_scheduler->initialized = true; // Field may not exist in header version

    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        g_process_table[i] = NULL;
    }

    return 0;
}

// Get process by PID
static tbos_process_t* get_process(pid_t process_id) {
    if (process_id <= 0 || process_id >= TBOS_MAX_PROCESSES) {
        return NULL;
    }
    return g_process_table[process_id];
}

// Create a new process
pid_t tbos_process_create(const char* name, void (*entry_point)(void*), void* args, int priority) {
    if (!name || !entry_point || !g_tbos_scheduler) {
        return -1;
    }

    pthread_mutex_lock(&g_process_table_mutex);

    // Find available slot
    pid_t process_id = -1;
    for (int i = 1; i < TBOS_MAX_PROCESSES; i++) {
        if (!g_process_table[i]) {
            process_id = i;
            break;
        }
    }

    if (process_id == -1) {
        pthread_mutex_unlock(&g_process_table_mutex);
        return -1; // No available slots
    }

    // Allocate process structure
    tbos_process_t* process = malloc(sizeof(tbos_process_t));
    if (!process) {
        pthread_mutex_unlock(&g_process_table_mutex);
        return -1;
    }

    // Initialize process
    memset(process, 0, sizeof(tbos_process_t));
    process->process_id = process_id;
    process->parent_process_id = 0; // Simplified
    strncpy(process->name, name, TBOS_MAX_PROCESS_NAME - 1);
    process->state = PROCESS_STATE_CREATED;
    process->priority = priority;
    process->creation_time = time(NULL);

    // Allocate stack memory
    process->page_directory = mmap(NULL, TBOS_STACK_SIZE,
                                  PROT_READ | PROT_WRITE,
                                  MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (process->page_directory == MAP_FAILED) {
        free(process);
        pthread_mutex_unlock(&g_process_table_mutex);
        return -1;
    }

    // Store in process table
    g_process_table[process_id] = process;

    // Add to ready queue
    process->state = PROCESS_STATE_READY;
    tbos_scheduler_add_ready(process);

    pthread_mutex_unlock(&g_process_table_mutex);
    return process_id;
}

// Add process to scheduler ready queue
void tbos_scheduler_add_ready(tbos_process_t* process) {
    if (!process || !g_tbos_scheduler) return;

    int priority = process->priority;
    if (priority < 0) priority = 0;
    if (priority >= 40) priority = 39;

    process->next = g_tbos_scheduler->ready_queue[priority];
    g_tbos_scheduler->ready_queue[priority] = process;
    process->state = PROCESS_STATE_READY;
}

// Remove process from ready queue
void tbos_scheduler_remove_ready(tbos_process_t* process) {
    if (!process || !g_tbos_scheduler) return;

    int priority = process->priority;
    if (priority < 0) priority = 0;
    if (priority >= 40) priority = 39;

    tbos_process_t** queue = &g_tbos_scheduler->ready_queue[priority];
    tbos_process_t* current = *queue;
    tbos_process_t* prev = NULL;

    while (current) {
        if (current == process) {
            if (prev) {
                prev->next = current->next;
            } else {
                *queue = current->next;
            }
            current->next = NULL;
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Schedule next process
tbos_process_t* tbos_scheduler_schedule(void) {
    if (!g_tbos_scheduler) return NULL;

    for (int priority = 0; priority < 40; priority++) {
        tbos_process_t* process = g_tbos_scheduler->ready_queue[priority];
        if (process) {
            tbos_scheduler_remove_ready(process);
            process->state = PROCESS_STATE_RUNNING;
            g_tbos_scheduler->current_process = process;
            g_tbos_scheduler->context_switches++;
            return process;
        }
    }
    return NULL;
}

// Terminate process
int tbos_process_terminate(pid_t process_id, int exit_code) {
    tbos_process_t* process = get_process(process_id);
    if (!process) return -1;

    pthread_mutex_lock(&g_process_table_mutex);

    // Clean up memory
    if (process->page_directory) {
        munmap(process->page_directory, TBOS_STACK_SIZE);
    }

    // Remove from scheduler
    tbos_scheduler_remove_ready(process);

    // Free process structure
    g_process_table[process_id] = NULL;
    free(process);

    pthread_mutex_unlock(&g_process_table_mutex);
    return 0;
}

// Get process statistics
void tbos_process_get_stats(tbos_process_stats_t* stats) {
    if (!stats) return;

    pthread_mutex_lock(&g_process_table_mutex);

    stats->total_processes_created = 0;
    stats->active_processes = 0;

    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        if (g_process_table[i]) {
            stats->total_processes_created++;
            if (g_process_table[i]->state != PROCESS_STATE_TERMINATED) {
                stats->active_processes++;
            }
        }
    }

    stats->total_context_switches = g_tbos_scheduler ? g_tbos_scheduler->context_switches : 0;
    stats->total_cpu_time = 0;
    stats->average_cpu_utilization = 0.0;
    stats->zombie_processes = 0;

    pthread_mutex_unlock(&g_process_table_mutex);
}

// List active processes
int tbos_process_list(tbos_process_t** processes, int max_processes) {
    if (!processes) return 0;

    pthread_mutex_lock(&g_process_table_mutex);
    int count = 0;
    for (int i = 0; i < TBOS_MAX_PROCESSES && count < max_processes; i++) {
        if (g_process_table[i]) {
            processes[count++] = g_process_table[i];
        }
    }
    pthread_mutex_unlock(&g_process_table_mutex);
    return count;
}

// Clean up process management
void tbos_process_cleanup(void) {
    if (!g_tbos_scheduler) return;

    pthread_mutex_lock(&g_process_table_mutex);

    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        if (g_process_table[i]) {
            tbos_process_terminate(i, 0);
        }
    }

    free(g_tbos_scheduler);
    g_tbos_scheduler = NULL;

    pthread_mutex_unlock(&g_process_table_mutex);
}

// Stub implementations for remaining functions
tbos_process_t* tbos_process_get(pid_t process_id) {
    return get_process(process_id);
}

int tbos_process_kill(pid_t process_id, int signal) {
    (void)process_id; (void)signal;
    return -1;
}

pid_t tbos_process_wait(pid_t process_id, int* status, int options) {
    (void)process_id; (void)status; (void)options;
    return -1;
}

int tbos_process_set_priority(pid_t process_id, int priority) {
    tbos_process_t* process = get_process(process_id);
    if (!process) return -1;
    process->priority = priority;
    return 0;
}

int tbos_process_get_priority(pid_t process_id) {
    tbos_process_t* process = get_process(process_id);
    return process ? process->priority : -1;
}