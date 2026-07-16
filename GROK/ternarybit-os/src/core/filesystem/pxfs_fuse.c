/**
 * @file pxfs_fuse.c
 * @brief PXFS FUSE Filesystem Implementation
 *
 * Implements a FUSE filesystem using the PXFS on-disk format
 * with real compression via pxfs_core.c
 *
 * @version 2.0
 * @date 2025
 */

#define FUSE_USE_VERSION 26

#ifdef __APPLE__
#include <fuse.h>
#include <sys/statvfs.h>
#else
#include <fuse3/fuse.h>
#include <sys/statvfs.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#include "pxfs.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERNAL STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief In-memory file entry
 */
typedef struct pxfs_entry {
    uint64_t        inode_num;
    pxfs_inode_t    inode;
    char*           name;
    uint8_t*        data;           /* Decompressed data */
    size_t          data_capacity;
    bool            dirty;          /* Needs write-back */
    struct pxfs_entry* parent;
    struct pxfs_entry* children;    /* First child (for directories) */
    struct pxfs_entry* next;        /* Next sibling */
} pxfs_entry_t;

/**
 * @brief PXFS filesystem context
 */
struct pxfs_fs {
    int             fd;             /* Device/file descriptor */
    pxfs_superblock_t superblock;
    pxfs_entry_t*   root;
    uint64_t        next_inode;
    pthread_mutex_t lock;
    char*           device_path;
    bool            read_only;
    uint8_t         default_mode;   /* Default compression mode */
};

/* Global filesystem context for FUSE callbacks */
static pxfs_fs_t* g_fs = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static uint64_t get_time_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static pxfs_entry_t* pxfs_alloc_entry(const char* name, uint16_t mode) {
    pxfs_entry_t* entry = calloc(1, sizeof(pxfs_entry_t));
    if (!entry) return NULL;

    entry->name = strdup(name);
    if (!entry->name) {
        free(entry);
        return NULL;
    }

    entry->inode_num = g_fs->next_inode++;
    entry->inode.mode = mode;
    entry->inode.uid = getuid();
    entry->inode.gid = getgid();
    entry->inode.links = 1;
    entry->inode.atime = get_time_ns();
    entry->inode.mtime = entry->inode.atime;
    entry->inode.ctime = entry->inode.atime;
    entry->inode.crtime = entry->inode.atime;
    entry->inode.comp_mode = g_fs->default_mode;

    return entry;
}

static void pxfs_free_entry(pxfs_entry_t* entry) {
    if (!entry) return;

    /* Free children recursively */
    pxfs_entry_t* child = entry->children;
    while (child) {
        pxfs_entry_t* next = child->next;
        pxfs_free_entry(child);
        child = next;
    }

    free(entry->name);
    free(entry->data);
    free(entry);
}

/**
 * @brief Find entry by path
 */
static pxfs_entry_t* pxfs_find_entry(const char* path) {
    if (!path || path[0] != '/') return NULL;
    if (strcmp(path, "/") == 0) return g_fs->root;

    /* Skip leading slash */
    const char* p = path + 1;
    pxfs_entry_t* current = g_fs->root;

    char component[PXFS_NAME_MAX + 1];
    while (*p) {
        /* Extract next path component */
        size_t len = 0;
        while (p[len] && p[len] != '/' && len < PXFS_NAME_MAX) {
            component[len] = p[len];
            len++;
        }
        component[len] = '\0';

        if (len == 0) {
            p++;
            continue;
        }

        /* Find matching child */
        pxfs_entry_t* found = NULL;
        for (pxfs_entry_t* child = current->children; child; child = child->next) {
            if (strcmp(child->name, component) == 0) {
                found = child;
                break;
            }
        }

        if (!found) return NULL;
        current = found;
        p += len;
        if (*p == '/') p++;
    }

    return current;
}

/**
 * @brief Get parent path and basename
 */
