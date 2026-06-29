/*
 * RF2S (Radio Frequency Filesystem) Codec
 * Parses and normalizes frequency-based paths
 *
 * Path formats supported:
 *   /rf2s/432000000/file.dat      (raw Hz)
 *   /rf2s/432MHz/file.dat         (with unit suffix)
 *   /rf2s/2.4GHz/wifi/beacon      (decimal + unit)
 */

#ifndef TBOS_FS_RF2S_CODEC_H
#define TBOS_FS_RF2S_CODEC_H

#include <stddef.h>
#include <stdint.h>

/* Frequency unit types */
typedef enum {
    RF2S_UNIT_HZ = 0,
    RF2S_UNIT_KHZ,
    RF2S_UNIT_MHZ,
    RF2S_UNIT_GHZ
} rf2s_unit_t;

/* Parsed frequency structure */
typedef struct {
    uint64_t frequency_hz;      /* Normalized to Hz */
    rf2s_unit_t original_unit;  /* Original unit from input */
    double original_value;      /* Original numeric value */
    char normalized[32];        /* String representation of Hz value */
} rf2s_freq_t;

/* Parsed RF2S path */
typedef struct {
    rf2s_freq_t frequency;      /* First component: frequency */
    size_t component_count;     /* Number of path components after frequency */
    char** components;          /* Path components after frequency */
} rf2s_path_t;

/*
 * Parse a frequency string into rf2s_freq_t
 *
 * Supported formats:
 *   "432"       -> 432 Hz
 *   "432Hz"     -> 432 Hz
 *   "432kHz"    -> 432000 Hz
 *   "432.5kHz"  -> 432500 Hz
 *   "2.4MHz"    -> 2400000 Hz
 *   "432MHz"    -> 432000000 Hz
 *   "2.4GHz"    -> 2400000000 Hz
 *
 * Returns: 0 on success, -EINVAL on invalid format
 */
int rf2s_parse_frequency(const char* str, rf2s_freq_t* out);

/*
 * Parse a full RF2S path
 *
 * Input:  "/432MHz/sensor/data.bin"
 * Output: frequency=432000000, components=["sensor", "data.bin"]
 *
 * Returns: 0 on success, negative errno on error
 */
int rf2s_parse(const char* path, rf2s_path_t* out);

/*
 * Free a parsed RF2S path
 */
void rf2s_free(rf2s_path_t* path);

/*
 * Convert RF2S path to canonical backing path
 *
 * Input:  "/432MHz/sensor/data.bin", base="/rf2s"
 * Output: "/rf2s/432000000/sensor/data.bin"
 *
 * Returns: 0 on success, -ENOSPC if buffer too small
 */
int rf2s_to_canonical(const rf2s_path_t* path, char* buffer, size_t buflen);
int rf2s_to_canonical_with_base(const rf2s_path_t* path, const char* base,
                                 char* buffer, size_t buflen);

/*
 * Check if a path component looks like a frequency
 * Returns 1 if it's a frequency, 0 otherwise
 */
int rf2s_is_frequency(const char* component);

/*
 * Format a frequency for display (e.g., 432000000 -> "432 MHz")
 */
int rf2s_format_frequency(uint64_t hz, char* buffer, size_t buflen);

#endif /* TBOS_FS_RF2S_CODEC_H */
