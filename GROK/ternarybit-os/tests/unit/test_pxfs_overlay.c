#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "tbos/vfs.h"
#include "tbos/fs_drivers.h"
#include "fs/pxfs_overlay.h"

static void setup_vfs(void) {
    vfs_init();
    assert(vfs_mount("/", &ramfs_driver) == 0);
}

static void test_pxfs_write_read(void) {
    const char* path = "{0,128,255}graphics{0,128,255}textures{0,128,255}sunset.png";
    const char* payload = "PIXEL DATA";
    assert(pxfs_write_file(path, payload, strlen(payload)) == 0);
    char buffer[32];
    size_t out = 0;
    assert(pxfs_read_file(path, buffer, sizeof(buffer), &out) == 0);
    buffer[out] = '\0';
    assert(strcmp(buffer, payload) == 0);
}

static void test_pxfs_exists(void) {
    const char* existing = "{0,128,255}graphics{0,128,255}textures{0,128,255}sunset.png";
    const char* missing = "{0,128,255}graphics{0,128,255}textures{0,128,255}missing.png";
    assert(pxfs_exists(existing));
    assert(!pxfs_exists(missing));
}

int main(void) {
    setup_vfs();
    test_pxfs_write_read();
    test_pxfs_exists();
    printf("PXFS overlay tests passed\n");
    return 0;
}
