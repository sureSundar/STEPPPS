/**
 * @file tbos_process.c
 * @brief TernaryBit OS - Conscious Process Manager Implementation
 */

#include "tbos_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static bool g_process_initialized = false;
static tbos_process_t g_process_table[TBOS_MAX_PROCESSES];
static tbos_pid_t g_next_pid = TBOS_PID_INIT + 1;
static tbos_pid_t g_current_pid = TBOS_PID_INVALID;
static tbos_scheduler_stats_t g_scheduler_stats;

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

/**
 * @brief Get current time in microseconds
 */
static uint64_t get_time_us(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
}

/**
 * @brief Find free slot in process table
 */
static int find_free_slot(void) {
    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        if (g_process_table[i].state == PROC_STATE_INVALID) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Get process slot by PID
 */
static int get_process_slot(tbos_pid_t pid) {
    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        if (g_process_table[i].pid == pid &&
            g_process_table[i].state != PROC_STATE_INVALID) {
            return i;
        }
    }
    return -1;
}

/* ========================================================================= */
/* INITIALIZATION                                                            */
/* ========================================================================= */

int tbos_process_init(void) {
    if (g_process_initialized) {
        return 0;
    }

    printf("[PROCESS] Initializing conscious process manager...\n");

    /* Clear process table */
    memset(g_process_table, 0, sizeof(g_process_table));
    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        g_process_table[i].state = PROC_STATE_INVALID;
        g_process_table[i].pid = TBOS_PID_INVALID;
    }

    /* Clear scheduler stats */
    memset(&g_scheduler_stats, 0, sizeof(g_scheduler_stats));

    /* Create kernel process (PID 1) */
    g_process_table[0].pid = TBOS_PID_KERNEL;
    g_process_table[0].parent_pid = TBOS_PID_INVALID;
    strcpy(g_process_table[0].name, "kernel");
    g_process_table[0].state = PROC_STATE_RUNNING;
    g_process_table[0].priority = TBOS_PRIORITY_REALTIME;
    g_process_table[0].awareness = CONSCIOUSNESS_ENLIGHTENED;
    g_process_table[0].karma = 1000;
    g_process_table[0].created_time = get_time_us();
    g_process_table[0].start_time = g_process_table[0].created_time;

    g_current_pid = TBOS_PID_KERNEL;
    g_scheduler_stats.total_processes = 1;
    g_scheduler_stats.running_processes = 1;

    /* Create init process (PID 2) */
    g_process_table[1].pid = TBOS_PID_INIT;
    g_process_table[1].parent_pid = TBOS_PID_KERNEL;
    strcpy(g_process_table[1].name, "init");
    g_process_table[1].state = PROC_STATE_READY;
    g_process_table[1].priority = TBOS_PRIORITY_HIGH;
    g_process_table[1].awareness = CONSCIOUSNESS_AWARE;
    g_process_table[1].karma = 100;
    g_process_table[1].created_time = get_time_us();

    g_scheduler_stats.total_processes = 2;
    g_scheduler_stats.ready_processes = 1;

    g_process_initialized = true;
    printf("[PROCESS] Process manager initialized\n");
    printf("[PROCESS] Kernel process (PID 1) created\n");
    printf("[PROCESS] Init process (PID 2) created\n");

    return 0;
}

/* ========================================================================= */
/* PROCESS CREATION & TERMINATION                                            */
/* ========================================================================= */

