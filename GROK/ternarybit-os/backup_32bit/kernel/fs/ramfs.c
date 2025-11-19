#include "tbos/vfs.h"
#include "tbos/libc.h"
#include "tbos/errno.h"

typedef struct ramfs_node {
    char* name;
    vfs_node_type_t type;
    struct ramfs_node* parent;
    struct ramfs_node** children;
    size_t child_count;
    size_t child_capacity;
    uint8_t* data;
    size_t size;
    size_t capacity;
} ramfs_node_t;

typedef struct {
    ramfs_node_t* root;
} ramfs_ctx_t;

static ramfs_node_t* ramfs_node_create(const char* name, vfs_node_type_t type, ramfs_node_t* parent) {
    ramfs_node_t* node = malloc(sizeof(ramfs_node_t));
    if (!node) return NULL;
    node->name = strdup(name ? name : "");
    node->type = type;
    node->parent = parent;
    node->children = NULL;
    node->child_count = 0;
    node->child_capacity = 0;
    node->data = NULL;
    node->size = 0;
    node->capacity = 0;
    return node;
}

static void ramfs_attach_child(ramfs_node_t* parent, ramfs_node_t* child) {
    if (!parent || !child) return;
    if (parent->child_count == parent->child_capacity) {
        size_t new_cap = parent->child_capacity ? parent->child_capacity * 2 : 4;
        ramfs_node_t** new_children = realloc(parent->children, new_cap * sizeof(ramfs_node_t*));
        if (!new_children) return;
        parent->children = new_children;
        parent->child_capacity = new_cap;
    }
    parent->children[parent->child_count++] = child;
}

static void ramfs_detach_child(ramfs_node_t* parent, ramfs_node_t* child) {
    if (!parent || !child) return;
    for (size_t i = 0; i < parent->child_count; i++) {
        if (parent->children[i] == child) {
            parent->children[i] = parent->children[parent->child_count - 1];
            parent->child_count--;
            return;
        }
    }
}

static ramfs_node_t* ramfs_find_child(ramfs_node_t* parent, const char* name) {
    if (!parent) return NULL;
    for (size_t i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) {
            return parent->children[i];
        }
    }
    return NULL;
}

static ramfs_node_t* ramfs_traverse(ramfs_ctx_t* ctx, const char* path, bool create_dirs, ramfs_node_t** out_parent, const char** out_leaf) {
    if (!ctx || !ctx->root || !path || path[0] != '/') return NULL;
    if (strcmp(path, "/") == 0) {
        if (out_parent) *out_parent = NULL;
        if (out_leaf) *out_leaf = "/";
        return ctx->root;
    }

    char* temp = strdup(path);
    if (!temp) return NULL;

    ramfs_node_t* current = ctx->root;
    ramfs_node_t* parent = NULL;

    char* saveptr;
    char* token = strtok_r(temp, "/", &saveptr);
    char* next_token = NULL;

    while (token) {
        next_token = strtok_r(NULL, "/", &saveptr);
        parent = current;
        ramfs_node_t* child = ramfs_find_child(current, token);

        if (!child) {
            if (!create_dirs || next_token != NULL) {
                free(temp);
                return NULL;
            }
        }

        if (!child) {
            child = ramfs_node_create(token, VFS_NODE_DIR, current);
            if (!child) {
                free(temp);
                return NULL;
            }
            ramfs_attach_child(current, child);
        }

        current = child;
        token = next_token;
    }

    if (out_parent) *out_parent = parent;
    if (out_leaf) *out_leaf = current ? current->name : NULL;
    free(temp);
    return current;
}

static ramfs_node_t* ramfs_traverse_parent(ramfs_ctx_t* ctx, const char* path, char** leaf_name) {
    if (!path || path[0] != '/') return NULL;
    char* temp = strdup(path);
    if (!temp) return NULL;
    char* pos = strrchr(temp, '/');
    if (!pos) {
        free(temp);
        return NULL;
    }
    if (pos == temp) {
        *leaf_name = strdup(pos + 1);
        free(temp);
        return ctx->root;
    }
    *pos = '\0';
    char* leaf = strdup(pos + 1);
    ramfs_node_t* parent = ramfs_traverse(ctx, temp, false, NULL, NULL);
    *leaf_name = leaf;
    free(temp);
    return parent;
}

