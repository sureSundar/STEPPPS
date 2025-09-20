#ifndef TBOS_ROCK_PANIC_H
#define TBOS_ROCK_PANIC_H

#include <stdint.h>
#include <stdnoreturn.h>

// Panic reasons
typedef enum {
    PANIC_GENERAL,          // General panic
    PANIC_OUT_OF_MEMORY,    // Out of memory
    PANIC_DOUBLE_FAULT,     // Double fault
    PANIC_INVALID_OPCODE,   // Invalid CPU instruction
    PANIC_DIVIDE_BY_ZERO,   // Division by zero
    PANIC_STACK_OVERFLOW,   // Stack overflow
    PANIC_ASSERTION_FAILED, // Assertion failed
    PANIC_HARDWARE,         // Hardware failure
    PANIC_KERNEL_BUG,       // Kernel bug detected
    PANIC_SECURITY          // Security violation
} panic_reason_t;

// Panic information structure
typedef struct {
    panic_reason_t reason;  // Reason for panic
    const char *message;    // Panic message
    const char *file;       // Source file where panic occurred
    int line;              // Line number where panic occurred
    uint64_t error_code;   // CPU error code (if any)
    void *address;         // Faulting address (if any)
    void *stack_trace[16];  // Stack trace
    size_t stack_depth;    // Depth of stack trace
} panic_info_t;

// Panic handler type
typedef void (*panic_handler_t)(const panic_info_t *info);

// Initialize panic handler
void panic_init(void);

// Set custom panic handler
void panic_set_handler(panic_handler_t handler);

// Trigger a kernel panic
noreturn void panic(panic_reason_t reason, const char *message, 
                   const char *file, int line, uint64_t error_code);

// Macro for triggering panics
#define PANIC(reason, message) panic((reason), (message), __FILE__, __LINE__, 0)

// Assertion macro that triggers panic on failure
#define PANIC_ASSERT(expr, message) \
    do { \
        if (!(expr)) { \
            panic(PANIC_ASSERTION_FAILED, (message), __FILE__, __LINE__, 0); \
        } \
    } while(0)

// Get current stack trace
size_t panic_get_stack_trace(void **buffer, size_t size);

// Default panic handler (implemented in platform-specific code)
void default_panic_handler(const panic_info_t *info);

#endif // TBOS_ROCK_PANIC_H
