/**
 * TBOS Hardware Detection - iOS Implementation
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#if defined(__APPLE__) && (defined(TARGET_OS_IPHONE) || defined(__IPHONE_OS_VERSION_MIN_REQUIRED))

#include "../include/tbos_common.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>

/**
 * Get device model
 */
static void get_device_model_ios(char* model, size_t size) {
    size_t len = size;
    if (sysctlbyname("hw.machine", model, &len, NULL, 0) != 0) {
        strncpy(model, "iPhone", size);
    }
}

/**
 * Get OS version
 */
void get_os_name(char* buffer, size_t size) {
    size_t len = size;
    char version[64];

    if (sysctlbyname("kern.osproductversion", version, &len, NULL, 0) == 0) {
        snprintf(buffer, size, "iOS %s", version);
    } else {
        strncpy(buffer, "iOS", size);
    }
}

/**
 * Get physical memory size
 */
static uint64_t get_physical_memory_ios(void) {
    uint64_t memsize = 0;
    size_t len = sizeof(memsize);

    if (sysctlbyname("hw.memsize", &memsize, &len, NULL, 0) == 0) {
        return memsize;
    }

    /* Fallback: Try getting from mach */
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vm_stat;

    if (host_statistics64(mach_host_self(), HOST_VM_INFO64,
                          (host_info64_t)&vm_stat, &count) == KERN_SUCCESS) {
        /* Estimate from page info */
        natural_t page_size;
        host_page_size(mach_host_self(), &page_size);
        return (vm_stat.free_count + vm_stat.active_count +
                vm_stat.inactive_count + vm_stat.wire_count) * page_size;
    }

    return 0;
}

/**
 * Get CPU count
 */
static int get_cpu_count_ios(void) {
    int cpu_count = 0;
    size_t len = sizeof(cpu_count);

    if (sysctlbyname("hw.ncpu", &cpu_count, &len, NULL, 0) == 0) {
        return cpu_count;
    }

    /* Fallback */
    return 2; /* Minimum for modern iOS devices */
}

/**
 * Detect hardware on iOS
 */
HardwareProfile detect_hardware(void) {
    HardwareProfile hw = {0};

    /* Get device model */
    char model[64];
    get_device_model_ios(model, sizeof(model));

    /* Determine CPU vendor based on device */
    if (strstr(model, "iPhone") || strstr(model, "iPad")) {
        strcpy(hw.cpu_vendor, "Apple");
    } else {
        strcpy(hw.cpu_vendor, "ARM");
    }

    /* Get CPU cores */
    hw.cpu_cores = get_cpu_count_ios();

    /* Get CPU speed (estimated for iOS devices) */
    /* iOS doesn't expose this directly, so we estimate */
    hw.cpu_mhz = 2400; /* Typical for modern iPhones */

    /* Get total memory */
    uint64_t memsize = get_physical_memory_ios();
    if (memsize > 0) {
        hw.total_memory_kb = memsize / 1024;
        hw.total_memory_mb = hw.total_memory_kb / 1024;
    }

    /* Set architecture */
    #if defined(__arm64__) || defined(__aarch64__)
    strcpy(hw.architecture, "arm64");
    #elif defined(__arm__)
    strcpy(hw.architecture, "armv7");
    #else
    strcpy(hw.architecture, "arm");
    #endif

    /* Get OS name */
    get_os_name(hw.os_name, sizeof(hw.os_name));

    /* Classify device */
    hw.device_class = classify_device(hw.total_memory_mb);

    return hw;
}

#endif /* iOS */
