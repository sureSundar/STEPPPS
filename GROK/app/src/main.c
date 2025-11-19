/**
 * TBOS Universal Hardware Detector - Application Mode
 * Runs on existing OS (Linux/Windows/macOS)
 * 🕉️ Swamiye Saranam Aiyappa 🕉️
 */

#include "../include/tbos_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char* prog_name) {
    printf("TBOS Universal Hardware Detector\n");
    printf("Usage: %s [OPTIONS]\n\n", prog_name);
    printf("Options:\n");
    printf("  --detect      Detect hardware and display report (default)\n");
    printf("  --json        Output in JSON format\n");
    printf("  --help        Show this help message\n");
    printf("\n");
}

void print_json(const HardwareProfile* hw) {
    printf("{\n");
    printf("  \"magic\": \"TBOS\",\n");
    printf("  \"version\": \"1.0\",\n");
    printf("  \"source\": \"application\",\n");
    printf("  \"platform\": \"linux\",\n");
    printf("  \"hardware\": {\n");
    printf("    \"cpu_vendor\": \"%s\",\n", hw->cpu_vendor);
    printf("    \"cpu_cores\": %u,\n", hw->cpu_cores);
    printf("    \"cpu_mhz\": %u,\n", hw->cpu_mhz);
    printf("    \"memory_kb\": %lu,\n", hw->total_memory_kb);
    printf("    \"memory_mb\": %lu,\n", hw->total_memory_mb);
    printf("    \"architecture\": \"%s\",\n", hw->architecture);
    printf("    \"os_name\": \"%s\",\n", hw->os_name);
    printf("    \"device_class\": %d,\n", hw->device_class);
    printf("    \"device_class_name\": \"%s\"\n", DEVICE_CLASS_NAMES[hw->device_class]);
    printf("  }\n");
    printf("}\n");
}

int main(int argc, char** argv) {
    int json_output = 0;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--json") == 0) {
            json_output = 1;
        } else if (strcmp(argv[i], "--detect") == 0) {
            /* Default behavior */
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    /* Detect hardware */
    HardwareProfile hw = detect_hardware();

    /* Output results */
    if (json_output) {
        print_json(&hw);
    } else {
        print_hardware_profile(&hw);
    }

    return 0;
}
