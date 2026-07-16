/**
 * @file mkfs.pxfs.c
 * @brief Create a PXFS filesystem
 *
 * Usage: mkfs.pxfs [options] <device|file>
 *
 * Options:
 *   -L <label>    Volume label (max 31 chars)
 *   -m <mode>     Default compression mode (raw, dense, fractal, quantum)
 *   -s <size>     Size in MB (for file-backed filesystems)
 *   -i <inodes>   Number of inodes
 *   -b <blocks>   Number of blocks
 *   -f            Force (overwrite existing filesystem)
 *   -v            Verbose output
 *   -h            Show help
 *
 * @version 1.0
 * @date 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

/* Include PXFS header */
#include "pxfs.h"

/* ═══════════════════════════════════════════════════════════════════════════
 * CONFIGURATION
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* device;
    const char* label;
    uint8_t     default_mode;
    size_t      size_mb;
    uint64_t    num_inodes;
    uint64_t    num_blocks;
    bool        force;
    bool        verbose;
} mkfs_config_t;

static mkfs_config_t g_config = {
    .device = NULL,
    .label = "PXFS Volume",
    .default_mode = PXFS_MODE_AUTO,
    .size_mb = 0,
    .num_inodes = 0,
    .num_blocks = 0,
    .force = false,
    .verbose = false
};

/* ═══════════════════════════════════════════════════════════════════════════
 * UUID GENERATION
 * ═══════════════════════════════════════════════════════════════════════════ */

static void generate_uuid(uint8_t uuid[16]) {
    /* Simple random UUID (version 4) */
    srand((unsigned int)time(NULL) ^ (unsigned int)getpid());
    for (int i = 0; i < 16; i++) {
        uuid[i] = rand() & 0xFF;
    }
    uuid[6] = (uuid[6] & 0x0F) | 0x40;  /* Version 4 */
    uuid[8] = (uuid[8] & 0x3F) | 0x80;  /* Variant 1 */
}

