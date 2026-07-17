/**
 * pxfs_steppps.c - PXFS STEPPPS Implementation
 *
 * Lossless embedding of STEPPPS JSON in PXFS pixel stream.
 */

#include "pxfs_steppps.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================= */
/* VALIDATION                                                                 */
/* ========================================================================= */

static bool pxfs_validate_header(const uint8_t* data, size_t len) {
    if (len < PXFS_STEPPPS_V2_HEADER_SIZE) return false;
    if (memcmp(data, PXFS_STEPPPS_MAGIC, 4) != 0) return false;
    if (data[4] == PXFS_STEPPPS_VERSION && len < PXFS_STEPPPS_HEADER_SIZE) {
        return false;
    }
    if (data[4] != PXFS_STEPPPS_VERSION &&
        data[4] != PXFS_STEPPPS_VERSION_LEGACY) {
        return false;
    }
    return true;
}

static size_t pxfs_header_size(const pxfs_header_t* hdr) {
    return hdr->version >= PXFS_STEPPPS_VERSION
        ? PXFS_STEPPPS_HEADER_SIZE
        : PXFS_STEPPPS_V2_HEADER_SIZE;
}

bool pxfs_has_steppps(const uint8_t* data, size_t len) {
    if (!pxfs_validate_header(data, len)) return false;
    const pxfs_header_t* hdr = (const pxfs_header_t*)data;
    return (hdr->flags & PXFS_FLAG_STEPPPS) && (hdr->steppps_len > 0);
}

uint16_t pxfs_get_steppps_length(const uint8_t* data, size_t len) {
    if (!pxfs_validate_header(data, len)) return 0;
    const pxfs_header_t* hdr = (const pxfs_header_t*)data;
    return hdr->steppps_len;
}

/* ========================================================================= */
/* CREATE                                                                     */
/* ========================================================================= */

int pxfs_create_with_steppps(
    const char* steppps_json,
    const uint8_t* content,
    size_t content_len,
    uint8_t* output,
    size_t output_max,
    size_t* output_len
) {
    if (!output || !output_len) return -1;

    size_t steppps_len = steppps_json ? strlen(steppps_json) : 0;
    if (steppps_len > PXFS_STEPPPS_MAX_JSON) return -1;

    /* Calculate sizes */
    size_t steppps_pixels = (steppps_len + 2) / 3;
    size_t content_pixels = (content_len + 2) / 3;
    size_t total_size = PXFS_STEPPPS_HEADER_SIZE + (steppps_pixels * 3) + (content_pixels * 3);

    if (total_size > output_max) return -1;

    /* Write header */
    pxfs_header_t* hdr = (pxfs_header_t*)output;
    memcpy(hdr->magic, PXFS_STEPPPS_MAGIC, 4);
    hdr->version = PXFS_STEPPPS_VERSION;
    hdr->flags = steppps_json ? PXFS_FLAG_STEPPPS : 0;
    hdr->steppps_len = (uint16_t)steppps_len;
    hdr->content_len = (uint32_t)content_len;

    size_t pos = PXFS_STEPPPS_HEADER_SIZE;

    /* Write STEPPPS as pixels */
    if (steppps_json && steppps_len > 0) {
        size_t pixels_written = pxfs_bytes_to_pixels(
            (const uint8_t*)steppps_json,
            steppps_len,
            output + pos,
            steppps_pixels
        );
        pos += pixels_written * 3;
    }

    /* Write content as pixels */
    if (content && content_len > 0) {
        size_t pixels_written = pxfs_bytes_to_pixels(
            content,
            content_len,
            output + pos,
            content_pixels
        );
        pos += pixels_written * 3;
    }

    *output_len = pos;
    return 0;
}

/* ========================================================================= */
/* READ                                                                       */
/* ========================================================================= */

