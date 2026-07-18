#include "../../src/core/compression/pxfs_lossless.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void round_trip(const uint8_t* input, size_t len) {
    uint8_t encoded[PXFS_LOSSLESS_MAX_ENCODED];
    uint8_t decoded[PXFS_LOSSLESS_MAX_INPUT];
    size_t encoded_len = pxfs_lossless_compress(input, len, encoded, sizeof(encoded));
    assert(encoded_len > 0);
    size_t decoded_len = pxfs_lossless_decompress(encoded, encoded_len, decoded, sizeof(decoded));
    assert(decoded_len == len);
    assert(memcmp(input, decoded, len) == 0);
}

int main(void) {
    static const uint8_t periodic[] = "abcabcabcabcabcabc";
    round_trip(periodic, sizeof(periodic) - 1);

    uint8_t mixed[512];
    for (size_t i = 0; i < sizeof(mixed); ++i)
        mixed[i] = (i >= 40 && i < 440) ? 0xaa : (uint8_t)i;
    uint8_t encoded[PXFS_LOSSLESS_MAX_ENCODED];
    uint8_t decoded[PXFS_LOSSLESS_MAX_INPUT];
    size_t encoded_len = pxfs_lossless_compress(mixed, sizeof(mixed), encoded, sizeof(encoded));
    assert(encoded_len < sizeof(mixed));
    assert(encoded[0] == PXFS_LOSSLESS_BLOCKS_V1);
    assert(pxfs_lossless_decompress(encoded, encoded_len, decoded, sizeof(decoded)) == sizeof(mixed));
    assert(memcmp(mixed, decoded, sizeof(mixed)) == 0);

    uint32_t state = 1;
    for (size_t len = 1; len <= PXFS_LOSSLESS_MAX_INPUT; len += 37) {
        uint8_t random_data[PXFS_LOSSLESS_MAX_INPUT];
        for (size_t i = 0; i < len; ++i) {
            state = state * UINT32_C(1664525) + UINT32_C(1013904223);
            random_data[i] = (uint8_t)(state >> 24);
        }
        round_trip(random_data, len);
    }

    encoded_len = pxfs_lossless_compress(mixed, sizeof(mixed), encoded, sizeof(encoded));
    encoded[encoded_len - 1] ^= 1u;
    assert(pxfs_lossless_decompress(encoded, encoded_len, decoded, sizeof(decoded)) == 0);

    static const uint8_t legacy_periodic[] = {1, 1, 4, 0, 'x'};
    assert(pxfs_lossless_decompress(legacy_periodic, sizeof(legacy_periodic), decoded,
                                    sizeof(decoded)) == 4);
    assert(memcmp(decoded, "xxxx", 4) == 0);

    puts("PXFS lossless codec tests passed");
    return 0;
}
