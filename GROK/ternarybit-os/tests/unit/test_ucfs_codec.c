#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fs/ucfs_codec.h"
#include "tbos/errno.h"

static void test_ucfs_parse_basic(void) {
    const char* path = "[🕉️]music[🕉️]chants[🕉️]108.mp3";
    ucfs_path_t parsed;
    assert(ucfs_parse(path, &parsed) == 0);
    assert(parsed.delimiter == 0x1F549u);
    assert(parsed.component_count == 3);
    assert(strcmp(parsed.components[0], "music") == 0);
    assert(strcmp(parsed.components[1], "chants") == 0);
    assert(strcmp(parsed.components[2], "108.mp3") == 0);
    ucfs_free(&parsed);
}

static void test_ucfs_parse_invalid(void) {
    const char* path = "[a][a]hello"; /* empty component between consecutive delimiters */
    ucfs_path_t parsed;
    assert(ucfs_parse(path, &parsed) == -EINVAL);
}

static void test_ucfs_canonical(void) {
    const char* path = "[🌍]🏠[🌍]👤[🌍]documents[🌍]resume.pdf";
    ucfs_path_t parsed;
    assert(ucfs_parse(path, &parsed) == 0);

    char buffer[128];
    assert(ucfs_to_canonical(&parsed, buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "/ucfs/U+1F30D/🏠/👤/documents/resume.pdf") == 0);

    assert(ucfs_to_canonical_with_base(&parsed, "/ucfs_store", buffer, sizeof(buffer)) == 0);
    assert(strcmp(buffer, "/ucfs_store/U+1F30D/🏠/👤/documents/resume.pdf") == 0);

    ucfs_free(&parsed);
}

int main(void) {
    test_ucfs_parse_basic();
    test_ucfs_parse_invalid();
    test_ucfs_canonical();
    printf("UCFS codec tests passed\n");
    return 0;
}
