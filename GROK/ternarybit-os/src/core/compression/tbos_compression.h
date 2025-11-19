/*
 * TBOS Compression Module - Integration Layer
 * Part of TernaryBit OS under STEPPPS Mother-Arcing Protocol
 *
 * This module integrates PXFS (Pixel File System) compression
 * into the TernaryBit OS ecosystem, providing:
 * - Ultra-high compression ratios for pattern-based data
 * - Pixel-based visual data representation
 * - STEPPPS-aligned data encoding
 *
 * Compression Modes:
 * - RAW: 1:1 pixel encoding (3 bytes per pixel)
 * - DENSE: Bit-packed encoding (~1.3:1)
 * - FRACTAL: Pattern detection (variable)
 * - ULTRA: Extreme pattern compression (up to 1365:1)
 */

#ifndef TBOS_COMPRESSION_H
#define TBOS_COMPRESSION_H

#include "pxfs_codec.h"
#include "../tbos_base.h"

/* ========================================================================= */
/* TBOS COMPRESSION API                                                      */
/* ========================================================================= */

/**
 * Initialize TBOS compression subsystem
 * Integrates with TBOS memory manager and consciousness layers
 */
int tbos_compression_init(void);

/**
 * Compress data using PXFS codec with TBOS integration
 *
 * @param data Input data buffer
 * @param data_len Input data length
 * @param compressed Output compressed buffer (caller allocates)
 * @param compressed_len Output compressed length
 * @param mode Compression mode (RAW, DENSE, FRACTAL, QUANTUM)
 * @return 0 on success, negative on error
 */
int tbos_compress(const uint8_t* data, size_t data_len,
                  uint8_t* compressed, size_t* compressed_len,
                  pxfs_mode_t mode);

/**
 * Decompress PXFS-encoded data
 *
 * @param compressed Input compressed buffer
 * @param compressed_len Input compressed length
 * @param data Output decompressed buffer (caller allocates)
 * @param data_len Output decompressed length
 * @return 0 on success, negative on error
 */
int tbos_decompress(const uint8_t* compressed, size_t compressed_len,
                    uint8_t* data, size_t* data_len);

/**
 * Get recommended compression mode for data type
 * Analyzes data characteristics and returns optimal mode
 *
 * @param data Input data buffer
 * @param data_len Input data length
 * @return Recommended pxfs_mode_t
 */
pxfs_mode_t tbos_compression_recommend_mode(const uint8_t* data, size_t data_len);

/**
 * Calculate maximum compressed size needed for buffer allocation
 *
 * @param data_len Input data length
 * @param mode Compression mode
 * @return Maximum possible compressed size in bytes
 */
size_t tbos_compression_max_size(size_t data_len, pxfs_mode_t mode);

/**
 * Export compressed data as PPM image file (for visual debugging)
 * Creates a visual representation of the compressed data
 *
 * @param pixels Pixel array
 * @param pixel_count Number of pixels
 * @param output_path Path to output PPM file
 * @return 0 on success, negative on error
 */
int tbos_compression_export_image(const pxfs_pixel_t* pixels, size_t pixel_count,
                                   const char* output_path);

/* ========================================================================= */
/* TBOS-SPECIFIC COMPRESSION STATISTICS                                      */
/* ========================================================================= */

typedef struct {
    uint64_t total_bytes_in;       /* Total input bytes */
    uint64_t total_bytes_out;      /* Total output bytes */
    uint64_t compression_calls;    /* Number of compress operations */
    uint64_t decompression_calls;  /* Number of decompress operations */
    double avg_compression_ratio;  /* Average compression ratio */
    uint32_t mode_usage[4];        /* Usage count per mode */
} tbos_compression_stats_t;

/**
 * Get compression statistics
 * Returns global compression statistics for monitoring
 */
void tbos_compression_get_stats(tbos_compression_stats_t* stats);

/**
 * Reset compression statistics
 */
void tbos_compression_reset_stats(void);

/* ========================================================================= */
/* STEPPPS PROTOCOL INTEGRATION                                              */
/* ========================================================================= */

/**
 * Compress data using STEPPPS-aligned patterns
 * Utilizes ternary state encoding for enhanced compression
 *
 * @param data Input data
 * @param data_len Input length
 * @param steppps_compressed Output buffer
 * @param steppps_len Output length
 * @return 0 on success, negative on error
 */
int tbos_steppps_compress(const uint8_t* data, size_t data_len,
                          uint8_t* steppps_compressed, size_t* steppps_len);

/**
 * Decompress STEPPPS-encoded data
 */
int tbos_steppps_decompress(const uint8_t* steppps_compressed, size_t steppps_len,
                            uint8_t* data, size_t* data_len);

/* ========================================================================= */
/* CONSCIOUSNESS-AWARE COMPRESSION                                           */
/* ========================================================================= */

/**
 * Compress with consciousness level tracking
 * Higher consciousness data gets priority compression modes
 *
 * @param data Input data
 * @param data_len Input length
 * @param consciousness_level Consciousness level (0-255)
 * @param compressed Output buffer
 * @param compressed_len Output length
 * @return 0 on success, negative on error
 */
int tbos_conscious_compress(const uint8_t* data, size_t data_len,
                            uint8_t consciousness_level,
                            uint8_t* compressed, size_t* compressed_len);

#endif /* TBOS_COMPRESSION_H */
