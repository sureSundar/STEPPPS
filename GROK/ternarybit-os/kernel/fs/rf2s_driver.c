/*
 * RF2S (Radio Frequency Filesystem) VFS Driver
 * Maps radio frequencies to filesystem paths
 *
 * Path format: /rf2s/432MHz/sensor.dat -> /rf2s/432000000/sensor.dat
 */

#include "tbos/vfs.h"
#include "fs/rf2s_codec.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

/* RF2S driver context */
typedef struct {
    void* backing_ctx;
    const vfs_driver_t* backing_driver;
    char backing_root[128];
    uint64_t base_frequency;      /* Default: 432 MHz */
    int auto_create_dirs;         /* Auto-create frequency directories */
} rf2s_driver_ctx_t;

/* Default sacred frequency: 432 MHz */
#define RF2S_DEFAULT_BASE_FREQ 432000000ULL

/*
 * Parse and canonicalize an RF2S path
 *
 * Input paths can be:
 *   /432MHz/file.dat        -> /rf2s/432000000/file.dat
 *   /432000000/file.dat     -> /rf2s/432000000/file.dat  (already normalized)
 *   432MHz/file.dat         -> /rf2s/432000000/file.dat
 */
static int rf2s_parse_and_canonicalize(rf2s_driver_ctx_t* ctx,
                                       const char* rf2s_path,
                                       char* canonical,
                                       size_t buflen) {
    if (!ctx || !rf2s_path || !canonical || buflen == 0) {
        return -EINVAL;
    }

    /* Skip leading slash */
    const char* path = rf2s_path;
    if (*path == '/') path++;

    /* Empty path - return backing root */
    if (*path == '\0') {
        if (strlen(ctx->backing_root) >= buflen) {
            return -ENOSPC;
        }
        strcpy(canonical, ctx->backing_root);
        return 0;
    }

    /* Extract first component */
    const char* slash = strchr(path, '/');
    size_t first_len = slash ? (size_t)(slash - path) : strlen(path);

    char first_component[64];
    if (first_len >= sizeof(first_component)) {
        return -EINVAL;
    }
    memcpy(first_component, path, first_len);
    first_component[first_len] = '\0';

    /* Check if first component is a frequency */
    if (rf2s_is_frequency(first_component)) {
        /* Parse as RF2S path */
        rf2s_path_t parsed;
        int rc = rf2s_parse(rf2s_path, &parsed);
        if (rc != 0) {
            return rc;
        }

        /* Convert to canonical form */
        rc = rf2s_to_canonical_with_base(&parsed, ctx->backing_root, canonical, buflen);
        rf2s_free(&parsed);
        return rc;
    }

    /* Not a frequency path - pass through with backing root prefix */
    size_t root_len = strlen(ctx->backing_root);
    size_t path_len = strlen(rf2s_path);

    if (root_len + path_len + 1 >= buflen) {
        return -ENOSPC;
    }

    strcpy(canonical, ctx->backing_root);
    if (rf2s_path[0] != '/') {
        strcat(canonical, "/");
    }
    strcat(canonical, rf2s_path);

    return 0;
}

/*
 * Ensure parent directories exist for a path
 * This is used to auto-create frequency directories
 */
static int rf2s_ensure_parents(rf2s_driver_ctx_t* ctx, const char* canonical) {
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->mkdir) {
        return 0;  /* Can't create, but don't fail */
    }

    char parent[512];
    if (strlen(canonical) >= sizeof(parent)) {
        return -ENOSPC;
    }
    strcpy(parent, canonical);

    /* Find last slash */
    char* last_slash = strrchr(parent, '/');
    if (!last_slash || last_slash == parent) {
        return 0;  /* No parent to create */
    }

    *last_slash = '\0';

    /* Check if parent exists */
    if (ctx->backing_driver->exists &&
        ctx->backing_driver->exists(ctx->backing_ctx, parent)) {
        return 0;
    }

    /* Recursively ensure grandparents */
    rf2s_ensure_parents(ctx, parent);

    /* Create parent */
    return ctx->backing_driver->mkdir(ctx->backing_ctx, parent);
}

/* VFS operations */

