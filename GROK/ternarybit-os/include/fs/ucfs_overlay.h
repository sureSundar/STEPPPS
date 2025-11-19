#ifndef TBOS_FS_UCFS_OVERLAY_H
#define TBOS_FS_UCFS_OVERLAY_H

#include <stddef.h>

int ucfs_set_backing_root(const char* path);
int ucfs_write_file_uc(const char* ucfs_path, const void* data, size_t size);
int ucfs_read_file_uc(const char* ucfs_path, void* buffer, size_t max_size, size_t* out_size);
int ucfs_exists_uc(const char* ucfs_path);
int ucfs_resolve_path(const char* ucfs_path, char* buffer, size_t buflen);
const char* ucfs_prompt_form(const char* canonical_path, char* buffer, size_t buflen);

#endif /* TBOS_FS_UCFS_OVERLAY_H */
