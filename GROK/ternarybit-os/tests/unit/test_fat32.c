#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tbos/blockdev.h"
#include "tbos/vfs.h"
#include "tbos/errno.h"
#include "fs/fat32_driver.h"

#define TEST_BLOCK_SIZE 512u
#define TEST_BLOCK_COUNT 2048u
#define RESERVED_SECTORS 32u
#define FAT_SECTORS 1u

static const tbos_blockdev_t* g_dev = NULL;
static fat32_fs_t* g_fs = NULL;

static void write_sector(uint32_t lba, const uint8_t* data) {
    int rc = blockdev_write(g_dev, lba, data, 1);
    assert(rc == 0);
}

static void zero_disk(void) {
    uint8_t zero[TEST_BLOCK_SIZE];
    memset(zero, 0, sizeof(zero));
    for (uint32_t i = 0; i < TEST_BLOCK_COUNT; i++) {
        write_sector(i, zero);
    }
}

static void build_boot_sector(void) {
    uint8_t sector[TEST_BLOCK_SIZE];
    memset(sector, 0, sizeof(sector));
    sector[0] = 0xEB;
    sector[1] = 0x58;
    sector[2] = 0x90;
    memcpy(&sector[3], "TBOSFAT ", 8);
    *(uint16_t*)&sector[11] = TEST_BLOCK_SIZE;
    sector[13] = 1; /* sectors per cluster */
    *(uint16_t*)&sector[14] = RESERVED_SECTORS;
    sector[16] = 1; /* FATs */
    *(uint32_t*)&sector[32] = TEST_BLOCK_COUNT;
    *(uint32_t*)&sector[36] = FAT_SECTORS;
    *(uint32_t*)&sector[44] = 2; /* root cluster */
    sector[64] = 0x80;
    sector[66] = 0x29;
    memcpy(&sector[71], "TBOS FAT32", 11);
    memcpy(&sector[82], "FAT32   ", 8);
    sector[510] = 0x55;
    sector[511] = 0xAA;
    write_sector(0, sector);
}

static void build_fat(void) {
    uint8_t sector[TEST_BLOCK_SIZE];
    memset(sector, 0, sizeof(sector));
    uint32_t* entries = (uint32_t*)sector;
    entries[0] = 0x0FFFFFF8u;
    entries[1] = 0x0FFFFFFFu;
    entries[2] = 0x0FFFFFFFu; /* root cluster */
    entries[3] = 0x0FFFFFFFu; /* data cluster */
    write_sector(RESERVED_SECTORS, sector);
}

static void build_root_directory(void) {
    uint8_t sector[TEST_BLOCK_SIZE];
    memset(sector, 0, sizeof(sector));
    typedef struct {
        char name[11];
        uint8_t attr;
        uint8_t ntres;
        uint8_t crt_time_tenths;
        uint16_t crt_time;
        uint16_t crt_date;
        uint16_t last_access_date;
        uint16_t first_cluster_high;
        uint16_t write_time;
        uint16_t write_date;
        uint16_t first_cluster_low;
        uint32_t file_size;
    } __attribute__((packed)) dir_entry_t;
    dir_entry_t* entry = (dir_entry_t*)sector;
    memcpy(entry->name, "HELLO   TXT", 11);
    entry->attr = 0x20;
    entry->first_cluster_low = 3;
    entry->file_size = 18;
    uint32_t root_lba = RESERVED_SECTORS + FAT_SECTORS; /* cluster 2 */
    write_sector(root_lba, sector);
}

static void build_file_data(void) {
    uint8_t sector[TEST_BLOCK_SIZE];
    memset(sector, 0, sizeof(sector));
    const char* message = "Hello from FAT32\n";
    memcpy(sector, message, strlen(message));
    uint32_t data_lba = RESERVED_SECTORS + FAT_SECTORS + 1; /* cluster 3 */
    write_sector(data_lba, sector);
}

static void setup_fs(void) {
    blockdev_init();
    vfs_init();
    g_dev = blockdev_create_ramdisk("fat-test", TEST_BLOCK_SIZE, TEST_BLOCK_COUNT);
    assert(g_dev != NULL);
    zero_disk();
    build_boot_sector();
    build_fat();
    build_root_directory();
    build_file_data();
    g_fs = fat32_mount_blockdev(g_dev);
    assert(g_fs != NULL);
    assert(vfs_mount_with_context("/fat", fat32_vfs_driver(), g_fs) == 0);
}

static void test_fat32_read_file(void) {
    char buffer[64];
    size_t out = 0;
    assert(vfs_read_file("/fat/HELLO.TXT", buffer, sizeof(buffer), &out) == 0);
    assert(out == 18);
    buffer[out] = '\0';
    assert(strcmp(buffer, "Hello from FAT32\n") == 0);
}

static void test_fat32_missing_file(void) {
    char buffer[16];
    size_t out = 0;
    assert(vfs_read_file("/fat/MISSING.TXT", buffer, sizeof(buffer), &out) == -ENOENT);
}

static void test_fat32_write_denied(void) {
    const char* data = "NOPE";
    assert(vfs_write_file("/fat/HELLO.TXT", data, strlen(data)) == -EROFS);
}

int main(void) {
    setup_fs();
    test_fat32_read_file();
    test_fat32_missing_file();
    test_fat32_write_denied();
    printf("FAT32 tests passed\n");
    return 0;
}
