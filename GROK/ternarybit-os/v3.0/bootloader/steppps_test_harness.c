/*
 * TBOS v3.0 STEPPPS Framework Test Harness
 * Test runner for the 7-dimensional integration
 */

#include "steppps_bootloader_integration.c"

int main(void) {
    printf("🚀 TBOS v3.0 STEPPPS Framework Test Harness\n");
    printf("🕉️ Testing 7-Dimensional Bootloader Integration\n\n");

    // Test ChemOS persona initialization
    printf("Testing ChemOS Persona (Architecture 7):\n");
    init_steppps_framework(PERSONA_CHEMOS);
    steppps_boot_sequence(PERSONA_CHEMOS);
    display_steppps_status();

    printf("\n" "─────────────────────────────────────────────\n\n");

    // Test x86 UEFI persona initialization
    printf("Testing x86 UEFI Persona (Architecture 3):\n");
    init_steppps_framework(PERSONA_X86_UEFI);
    steppps_boot_sequence(PERSONA_X86_UEFI);
    display_steppps_status();

    printf("\n🎉 STEPPPS Framework Integration Test Complete!\n");

    return 0;
}