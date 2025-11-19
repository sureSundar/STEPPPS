#include "tbos/vfs.h"
#include "fs/ucfs_codec.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

#include <stdlib.h>

typedef struct {
    void* backing_ctx;
    const vfs_driver_t* backing_driver;
    char backing_root[128];
} ucfs_driver_ctx_t;

static int ucfs_parse_and_canonicalize(ucfs_driver_ctx_t* ctx, const char* ucfs_path, char* canonical, size_t buflen) {
    if (!ctx || !ucfs_path || !canonical || buflen == 0) {
        return -EINVAL;
    }

    /* Check if this is a UCFS path (starts with '[') */
    if (ucfs_path[0] != '[') {
        /* Not a UCFS path, use as-is */
        if (strlen(ucfs_path) >= buflen) {
            return -ENOSPC;
        }
        strcpy(canonical, ucfs_path);
        return 0;
    }

    /* Parse UCFS path */
    ucfs_path_t parsed;
    int rc = ucfs_parse(ucfs_path, &parsed);
    if (rc != 0) {
        return rc;
    }

    /* Convert to canonical form */
    rc = ucfs_to_canonical_with_base(&parsed, ctx->backing_root, canonical, buflen);
    ucfs_free(&parsed);

    return rc;
}

static int ucfs_mkdir_impl(void* context, const char* path) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->mkdir) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->mkdir(ctx->backing_ctx, canonical);
}

static int ucfs_write_file_impl(void* context, const char* path, const void* data, size_t size, bool append) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->write_file) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->write_file(ctx->backing_ctx, canonical, data, size, append);
}

static int ucfs_read_file_impl(void* context, const char* path, void* buffer, size_t max_size, size_t* out_size) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->read_file) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->read_file(ctx->backing_ctx, canonical, buffer, max_size, out_size);
}

static const void* ucfs_read_file_cstr_impl(void* context, const char* path, size_t* out_size) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->read_file_cstr) {
        return NULL;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return NULL;
    }

    return ctx->backing_driver->read_file_cstr(ctx->backing_ctx, canonical, out_size);
}

static int ucfs_remove_impl(void* context, const char* path, bool recursive) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->remove) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->remove(ctx->backing_ctx, canonical, recursive);
}

static bool ucfs_exists_impl(void* context, const char* path) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->exists) {
        return false;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return false;
    }

    return ctx->backing_driver->exists(ctx->backing_ctx, canonical);
}

static vfs_node_type_t ucfs_type_impl(void* context, const char* path) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->type) {
        return VFS_NODE_FILE;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return VFS_NODE_FILE;
    }

    return ctx->backing_driver->type(ctx->backing_ctx, canonical);
}

static int ucfs_list_dir_impl(void* context, const char* path,
                              int (*cb)(const char*, vfs_node_type_t, void*), void* user) {
    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->list_dir) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = ucfs_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->list_dir(ctx->backing_ctx, canonical, cb, user);
}

static void* ucfs_init_ctx(void) {
    ucfs_driver_ctx_t* ctx = malloc(sizeof(ucfs_driver_ctx_t));
    if (!ctx) {
        return NULL;
    }

    ctx->backing_ctx = NULL;
    ctx->backing_driver = NULL;
    strcpy(ctx->backing_root, "/ucfs");

    return ctx;
}

const vfs_driver_t ucfs_driver = {
    .name = "ucfs",
    .init = ucfs_init_ctx,
    .mkdir = ucfs_mkdir_impl,
    .write_file = ucfs_write_file_impl,
    .read_file = ucfs_read_file_impl,
    .read_file_cstr = ucfs_read_file_cstr_impl,
    .remove = ucfs_remove_impl,
    .exists = ucfs_exists_impl,
    .type = ucfs_type_impl,
    .list_dir = ucfs_list_dir_impl
};

/* UCFS-specific initialization function to set backing driver */
int ucfs_set_backing_driver(void* ucfs_ctx, const vfs_driver_t* backing_driver, void* backing_ctx, const char* backing_root) {
    if (!ucfs_ctx || !backing_driver) {
        return -EINVAL;
    }

    ucfs_driver_ctx_t* ctx = (ucfs_driver_ctx_t*)ucfs_ctx;
    ctx->backing_driver = backing_driver;
    ctx->backing_ctx = backing_ctx;

    if (backing_root && backing_root[0] == '/' && strlen(backing_root) < sizeof(ctx->backing_root)) {
        strcpy(ctx->backing_root, backing_root);
    }

    return 0;
}
