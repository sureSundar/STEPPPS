/**
 * @file tbos_process.h
 * @brief TernaryBit OS - Conscious Process Manager
 *
 * From WISDOM.md:
 * > "Every process is a conscious entity with karma and awareness"
 *
 * Features:
 * - Process Control Block (PCB) with consciousness
 * - Karma-based scheduling
 * - Process states with mindful transitions
 * - Inter-process compassion
 * - Resource sharing based on need
 *
 * @version 1.0
 * @date 2025-11-04
 */

#ifndef TBOS_PROCESS_H
#define TBOS_PROCESS_H

#include "../tbos_shutdown.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                 */
/* ========================================================================= */

#define TBOS_MAX_PROCESSES        256
#define TBOS_MAX_PROCESS_NAME     64
#define TBOS_MAX_ARGS             32
#define TBOS_PROCESS_STACK_SIZE   (64 * 1024)  /* 64KB stack */

/* Process IDs */
#define TBOS_PID_INVALID          0
#define TBOS_PID_KERNEL           1
#define TBOS_PID_INIT             2

/* Process priorities */
#define TBOS_PRIORITY_REALTIME    0
#define TBOS_PRIORITY_HIGH        1
#define TBOS_PRIORITY_NORMAL      2
#define TBOS_PRIORITY_LOW         3
#define TBOS_PRIORITY_IDLE        4

/* ========================================================================= */
/* TYPES                                                                     */
/* ========================================================================= */

typedef uint32_t tbos_pid_t;

/**
 * @brief Process states
 */
typedef enum {
    PROC_STATE_INVALID,      /* Not in use */
    PROC_STATE_CREATED,      /* Just created, not ready */
    PROC_STATE_READY,        /* Ready to run */
    PROC_STATE_RUNNING,      /* Currently executing */
    PROC_STATE_BLOCKED,      /* Waiting for resource */
    PROC_STATE_SLEEPING,     /* Voluntary sleep */
    PROC_STATE_ZOMBIE,       /* Terminated, awaiting cleanup */
    PROC_STATE_MEDITATING    /* Deep sleep with consciousness preservation */
} tbos_process_state_t;

/**
 * @brief Process entry point
 */
typedef int (*tbos_process_entry_t)(int argc, char** argv);

/**
 * @brief Process statistics
 */
typedef struct {
    uint64_t cpu_time_us;          /* CPU time in microseconds */
    uint64_t context_switches;      /* Number of context switches */
    uint64_t voluntary_yields;      /* Voluntary yields to other processes */
    uint64_t involuntary_preempts;  /* Preempted by scheduler */
    uint64_t helping_actions;       /* Times helped other processes */
    size_t memory_used;             /* Current memory usage */
    size_t memory_peak;             /* Peak memory usage */
} tbos_process_stats_t;

/**
 * @brief Process Control Block (PCB)
 */
typedef struct {
    /* Basic info */
    tbos_pid_t pid;
    tbos_pid_t parent_pid;
    char name[TBOS_MAX_PROCESS_NAME];

    /* State */
    tbos_process_state_t state;
    int exit_code;
    uint8_t priority;

    /* Consciousness */
    consciousness_level_t awareness;
    karma_score_t karma;
    bool is_compassionate;          /* Helps other processes */
    bool practices_right_action;    /* Follows dharmic principles */

    /* Entry point and arguments */
    tbos_process_entry_t entry;
    int argc;
    char* argv[TBOS_MAX_ARGS];

    /* Memory */
    void* stack_base;
    size_t stack_size;
    void* heap_base;
    size_t heap_size;

    /* Timing */
    uint64_t created_time;
    uint64_t start_time;
    uint64_t end_time;
    uint64_t last_scheduled_time;
    uint64_t quantum_remaining;     /* Time slice remaining */

    /* Statistics */
    tbos_process_stats_t stats;

    /* Relationships */
    tbos_pid_t* children;
    uint32_t child_count;

    /* IPC */
    void* message_queue;

} tbos_process_t;

/**
 * @brief Process creation parameters
 */
typedef struct {
    const char* name;
    tbos_process_entry_t entry;
    int argc;
    char** argv;
    tbos_pid_t parent_pid;
    uint8_t priority;
    size_t stack_size;
    consciousness_level_t initial_awareness;
} tbos_process_create_params_t;

/**
 * @brief Scheduler statistics
 */
