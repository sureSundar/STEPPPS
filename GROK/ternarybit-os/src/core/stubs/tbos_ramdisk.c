/**
 * @file tbos_ramdisk.c
 * @brief Stub implementation for RAMDISK functions
 *
 * These stubs allow the shell to compile and run on hosted systems
 * without the full ramdisk implementation.
 */

#include <stddef.h>

/* Legacy stub */
void ramdisk_init(void) {}

/* Required by universal shell */
int tbos_ramdisk_init(size_t size_bytes) {
    (void)size_bytes;
    return 0;  /* Success - hosted mode uses real filesystem */
}

int tbos_ramdisk_create_root_structure(void) {
    return 0;  /* Success - hosted mode uses real filesystem */
}