static void format_uuid(const uint8_t uuid[16], char* out) {
    sprintf(out, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            uuid[0], uuid[1], uuid[2], uuid[3],
            uuid[4], uuid[5], uuid[6], uuid[7],
            uuid[8], uuid[9], uuid[10], uuid[11],
            uuid[12], uuid[13], uuid[14], uuid[15]);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * FILESYSTEM CREATION
 * ═══════════════════════════════════════════════════════════════════════════ */

static int create_filesystem(void) {
    int fd = -1;
    int ret = 0;
    struct stat st;

    /* Check if device exists */
    if (stat(g_config.device, &st) == 0) {
        if (!g_config.force) {
            fprintf(stderr, "mkfs.pxfs: %s already exists. Use -f to force.\n",
                    g_config.device);
            return 1;
        }
    }

    /* Determine size */
    uint64_t total_size;
    if (g_config.size_mb > 0) {
        total_size = g_config.size_mb * 1024 * 1024;
    } else if (stat(g_config.device, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            total_size = st.st_size;
        } else if (S_ISBLK(st.st_mode)) {
            /* For block devices, try to get size via seek */
            fd = open(g_config.device, O_RDONLY);
            if (fd >= 0) {
                total_size = lseek(fd, 0, SEEK_END);
                close(fd);
                fd = -1;
            } else {
                fprintf(stderr, "mkfs.pxfs: cannot determine size of %s\n",
                        g_config.device);
                return 1;
            }
        } else {
            fprintf(stderr, "mkfs.pxfs: %s is not a regular file or block device\n",
                    g_config.device);
            return 1;
        }
    } else {
        /* Default to 100MB for new files */
        total_size = 100 * 1024 * 1024;
    }

    /* Calculate layout */
    uint64_t block_size = PXFS_BLOCK_SIZE;
    uint64_t total_blocks = total_size / block_size;

    if (total_blocks < 16) {
        fprintf(stderr, "mkfs.pxfs: filesystem too small (minimum 64KB)\n");
        return 1;
    }

    /* Reserve blocks for metadata */
    uint64_t superblock_blocks = 1;  /* 1 block for superblock */
    uint64_t inode_count = g_config.num_inodes > 0 ?
                           g_config.num_inodes : total_blocks / 4;
    uint64_t inode_blocks = (inode_count * sizeof(pxfs_inode_t) + block_size - 1) / block_size;
    uint64_t bitmap_blocks = (total_blocks + block_size * 8 - 1) / (block_size * 8);
    uint64_t data_blocks = total_blocks - superblock_blocks - inode_blocks - bitmap_blocks;

    if (g_config.verbose) {
        printf("Creating PXFS filesystem:\n");
        printf("  Device:       %s\n", g_config.device);
        printf("  Total size:   %llu MB\n", (unsigned long long)(total_size / (1024 * 1024)));
        printf("  Block size:   %llu bytes\n", (unsigned long long)block_size);
        printf("  Total blocks: %llu\n", (unsigned long long)total_blocks);
        printf("  Inode blocks: %llu (%llu inodes)\n",
               (unsigned long long)inode_blocks, (unsigned long long)inode_count);
        printf("  Bitmap blocks: %llu\n", (unsigned long long)bitmap_blocks);
        printf("  Data blocks:  %llu\n", (unsigned long long)data_blocks);
        printf("  Label:        %s\n", g_config.label);
    }

    /* Open/create device */
    fd = open(g_config.device, O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        fprintf(stderr, "mkfs.pxfs: cannot create %s: %s\n",
                g_config.device, strerror(errno));
        return 1;
    }

    /* Extend file to full size */
    if (ftruncate(fd, total_size) < 0) {
        fprintf(stderr, "mkfs.pxfs: cannot set size: %s\n", strerror(errno));
        ret = 1;
        goto cleanup;
    }

    /* Create superblock */
    pxfs_superblock_t sb;
    memset(&sb, 0, sizeof(sb));

    sb.magic = PXFS_MAGIC;
    sb.version = PXFS_VERSION;
    sb.flags = 0;
    sb.total_blocks = total_blocks;
    sb.free_blocks = data_blocks;
    sb.total_inodes = inode_count;
    sb.free_inodes = inode_count - 1;  /* Reserve inode 0 */
    sb.block_size = block_size;
    sb.inode_size = sizeof(pxfs_inode_t);
    sb.inode_table_start = superblock_blocks;
    sb.bitmap_start = superblock_blocks + inode_blocks;
    sb.data_start = superblock_blocks + inode_blocks + bitmap_blocks;
    sb.default_mode = g_config.default_mode;
    sb.create_time = time(NULL);
    sb.mount_time = 0;
    sb.write_time = time(NULL);
    sb.mount_count = 0;
    sb.max_mount_count = 100;
    generate_uuid(sb.uuid);
    strncpy(sb.label, g_config.label, sizeof(sb.label) - 1);
    sb.bytes_stored = 0;
    sb.bytes_original = 0;
    sb.separator_rgb[0] = 255;
    sb.separator_rgb[1] = 255;
    sb.separator_rgb[2] = 255;

    /* Write superblock */
    if (g_config.verbose) {
        printf("Writing superblock...\n");
    }
    if (write(fd, &sb, sizeof(sb)) != sizeof(sb)) {
        fprintf(stderr, "mkfs.pxfs: failed to write superblock\n");
        ret = 1;
        goto cleanup;
    }

    /* Pad superblock to block size */
    uint8_t zeros[PXFS_BLOCK_SIZE];
    memset(zeros, 0, sizeof(zeros));
    size_t pad = block_size - sizeof(sb);
    if (write(fd, zeros, pad) != (ssize_t)pad) {
        fprintf(stderr, "mkfs.pxfs: failed to pad superblock\n");
        ret = 1;
        goto cleanup;
    }

    /* Initialize root directory inode (inode 1) */
    if (g_config.verbose) {
        printf("Creating root directory...\n");
    }

    pxfs_inode_t root_inode;
    memset(&root_inode, 0, sizeof(root_inode));
    root_inode.mode = 0040755;  /* Directory with rwxr-xr-x */
    root_inode.uid = getuid();
    root_inode.gid = getgid();
    root_inode.size = block_size;
    root_inode.size_compressed = block_size;
    root_inode.atime = time(NULL);
    root_inode.mtime = time(NULL);
    root_inode.ctime = time(NULL);
    root_inode.crtime = time(NULL);
    root_inode.links = 2;  /* . and parent */
    root_inode.comp_mode = PXFS_MODE_RAW;
    root_inode.direct[0] = sb.data_start;  /* First data block */

    /* Write empty inode 0 (reserved) */
    pxfs_inode_t empty_inode;
    memset(&empty_inode, 0, sizeof(empty_inode));
    if (write(fd, &empty_inode, sizeof(empty_inode)) != sizeof(empty_inode)) {
        fprintf(stderr, "mkfs.pxfs: failed to write inode 0\n");
        ret = 1;
        goto cleanup;
    }

    /* Write root inode (inode 1) */
    if (write(fd, &root_inode, sizeof(root_inode)) != sizeof(root_inode)) {
        fprintf(stderr, "mkfs.pxfs: failed to write root inode\n");
        ret = 1;
        goto cleanup;
    }

    /* Initialize bitmap - mark used blocks */
    if (g_config.verbose) {
        printf("Initializing block bitmap...\n");
    }
    lseek(fd, sb.bitmap_start * block_size, SEEK_SET);

    uint8_t* bitmap = calloc(1, bitmap_blocks * block_size);
    if (!bitmap) {
        fprintf(stderr, "mkfs.pxfs: out of memory\n");
        ret = 1;
        goto cleanup;
    }

    /* Mark metadata blocks as used */
    uint64_t used_blocks = sb.data_start + 1;  /* +1 for root directory block */
    for (uint64_t i = 0; i < used_blocks; i++) {
        bitmap[i / 8] |= (1 << (i % 8));
    }

    if (write(fd, bitmap, bitmap_blocks * block_size) !=
        (ssize_t)(bitmap_blocks * block_size)) {
        fprintf(stderr, "mkfs.pxfs: failed to write bitmap\n");
        free(bitmap);
        ret = 1;
        goto cleanup;
    }
    free(bitmap);

    /* Initialize root directory block with . and .. entries */
    if (g_config.verbose) {
        printf("Initializing root directory block...\n");
    }
    lseek(fd, sb.data_start * block_size, SEEK_SET);

    uint8_t* dir_block = calloc(1, block_size);
    if (!dir_block) {
        fprintf(stderr, "mkfs.pxfs: out of memory\n");
        ret = 1;
        goto cleanup;
    }

    /* . entry */
    pxfs_dirent_t* dot = (pxfs_dirent_t*)dir_block;
    dot->inode = 1;
    dot->rec_len = sizeof(pxfs_dirent_t);
    dot->name_len = 1;
    dot->file_type = PXFS_INODE_DIR;
    strcpy(dot->name, ".");

    /* .. entry */
    pxfs_dirent_t* dotdot = (pxfs_dirent_t*)(dir_block + sizeof(pxfs_dirent_t));
    dotdot->inode = 1;
    dotdot->rec_len = block_size - sizeof(pxfs_dirent_t);  /* Rest of block */
    dotdot->name_len = 2;
    dotdot->file_type = PXFS_INODE_DIR;
    strcpy(dotdot->name, "..");

    if (write(fd, dir_block, block_size) != (ssize_t)block_size) {
        fprintf(stderr, "mkfs.pxfs: failed to write root directory\n");
        free(dir_block);
        ret = 1;
        goto cleanup;
    }
    free(dir_block);

    /* Success */
    if (g_config.verbose) {
        char uuid_str[40];
        format_uuid(sb.uuid, uuid_str);
        printf("\n");
        printf("PXFS filesystem created successfully!\n");
        printf("  UUID:         %s\n", uuid_str);
        printf("  Label:        %s\n", sb.label);
        printf("  Total size:   %llu MB\n", (unsigned long long)(total_size / (1024 * 1024)));
        printf("  Usable:       %llu MB\n",
               (unsigned long long)(data_blocks * block_size / (1024 * 1024)));
        printf("  Inodes:       %llu\n", (unsigned long long)inode_count);
        printf("\n");
        printf("To mount: mount -t pxfs %s /mnt/pxfs\n", g_config.device);
        printf("    or:   pxfs-mount %s /mnt/pxfs\n", g_config.device);
    } else {
        printf("Created PXFS filesystem on %s\n", g_config.device);
    }

cleanup:
    if (fd >= 0) close(fd);
    return ret;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

static void usage(const char* progname) {
    printf("Usage: %s [options] <device|file>\n", progname);
    printf("\n");
    printf("Create a PXFS (Pixel eXchange FileSystem) on <device> or <file>.\n");
    printf("\n");
    printf("Options:\n");
    printf("  -L <label>    Volume label (max 31 chars)\n");
    printf("  -m <mode>     Default compression mode:\n");
    printf("                  raw     - No compression (1:1)\n");
    printf("                  dense   - Bit-packed (1:1)\n");
    printf("                  fractal - Pattern-based (~2:1)\n");
    printf("                  quantum - Extreme compression (up to 1365:1)\n");
    printf("                  auto    - Auto-detect best mode (default)\n");
    printf("  -s <size>     Size in MB (for file-backed filesystems)\n");
    printf("  -i <inodes>   Number of inodes (default: blocks/4)\n");
    printf("  -b <blocks>   Number of blocks\n");
    printf("  -f            Force (overwrite existing filesystem)\n");
    printf("  -v            Verbose output\n");
    printf("  -h            Show this help\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s -L \"My PXFS\" /dev/sdb1       # Format block device\n", progname);
    printf("  %s -s 100 -v pxfs.img            # Create 100MB image file\n", progname);
    printf("  %s -m quantum -f /dev/sdc1       # Force quantum mode\n", progname);
    printf("\n");
}

int main(int argc, char** argv) {
    int opt;

    while ((opt = getopt(argc, argv, "L:m:s:i:b:fvh")) != -1) {
        switch (opt) {
            case 'L':
                g_config.label = optarg;
                break;

            case 'm':
                if (strcasecmp(optarg, "raw") == 0) {
                    g_config.default_mode = PXFS_MODE_RAW;
                } else if (strcasecmp(optarg, "dense") == 0) {
                    g_config.default_mode = PXFS_MODE_DENSE;
                } else if (strcasecmp(optarg, "fractal") == 0) {
                    g_config.default_mode = PXFS_MODE_FRACTAL;
                } else if (strcasecmp(optarg, "quantum") == 0) {
                    g_config.default_mode = PXFS_MODE_QUANTUM;
                } else if (strcasecmp(optarg, "auto") == 0) {
                    g_config.default_mode = PXFS_MODE_AUTO;
                } else {
                    fprintf(stderr, "mkfs.pxfs: unknown mode: %s\n", optarg);
                    return 1;
                }
                break;

            case 's':
                g_config.size_mb = atoi(optarg);
                break;

            case 'i':
                g_config.num_inodes = atoll(optarg);
                break;

            case 'b':
                g_config.num_blocks = atoll(optarg);
                break;

            case 'f':
                g_config.force = true;
                break;

            case 'v':
                g_config.verbose = true;
                break;

            case 'h':
                usage(argv[0]);
                return 0;

            default:
                usage(argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "mkfs.pxfs: missing device argument\n");
        usage(argv[0]);
        return 1;
    }

    g_config.device = argv[optind];

    printf("mkfs.pxfs - PXFS filesystem creator v%d.%d\n",
           PXFS_VERSION >> 8, PXFS_VERSION & 0xFF);

    return create_filesystem();
}