int pxfs_read_with_steppps(
    const uint8_t* input,
    size_t input_len,
    char* steppps_json,
    size_t steppps_max,
    uint8_t* content,
    size_t content_max,
    size_t* content_len
) {
    if (!input || input_len < PXFS_STEPPPS_V2_HEADER_SIZE) return -1;

    /* Validate header */
    if (!pxfs_validate_header(input, input_len)) return -1;

    const pxfs_header_t* hdr = (const pxfs_header_t*)input;

    size_t pos = pxfs_header_size(hdr);
    uint16_t steppps_len = hdr->steppps_len;

    /* Read STEPPPS */
    if ((hdr->flags & PXFS_FLAG_STEPPPS) && steppps_len > 0) {
        size_t steppps_pixels = (steppps_len + 2) / 3;
        size_t steppps_bytes = steppps_pixels * 3;

        if (pos + steppps_bytes > input_len) return -1;

        if (steppps_json && steppps_max > 0) {
            if (steppps_len >= steppps_max) return -1;

            pxfs_pixels_to_bytes(
                input + pos,
                steppps_pixels,
                (uint8_t*)steppps_json,
                steppps_len
            );
            steppps_json[steppps_len] = '\0';
        }

        pos += steppps_bytes;
    } else if (steppps_json && steppps_max > 0) {
        steppps_json[0] = '\0';
    }

    /* Read content */
    size_t remaining = input_len - pos;
    size_t remaining_pixels = remaining / 3;
    size_t raw_content_len = hdr->version >= PXFS_STEPPPS_VERSION
        ? hdr->content_len
        : remaining_pixels * 3;
    if (raw_content_len > remaining_pixels * 3) return -1;

    if (content && content_max > 0) {
        if (raw_content_len > content_max) return -1;

        size_t bytes_read = pxfs_pixels_to_bytes(
            input + pos,
            remaining_pixels,
            content,
            raw_content_len
        );

        if (content_len) *content_len = bytes_read;
    } else if (content_len) {
        *content_len = raw_content_len;
    }

    return 0;
}

/* ========================================================================= */
/* UPDATE                                                                     */
/* ========================================================================= */

int pxfs_update_steppps(
    uint8_t* pxfs_data,
    size_t pxfs_len,
    size_t pxfs_max,
    const char* new_steppps_json,
    size_t* new_pxfs_len
) {
    if (!pxfs_data || !new_pxfs_len) return -1;
    if (!pxfs_validate_header(pxfs_data, pxfs_len)) return -1;

    /* Read existing content */
    pxfs_header_t* hdr = (pxfs_header_t*)pxfs_data;
    size_t header_size = pxfs_header_size(hdr);
    uint16_t old_steppps_len = hdr->steppps_len;
    size_t old_steppps_pixels = (old_steppps_len + 2) / 3;
    size_t old_steppps_bytes = old_steppps_pixels * 3;

    size_t content_start = header_size + old_steppps_bytes;
    size_t content_bytes = pxfs_len - content_start;

    /* Calculate new sizes */
    size_t new_steppps_len = new_steppps_json ? strlen(new_steppps_json) : 0;
    if (new_steppps_len > PXFS_STEPPPS_MAX_JSON) return -1;

    size_t new_steppps_pixels = (new_steppps_len + 2) / 3;
    size_t new_steppps_bytes = new_steppps_pixels * 3;

    size_t new_total = header_size + new_steppps_bytes + content_bytes;
    if (new_total > pxfs_max) return -1;

    /* Shift content if needed */
    if (new_steppps_bytes != old_steppps_bytes) {
        memmove(
            pxfs_data + header_size + new_steppps_bytes,
            pxfs_data + content_start,
            content_bytes
        );
    }

    /* Write new STEPPPS */
    hdr->steppps_len = (uint16_t)new_steppps_len;
    if (new_steppps_json) {
        hdr->flags |= PXFS_FLAG_STEPPPS;
        pxfs_bytes_to_pixels(
            (const uint8_t*)new_steppps_json,
            new_steppps_len,
            pxfs_data + header_size,
            new_steppps_pixels
        );
    } else {
        hdr->flags &= ~PXFS_FLAG_STEPPPS;
    }

    *new_pxfs_len = new_total;
    return 0;
}

/* ========================================================================= */
/* DEBUG / PRINT                                                              */
/* ========================================================================= */

void pxfs_print_info(const uint8_t* data, size_t len) {
    if (!pxfs_validate_header(data, len)) {
        printf("Invalid PXFS data\n");
        return;
    }

    const pxfs_header_t* hdr = (const pxfs_header_t*)data;

    printf("PXFS File Info:\n");
    printf("  Magic:    %.4s\n", hdr->magic);
    printf("  Version:  %d\n", hdr->version);
    printf("  Flags:    0x%02X", hdr->flags);
    if (hdr->flags & PXFS_FLAG_STEPPPS) printf(" [STEPPPS]");
    if (hdr->flags & PXFS_FLAG_SIGNED) printf(" [SIGNED]");
    if (hdr->flags & PXFS_FLAG_ENCRYPTED) printf(" [ENCRYPTED]");
    if (hdr->flags & PXFS_FLAG_COMPRESSED) printf(" [COMPRESSED]");
    printf("\n");
    printf("  STEPPPS:  %d bytes\n", hdr->steppps_len);

    size_t steppps_pixels = (hdr->steppps_len + 2) / 3;
    size_t content_start = pxfs_header_size(hdr) + (steppps_pixels * 3);
    size_t content_bytes = hdr->version >= PXFS_STEPPPS_VERSION
        ? hdr->content_len
        : len - content_start;
    printf("  Content:  %zu bytes\n", content_bytes);
    printf("  Total:    %zu bytes\n", len);
}
