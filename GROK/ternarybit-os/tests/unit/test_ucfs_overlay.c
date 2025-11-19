#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "tbos/vfs.h"
#include "tbos/fs_drivers.h"

#include "fs/ucfs_overlay.h"

static void setup_vfs(void) {
    vfs_init();
    assert(vfs_mount("/", &ramfs_driver) == 0);
}

static void test_ucfs_write_read(void) {
    const char* ucfs_path = "[🕉️]music[🕉️]chants[🕉️]108.mp3";
    const char* payload = "Om Bhur Bhuvah Swaha";
    assert(ucfs_write_file_uc(ucfs_path, payload, strlen(payload)) == 0);

    char buffer[64];
    size_t out = 0;
    assert(ucfs_read_file_uc(ucfs_path, buffer, sizeof(buffer), &out) == 0);
    buffer[out] = '\0';
    assert(strcmp(buffer, payload) == 0);
}

static void test_ucfs_exists(void) {
    assert(ucfs_exists_uc("[🕉️]music[🕉️]chants[🕉️]108.mp3"));
    assert(!ucfs_exists_uc("[🕉️]music[🕉️]chants[🕉️]missing.mp3"));
}

int main(void) {
    setup_vfs();
    test_ucfs_write_read();
    test_ucfs_exists();
    printf("UCFS overlay tests passed\n");
    return 0;
}
