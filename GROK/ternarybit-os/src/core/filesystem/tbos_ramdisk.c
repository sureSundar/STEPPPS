/**
 * @file tbos_ramdisk.c
 * @brief TernaryBit OS - In-Memory Ramdisk Filesystem Implementation
 */

#include "tbos_ramdisk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static bool g_fs_initialized = false;
static tbos_inode_t g_inodes[TBOS_MAX_FILES];
static uint32_t g_next_inode = 1;
static tbos_fd_t g_file_descriptors[TBOS_MAX_OPEN_FILES];
static tbos_directory_t* g_root_dir = NULL;
static size_t g_total_bytes_used = 0;
static size_t g_total_bytes_available = 0;

/* ========================================================================= */
/* HELPER UTILITIES                                                          */
/* ========================================================================= */

static bool normalize_path(const char* path, char* buffer, size_t buffer_len) {
    if (!path || !buffer || buffer_len == 0) {
        return false;
    }

    if (path[0] == '\0') {
        return false;
    }

    if (path[0] != '/') {
        return false;  /* expect absolute paths */
    }

    char temp[TBOS_MAX_PATH];
    if (strlen(path) >= sizeof(temp)) {
        return false;
    }
    strcpy(temp, path);

    char* segments[TBOS_MAX_PATH / 2];
    size_t depth = 0;
    char* save = NULL;
    char* token = strtok_r(temp, "/", &save);

    while (token) {
        if (strcmp(token, ".") == 0 || token[0] == '\0') {
            /* Ignore */
        } else if (strcmp(token, "..") == 0) {
            if (depth > 0) {
                depth--;
            }
        } else {
            segments[depth++] = token;
        }
        token = strtok_r(NULL, "/", &save);
    }

    if (depth == 0) {
        if (buffer_len < 2) {
            return false;
        }
        buffer[0] = '/';
        buffer[1] = '\0';
        return true;
    }

    size_t pos = 0;
    for (size_t i = 0; i < depth; i++) {
        size_t name_len = strlen(segments[i]);
        if (pos + name_len + 1 >= buffer_len) {
            return false;
        }
        buffer[pos++] = '/';
        memcpy(buffer + pos, segments[i], name_len);
        pos += name_len;
    }
    buffer[pos] = '\0';
    return true;
}

static tbos_inode_t* find_inode_exact(const char* normalized_path) {
    if (!normalized_path) return NULL;
    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num != 0 &&
            strcmp(g_inodes[i].path, normalized_path) == 0) {
            return &g_inodes[i];
        }
    }
    return NULL;
}

static bool split_parent_child(const char* normalized_path,
                               char* parent,
                               size_t parent_len,
                               char* child,
                               size_t child_len) {
    if (!normalized_path || !parent || !child) return false;
    size_t len = strlen(normalized_path);
    if (len == 0 || normalized_path[0] != '/') return false;
    if (strcmp(normalized_path, "/") == 0) {
        return false;  /* root has no parent */
    }

    const char* last = strrchr(normalized_path, '/');
    size_t parent_size = (last == normalized_path) ? 1 : (size_t)(last - normalized_path);
    if (parent_size >= parent_len) return false;
    if (last && last != normalized_path) {
        memcpy(parent, normalized_path, parent_size);
        parent[parent_size] = '\0';
    } else {
        parent[0] = '/';
        parent[1] = '\0';
    }

    const char* child_name = last ? last + 1 : normalized_path;
    if (strlen(child_name) + 1 > child_len) return false;
    strcpy(child, child_name);
    return true;
}

