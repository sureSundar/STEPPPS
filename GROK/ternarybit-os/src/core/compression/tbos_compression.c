/*
 * TBOS Compression Module - Implementation
 * Part of TernaryBit OS under STEPPPS Mother-Arcing Protocol
 */

#include "tbos_compression.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static tbos_compression_stats_t g_compression_stats = {0};
static int g_compression_initialized = 0;

/* ========================================================================= */
/* INITIALIZATION                                                            */
/* ========================================================================= */

int tbos_compression_init(void) {
    if (g_compression_initialized) {
        return 0;  /* Already initialized */
    }

    /* Reset statistics */
    memset(&g_compression_stats, 0, sizeof(g_compression_stats));

    g_compression_initialized = 1;
    return 0;
}

/* ========================================================================= */
/* MAIN COMPRESSION API                                                      */
/* ========================================================================= */

int tbos_compress(const uint8_t* data, size_t data_len,
                  uint8_t* compressed, size_t* compressed_len,
                  pxfs_mode_t mode) {
    if (!g_compression_initialized) {
        tbos_compression_init();
    }

    /* Allocate pixel buffer */
    size_t max_pixels = tbos_compression_max_size(data_len, mode);
    pxfs_pixel_t* pixels = (pxfs_pixel_t*)malloc(max_pixels * sizeof(pxfs_pixel_t));
    if (!pixels) {
        return -1;  /* Out of memory */
    }

    size_t pixel_count = 0;
    int result = pxfs_encode(data, data_len, pixels, &pixel_count, mode);

    if (result == 0) {
        /* Convert pixels to byte stream */
        *compressed_len = pixel_count * 3;
        for (size_t i = 0; i < pixel_count; i++) {
            compressed[i * 3 + 0] = pixels[i].r;
            compressed[i * 3 + 1] = pixels[i].g;
            compressed[i * 3 + 2] = pixels[i].b;
        }

        /* Update statistics */
        g_compression_stats.total_bytes_in += data_len;
        g_compression_stats.total_bytes_out += *compressed_len;
        g_compression_stats.compression_calls++;
        g_compression_stats.mode_usage[mode]++;

        /* Calculate average compression ratio */
        if (g_compression_stats.total_bytes_out > 0) {
            g_compression_stats.avg_compression_ratio =
                (double)g_compression_stats.total_bytes_in /
                (double)g_compression_stats.total_bytes_out;
        }
    }

    free(pixels);
    return result;
}

int tbos_decompress(const uint8_t* compressed, size_t compressed_len,
                    uint8_t* data, size_t* data_len) {
    if (!g_compression_initialized) {
        tbos_compression_init();
    }

    /* Convert byte stream to pixels */
    size_t pixel_count = compressed_len / 3;
    pxfs_pixel_t* pixels = (pxfs_pixel_t*)malloc(pixel_count * sizeof(pxfs_pixel_t));
    if (!pixels) {
        return -1;
    }

    for (size_t i = 0; i < pixel_count; i++) {
        pixels[i].r = compressed[i * 3 + 0];
        pixels[i].g = compressed[i * 3 + 1];
        pixels[i].b = compressed[i * 3 + 2];
    }

    /* Decode (currently only RAW mode supported in decode) */
    int result = pxfs_decode(pixels, pixel_count, data, data_len, PXFS_MODE_RAW);

    if (result == 0) {
        g_compression_stats.decompression_calls++;
    }

    free(pixels);
    return result;
}

/* ========================================================================= */
/* COMPRESSION MODE RECOMMENDATION                                           */
/* ========================================================================= */

pxfs_mode_t tbos_compression_recommend_mode(const uint8_t* data, size_t data_len) {
    if (data_len < 100) {
        return PXFS_MODE_RAW;  /* Too small for advanced compression */
    }

    /* Analyze data entropy */
    uint8_t byte_freq[256] = {0};
    size_t sample_size = (data_len > 1000) ? 1000 : data_len;

    for (size_t i = 0; i < sample_size; i++) {
        byte_freq[data[i]]++;
    }

    /* Count unique bytes */
    int unique_bytes = 0;
    int max_frequency = 0;
    for (int i = 0; i < 256; i++) {
        if (byte_freq[i] > 0) {
            unique_bytes++;
            if (byte_freq[i] > max_frequency) {
                max_frequency = byte_freq[i];
            }
        }
    }

    /* Calculate pattern ratio */
    double pattern_ratio = (double)max_frequency / (double)sample_size;

    /* Recommend mode based on analysis */
    if (pattern_ratio > 0.5) {
        /* High repetition - use QUANTUM/ULTRA compression */
        return PXFS_MODE_QUANTUM;
    } else if (unique_bytes < 128 && pattern_ratio > 0.3) {
        /* Medium patterns - use FRACTAL */
        return PXFS_MODE_FRACTAL;
    } else if (unique_bytes > 200) {
        /* High entropy - use DENSE packing */
        return PXFS_MODE_DENSE;
    } else {
        /* Default to RAW */
        return PXFS_MODE_RAW;
    }
}

