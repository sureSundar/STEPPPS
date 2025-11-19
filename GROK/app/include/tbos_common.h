/**
 * TBOS Universal Hardware Detector - Common Definitions
 * Works on: Windows, Linux, macOS, and as bootloader
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#ifndef TBOS_COMMON_H
#define TBOS_COMMON_H

#include <stdint.h>
#include <stddef.h>

/* Device classification based on RAM */
typedef enum {
    CALCULATOR     = 0,  /* 512B - 2KB */
    EMBEDDED       = 1,  /* 2KB - 64KB */
    MOBILE         = 2,  /* 64KB - 4MB */
    DESKTOP        = 3,  /* 4MB - 16MB */
    WORKSTATION    = 4,  /* 16MB - 32MB */
    SERVER         = 5,  /* 32MB - 64MB */
    CLUSTER        = 6,  /* 64MB - 128MB */
    SUPERCOMPUTER  = 7   /* 128MB+ */
} DeviceClass;

/* Hardware profile structure */
typedef struct {
    char cpu_vendor[13];      /* CPU vendor string (e.g., "GenuineIntel") */
    uint32_t cpu_cores;       /* Number of CPU cores */
    uint32_t cpu_mhz;         /* CPU speed in MHz */
    uint64_t total_memory_kb; /* Total RAM in KB */
    uint64_t total_memory_mb; /* Total RAM in MB */
    DeviceClass device_class; /* Classified device type */
    char os_name[64];         /* Current OS name */
    char architecture[16];    /* CPU architecture (x86, x86_64, arm, etc.) */
} HardwareProfile;

/* Device class name strings */
static const char* DEVICE_CLASS_NAMES[] = {
    "CALCULATOR (512B-2KB)",
    "EMBEDDED (2KB-64KB)",
    "MOBILE (64KB-4MB)",
    "DESKTOP (4MB-16MB)",
    "WORKSTATION (16MB-32MB)",
    "SERVER (32MB-64MB)",
    "CLUSTER (64MB-128MB)",
    "SUPERCOMPUTER (128MB+)"
};

/* Function prototypes */

/**
 * Detect hardware and populate profile
 * Platform-specific implementations in:
 *   - hardware_linux.c
 *   - hardware_windows.c
 *   - hardware_macos.c
 */
HardwareProfile detect_hardware(void);

/**
 * Classify device based on memory
 * Universal across all platforms
 */
DeviceClass classify_device(uint64_t memory_mb);

/**
 * Print hardware profile in readable format
 */
void print_hardware_profile(const HardwareProfile* hw);

/**
 * Get OS name (platform-specific)
 */
void get_os_name(char* buffer, size_t size);

#endif /* TBOS_COMMON_H */