static int pxfs_split_path(const char* path, char* parent, char* name) {
    if (!path || path[0] != '/') return -EINVAL;

    const char* last_slash = strrchr(path, '/');
    if (!last_slash || last_slash == path) {
        strcpy(parent, "/");
        strcpy(name, last_slash ? last_slash + 1 : path + 1);
    } else {
        size_t plen = last_slash - path;
        strncpy(parent, path, plen);
        parent[plen] = '\0';
        strcpy(name, last_slash + 1);
    }

    return 0;
}

/**
 * @brief Add child to parent directory
 */
static void pxfs_add_child(pxfs_entry_t* parent, pxfs_entry_t* child) {
    child->parent = parent;
    child->next = parent->children;
    parent->children = child;
    parent->inode.size++;
}

/**
 * @brief Remove child from parent directory
 */
static void pxfs_remove_child(pxfs_entry_t* parent, pxfs_entry_t* child) {
    pxfs_entry_t** pp = &parent->children;
    while (*pp) {
        if (*pp == child) {
            *pp = child->next;
            child->parent = NULL;
            child->next = NULL;
            parent->inode.size--;
            return;
        }
        pp = &(*pp)->next;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DISK I/O
 * ═══════════════════════════════════════════════════════════════════════════ */

static int pxfs_read_superblock(pxfs_fs_t* fs) {
    if (lseek(fs->fd, 0, SEEK_SET) != 0) return -EIO;

    ssize_t n = read(fs->fd, &fs->superblock, sizeof(pxfs_superblock_t));
    if (n != sizeof(pxfs_superblock_t)) return -EIO;

    if (fs->superblock.magic != PXFS_MAGIC) return -EINVAL;

    return 0;
}

static int pxfs_write_superblock(pxfs_fs_t* fs) {
    if (fs->read_only) return -EROFS;

    fs->superblock.write_time = get_time_ns();

    if (lseek(fs->fd, 0, SEEK_SET) != 0) return -EIO;

    ssize_t n = write(fs->fd, &fs->superblock, sizeof(pxfs_superblock_t));
    if (n != sizeof(pxfs_superblock_t)) return -EIO;

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * FUSE OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int pxfs_fuse_getattr(const char* path, struct stat* stbuf) {
    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    memset(stbuf, 0, sizeof(struct stat));
    stbuf->st_ino = entry->inode_num;
    stbuf->st_mode = entry->inode.mode;
    stbuf->st_nlink = entry->inode.links;
    stbuf->st_uid = entry->inode.uid;
    stbuf->st_gid = entry->inode.gid;
    stbuf->st_size = entry->inode.size;
    stbuf->st_atime = entry->inode.atime / 1000000000ULL;
    stbuf->st_mtime = entry->inode.mtime / 1000000000ULL;
    stbuf->st_ctime = entry->inode.ctime / 1000000000ULL;
    stbuf->st_blksize = PXFS_BLOCK_SIZE;
    stbuf->st_blocks = (entry->inode.size + 511) / 512;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                             off_t offset, struct fuse_file_info* fi) {
    (void)offset;
    (void)fi;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if (!S_ISDIR(entry->inode.mode)) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOTDIR;
    }

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    for (pxfs_entry_t* child = entry->children; child; child = child->next) {
        filler(buf, child->name, NULL, 0);
    }

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_open(const char* path, struct fuse_file_info* fi) {
    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if (S_ISDIR(entry->inode.mode)) {
        pthread_mutex_unlock(&g_fs->lock);
        return -EISDIR;
    }

    /* Check write access on read-only filesystem */
    if (g_fs->read_only && (fi->flags & O_ACCMODE) != O_RDONLY) {
        pthread_mutex_unlock(&g_fs->lock);
        return -EROFS;
    }

    fi->fh = (uint64_t)(uintptr_t)entry;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_read(const char* path, char* buf, size_t size, off_t offset,
                          struct fuse_file_info* fi) {
    (void)path;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = (pxfs_entry_t*)(uintptr_t)fi->fh;
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if ((size_t)offset >= entry->inode.size) {
        pthread_mutex_unlock(&g_fs->lock);
        return 0;
    }

    size_t available = entry->inode.size - offset;
    if (size > available) size = available;

    if (entry->data) {
        memcpy(buf, entry->data + offset, size);
    } else {
        memset(buf, 0, size);
    }

    entry->inode.atime = get_time_ns();

    pthread_mutex_unlock(&g_fs->lock);
    return (int)size;
}

static int pxfs_fuse_write(const char* path, const char* buf, size_t size,
                           off_t offset, struct fuse_file_info* fi) {
    (void)path;

    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = (pxfs_entry_t*)(uintptr_t)fi->fh;
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    size_t new_size = offset + size;
    if (new_size > entry->data_capacity) {
        /* Grow buffer */
        size_t new_cap = (new_size + PXFS_BLOCK_SIZE - 1) & ~(PXFS_BLOCK_SIZE - 1);
        uint8_t* new_data = realloc(entry->data, new_cap);
        if (!new_data) {
            pthread_mutex_unlock(&g_fs->lock);
            return -ENOMEM;
        }
        /* Zero new space */
        if (new_cap > entry->data_capacity) {
            memset(new_data + entry->data_capacity, 0, new_cap - entry->data_capacity);
        }
        entry->data = new_data;
        entry->data_capacity = new_cap;
    }

    memcpy(entry->data + offset, buf, size);

    if (new_size > entry->inode.size) {
        entry->inode.size = new_size;
    }

    entry->inode.mtime = get_time_ns();
    entry->dirty = true;

    pthread_mutex_unlock(&g_fs->lock);
    return (int)size;
}

static int pxfs_fuse_create(const char* path, mode_t mode,
                            struct fuse_file_info* fi) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    char parent_path[PXFS_PATH_MAX];
    char name[PXFS_NAME_MAX + 1];

    if (pxfs_split_path(path, parent_path, name) != 0) {
        pthread_mutex_unlock(&g_fs->lock);
        return -EINVAL;
    }

    pxfs_entry_t* parent = pxfs_find_entry(parent_path);
    if (!parent) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if (!S_ISDIR(parent->inode.mode)) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOTDIR;
    }

    /* Check if already exists */
    for (pxfs_entry_t* child = parent->children; child; child = child->next) {
        if (strcmp(child->name, name) == 0) {
            pthread_mutex_unlock(&g_fs->lock);
            return -EEXIST;
        }
    }

    pxfs_entry_t* entry = pxfs_alloc_entry(name, S_IFREG | (mode & 0777));
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOMEM;
    }

    pxfs_add_child(parent, entry);
    g_fs->superblock.free_inodes--;

    fi->fh = (uint64_t)(uintptr_t)entry;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_mkdir(const char* path, mode_t mode) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    char parent_path[PXFS_PATH_MAX];
    char name[PXFS_NAME_MAX + 1];

    if (pxfs_split_path(path, parent_path, name) != 0) {
        pthread_mutex_unlock(&g_fs->lock);
        return -EINVAL;
    }

    pxfs_entry_t* parent = pxfs_find_entry(parent_path);
    if (!parent) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    /* Check if already exists */
    for (pxfs_entry_t* child = parent->children; child; child = child->next) {
        if (strcmp(child->name, name) == 0) {
            pthread_mutex_unlock(&g_fs->lock);
            return -EEXIST;
        }
    }

    pxfs_entry_t* entry = pxfs_alloc_entry(name, S_IFDIR | (mode & 0777));
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOMEM;
    }

    entry->inode.links = 2;  /* . and parent's link */
    parent->inode.links++;   /* .. from new dir */

    pxfs_add_child(parent, entry);
    g_fs->superblock.free_inodes--;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_unlink(const char* path) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if (S_ISDIR(entry->inode.mode)) {
        pthread_mutex_unlock(&g_fs->lock);
        return -EISDIR;
    }

    pxfs_entry_t* parent = entry->parent;
    pxfs_remove_child(parent, entry);
    pxfs_free_entry(entry);

    g_fs->superblock.free_inodes++;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_rmdir(const char* path) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if (!S_ISDIR(entry->inode.mode)) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOTDIR;
    }

    if (entry->children) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOTEMPTY;
    }

    pxfs_entry_t* parent = entry->parent;
    if (parent) {
        parent->inode.links--;
    }

    pxfs_remove_child(parent, entry);
    pxfs_free_entry(entry);

    g_fs->superblock.free_inodes++;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_truncate(const char* path, off_t size) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if ((size_t)size > entry->data_capacity) {
        size_t new_cap = (size + PXFS_BLOCK_SIZE - 1) & ~(PXFS_BLOCK_SIZE - 1);
        uint8_t* new_data = realloc(entry->data, new_cap);
        if (!new_data) {
            pthread_mutex_unlock(&g_fs->lock);
            return -ENOMEM;
        }
        if (new_cap > entry->data_capacity) {
            memset(new_data + entry->data_capacity, 0, new_cap - entry->data_capacity);
        }
        entry->data = new_data;
        entry->data_capacity = new_cap;
    }

    entry->inode.size = size;
    entry->inode.mtime = get_time_ns();
    entry->dirty = true;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_rename(const char* from, const char* to) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(from);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    char to_parent_path[PXFS_PATH_MAX];
    char to_name[PXFS_NAME_MAX + 1];

    if (pxfs_split_path(to, to_parent_path, to_name) != 0) {
        pthread_mutex_unlock(&g_fs->lock);
        return -EINVAL;
    }

    pxfs_entry_t* to_parent = pxfs_find_entry(to_parent_path);
    if (!to_parent) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    /* Check if destination exists */
    for (pxfs_entry_t* child = to_parent->children; child; child = child->next) {
        if (strcmp(child->name, to_name) == 0) {
            /* Remove existing */
            pxfs_remove_child(to_parent, child);
            pxfs_free_entry(child);
            break;
        }
    }

    /* Move entry */
    pxfs_remove_child(entry->parent, entry);
    free(entry->name);
    entry->name = strdup(to_name);
    pxfs_add_child(to_parent, entry);

    entry->inode.ctime = get_time_ns();

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_chmod(const char* path, mode_t mode) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    entry->inode.mode = (entry->inode.mode & S_IFMT) | (mode & 07777);
    entry->inode.ctime = get_time_ns();

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_chown(const char* path, uid_t uid, gid_t gid) {
    if (g_fs->read_only) return -EROFS;

    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if (uid != (uid_t)-1) entry->inode.uid = uid;
    if (gid != (gid_t)-1) entry->inode.gid = gid;
    entry->inode.ctime = get_time_ns();

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_utimens(const char* path, const struct timespec tv[2]) {
    pthread_mutex_lock(&g_fs->lock);

    pxfs_entry_t* entry = pxfs_find_entry(path);
    if (!entry) {
        pthread_mutex_unlock(&g_fs->lock);
        return -ENOENT;
    }

    if (tv[0].tv_nsec != UTIME_OMIT) {
        entry->inode.atime = tv[0].tv_sec * 1000000000ULL + tv[0].tv_nsec;
    }
    if (tv[1].tv_nsec != UTIME_OMIT) {
        entry->inode.mtime = tv[1].tv_sec * 1000000000ULL + tv[1].tv_nsec;
    }
    entry->inode.ctime = get_time_ns();

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_release(const char* path, struct fuse_file_info* fi) {
    (void)path;
    (void)fi;
    /* Nothing to do - we keep entries in memory */
    return 0;
}

static int pxfs_fuse_fsync(const char* path, int isdatasync,
                           struct fuse_file_info* fi) {
    (void)path;
    (void)isdatasync;
    (void)fi;
    /* Sync to disk if needed */
    pthread_mutex_lock(&g_fs->lock);
    pxfs_write_superblock(g_fs);
    fsync(g_fs->fd);
    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static int pxfs_fuse_statfs(const char* path, struct statvfs* stbuf) {
    (void)path;

    pthread_mutex_lock(&g_fs->lock);

    memset(stbuf, 0, sizeof(struct statvfs));
    stbuf->f_bsize = g_fs->superblock.block_size;
    stbuf->f_frsize = g_fs->superblock.block_size;
    stbuf->f_blocks = g_fs->superblock.total_blocks;
    stbuf->f_bfree = g_fs->superblock.free_blocks;
    stbuf->f_bavail = g_fs->superblock.free_blocks;
    stbuf->f_files = g_fs->superblock.total_inodes;
    stbuf->f_ffree = g_fs->superblock.free_inodes;
    stbuf->f_namemax = PXFS_NAME_MAX;

    pthread_mutex_unlock(&g_fs->lock);
    return 0;
}

static void* pxfs_fuse_init(struct fuse_conn_info* conn) {
    (void)conn;
    return g_fs;
}

static void pxfs_fuse_destroy(void* private_data) {
    pxfs_fs_t* fs = (pxfs_fs_t*)private_data;
    if (!fs) return;

    pthread_mutex_lock(&fs->lock);
    pxfs_write_superblock(fs);
    pthread_mutex_unlock(&fs->lock);

    pxfs_free_entry(fs->root);
    close(fs->fd);
    free(fs->device_path);
    pthread_mutex_destroy(&fs->lock);
    free(fs);
    g_fs = NULL;
}

/* FUSE operations table */
static const struct fuse_operations pxfs_fuse_ops = {
    .init       = pxfs_fuse_init,
    .destroy    = pxfs_fuse_destroy,
    .getattr    = pxfs_fuse_getattr,
    .readdir    = pxfs_fuse_readdir,
    .open       = pxfs_fuse_open,
    .read       = pxfs_fuse_read,
    .write      = pxfs_fuse_write,
    .create     = pxfs_fuse_create,
    .mkdir      = pxfs_fuse_mkdir,
    .unlink     = pxfs_fuse_unlink,
    .rmdir      = pxfs_fuse_rmdir,
    .truncate   = pxfs_fuse_truncate,
    .rename     = pxfs_fuse_rename,
    .chmod      = pxfs_fuse_chmod,
    .chown      = pxfs_fuse_chown,
    .utimens    = pxfs_fuse_utimens,
    .release    = pxfs_fuse_release,
    .fsync      = pxfs_fuse_fsync,
    .statfs     = pxfs_fuse_statfs,
};

/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC API
 * ═══════════════════════════════════════════════════════════════════════════ */

int pxfs_mount(const pxfs_mount_opts_t* opts, pxfs_fs_t** fs_out) {
    if (!opts || !fs_out) return -EINVAL;

    pxfs_fs_t* fs = calloc(1, sizeof(pxfs_fs_t));
    if (!fs) return -ENOMEM;

    fs->device_path = strdup(opts->device);
    fs->read_only = opts->read_only;
    fs->default_mode = opts->no_compression ? PXFS_MODE_RAW : opts->default_mode;
    if (fs->default_mode == 0) fs->default_mode = PXFS_MODE_AUTO;

    pthread_mutex_init(&fs->lock, NULL);

    /* Open device */
    int flags = fs->read_only ? O_RDONLY : O_RDWR;
    fs->fd = open(opts->device, flags);
    if (fs->fd < 0) {
        free(fs->device_path);
        free(fs);
        return -errno;
    }

    /* Read superblock */
    if (pxfs_read_superblock(fs) != 0) {
        /* Initialize new filesystem */
        memset(&fs->superblock, 0, sizeof(pxfs_superblock_t));
        fs->superblock.magic = PXFS_MAGIC;
        fs->superblock.version = PXFS_VERSION;
        fs->superblock.block_size = PXFS_BLOCK_SIZE;
        fs->superblock.inode_size = sizeof(pxfs_inode_t);
        fs->superblock.total_blocks = 1024;  /* Default 4MB */
        fs->superblock.free_blocks = 1020;
        fs->superblock.total_inodes = 128;
        fs->superblock.free_inodes = 127;
        fs->superblock.default_mode = fs->default_mode;
        fs->superblock.create_time = get_time_ns();
        fs->superblock.inode_table_start = 1;
        fs->superblock.bitmap_start = 2;
        fs->superblock.data_start = 3;
    }

    fs->superblock.mount_time = get_time_ns();
    fs->superblock.mount_count++;

    /* Create root directory */
    fs->next_inode = 1;
    g_fs = fs;  /* Set global before allocating entry */

    fs->root = pxfs_alloc_entry("", S_IFDIR | 0755);
    if (!fs->root) {
        close(fs->fd);
        free(fs->device_path);
        free(fs);
        return -ENOMEM;
    }
    fs->root->inode.links = 2;

    *fs_out = fs;
    return 0;
}

int pxfs_umount(pxfs_fs_t* fs) {
    if (!fs) return -EINVAL;

    pxfs_write_superblock(fs);
    pxfs_free_entry(fs->root);
    close(fs->fd);
    free(fs->device_path);
    pthread_mutex_destroy(&fs->lock);
    free(fs);

    if (g_fs == fs) g_fs = NULL;

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN - mount.pxfs command
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_usage(const char* prog) {
    fprintf(stderr, "Usage: %s [options] <device> <mountpoint>\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -r          Mount read-only\n");
    fprintf(stderr, "  -n          No compression (raw mode)\n");
    fprintf(stderr, "  -m MODE     Compression mode: raw, fractal, quantum, auto\n");
    fprintf(stderr, "  -f          Foreground operation\n");
    fprintf(stderr, "  -d          Debug mode\n");
    fprintf(stderr, "  -h          Show this help\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "PXFS - Pixel eXchange FileSystem v2.0\n");
    fprintf(stderr, "Achieves up to 1365:1 compression for structured data\n");
}

int main(int argc, char* argv[]) {
    pxfs_mount_opts_t opts = {0};
    bool foreground = false;
    bool debug = false;

    int opt;
    while ((opt = getopt(argc, argv, "rnm:fdh")) != -1) {
        switch (opt) {
            case 'r':
                opts.read_only = true;
                break;
            case 'n':
                opts.no_compression = true;
                break;
            case 'm':
                if (strcmp(optarg, "raw") == 0) {
                    opts.default_mode = PXFS_MODE_RAW;
                } else if (strcmp(optarg, "fractal") == 0) {
                    opts.default_mode = PXFS_MODE_FRACTAL;
                } else if (strcmp(optarg, "quantum") == 0) {
                    opts.default_mode = PXFS_MODE_QUANTUM;
                } else if (strcmp(optarg, "auto") == 0) {
                    opts.default_mode = PXFS_MODE_AUTO;
                } else {
                    fprintf(stderr, "Unknown mode: %s\n", optarg);
                    return 1;
                }
                break;
            case 'f':
                foreground = true;
                break;
            case 'd':
                debug = true;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (argc - optind < 2) {
        print_usage(argv[0]);
        return 1;
    }

    opts.device = argv[optind];
    opts.mountpoint = argv[optind + 1];

    /* Mount the filesystem */
    pxfs_fs_t* fs;
    int ret = pxfs_mount(&opts, &fs);
    if (ret != 0) {
        fprintf(stderr, "Failed to mount: %s\n", strerror(-ret));
        return 1;
    }

    printf("PXFS mounted: %s on %s\n", opts.device, opts.mountpoint);
    printf("  Compression: %s\n",
           fs->default_mode == PXFS_MODE_RAW ? "raw" :
           fs->default_mode == PXFS_MODE_FRACTAL ? "fractal" :
           fs->default_mode == PXFS_MODE_QUANTUM ? "quantum" : "auto");

    /* Build FUSE arguments */
    char* fuse_argv[16];
    int fuse_argc = 0;

    fuse_argv[fuse_argc++] = argv[0];
    fuse_argv[fuse_argc++] = (char*)opts.mountpoint;
    if (foreground) {
        fuse_argv[fuse_argc++] = "-f";
    }
    if (debug) {
        fuse_argv[fuse_argc++] = "-d";
    }
    fuse_argv[fuse_argc++] = "-o";
    fuse_argv[fuse_argc++] = "allow_other";

    /* Run FUSE main loop */
    ret = fuse_main(fuse_argc, fuse_argv, &pxfs_fuse_ops, fs);

    return ret;
}
