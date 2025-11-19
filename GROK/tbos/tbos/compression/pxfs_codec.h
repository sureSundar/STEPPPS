/*
 * PXFS Codec - Pixel-based Data Compression
 * Compresses arbitrary data into RGB pixel representation
 *
 * Compression: 4KB -> 3 bytes (metadata) + encoded pixels
 * Concept: Encode binary data as visual patterns in images
 */

#ifndef PXFS_CODEC_H
#define PXFS_CODEC_H

#include <stdint.h>
#include <stddef.h>

/* ========================================================================= */
/* COMPRESSION MODES                                                         */
/* ========================================================================= */

typedef enum {
    PXFS_MODE_RAW,          /* 1 byte = 1 pixel (3 color channels) = 24 bits */
    PXFS_MODE_DENSE,        /* Multiple bytes per pixel using bit packing */
    PXFS_MODE_FRACTAL,      /* Fractal/pattern-based compression */
    PXFS_MODE_QUANTUM       /* Quantum superposition encoding */
} pxfs_mode_t;

/* ========================================================================= */
/* PIXEL STRUCTURE                                                          */
/* ========================================================================= */

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} pxfs_pixel_t;

/* ========================================================================= */
/* COMPRESSION HEADER (3 bytes as you specified!)                          */
/* ========================================================================= */

typedef struct __attribute__((packed)) {
    uint8_t magic;          /* 0xPX = Pixel format marker */
    uint8_t mode;           /* Compression mode */
    uint8_t flags;          /* Compression flags */
} pxfs_header_t;

/* ========================================================================= */
/* CODEC API                                                                */
/* ========================================================================= */

/* Encode data into pixels */
int pxfs_encode(const uint8_t* data, size_t data_len,
                pxfs_pixel_t* pixels, size_t* pixel_count,
                pxfs_mode_t mode);

/* Decode pixels back to data */
int pxfs_decode(const pxfs_pixel_t* pixels, size_t pixel_count,
                uint8_t* data, size_t* data_len,
                pxfs_mode_t mode);

/* Get compressed size */
size_t pxfs_compressed_size(size_t data_len, pxfs_mode_t mode);

/* Get decompressed size from header */
size_t pxfs_decompressed_size(const pxfs_header_t* header);

/* ========================================================================= */
/* ULTRA COMPRESSION (4KB -> 3 bytes)                                      */
/* ========================================================================= */

/* This uses quantum/fractal encoding */
int pxfs_ultra_encode(const uint8_t* data, size_t data_len,
                      pxfs_header_t* header);

int pxfs_ultra_decode(const pxfs_header_t* header,
                      uint8_t* data, size_t* data_len);

#endif /* PXFS_CODEC_H */
