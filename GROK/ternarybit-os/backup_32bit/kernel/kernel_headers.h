// TBOS Kernel Headers
// Missing type definitions

#ifndef KERNEL_HEADERS_H
#define KERNEL_HEADERS_H

#include <stdint.h>

// TBDS (TernaryBit Descriptor Stream) structures
#define TBDS_SIGNATURE 0x53444254  // "TBDS"

typedef struct {
    uint32_t signature;
    uint32_t total_length;
    uint16_t descriptor_count;
    uint16_t reserved;
} __attribute__((packed)) tbds_header_t;

typedef struct {
    uint16_t type;
    uint16_t length;
} __attribute__((packed)) tbds_tlv_t;

// TBDS type definitions
#define TBDS_TYPE_ARCH_INFO         0x0001
#define TBDS_TYPE_FIRMWARE_INFO     0x0002
#define TBDS_TYPE_MEMORY_MAP        0x0003
#define TBDS_TYPE_BOOT_DEVICE       0x0004
#define TBDS_TYPE_CONSOLE_INFO      0x0005
#define TBDS_TYPE_STEPPPS_TELEMETRY 0x0006

// Forward declarations
void keyboard_init(void);

#endif