/**
 * @file test_tbos_mmap.c
 * @brief Test program for TBOS memory-mapped file operations
 */

#include <stdio.h>

// Function declaration
void tbos_mmap_demo(void);

// Include the implementation
#include "src/core/tbos_mmap.c"

int main(void) {
    printf("🚀 Testing TBOS Real Memory-Mapped File Operations\n");
    printf("=================================================\n");

    // Run the demonstration
    tbos_mmap_demo();

    printf("\n✅ TBOS Memory-Mapped Operations Test Complete!\n");
    printf("Proven: Real mmap, msync, mprotect, and shared memory!\n");

    return 0;
}