/**
 * @file tbos_process.h
 * @brief TernaryBit OS Real Process Management System
 *
 * Implements genuine process management with real task scheduling,
 * memory allocation, and inter-process communication.
 *
 * @version 1.0
 * @date 2025-01-XX
 * @author TernaryBit Development Team
 */

#ifndef TBOS_PROCESS_H
#define TBOS_PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <signal.h>

// Process limits
#define TBOS_MAX_PROCESSES 1024
#define TBOS_MAX_THREADS 4096
#define TBOS_MAX_OPEN_FILES 1024
#define TBOS_MAX_PROCESS_NAME 256
#define TBOS_STACK_SIZE (64 * 1024)  // 64KB default stack
#define TBOS_DEFAULT_PRIORITY 20

// Process states
typedef enum {
    PROCESS_STATE_CREATED = 0,
    PROCESS_STATE_READY = 1,
    PROCESS_STATE_RUNNING = 2,
    PROCESS_STATE_BLOCKED = 3,
    PROCESS_STATE_SLEEPING = 4,
    PROCESS_STATE_ZOMBIE = 5,
    PROCESS_STATE_TERMINATED = 6
} tbos_process_state_t;

// Process priority levels
typedef enum {
    PRIORITY_REAL_TIME = 0,     // Real-time processes
    PRIORITY_HIGH = 10,         // High priority
    PRIORITY_NORMAL = 20,       // Normal priority
    PRIORITY_LOW = 30,          // Low priority
    PRIORITY_IDLE = 39          // Idle priority
} tbos_priority_t;

// CPU registers context (x86_64)
typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip;               // Instruction pointer
    uint64_t rflags;            // Flags register
    uint16_t cs, ds, es, fs, gs, ss;  // Segment registers
} tbos_cpu_context_t;

// Memory mapping
typedef struct {
    uint64_t virtual_addr;
    uint64_t physical_addr;
    size_t size;
    uint32_t permissions;  // Read/Write/Execute
    struct tbos_memory_map* next;
} tbos_memory_map_t;

// Process file descriptor table
typedef struct {
    int fd;
    void* file_handle;
    uint32_t flags;
    bool in_use;
} tbos_process_fd_t;

// Thread control block
typedef struct {
    uint32_t thread_id;
    uint32_t process_id;
    tbos_cpu_context_t context;
    void* stack_base;
    size_t stack_size;
    tbos_process_state_t state;
    int priority;
    uint64_t cpu_time_used;
    uint64_t creation_time;
    struct tbos_thread* next;
} tbos_thread_t;

// Process control block
typedef struct {
    uint32_t process_id;
    uint32_t parent_process_id;
    char name[TBOS_MAX_PROCESS_NAME];

    tbos_process_state_t state;
    int priority;
    int nice_value;

    // Timing information
    uint64_t creation_time;
    uint64_t start_time;
    uint64_t cpu_time_used;
    uint64_t last_scheduled_time;

    // Memory management
    uint64_t virtual_memory_size;
    uint64_t resident_memory_size;
    tbos_memory_map_t* memory_maps;
    void* page_directory;

    // File management
    tbos_process_fd_t file_descriptors[TBOS_MAX_OPEN_FILES];
    char current_working_directory[4096];

    // Thread management
    tbos_thread_t* main_thread;
    tbos_thread_t* threads;
    uint32_t thread_count;

    // Parent/child relationships
    struct tbos_process* parent;
    struct tbos_process* children;
    struct tbos_process* siblings;

    // IPC
    void* message_queue;
    void* shared_memory;

    // Exit status
    int exit_status;
    bool has_exited;

    // Security
    uint32_t uid;
    uint32_t gid;
    uint32_t effective_uid;
    uint32_t effective_gid;

    // Linked list for process table
    struct tbos_process* next;
    struct tbos_process* prev;
} tbos_process_t;

// Scheduler context
typedef struct {
    tbos_process_t* current_process;
    tbos_thread_t* current_thread;
    tbos_process_t* ready_queue[40];  // Priority queues
    tbos_process_t* blocked_queue;
    tbos_process_t* zombie_queue;

    uint32_t total_processes;
    uint32_t running_processes;
    uint64_t context_switches;
    uint64_t scheduler_ticks;

    // Scheduling algorithm parameters
    uint32_t time_slice_ms;
    uint32_t aging_factor;
    bool preemptive;
} tbos_scheduler_t;

// Process statistics
typedef struct {
    uint32_t total_processes_created;
    uint32_t active_processes;
    uint32_t zombie_processes;
    uint64_t total_context_switches;
    uint64_t total_cpu_time;
    double average_cpu_utilization;
} tbos_process_stats_t;

// Function prototypes

/**
 * @brief Initialize process management system
 * @return 0 on success, -1 on error
 */
int tbos_process_init(void);

/**
 * @brief Create new process
 * @param name Process name
 * @param entry_point Process entry point function
 * @param args Arguments to pass to process
 * @param priority Process priority
 * @return Process ID on success, -1 on error
 */
pid_t tbos_process_create(const char* name, void (*entry_point)(void*), void* args, int priority);

/**
 * @brief Create new thread in process
 * @param process_id Target process ID
 * @param entry_point Thread entry point
 * @param args Thread arguments
 * @param stack_size Stack size for thread
 * @return Thread ID on success, -1 on error
 */
uint32_t tbos_thread_create(pid_t process_id, void (*entry_point)(void*), void* args, size_t stack_size);

