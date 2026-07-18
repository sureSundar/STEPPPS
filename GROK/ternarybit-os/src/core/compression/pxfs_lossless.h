#ifndef TBOS_PXFS_LOSSLESS_H
#define TBOS_PXFS_LOSSLESS_H

#include <stddef.h>
#include <stdint.h>

#define PXFS_LOSSLESS_MAX_INPUT 4096u
#define PXFS_LOSSLESS_MAX_ENCODED (PXFS_LOSSLESS_MAX_INPUT + 8u)

enum {
    PXFS_LOSSLESS_LITERAL = 0,
    PXFS_LOSSLESS_PERIODIC = 1,
    PXFS_LOSSLESS_BLOCKS_V1 = 2
};

/* Encodes CC's compatible modes 0/1 or the checked block mode 2, whichever
 * is smallest. Returns encoded length, or zero on invalid input/capacity. */
size_t pxfs_lossless_compress(const uint8_t* input, size_t input_len,
                              uint8_t* output, size_t output_capacity);

/* Decodes modes 0/1 and mode 2. Mode 2 verifies its original length and CRC32.
 * Returns decoded length, or zero for malformed/corrupt input. */
size_t pxfs_lossless_decompress(const uint8_t* input, size_t input_len,
                                uint8_t* output, size_t output_capacity);

#endif
