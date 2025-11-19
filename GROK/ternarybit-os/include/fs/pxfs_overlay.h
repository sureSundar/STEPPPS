#ifndef TBOS_FS_PXFS_OVERLAY_H
#define TBOS_FS_PXFS_OVERLAY_H

#include <stddef.h>

int pxfs_set_backing_root(const char* path);
int pxfs_resolve_path(const char* pxfs_path, char* buffer, size_t buflen);
const char* pxfs_prompt_form(const char* canonical_path, char* buffer, size_t buflen);
int pxfs_write_file(const char* pxfs_path, const void* data, size_t size);
int pxfs_read_file(const char* pxfs_path, void* buffer, size_t max_size, size_t* out_size);
int pxfs_exists(const char* pxfs_path);

#endif /* TBOS_FS_PXFS_OVERLAY_H */
