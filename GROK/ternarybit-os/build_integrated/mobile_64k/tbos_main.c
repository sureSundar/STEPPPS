/**
 * TernaryBit OS - Integrated System
 * Boot -> HAL Init -> Shell
 */

#include "../../src/boot/tbos_minimal_boot.h"
#include "../../src/hal/tbos_hal.h"
#include "../../src/core/shell/tbos_shell.h"
#include "../../src/core/shell/tbos_supershell.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    int result;

    /* Stage 1: Boot */
    printf("\n=== TernaryBit OS Boot Sequence ===\n\n");
    result = tbos_minimal_boot();
    if (result != 0) {
        fprintf(stderr, "Boot failed!\n");
        return 1;
    }

    /* Stage 2: HAL Init */
    printf("\n=== Hardware Abstraction Layer Init ===\n\n");
    result = hal_init();
    if (result != 0) {
        fprintf(stderr, "HAL init failed!\n");
        return 1;
    }

    hal_profile_t profile = hal_get_profile();
    printf("Hardware Profile: %d\n", profile);
    printf("Free Memory: %u bytes\n", hal_get_free_memory());
    printf("\n");

    /* Stage 3: Shell Init & Run */
    printf("=== Conscious Shell Init ===\n\n");
    result = tbos_shell_init();
    if (result != 0) {
        fprintf(stderr, "Shell init failed!\n");
        return 1;
    }

    printf("TernaryBit OS Ready!\n");
    printf("Type 'help' for commands, 'exit' to quit.\n\n");

    tbos_supershell_init();
    const char* persona_id = getenv("TBOS_PERSONA_NAME");
    if (!persona_id || persona_id[0] == '\0') {
        persona_id = "desktop_512m";
    }
    result = tbos_supershell_launch(persona_id);

    printf("\n=== Graceful Shutdown ===\n\n");
    printf("॥ तत् सत् ॥\n\n");

    return result;
}
