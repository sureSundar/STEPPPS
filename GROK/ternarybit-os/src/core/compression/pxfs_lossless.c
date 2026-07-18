#include "pxfs_lossless.h"

#include <string.h>

#define BLOCK_HEADER_SIZE 8u
#define BLOCK_VERSION 1u
#define BLOCK_MAX 128u

static uint32_t pxfs_crc32(const uint8_t* data, size_t len) {
    uint32_t crc = UINT32_C(0xffffffff);
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (unsigned bit = 0; bit < 8; ++bit) {
            uint32_t mask = (uint32_t)-(int32_t)(crc & 1u);
            crc = (crc >> 1) ^ (UINT32_C(0xedb88320) & mask);
        }
    }
    return ~crc;
}

static size_t repeated_run(const uint8_t* data, size_t len, size_t at) {
    size_t run = 1;
    while (at + run < len && run < BLOCK_MAX && data[at + run] == data[at]) {
        ++run;
    }
    return run;
}

static size_t find_period(const uint8_t* data, size_t len) {
    for (size_t period = 1; period < len && period < 256; ++period) {
        if (len % period != 0) continue;
        size_t i = period;
        while (i < len && data[i] == data[i % period]) ++i;
        if (i == len) return period;
    }
    return 0;
}

static size_t encode_blocks(const uint8_t* input, size_t input_len,
                            uint8_t* output, size_t capacity) {
    if (capacity < BLOCK_HEADER_SIZE) return 0;
    output[0] = PXFS_LOSSLESS_BLOCKS_V1;
    output[1] = BLOCK_VERSION;
    output[2] = (uint8_t)(input_len & 0xffu);
    output[3] = (uint8_t)((input_len >> 8) & 0xffu);
    uint32_t crc = pxfs_crc32(input, input_len);
    for (unsigned i = 0; i < 4; ++i) output[4 + i] = (uint8_t)(crc >> (i * 8));

    size_t in_pos = 0, out_pos = BLOCK_HEADER_SIZE;
    while (in_pos < input_len) {
        size_t run = repeated_run(input, input_len, in_pos);
        if (run >= 3) {
            if (out_pos + 2 > capacity) return 0;
            output[out_pos++] = (uint8_t)(0x80u | (run - 1));
            output[out_pos++] = input[in_pos];
            in_pos += run;
            continue;
        }

        size_t literal_start = in_pos++;
        while (in_pos < input_len && in_pos - literal_start < BLOCK_MAX) {
            if (repeated_run(input, input_len, in_pos) >= 3) break;
            ++in_pos;
        }
        size_t literal_len = in_pos - literal_start;
        if (out_pos + 1 + literal_len > capacity) return 0;
        output[out_pos++] = (uint8_t)(literal_len - 1);
        memcpy(output + out_pos, input + literal_start, literal_len);
        out_pos += literal_len;
    }
    return out_pos;
}

size_t pxfs_lossless_compress(const uint8_t* input, size_t input_len,
                              uint8_t* output, size_t output_capacity) {
    if (!input || !output || input_len == 0 || input_len > PXFS_LOSSLESS_MAX_INPUT ||
        output_capacity < input_len + 1) return 0;

    output[0] = PXFS_LOSSLESS_LITERAL;
    memcpy(output + 1, input, input_len);
    size_t best_len = input_len + 1;

    size_t period = find_period(input, input_len);
    if (period && 4 + period < best_len) {
        uint16_t repeat = (uint16_t)(input_len / period);
        output[0] = PXFS_LOSSLESS_PERIODIC;
        output[1] = (uint8_t)period;
        output[2] = (uint8_t)(repeat & 0xffu);
        output[3] = (uint8_t)(repeat >> 8);
        memcpy(output + 4, input, period);
        best_len = 4 + period;
    }

    uint8_t blocks[PXFS_LOSSLESS_MAX_ENCODED];
    size_t block_len = encode_blocks(input, input_len, blocks, sizeof(blocks));
    if (block_len && block_len < best_len) {
        memcpy(output, blocks, block_len);
        best_len = block_len;
    }
    return best_len;
}

size_t pxfs_lossless_decompress(const uint8_t* input, size_t input_len,
                                uint8_t* output, size_t output_capacity) {
    if (!input || !output || input_len < 1) return 0;
    if (input[0] == PXFS_LOSSLESS_LITERAL) {
        size_t len = input_len - 1;
        if (!len || len > output_capacity) return 0;
        memcpy(output, input + 1, len);
        return len;
    }
    if (input[0] == PXFS_LOSSLESS_PERIODIC) {
        if (input_len < 5) return 0;
        size_t period = input[1];
        uint16_t repeat = (uint16_t)(input[2] | ((uint16_t)input[3] << 8));
        size_t total = period * (size_t)repeat;
        if (!period || !repeat || input_len != 4 + period || !total ||
            total > output_capacity) return 0;
        for (size_t i = 0; i < total; ++i) output[i] = input[4 + i % period];
        return total;
    }
    if (input[0] != PXFS_LOSSLESS_BLOCKS_V1 || input_len < BLOCK_HEADER_SIZE ||
        input[1] != BLOCK_VERSION) return 0;

    size_t expected = input[2] | ((size_t)input[3] << 8);
    uint32_t expected_crc = 0;
    for (unsigned i = 0; i < 4; ++i) expected_crc |= (uint32_t)input[4 + i] << (i * 8);
    if (!expected || expected > output_capacity) return 0;

    size_t in_pos = BLOCK_HEADER_SIZE, out_pos = 0;
    while (in_pos < input_len && out_pos < expected) {
        uint8_t control = input[in_pos++];
        size_t count = (control & 0x7fu) + 1u;
        if (count > expected - out_pos) return 0;
        if (control & 0x80u) {
            if (in_pos >= input_len) return 0;
            memset(output + out_pos, input[in_pos++], count);
        } else {
            if (count > input_len - in_pos) return 0;
            memcpy(output + out_pos, input + in_pos, count);
            in_pos += count;
        }
        out_pos += count;
    }
    if (out_pos != expected || in_pos != input_len || pxfs_crc32(output, out_pos) != expected_crc)
        return 0;
    return out_pos;
}
