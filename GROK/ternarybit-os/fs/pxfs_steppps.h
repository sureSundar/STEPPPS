/**
 * pxfs_steppps.h - PXFS STEPPPS Header Format
 *
 * Embeds STEPPPS metadata losslessly in PXFS pixel stream.
 *
 * PXFS File Format with STEPPPS:
 * ┌────────────────────────────────────────────────┐
 * │ Magic: "PXFS" (4 bytes)                        │
 * │ Version: 2 (1 byte)                            │
 * │ Flags: HAS_STEPPPS (1 byte)                    │
 * │ STEPPPS Header Length (2 bytes)                │
 * ├────────────────────────────────────────────────┤
 * │ STEPPPS Header (variable, JSON UTF-8)          │
 * │ - Encoded as pixels: 3 bytes/pixel (RGB)       │
 * │ - Each byte = one color channel                │
 * │ - Padding to align to pixel boundary           │
 * ├────────────────────────────────────────────────┤
 * │ Content Pixels (actual file data)              │
 * └────────────────────────────────────────────────┘
 *
 * This is LOSSLESS because:
 * - JSON is UTF-8 text, stored byte-for-byte in RGB channels
 * - No color space conversion or compression
 * - Perfect round-trip: JSON → pixels → JSON
 */

#ifndef PXFS_STEPPPS_H
#define PXFS_STEPPPS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* CONSTANTS                                                                  */
/* ========================================================================= */

#define PXFS_STEPPPS_MAGIC          "PXFS"
#define PXFS_STEPPPS_VERSION        2
#define PXFS_FLAG_STEPPPS   0x01    /* Has STEPPPS header */
#define PXFS_FLAG_SIGNED    0x02    /* STEPPPS is signed */
#define PXFS_FLAG_ENCRYPTED 0x04    /* Content is encrypted */
#define PXFS_FLAG_COMPRESSED 0x08   /* Content is compressed */

#define PXFS_STEPPPS_HEADER_SIZE    8     /* Magic + version + flags + JSON length */
#define PXFS_STEPPPS_MAX_JSON       65535 /* Max embedded STEPPPS JSON size */

/* ========================================================================= */
/* PXFS HEADER                                                                */
/* ========================================================================= */

typedef struct __attribute__((packed)) {
    char magic[4];              /* "PXFS" */
    uint8_t version;            /* 2 */
    uint8_t flags;              /* PXFS_FLAG_* */
    uint16_t steppps_len;       /* Length of STEPPPS JSON (0 if none) */
} pxfs_header_t;

/* ========================================================================= */
/* PIXEL ENCODING                                                             */
/* ========================================================================= */

/**
 * Encode bytes as pixels (3 bytes per pixel, RGB)
 * Returns number of pixels written
 */
static inline size_t pxfs_bytes_to_pixels(
    const uint8_t* bytes,
    size_t byte_count,
    uint8_t* pixels,
    size_t max_pixels
) {
    size_t pixel_count = (byte_count + 2) / 3;  /* Ceiling division */
    if (pixel_count > max_pixels) return 0;

    size_t p = 0;
    for (size_t i = 0; i < byte_count; i += 3) {
        pixels[p * 3 + 0] = bytes[i];                           /* R */
        pixels[p * 3 + 1] = (i + 1 < byte_count) ? bytes[i + 1] : 0;  /* G */
        pixels[p * 3 + 2] = (i + 2 < byte_count) ? bytes[i + 2] : 0;  /* B */
        p++;
    }

    return pixel_count;
}

/**
 * Decode pixels to bytes (3 bytes per pixel, RGB)
 * Returns number of bytes written
 */
static inline size_t pxfs_pixels_to_bytes(
    const uint8_t* pixels,
    size_t pixel_count,
    uint8_t* bytes,
    size_t actual_byte_count  /* Original byte count, to handle padding */
) {
    size_t written = 0;
    for (size_t p = 0; p < pixel_count && written < actual_byte_count; p++) {
        bytes[written++] = pixels[p * 3 + 0];  /* R */
        if (written < actual_byte_count) bytes[written++] = pixels[p * 3 + 1];  /* G */
        if (written < actual_byte_count) bytes[written++] = pixels[p * 3 + 2];  /* B */
    }
    return written;
}

/* ========================================================================= */
/* PXFS STEPPPS API                                                           */
/* ========================================================================= */

/**
 * Create PXFS file with embedded STEPPPS
 *
 * @param steppps_json  STEPPPS metadata as JSON string (NULL for none)
 * @param content       File content bytes
 * @param content_len   Length of content
 * @param output        Output buffer for PXFS data
 * @param output_max    Maximum output size
 * @param output_len    Actual output length written
 * @return 0 on success, -1 on error
 */
int pxfs_create_with_steppps(
    const char* steppps_json,
    const uint8_t* content,
    size_t content_len,
    uint8_t* output,
    size_t output_max,
    size_t* output_len
);

/**
 * Read PXFS file and extract STEPPPS + content
 *
 * @param input         PXFS data
 * @param input_len     Length of PXFS data
 * @param steppps_json  Output buffer for STEPPPS JSON (NULL to skip)
 * @param steppps_max   Max STEPPPS buffer size
 * @param content       Output buffer for content (NULL to skip)
 * @param content_max   Max content buffer size
 * @param content_len   Actual content length
 * @return 0 on success, -1 on error
 */
int pxfs_read_with_steppps(
    const uint8_t* input,
    size_t input_len,
    char* steppps_json,
    size_t steppps_max,
    uint8_t* content,
    size_t content_max,
    size_t* content_len
);

/**
 * Check if PXFS data has embedded STEPPPS
 */
bool pxfs_has_steppps(const uint8_t* data, size_t len);

/**
 * Get STEPPPS length from PXFS header
 */
uint16_t pxfs_get_steppps_length(const uint8_t* data, size_t len);

/**
 * Update STEPPPS in existing PXFS file (in-place if possible)
 */
int pxfs_update_steppps(
    uint8_t* pxfs_data,
    size_t pxfs_len,
    size_t pxfs_max,
    const char* new_steppps_json,
    size_t* new_pxfs_len
);

/**
 * Print PXFS file info
 */
void pxfs_print_info(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* PXFS_STEPPPS_H */