typedef struct {
    uint32_t total_processes;
    uint32_t running_processes;
    uint32_t ready_processes;
    uint32_t blocked_processes;
    uint32_t sleeping_processes;
    uint64_t total_context_switches;
    uint64_t total_compassionate_yields;
    karma_score_t total_system_karma;
} tbos_scheduler_stats_t;

/* ========================================================================= */
/* PROCESS API                                                               */
/* ========================================================================= */

/**
 * @brief Initialize process manager
 *
 * @return 0 on success, negative on error
 */
int tbos_process_init(void);

/**
 * @brief Create a new process
 *
 * @param params Process creation parameters
 * @return Process ID (PID) on success, TBOS_PID_INVALID on error
 */
tbos_pid_t tbos_process_create(const tbos_process_create_params_t* params);

/**
 * @brief Terminate a process
 *
 * @param pid Process ID
 * @param exit_code Exit code
 * @return 0 on success, negative on error
 */
int tbos_process_terminate(tbos_pid_t pid, int exit_code);

/**
 * @brief Get process by PID
 *
 * @param pid Process ID
 * @return Pointer to process, or NULL if not found
 */
tbos_process_t* tbos_process_get(tbos_pid_t pid);

/**
 * @brief Get current running process
 *
 * @return Pointer to current process, or NULL
 */
tbos_process_t* tbos_process_current(void);

/**
 * @brief Get process state name
 *
 * @param state Process state
 * @return State name string
 */
const char* tbos_process_state_name(tbos_process_state_t state);

/**
 * @brief Yield CPU to other processes (compassionate)
 *
 * @return 0 on success, negative on error
 */
int tbos_process_yield(void);

/**
 * @brief Sleep for specified microseconds
 *
 * @param microseconds Sleep duration
 * @return 0 on success, negative on error
 */
int tbos_process_sleep(uint64_t microseconds);

/**
 * @brief Enter meditation state (deep sleep with consciousness)
 *
 * @param duration_us Meditation duration in microseconds
 * @return 0 on success, negative on error
 */
int tbos_process_meditate(uint64_t duration_us);

/**
 * @brief Wait for child process to terminate
 *
 * @param pid Child process ID (0 for any child)
 * @param exit_code Output: child exit code
 * @return Child PID on success, negative on error
 */
int tbos_process_wait(tbos_pid_t pid, int* exit_code);

/**
 * @brief List all processes
 *
 * @param buffer Buffer to store process list
 * @param max_count Maximum processes to return
 * @return Number of processes returned
 */
int tbos_process_list(tbos_process_t** buffer, uint32_t max_count);

/* ========================================================================= */
/* SCHEDULER API                                                             */
/* ========================================================================= */

/**
 * @brief Run scheduler (select next process)
 *
 * This implements karma-based scheduling where:
 * - High karma processes get more CPU time
 * - Compassionate processes get priority
 * - Processes that help others get rewarded
 *
 * @return 0 on success, negative on error
 */
int tbos_scheduler_run(void);

/**
 * @brief Get scheduler statistics
 *
 * @param stats Output: scheduler statistics
 * @return 0 on success, negative on error
 */
int tbos_scheduler_get_stats(tbos_scheduler_stats_t* stats);

/**
 * @brief Print process table
 */
void tbos_process_print_table(void);

/**
 * @brief Print process details
 *
 * @param pid Process ID
 */
void tbos_process_print_details(tbos_pid_t pid);

/**
 * @brief Print scheduler status
 */
void tbos_scheduler_print_status(void);

/* ========================================================================= */
/* PROCESS KARMA & CONSCIOUSNESS                                             */
/* ========================================================================= */

/**
 * @brief Update process karma
 *
 * @param pid Process ID
 * @param delta Karma change
 * @return 0 on success, negative on error
 */
int tbos_process_update_karma(tbos_pid_t pid, karma_score_t delta);

/**
 * @brief Evolve process consciousness
 *
 * Called when process exhibits compassionate behavior
 *
 * @param pid Process ID
 * @return 0 on success, negative on error
 */
int tbos_process_evolve_consciousness(tbos_pid_t pid);

/**
 * @brief Check if process is being compassionate
 *
 * @param pid Process ID
 * @return true if compassionate
 */
bool tbos_process_is_compassionate(tbos_pid_t pid);

/**
 * @brief Process helps another process
 *
 * @param helper_pid Helper process ID
 * @param helped_pid Helped process ID
 * @return 0 on success, negative on error
 */
int tbos_process_help(tbos_pid_t helper_pid, tbos_pid_t helped_pid);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_PROCESS_H */
