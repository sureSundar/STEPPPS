#include "panic.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../error/error.h"

// Current panic handler
static panic_handler_t current_panic_handler = NULL;

// Default panic handler implementation
void default_panic_handler(const panic_info_t *info) {
    // Disable interrupts if they're enabled
    // asm volatile("cli");
    
    // Get the panic reason as a string
    const char *reason_str;
    switch (info->reason) {
        case PANIC_GENERAL:          reason_str = "General failure"; break;
        case PANIC_OUT_OF_MEMORY:    reason_str = "Out of memory"; break;
        case PANIC_DOUBLE_FAULT:     reason_str = "Double fault"; break;
        case PANIC_INVALID_OPCODE:   reason_str = "Invalid opcode"; break;
        case PANIC_DIVIDE_BY_ZERO:   reason_str = "Divide by zero"; break;
        case PANIC_STACK_OVERFLOW:   reason_str = "Stack overflow"; break;
        case PANIC_ASSERTION_FAILED: reason_str = "Assertion failed"; break;
        case PANIC_HARDWARE:         reason_str = "Hardware failure"; break;
        case PANIC_KERNEL_BUG:       reason_str = "Kernel bug"; break;
        case PANIC_SECURITY:         reason_str = "Security violation"; break;
        default:                     reason_str = "Unknown reason";
    }
    
    // Print panic information
    printf("\n\n=== KERNEL PANIC ===\n");
    printf("Reason: %s\n", reason_str);
    printf("Message: %s\n", info->message ? info->message : "None");
    printf("Location: %s:%d\n", info->file, info->line);
    
    if (info->error_code != 0) {
        printf("Error code: 0x%llX\n", (unsigned long long)info->error_code);
    }
    
    if (info->address != NULL) {
        printf("Faulting address: %p\n", info->address);
    }
    
    // Print stack trace if available
    if (info->stack_depth > 0) {
        printf("\nStack trace (depth: %zu):\n", info->stack_depth);
        for (size_t i = 0; i < info->stack_depth; i++) {
            printf("  [%02zu] %p\n", i, info->stack_trace[i]);
        }
    }
    
    printf("\nSystem halted.\n");
    
    // Halt the system
    while (1) {
        // Wait for interrupts (which are disabled) to halt the CPU
        asm volatile("hlt");
    }
}

void panic_init(void) {
    if (current_panic_handler == NULL) {
        current_panic_handler = default_panic_handler;
    }
}

void panic_set_handler(panic_handler_t handler) {
    current_panic_handler = (handler != NULL) ? handler : default_panic_handler;
}

// Architecture-specific function to get the current stack pointer
static void *get_stack_pointer(void) {
    void *sp;
    asm volatile ("mov %%rsp, %0" : "=r"(sp));
    return sp;
}

// Simple stack trace implementation (x86_64 specific)
size_t panic_get_stack_trace(void **buffer, size_t size) {
    if (buffer == NULL || size == 0) {
        return 0;
    }
    
    void **rbp;
    asm volatile ("mov %%rbp, %0" : "=r"(rbp));
    
    size_t depth = 0;
    const size_t max_depth = size < 16 ? size : 16;  // Limit depth for safety
    
    // Walk up the stack
    while (rbp != NULL && depth < max_depth) {
        void *ret_addr = *(rbp + 1);
        if (ret_addr == NULL) {
            break;
        }
        
        buffer[depth++] = ret_addr;
        void **next_rbp = (void **)(*rbp);
        
        // Basic sanity check to prevent infinite loops
        if (next_rbp <= rbp || (uintptr_t)next_rbp > (uintptr_t)get_stack_pointer() + 0x100000) {
            break;
        }
        
        rbp = next_rbp;
    }
    
    return depth;
}

noreturn void panic(panic_reason_t reason, const char *message, 
                   const char *file, int line, uint64_t error_code) {
    panic_info_t info = {
        .reason = reason,
        .message = message,
        .file = file,
        .line = line,
        .error_code = error_code,
        .address = NULL,  // Can be filled by architecture-specific code
        .stack_depth = 0
    };
    
    // Capture stack trace
    info.stack_depth = panic_get_stack_trace(info.stack_trace, 
                                           sizeof(info.stack_trace) / sizeof(info.stack_trace[0]));
    
    // Report the panic as a critical error
    ERROR_CRITICAL(ERROR_DOMAIN_SYSTEM, 0xDEADBEEF, message ? message : "Kernel panic");
    
    // Call the panic handler (this should not return)
    if (current_panic_handler != NULL) {
        current_panic_handler(&info);
    }
    
    // If we get here, the panic handler returned (which it shouldn't)
    while (1) {
        asm volatile("cli; hlt");
    }
}