static bool extract_child_name(const char* parent,
                               const char* candidate,
                               char* out,
                               size_t out_len) {
    if (!parent || !candidate) return false;
    if (strcmp(parent, candidate) == 0) return false;

    size_t parent_len = strlen(parent);
    const char* prefix = parent;
    size_t prefix_len = parent_len;

    if (strcmp(parent, "/") == 0) {
        prefix = "/";
        prefix_len = 1;
    }

    if (strncmp(candidate, prefix, prefix_len) != 0) {
        return false;
    }

    const char* rest = candidate + prefix_len;
    if (strcmp(parent, "/") != 0) {
        if (rest[0] != '/') {
            return false;
        }
        rest++;
    }

    if (rest[0] == '\0') {
        return false;
    }

    const char* slash = strchr(rest, '/');
    if (slash) {
        return false;  /* deeper descendant */
    }

    if (out && out_len > 0) {
        if (strlen(rest) + 1 > out_len) return false;
        strcpy(out, rest);
    }
    return true;
}

/* ========================================================================= */
/* HELPER FUNCTIONS                                                          */
/* ========================================================================= */

/**
 * @brief Allocate new inode
 */
static tbos_inode_t* allocate_inode(uint8_t file_type) {
    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num == 0) {
            tbos_inode_t* inode = &g_inodes[i];
            memset(inode, 0, sizeof(tbos_inode_t));

            inode->inode_num = g_next_inode++;
            inode->file_type = file_type;
            inode->permissions = TBOS_PERM_READ | TBOS_PERM_WRITE;
            inode->created_time = time(NULL);
            inode->modified_time = inode->created_time;
            inode->ref_count = 1;
            inode->path[0] = '\0';

            return inode;
        }
    }

    return NULL;
}

/**
 * @brief Find free file descriptor
 */
static int allocate_fd(void) {
    for (int i = 0; i < TBOS_MAX_OPEN_FILES; i++) {
        if (!g_file_descriptors[i].in_use) {
            g_file_descriptors[i].in_use = true;
            return i;
        }
    }
    return -1;
}

/* ========================================================================= */
/* FILESYSTEM INITIALIZATION                                                 */
/* ========================================================================= */

int tbos_ramdisk_init(size_t size_bytes) {
    if (g_fs_initialized) {
        return 0;
    }

    printf("[RAMDISK] Initializing filesystem (size: %zu bytes)...\n", size_bytes);

    /* Clear all structures */
    memset(g_inodes, 0, sizeof(g_inodes));
    memset(g_file_descriptors, 0, sizeof(g_file_descriptors));

    g_next_inode = 1;
    g_total_bytes_available = size_bytes;
    g_total_bytes_used = 0;

    /* Create root directory */
    g_root_dir = malloc(sizeof(tbos_directory_t));
    if (!g_root_dir) {
        printf("[RAMDISK] Failed to allocate root directory\n");
        return -1;
    }

    memset(g_root_dir, 0, sizeof(tbos_directory_t));
    strcpy(g_root_dir->path, "/");
    g_root_dir->entries = NULL;
    g_root_dir->entry_count = 0;

    /* Allocate root inode */
    tbos_inode_t* root_inode = allocate_inode(TBOS_FT_DIRECTORY);
    if (!root_inode) {
        printf("[RAMDISK] Failed to allocate root inode\n");
        free(g_root_dir);
        return -1;
    }
    strcpy(root_inode->path, "/");
    g_root_dir->inode_num = root_inode->inode_num;

    g_fs_initialized = true;
    tbos_ramdisk_create_root_structure();
    printf("[RAMDISK] Filesystem initialized successfully\n");

    return 0;
}

int tbos_ramdisk_create_root_structure(void) {
    if (!g_fs_initialized) {
        tbos_ramdisk_init(1024 * 1024);  /* 1MB default */
    }

    printf("[RAMDISK] Creating root directory structure...\n");

    /* Create standard directories */
    const char* dirs[] = {"/bin", "/tmp", "/home", "/dev"};

    for (int i = 0; i < 4; i++) {
        if (tbos_mkdir(dirs[i]) == 0) {
            printf("[RAMDISK] Created %s\n", dirs[i]);
        }
    }

    printf("[RAMDISK] Root structure created\n");
    return 0;
}