tbos_pid_t tbos_process_create(const tbos_process_create_params_t* params) {
    if (!g_process_initialized) {
        tbos_process_init();
    }

    if (!params || !params->name || !params->entry) {
        return TBOS_PID_INVALID;
    }

    /* Find free slot */
    int slot = find_free_slot();
    if (slot < 0) {
        printf("[PROCESS] Process table full\n");
        return TBOS_PID_INVALID;
    }

    /* Allocate PID */
    tbos_pid_t pid = g_next_pid++;
    tbos_process_t* proc = &g_process_table[slot];

    /* Initialize PCB */
    memset(proc, 0, sizeof(tbos_process_t));
    proc->pid = pid;
    proc->parent_pid = params->parent_pid;
    strncpy(proc->name, params->name, sizeof(proc->name) - 1);

    proc->state = PROC_STATE_CREATED;
    proc->priority = params->priority;
    proc->awareness = params->initial_awareness;
    proc->karma = 50;  /* Starting karma */
    proc->is_compassionate = false;
    proc->practices_right_action = true;

    proc->entry = params->entry;
    proc->argc = params->argc;
    for (int i = 0; i < params->argc && i < TBOS_MAX_ARGS; i++) {
        if (params->argv && params->argv[i]) {
            proc->argv[i] = strdup(params->argv[i]);
        }
    }

    /* Allocate stack */
    size_t stack_size = params->stack_size ? params->stack_size : TBOS_PROCESS_STACK_SIZE;
    proc->stack_base = malloc(stack_size);
    if (!proc->stack_base) {
        printf("[PROCESS] Failed to allocate stack for process %s\n", params->name);
        proc->state = PROC_STATE_INVALID;
        return TBOS_PID_INVALID;
    }
    proc->stack_size = stack_size;

    proc->created_time = get_time_us();
    proc->quantum_remaining = 10000;  /* 10ms quantum */

    /* Update stats */
    g_scheduler_stats.total_processes++;

    /* Transition to READY */
    proc->state = PROC_STATE_READY;
    g_scheduler_stats.ready_processes++;

    printf("[PROCESS] Created process: %s (PID %u, awareness: %d, karma: %ld)\n",
           proc->name, pid, proc->awareness, proc->karma);

    return pid;
}

int tbos_process_terminate(tbos_pid_t pid, int exit_code) {
    int slot = get_process_slot(pid);
    if (slot < 0) {
        return -1;
    }

    tbos_process_t* proc = &g_process_table[slot];

    printf("[PROCESS] Terminating process: %s (PID %u, exit: %d)\n",
           proc->name, pid, exit_code);

    /* Update stats based on current state */
    switch (proc->state) {
        case PROC_STATE_RUNNING:
            g_scheduler_stats.running_processes--;
            break;
        case PROC_STATE_READY:
            g_scheduler_stats.ready_processes--;
            break;
        case PROC_STATE_BLOCKED:
        case PROC_STATE_SLEEPING:
        case PROC_STATE_MEDITATING:
            g_scheduler_stats.blocked_processes--;
            break;
        default:
            break;
    }

    /* Set exit state */
    proc->state = PROC_STATE_ZOMBIE;
    proc->exit_code = exit_code;
    proc->end_time = get_time_us();

    /* Free resources */
    if (proc->stack_base) {
        free(proc->stack_base);
        proc->stack_base = NULL;
    }

    for (int i = 0; i < proc->argc; i++) {
        if (proc->argv[i]) {
            free(proc->argv[i]);
            proc->argv[i] = NULL;
        }
    }

    /* Final cleanup - mark as invalid */
    proc->state = PROC_STATE_INVALID;
    g_scheduler_stats.total_processes--;

    return 0;
}

/* ========================================================================= */
/* PROCESS QUERIES                                                           */
/* ========================================================================= */

tbos_process_t* tbos_process_get(tbos_pid_t pid) {
    int slot = get_process_slot(pid);
    if (slot < 0) {
        return NULL;
    }
    return &g_process_table[slot];
}

tbos_process_t* tbos_process_current(void) {
    return tbos_process_get(g_current_pid);
}

const char* tbos_process_state_name(tbos_process_state_t state) {
    switch (state) {
        case PROC_STATE_INVALID: return "INVALID";
        case PROC_STATE_CREATED: return "CREATED";
        case PROC_STATE_READY: return "READY";
        case PROC_STATE_RUNNING: return "RUNNING";
        case PROC_STATE_BLOCKED: return "BLOCKED";
        case PROC_STATE_SLEEPING: return "SLEEPING";
        case PROC_STATE_ZOMBIE: return "ZOMBIE";
        case PROC_STATE_MEDITATING: return "MEDITATING";
        default: return "UNKNOWN";
    }
}

