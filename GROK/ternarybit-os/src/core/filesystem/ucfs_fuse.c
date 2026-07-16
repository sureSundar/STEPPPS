/**
 * @file ucfs_fuse.c
 * @brief UCFS FUSE Filesystem Implementation
 *
 * Unicode Character FileSystem - uses Unicode characters as path roots.
 * Example: 🏠/documents/file.txt -> /ucfs/U+1F3E0/documents/file.txt
 *
 * @version 1.0
 * @date 2025
 */

#define FUSE_USE_VERSION 26

#ifdef __APPLE__
#include <fuse.h>
#else
#include <fuse.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <pthread.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * UCFS STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

#define UCFS_MAX_PATH 4096
#define UCFS_MAX_DELIMITERS 64

/**
 * @brief Delimiter configuration
 */
typedef struct {
    uint32_t codepoint;
    char utf8[8];
    size_t utf8_len;
    char backing_path[256];
    char description[128];
    int active;
} ucfs_delimiter_t;

/**
 * @brief UCFS filesystem context
 */
typedef struct {
    char default_backing[256];
    char default_delimiter[8];
    size_t default_delimiter_len;
    ucfs_delimiter_t delimiters[UCFS_MAX_DELIMITERS];
    size_t delimiter_count;
    pthread_mutex_t lock;
} ucfs_fs_t;

static ucfs_fs_t* g_fs = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * PATH TRANSLATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Convert UCFS virtual path to backing path (simple passthrough)
 * /docs/file.txt -> /backing/docs/file.txt
 */
static int ucfs_to_backing(const char* path, char* out, size_t out_len) {
    if (!path || !out || out_len == 0) return -EINVAL;

    size_t backing_len = strlen(g_fs->default_backing);
    size_t path_len = strlen(path);

    /* For root, just return backing path */
    if (strcmp(path, "/") == 0) {
        if (backing_len >= out_len) return -ENAMETOOLONG;
        strcpy(out, g_fs->default_backing);
        return 0;
    }

    /* Concatenate: backing + path */
    if (backing_len + path_len >= out_len) return -ENAMETOOLONG;

    strcpy(out, g_fs->default_backing);
    strcat(out, path);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * FUSE OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int ucfs_getattr(const char* path, struct stat* stbuf) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (lstat(backing, stbuf) == -1) {
        return -errno;
    }

    return 0;
}

static int ucfs_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info* fi) {
    (void)offset;
    (void)fi;

    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    DIR* dir = opendir(backing);
    if (!dir) {
        return -errno;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        filler(buf, entry->d_name, NULL, 0);
    }

    closedir(dir);
    return 0;
}

static int ucfs_open(const char* path, struct fuse_file_info* fi) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    int fd = open(backing, fi->flags);
    if (fd == -1) return -errno;

    fi->fh = fd;
    return 0;
}

static int ucfs_read(const char* path, char* buf, size_t size, off_t offset,
                     struct fuse_file_info* fi) {
    (void)path;

    ssize_t res = pread(fi->fh, buf, size, offset);
    if (res == -1) return -errno;

    return (int)res;
}

static int ucfs_write(const char* path, const char* buf, size_t size, off_t offset,
                      struct fuse_file_info* fi) {
    (void)path;

    ssize_t res = pwrite(fi->fh, buf, size, offset);
    if (res == -1) return -errno;

    return (int)res;
}

static int ucfs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    int fd = open(backing, fi->flags | O_CREAT, mode);
    if (fd == -1) return -errno;

    fi->fh = fd;
    return 0;
}

static int ucfs_mkdir(const char* path, mode_t mode) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (mkdir(backing, mode) == -1) return -errno;

    return 0;
}

static int ucfs_rmdir(const char* path) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (rmdir(backing) == -1) return -errno;

    return 0;
}

static int ucfs_unlink(const char* path) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (unlink(backing) == -1) return -errno;

    return 0;
}

static int ucfs_truncate(const char* path, off_t size) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (truncate(backing, size) == -1) return -errno;

    return 0;
}

static int ucfs_release(const char* path, struct fuse_file_info* fi) {
    (void)path;
    close(fi->fh);
    return 0;
}

static int ucfs_chmod(const char* path, mode_t mode) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (chmod(backing, mode) == -1) return -errno;

    return 0;
}

static int ucfs_chown(const char* path, uid_t uid, gid_t gid) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (chown(backing, uid, gid) == -1) return -errno;

    return 0;
}