static int ramfs_mkdir(void* context, const char* path) {
    ramfs_ctx_t* ctx = (ramfs_ctx_t*)context;
    if (!ctx || !path || path[0] != '/') return -EINVAL;
    if (strcmp(path, "/") == 0) return 0;

    char* leaf = NULL;
    ramfs_node_t* parent = ramfs_traverse_parent(ctx, path, &leaf);
    if (!parent || !leaf) {
        free(leaf);
        return -ENOENT;
    }

    ramfs_node_t* existing = ramfs_find_child(parent, leaf);
    if (existing) {
        free(leaf);
        return existing->type == VFS_NODE_DIR ? 0 : -EEXIST;
    }

    ramfs_node_t* node = ramfs_node_create(leaf, VFS_NODE_DIR, parent);
    free(leaf);
    if (!node) return -ENOMEM;
    ramfs_attach_child(parent, node);
    return 0;
}

static int ramfs_ensure_parent_dirs(ramfs_ctx_t* ctx, const char* path) {
    if (!path || path[0] != '/') return -EINVAL;
    char* temp = strdup(path);
    if (!temp) return -ENOMEM;
    char* pos = strrchr(temp, '/');
    if (!pos) {
        free(temp);
        return -EINVAL;
    }
    if (pos == temp) {
        free(temp);
        return 0;
    }
    *pos = '\0';
    char* token = temp;
    while (*token == '/') token++;
    char* saveptr;
    char* component = strtok_r(token, "/", &saveptr);
    ramfs_node_t* current = ctx->root;
    while (component) {
        ramfs_node_t* child = ramfs_find_child(current, component);
        if (!child) {
            child = ramfs_node_create(component, VFS_NODE_DIR, current);
            if (!child) {
                free(temp);
                return -ENOMEM;
            }
            ramfs_attach_child(current, child);
        } else if (child->type != VFS_NODE_DIR) {
            free(temp);
            return -ENOTDIR;
        }
        current = child;
        component = strtok_r(NULL, "/", &saveptr);
    }
    free(temp);
    return 0;
}

static ramfs_node_t* ramfs_get_or_create_file(ramfs_ctx_t* ctx, const char* path, bool create) {
    if (!ctx || !path || path[0] != '/') return NULL;
    if (create) {
        if (ramfs_ensure_parent_dirs(ctx, path) != 0) return NULL;
    }
    char* leaf = NULL;
    ramfs_node_t* parent = ramfs_traverse_parent(ctx, path, &leaf);
    if (!parent || !leaf) {
        free(leaf);
        return NULL;
    }
    ramfs_node_t* file = ramfs_find_child(parent, leaf);
    if (!file && create) {
        file = ramfs_node_create(leaf, VFS_NODE_FILE, parent);
        if (file) {
            ramfs_attach_child(parent, file);
        }
    }
    free(leaf);
    if (file && file->type != VFS_NODE_FILE) return NULL;
    return file;
}

static int ramfs_write_file_impl(ramfs_ctx_t* ctx, const char* path, const void* data, size_t size, bool append) {
    ramfs_node_t* file = ramfs_get_or_create_file(ctx, path, true);
    if (!file) return -ENOENT;
    size_t new_size = append ? file->size + size : size;
    if (new_size > file->capacity) {
        size_t new_cap = file->capacity ? file->capacity * 2 : size;
        if (new_cap < new_size) new_cap = new_size;
        uint8_t* new_data = realloc(file->data, new_cap);
        if (!new_data) return -ENOMEM;
        file->data = new_data;
        file->capacity = new_cap;
    }
    if (append) {
        memcpy(file->data + file->size, data, size);
        file->size = new_size;
    } else {
        memcpy(file->data, data, size);
        file->size = size;
    }
    return 0;
}