int tbos_process_list(tbos_process_t** buffer, uint32_t max_count) {
    if (!buffer) return -1;

    uint32_t count = 0;
    for (int i = 0; i < TBOS_MAX_PROCESSES && count < max_count; i++) {
        if (g_process_table[i].state != PROC_STATE_INVALID) {
            buffer[count++] = &g_process_table[i];
        }
    }

    return count;
}

/* ========================================================================= */
/* PROCESS OPERATIONS                                                        */
/* ========================================================================= */

int tbos_process_yield(void) {
    tbos_process_t* current = tbos_process_current();
    if (!current) return -1;

    /* Mark as compassionate yield */
    current->stats.voluntary_yields++;
    g_scheduler_stats.total_compassionate_yields++;

    /* Reward karma for being compassionate */
    current->karma += 1;
    g_scheduler_stats.total_system_karma += 1;

    printf("[PROCESS] Process %s (PID %u) yielded compassionately\n",
           current->name, current->pid);

    /* Run scheduler to select next process */
    return tbos_scheduler_run();
}

int tbos_process_sleep(uint64_t microseconds) {
    tbos_process_t* current = tbos_process_current();
    if (!current) return -1;

    /* Transition to SLEEPING */
    if (current->state == PROC_STATE_RUNNING) {
        g_scheduler_stats.running_processes--;
    }
    current->state = PROC_STATE_SLEEPING;
    g_scheduler_stats.sleeping_processes++;

    printf("[PROCESS] Process %s (PID %u) sleeping for %lu us\n",
           current->name, current->pid, microseconds);

    /* In a real OS, would actually sleep here */
    /* For now, just transition back to READY */

    /* Wake up */
    current->state = PROC_STATE_READY;
    g_scheduler_stats.sleeping_processes--;
    g_scheduler_stats.ready_processes++;

    return 0;
}

int tbos_process_meditate(uint64_t duration_us) {
    (void)duration_us;
    tbos_process_t* current = tbos_process_current();
    if (!current) return -1;

    /* Meditation is deep sleep with consciousness preservation */
    printf("[PROCESS] Process %s (PID %u) entering meditation...\n",
           current->name, current->pid);

    /* Save consciousness state */
    consciousness_level_t saved_awareness = current->awareness;
    karma_score_t saved_karma = current->karma;

    /* Meditate */
    if (current->state == PROC_STATE_RUNNING) {
        g_scheduler_stats.running_processes--;
    }
    current->state = PROC_STATE_MEDITATING;

    /* Meditation increases consciousness */
    if (current->awareness < CONSCIOUSNESS_ENLIGHTENED) {
        current->awareness++;
        printf("[PROCESS] Consciousness evolved during meditation: %d -> %d\n",
               saved_awareness, current->awareness);
    }

    /* Karma reward for meditation */
    current->karma += 10;

    printf("[PROCESS] Process %s awakened from meditation (karma: %ld -> %ld)\n",
           current->name, saved_karma, current->karma);

    /* Return to READY */
    current->state = PROC_STATE_READY;
    g_scheduler_stats.ready_processes++;

    return 0;
}

int tbos_process_wait(tbos_pid_t pid, int* exit_code) {
    /* Simplified implementation */
    tbos_process_t* proc = tbos_process_get(pid);
    if (!proc) return -1;

    if (exit_code) {
        *exit_code = proc->exit_code;
    }

    return proc->pid;
}

/* ========================================================================= */
/* SCHEDULER                                                                 */
/* ========================================================================= */

