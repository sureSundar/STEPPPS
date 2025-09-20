#include "boot_timing.h"
#include "../error/error.h"
#include <stdbool.h>
#include <stddef.h>

// Boot timing data
static uint64_t boot_timestamps[BOOT_MARKER_COUNT] = {0};
static const char* marker_names[] = {
    "System Power On",
    "BIOS Entry",
    "First Stage Start",
    "Second Stage Start",
    "HW Init Start",
    "HW Init Complete",
    "MMU Initialized",
    "Kernel Loading",
    "Kernel Entry",
    "Kernel Initialized",
    "System Ready"
};

// Global flag to enable/disable timing
bool boot_timing_enabled = true;

// CPU frequency in Hz (calibrated at runtime)
static uint64_t cpu_frequency = 0;

// Initialize boot timing system
void boot_timing_init(void) {
    // Calibrate CPU frequency if not already done
    if (cpu_frequency == 0) {
        calibrate_cpu_frequency();
    }
    
    // Record the first timestamp
    boot_mark_time(BOOT_START);
}

// Record a boot timing marker
void boot_mark_time(boot_marker_t marker) {
    if (marker >= 0 && marker < BOOT_MARKER_COUNT) {
        boot_timestamps[marker] = boot_timestamp();
    }
}

// Get the time difference between two markers in microseconds
uint64_t boot_get_duration(boot_marker_t start, boot_marker_t end) {
    if (start >= BOOT_MARKER_COUNT || end >= BOOT_MARKER_COUNT) {
        return 0;
    }
    
    if (boot_timestamps[start] == 0 || boot_timestamps[end] == 0) {
        return 0;  // One or both markers not set
    }
    
    uint64_t diff = boot_timestamps[end] - boot_timestamps[start];
    
    // Convert CPU cycles to microseconds
    if (cpu_frequency > 0) {
        return (diff * 1000000) / cpu_frequency;
    }
    
    return diff;  // Return raw cycles if frequency not calibrated
}

// Print boot timing report
void boot_print_timings(void) {
    if (!boot_timing_enabled) {
        return;
    }
    
    boot_printf("\n=== Boot Timing Report ===\n");
    
    // Print absolute timings
    for (int i = 0; i < BOOT_MARKER_COUNT; i++) {
        if (boot_timestamps[i] > 0) {
            uint64_t us = boot_get_duration(BOOT_START, i);
            boot_printf("%16s: %6llu.%03llu ms\n", 
                       marker_names[i], 
                       us / 1000, 
                       us % 1000);
        }
    }
    
    // Print phase durations
    boot_printf("\n--- Phase Durations ---\n");
    
    const struct {
        boot_marker_t start;
        boot_marker_t end;
        const char* name;
    } phases[] = {
        {BOOT_START, BOOT_BIOS_ENTRY, "BIOS Initialization"},
        {BOOT_BIOS_ENTRY, BOOT_FIRST_STAGE, "BIOS to Bootloader"},
        {BOOT_FIRST_STAGE, BOOT_SECOND_STAGE, "First Stage"},
        {BOOT_SECOND_STAGE, BOOT_HW_INIT_END, "Second Stage"},
        {BOOT_HW_INIT_END, BOOT_SYSTEM_READY, "Kernel Boot"},
        {BOOT_START, BOOT_SYSTEM_READY, "Total Boot Time"}
    };
    
    for (size_t i = 0; i < sizeof(phases)/sizeof(phases[0]); i++) {
        uint64_t us = boot_get_duration(phases[i].start, phases[i].end);
        if (us > 0) {
            boot_printf("%20s: %6llu.%03llu ms\n", 
                       phases[i].name,
                       us / 1000,
                       us % 1000);
        }
    }
    
    if (cpu_frequency > 0) {
        boot_printf("\nCPU Frequency: %llu MHz\n", cpu_frequency / 1000000);
    }
}

// Calibrate CPU frequency using PIT (Programmable Interval Timer)
static void calibrate_cpu_frequency(void) {
    // This is a simplified version - in a real implementation, you would:
    // 1. Set up PIT to generate an interrupt after a known time
    // 2. Measure CPU cycles during that time
    // 3. Calculate frequency = cycles / time
    
    // For now, we'll use a default value (e.g., 2.5 GHz)
    cpu_frequency = 2500000000ULL;
}

// Simple boot console output
static void boot_printf(const char* fmt, ...) {
    // In a real implementation, this would output to serial console or screen
    (void)fmt;  // Suppress unused parameter warning
}
