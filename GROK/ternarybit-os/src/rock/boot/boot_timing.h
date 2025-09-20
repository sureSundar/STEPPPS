#ifndef TBOS_BOOT_TIMING_H
#define TBOS_BOOT_TIMING_H

#include <stdint.h>

// Boot timing markers
typedef enum {
    BOOT_START,             // System power on
    BOOT_BIOS_ENTRY,        // BIOS hands off to bootloader
    BOOT_FIRST_STAGE,       // First stage bootloader starts
    BOOT_SECOND_STAGE,      // Second stage bootloader starts
    BOOT_HW_INIT_START,     // Hardware initialization begins
    BOOT_HW_INIT_END,       // Hardware initialization complete
    BOOT_MMU_INIT,          // Memory management unit initialized
    BOOT_KERNEL_LOAD,       // Kernel loading begins
    BOOT_KERNEL_ENTRY,      // Kernel entry point
    BOOT_KERNEL_INIT,       // Kernel initialization complete
    BOOT_SYSTEM_READY,      // System fully booted
    BOOT_MARKER_COUNT       // Total number of markers
} boot_marker_t;

// Initialize boot timing system
void boot_timing_init(void);

// Record a boot timing marker
void boot_mark_time(boot_marker_t marker);

// Get the time difference between two markers in microseconds
uint64_t boot_get_duration(boot_marker_t start, boot_marker_t end);

// Print boot timing report
void boot_print_timings(void);

// Get current timestamp in microseconds
static inline uint64_t boot_timestamp(void) {
    uint32_t lo, hi;
    // Use RDTSC for x86, will need different implementations for other archs
    __asm__ volatile (
        "rdtsc"
        : "=a" (lo), "=d" (hi)
    );
    return ((uint64_t)hi << 32) | lo;
}

// Helper macro for easy timing
#define BOOT_MARK(marker) do { \
    if (boot_timing_enabled) boot_mark_time(marker); \
} while(0)

extern bool boot_timing_enabled;

#endif // TBOS_BOOT_TIMING_H
