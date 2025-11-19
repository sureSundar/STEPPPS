#ifndef TBOS_FS_FAT32_DRIVER_H
#define TBOS_FS_FAT32_DRIVER_H

#include "tbos/blockdev.h"
#include "tbos/vfs.h"

typedef struct fat32_fs fat32_fs_t;

fat32_fs_t* fat32_mount_blockdev(const tbos_blockdev_t* dev);
const vfs_driver_t* fat32_vfs_driver(void);

#endif /* TBOS_FS_FAT32_DRIVER_H */