static int ramfs_write_file(void* context, const char* path, const void* data, size_t size, bool append) {
    return ramfs_write_file_impl((ramfs_ctx_t*)context, path, data, size, append);
}

static int ramfs_read_file(void* context, const char* path, void* buffer, size_t max_size, size_t* out_size) {
    ramfs_ctx_t* ctx = (ramfs_ctx_t*)context;
    ramfs_node_t* file = ramfs_get_or_create_file(ctx, path, false);
    if (!file) return -ENOENT;
    if (file->type != VFS_NODE_FILE) return -EISDIR;
    size_t to_copy = file->size < max_size ? file->size : max_size;
    memcpy(buffer, file->data, to_copy);
    if (out_size) *out_size = file->size;
    return 0;
}

static const void* ramfs_read_file_cstr(void* context, const char* path, size_t* out_size) {
    ramfs_ctx_t* ctx = (ramfs_ctx_t*)context;
    ramfs_node_t* file = ramfs_get_or_create_file(ctx, path, false);
    if (!file || file->type != VFS_NODE_FILE) return NULL;
    if (out_size) *out_size = file->size;
    return file->data;
}

static void ramfs_free_node(ramfs_node_t* node) {
    if (!node) return;
    for (size_t i = 0; i < node->child_count; i++) {
        ramfs_free_node(node->children[i]);
    }
    free(node->children);
    free(node->data);
    free(node->name);
    free(node);
}

static int ramfs_remove(void* context, const char* path, bool recursive) {
    ramfs_ctx_t* ctx = (ramfs_ctx_t*)context;
    if (!ctx || !path || path[0] != '/') return -EINVAL;
    if (strcmp(path, "/") == 0) return -EPERM;
    ramfs_node_t* node = ramfs_traverse(ctx, path, false, NULL, NULL);
    if (!node) return -ENOENT;
    if (node->type == VFS_NODE_DIR && node->child_count > 0 && !recursive) {
        return -EBUSY;
    }
    if (node->parent) {
        ramfs_detach_child(node->parent, node);
    }
    ramfs_free_node(node);
    return 0;
}

static bool ramfs_exists(void* context, const char* path) {
    ramfs_ctx_t* ctx = (ramfs_ctx_t*)context;
    return ramfs_traverse(ctx, path, false, NULL, NULL) != NULL;
}

static vfs_node_type_t ramfs_type(void* context, const char* path) {
    ramfs_ctx_t* ctx = (ramfs_ctx_t*)context;
    ramfs_node_t* node = ramfs_traverse(ctx, path, false, NULL, NULL);
    return node ? node->type : VFS_NODE_FILE;
}

static int ramfs_list_dir(void* context, const char* path,
                          int (*cb)(const char*, vfs_node_type_t, void*), void* user) {
    ramfs_ctx_t* ctx = (ramfs_ctx_t*)context;
    ramfs_node_t* node = ramfs_traverse(ctx, path, false, NULL, NULL);
    if (!node) return -ENOENT;
    if (node->type != VFS_NODE_DIR) return -ENOTDIR;
    for (size_t i = 0; i < node->child_count; i++) {
        int res = cb(node->children[i]->name, node->children[i]->type, user);
        if (res != 0) return res;
    }
    return 0;
}

static void* ramfs_init_ctx(void) {
    ramfs_ctx_t* ctx = malloc(sizeof(ramfs_ctx_t));
    if (!ctx) return NULL;
    ctx->root = ramfs_node_create("/", VFS_NODE_DIR, NULL);
    if (!ctx->root) {
        free(ctx);
        return NULL;
    }
    return ctx;
}

const vfs_driver_t ramfs_driver = {
    .name = "ramfs",
    .init = ramfs_init_ctx,
    .mkdir = ramfs_mkdir,
    .write_file = ramfs_write_file,
    .read_file = ramfs_read_file,
    .read_file_cstr = ramfs_read_file_cstr,
    .remove = ramfs_remove,
    .exists = ramfs_exists,
    .type = ramfs_type,
    .list_dir = ramfs_list_dir
};
