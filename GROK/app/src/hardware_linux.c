/**
 * TBOS Hardware Detection - Linux Implementation
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#include "../include/tbos_common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

/**
 * Read CPU vendor from /proc/cpuinfo
 */
static void get_cpu_vendor(char* vendor, size_t size) {
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        strncpy(vendor, "Unknown", size);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "vendor_id", 9) == 0) {
            char* colon = strchr(line, ':');
            if (colon) {
                colon++;
                while (*colon == ' ' || *colon == '\t') colon++;

                /* Remove newline */
                char* newline = strchr(colon, '\n');
                if (newline) *newline = '\0';

                strncpy(vendor, colon, size - 1);
                vendor[size - 1] = '\0';
                fclose(fp);
                return;
            }
        }
    }

    strncpy(vendor, "Unknown", size);
    fclose(fp);
}

/**
 * Read CPU speed from /proc/cpuinfo
 */
static uint32_t get_cpu_mhz(void) {
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "cpu MHz", 7) == 0) {
            char* colon = strchr(line, ':');
            if (colon) {
                colon++;
                float mhz;
                sscanf(colon, "%f", &mhz);
                fclose(fp);
                return (uint32_t)mhz;
            }
        }
    }

    fclose(fp);
    return 0;
}

/**
 * Get OS name
 */
void get_os_name(char* buffer, size_t size) {
    struct utsname uts;
    if (uname(&uts) == 0) {
        snprintf(buffer, size, "%s %s", uts.sysname, uts.release);
    } else {
        strncpy(buffer, "Linux", size);
    }
}

/**
 * Detect hardware on Linux
 */
HardwareProfile detect_hardware(void) {
    HardwareProfile hw = {0};

    /* Get CPU vendor */
    get_cpu_vendor(hw.cpu_vendor, sizeof(hw.cpu_vendor));

    /* Get CPU cores */
    hw.cpu_cores = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);

    /* Get CPU speed */
    hw.cpu_mhz = get_cpu_mhz();

    /* Get total memory */
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        hw.total_memory_kb = (si.totalram * si.mem_unit) / 1024;
        hw.total_memory_mb = hw.total_memory_kb / 1024;
    }

    /* Get architecture */
    struct utsname uts;
    if (uname(&uts) == 0) {
        strncpy(hw.architecture, uts.machine, sizeof(hw.architecture) - 1);
    } else {
        strcpy(hw.architecture, "unknown");
    }

    /* Get OS name */
    get_os_name(hw.os_name, sizeof(hw.os_name));

    /* Classify device */
    hw.device_class = classify_device(hw.total_memory_mb);

    return hw;
}
