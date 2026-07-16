/**
 * @file rf2s_fuse.c
 * @brief RF2S FUSE Filesystem Implementation
 *
 * Radio Frequency FileSystem - organizes data by radio frequency.
 * Example: /432MHz/sensor/data.bin -> /backing/432000000/sensor/data.bin
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
#include <sys/time.h>
#include <pthread.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * RF2S STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

#define RF2S_MAX_PATH 4096

/**
 * @brief RF2S filesystem context
 */
typedef struct {
    char backing_path[256];     /* Backing storage directory */
    uint64_t base_freq_hz;      /* Base frequency (default 432 MHz) */
    pthread_mutex_t lock;
} rf2s_fs_t;

static rf2s_fs_t* g_fs = NULL;

/* ═══════════════════════════════════════════════════════════════════════════
 * FREQUENCY PARSING (inline implementation)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

static int starts_with_ci(const char* str, const char* prefix) {
    while (*prefix) {
        if (to_lower(*str) != to_lower(*prefix)) return 0;
        str++;
        prefix++;
    }
    return 1;
}

/**
 * @brief Parse frequency string to Hz
 * @return frequency in Hz, or 0 on error
 */
static uint64_t parse_frequency(const char* str) {
    if (!str || !is_digit(*str)) return 0;

    /* Parse numeric part */
    double value = 0.0;
    while (is_digit(*str)) {
        value = value * 10.0 + (*str - '0');
        str++;
    }

    /* Handle decimal */
    if (*str == '.') {
        str++;
        double fraction = 0.1;
        while (is_digit(*str)) {
            value += (*str - '0') * fraction;
            fraction *= 0.1;
            str++;
        }
    }

    /* Skip whitespace */
    while (*str == ' ' || *str == '\t') str++;

    /* Parse unit */
    uint64_t multiplier = 1;
    if (is_alpha(*str)) {
        if (starts_with_ci(str, "ghz")) {
            multiplier = 1000000000ULL;
        } else if (starts_with_ci(str, "mhz")) {
            multiplier = 1000000ULL;
        } else if (starts_with_ci(str, "khz")) {
            multiplier = 1000ULL;
        } else if (starts_with_ci(str, "hz")) {
            multiplier = 1;
        } else {
            return 0;  /* Unknown unit */
        }
    }

    return (uint64_t)(value * (double)multiplier);
}

/**
 * @brief Check if string looks like a frequency
 */
static int is_frequency(const char* str) {
    if (!str || !is_digit(*str)) return 0;

    while (is_digit(*str) || *str == '.') str++;
    while (*str == ' ' || *str == '\t') str++;

    if (*str == '\0') return 1;

    return (starts_with_ci(str, "hz") ||
            starts_with_ci(str, "khz") ||
            starts_with_ci(str, "mhz") ||
            starts_with_ci(str, "ghz"));
}

/**
 * @brief Format frequency for display
 */
