#ifndef TBOS_BOOT_DESCRIPTOR_H
#define TBOS_BOOT_DESCRIPTOR_H

#include <stdint.h>

#define TBDS_SIGNATURE 0x53444454u /* 'TBDS' little-endian */
#define TBDS_HEADER_RESERVED_BYTES 8

/* Descriptor flag bits */
#define TBDS_FLAG_CRITICAL 0x0001u
#define TBDS_FLAG_COMPRESSED 0x0002u

/* Descriptor types (initial) */
#define TBDS_TYPE_ARCH_INFO           0x0001u
#define TBDS_TYPE_FIRMWARE_INFO       0x0002u
#define TBDS_TYPE_MEMORY_MAP          0x0003u
#define TBDS_TYPE_BOOT_DEVICE         0x0004u
#define TBDS_TYPE_VIDEO_INFO          0x0005u
#define TBDS_TYPE_CONSOLE_INFO        0x0006u
#define TBDS_TYPE_STEPPPS_TELEMETRY   0x0007u
#define TBDS_TYPE_SECURITY_ATTESTATION 0x0008u
#define TBDS_TYPE_VENDOR_CUSTOM       0x7FFFu

#pragma pack(push, 1)
typedef struct tbds_header {
    uint32_t signature;
    uint32_t total_length;
    uint16_t version;
    uint16_t descriptor_count;
    uint8_t  reserved[TBDS_HEADER_RESERVED_BYTES];
} tbds_header_t;

typedef struct tbds_tlv {
    uint16_t type;
    uint16_t flags;
    uint32_t length;
    /* uint8_t value[length]; follows inline */
} tbds_tlv_t;
#pragma pack(pop)

#define TBDS_VERSION_MAKE(major, minor) ((((uint16_t)(major)) << 8) | ((minor) & 0xFF))
#define TBDS_VERSION_1_0 TBDS_VERSION_MAKE(1, 0)

static inline void tbds_init_header(tbds_header_t *hdr)
{
    hdr->signature = TBDS_SIGNATURE;
    hdr->total_length = sizeof(tbds_header_t);
    hdr->version = TBDS_VERSION_1_0;
    hdr->descriptor_count = 0;
    for (int i = 0; i < TBDS_HEADER_RESERVED_BYTES; ++i) {
        hdr->reserved[i] = 0;
    }
}

static inline uint8_t *tbds_append_descriptor(uint8_t *cursor,
                                              uint16_t type,
                                              uint16_t flags,
                                              uint32_t length,
                                              const void *payload)
{
    tbds_tlv_t *tlv = (tbds_tlv_t *)cursor;
    tlv->type = type;
    tlv->flags = flags;
    tlv->length = length;

    cursor += sizeof(tbds_tlv_t);
    if (length && payload) {
        for (uint32_t i = 0; i < length; ++i) {
            cursor[i] = ((const uint8_t *)payload)[i];
        }
    }
    return cursor + length;
}

#endif /* TBOS_BOOT_DESCRIPTOR_H */