/* ========================================================================= */
/* FILE OPERATIONS                                                           */
/* ========================================================================= */

int tbos_open(const char* path, uint32_t flags) {
    if (!g_fs_initialized) {
        tbos_ramdisk_init(1024 * 1024);
    }

    if (!path) return -1;
    char normalized[TBOS_MAX_PATH];
    if (!normalize_path(path, normalized, sizeof(normalized))) {
        return -1;
    }

    tbos_inode_t* inode = find_inode_exact(normalized);

    if (!inode) {
        if (!(flags & TBOS_O_CREAT)) {
            return -1;
        }

        char parent[TBOS_MAX_PATH];
        char name[TBOS_MAX_FILENAME];
        if (!split_parent_child(normalized, parent, sizeof(parent), name, sizeof(name))) {
            return -1;
        }
        tbos_inode_t* parent_inode = find_inode_exact(parent);
        if (!parent_inode || parent_inode->file_type != TBOS_FT_DIRECTORY) {
            return -1;
        }

        inode = allocate_inode(TBOS_FT_REGULAR);
        if (!inode) return -1;
        strncpy(inode->path, normalized, sizeof(inode->path) - 1);
        inode->path[sizeof(inode->path) - 1] = '\0';
        inode->file_size = 0;
        inode->data = NULL;
    }

    if (inode->file_type != TBOS_FT_REGULAR) {
        return -1;
    }

    int fd = allocate_fd();
    if (fd < 0) return -1;

    g_file_descriptors[fd].inode_num = inode->inode_num;
    g_file_descriptors[fd].flags = flags;
    g_file_descriptors[fd].position = 0;

    if (flags & TBOS_O_TRUNC) {
        if (inode->data) {
            g_total_bytes_used -= inode->file_size;
            free(inode->data);
        }
        inode->data = NULL;
        inode->file_size = 0;
    }

    inode->ref_count++;
    inode->access_count++;

    if (flags & TBOS_O_APPEND) {
        g_file_descriptors[fd].position = inode->file_size;
    }

    return fd;
}

int tbos_close(int fd) {
    if (fd < 0 || fd >= TBOS_MAX_OPEN_FILES) return -1;
    if (!g_file_descriptors[fd].in_use) return -1;

    /* Find inode and decrement ref count */
    uint32_t inode_num = g_file_descriptors[fd].inode_num;
    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num == inode_num) {
            g_inodes[i].ref_count--;
            break;
        }
    }

    /* Free file descriptor */
    memset(&g_file_descriptors[fd], 0, sizeof(tbos_fd_t));

    return 0;
}

int tbos_read(int fd, void* buffer, size_t count) {
    if (fd < 0 || fd >= TBOS_MAX_OPEN_FILES) return -1;
    if (!g_file_descriptors[fd].in_use) return -1;
    if (!buffer) return -1;

    /* Find inode */
    uint32_t inode_num = g_file_descriptors[fd].inode_num;
    tbos_inode_t* inode = NULL;

    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num == inode_num) {
            inode = &g_inodes[i];
            break;
        }
    }

    if (!inode || !inode->data) return 0;

    /* Calculate bytes to read */
    size_t pos = g_file_descriptors[fd].position;
    size_t available = inode->file_size - pos;
    size_t to_read = (count < available) ? count : available;

    if (to_read == 0) return 0;

    /* Copy data */
    memcpy(buffer, (char*)inode->data + pos, to_read);
    g_file_descriptors[fd].position += to_read;

    return (int)to_read;
}

