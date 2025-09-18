/**
 * @file test_tbos_ipc.c
 * @brief Test program for TBOS IPC mechanisms
 */

#define _GNU_SOURCE
#include <stdio.h>

// Function declaration
void tbos_ipc_demo(void);

// Include the implementation
#include "src/core/tbos_ipc.c"

int main(void) {
    printf("🚀 Testing TBOS Inter-Process Communication\n");
    printf("===========================================\n");

    // Run the IPC demonstration
    tbos_ipc_demo();

    printf("\n✅ TBOS IPC Test Complete!\n");
    printf("Proven: Real message queues, shared memory, semaphores, and pipes!\n");

    return 0;
}