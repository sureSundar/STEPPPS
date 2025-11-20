#include "tbos/config.h"

#if !CONFIG_FS

#include "tbos/vfs.h"
#include "fs/ucfs_overlay.h"
#include "fs/ucfs_codec.h"
#include "fs/ucfs_config.h"
#include "tbos/libc.h"
#include "tbos/errno.h"

void vfs_init(void) {}
int vfs_mount(const char* mount_point, const vfs_driver_t* driver) {
    (void)mount_point; (void)driver; return -ENOSYS;
}
int vfs_mount_with_context(const char* mount_point, const vfs_driver_t* driver, void* ctx) {
    (void)mount_point; (void)driver; (void)ctx; return -ENOSYS;
}
int vfs_mkdir(const char* path) { (void)path; return -ENOSYS; }
int vfs_write_file(const char* path, const void* data, size_t size) {
    (void)path; (void)data; (void)size; return -ENOSYS;
}
int vfs_append_file(const char* path, const void* data, size_t size) {
    (void)path; (void)data; (void)size; return -ENOSYS;
}
int vfs_read_file(const char* path, void* buffer, size_t max_size, size_t* out_size) {
    (void)path; (void)buffer; (void)max_size; if (out_size) *out_size = 0; return -ENOSYS;
}
const void* vfs_read_file_cstr(const char* path, size_t* out_size) {
    (void)path;
    if (out_size) *out_size = 0;
    return NULL;
}
int vfs_remove(const char* path, bool recursive) {
    (void)path; (void)recursive; return -ENOSYS;
}
bool vfs_exists(const char* path) {
    (void)path; return false;
}
vfs_node_type_t vfs_type(const char* path) {
    (void)path; return VFS_NODE_FILE;
}
int vfs_list_dir(const char* path, int (*cb)(const char*, vfs_node_type_t, void*), void* user) {
    (void)path; (void)cb; (void)user; return -ENOSYS;
}

int ucfs_set_backing_root(const char* path) { (void)path; return -ENOSYS; }
int ucfs_write_file_uc(const char* path, const void* data, size_t size) {
    (void)path; (void)data; (void)size; return -ENOSYS;
}
int ucfs_read_file_uc(const char* path, void* buffer, size_t max_size, size_t* out_size) {
    (void)path; (void)buffer; (void)max_size; if (out_size) *out_size = 0; return -ENOSYS;
}
int ucfs_exists_uc(const char* path) { (void)path; return 0; }
int ucfs_resolve_path(const char* ucfs_path, char* buffer, size_t buflen) {
    (void)ucfs_path; if (buffer && buflen) buffer[0] = '\0'; return -ENOSYS;
}
const char* ucfs_prompt_form(const char* canonical_path, char* buffer, size_t buflen) {
    (void)canonical_path; if (buffer && buflen) buffer[0] = '\0'; return NULL;
}

int ucfs_parse(const char* utf8_path, ucfs_path_t* out_path) {
    (void)utf8_path; (void)out_path; return -ENOSYS;
}
void ucfs_free(ucfs_path_t* path) {
    (void)path;
}
int ucfs_to_canonical(const ucfs_path_t* path, char* buffer, size_t buflen) {
    (void)path; if (buffer && buflen) buffer[0] = '\0'; return -ENOSYS;
}
int ucfs_to_canonical_with_base(const ucfs_path_t* path, const char* base, char* buffer, size_t buflen) {
    (void)path; (void)base; if (buffer && buflen) buffer[0] = '\0'; return -ENOSYS;
}

const ucfs_delimiter_config_t* ucfs_config_get_delimiter(
    const ucfs_config_t* config, uint32_t delimiter) {
    (void)config; (void)delimiter; return NULL;
}
int ucfs_config_set_delimiter(ucfs_config_t* config,
                               const ucfs_delimiter_config_t* delim_config) {
    (void)config; (void)delim_config; return -ENOSYS;
}
int ucfs_config_load(const char* config_path, ucfs_config_t* config) {
    (void)config_path;
    if (config) memset(config, 0, sizeof(*config));
    return -ENOSYS;
}
int ucfs_config_save(const char* config_path, const ucfs_config_t* config) {
    (void)config_path; (void)config; return -ENOSYS;
}
void ucfs_config_init_defaults(ucfs_config_t* config) {
    if (config) {
        memset(config, 0, sizeof(*config));
    }
}

#endif /* !CONFIG_FS */
