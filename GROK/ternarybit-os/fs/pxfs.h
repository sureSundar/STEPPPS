#ifndef TBOS_PXFS_H
#define TBOS_PXFS_H

#include <stddef.h>
#include <stdint.h>

// WINDSURF: Minimal PXFS interface
int pxfs_init(void);
int pxfs_mount(const char* dev);
int pxfs_umount(void);
int pxfs_read(const char* path, void* buf, size_t sz);
int pxfs_write(const char* path, const void* buf, size_t sz);

#endif // TBOS_PXFS_H
