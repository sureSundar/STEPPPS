#include "fs/fat32_driver.h"
#include "tbos/blockdev.h"
#include "tbos/errno.h"
#include "tbos/libc.h"
#include <ctype.h>

#define FAT32_ATTR_LONG_NAME 0x0F
#define FAT32_ATTR_DIRECTORY 0x10
#define FAT32_ATTR_VOLUME_ID 0x08
#define FAT32_END_OF_CHAIN   0x0FFFFFF8u

typedef struct fat32_fs {
    const tbos_blockdev_t* dev;
    uint32_t bytes_per_sector;
    uint32_t sectors_per_cluster;
    uint32_t reserved_sectors;
    uint32_t fats;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;
    uint32_t fat_start_lba;
    uint32_t data_start_lba;
    uint8_t* sector_buffer;
    uint8_t* cluster_buffer;
} fat32_fs_t;

#pragma pack(push, 1)
typedef struct {
    uint8_t  jump_boot[3];
    char     oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  number_of_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t  reserved[12];
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    char     volume_label[11];
    char     fs_type[8];
} fat32_boot_sector_t;

typedef struct {
    char     name[11];
    uint8_t  attr;
    uint8_t  ntres;
    uint8_t  crt_time_tenths;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} fat32_dir_entry_t;
#pragma pack(pop)

static uint32_t fat32_cluster_to_lba(const fat32_fs_t* fs, uint32_t cluster) {
    return fs->data_start_lba + (cluster - 2u) * fs->sectors_per_cluster;
}

static int fat32_read_sector(const fat32_fs_t* fs, uint32_t lba, void* buffer) {
    return blockdev_read(fs->dev, lba, buffer, 1);
}

static uint32_t fat32_read_fat_entry(fat32_fs_t* fs, uint32_t cluster) {
    uint64_t fat_offset = (uint64_t)cluster * 4u;
    uint32_t sector = fs->fat_start_lba + (uint32_t)(fat_offset / fs->bytes_per_sector);
    uint32_t offset = (uint32_t)(fat_offset % fs->bytes_per_sector);
    if (fat32_read_sector(fs, sector, fs->sector_buffer) != 0) {
        return FAT32_END_OF_CHAIN;
    }
    uint32_t entry = *(uint32_t*)(fs->sector_buffer + offset);
    return entry & 0x0FFFFFFFu;
}

static int fat32_format_83_name(const char* path, char out[11]) {
    if (!path || path[0] != '/') return -EINVAL;
    path++;
    if (*path == '\0') return -EINVAL;
    char name[8];
    char ext[3];
    memset(name, ' ', sizeof(name));
    memset(ext, ' ', sizeof(ext));
    size_t name_len = 0;
    while (*path && *path != '.' && name_len < 8) {
        name[name_len++] = (char)toupper((unsigned char)*path);
        path++;
    }
    if (*path == '.') {
        path++;
        size_t ext_len = 0;
        while (*path && ext_len < 3) {
            ext[ext_len++] = (char)toupper((unsigned char)*path);
            path++;
        }
    }
    if (*path != '\0') {
        return -ENAMETOOLONG;
    }
    memcpy(out, name, 8);
    memcpy(out + 8, ext, 3);
    return 0;
}

static int fat32_find_entry(fat32_fs_t* fs, const char name83[11], fat32_dir_entry_t* out_entry) {
    uint32_t cluster = fs->root_cluster;
    uint32_t sector_count = fs->sectors_per_cluster;
    size_t entries_per_sector = fs->bytes_per_sector / sizeof(fat32_dir_entry_t);

    while (cluster < FAT32_END_OF_CHAIN) {
        uint32_t lba = fat32_cluster_to_lba(fs, cluster);
        for (uint32_t s = 0; s < sector_count; s++) {
            if (fat32_read_sector(fs, lba + s, fs->sector_buffer) != 0) {
                return -EIO;
            }
            fat32_dir_entry_t* entries = (fat32_dir_entry_t*)fs->sector_buffer;
            for (size_t i = 0; i < entries_per_sector; i++) {
                uint8_t first = (uint8_t)entries[i].name[0];
                if (first == 0x00) {
                    return -ENOENT;
                }
                if (first == 0xE5) {
                    continue;
                }
                if (entries[i].attr == FAT32_ATTR_LONG_NAME || (entries[i].attr & FAT32_ATTR_VOLUME_ID)) {
                    continue;
                }
                if (memcmp(entries[i].name, name83, 11) == 0) {
                    memcpy(out_entry, &entries[i], sizeof(fat32_dir_entry_t));
                    return 0;
                }
            }
        }
        uint32_t next = fat32_read_fat_entry(fs, cluster);
        if (next >= FAT32_END_OF_CHAIN) {
            break;
        }
        cluster = next;
    }
    return -ENOENT;
}

static int fat32_mkdir(void* ctx, const char* path) {
    (void)ctx; (void)path;
    return -EROFS;
}

static int fat32_write(void* ctx, const char* path, const void* data, size_t size, bool append) {
    (void)ctx; (void)path; (void)data; (void)size; (void)append;
    return -EROFS;
}