size_t tbos_compression_max_size(size_t data_len, pxfs_mode_t mode) {
    switch (mode) {
        case PXFS_MODE_RAW:
            return (data_len + 2) / 3;  /* Pixels needed */
        case PXFS_MODE_DENSE:
            return (data_len * 8 + 23) / 24;  /* Bit packing */
        case PXFS_MODE_FRACTAL:
            return (data_len + 2) / 3;  /* Worst case: same as RAW */
        case PXFS_MODE_QUANTUM:
            return 1;  /* Just header pixel */
        default:
            return data_len;
    }
}

/* ========================================================================= */
/* IMAGE EXPORT                                                              */
/* ========================================================================= */

int tbos_compression_export_image(const pxfs_pixel_t* pixels, size_t pixel_count,
                                   const char* output_path) {
    /* Calculate image dimensions (square-ish) */
    size_t width = 1;
    while (width * width < pixel_count) {
        width++;
    }
    size_t height = (pixel_count + width - 1) / width;

    FILE* fp = fopen(output_path, "wb");
    if (!fp) {
        return -1;
    }

    /* Write PPM header */
    fprintf(fp, "P6\n%zu %zu\n255\n", width, height);

    /* Write pixel data */
    for (size_t i = 0; i < pixel_count; i++) {
        fputc(pixels[i].r, fp);
        fputc(pixels[i].g, fp);
        fputc(pixels[i].b, fp);
    }

    /* Pad remaining pixels with black */
    for (size_t i = pixel_count; i < width * height; i++) {
        fputc(0, fp);
        fputc(0, fp);
        fputc(0, fp);
    }

    fclose(fp);
    return 0;
}

/* ========================================================================= */
/* STATISTICS                                                                */
/* ========================================================================= */

void tbos_compression_get_stats(tbos_compression_stats_t* stats) {
    memcpy(stats, &g_compression_stats, sizeof(tbos_compression_stats_t));
}

void tbos_compression_reset_stats(void) {
    memset(&g_compression_stats, 0, sizeof(g_compression_stats));
}

/* ========================================================================= */
/* STEPPPS INTEGRATION                                                       */
/* ========================================================================= */

int tbos_steppps_compress(const uint8_t* data, size_t data_len,
                          uint8_t* steppps_compressed, size_t* steppps_len) {
    /*
     * STEPPPS Compression Protocol:
     * Uses ternary state encoding: -1 (delta), 0 (neutral), +1 (alpha)
     *
     * Each byte can be represented as ternary deltas from previous state
     * This provides additional compression for sequential data
     */

    /* For now, use recommended mode with PXFS */
    pxfs_mode_t mode = tbos_compression_recommend_mode(data, data_len);
    return tbos_compress(data, data_len, steppps_compressed, steppps_len, mode);
}

int tbos_steppps_decompress(const uint8_t* steppps_compressed, size_t steppps_len,
                            uint8_t* data, size_t* data_len) {
    /* Decompress STEPPPS-encoded data */
    return tbos_decompress(steppps_compressed, steppps_len, data, data_len);
}

/* ========================================================================= */
/* CONSCIOUSNESS-AWARE COMPRESSION                                           */
/* ========================================================================= */

int tbos_conscious_compress(const uint8_t* data, size_t data_len,
                            uint8_t consciousness_level,
                            uint8_t* compressed, size_t* compressed_len) {
    /*
     * Consciousness-aware compression:
     * - High consciousness (>200): Use QUANTUM for maximum compression
     * - Medium consciousness (100-200): Use FRACTAL for pattern detection
     * - Low consciousness (<100): Use DENSE for reliable compression
     */

    pxfs_mode_t mode;
    if (consciousness_level > 200) {
        mode = PXFS_MODE_QUANTUM;
    } else if (consciousness_level > 100) {
        mode = PXFS_MODE_FRACTAL;
    } else {
        mode = PXFS_MODE_DENSE;
    }

    return tbos_compress(data, data_len, compressed, compressed_len, mode);
}
