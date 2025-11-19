/**
 * TBOS Hardware Detection - Windows Implementation
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#ifdef _WIN32

#include "../include/tbos_common.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>

/**
 * Get CPU vendor using CPUID
 */
static void get_cpu_vendor_windows(char* vendor, size_t size) {
    int cpu_info[4] = {0};

    #ifdef _MSC_VER
    __cpuid(cpu_info, 0);
    #elif defined(__GNUC__)
    __asm__ __volatile__(
        "cpuid"
        : "=a" (cpu_info[0]), "=b" (cpu_info[1]), "=c" (cpu_info[2]), "=d" (cpu_info[3])
        : "a" (0)
    );
    #endif

    /* Vendor string is in EBX, EDX, ECX */
    memcpy(vendor, &cpu_info[1], 4);
    memcpy(vendor + 4, &cpu_info[3], 4);
    memcpy(vendor + 8, &cpu_info[2], 4);
    vendor[12] = '\0';
}

/**
 * Get OS name
 */
void get_os_name(char* buffer, size_t size) {
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    #pragma warning(push)
    #pragma warning(disable: 4996)
    if (GetVersionEx((OSVERSIONINFO*)&osvi)) {
        snprintf(buffer, size, "Windows %lu.%lu Build %lu",
                 osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
    } else {
        strncpy(buffer, "Windows", size);
    }
    #pragma warning(pop)
}

/**
 * Detect hardware on Windows
 */
HardwareProfile detect_hardware(void) {
    HardwareProfile hw = {0};

    /* Get CPU vendor */
    get_cpu_vendor_windows(hw.cpu_vendor, sizeof(hw.cpu_vendor));

    /* Get CPU cores */
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    hw.cpu_cores = si.dwNumberOfProcessors;

    /* Get CPU speed from registry */
    HKEY hKey;
    DWORD cpu_mhz = 0;
    DWORD dataSize = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&cpu_mhz, &dataSize);
        RegCloseKey(hKey);
        hw.cpu_mhz = cpu_mhz;
    }

    /* Get total memory */
    MEMORYSTATUSEX ms;
    ms.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&ms)) {
        hw.total_memory_kb = ms.ullTotalPhys / 1024;
        hw.total_memory_mb = hw.total_memory_kb / 1024;
    }

    /* Get architecture */
    #if defined(_M_X64) || defined(__x86_64__)
    strcpy(hw.architecture, "x86_64");
    #elif defined(_M_IX86) || defined(__i386__)
    strcpy(hw.architecture, "x86");
    #elif defined(_M_ARM64) || defined(__aarch64__)
    strcpy(hw.architecture, "arm64");
    #elif defined(_M_ARM) || defined(__arm__)
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

#endif /* _WIN32 */
