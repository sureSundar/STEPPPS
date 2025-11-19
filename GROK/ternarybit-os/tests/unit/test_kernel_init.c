/**
 * @file test_kernel_init.c
 * @brief Test TernaryBit OS Kernel Initialization
 *
 * Tests the complete kernel boot sequence with module registry
 */

#include "../../src/core/tbos_init.c"  /* Include to test */
#include <stdio.h>

int main(void) {
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Kernel Initialization Test            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    /* Test kernel initialization */
    tbos_result_t result = tbos_kernel_init();

    if (result == TBOS_SUCCESS) {
        printf("\n✅ Kernel initialization test PASSED\n");
        printf("\nSimulating shell prompt:\n");
        printf("tbos> _\n\n");

        /* Shutdown */
        tbos_kernel_shutdown();

        return 0;
    } else {
        printf("\n❌ Kernel initialization test FAILED (code: %d)\n", result);
        return 1;
    }
}
