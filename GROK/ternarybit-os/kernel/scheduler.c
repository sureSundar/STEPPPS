// TBOS Basic Scheduler - Hour 6 Sprints
// 🕉️ Swamiye Saranam Aiyappa 🕉️
// Advanced scheduling algorithms and priority management

#include <stdint.h>
#include <stddef.h>

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);

// Process structure (from process.c)
typedef enum {
    PROCESS_CREATED,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

typedef struct process {
    uint32_t pid;
    char name[32];
    process_state_t state;
    uint32_t* stack_ptr;
    uint32_t stack_base;
    uint32_t stack_size;
    uint32_t entry_point;
    uint32_t priority;
    uint32_t time_slice;
    uint32_t total_time;
    struct process* next;
    uint32_t registers[8];
    uint32_t eip;
    uint32_t esp;
    uint32_t ebp;
    uint32_t eflags;
} process_t;

// External process functions
extern process_t* process_get_current(void);
extern void process_switch(void);
extern void process_set_ready(process_t* proc);

// Scheduling policies
typedef enum {
    SCHED_ROUND_ROBIN,     // Fair time sharing
    SCHED_PRIORITY,        // Priority-based
    SCHED_SHORTEST_JOB,    // Shortest job first
    SCHED_MULTILEVEL       // Multi-level feedback queue
} sched_policy_t;

// Scheduler statistics
typedef struct {
    uint32_t context_switches;
    uint32_t preemptions;
    uint32_t total_runtime;
    uint32_t idle_time;
    uint32_t active_processes;
    sched_policy_t current_policy;
} scheduler_stats_t;

// Global scheduler state
static scheduler_stats_t sched_stats = {0};
static sched_policy_t current_policy = SCHED_MULTILEVEL;
static uint32_t quantum_ticks = 10;  // Default time quantum
static uint32_t scheduler_tick_count = 0;

// Priority queues for multilevel scheduling
#define PRIORITY_LEVELS 8
static process_t* priority_queues[PRIORITY_LEVELS];
static uint32_t queue_time_slices[PRIORITY_LEVELS] = {50, 40, 30, 20, 15, 10, 5, 2};

// Initialize scheduler
void scheduler_init(void) {
    kernel_print("[SCHEDULER] Initializing advanced scheduler...\\n");

    // Clear statistics
    sched_stats.context_switches = 0;
    sched_stats.preemptions = 0;
    sched_stats.total_runtime = 0;
    sched_stats.idle_time = 0;
    sched_stats.active_processes = 0;
    sched_stats.current_policy = current_policy;

    // Initialize priority queues
    for (int i = 0; i < PRIORITY_LEVELS; i++) {
        priority_queues[i] = NULL;
    }

    scheduler_tick_count = 0;

    kernel_print("[SCHEDULER] Multi-level feedback queue initialized\\n");
    kernel_print("  Priority levels: ");
    kernel_print_hex(PRIORITY_LEVELS);
    kernel_print("\\n  Default quantum: ");
    kernel_print_hex(quantum_ticks);
    kernel_print(" ticks\\n");
}

// Add process to priority queue
void scheduler_add_to_priority_queue(process_t* proc) {
    if (!proc || proc->priority >= PRIORITY_LEVELS) return;

    uint32_t level = proc->priority;

    // Set time slice based on priority level
    proc->time_slice = queue_time_slices[level];

    // Add to end of priority queue
    if (!priority_queues[level]) {
        priority_queues[level] = proc;
        proc->next = NULL;
    } else {
        process_t* current = priority_queues[level];
        while (current->next) {
            current = current->next;
        }
        current->next = proc;
        proc->next = NULL;
    }
}

// Get next process from priority queues
process_t* scheduler_get_next_priority(void) {
    // Check queues from highest to lowest priority
    for (int level = 0; level < PRIORITY_LEVELS; level++) {
        if (priority_queues[level]) {
            process_t* next = priority_queues[level];
            priority_queues[level] = priority_queues[level]->next;
            next->next = NULL;
            return next;
        }
    }

    return NULL;  // No ready processes
}

// Round-robin scheduler
process_t* schedule_round_robin(void) {
    process_t* current = process_get_current();
    if (!current) return NULL;

    // Simple round-robin implementation
    // (This would integrate with process queue from process.c)

    sched_stats.context_switches++;
    return current;  // Simplified for demo
}

// Priority-based scheduler
process_t* schedule_priority(void) {
    return scheduler_get_next_priority();
}

// Shortest job first scheduler
process_t* schedule_shortest_job(void) {
    // Find process with minimum estimated runtime
    // For demo, use simple priority mapping
    process_t* shortest = scheduler_get_next_priority();

    if (shortest) {
        kernel_print("[SCHEDULER] SJF: Selected process ");
        kernel_print(shortest->name);
        kernel_print("\\n");
    }

    return shortest;
}

// Multi-level feedback queue scheduler
process_t* schedule_multilevel_feedback(void) {
    process_t* next = scheduler_get_next_priority();

    if (next) {
        // Adjust priority based on behavior
        process_t* current = process_get_current();
        if (current && current->time_slice == 0) {
            // Process used full time slice, demote priority
            if (current->priority < PRIORITY_LEVELS - 1) {
                current->priority++;
                kernel_print("[SCHEDULER] Demoted ");
                kernel_print(current->name);
                kernel_print(" to priority ");
                kernel_print_hex(current->priority);
                kernel_print("\\n");
            }
        }

        sched_stats.context_switches++;
    }

    return next;
}

// Main scheduler function
process_t* scheduler_select_next(void) {
    process_t* next = NULL;

    switch (current_policy) {
        case SCHED_ROUND_ROBIN:
            next = schedule_round_robin();
            break;

        case SCHED_PRIORITY:
            next = schedule_priority();
            break;

        case SCHED_SHORTEST_JOB:
            next = schedule_shortest_job();
            break;

        case SCHED_MULTILEVEL:
            next = schedule_multilevel_feedback();
            break;

        default:
            next = schedule_multilevel_feedback();
            break;
    }

    if (next) {
        sched_stats.active_processes++;
    } else {
        sched_stats.idle_time++;
    }

    return next;
}

// Scheduler tick (called by timer)
void scheduler_tick(void) {
    scheduler_tick_count++;
    sched_stats.total_runtime++;

    process_t* current = process_get_current();
    if (!current) return;

    // Decrement time slice
    if (current->time_slice > 0) {
        current->time_slice--;
    }

    // Check for preemption
    if (current->time_slice == 0) {
        sched_stats.preemptions++;

        // Reset time slice based on priority
        if (current->priority < PRIORITY_LEVELS) {
            current->time_slice = queue_time_slices[current->priority];
        } else {
            current->time_slice = quantum_ticks;
        }

        // Add current process back to appropriate queue
        if (current->state == PROCESS_RUNNING) {
            current->state = PROCESS_READY;
            scheduler_add_to_priority_queue(current);
        }

        // Select and switch to next process
        process_t* next = scheduler_select_next();
        if (next && next != current) {
            kernel_print("[SCHEDULER] Switching: ");
            kernel_print(current->name);
            kernel_print(" -> ");
            kernel_print(next->name);
            kernel_print("\\n");
            process_switch();
        }
    }
}

// Change scheduling policy
void scheduler_set_policy(sched_policy_t policy) {
    kernel_print("[SCHEDULER] Changing policy to: ");

    switch (policy) {
        case SCHED_ROUND_ROBIN:
            kernel_print("Round Robin\\n");
            break;
        case SCHED_PRIORITY:
            kernel_print("Priority-based\\n");
            break;
        case SCHED_SHORTEST_JOB:
            kernel_print("Shortest Job First\\n");
            break;
        case SCHED_MULTILEVEL:
            kernel_print("Multi-level Feedback Queue\\n");
            break;
    }

    current_policy = policy;
    sched_stats.current_policy = policy;
}

// Set time quantum
void scheduler_set_quantum(uint32_t ticks) {
    kernel_print("[SCHEDULER] Setting quantum to ");
    kernel_print_hex(ticks);
    kernel_print(" ticks\\n");

    quantum_ticks = ticks;

    // Update all queue time slices proportionally
    for (int i = 0; i < PRIORITY_LEVELS; i++) {
        queue_time_slices[i] = ticks * (PRIORITY_LEVELS - i);
    }
}

// Display scheduler statistics
void scheduler_show_stats(void) {
    kernel_print("\\n=== SCHEDULER STATISTICS ===\\n");

    kernel_print("Policy: ");
    switch (sched_stats.current_policy) {
        case SCHED_ROUND_ROBIN:
            kernel_print("Round Robin\\n");
            break;
        case SCHED_PRIORITY:
            kernel_print("Priority-based\\n");
            break;
        case SCHED_SHORTEST_JOB:
            kernel_print("Shortest Job First\\n");
            break;
        case SCHED_MULTILEVEL:
            kernel_print("Multi-level Feedback\\n");
            break;
    }

    kernel_print("Context switches: ");
    kernel_print_hex(sched_stats.context_switches);
    kernel_print("\\nPreemptions: ");
    kernel_print_hex(sched_stats.preemptions);
    kernel_print("\\nTotal runtime: ");
    kernel_print_hex(sched_stats.total_runtime);
    kernel_print(" ticks\\nIdle time: ");
    kernel_print_hex(sched_stats.idle_time);
    kernel_print(" ticks\\nActive processes: ");
    kernel_print_hex(sched_stats.active_processes);

    // Calculate CPU utilization
    if (sched_stats.total_runtime > 0) {
        uint32_t utilization = ((sched_stats.total_runtime - sched_stats.idle_time) * 100) / sched_stats.total_runtime;
        kernel_print("\\nCPU Utilization: ");
        kernel_print_hex(utilization);
        kernel_print("%\\n");
    }

    kernel_print("Scheduler ticks: ");
    kernel_print_hex(scheduler_tick_count);
    kernel_print("\\n");
}

// Priority queue status
void scheduler_show_queues(void) {
    kernel_print("\\n=== PRIORITY QUEUE STATUS ===\\n");

    for (int level = 0; level < PRIORITY_LEVELS; level++) {
        kernel_print("Priority ");
        kernel_print_hex(level);
        kernel_print(": ");

        if (priority_queues[level]) {
            process_t* proc = priority_queues[level];
            int count = 0;
            while (proc) {
                count++;
                kernel_print(proc->name);
                if (proc->next) kernel_print(", ");
                proc = proc->next;
            }
            kernel_print(" (");
            kernel_print_hex(count);
            kernel_print(" processes)\\n");
        } else {
            kernel_print("Empty\\n");
        }
    }
}

// Test scheduling algorithms
void scheduler_test_algorithms(void) {
    kernel_print("\\n=== SCHEDULER ALGORITHM TESTS ===\\n");

    // Test each scheduling policy
    sched_policy_t policies[] = {SCHED_ROUND_ROBIN, SCHED_PRIORITY, SCHED_SHORTEST_JOB, SCHED_MULTILEVEL};
    const char* policy_names[] = {"Round Robin", "Priority", "Shortest Job", "Multi-level"};

    for (int i = 0; i < 4; i++) {
        kernel_print("\\nTesting ");
        kernel_print(policy_names[i]);
        kernel_print(" scheduling:\\n");

        scheduler_set_policy(policies[i]);

        // Simulate some scheduling decisions
        for (int j = 0; j < 5; j++) {
            process_t* next = scheduler_select_next();
            if (next) {
                kernel_print("  Selected: ");
                kernel_print(next->name);
                kernel_print("\\n");
            }
        }
    }

    // Reset to multilevel
    scheduler_set_policy(SCHED_MULTILEVEL);
}

// Initialize scheduler management system
void scheduler_management_init(void) {
    kernel_print("\\n=== HOUR 6: BASIC SCHEDULER ===\\n");

    // Initialize scheduler
    scheduler_init();

    // Set default policy
    scheduler_set_policy(SCHED_MULTILEVEL);

    // Set default quantum
    scheduler_set_quantum(10);

    // Test scheduling algorithms
    scheduler_test_algorithms();

    // Show initial statistics
    scheduler_show_stats();
    scheduler_show_queues();

    kernel_print("\\n[SCHEDULER] Advanced scheduler ready!\\n");
    kernel_print("Hour 6 Complete - Multi-level Scheduler Active\\n");
}