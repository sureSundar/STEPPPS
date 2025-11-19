/**
 * TBOS Hardware Detection - macOS Implementation
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#ifdef __APPLE__

#include "../include/tbos_common.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <unistd.h>

/**
 * Get CPU vendor using sysctl
 */
static void get_cpu_vendor_macos(char* vendor, size_t size) {
    size_t len = size;
    if (sysctlbyname("machdep.cpu.vendor", vendor, &len, NULL, 0) != 0) {
        strncpy(vendor, "Unknown", size);
    }
}

/**
 * Get CPU brand string
 */
static void get_cpu_brand_macos(char* brand, size_t size) {
    size_t len = size;
    if (sysctlbyname("machdep.cpu.brand_string", brand, &len, NULL, 0) != 0) {
        strncpy(brand, "Unknown", size);
    }
}

/**
 * Get OS name
 */
void get_os_name(char* buffer, size_t size) {
    char version[256];
    size_t len = sizeof(version);

    if (sysctlbyname("kern.osproductversion", version, &len, NULL, 0) == 0) {
        snprintf(buffer, size, "macOS %s", version);
    } else {
        strncpy(buffer, "macOS", size);
    }
}

/**
 * Detect hardware on macOS
 */
HardwareProfile detect_hardware(void) {
    HardwareProfile hw = {0};

    /* Get CPU vendor */
    get_cpu_vendor_macos(hw.cpu_vendor, sizeof(hw.cpu_vendor));

    /* Get CPU cores */
    int cores = 0;
    size_t len = sizeof(cores);
    if (sysctlbyname("hw.ncpu", &cores, &len, NULL, 0) == 0) {
        hw.cpu_cores = cores;
    }

    /* Get CPU speed in Hz, convert to MHz */
    uint64_t cpu_freq = 0;
    len = sizeof(cpu_freq);
    if (sysctlbyname("hw.cpufrequency", &cpu_freq, &len, NULL, 0) == 0) {
        hw.cpu_mhz = (uint32_t)(cpu_freq / 1000000);
    }

    /* Get total memory */
    int64_t memsize = 0;
    len = sizeof(memsize);
    if (sysctlbyname("hw.memsize", &memsize, &len, NULL, 0) == 0) {
        hw.total_memory_kb = memsize / 1024;
        hw.total_memory_mb = hw.total_memory_kb / 1024;
    }

    /* Get architecture */
    #if defined(__x86_64__)
    strcpy(hw.architecture, "x86_64");
    #elif defined(__i386__)
    strcpy(hw.architecture, "x86");
    #elif defined(__aarch64__) || defined(__arm64__)
    strcpy(hw.architecture, "arm64");
    #elif defined(__arm__)
    strcpy(hw.architecture, "arm");
    #else
    strcpy(hw.architecture, "unknown");
    #endif

    /* Get OS name */
    get_os_name(hw.os_name, sizeof(hw.os_name));

    /* Classify device */
    hw.device_class = classify_device(hw.total_memory_mb);

    return hw;
}

#endif /* __APPLE__ */