static int fat32_read(void* context, const char* path, void* buffer, size_t max_size, size_t* out_size) {
    fat32_fs_t* fs = (fat32_fs_t*)context;
    char name83[11];
    int rc = fat32_format_83_name(path, name83);
    if (rc != 0) return rc;
    fat32_dir_entry_t entry;
    rc = fat32_find_entry(fs, name83, &entry);
    if (rc != 0) return rc;
    if (entry.attr & FAT32_ATTR_DIRECTORY) return -EISDIR;
    size_t file_size = entry.file_size;
    size_t to_read = file_size < max_size ? file_size : max_size;
    if (out_size) *out_size = to_read;
    uint32_t cluster = ((uint32_t)entry.first_cluster_high << 16) | entry.first_cluster_low;
    size_t bytes_read = 0;
    while (cluster < FAT32_END_OF_CHAIN && bytes_read < to_read) {
        uint32_t lba = fat32_cluster_to_lba(fs, cluster);
        for (uint32_t s = 0; s < fs->sectors_per_cluster && bytes_read < to_read; s++) {
            if (fat32_read_sector(fs, lba + s, fs->cluster_buffer) != 0) {
                return -EIO;
            }
            size_t chunk = fs->bytes_per_sector;
            if (bytes_read + chunk > to_read) {
                chunk = to_read - bytes_read;
            }
            memcpy((uint8_t*)buffer + bytes_read, fs->cluster_buffer, chunk);
            bytes_read += chunk;
        }
        if (bytes_read >= to_read) break;
        uint32_t next = fat32_read_fat_entry(fs, cluster);
        if (next >= FAT32_END_OF_CHAIN) break;
        cluster = next;
    }
    return 0;
}

static const void* fat32_read_cstr(void* ctx, const char* path, size_t* out_size) {
    static char static_buf[256];
    size_t read = 0;
    int rc = fat32_read(ctx, path, static_buf, sizeof(static_buf) - 1, &read);
    if (rc != 0) return NULL;
    static_buf[read] = '\0';
    if (out_size) *out_size = read;
    return static_buf;
}

static int fat32_remove_entry(void* ctx, const char* path, bool recursive) {
    (void)ctx; (void)path; (void)recursive;
    return -EROFS;
}

static bool fat32_exists(void* ctx, const char* path) {
    fat32_fs_t* fs = (fat32_fs_t*)ctx;
    char name83[11];
    if (fat32_format_83_name(path, name83) != 0) return false;
    fat32_dir_entry_t entry;
    return fat32_find_entry(fs, name83, &entry) == 0;
}

static vfs_node_type_t fat32_type(void* ctx, const char* path) {
    fat32_fs_t* fs = (fat32_fs_t*)ctx;
    if (strcmp(path, "/") == 0) return VFS_NODE_DIR;
    char name83[11];
    if (fat32_format_83_name(path, name83) != 0) return VFS_NODE_FILE;
    fat32_dir_entry_t entry;
    if (fat32_find_entry(fs, name83, &entry) != 0) return VFS_NODE_FILE;
    return (entry.attr & FAT32_ATTR_DIRECTORY) ? VFS_NODE_DIR : VFS_NODE_FILE;
}

static int fat32_list_dir(void* ctx, const char* path, int (*cb)(const char*, vfs_node_type_t, void*), void* user) {
    (void)ctx; (void)path; (void)cb; (void)user;
    return -ENOSYS;
}

static const vfs_driver_t FAT32_DRIVER = {
    .name = "fat32",
    .init = NULL,
    .mkdir = fat32_mkdir,
    .write_file = fat32_write,
    .read_file = fat32_read,
    .read_file_cstr = fat32_read_cstr,
    .remove = fat32_remove_entry,
    .exists = fat32_exists,
    .type = fat32_type,
    .list_dir = fat32_list_dir,
};

const vfs_driver_t* fat32_vfs_driver(void) {
    return &FAT32_DRIVER;
}

fat32_fs_t* fat32_mount_blockdev(const tbos_blockdev_t* dev) {
    if (!dev) return NULL;
    uint8_t sector[512];
    if (blockdev_read(dev, 0, sector, 1) != 0) return NULL;
    fat32_boot_sector_t* bpb = (fat32_boot_sector_t*)sector;
    if (bpb->bytes_per_sector == 0 || bpb->sectors_per_cluster == 0) {
        return NULL;
    }
    fat32_fs_t* fs = malloc(sizeof(fat32_fs_t));
    if (!fs) return NULL;
    memset(fs, 0, sizeof(*fs));
    fs->dev = dev;
    fs->bytes_per_sector = bpb->bytes_per_sector;
    fs->sectors_per_cluster = bpb->sectors_per_cluster;
    fs->reserved_sectors = bpb->reserved_sector_count;
    fs->fats = bpb->number_of_fats;
    fs->sectors_per_fat = bpb->fat_size_32 ? bpb->fat_size_32 : bpb->fat_size_16;
    fs->root_cluster = bpb->root_cluster;
    fs->fat_start_lba = fs->reserved_sectors;
    fs->data_start_lba = fs->reserved_sectors + fs->fats * fs->sectors_per_fat;
    fs->sector_buffer = malloc(fs->bytes_per_sector);
    fs->cluster_buffer = malloc(fs->bytes_per_sector * fs->sectors_per_cluster);
    if (!fs->sector_buffer || !fs->cluster_buffer) {
        free(fs->sector_buffer);
        free(fs->cluster_buffer);
        free(fs);
        return NULL;
    }
    return fs;
}
