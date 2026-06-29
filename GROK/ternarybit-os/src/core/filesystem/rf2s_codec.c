/*
 * RF2S (Radio Frequency Filesystem) Codec Implementation
 * Parses frequency-based paths and normalizes to Hz
 */

#include "fs/rf2s_codec.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

/* Helper: duplicate a substring */
static char* dup_bytes(const char* start, size_t len) {
    char* out = malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

/* Helper: check if character is a digit */
static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

/* Helper: check if character is alphabetic */
static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/* Helper: convert character to lowercase */
static char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') return c + ('a' - 'A');
    return c;
}

/* Helper: case-insensitive string prefix match */
static int starts_with_ci(const char* str, const char* prefix) {
    while (*prefix) {
        if (to_lower(*str) != to_lower(*prefix)) return 0;
        str++;
        prefix++;
    }
    return 1;
}

/* Parse a frequency string into rf2s_freq_t */
int rf2s_parse_frequency(const char* str, rf2s_freq_t* out) {
    if (!str || !out) return -EINVAL;

    memset(out, 0, sizeof(*out));

    /* Skip leading whitespace */
    while (*str == ' ' || *str == '\t') str++;

    /* Must start with a digit */
    if (!is_digit(*str)) return -EINVAL;

    /* Parse the numeric part (integer or decimal) */
    double value = 0.0;
    while (is_digit(*str)) {
        value = value * 10.0 + (*str - '0');
        str++;
    }

    /* Handle decimal part */
    if (*str == '.') {
        str++;
        double fraction = 0.1;
        while (is_digit(*str)) {
            value += (*str - '0') * fraction;
            fraction *= 0.1;
            str++;
        }
    }

    out->original_value = value;

    /* Skip whitespace before unit */
    while (*str == ' ' || *str == '\t') str++;

    /* Parse unit suffix (case insensitive) */
    uint64_t multiplier = 1;
    out->original_unit = RF2S_UNIT_HZ;

    if (*str != '\0' && is_alpha(*str)) {
        if (starts_with_ci(str, "ghz")) {
            multiplier = 1000000000ULL;
            out->original_unit = RF2S_UNIT_GHZ;
            str += 3;
        } else if (starts_with_ci(str, "mhz")) {
            multiplier = 1000000ULL;
            out->original_unit = RF2S_UNIT_MHZ;
            str += 3;
        } else if (starts_with_ci(str, "khz")) {
            multiplier = 1000ULL;
            out->original_unit = RF2S_UNIT_KHZ;
            str += 3;
        } else if (starts_with_ci(str, "hz")) {
            multiplier = 1;
            out->original_unit = RF2S_UNIT_HZ;
            str += 2;
        } else {
            return -EINVAL;  /* Unknown unit */
        }
    }

    /* Skip trailing whitespace */
    while (*str == ' ' || *str == '\t') str++;

    /* Must be at end of string or at a path separator */
    if (*str != '\0' && *str != '/') {
        return -EINVAL;
    }

    /* Calculate frequency in Hz */
    out->frequency_hz = (uint64_t)(value * (double)multiplier);

    /* Format normalized string */
    snprintf(out->normalized, sizeof(out->normalized), "%llu",
             (unsigned long long)out->frequency_hz);

    return 0;
}

/* Check if a path component looks like a frequency */
int rf2s_is_frequency(const char* component) {
    if (!component || !*component) return 0;

    /* Must start with a digit */
    if (!is_digit(*component)) return 0;

    /* Skip digits and decimal point */
    while (is_digit(*component) || *component == '.') {
        component++;
    }

    /* Skip optional whitespace */
    while (*component == ' ' || *component == '\t') {
        component++;
    }

    /* Must end or have a valid unit */
    if (*component == '\0') return 1;

    if (starts_with_ci(component, "hz") ||
        starts_with_ci(component, "khz") ||
        starts_with_ci(component, "mhz") ||
        starts_with_ci(component, "ghz")) {
        return 1;
    }

    return 0;
}