int tbos_write(int fd, const void* buffer, size_t count) {
    if (fd < 0 || fd >= TBOS_MAX_OPEN_FILES) return -1;
    if (!g_file_descriptors[fd].in_use) return -1;
    if (!buffer) return -1;

    /* Find inode */
    uint32_t inode_num = g_file_descriptors[fd].inode_num;
    tbos_inode_t* inode = NULL;

    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num == inode_num) {
            inode = &g_inodes[i];
            break;
        }
    }

    if (!inode) return -1;

    /* Calculate new size */
    size_t pos = g_file_descriptors[fd].position;
    size_t new_size = pos + count;

    if (new_size > TBOS_MAX_FILE_SIZE) return -1;

    if (new_size > inode->file_size) {
        size_t old_size = inode->file_size;
        void* new_data = realloc(inode->data, new_size);
        if (!new_data) return -1;
        inode->data = new_data;
        inode->file_size = new_size;
        g_total_bytes_used += (new_size - old_size);
    }

    /* Write data */
    memcpy((char*)inode->data + pos, buffer, count);
    g_file_descriptors[fd].position += count;
    inode->modified_time = time(NULL);

    return (int)count;
}

int tbos_seek(int fd, int offset, int whence) {
    if (fd < 0 || fd >= TBOS_MAX_OPEN_FILES) return -1;
    if (!g_file_descriptors[fd].in_use) return -1;

    /* Find inode for file size */
    uint32_t inode_num = g_file_descriptors[fd].inode_num;
    tbos_inode_t* inode = NULL;

    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num == inode_num) {
            inode = &g_inodes[i];
            break;
        }
    }

    if (!inode) return -1;

    size_t new_pos;

    switch (whence) {
        case TBOS_SEEK_SET:
            new_pos = offset;
            break;
        case TBOS_SEEK_CUR:
            new_pos = g_file_descriptors[fd].position + offset;
            break;
        case TBOS_SEEK_END:
            new_pos = inode->file_size + offset;
            break;
        default:
            return -1;
    }

    if (new_pos > inode->file_size) return -1;

    g_file_descriptors[fd].position = new_pos;
    return (int)new_pos;
}

/* ========================================================================= */
/* DIRECTORY OPERATIONS                                                      */
/* ========================================================================= */

int tbos_mkdir(const char* path) {
    if (!g_fs_initialized) {
        tbos_ramdisk_init(1024 * 1024);
    }

    if (!path) return -1;

    char normalized[TBOS_MAX_PATH];
    if (!normalize_path(path, normalized, sizeof(normalized))) {
        return -1;
    }

    if (strcmp(normalized, "/") == 0) {
        return -1;  /* root already exists */
    }

    if (find_inode_exact(normalized)) {
        return -1;
    }

    char parent[TBOS_MAX_PATH];
    char name[TBOS_MAX_FILENAME];
    if (!split_parent_child(normalized, parent, sizeof(parent), name, sizeof(name))) {
        return -1;
    }

    tbos_inode_t* parent_inode = find_inode_exact(parent);
    if (!parent_inode || parent_inode->file_type != TBOS_FT_DIRECTORY) {
        return -1;
    }

    tbos_inode_t* inode = allocate_inode(TBOS_FT_DIRECTORY);
    if (!inode) return -1;

    strncpy(inode->path, normalized, sizeof(inode->path) - 1);
    inode->path[sizeof(inode->path) - 1] = '\0';
    return 0;
}

int tbos_rmdir(const char* path) {
    if (!path) return -1;

    if (!g_fs_initialized) {
        tbos_ramdisk_init(1024 * 1024);
    }

    char normalized[TBOS_MAX_PATH];
    if (!normalize_path(path, normalized, sizeof(normalized))) {
        return -1;
    }

    if (strcmp(normalized, "/") == 0) {
        return -1;  /* cannot remove root */
    }

    tbos_inode_t* inode = find_inode_exact(normalized);
    if (!inode || inode->file_type != TBOS_FT_DIRECTORY) {
        return -1;
    }

    /* Ensure directory empty */
    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num == 0) continue;
        char child_name[TBOS_MAX_FILENAME];
        if (extract_child_name(normalized, g_inodes[i].path, child_name, sizeof(child_name))) {
            return -1;  /* not empty */
        }
    }

    memset(inode, 0, sizeof(tbos_inode_t));
    return 0;
}

