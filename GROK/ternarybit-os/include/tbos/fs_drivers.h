#ifndef TBOS_FS_DRIVERS_H
#define TBOS_FS_DRIVERS_H

#include "tbos/vfs.h"

extern const vfs_driver_t ramfs_driver;
extern const vfs_driver_t ucfs_driver;
extern const vfs_driver_t rf2s_driver;

/* Configuration functions */
int ucfs_set_backing_driver(void* ucfs_ctx, const vfs_driver_t* backing,
                            void* backing_ctx, const char* backing_root);
int rf2s_set_backing_driver(void* rf2s_ctx, const vfs_driver_t* backing,
                            void* backing_ctx, const char* backing_root);

#endif /* TBOS_FS_DRIVERS_H */
