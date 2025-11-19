#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fs/pxfs_codec.h"
#include "tbos/errno.h"

static void test_pxfs_parse_basic(void) {
    const char* path = "{255,0,0}system{255,0,0}kernel{255,0,0}boot.bin";
    pxfs_path_t px;
    assert(pxfs_parse(path, &px) == 0);
    assert(px.r == 255 && px.g == 0 && px.b == 0);
    assert(px.component_count == 3);
    assert(strcmp(px.components[0], "system") == 0);
    assert(strcmp(px.components[1], "kernel") == 0);
    assert(strcmp(px.components[2], "boot.bin") == 0);
    pxfs_free(&px);
}

static void test_pxfs_canonical(void) {
    const char* path = "{0,128,255}graphics{0,128,255}textures{0,128,255}sunset.png";
    pxfs_path_t px;
    assert(pxfs_parse(path, &px) == 0);
    char buffer[128];
    assert(pxfs_to_canonical(&px, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "/pxfs/0080FF/graphics/textures/sunset.png") == 0);
    assert(pxfs_to_canonical_with_base(&px, "/pxfs_store", buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "/pxfs_store/0080FF/graphics/textures/sunset.png") == 0);
    pxfs_free(&px);
}

static void test_pxfs_invalid(void) {
    pxfs_path_t px;
    assert(pxfs_parse("{256,0,0}foo{256,0,0}bar", &px) == -EINVAL);
    assert(pxfs_parse("{255,0}foo", &px) == -EINVAL);
    assert(pxfs_parse("{255,0,0}", &px) == -EINVAL);
}

int main(void) {
    test_pxfs_parse_basic();
    test_pxfs_canonical();
    test_pxfs_invalid();
    printf("PXFS codec tests passed\n");
    return 0;
}
