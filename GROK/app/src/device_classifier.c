/**
 * TBOS Universal Device Classifier
 * Same logic for bootloader AND application
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#include "../include/tbos_common.h"

/**
 * Classify device based on total memory
 * This function is IDENTICAL across all platforms
 */
DeviceClass classify_device(uint64_t memory_mb) {
    /* Calculator: < 1 MB */
    if (memory_mb < 1) {
        return CALCULATOR;
    }

    /* Embedded: 1 MB - 64 MB */
    if (memory_mb < 64) {
        return EMBEDDED;
    }

    /* Mobile: 64 MB - 4 GB */
    if (memory_mb < 4 * 1024) {
        return MOBILE;
    }

    /* Desktop: 4 GB - 16 GB */
    if (memory_mb < 16 * 1024) {
        return DESKTOP;
    }

    /* Workstation: 16 GB - 32 GB */
    if (memory_mb < 32 * 1024) {
        return WORKSTATION;
    }

    /* Server: 32 GB - 64 GB */
    if (memory_mb < 64 * 1024) {
        return SERVER;
    }

    /* Cluster: 64 GB - 128 GB */
    if (memory_mb < 128 * 1024) {
        return CLUSTER;
    }

    /* Supercomputer: >= 128 GB */
    return SUPERCOMPUTER;
}
