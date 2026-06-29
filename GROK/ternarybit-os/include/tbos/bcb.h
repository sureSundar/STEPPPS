/**
 * @file bcb.h
 * @brief Boot Capability Block (BCB) - V4.0 Universal Boot Protocol
 *
 * The BCB provides a fixed-size structure for boot stage handoff,
 * enabling universal boot across bare-metal, hosted, and virtualized
 * environments.
 *
 * Design Goals:
 * - Fixed size for predictable memory layout
 * - Stage flags for HAL driver selection
 * - Host API pointer for hosted/virtualized environments
 * - Backwards compatible with TBDS (can coexist)
 *
 * @version 1.0
 * @date 2025
 * @author TernaryBit Development Team
 *
 * Traceability: v4.0/ROADMAP.md V4-001
 */

#ifndef TBOS_BCB_H
#define TBOS_BCB_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* BCB CONSTANTS                                                              */
/* ========================================================================= */

/** BCB Magic: "TBBC" in little-endian */
#define BCB_MAGIC           0x43424254  /* "TBBC" */
#define BCB_MAGIC_STR       "TBBC"

/** BCB Version */
#define BCB_VERSION_1_0     0x0001
#define BCB_VERSION_CURRENT BCB_VERSION_1_0

/** BCB Memory Location (low memory, after Stage 1 stack) */
#define BCB_PHYSICAL_ADDR   0x6000

/** BCB Total Size (fixed at 128 bytes for alignment) */
#define BCB_SIZE            128

/* ========================================================================= */
/* STAGE FLAGS                                                                */
/* ========================================================================= */

/**
 * @brief Stage flags indicate boot environment capabilities
 *
 * These flags help the kernel select appropriate HAL drivers
 * based on the boot environment.
 */
typedef enum {
    BCB_FLAG_BIOS           = (1 << 0),   /**< Booted via BIOS */
    BCB_FLAG_UEFI           = (1 << 1),   /**< Booted via UEFI */
    BCB_FLAG_HOSTED         = (1 << 2),   /**< Running as hosted app (Linux/Windows) */
    BCB_FLAG_VIRTUALIZED    = (1 << 3),   /**< Running in VM (QEMU, VirtualBox) */
    BCB_FLAG_SECURE_BOOT    = (1 << 4),   /**< Secure boot chain verified */
    BCB_FLAG_TPM_PRESENT    = (1 << 5),   /**< TPM available */
    BCB_FLAG_ACPI_PRESENT   = (1 << 6),   /**< ACPI tables available */
    BCB_FLAG_EFI_RUNTIME    = (1 << 7),   /**< EFI runtime services available */
    BCB_FLAG_SERIAL_CONSOLE = (1 << 8),   /**< Serial console configured */
    BCB_FLAG_VGA_CONSOLE    = (1 << 9),   /**< VGA text mode available */
    BCB_FLAG_FRAMEBUFFER    = (1 << 10),  /**< Framebuffer available */
    BCB_FLAG_NETWORK_BOOT   = (1 << 11),  /**< Booted via network (PXE/HTTP) */
    BCB_FLAG_WASM           = (1 << 12),  /**< Running as WASM module */
    BCB_FLAG_64BIT          = (1 << 13),  /**< 64-bit mode active */
    BCB_FLAG_SMP            = (1 << 14),  /**< Multiple CPUs detected */
    BCB_FLAG_DEBUG          = (1 << 15),  /**< Debug mode enabled */
} bcb_stage_flags_t;

/* ========================================================================= */
/* HOST API TABLE                                                             */
/* ========================================================================= */

/**
 * @brief Host API function table for hosted environments
 *
 * When running as a hosted application (Linux/Windows), the host stub
 * provides system call shims through this table.
 */
typedef struct tbos_host_api {
    uint32_t version;           /**< API version (0x0001) */
    uint32_t flags;             /**< API capability flags */

    /* Console I/O */
    int (*write_console)(const char* buf, uint32_t len);
    int (*read_console)(char* buf, uint32_t max_len);

    /* Memory management */
    void* (*allocate)(uint32_t size);
    void (*deallocate)(void* ptr);

    /* File I/O (for hosted filesystem access) */
    int (*file_open)(const char* path, int flags);
    int (*file_close)(int fd);
    int (*file_read)(int fd, void* buf, uint32_t len);
    int (*file_write)(int fd, const void* buf, uint32_t len);

    /* Time */
    uint64_t (*get_time_ms)(void);
    void (*sleep_ms)(uint32_t ms);

    /* Exit */
    void (*exit)(int status);

    /* Reserved for future use */
    void* reserved[8];
} tbos_host_api_t;

/* ========================================================================= */
/* BOOT CAPABILITY BLOCK STRUCTURE                                            */
/* ========================================================================= */

/**
 * @brief Boot Capability Block (BCB) v1.0
 *
 * Fixed-size structure providing boot environment information.
 * Total size: 128 bytes (aligned for cache efficiency)
 *
 * Memory layout:
 *   [0x6000] BCB Header (128 bytes)
 *   [0x6080] Optional extension data
 */