static int ucfs_utimens(const char* path, const struct timespec tv[2]) {
    char backing[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    struct timeval times[2];
    times[0].tv_sec = tv[0].tv_sec;
    times[0].tv_usec = tv[0].tv_nsec / 1000;
    times[1].tv_sec = tv[1].tv_sec;
    times[1].tv_usec = tv[1].tv_nsec / 1000;

    if (utimes(backing, times) == -1) return -errno;

    return 0;
}

static int ucfs_rename(const char* from, const char* to) {
    char backing_from[UCFS_MAX_PATH];
    char backing_to[UCFS_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = ucfs_to_backing(from, backing_from, sizeof(backing_from));
    if (ret == 0) {
        ret = ucfs_to_backing(to, backing_to, sizeof(backing_to));
    }
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (rename(backing_from, backing_to) == -1) return -errno;

    return 0;
}

static int ucfs_statfs(const char* path, struct statvfs* stbuf) {
    (void)path;

    if (statvfs(g_fs->default_backing, stbuf) == -1) return -errno;

    return 0;
}

static void* ucfs_init(struct fuse_conn_info* conn) {
    (void)conn;
    return g_fs;
}

static void ucfs_destroy(void* private_data) {
    ucfs_fs_t* fs = (ucfs_fs_t*)private_data;
    if (fs) {
        pthread_mutex_destroy(&fs->lock);
        free(fs);
        g_fs = NULL;
    }
}

static struct fuse_operations ucfs_ops = {
    .init       = ucfs_init,
    .destroy    = ucfs_destroy,
    .getattr    = ucfs_getattr,
    .readdir    = ucfs_readdir,
    .open       = ucfs_open,
    .read       = ucfs_read,
    .write      = ucfs_write,
    .create     = ucfs_create,
    .mkdir      = ucfs_mkdir,
    .rmdir      = ucfs_rmdir,
    .unlink     = ucfs_unlink,
    .truncate   = ucfs_truncate,
    .release    = ucfs_release,
    .chmod      = ucfs_chmod,
    .chown      = ucfs_chown,
    .utimens    = ucfs_utimens,
    .rename     = ucfs_rename,
    .statfs     = ucfs_statfs,
};

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_usage(const char* prog) {
    fprintf(stderr, "Usage: %s [options] <mountpoint>\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -b <path>      Backing storage directory (default: /var/lib/ucfs)\n");
    fprintf(stderr, "  -d <char>      Delimiter character (default: middle dot '·')\n");
    fprintf(stderr, "  -f             Foreground mode\n");
    fprintf(stderr, "  -D             Debug mode\n");
    fprintf(stderr, "  -h             Show this help\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "UCFS - Unicode Character FileSystem\n");
    fprintf(stderr, "Uses Unicode characters as visual path delimiters.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s -b /var/lib/ucfs /mnt/ucfs\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Then access files as:\n");
    fprintf(stderr, "  echo hello > /mnt/ucfs/documents/file.txt\n");
}

int main(int argc, char* argv[]) {
    char* backing = "/var/lib/ucfs";
    char* delimiter = "·";  /* Middle dot U+00B7 */
    int foreground = 0;
    int debug = 0;

    int opt;
    while ((opt = getopt(argc, argv, "b:d:fDh")) != -1) {
        switch (opt) {
            case 'b':
                backing = optarg;
                break;
            case 'd':
                delimiter = optarg;
                break;
            case 'f':
                foreground = 1;
                break;
            case 'D':
                debug = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Error: mountpoint required\n");
        print_usage(argv[0]);
        return 1;
    }

    char* mountpoint = argv[optind];

    /* Create UCFS context */
    g_fs = calloc(1, sizeof(ucfs_fs_t));
    if (!g_fs) {
        fprintf(stderr, "Error: out of memory\n");
        return 1;
    }

    strncpy(g_fs->default_backing, backing, sizeof(g_fs->default_backing) - 1);

    /* Copy delimiter */
    size_t delim_len = strlen(delimiter);
    if (delim_len >= sizeof(g_fs->default_delimiter)) {
        fprintf(stderr, "Error: delimiter too long\n");
        free(g_fs);
        return 1;
    }
    strcpy(g_fs->default_delimiter, delimiter);
    g_fs->default_delimiter_len = delim_len;

    pthread_mutex_init(&g_fs->lock, NULL);

    /* Ensure backing directory exists */
    mkdir(backing, 0755);

    printf("UCFS mounted:\n");
    printf("  Mountpoint: %s\n", mountpoint);
    printf("  Backing:    %s\n", backing);
    printf("  Delimiter:  %s\n", delimiter);

    /* Build FUSE arguments */
    char* fuse_argv[16];
    int fuse_argc = 0;

    fuse_argv[fuse_argc++] = argv[0];
    fuse_argv[fuse_argc++] = mountpoint;

    if (foreground) {
        fuse_argv[fuse_argc++] = "-f";
    }
    if (debug) {
        fuse_argv[fuse_argc++] = "-d";
    }

    fuse_argv[fuse_argc++] = "-o";
    fuse_argv[fuse_argc++] = "allow_other";

    return fuse_main(fuse_argc, fuse_argv, &ucfs_ops, g_fs);
}
