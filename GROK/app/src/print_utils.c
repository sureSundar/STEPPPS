/**
 * TBOS Printing Utilities
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#include "../include/tbos_common.h"
#include <stdio.h>

/**
 * Print hardware profile in readable format
 */
void print_hardware_profile(const HardwareProfile* hw) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     TBOS Universal Hardware Detection Report            ║\n");
    printf("║         🕉️ Swamiye Saranam Aiyappa 🕉️                   ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf("=== SYSTEM INFORMATION ===\n");
    printf("  Operating System: %s\n", hw->os_name);
    printf("  Architecture: %s\n", hw->architecture);
    printf("\n");

    printf("=== CPU INFORMATION ===\n");
    printf("  Vendor: %s\n", hw->cpu_vendor);
    printf("  Cores: %u\n", hw->cpu_cores);
    if (hw->cpu_mhz > 0) {
        printf("  Speed: %u MHz (%.2f GHz)\n", hw->cpu_mhz, hw->cpu_mhz / 1000.0);
    }
    printf("\n");

    printf("=== MEMORY INFORMATION ===\n");
    printf("  Total RAM: %lu KB\n", hw->total_memory_kb);
    printf("  Total RAM: %lu MB\n", hw->total_memory_mb);
    printf("  Total RAM: %.2f GB\n", hw->total_memory_mb / 1024.0);
    printf("\n");

    printf("=== DEVICE CLASSIFICATION ===\n");
    printf("  Device Class: %s\n", DEVICE_CLASS_NAMES[hw->device_class]);
    printf("  Class ID: %d\n", hw->device_class);
    printf("\n");

    /* Recommend OS based on class and current platform */
    printf("=== HARDWARE ASSESSMENT ===\n");

    /* Check if running on Windows */
    int on_windows = 0;
    #ifdef _WIN32
    on_windows = 1;
    #endif

    if (on_windows) {
        printf("  Current OS: Windows (detected)\n");
        printf("  Hardware Class: %s\n", DEVICE_CLASS_NAMES[hw->device_class]);
        printf("\n");
        printf("  Your Windows %lu hardware is classified as: %s\n",
               hw->total_memory_mb / 1024, DEVICE_CLASS_NAMES[hw->device_class]);
        printf("\n");
        printf("  Options for trying Linux:\n");
        printf("  1. Dual Boot: Install Linux alongside Windows\n");
        printf("  2. Virtual Machine: Use VirtualBox/VMware\n");
        printf("  3. WSL2: Windows Subsystem for Linux\n");
        printf("  4. Live USB: Try without installing\n");
        printf("\n");

        switch (hw->device_class) {
            case DESKTOP:
            case WORKSTATION:
                printf("  Recommended Linux distributions for your hardware:\n");
                printf("  - Ubuntu Desktop (beginner-friendly)\n");
                printf("  - Linux Mint (Windows-like interface)\n");
                printf("  - Fedora Workstation (modern, cutting-edge)\n");
                printf("  - Pop!_OS (great for gaming)\n");
                break;
            case SERVER:
            case CLUSTER:
                printf("  Recommended Linux distributions for your hardware:\n");
                printf("  - Ubuntu Server\n");
                printf("  - RHEL / Rocky Linux\n");
                printf("  - Debian Server\n");
                break;
            case MOBILE:
                printf("  Recommended lightweight Linux:\n");
                printf("  - Lubuntu (lightweight Ubuntu)\n");
                printf("  - Xubuntu (XFCE desktop)\n");
                printf("  - Linux Lite\n");
                break;
            default:
                printf("  Recommended: Standard Linux distributions\n");
                break;
        }
    } else {
        /* Original recommendations for non-Windows systems */
        printf("  Recommended OS:\n");
        switch (hw->device_class) {
            case CALCULATOR:
                printf("  - TinyOS / Custom Embedded OS\n");
                printf("  Reason: Extremely limited resources\n");
                break;
            case EMBEDDED:
                printf("  - Alpine Linux Embedded / Buildroot\n");
                printf("  Reason: Minimal footprint for embedded systems\n");
                break;
            case MOBILE:
                printf("  - Alpine Linux Standard / Debian Minimal\n");
                printf("  Reason: Lightweight OS for mobile/low-resource devices\n");
                break;
            case DESKTOP:
                printf("  - Ubuntu Desktop / Alpine Linux Desktop\n");
                printf("  Reason: Good balance of features and performance\n");
                break;
            case WORKSTATION:
                printf("  - Ubuntu / Fedora Workstation\n");
                printf("  Reason: Full-featured desktop for professional work\n");
                break;
            case SERVER:
                printf("  - RHEL / Ubuntu Server / Alpine Server\n");
                printf("  Reason: Optimized for server workloads\n");
                break;
            case CLUSTER:
                printf("  - CentOS / Rocky Linux / HPC Linux\n");
                printf("  Reason: Cluster-optimized distribution\n");
                break;
            case SUPERCOMPUTER:
                printf("  - Cray Linux / Custom HPC Distribution\n");
                printf("  Reason: Exascale-optimized OS\n");
                break;
        }
    }
    printf("\n");

    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  Detection Complete - Universal Compatibility Verified   ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
}
