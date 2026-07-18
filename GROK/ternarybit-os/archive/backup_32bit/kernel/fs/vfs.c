#include "tbos/vfs.h"
#include "tbos/errno.h"

#define MAX_MOUNTS 8

typedef struct {
    char mount_point[64];
    size_t mount_len;
    const vfs_driver_t* driver;
    void* ctx;
} vfs_mount_t;

static vfs_mount_t vfs_mounts[MAX_MOUNTS];
static size_t vfs_mount_count = 0;

static const vfs_mount_t* vfs_find_mount(const char* path, const char** subpath) {
    if (!path || path[0] != '/') return NULL;
    const vfs_mount_t* best = NULL;
    for (size_t i = 0; i < vfs_mount_count; i++) {
        const vfs_mount_t* mount = &vfs_mounts[i];
        if (mount->mount_len == 1) {
            best = mount;
            continue;
        }
        if (strncmp(path, mount->mount_point, mount->mount_len) == 0) {
            if (best == NULL || mount->mount_len > best->mount_len) {
                best = mount;
            }
        }
    }
    if (!best) return NULL;
    *subpath = (best->mount_len == 1) ? path : path + best->mount_len;
    if (**subpath == '\0') *subpath = "/";
    return best;
}

void vfs_init(void) {
    vfs_mount_count = 0;
}

static int vfs_mount_internal(const char* mount_point, const vfs_driver_t* driver, void* ctx) {
    if (!mount_point || mount_point[0] != '/' || !driver) return -EINVAL;
    if (vfs_mount_count >= MAX_MOUNTS) return -ENOSPC;
    for (size_t i = 0; i < vfs_mount_count; i++) {
        if (strcmp(vfs_mounts[i].mount_point, mount_point) == 0) {
            return -EEXIST;
        }
    }
    vfs_mount_t* mount = &vfs_mounts[vfs_mount_count];
    strncpy(mount->mount_point, mount_point, sizeof(mount->mount_point) - 1);
    mount->mount_point[sizeof(mount->mount_point) - 1] = '\0';
    mount->mount_len = strlen(mount->mount_point);
    mount->driver = driver;
    mount->ctx = ctx ? ctx : (driver->init ? driver->init() : NULL);
    vfs_mount_count++;
    return 0;
}

int vfs_mount(const char* mount_point, const vfs_driver_t* driver) {
    return vfs_mount_internal(mount_point, driver, NULL);
}

int vfs_mount_with_context(const char* mount_point, const vfs_driver_t* driver, void* ctx) {
    return vfs_mount_internal(mount_point, driver, ctx);
}

static int vfs_dispatch(const char* path, int (*op)(const vfs_mount_t*, const char*, void*), void* arg) {
    const char* subpath = NULL;
    const vfs_mount_t* mount = vfs_find_mount(path, &subpath);
    if (!mount) return -ENOENT;
    return op(mount, subpath, arg);
}

static int op_mkdir(const vfs_mount_t* mount, const char* subpath, void* arg) {
    (void)arg;
    if (!mount->driver->mkdir) return -ENOSYS;
    return mount->driver->mkdir(mount->ctx, subpath);
}

int vfs_mkdir(const char* path) {
    return vfs_dispatch(path, op_mkdir, NULL);
}

typedef struct {
    const void* data;
    size_t size;
    bool append;
} write_args_t;

static int op_write(const vfs_mount_t* mount, const char* subpath, void* arg) {
    if (!mount->driver->write_file) return -ENOSYS;
    write_args_t* wa = (write_args_t*)arg;
    return mount->driver->write_file(mount->ctx, subpath, wa->data, wa->size, wa->append);
}

int vfs_write_file(const char* path, const void* data, size_t size) {
    write_args_t args = { data, size, false };
    return vfs_dispatch(path, op_write, &args);
}

int vfs_append_file(const char* path, const void* data, size_t size) {
    write_args_t args = { data, size, true };
    return vfs_dispatch(path, op_write, &args);
}

typedef struct {
    void* buffer;
    size_t max_size;
    size_t* out_size;
    int result;
} read_args_t;

static int op_read(const vfs_mount_t* mount, const char* subpath, void* arg) {
    if (!mount->driver->read_file) return -ENOSYS;
    read_args_t* ra = (read_args_t*)arg;
    return mount->driver->read_file(mount->ctx, subpath, ra->buffer, ra->max_size, ra->out_size);
}

int vfs_read_file(const char* path, void* buffer, size_t max_size, size_t* out_size) {
    read_args_t args = { buffer, max_size, out_size, 0 };
    return vfs_dispatch(path, op_read, &args);
}

const void* vfs_read_file_cstr(const char* path, size_t* out_size) {
    const char* subpath = NULL;
    const vfs_mount_t* mount = vfs_find_mount(path, &subpath);
    if (!mount || !mount->driver->read_file_cstr) return NULL;
    return mount->driver->read_file_cstr(mount->ctx, subpath, out_size);
}

static int op_remove(const vfs_mount_t* mount, const char* subpath, void* arg) {
    if (!mount->driver->remove) return -ENOSYS;
    bool recursive = arg != NULL;
    return mount->driver->remove(mount->ctx, subpath, recursive);
}

int vfs_remove(const char* path, bool recursive) {
    return vfs_dispatch(path, op_remove, recursive ? (void*)1 : NULL);
}

static int op_exists(const vfs_mount_t* mount, const char* subpath, void* arg) {
    if (!mount->driver->exists) return 0;
    bool exists = mount->driver->exists(mount->ctx, subpath);
    *(bool*)arg = exists;
    return 0;
}

bool vfs_exists(const char* path) {
    bool exists = false;
    if (vfs_dispatch(path, op_exists, &exists) != 0) return false;
    return exists;
}

static int op_type(const vfs_mount_t* mount, const char* subpath, void* arg) {
    if (!mount->driver->type) return -ENOSYS;
    vfs_node_type_t* out = (vfs_node_type_t*)arg;
    *out = mount->driver->type(mount->ctx, subpath);
    return 0;
}

vfs_node_type_t vfs_type(const char* path) {
    vfs_node_type_t type = VFS_NODE_FILE;
    if (vfs_dispatch(path, op_type, &type) != 0) return VFS_NODE_FILE;
    return type;
}

typedef struct {
    int (*cb)(const char*, vfs_node_type_t, void*);
    void* user;
} list_args_t;

static int op_list(const vfs_mount_t* mount, const char* subpath, void* arg) {
    if (!mount->driver->list_dir) return -ENOSYS;
    list_args_t* la = (list_args_t*)arg;
    return mount->driver->list_dir(mount->ctx, subpath, la->cb, la->user);
}

int vfs_list_dir(const char* path, int (*cb)(const char*, vfs_node_type_t, void*), void* user) {
    list_args_t args = { cb, user };
    return vfs_dispatch(path, op_list, &args);
}
