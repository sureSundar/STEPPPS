#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fs/ucfs_codec.h"
#include "tbos/errno.h"

static void test_ucfs_parse_basic(void) {
    /* UCFS format: emoji/component1/component2/... */
    const char* path = "🕉/music/chants/108.mp3";
    ucfs_path_t parsed;
    assert(ucfs_parse(path, &parsed) == 0);
    assert(parsed.delimiter == 0x1F549u);  /* U+1F549 = 🕉 (Om symbol) */
    assert(parsed.component_count == 3);
    assert(strcmp(parsed.components[0], "music") == 0);
    assert(strcmp(parsed.components[1], "chants") == 0);
    assert(strcmp(parsed.components[2], "108.mp3") == 0);
    ucfs_free(&parsed);
    printf("  ✅ test_ucfs_parse_basic passed\n");
}

static void test_ucfs_parse_invalid(void) {
    /* ASCII paths should be rejected (not UCFS format) */
    const char* path = "/regular/posix/path";
    ucfs_path_t parsed;
    assert(ucfs_parse(path, &parsed) == -EINVAL);

    /* Empty path should fail */
    assert(ucfs_parse("", &parsed) == -EINVAL);

    /* NULL should fail */
    assert(ucfs_parse(NULL, &parsed) == -EINVAL);

    printf("  ✅ test_ucfs_parse_invalid passed\n");
}

static void test_ucfs_canonical(void) {
    /* UCFS path with globe emoji delimiter */
    const char* path = "🌍/home/user/documents/resume.pdf";
    ucfs_path_t parsed;
    assert(ucfs_parse(path, &parsed) == 0);

    char buffer[128];
    assert(ucfs_to_canonical(&parsed, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "/ucfs/U+1F30D/home/user/documents/resume.pdf") == 0);

    assert(ucfs_to_canonical_with_base(&parsed, "/ucfs_store", buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "/ucfs_store/U+1F30D/home/user/documents/resume.pdf") == 0);

    ucfs_free(&parsed);
    printf("  ✅ test_ucfs_canonical passed\n");
}

static void test_ucfs_empty_components(void) {
    /* Path with just the delimiter (root only) */
    const char* path = "🎵";
    ucfs_path_t parsed;
    assert(ucfs_parse(path, &parsed) == 0);
    assert(parsed.component_count == 0);
    assert(parsed.delimiter == 0x1F3B5u);  /* U+1F3B5 = 🎵 */
    ucfs_free(&parsed);
    printf("  ✅ test_ucfs_empty_components passed\n");
}

int main(void) {
    printf("UCFS Codec Unit Tests\n");
    printf("=====================\n\n");

    test_ucfs_parse_basic();
    test_ucfs_parse_invalid();
    test_ucfs_canonical();
    test_ucfs_empty_components();

    printf("\n✅ All UCFS codec tests passed\n");
    return 0;
}