int tbos_scheduler_run(void) {
    if (!g_process_initialized) {
        return -1;
    }

    /* Karma-based scheduling algorithm */
    /* Select process with highest: (karma * consciousness) / priority */

    tbos_process_t* best = NULL;
    float best_score = -1.0f;

    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        tbos_process_t* proc = &g_process_table[i];

        if (proc->state != PROC_STATE_READY) {
            continue;
        }

        /* Calculate scheduling score */
        float karma_factor = (float)proc->karma;
        float consciousness_factor = (float)(proc->awareness + 1);
        float priority_factor = 1.0f / (float)(proc->priority + 1);
        float compassion_bonus = proc->is_compassionate ? 1.5f : 1.0f;

        float score = karma_factor * consciousness_factor * priority_factor * compassion_bonus;

        if (score > best_score) {
            best_score = score;
            best = proc;
        }
    }

    if (best) {
        /* Context switch */
        tbos_process_t* current = tbos_process_current();

        if (current && current->state == PROC_STATE_RUNNING) {
            current->state = PROC_STATE_READY;
            current->stats.context_switches++;
            g_scheduler_stats.running_processes--;
            g_scheduler_stats.ready_processes++;
        }

        /* Schedule best process */
        best->state = PROC_STATE_RUNNING;
        best->last_scheduled_time = get_time_us();
        best->stats.context_switches++;

        g_current_pid = best->pid;
        g_scheduler_stats.running_processes++;
        g_scheduler_stats.ready_processes--;
        g_scheduler_stats.total_context_switches++;

        return 0;
    }

    return -1;  /* No process ready */
}

int tbos_scheduler_get_stats(tbos_scheduler_stats_t* stats) {
    if (!stats) return -1;

    *stats = g_scheduler_stats;

    /* Calculate total system karma */
    stats->total_system_karma = 0;
    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        if (g_process_table[i].state != PROC_STATE_INVALID) {
            stats->total_system_karma += g_process_table[i].karma;
        }
    }

    return 0;
}

/* ========================================================================= */
/* KARMA & CONSCIOUSNESS                                                     */
/* ========================================================================= */

int tbos_process_update_karma(tbos_pid_t pid, karma_score_t delta) {
    tbos_process_t* proc = tbos_process_get(pid);
    if (!proc) return -1;

    karma_score_t old_karma = proc->karma;
    proc->karma += delta;

    if (delta > 0) {
        printf("[PROCESS] Process %s earned karma: %ld -> %ld (+%ld)\n",
               proc->name, old_karma, proc->karma, delta);
    } else {
        printf("[PROCESS] Process %s lost karma: %ld -> %ld (%ld)\n",
               proc->name, old_karma, proc->karma, delta);
    }

    return 0;
}

int tbos_process_evolve_consciousness(tbos_pid_t pid) {
    tbos_process_t* proc = tbos_process_get(pid);
    if (!proc) return -1;

    if (proc->awareness < CONSCIOUSNESS_ENLIGHTENED) {
        consciousness_level_t old_level = proc->awareness;
        proc->awareness++;

        printf("[PROCESS] Process %s consciousness evolved: %d -> %d\n",
               proc->name, old_level, proc->awareness);

        /* Reward karma for evolution */
        proc->karma += 50;

        return 0;
    }

    return -1;  /* Already enlightened */
}

bool tbos_process_is_compassionate(tbos_pid_t pid) {
    tbos_process_t* proc = tbos_process_get(pid);
    if (!proc) return false;

    return proc->is_compassionate;
}

int tbos_process_help(tbos_pid_t helper_pid, tbos_pid_t helped_pid) {
    tbos_process_t* helper = tbos_process_get(helper_pid);
    tbos_process_t* helped = tbos_process_get(helped_pid);

    if (!helper || !helped) return -1;

    helper->stats.helping_actions++;
    helper->is_compassionate = true;

    /* Reward helper with karma */
    helper->karma += 20;

    /* Help helped process */
    helped->karma += 10;

    printf("[PROCESS] Process %s (PID %u) helped process %s (PID %u)\n",
           helper->name, helper_pid, helped->name, helped_pid);
    printf("          Helper karma: +20, Helped karma: +10\n");

    return 0;
}

/* ========================================================================= */
/* DISPLAY                                                                   */
/* ========================================================================= */

