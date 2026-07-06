#ifndef TBOS_VFS_H
#define TBOS_VFS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "tbos/libc.h"

typedef enum {
    VFS_NODE_DIR,
    VFS_NODE_FILE
} vfs_node_type_t;

/* Forward declaration of stat structure */
struct stat;

typedef struct vfs_driver {
    const char* name;
    void* (*init)(void);
    int   (*mkdir)(void* ctx, const char* path);
    int   (*write_file)(void* ctx, const char* path, const void* data, size_t size, bool append);
    int   (*read_file)(void* ctx, const char* path, void* buffer, size_t max_size, size_t* out_size);
    const void* (*read_file_cstr)(void* ctx, const char* path, size_t* out_size);
    int   (*remove)(void* ctx, const char* path, bool recursive);
    bool  (*exists)(void* ctx, const char* path);
    vfs_node_type_t (*type)(void* ctx, const char* path);
    int   (*list_dir)(void* ctx, const char* path, int (*cb)(const char* name, vfs_node_type_t type, void* user), void* user);

    /* Permission operations (Phase 2) */
    int   (*chmod)(void* ctx, const char* path, uint32_t mode);
    int   (*chown)(void* ctx, const char* path, uint32_t uid, uint32_t gid);
    int   (*stat)(void* ctx, const char* path, struct stat* st);
} vfs_driver_t;

void vfs_init(void);
int  vfs_mount(const char* mount_point, const vfs_driver_t* driver);
int  vfs_mount_with_context(const char* mount_point, const vfs_driver_t* driver, void* ctx);

int  vfs_mkdir(const char* path);
int  vfs_write_file(const char* path, const void* data, size_t size);
int  vfs_append_file(const char* path, const void* data, size_t size);
int  vfs_read_file(const char* path, void* buffer, size_t max_size, size_t* out_size);
const void* vfs_read_file_cstr(const char* path, size_t* out_size);
int  vfs_remove(const char* path, bool recursive);
bool vfs_exists(const char* path);
vfs_node_type_t vfs_type(const char* path);
int  vfs_list_dir(const char* path, int (*cb)(const char* name, vfs_node_type_t type, void* user), void* user);

/* Permission operations */
int  vfs_chmod(const char* path, uint32_t mode);
int  vfs_chown(const char* path, uint32_t uid, uint32_t gid);
int  vfs_stat(const char* path, struct stat* st);

/* STEPPPS integration */
void vfs_enable_steppps(bool enable);
bool vfs_steppps_enabled(void);

#endif /* TBOS_VFS_H */
