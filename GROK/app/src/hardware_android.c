/**
 * TBOS Hardware Detection - Android Implementation (NDK)
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#ifdef __ANDROID__

#include "../include/tbos_common.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>

/**
 * Read value from /proc/cpuinfo
 */
static int read_cpuinfo_field(const char* field, char* buffer, size_t size) {
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return 0;

    char line[256];
    size_t field_len = strlen(field);

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, field, field_len) == 0) {
            char* colon = strchr(line, ':');
            if (colon) {
                colon++;
                while (*colon == ' ' || *colon == '\t') colon++;

                char* newline = strchr(colon, '\n');
                if (newline) *newline = '\0';

                strncpy(buffer, colon, size - 1);
                buffer[size - 1] = '\0';
                fclose(fp);
                return 1;
            }
        }
    }

    fclose(fp);
    return 0;
}

/**
 * Get Android version from system property
 */
static void get_android_version(char* buffer, size_t size) {
    /* Try to read from build.prop */
    FILE* fp = fopen("/system/build.prop", "r");
    if (!fp) {
        strncpy(buffer, "Android", size);
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "ro.build.version.release=", 25) == 0) {
            char* value = line + 25;
            char* newline = strchr(value, '\n');
            if (newline) *newline = '\0';

            snprintf(buffer, size, "Android %s", value);
            fclose(fp);
            return;
        }
    }

    fclose(fp);
    strncpy(buffer, "Android", size);
}

/**
 * Get OS name
 */
void get_os_name(char* buffer, size_t size) {
    get_android_version(buffer, size);
}

/**
 * Detect hardware on Android
 */
HardwareProfile detect_hardware(void) {
    HardwareProfile hw = {0};

    /* Get CPU vendor/hardware */
    char hardware[64] = {0};
    if (read_cpuinfo_field("Hardware", hardware, sizeof(hardware))) {
        /* Parse hardware string to determine vendor */
        if (strstr(hardware, "Qualcomm") || strstr(hardware, "Snapdragon")) {
            strcpy(hw.cpu_vendor, "Qualcomm");
        } else if (strstr(hardware, "Exynos") || strstr(hardware, "Samsung")) {
            strcpy(hw.cpu_vendor, "Samsung");
        } else if (strstr(hardware, "MediaTek") || strstr(hardware, "MTK")) {
            strcpy(hw.cpu_vendor, "MediaTek");
        } else if (strstr(hardware, "Kirin")) {
            strcpy(hw.cpu_vendor, "HiSilicon");
        } else {
            strncpy(hw.cpu_vendor, hardware, sizeof(hw.cpu_vendor) - 1);
        }
    } else {
        strcpy(hw.cpu_vendor, "ARM");
    }

    /* Get CPU cores */
    hw.cpu_cores = (uint32_t)sysconf(_SC_NPROCESSORS_ONLN);

    /* Get CPU speed */
    char cpu_mhz_str[64] = {0};
    if (read_cpuinfo_field("cpu MHz", cpu_mhz_str, sizeof(cpu_mhz_str))) {
        float mhz;
        if (sscanf(cpu_mhz_str, "%f", &mhz) == 1) {
            hw.cpu_mhz = (uint32_t)mhz;
        }
    }

    /* If CPU MHz not found, try reading max frequency */
    if (hw.cpu_mhz == 0) {
        FILE* fp = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
        if (fp) {
            unsigned long khz;
            if (fscanf(fp, "%lu", &khz) == 1) {
                hw.cpu_mhz = khz / 1000;
            }
            fclose(fp);
        }
    }

    /* Get total memory */
    struct sysinfo si;
    if (sysinfo(&si) == 0) {
        hw.total_memory_kb = (si.totalram * si.mem_unit) / 1024;
        hw.total_memory_mb = hw.total_memory_kb / 1024;
    }

    /* Set architecture */
    #if defined(__aarch64__) || defined(__arm64__)
    strcpy(hw.architecture, "arm64");
    #elif defined(__arm__)
    strcpy(hw.architecture, "armv7");
    #elif defined(__i386__)
    strcpy(hw.architecture, "x86");
    #elif defined(__x86_64__)
    strcpy(hw.architecture, "x86_64");
    #else
    strcpy(hw.architecture, "unknown");
    #endif

    /* Get OS name */
    get_os_name(hw.os_name, sizeof(hw.os_name));

    /* Classify device */
    hw.device_class = classify_device(hw.total_memory_mb);

    return hw;
}

#endif /* __ANDROID__ */
