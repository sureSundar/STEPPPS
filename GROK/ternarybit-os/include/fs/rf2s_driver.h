/*
 * RF2S (Radio Frequency Filesystem) VFS Driver
 * Maps radio frequencies to filesystem paths
 */

#ifndef TBOS_FS_RF2S_DRIVER_H
#define TBOS_FS_RF2S_DRIVER_H

#include "tbos/vfs.h"

/* RF2S VFS driver instance */
extern const vfs_driver_t rf2s_driver;

/*
 * Configure backing driver for RF2S
 *
 * RF2S is an overlay filesystem - it translates frequency-based paths
 * and delegates to a backing driver (typically RAMFS).
 *
 * Parameters:
 *   rf2s_ctx     - Context returned by rf2s_driver.init()
 *   backing      - Backing VFS driver (e.g., &ramfs_driver)
 *   backing_ctx  - Context for backing driver
 *   backing_root - Root path in backing store (e.g., "/rf2s")
 *
 * Returns: 0 on success, negative errno on error
 */
int rf2s_set_backing_driver(void* rf2s_ctx,
                            const vfs_driver_t* backing,
                            void* backing_ctx,
                            const char* backing_root);

/*
 * Set the base frequency for RF2S operations
 * Default: 432000000 (432 MHz - sacred carrier)
 */
void rf2s_set_base_frequency(void* rf2s_ctx, uint64_t base_freq_hz);

/*
 * Get current base frequency
 */
uint64_t rf2s_get_base_frequency(void* rf2s_ctx);

#endif /* TBOS_FS_RF2S_DRIVER_H */
