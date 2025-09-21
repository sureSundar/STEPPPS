// WINDSURF: Minimal PXFS stub
#include "pxfs.h"

int pxfs_init(void) { return 0; }
int pxfs_mount(const char* dev) { (void)dev; return 0; }
int pxfs_umount(void) { return 0; }
int pxfs_read(const char* path, void* buf, size_t sz) { (void)path; (void)buf; (void)sz; return 0; }
int pxfs_write(const char* path, const void* buf, size_t sz) { (void)path; (void)buf; (void)sz; return 0; }