/**
 * @brief Terminate process
 * @param process_id Process to terminate
 * @param exit_status Exit status
 * @return 0 on success, -1 on error
 */
int tbos_process_terminate(pid_t process_id, int exit_status);

/**
 * @brief Send signal to process
 * @param process_id Target process
 * @param signal Signal to send
 * @return 0 on success, -1 on error
 */
int tbos_process_kill(pid_t process_id, int signal);

/**
 * @brief Wait for child process to terminate
 * @param process_id Child process ID (0 for any child)
 * @param status Pointer to store exit status
 * @param options Wait options
 * @return Child PID on success, -1 on error
 */
pid_t tbos_process_wait(pid_t process_id, int* status, int options);

/**
 * @brief Get process information
 * @param process_id Process ID
 * @return Process control block or NULL if not found
 */
tbos_process_t* tbos_process_get(pid_t process_id);

/**
 * @brief List all processes
 * @param processes Array to store process information
 * @param max_processes Maximum number of processes to return
 * @return Number of processes returned
 */
int tbos_process_list(tbos_process_t** processes, int max_processes);

/**
 * @brief Get current process ID
 * @return Current process ID
 */
pid_t tbos_process_getpid(void);

/**
 * @brief Get parent process ID
 * @return Parent process ID
 */
pid_t tbos_process_getppid(void);

/**
 * @brief Change process priority
 * @param process_id Process ID
 * @param priority New priority
 * @return 0 on success, -1 on error
 */
int tbos_process_set_priority(pid_t process_id, int priority);

/**
 * @brief Get process priority
 * @param process_id Process ID
 * @return Priority value or -1 on error
 */
int tbos_process_get_priority(pid_t process_id);

// Scheduler functions

/**
 * @brief Initialize scheduler
 * @param time_slice_ms Time slice in milliseconds
 * @param preemptive Enable preemptive scheduling
 * @return 0 on success, -1 on error
 */
int tbos_scheduler_init(uint32_t time_slice_ms, bool preemptive);

/**
 * @brief Schedule next process to run
 * @return Next process to run or NULL
 */
tbos_process_t* tbos_scheduler_schedule(void);

/**
 * @brief Add process to ready queue
 * @param process Process to add
 */
void tbos_scheduler_add_ready(tbos_process_t* process);

/**
 * @brief Remove process from ready queue
 * @param process Process to remove
 */
void tbos_scheduler_remove_ready(tbos_process_t* process);

/**
 * @brief Block process (move to blocked queue)
 * @param process Process to block
 * @param reason Reason for blocking
 */
void tbos_scheduler_block(tbos_process_t* process, const char* reason);

/**
 * @brief Unblock process (move to ready queue)
 * @param process Process to unblock
 */
void tbos_scheduler_unblock(tbos_process_t* process);

/**
 * @brief Perform context switch
 * @param from_process Source process
 * @param to_process Target process
 */
void tbos_scheduler_context_switch(tbos_process_t* from_process, tbos_process_t* to_process);

/**
 * @brief Handle timer interrupt for scheduling
 */
void tbos_scheduler_timer_interrupt(void);

/**
 * @brief Yield CPU to other processes
 */
void tbos_scheduler_yield(void);

// Memory management for processes

/**
 * @brief Allocate virtual memory for process
 * @param process Process to allocate for
 * @param size Size to allocate
 * @param permissions Memory permissions
 * @return Virtual address or NULL on error
 */
void* tbos_process_alloc_memory(tbos_process_t* process, size_t size, uint32_t permissions);

/**
 * @brief Free virtual memory for process
 * @param process Process to free for
 * @param address Virtual address to free
 * @param size Size to free
 * @return 0 on success, -1 on error
 */
int tbos_process_free_memory(tbos_process_t* process, void* address, size_t size);

/**
 * @brief Map physical memory to virtual address
 * @param process Process to map for
 * @param virtual_addr Virtual address
 * @param physical_addr Physical address
 * @param size Size to map
 * @param permissions Memory permissions
 * @return 0 on success, -1 on error
 */
int tbos_process_map_memory(tbos_process_t* process, void* virtual_addr, void* physical_addr,
                           size_t size, uint32_t permissions);

// IPC functions

/**
 * @brief Send message to process
 * @param to_process_id Target process ID
 * @param message Message data
 * @param size Message size
 * @return 0 on success, -1 on error
 */
int tbos_process_send_message(pid_t to_process_id, const void* message, size_t size);

/**
 * @brief Receive message from process
 * @param from_process_id Source process ID (0 for any)
 * @param message Buffer for message
 * @param max_size Maximum message size
 * @return Message size or -1 on error
 */
ssize_t tbos_process_receive_message(pid_t from_process_id, void* message, size_t max_size);

// Statistics and monitoring

/**
 * @brief Get process statistics
 * @param stats Pointer to store statistics
 */
void tbos_process_get_stats(tbos_process_stats_t* stats);

/**
 * @brief Get CPU usage for process
 * @param process_id Process ID
 * @return CPU usage percentage
 */
double tbos_process_get_cpu_usage(pid_t process_id);

/**
 * @brief Get memory usage for process
 * @param process_id Process ID
 * @return Memory usage in bytes
 */
size_t tbos_process_get_memory_usage(pid_t process_id);

// Cleanup
void tbos_process_cleanup(void);

// Global scheduler instance
extern tbos_scheduler_t* g_tbos_scheduler;

#endif // TBOS_PROCESS_H