// TBOS Process Management - Hour 5 Sprints
// 🕉️ Swamiye Saranam Aiyappa 🕉️
// Process creation, scheduling, and management

#include <stdint.h>
#include <stddef.h>

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);
extern void* kmalloc(size_t size);
extern void kfree(void* ptr);

// Process states
typedef enum {
    PROCESS_CREATED,
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} process_state_t;

// Process structure
typedef struct process {
    uint32_t pid;                    // Process ID
    char name[32];                   // Process name
    process_state_t state;           // Current state
    uint32_t* stack_ptr;             // Stack pointer
    uint32_t stack_base;             // Stack base address
    uint32_t stack_size;             // Stack size
    uint32_t entry_point;            // Entry function
    uint32_t priority;               // Process priority (0-7)
    uint32_t time_slice;             // Time slice remaining
    uint32_t total_time;             // Total CPU time used
    struct process* next;            // Next in linked list
    uint32_t registers[8];           // Saved registers (EAX-EDI)
    uint32_t eip;                    // Instruction pointer
    uint32_t esp;                    // Stack pointer
    uint32_t ebp;                    // Base pointer
    uint32_t eflags;                 // Flags register
} process_t;

// Process table
#define MAX_PROCESSES 64
static process_t process_table[MAX_PROCESSES];
static process_t* current_process = NULL;
static process_t* ready_queue = NULL;
static uint32_t next_pid = 1;
static uint32_t process_count = 0;

// Initialize process management
void process_init(void) {
    kernel_print("[PROCESS] Initializing process management...\n");

    // Clear process table
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = PROCESS_TERMINATED;
        process_table[i].next = NULL;
    }

    current_process = NULL;
    ready_queue = NULL;
    next_pid = 1;
    process_count = 0;

    // Create kernel process (PID 0)
    create_kernel_process();

    kernel_print("[PROCESS] Process management initialized\n");
    kernel_print("  Max processes: ");
    kernel_print_hex(MAX_PROCESSES);
    kernel_print("\n  Kernel process: PID 0 created\n");
}

// Create kernel process
void create_kernel_process(void) {
    process_t* kernel_proc = &process_table[0];

    kernel_proc->pid = 0;
    kernel_proc->name[0] = 'k'; kernel_proc->name[1] = 'e'; kernel_proc->name[2] = 'r';
    kernel_proc->name[3] = 'n'; kernel_proc->name[4] = 'e'; kernel_proc->name[5] = 'l';
    kernel_proc->name[6] = '\0';
    kernel_proc->state = PROCESS_RUNNING;
    kernel_proc->priority = 0;  // Highest priority
    kernel_proc->time_slice = 100;
    kernel_proc->total_time = 0;
    kernel_proc->next = NULL;

    current_process = kernel_proc;
    process_count = 1;
}

// Create a new process
uint32_t process_create(const char* name, uint32_t entry_point, uint32_t priority) {
    // Find free slot
    int slot = -1;
    for (int i = 1; i < MAX_PROCESSES; i++) {  // Skip slot 0 (kernel)
        if (process_table[i].pid == 0) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        kernel_print("[PROCESS] ERROR: No free process slots\n");
        return 0;
    }

    process_t* proc = &process_table[slot];

    // Initialize process
    proc->pid = next_pid++;

    // Copy name
    int i;
    for (i = 0; i < 31 && name[i]; i++) {
        proc->name[i] = name[i];
    }
    proc->name[i] = '\0';

    proc->state = PROCESS_CREATED;
    proc->entry_point = entry_point;
    proc->priority = priority & 0x7;  // 0-7 range
    proc->time_slice = 10 + (priority * 5);  // Higher priority = more time
    proc->total_time = 0;

    // Allocate stack (4KB)
    proc->stack_size = 4096;
    proc->stack_base = (uint32_t)kmalloc(proc->stack_size);
    if (!proc->stack_base) {
        kernel_print("[PROCESS] ERROR: Cannot allocate stack\n");
        proc->pid = 0;  // Mark as free
        return 0;
    }

    proc->stack_ptr = (uint32_t*)(proc->stack_base + proc->stack_size - 4);
    proc->esp = (uint32_t)proc->stack_ptr;
    proc->ebp = proc->esp;
    proc->eip = entry_point;
    proc->eflags = 0x202;  // IF flag set

    // Clear registers
    for (int j = 0; j < 8; j++) {
        proc->registers[j] = 0;
    }

    proc->next = NULL;
    process_count++;

    kernel_print("[PROCESS] Created process: ");
    kernel_print(name);
    kernel_print(" PID=");
    kernel_print_hex(proc->pid);
    kernel_print(" Priority=");
    kernel_print_hex(priority);
    kernel_print("\n");

    // Add to ready queue
    process_set_ready(proc);

    return proc->pid;
}