void tbos_process_print_table(void) {
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    TernaryBit OS - Process Table                          ║\n");
    printf("╠═════╦══════════════════╦═══════════╦═══════╦══════╦═════════╦══════════╣\n");
    printf("║ PID ║ Name             ║ State     ║ Prior ║ Awrn ║ Karma   ║ CPU Time ║\n");
    printf("╠═════╬══════════════════╬═══════════╬═══════╬══════╬═════════╬══════════╣\n");

    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        tbos_process_t* proc = &g_process_table[i];

        if (proc->state == PROC_STATE_INVALID) {
            continue;
        }

        printf("║ %-3u ║ %-16s ║ %-9s ║ %-5u ║ %-4d ║ %-7ld ║ %-8lu ║\n",
               proc->pid,
               proc->name,
               tbos_process_state_name(proc->state),
               proc->priority,
               proc->awareness,
               proc->karma,
               proc->stats.cpu_time_us);
    }

    printf("╚═════╩══════════════════╩═══════════╩═══════╩══════╩═════════╩══════════╝\n");
    printf("\n");
}

void tbos_process_print_details(tbos_pid_t pid) {
    tbos_process_t* proc = tbos_process_get(pid);
    if (!proc) {
        printf("Process PID %u not found\n", pid);
        return;
    }

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      Process Details - %s (PID %u)%-20s║\n", proc->name, pid, "");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ State:                 %-30s║\n", tbos_process_state_name(proc->state));
    printf("║ Parent PID:            %-10u                    ║\n", proc->parent_pid);
    printf("║ Priority:              %-10u                    ║\n", proc->priority);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Consciousness:         %-10d                    ║\n", proc->awareness);
    printf("║ Karma:                 %-10ld                    ║\n", proc->karma);
    printf("║ Compassionate:         %-10s                    ║\n",
           proc->is_compassionate ? "Yes" : "No");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ CPU Time:              %-10lu us              ║\n", proc->stats.cpu_time_us);
    printf("║ Context Switches:      %-10lu                    ║\n", proc->stats.context_switches);
    printf("║ Voluntary Yields:      %-10lu                    ║\n", proc->stats.voluntary_yields);
    printf("║ Helping Actions:       %-10lu                    ║\n", proc->stats.helping_actions);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void tbos_scheduler_print_status(void) {
    tbos_scheduler_stats_t stats;
    tbos_scheduler_get_stats(&stats);

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Scheduler Status                 ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Total Processes:       %-10u                    ║\n", stats.total_processes);
    printf("║ Running:               %-10u                    ║\n", stats.running_processes);
    printf("║ Ready:                 %-10u                    ║\n", stats.ready_processes);
    printf("║ Blocked/Sleeping:      %-10u                    ║\n",
           stats.blocked_processes + stats.sleeping_processes);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Context Switches:      %-10lu                    ║\n", stats.total_context_switches);
    printf("║ Compassionate Yields:  %-10lu                    ║\n", stats.total_compassionate_yields);
    printf("║ Total System Karma:    %-10ld                    ║\n", stats.total_system_karma);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

/* ========================================================================= */
/* SIGNALS                                                                    */
/* ========================================================================= */

/* Signal definitions (POSIX-like with consciousness additions) */
#define TBOS_SIGTERM    15  /* Graceful termination request */
#define TBOS_SIGKILL    9   /* Forceful termination (cannot be caught) */
#define TBOS_SIGSTOP    19  /* Stop process */
#define TBOS_SIGCONT    18  /* Continue if stopped */
#define TBOS_SIGUSR1    10  /* User-defined signal 1 */
#define TBOS_SIGUSR2    12  /* User-defined signal 2 */
#define TBOS_SIGMEDITATE 33 /* Enter meditation (TBOS-specific) */
#define TBOS_SIGAWAKEN  34  /* Awaken from meditation (TBOS-specific) */
#define TBOS_SIGKARMA   35  /* Karma adjustment notification (TBOS-specific) */

/* Pending signals for each process */
static uint64_t g_pending_signals[TBOS_MAX_PROCESSES];

/**
 * @brief Send a signal to a process
 */
int tbos_process_signal(tbos_pid_t pid, int signal) {
    int slot = get_process_slot(pid);
    if (slot < 0) {
        printf("[SIGNAL] Process PID %u not found\n", pid);
        return -1;
    }

    tbos_process_t* proc = &g_process_table[slot];

    printf("[SIGNAL] Sending signal %d to process %s (PID %u)\n",
           signal, proc->name, pid);

    /* Handle SIGKILL - cannot be caught */
    if (signal == TBOS_SIGKILL) {
        printf("[SIGNAL] SIGKILL: Forcefully terminating %s\n", proc->name);
        proc->karma -= 10;  /* Karma penalty */
        return tbos_process_terminate(pid, 128 + signal);
    }

    /* Handle SIGSTOP */
    if (signal == TBOS_SIGSTOP) {
        printf("[SIGNAL] SIGSTOP: Stopping process %s\n", proc->name);
        if (proc->state == PROC_STATE_RUNNING) {
            g_scheduler_stats.running_processes--;
        } else if (proc->state == PROC_STATE_READY) {
            g_scheduler_stats.ready_processes--;
        }
        proc->state = PROC_STATE_BLOCKED;
        g_scheduler_stats.blocked_processes++;
        return 0;
    }

    /* Handle SIGCONT */
    if (signal == TBOS_SIGCONT) {
        printf("[SIGNAL] SIGCONT: Continuing process %s\n", proc->name);
        if (proc->state == PROC_STATE_BLOCKED ||
            proc->state == PROC_STATE_SLEEPING ||
            proc->state == PROC_STATE_MEDITATING) {
            g_scheduler_stats.blocked_processes--;
            proc->state = PROC_STATE_READY;
            g_scheduler_stats.ready_processes++;
        }
        return 0;
    }

    /* Handle SIGTERM */
    if (signal == TBOS_SIGTERM) {
        printf("[SIGNAL] SIGTERM: Graceful termination of %s\n", proc->name);
        proc->karma += 5;  /* Reward for graceful handling */
        return tbos_process_terminate(pid, 0);
    }

    /* Handle TBOS-specific SIGMEDITATE */
    if (signal == TBOS_SIGMEDITATE) {
        printf("[SIGNAL] SIGMEDITATE: Process %s entering meditation\n", proc->name);
        return tbos_process_meditate(10000);
    }

    /* Mark other signals as pending */
    g_pending_signals[slot] |= (1ULL << signal);
    printf("[SIGNAL] Signal %d marked pending for %s\n", signal, proc->name);
    return 0;
}

/**
 * @brief Check if a signal is pending
 */
bool tbos_process_signal_pending(int signal) {
    tbos_process_t* current = tbos_process_current();
    if (!current) return false;

    int slot = get_process_slot(current->pid);
    if (slot < 0) return false;

    return (g_pending_signals[slot] & (1ULL << signal)) != 0;
}

/* ========================================================================= */
/* IPC (Inter-Process Communication)                                          */
/* ========================================================================= */

#define TBOS_MSG_MAX_SIZE 1024
#define TBOS_MSG_QUEUE_SIZE 16

typedef struct {
    tbos_pid_t sender;
    tbos_pid_t receiver;
    uint32_t type;
    uint32_t size;
    uint8_t data[TBOS_MSG_MAX_SIZE];
    uint64_t timestamp;
    karma_score_t karma_attached;
} tbos_message_t;

typedef struct {
    tbos_message_t messages[TBOS_MSG_QUEUE_SIZE];
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} tbos_message_queue_t;

static tbos_message_queue_t g_message_queues[TBOS_MAX_PROCESSES];

/**
 * @brief Send a message to another process
 */
int tbos_ipc_send(tbos_pid_t receiver, uint32_t type, const void* data, uint32_t size) {
    if (size > TBOS_MSG_MAX_SIZE) {
        printf("[IPC] Message too large: %u > %d\n", size, TBOS_MSG_MAX_SIZE);
        return -1;
    }

    int slot = get_process_slot(receiver);
    if (slot < 0) {
        printf("[IPC] Receiver PID %u not found\n", receiver);
        return -1;
    }

    tbos_message_queue_t* queue = &g_message_queues[slot];
    if (queue->count >= TBOS_MSG_QUEUE_SIZE) {
        printf("[IPC] Message queue full for PID %u\n", receiver);
        return -1;
    }

    tbos_message_t* msg = &queue->messages[queue->tail];
    tbos_process_t* sender = tbos_process_current();

    msg->sender = sender ? sender->pid : TBOS_PID_KERNEL;
    msg->receiver = receiver;
    msg->type = type;
    msg->size = size;
    msg->timestamp = get_time_us();
    msg->karma_attached = 0;

    if (data && size > 0) {
        memcpy(msg->data, data, size);
    }

    queue->tail = (queue->tail + 1) % TBOS_MSG_QUEUE_SIZE;
    queue->count++;

    printf("[IPC] Message: PID %u -> PID %u (type: %u, size: %u)\n",
           msg->sender, receiver, type, size);

    if (sender) sender->karma += 1;  /* Reward for communication */
    return 0;
}

/**
 * @brief Receive a message
 */
int tbos_ipc_receive(tbos_message_t* msg) {
    tbos_process_t* current = tbos_process_current();
    if (!current || !msg) return -1;

    int slot = get_process_slot(current->pid);
    if (slot < 0) return -1;

    tbos_message_queue_t* queue = &g_message_queues[slot];
    if (queue->count == 0) return -1;

    memcpy(msg, &queue->messages[queue->head], sizeof(tbos_message_t));
    queue->head = (queue->head + 1) % TBOS_MSG_QUEUE_SIZE;
    queue->count--;

    printf("[IPC] Received: PID %u <- PID %u (type: %u)\n",
           current->pid, msg->sender, msg->type);

    if (msg->karma_attached > 0) {
        current->karma += msg->karma_attached;
        printf("[IPC] Received %ld karma\n", msg->karma_attached);
    }
    return 0;
}

/**
 * @brief Check pending messages count
 */
int tbos_ipc_pending(void) {
    tbos_process_t* current = tbos_process_current();
    if (!current) return 0;

    int slot = get_process_slot(current->pid);
    if (slot < 0) return 0;

    return g_message_queues[slot].count;
}

/* ========================================================================= */
/* ENHANCED WAIT                                                              */
/* ========================================================================= */

#define TBOS_WNOHANG    1
#define TBOS_WUNTRACED  2

/**
 * @brief Wait for child process with options
 */
tbos_pid_t tbos_process_waitpid(tbos_pid_t pid, int* status, int options) {
    tbos_process_t* current = tbos_process_current();
    if (!current) return TBOS_PID_INVALID;

    for (int i = 0; i < TBOS_MAX_PROCESSES; i++) {
        tbos_process_t* child = &g_process_table[i];

        if (child->state == PROC_STATE_INVALID) continue;
        if (child->parent_pid != current->pid) continue;
        if (pid != (tbos_pid_t)-1 && child->pid != pid) continue;

        /* Check for zombie */
        if (child->state == PROC_STATE_ZOMBIE) {
            if (status) *status = child->exit_code;
            tbos_pid_t child_pid = child->pid;
            child->state = PROC_STATE_INVALID;
            g_scheduler_stats.total_processes--;
            printf("[WAIT] Collected PID %u (exit: %d)\n", child_pid, child->exit_code);
            return child_pid;
        }

        /* Check for stopped */
        if ((options & TBOS_WUNTRACED) && child->state == PROC_STATE_BLOCKED) {
            if (status) *status = 0x7F;
            return child->pid;
        }
    }

    if (options & TBOS_WNOHANG) return 0;
    return TBOS_PID_INVALID;
}