typedef struct __attribute__((packed)) tbos_bcb_v1 {
    /* Header (8 bytes) */
    uint32_t magic;             /**< "TBBC" (0x43424254) */
    uint16_t version;           /**< BCB version (0x0001) */
    uint16_t length;            /**< Bytes following header (120) */

    /* Boot Environment (8 bytes) */
    uint32_t stage_flags;       /**< Bitmask of bcb_stage_flags_t */
    uint32_t boot_stage;        /**< Which stage populated this (1, 2, or 0 for hosted) */

    /* Boot Device (16 bytes) */
    uint64_t boot_drive_id;     /**< BIOS DL value, UEFI handle, or host descriptor */
    uint32_t boot_partition;    /**< Partition number (0 = whole disk) */
    uint32_t boot_fs_type;      /**< Filesystem type hint */

    /* Memory Map Summary (24 bytes) */
    uint64_t mem_low_bytes;     /**< Conventional memory below 1MB */
    uint64_t mem_high_bytes;    /**< Extended memory above 1MB */
    uint64_t mem_total_bytes;   /**< Total usable memory */

    /* Kernel Information (16 bytes) */
    uint64_t kernel_entry;      /**< Kernel entry point (physical addr) */
    uint32_t kernel_size;       /**< Kernel image size in bytes */
    uint32_t kernel_flags;      /**< Kernel-specific flags */

    /* Host API (for hosted environments) (16 bytes) */
    uint64_t host_api_ptr;      /**< Pointer to tbos_host_api_t (or 0) */
    uint64_t host_context;      /**< Host-specific context pointer */

    /* Checksums & Reserved (40 bytes) */
    uint32_t bcb_checksum;      /**< CRC32 of BCB (excluding this field) */
    uint32_t tbds_pointer;      /**< Pointer to TBDS if available (backwards compat) */
    uint32_t tbds_length;       /**< TBDS length */
    uint8_t  reserved[28];      /**< Future use (network cfg, etc.) */
} tbos_bcb_v1_t;

/* Verify structure size at compile time */
_Static_assert(sizeof(tbos_bcb_v1_t) == BCB_SIZE,
               "BCB size must be exactly 128 bytes");

/* ========================================================================= */
/* BCB ACCESS MACROS                                                          */
/* ========================================================================= */

/** Get pointer to BCB at default physical address */
#define BCB_GET_DEFAULT()   ((tbos_bcb_v1_t*)(uintptr_t)BCB_PHYSICAL_ADDR)

/** Check if BCB is valid */
#define BCB_IS_VALID(bcb)   ((bcb) && (bcb)->magic == BCB_MAGIC)

/** Check specific stage flag */
#define BCB_HAS_FLAG(bcb, flag)   (((bcb)->stage_flags & (flag)) != 0)

/** Check if hosted environment */
#define BCB_IS_HOSTED(bcb)  BCB_HAS_FLAG(bcb, BCB_FLAG_HOSTED)

/** Check if bare-metal BIOS */
#define BCB_IS_BIOS(bcb)    BCB_HAS_FLAG(bcb, BCB_FLAG_BIOS)

/** Check if UEFI */
#define BCB_IS_UEFI(bcb)    BCB_HAS_FLAG(bcb, BCB_FLAG_UEFI)

/** Get host API if available */
#define BCB_GET_HOST_API(bcb) \
    (BCB_IS_HOSTED(bcb) ? (tbos_host_api_t*)(uintptr_t)(bcb)->host_api_ptr : NULL)

/* ========================================================================= */
/* BCB HELPER FUNCTIONS                                                       */
/* ========================================================================= */

/**
 * @brief Initialize a BCB structure with defaults
 *
 * @param bcb Pointer to BCB structure to initialize
 */
static inline void bcb_init(tbos_bcb_v1_t* bcb) {
    if (!bcb) return;

    /* Clear entire structure */
    uint8_t* ptr = (uint8_t*)bcb;
    for (int i = 0; i < BCB_SIZE; i++) {
        ptr[i] = 0;
    }

    /* Set header */
    bcb->magic = BCB_MAGIC;
    bcb->version = BCB_VERSION_CURRENT;
    bcb->length = BCB_SIZE - 8;  /* Exclude magic and version/length fields */
}

/**
 * @brief Validate BCB structure
 *
 * @param bcb Pointer to BCB structure
 * @return true if BCB is valid
 */
static inline bool bcb_validate(const tbos_bcb_v1_t* bcb) {
    if (!bcb) return false;
    if (bcb->magic != BCB_MAGIC) return false;
    if (bcb->version == 0) return false;
    if (bcb->length != BCB_SIZE - 8) return false;
    return true;
}

/**
 * @brief Get memory size string for display
 *
 * @param bytes Memory size in bytes
 * @param buf Output buffer (at least 16 chars)
 */