int tbos_unlink(const char* path) {
    if (!path) return -1;

    if (!g_fs_initialized) {
        tbos_ramdisk_init(1024 * 1024);
    }

    char normalized[TBOS_MAX_PATH];
    if (!normalize_path(path, normalized, sizeof(normalized))) {
        return -1;
    }

    tbos_inode_t* inode = find_inode_exact(normalized);
    if (!inode || inode->file_type != TBOS_FT_REGULAR) {
        return -1;
    }

    if (inode->data) {
        g_total_bytes_used -= inode->file_size;
        free(inode->data);
    }

    memset(inode, 0, sizeof(tbos_inode_t));
    return 0;
}

int tbos_readdir(const char* path, tbos_dirent_t* buffer, uint32_t max_entries) {
    if (!path) return -1;

    if (!g_fs_initialized) {
        tbos_ramdisk_init(1024 * 1024);
    }

    char normalized[TBOS_MAX_PATH];
    if (!normalize_path(path, normalized, sizeof(normalized))) {
        return -1;
    }

    tbos_inode_t* dir = find_inode_exact(normalized);
    if (!dir || dir->file_type != TBOS_FT_DIRECTORY) {
        return -1;
    }

    uint32_t total = 0;
    uint32_t stored = 0;
    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num == 0) continue;
        char child_name[TBOS_MAX_FILENAME];
        if (!extract_child_name(normalized, g_inodes[i].path, child_name, sizeof(child_name))) {
            continue;
        }
        if (buffer && stored < max_entries) {
            strncpy(buffer[stored].name, child_name, sizeof(buffer[stored].name) - 1);
            buffer[stored].name[sizeof(buffer[stored].name) - 1] = '\0';
            buffer[stored].inode_num = g_inodes[i].inode_num;
            stored++;
        }
        total++;
    }

    return (buffer ? (int)stored : (int)total);
}

/* ========================================================================= */
/* FILESYSTEM STATISTICS                                                     */
/* ========================================================================= */

int tbos_fs_get_stats(tbos_fs_stats_t* stats) {
    if (!stats) return -1;

    memset(stats, 0, sizeof(tbos_fs_stats_t));

    /* Count files and directories */
    for (uint32_t i = 0; i < TBOS_MAX_FILES; i++) {
        if (g_inodes[i].inode_num != 0) {
            if (g_inodes[i].file_type == TBOS_FT_REGULAR) {
                stats->total_files++;
            } else if (g_inodes[i].file_type == TBOS_FT_DIRECTORY) {
                stats->total_directories++;
            }
        }
    }

    /* Count open files */
    for (int i = 0; i < TBOS_MAX_OPEN_FILES; i++) {
        if (g_file_descriptors[i].in_use) {
            stats->open_files++;
        }
    }

    stats->total_bytes_used = g_total_bytes_used;
    stats->total_bytes_available = g_total_bytes_available;

    return 0;
}

void tbos_fs_print_status(void) {
    tbos_fs_stats_t stats;
    tbos_fs_get_stats(&stats);

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Filesystem Status                ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Files:                 %-10u                    ║\n",
           stats.total_files);
    printf("║ Directories:           %-10u                    ║\n",
           stats.total_directories);
    printf("║ Open Files:            %-10u                    ║\n",
           stats.open_files);
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Bytes Used:            %-10zu                    ║\n",
           stats.total_bytes_used);
    printf("║ Bytes Available:       %-10zu                    ║\n",
           stats.total_bytes_available);
    printf("║ Usage:                 %.1f%%                          ║\n",
           (float)stats.total_bytes_used / stats.total_bytes_available * 100.0f);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

void tbos_fs_print_tree(const char* path) {
    printf("Directory tree for: %s\n", path);
    printf("(Tree view not yet implemented)\n");
}