/* Parse a full RF2S path */
int rf2s_parse(const char* path, rf2s_path_t* out) {
    if (!path || !out) return -EINVAL;

    memset(out, 0, sizeof(*out));

    /* Skip leading slash if present */
    const char* cursor = path;
    if (*cursor == '/') cursor++;

    /* Find the end of the frequency component */
    const char* freq_start = cursor;
    while (*cursor && *cursor != '/') cursor++;

    /* Extract frequency component */
    size_t freq_len = (size_t)(cursor - freq_start);
    if (freq_len == 0) return -EINVAL;

    char* freq_str = dup_bytes(freq_start, freq_len);
    if (!freq_str) return -ENOMEM;

    /* Parse the frequency */
    int rc = rf2s_parse_frequency(freq_str, &out->frequency);
    free(freq_str);
    if (rc != 0) return rc;

    /* Skip the separator */
    if (*cursor == '/') cursor++;

    /* Parse remaining path components */
    const char* segment_start = cursor;
    size_t capacity = 4;
    out->components = malloc(sizeof(char*) * capacity);
    if (!out->components) return -ENOMEM;

    while (*cursor) {
        if (*cursor == '/') {
            size_t seg_len = (size_t)(cursor - segment_start);
            if (seg_len > 0) {
                if (out->component_count == capacity) {
                    capacity *= 2;
                    char** resized = realloc(out->components, sizeof(char*) * capacity);
                    if (!resized) {
                        rf2s_free(out);
                        return -ENOMEM;
                    }
                    out->components = resized;
                }
                char* dup = dup_bytes(segment_start, seg_len);
                if (!dup) {
                    rf2s_free(out);
                    return -ENOMEM;
                }
                out->components[out->component_count++] = dup;
            }
            cursor++;
            segment_start = cursor;
            continue;
        }
        cursor++;
    }

    /* Handle last segment */
    if (cursor != segment_start) {
        size_t seg_len = (size_t)(cursor - segment_start);
        if (out->component_count == capacity) {
            capacity *= 2;
            char** resized = realloc(out->components, sizeof(char*) * capacity);
            if (!resized) {
                rf2s_free(out);
                return -ENOMEM;
            }
            out->components = resized;
        }
        char* dup = dup_bytes(segment_start, seg_len);
        if (!dup) {
            rf2s_free(out);
            return -ENOMEM;
        }
        out->components[out->component_count++] = dup;
    }

    return 0;
}

/* Free a parsed RF2S path */
void rf2s_free(rf2s_path_t* path) {
    if (!path) return;
    if (path->components) {
        for (size_t i = 0; i < path->component_count; i++) {
            free(path->components[i]);
        }
        free(path->components);
    }
    memset(path, 0, sizeof(*path));
}

/* Helper: append string to buffer */
static int append_string(char* buffer, size_t buflen, size_t* offset, const char* text) {
    size_t len = strlen(text);
    if (*offset + len >= buflen) return -ENOSPC;
    memcpy(buffer + *offset, text, len);
    *offset += len;
    buffer[*offset] = '\0';
    return 0;
}

/* Convert RF2S path to canonical backing path with custom base */
int rf2s_to_canonical_with_base(const rf2s_path_t* path, const char* base,
                                 char* buffer, size_t buflen) {
    if (!path || !buffer || buflen == 0 || !base || base[0] != '/') {
        return -EINVAL;
    }

    size_t offset = 0;

    /* Append base path */
    if (append_string(buffer, buflen, &offset, base) != 0) {
        return -ENOSPC;
    }

    /* Ensure trailing slash after base */
    if (offset > 0 && buffer[offset - 1] != '/') {
        if (append_string(buffer, buflen, &offset, "/") != 0) {
            return -ENOSPC;
        }
    }

    /* Append normalized frequency */
    if (append_string(buffer, buflen, &offset, path->frequency.normalized) != 0) {
        return -ENOSPC;
    }

    /* Append remaining path components */
    for (size_t i = 0; i < path->component_count; i++) {
        if (append_string(buffer, buflen, &offset, "/") != 0) {
            return -ENOSPC;
        }
        if (append_string(buffer, buflen, &offset, path->components[i]) != 0) {
            return -ENOSPC;
        }
    }

    return 0;
}

/* Convert RF2S path to canonical backing path (default base: /rf2s) */
int rf2s_to_canonical(const rf2s_path_t* path, char* buffer, size_t buflen) {
    return rf2s_to_canonical_with_base(path, "/rf2s", buffer, buflen);
}

/* Format a frequency for display */
int rf2s_format_frequency(uint64_t hz, char* buffer, size_t buflen) {
    if (!buffer || buflen < 16) return -EINVAL;

    if (hz >= 1000000000ULL && (hz % 1000000000ULL) == 0) {
        snprintf(buffer, buflen, "%llu GHz", (unsigned long long)(hz / 1000000000ULL));
    } else if (hz >= 1000000000ULL) {
        snprintf(buffer, buflen, "%.3f GHz", (double)hz / 1000000000.0);
    } else if (hz >= 1000000ULL && (hz % 1000000ULL) == 0) {
        snprintf(buffer, buflen, "%llu MHz", (unsigned long long)(hz / 1000000ULL));
    } else if (hz >= 1000000ULL) {
        snprintf(buffer, buflen, "%.3f MHz", (double)hz / 1000000.0);
    } else if (hz >= 1000ULL && (hz % 1000ULL) == 0) {
        snprintf(buffer, buflen, "%llu kHz", (unsigned long long)(hz / 1000ULL));
    } else if (hz >= 1000ULL) {
        snprintf(buffer, buflen, "%.3f kHz", (double)hz / 1000.0);
    } else {
        snprintf(buffer, buflen, "%llu Hz", (unsigned long long)hz);
    }

    return 0;
}
