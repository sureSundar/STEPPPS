#ifndef TBOS_FS_UCFS_DRIVER_H
#define TBOS_FS_UCFS_DRIVER_H

#include "tbos/vfs.h"

/* UCFS VFS Driver - provides Unicode Character Filesystem overlay */
extern const vfs_driver_t ucfs_driver;

/**
 * Configure the backing driver for UCFS.
 *
 * @param ucfs_ctx Context returned from ucfs_driver.init()
 * @param backing_driver The underlying VFS driver (e.g., ramfs_driver)
 * @param backing_ctx Context for the backing driver
 * @param backing_root Root path for UCFS storage (default: "/ucfs")
 * @return 0 on success, negative error code on failure
 */
int ucfs_set_backing_driver(void* ucfs_ctx, const vfs_driver_t* backing_driver,
                            void* backing_ctx, const char* backing_root);

#endif /* TBOS_FS_UCFS_DRIVER_H */