static void format_frequency(uint64_t hz, char* buf, size_t len) {
    if (hz >= 1000000000ULL) {
        double ghz = (double)hz / 1000000000.0;
        if (hz % 1000000000ULL == 0) {
            snprintf(buf, len, "%.0fGHz", ghz);
        } else {
            snprintf(buf, len, "%.1fGHz", ghz);
        }
    } else if (hz >= 1000000ULL) {
        double mhz = (double)hz / 1000000.0;
        if (hz % 1000000ULL == 0) {
            snprintf(buf, len, "%.0fMHz", mhz);
        } else {
            snprintf(buf, len, "%.1fMHz", mhz);
        }
    } else if (hz >= 1000ULL) {
        double khz = (double)hz / 1000.0;
        if (hz % 1000ULL == 0) {
            snprintf(buf, len, "%.0fkHz", khz);
        } else {
            snprintf(buf, len, "%.1fkHz", khz);
        }
    } else {
        snprintf(buf, len, "%lluHz", (unsigned long long)hz);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PATH TRANSLATION
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Convert RF2S path to backing path
 * /432MHz/data.txt -> /backing/432000000/data.txt
 */
static int rf2s_to_backing(const char* path, char* out, size_t out_len) {
    if (!path || !out || out_len == 0) return -EINVAL;

    /* Root directory */
    if (strcmp(path, "/") == 0) {
        strncpy(out, g_fs->backing_path, out_len - 1);
        out[out_len - 1] = '\0';
        return 0;
    }

    /* Skip leading slash */
    const char* p = path;
    if (*p == '/') p++;

    /* Find first component (should be frequency) */
    const char* slash = strchr(p, '/');
    size_t first_len = slash ? (size_t)(slash - p) : strlen(p);

    char first_comp[64];
    if (first_len >= sizeof(first_comp)) return -ENAMETOOLONG;
    memcpy(first_comp, p, first_len);
    first_comp[first_len] = '\0';

    /* Check if first component is a frequency */
    if (is_frequency(first_comp)) {
        uint64_t freq_hz = parse_frequency(first_comp);
        if (freq_hz == 0) return -EINVAL;

        /* Build path with normalized frequency */
        size_t pos = snprintf(out, out_len, "%s/%llu",
                              g_fs->backing_path, (unsigned long long)freq_hz);

        /* Append rest of path */
        if (slash) {
            size_t rest_len = strlen(slash);
            if (pos + rest_len >= out_len) return -ENAMETOOLONG;
            strcpy(out + pos, slash);
        }
    } else {
        /* Not a frequency - direct passthrough */
        size_t backing_len = strlen(g_fs->backing_path);
        size_t path_len = strlen(path);
        if (backing_len + path_len >= out_len) return -ENAMETOOLONG;
        strcpy(out, g_fs->backing_path);
        strcat(out, path);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * FUSE OPERATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int rf2s_getattr(const char* path, struct stat* stbuf) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (lstat(backing, stbuf) == -1) {
        return -errno;
    }

    return 0;
}

static int rf2s_readdir(const char* path, void* buf, fuse_fill_dir_t filler,
                        off_t offset, struct fuse_file_info* fi) {
    (void)offset;
    (void)fi;

    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
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

        /* For root directory, convert numeric dirs to frequency display */
        if (strcmp(path, "/") == 0) {
            /* Check if entry is a numeric directory (Hz value) */
            char* endptr;
            unsigned long long hz = strtoull(entry->d_name, &endptr, 10);
            if (*endptr == '\0' && hz > 0) {
                /* Format as human-readable frequency */
                char freq_name[64];
                format_frequency(hz, freq_name, sizeof(freq_name));
                filler(buf, freq_name, NULL, 0);
                continue;
            }
        }

        filler(buf, entry->d_name, NULL, 0);
    }

    closedir(dir);
    return 0;
}

static int rf2s_open(const char* path, struct fuse_file_info* fi) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    int fd = open(backing, fi->flags);
    if (fd == -1) return -errno;

    fi->fh = fd;
    return 0;
}

static int rf2s_read(const char* path, char* buf, size_t size, off_t offset,
                     struct fuse_file_info* fi) {
    (void)path;

    ssize_t res = pread(fi->fh, buf, size, offset);
    if (res == -1) return -errno;

    return (int)res;
}

static int rf2s_write(const char* path, const char* buf, size_t size, off_t offset,
                      struct fuse_file_info* fi) {
    (void)path;

    ssize_t res = pwrite(fi->fh, buf, size, offset);
    if (res == -1) return -errno;

    return (int)res;
}

static int rf2s_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    int fd = open(backing, fi->flags | O_CREAT, mode);
    if (fd == -1) return -errno;

    fi->fh = fd;
    return 0;
}

static int rf2s_mkdir(const char* path, mode_t mode) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (mkdir(backing, mode) == -1) return -errno;

    return 0;
}

static int rf2s_rmdir(const char* path) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (rmdir(backing) == -1) return -errno;

    return 0;
}

static int rf2s_unlink(const char* path) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (unlink(backing) == -1) return -errno;

    return 0;
}

static int rf2s_truncate(const char* path, off_t size) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (truncate(backing, size) == -1) return -errno;

    return 0;
}

static int rf2s_release(const char* path, struct fuse_file_info* fi) {
    (void)path;
    close(fi->fh);
    return 0;
}

static int rf2s_chmod(const char* path, mode_t mode) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (chmod(backing, mode) == -1) return -errno;

    return 0;
}

static int rf2s_chown(const char* path, uid_t uid, gid_t gid) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (chown(backing, uid, gid) == -1) return -errno;

    return 0;
}