static void* rf2s_init_ctx(void) {
    rf2s_driver_ctx_t* ctx = malloc(sizeof(rf2s_driver_ctx_t));
    if (!ctx) {
        return NULL;
    }

    ctx->backing_ctx = NULL;
    ctx->backing_driver = NULL;
    strcpy(ctx->backing_root, "/rf2s");
    ctx->base_frequency = RF2S_DEFAULT_BASE_FREQ;
    ctx->auto_create_dirs = 1;

    return ctx;
}

static int rf2s_mkdir_impl(void* context, const char* path) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->mkdir) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    /* Ensure parent directories exist */
    if (ctx->auto_create_dirs) {
        rf2s_ensure_parents(ctx, canonical);
    }

    return ctx->backing_driver->mkdir(ctx->backing_ctx, canonical);
}

static int rf2s_write_file_impl(void* context, const char* path,
                                 const void* data, size_t size, bool append) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->write_file) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    /* Ensure parent directories exist */
    if (ctx->auto_create_dirs) {
        rf2s_ensure_parents(ctx, canonical);
    }

    return ctx->backing_driver->write_file(ctx->backing_ctx, canonical, data, size, append);
}

static int rf2s_read_file_impl(void* context, const char* path,
                                void* buffer, size_t max_size, size_t* out_size) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->read_file) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->read_file(ctx->backing_ctx, canonical, buffer, max_size, out_size);
}

static const void* rf2s_read_file_cstr_impl(void* context, const char* path, size_t* out_size) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->read_file_cstr) {
        return NULL;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return NULL;
    }

    return ctx->backing_driver->read_file_cstr(ctx->backing_ctx, canonical, out_size);
}

static int rf2s_remove_impl(void* context, const char* path, bool recursive) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->remove) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->remove(ctx->backing_ctx, canonical, recursive);
}

static bool rf2s_exists_impl(void* context, const char* path) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->exists) {
        return false;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return false;
    }

    return ctx->backing_driver->exists(ctx->backing_ctx, canonical);
}

static vfs_node_type_t rf2s_type_impl(void* context, const char* path) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->type) {
        return VFS_NODE_FILE;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return VFS_NODE_FILE;
    }

    return ctx->backing_driver->type(ctx->backing_ctx, canonical);
}

static int rf2s_list_dir_impl(void* context, const char* path,
                               int (*cb)(const char*, vfs_node_type_t, void*),
                               void* user) {
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)context;
    if (!ctx || !ctx->backing_driver || !ctx->backing_driver->list_dir) {
        return -ENOSYS;
    }

    char canonical[512];
    int rc = rf2s_parse_and_canonicalize(ctx, path, canonical, sizeof(canonical));
    if (rc != 0) {
        return rc;
    }

    return ctx->backing_driver->list_dir(ctx->backing_ctx, canonical, cb, user);
}

/* RF2S VFS driver instance */
const vfs_driver_t rf2s_driver = {
    .name = "rf2s",
    .init = rf2s_init_ctx,
    .mkdir = rf2s_mkdir_impl,
    .write_file = rf2s_write_file_impl,
    .read_file = rf2s_read_file_impl,
    .read_file_cstr = rf2s_read_file_cstr_impl,
    .remove = rf2s_remove_impl,
    .exists = rf2s_exists_impl,
    .type = rf2s_type_impl,
    .list_dir = rf2s_list_dir_impl
};

/* Configuration functions */

int rf2s_set_backing_driver(void* rf2s_ctx,
                            const vfs_driver_t* backing,
                            void* backing_ctx,
                            const char* backing_root) {
    if (!rf2s_ctx || !backing) {
        return -EINVAL;
    }

    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)rf2s_ctx;
    ctx->backing_driver = backing;
    ctx->backing_ctx = backing_ctx;

    if (backing_root && backing_root[0] == '/' &&
        strlen(backing_root) < sizeof(ctx->backing_root)) {
        strcpy(ctx->backing_root, backing_root);
    }

    return 0;
}

void rf2s_set_base_frequency(void* rf2s_ctx, uint64_t base_freq_hz) {
    if (!rf2s_ctx) return;
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)rf2s_ctx;
    ctx->base_frequency = base_freq_hz;
}

uint64_t rf2s_get_base_frequency(void* rf2s_ctx) {
    if (!rf2s_ctx) return RF2S_DEFAULT_BASE_FREQ;
    rf2s_driver_ctx_t* ctx = (rf2s_driver_ctx_t*)rf2s_ctx;
    return ctx->base_frequency;
}