// Set process to ready state
void process_set_ready(process_t* proc) {
    if (!proc || proc->state == PROCESS_TERMINATED) return;

    proc->state = PROCESS_READY;

    // Add to ready queue (simple FIFO for now)
    if (!ready_queue) {
        ready_queue = proc;
        proc->next = NULL;
    } else {
        process_t* current = ready_queue;
        while (current->next) {
            current = current->next;
        }
        current->next = proc;
        proc->next = NULL;
    }
}

// Get next ready process
process_t* process_get_next_ready(void) {
    if (!ready_queue) return NULL;

    process_t* next = ready_queue;
    ready_queue = ready_queue->next;
    next->next = NULL;

    return next;
}

// Switch to next process (basic round-robin)
void process_switch(void) {
    if (!current_process) return;

    // Save current process state (simplified)
    if (current_process->state == PROCESS_RUNNING) {
        current_process->state = PROCESS_READY;
        process_set_ready(current_process);
    }

    // Get next process
    process_t* next = process_get_next_ready();
    if (!next) {
        // No ready processes, continue with current
        return;
    }

    // Switch context
    process_t* old_process = current_process;
    current_process = next;
    current_process->state = PROCESS_RUNNING;

    kernel_print("[PROCESS] Context switch: ");
    kernel_print(old_process->name);
    kernel_print(" -> ");
    kernel_print(current_process->name);
    kernel_print("\n");

    // Actual context switch would happen here in assembly
    // For now, just simulate the switch
}

// Terminate a process
void process_terminate(uint32_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            process_t* proc = &process_table[i];

            kernel_print("[PROCESS] Terminating process: ");
            kernel_print(proc->name);
            kernel_print(" PID=");
            kernel_print_hex(pid);
            kernel_print("\n");

            // Free stack
            if (proc->stack_base) {
                kfree((void*)proc->stack_base);
            }

            // Mark as terminated
            proc->state = PROCESS_TERMINATED;
            proc->pid = 0;
            process_count--;

            // If this was current process, switch
            if (current_process == proc) {
                current_process = NULL;
                process_switch();
            }

            break;
        }
    }
}

// List all processes
void process_list(void) {
    kernel_print("\n=== PROCESS LIST ===\n");
    kernel_print("PID  Name           State    Priority  Time\n");
    kernel_print("--------------------------------------------\n");

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != 0) {
            process_t* proc = &process_table[i];

            kernel_print_hex(proc->pid);
            kernel_print("   ");
            kernel_print(proc->name);

            // Pad name to 15 chars
            int len = 0;
            while (proc->name[len]) len++;
            for (int j = len; j < 15; j++) {
                kernel_print(" ");
            }

            // State
            const char* state_names[] = {"CREATED", "READY", "RUNNING", "BLOCKED", "TERMINATED"};
            kernel_print(state_names[proc->state]);
            kernel_print("   ");

            kernel_print_hex(proc->priority);
            kernel_print("        ");
            kernel_print_hex(proc->total_time);
            kernel_print("\n");
        }
    }

    kernel_print("\nTotal processes: ");
    kernel_print_hex(process_count);
    kernel_print("\nCurrent process: ");
    if (current_process) {
        kernel_print(current_process->name);
    } else {
        kernel_print("None");
    }
    kernel_print("\n");
}

// Get current process
process_t* process_get_current(void) {
    return current_process;
}

// Process scheduler tick (called by timer)
void process_tick(void) {
    if (!current_process) return;

    current_process->total_time++;

    // Decrement time slice
    if (current_process->time_slice > 0) {
        current_process->time_slice--;
    }

    // Time slice expired?
    if (current_process->time_slice == 0) {
        // Reset time slice
        current_process->time_slice = 10 + (current_process->priority * 5);

        // Switch to next process
        process_switch();
    }
}

// Create some test processes
void process_create_test_processes(void) {
    kernel_print("[PROCESS] Creating test processes...\n");

    // Create STEPPPS service
    process_create("steppps-svc", 0x200000, 2);

    // Create filesystem service
    process_create("pxfs-daemon", 0x201000, 3);

    // Create shell
    process_create("tbos-shell", 0x202000, 4);

    // Create consciousness module
    process_create("consciousness", 0x203000, 1);

    kernel_print("[PROCESS] Test processes created\n");
}

// Initialize process management system
void process_management_init(void) {
    kernel_print("\n=== HOUR 5: PROCESS MANAGEMENT ===\n");

    // Initialize process system
    process_init();

    // Create test processes
    process_create_test_processes();

    // Show process list
    process_list();

    kernel_print("\n[PROCESS] Process management ready!\n");
    kernel_print("Hour 5 Complete - Process Management Active\n");
}