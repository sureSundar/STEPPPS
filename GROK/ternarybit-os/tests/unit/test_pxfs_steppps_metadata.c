#include "steppps_xattr.h"
#include "pxfs_steppps.h"

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    steppps_xattr_t xattr;
    steppps_create_ctx_t context = {
        .path = "/demo/namaste.txt",
        .filesystem = "pxfs",
        .realm = "bhulok",
        .owner = "steppps://human/guru",
        .nation = "steppps://nation/bharat",
        .creation_prompt = "Canonical conformance demo",
        .is_directory = false,
        .mode = 0644,
        .uid = 1000,
        .gid = 1000,
        .size = 7,
        .mime_type = "text/plain",
    };
    static const uint8_t content[] = "Namaste";
    uint8_t encoded[8192];
    uint8_t decoded[sizeof(content)] = {0};
    char metadata[4096];
    char prompt[1024];
    size_t encoded_size = 0;
    size_t decoded_size = 0;
    ssize_t prompt_size;

    assert(steppps_xattr_create(&xattr, &context) == 0);
    prompt_size = steppps_xattr_get(
        &xattr, STEPPPS_XATTR_PROMPT, prompt, sizeof(prompt) - 1
    );
    assert(prompt_size > 0);
    prompt[prompt_size] = '\0';
    assert(strstr(prompt, "Canonical conformance demo") != NULL);

    assert(pxfs_create_with_steppps(
        xattr.prompt,
        content,
        sizeof(content) - 1,
        encoded,
        sizeof(encoded),
        &encoded_size
    ) == 0);
    assert(pxfs_has_steppps(encoded, encoded_size));

    assert(pxfs_read_with_steppps(
        encoded,
        encoded_size,
        metadata,
        sizeof(metadata),
        decoded,
        sizeof(decoded),
        &decoded_size
    ) == 0);
    assert(strcmp(metadata, xattr.prompt) == 0);
    assert(decoded_size == sizeof(content) - 1);
    assert(memcmp(decoded, content, decoded_size) == 0);

    assert(steppps_xattr_remove(&xattr, STEPPPS_XATTR_PROMPT) == 0);
    assert(steppps_xattr_get(&xattr, STEPPPS_XATTR_PROMPT, NULL, 0) == -ENODATA);
    steppps_xattr_free(&xattr);
    return 0;
}