static int rf2s_utimens(const char* path, const struct timespec tv[2]) {
    char backing[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(path, backing, sizeof(backing));
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

static int rf2s_rename(const char* from, const char* to) {
    char backing_from[RF2S_MAX_PATH];
    char backing_to[RF2S_MAX_PATH];

    pthread_mutex_lock(&g_fs->lock);
    int ret = rf2s_to_backing(from, backing_from, sizeof(backing_from));
    if (ret == 0) {
        ret = rf2s_to_backing(to, backing_to, sizeof(backing_to));
    }
    pthread_mutex_unlock(&g_fs->lock);

    if (ret != 0) return ret;

    if (rename(backing_from, backing_to) == -1) return -errno;

    return 0;
}

static int rf2s_statfs(const char* path, struct statvfs* stbuf) {
    (void)path;

    if (statvfs(g_fs->backing_path, stbuf) == -1) return -errno;

    return 0;
}

static void* rf2s_init(struct fuse_conn_info* conn) {
    (void)conn;
    return g_fs;
}

static void rf2s_destroy(void* private_data) {
    rf2s_fs_t* fs = (rf2s_fs_t*)private_data;
    if (fs) {
        pthread_mutex_destroy(&fs->lock);
        free(fs);
        g_fs = NULL;
    }
}

static struct fuse_operations rf2s_ops = {
    .init       = rf2s_init,
    .destroy    = rf2s_destroy,
    .getattr    = rf2s_getattr,
    .readdir    = rf2s_readdir,
    .open       = rf2s_open,
    .read       = rf2s_read,
    .write      = rf2s_write,
    .create     = rf2s_create,
    .mkdir      = rf2s_mkdir,
    .rmdir      = rf2s_rmdir,
    .unlink     = rf2s_unlink,
    .truncate   = rf2s_truncate,
    .release    = rf2s_release,
    .chmod      = rf2s_chmod,
    .chown      = rf2s_chown,
    .utimens    = rf2s_utimens,
    .rename     = rf2s_rename,
    .statfs     = rf2s_statfs,
};

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_usage(const char* prog) {
    fprintf(stderr, "Usage: %s [options] <mountpoint>\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -b <path>      Backing storage directory (default: /var/lib/rf2s)\n");
    fprintf(stderr, "  -f             Foreground mode\n");
    fprintf(stderr, "  -D             Debug mode\n");
    fprintf(stderr, "  -h             Show this help\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "RF2S - Radio Frequency FileSystem\n");
    fprintf(stderr, "Organizes data by radio frequency.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s -b /var/lib/rf2s /mnt/rf2s\n", prog);
    fprintf(stderr, "\n");
    fprintf(stderr, "Then access files as:\n");
    fprintf(stderr, "  mkdir /mnt/rf2s/432MHz\n");
    fprintf(stderr, "  echo 'data' > /mnt/rf2s/432MHz/sensor.dat\n");
    fprintf(stderr, "  ls /mnt/rf2s/2.4GHz/\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Frequency formats: 432Hz, 144kHz, 432MHz, 2.4GHz\n");
}

int main(int argc, char* argv[]) {
    char* backing = "/var/lib/rf2s";
    int foreground = 0;
    int debug = 0;

    int opt;
    while ((opt = getopt(argc, argv, "b:fDh")) != -1) {
        switch (opt) {
            case 'b':
                backing = optarg;
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

    /* Create RF2S context */
    g_fs = calloc(1, sizeof(rf2s_fs_t));
    if (!g_fs) {
        fprintf(stderr, "Error: out of memory\n");
        return 1;
    }

    strncpy(g_fs->backing_path, backing, sizeof(g_fs->backing_path) - 1);
    g_fs->base_freq_hz = 432000000ULL;  /* 432 MHz - sacred carrier */

    pthread_mutex_init(&g_fs->lock, NULL);

    /* Ensure backing directory exists */
    mkdir(backing, 0755);

    printf("RF2S mounted:\n");
    printf("  Mountpoint:     %s\n", mountpoint);
    printf("  Backing:        %s\n", backing);
    printf("  Base frequency: 432 MHz\n");

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

    return fuse_main(fuse_argc, fuse_argv, &rf2s_ops, g_fs);
}