static inline void bcb_format_memory(uint64_t bytes, char* buf) {
    if (bytes >= (1ULL << 30)) {
        /* GB */
        uint32_t gb = (uint32_t)(bytes >> 30);
        buf[0] = '0' + (gb / 10);
        buf[1] = '0' + (gb % 10);
        buf[2] = ' ';
        buf[3] = 'G';
        buf[4] = 'B';
        buf[5] = '\0';
    } else if (bytes >= (1ULL << 20)) {
        /* MB */
        uint32_t mb = (uint32_t)(bytes >> 20);
        buf[0] = '0' + (mb / 100) % 10;
        buf[1] = '0' + (mb / 10) % 10;
        buf[2] = '0' + (mb % 10);
        buf[3] = ' ';
        buf[4] = 'M';
        buf[5] = 'B';
        buf[6] = '\0';
    } else {
        /* KB */
        uint32_t kb = (uint32_t)(bytes >> 10);
        buf[0] = '0' + (kb / 100) % 10;
        buf[1] = '0' + (kb / 10) % 10;
        buf[2] = '0' + (kb % 10);
        buf[3] = ' ';
        buf[4] = 'K';
        buf[5] = 'B';
        buf[6] = '\0';
    }
}

/**
 * @brief Simple CRC32 for BCB checksum
 *
 * @param data Data to checksum
 * @param len Length in bytes
 * @return CRC32 value
 */
static inline uint32_t bcb_crc32(const uint8_t* data, uint32_t len) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
        }
    }
    return ~crc;
}

/**
 * @brief Compute and set BCB checksum
 *
 * @param bcb Pointer to BCB structure
 */
static inline void bcb_compute_checksum(tbos_bcb_v1_t* bcb) {
    if (!bcb) return;
    /* Compute CRC32 excluding the checksum field itself */
    uint32_t offset = (uint32_t)((uint8_t*)&bcb->bcb_checksum - (uint8_t*)bcb);
    bcb->bcb_checksum = bcb_crc32((const uint8_t*)bcb, offset);
}

/**
 * @brief Verify BCB checksum
 *
 * @param bcb Pointer to BCB structure
 * @return true if checksum matches
 */
static inline bool bcb_verify_checksum(const tbos_bcb_v1_t* bcb) {
    if (!bcb) return false;
    uint32_t offset = (uint32_t)((uint8_t*)&bcb->bcb_checksum - (uint8_t*)bcb);
    uint32_t computed = bcb_crc32((const uint8_t*)bcb, offset);
    return computed == bcb->bcb_checksum;
}

/* ========================================================================= */
/* HAL SELECTION HELPERS                                                      */
/* ========================================================================= */

/**
 * @brief HAL type based on BCB flags
 */
typedef enum {
    BCB_HAL_BARE_METAL_BIOS,    /**< Direct hardware, BIOS boot */
    BCB_HAL_BARE_METAL_UEFI,    /**< Direct hardware, UEFI boot */
    BCB_HAL_HOSTED_LINUX,       /**< Linux userspace host */
    BCB_HAL_HOSTED_WINDOWS,     /**< Windows userspace host */
    BCB_HAL_VIRTUALIZED,        /**< VM with paravirt drivers */
    BCB_HAL_WASM,               /**< WebAssembly runtime */
    BCB_HAL_UNKNOWN             /**< Unknown/fallback */
} bcb_hal_type_t;

/**
 * @brief Determine HAL type from BCB flags
 *
 * @param bcb Pointer to BCB structure
 * @return HAL type to use
 */
static inline bcb_hal_type_t bcb_get_hal_type(const tbos_bcb_v1_t* bcb) {
    if (!bcb || !BCB_IS_VALID(bcb)) {
        return BCB_HAL_UNKNOWN;
    }

    if (BCB_HAS_FLAG(bcb, BCB_FLAG_WASM)) {
        return BCB_HAL_WASM;
    }
    if (BCB_HAS_FLAG(bcb, BCB_FLAG_HOSTED)) {
        /* Check host context for specific platform */
        return BCB_HAL_HOSTED_LINUX;  /* Default to Linux */
    }
    if (BCB_HAS_FLAG(bcb, BCB_FLAG_VIRTUALIZED)) {
        return BCB_HAL_VIRTUALIZED;
    }
    if (BCB_HAS_FLAG(bcb, BCB_FLAG_UEFI)) {
        return BCB_HAL_BARE_METAL_UEFI;
    }
    if (BCB_HAS_FLAG(bcb, BCB_FLAG_BIOS)) {
        return BCB_HAL_BARE_METAL_BIOS;
    }

    return BCB_HAL_UNKNOWN;
}

/**
 * @brief Get HAL type name string
 *
 * @param type HAL type
 * @return Human-readable string
 */
static inline const char* bcb_hal_type_name(bcb_hal_type_t type) {
    switch (type) {
        case BCB_HAL_BARE_METAL_BIOS: return "Bare-Metal (BIOS)";
        case BCB_HAL_BARE_METAL_UEFI: return "Bare-Metal (UEFI)";
        case BCB_HAL_HOSTED_LINUX:    return "Hosted (Linux)";
        case BCB_HAL_HOSTED_WINDOWS:  return "Hosted (Windows)";
        case BCB_HAL_VIRTUALIZED:     return "Virtualized";
        case BCB_HAL_WASM:            return "WebAssembly";
        default:                      return "Unknown";
    }
}

#ifdef __cplusplus
}
#endif

#endif /* TBOS_BCB_H */
