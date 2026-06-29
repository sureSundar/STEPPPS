/**
 * @file tbos_arch.h
 * @brief TernaryBit OS - Universal Architecture Abstraction Layer
 *
 * Supports ALL processor levels from calculators to supercomputers:
 * - Tier 0: 8-bit calculators (AVR, Z80)
 * - Tier 1: 16-bit embedded (MSP430, 8086)
 * - Tier 2: 32-bit retro/embedded (ARM Cortex-M, x86)
 * - Tier 3: 64-bit desktop (x86-64, ARM64, RISC-V)
 * - Tier 4: Server/HPC (multi-socket, NUMA)
 * - Tier 5: Supercomputer (massive parallelism)
 *
 * @version 1.0 (Production)
 * @date 2025-11-03
 */

#ifndef TBOS_ARCH_H
#define TBOS_ARCH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* ARCHITECTURE DETECTION                                                    */
/* ========================================================================= */

/* Detect architecture at compile time */
#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
    #define TBOS_ARCH_X86_64    1
    #define TBOS_ARCH_NAME      "x86-64"
    #define TBOS_ARCH_BITS      64
    #define TBOS_ARCH_TIER      3
    #define TBOS_LITTLE_ENDIAN  1
#elif defined(__i386__) || defined(_M_IX86) || defined(__i686__)
    #define TBOS_ARCH_X86       1
    #define TBOS_ARCH_NAME      "x86"
    #define TBOS_ARCH_BITS      32
    #define TBOS_ARCH_TIER      2
    #define TBOS_LITTLE_ENDIAN  1
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define TBOS_ARCH_ARM64     1
    #define TBOS_ARCH_NAME      "ARM64"
    #define TBOS_ARCH_BITS      64
    #define TBOS_ARCH_TIER      3
    #define TBOS_LITTLE_ENDIAN  1
#elif defined(__arm__) || defined(_M_ARM)
    #define TBOS_ARCH_ARM       1
    #define TBOS_ARCH_NAME      "ARM"
    #define TBOS_ARCH_BITS      32
    #define TBOS_ARCH_TIER      2
    #define TBOS_LITTLE_ENDIAN  1
#elif defined(__riscv)
    #if __riscv_xlen == 64
        #define TBOS_ARCH_RISCV64   1
        #define TBOS_ARCH_NAME      "RISC-V 64"
        #define TBOS_ARCH_BITS      64
        #define TBOS_ARCH_TIER      3
    #else
        #define TBOS_ARCH_RISCV32   1
        #define TBOS_ARCH_NAME      "RISC-V 32"
        #define TBOS_ARCH_BITS      32
        #define TBOS_ARCH_TIER      2
    #endif
    #define TBOS_LITTLE_ENDIAN  1
#elif defined(__mips64)
    #define TBOS_ARCH_MIPS64    1
    #define TBOS_ARCH_NAME      "MIPS64"
    #define TBOS_ARCH_BITS      64
    #define TBOS_ARCH_TIER      3
    #define TBOS_BIG_ENDIAN     1
#elif defined(__mips__)
    #define TBOS_ARCH_MIPS      1
    #define TBOS_ARCH_NAME      "MIPS"
    #define TBOS_ARCH_BITS      32
    #define TBOS_ARCH_TIER      2
    #define TBOS_BIG_ENDIAN     1
#elif defined(__powerpc64__) || defined(__ppc64__)
    #define TBOS_ARCH_PPC64     1
    #define TBOS_ARCH_NAME      "PowerPC64"
    #define TBOS_ARCH_BITS      64
    #define TBOS_ARCH_TIER      4
    #define TBOS_BIG_ENDIAN     1
#elif defined(__AVR__)
    #define TBOS_ARCH_AVR       1
    #define TBOS_ARCH_NAME      "AVR"
    #define TBOS_ARCH_BITS      8
    #define TBOS_ARCH_TIER      0
    #define TBOS_LITTLE_ENDIAN  1
#elif defined(__MSP430__)
    #define TBOS_ARCH_MSP430    1
    #define TBOS_ARCH_NAME      "MSP430"
    #define TBOS_ARCH_BITS      16
    #define TBOS_ARCH_TIER      1
    #define TBOS_LITTLE_ENDIAN  1
#elif defined(__s390x__)
    #define TBOS_ARCH_S390X     1
    #define TBOS_ARCH_NAME      "IBM z/Architecture"
    #define TBOS_ARCH_BITS      64
    #define TBOS_ARCH_TIER      5
    #define TBOS_BIG_ENDIAN     1
#else
    #define TBOS_ARCH_UNKNOWN   1
    #define TBOS_ARCH_NAME      "Unknown"
    #define TBOS_ARCH_BITS      (sizeof(void*) * 8)
    #define TBOS_ARCH_TIER      2
    #define TBOS_LITTLE_ENDIAN  1
#endif

/* Default endianness if not set */
#if !defined(TBOS_LITTLE_ENDIAN) && !defined(TBOS_BIG_ENDIAN)
    #define TBOS_LITTLE_ENDIAN 1
#endif

/* ========================================================================= */
/* PROCESSOR TIERS                                                           */
/* ========================================================================= */

typedef enum {
    PROCESSOR_TIER_CALCULATOR = 0,    /* 8-bit: AVR, Z80, <1MB RAM */
    PROCESSOR_TIER_EMBEDDED = 1,      /* 16-bit: MSP430, 8086, 1-64MB */
    PROCESSOR_TIER_RETRO = 2,         /* 32-bit: ARM Cortex-M, x86, 64MB-1GB */
    PROCESSOR_TIER_DESKTOP = 3,       /* 64-bit: x86-64, ARM64, 1-16GB */
    PROCESSOR_TIER_SERVER = 4,        /* 64-bit multi-socket: 16-256GB */
    PROCESSOR_TIER_SUPERCOMPUTER = 5  /* HPC: >256GB, massive parallelism */
} tbos_processor_tier_t;

/* ========================================================================= */
/* ARCHITECTURE CAPABILITIES                                                 */
/* ========================================================================= */

/* CPU capability flags */
#define TBOS_CAP_FPU            (1ULL << 0)   /* Floating point unit */
#define TBOS_CAP_MMU            (1ULL << 1)   /* Memory management unit */
#define TBOS_CAP_CACHE_L1       (1ULL << 2)   /* L1 cache */
#define TBOS_CAP_CACHE_L2       (1ULL << 3)   /* L2 cache */
#define TBOS_CAP_CACHE_L3       (1ULL << 4)   /* L3 cache */
#define TBOS_CAP_SIMD           (1ULL << 5)   /* SIMD instructions */
#define TBOS_CAP_SSE            (1ULL << 6)   /* SSE (x86) */
#define TBOS_CAP_SSE2           (1ULL << 7)   /* SSE2 */
#define TBOS_CAP_SSE3           (1ULL << 8)   /* SSE3 */
#define TBOS_CAP_SSSE3          (1ULL << 9)   /* SSSE3 */
#define TBOS_CAP_SSE41          (1ULL << 10)  /* SSE4.1 */
#define TBOS_CAP_SSE42          (1ULL << 11)  /* SSE4.2 */
#define TBOS_CAP_AVX            (1ULL << 12)  /* AVX */
#define TBOS_CAP_AVX2           (1ULL << 13)  /* AVX2 */
#define TBOS_CAP_AVX512         (1ULL << 14)  /* AVX-512 */
#define TBOS_CAP_NEON           (1ULL << 15)  /* ARM NEON */
#define TBOS_CAP_SVE            (1ULL << 16)  /* ARM SVE */
#define TBOS_CAP_NUMA           (1ULL << 17)  /* NUMA support */
#define TBOS_CAP_VIRTUALIZATION (1ULL << 18)  /* VT-x/AMD-V/ARM-VE */
#define TBOS_CAP_AES            (1ULL << 19)  /* AES acceleration */
#define TBOS_CAP_SHA            (1ULL << 20)  /* SHA acceleration */
#define TBOS_CAP_CRC32          (1ULL << 21)  /* CRC32 acceleration */
#define TBOS_CAP_ATOMIC64       (1ULL << 22)  /* 64-bit atomics */
#define TBOS_CAP_TSC            (1ULL << 23)  /* Timestamp counter */
#define TBOS_CAP_RDRAND         (1ULL << 24)  /* Hardware RNG */
#define TBOS_CAP_HYPERTHREADING (1ULL << 25)  /* SMT/Hyperthreading */

/* ========================================================================= */
/* ARCHITECTURE INFO STRUCTURE                                               */
/* ========================================================================= */

typedef struct {
    /* Identification */
    const char *arch_name;
    uint32_t arch_bits;
    tbos_processor_tier_t tier;
    bool is_little_endian;

    /* CPU info */
    uint32_t cpu_count;
    uint32_t cores_per_cpu;
    uint32_t threads_per_core;
    uint32_t total_threads;
    uint64_t cpu_freq_hz;
    char cpu_vendor[64];
    char cpu_model[128];

    /* Capabilities */
    uint64_t capabilities;

    /* Cache info */
    uint32_t l1_cache_kb;
    uint32_t l2_cache_kb;
    uint32_t l3_cache_kb;
    uint32_t cache_line_size;

    /* Memory info */
    uint64_t total_memory;
    uint64_t available_memory;
    uint32_t page_size;
    uint32_t numa_nodes;

    /* Platform */
    bool is_virtual;
    bool is_container;
    const char *hypervisor;
} tbos_arch_info_t;

/* ========================================================================= */
/* ENDIANNESS MACROS                                                         */
/* ========================================================================= */

#ifdef TBOS_LITTLE_ENDIAN
    #define TBOS_SWAP16(x) ((uint16_t)(((x) >> 8) | ((x) << 8)))
    #define TBOS_SWAP32(x) ((uint32_t)(((x) >> 24) | (((x) >> 8) & 0xFF00) | \
                           (((x) << 8) & 0xFF0000) | ((x) << 24)))
    #define TBOS_SWAP64(x) ((uint64_t)(((x) >> 56) | (((x) >> 40) & 0xFF00ULL) | \
                           (((x) >> 24) & 0xFF0000ULL) | (((x) >> 8) & 0xFF000000ULL) | \
                           (((x) << 8) & 0xFF00000000ULL) | (((x) << 24) & 0xFF0000000000ULL) | \
                           (((x) << 40) & 0xFF000000000000ULL) | ((x) << 56)))

    /* Convert to/from big endian (network byte order) */
    #define TBOS_TO_BE16(x)   TBOS_SWAP16(x)
    #define TBOS_TO_BE32(x)   TBOS_SWAP32(x)
    #define TBOS_TO_BE64(x)   TBOS_SWAP64(x)
    #define TBOS_FROM_BE16(x) TBOS_SWAP16(x)
    #define TBOS_FROM_BE32(x) TBOS_SWAP32(x)
    #define TBOS_FROM_BE64(x) TBOS_SWAP64(x)

    /* Little endian is native */
    #define TBOS_TO_LE16(x)   (x)
    #define TBOS_TO_LE32(x)   (x)
    #define TBOS_TO_LE64(x)   (x)
    #define TBOS_FROM_LE16(x) (x)
    #define TBOS_FROM_LE32(x) (x)
    #define TBOS_FROM_LE64(x) (x)
#else
    /* Big endian */
    #define TBOS_TO_BE16(x)   (x)
    #define TBOS_TO_BE32(x)   (x)
    #define TBOS_TO_BE64(x)   (x)
    #define TBOS_FROM_BE16(x) (x)
    #define TBOS_FROM_BE32(x) (x)
    #define TBOS_FROM_BE64(x) (x)

    #define TBOS_TO_LE16(x)   TBOS_SWAP16(x)
    #define TBOS_TO_LE32(x)   TBOS_SWAP32(x)
    #define TBOS_TO_LE64(x)   TBOS_SWAP64(x)
    #define TBOS_FROM_LE16(x) TBOS_SWAP16(x)
    #define TBOS_FROM_LE32(x) TBOS_SWAP32(x)
    #define TBOS_FROM_LE64(x) TBOS_SWAP64(x)
#endif

/* ========================================================================= */
/* TIER-SPECIFIC TYPE DEFINITIONS                                            */
/* ========================================================================= */

/* Native-sized types for each tier */
#if TBOS_ARCH_BITS == 8
    typedef uint8_t  tbos_native_t;
    typedef int8_t   tbos_native_signed_t;
    typedef uint8_t  tbos_ptr_t;
    #define TBOS_NATIVE_MAX  UINT8_MAX
#elif TBOS_ARCH_BITS == 16
    typedef uint16_t tbos_native_t;
    typedef int16_t  tbos_native_signed_t;
    typedef uint16_t tbos_ptr_t;
    #define TBOS_NATIVE_MAX  UINT16_MAX
#elif TBOS_ARCH_BITS == 32
    typedef uint32_t tbos_native_t;
    typedef int32_t  tbos_native_signed_t;
    typedef uint32_t tbos_ptr_t;
    #define TBOS_NATIVE_MAX  UINT32_MAX
#else /* 64-bit */
    typedef uint64_t tbos_native_t;
    typedef int64_t  tbos_native_signed_t;
    typedef uint64_t tbos_ptr_t;
    #define TBOS_NATIVE_MAX  UINT64_MAX
#endif

/* ========================================================================= */
/* MEMORY BARRIERS & ATOMICS                                                 */
/* ========================================================================= */

#if defined(__GNUC__) || defined(__clang__)
    #define TBOS_MEMORY_BARRIER()       __sync_synchronize()
    #define TBOS_COMPILER_BARRIER()     __asm__ __volatile__("" ::: "memory")
    #define TBOS_ATOMIC_LOAD(ptr)       __atomic_load_n(ptr, __ATOMIC_SEQ_CST)
    #define TBOS_ATOMIC_STORE(ptr, val) __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST)
    #define TBOS_ATOMIC_ADD(ptr, val)   __atomic_add_fetch(ptr, val, __ATOMIC_SEQ_CST)
    #define TBOS_ATOMIC_SUB(ptr, val)   __atomic_sub_fetch(ptr, val, __ATOMIC_SEQ_CST)
    #define TBOS_ATOMIC_CAS(ptr, expected, desired) \
        __atomic_compare_exchange_n(ptr, expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)
#elif TBOS_ARCH_BITS <= 16
    /* No atomics on 8/16-bit - use interrupt disable */
    #define TBOS_MEMORY_BARRIER()       ((void)0)
    #define TBOS_COMPILER_BARRIER()     ((void)0)
    #define TBOS_ATOMIC_LOAD(ptr)       (*(ptr))
    #define TBOS_ATOMIC_STORE(ptr, val) (*(ptr) = (val))
    #define TBOS_ATOMIC_ADD(ptr, val)   (*(ptr) += (val))
    #define TBOS_ATOMIC_SUB(ptr, val)   (*(ptr) -= (val))
    #define TBOS_ATOMIC_CAS(ptr, expected, desired) \
        ((*(ptr) == *(expected)) ? (*(ptr) = (desired), true) : false)
#else
    #define TBOS_MEMORY_BARRIER()       ((void)0)
    #define TBOS_COMPILER_BARRIER()     ((void)0)
    #define TBOS_ATOMIC_LOAD(ptr)       (*(volatile typeof(*(ptr))*)(ptr))
    #define TBOS_ATOMIC_STORE(ptr, val) (*(volatile typeof(*(ptr))*)(ptr) = (val))
    #define TBOS_ATOMIC_ADD(ptr, val)   (*(ptr) += (val))
    #define TBOS_ATOMIC_SUB(ptr, val)   (*(ptr) -= (val))
    #define TBOS_ATOMIC_CAS(ptr, expected, desired) \
        ((*(ptr) == *(expected)) ? (*(ptr) = (desired), true) : false)
#endif

/* ========================================================================= */
/* ARCHITECTURE-SPECIFIC INLINE FUNCTIONS                                    */
/* ========================================================================= */

/* CPU pause hint for spinlocks */
static inline void tbos_cpu_pause(void) {
#if defined(TBOS_ARCH_X86_64) || defined(TBOS_ARCH_X86)
    __asm__ __volatile__("pause" ::: "memory");
#elif defined(TBOS_ARCH_ARM64)
    __asm__ __volatile__("yield" ::: "memory");
#elif defined(TBOS_ARCH_ARM)
    __asm__ __volatile__("nop" ::: "memory");
#elif defined(TBOS_ARCH_RISCV64) || defined(TBOS_ARCH_RISCV32)
    __asm__ __volatile__("nop" ::: "memory");
#else
    /* No-op for unknown architectures */
    TBOS_COMPILER_BARRIER();
#endif
}

/* Read timestamp counter */
static inline uint64_t tbos_rdtsc(void) {
#if defined(TBOS_ARCH_X86_64) || defined(TBOS_ARCH_X86)
    uint32_t lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#elif defined(TBOS_ARCH_ARM64)
    uint64_t val;
    __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(val));
    return val;
#elif defined(TBOS_ARCH_ARM)
    /* ARM32 doesn't have a reliable cycle counter */
    return 0;
#elif defined(TBOS_ARCH_RISCV64) || defined(TBOS_ARCH_RISCV32)
    uint64_t val;
    __asm__ __volatile__("rdcycle %0" : "=r"(val));
    return val;
#else
    return 0;
#endif
}

/* Prefetch data into cache */
static inline void tbos_prefetch(const void *addr) {
#if defined(__GNUC__) || defined(__clang__)
    __builtin_prefetch(addr, 0, 3);
#else
    (void)addr;
#endif
}

/* Count leading zeros */
static inline int tbos_clz(uint32_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return x ? __builtin_clz(x) : 32;
#else
    int n = 0;
    if (x == 0) return 32;
    if ((x & 0xFFFF0000) == 0) { n += 16; x <<= 16; }
    if ((x & 0xFF000000) == 0) { n += 8;  x <<= 8;  }
    if ((x & 0xF0000000) == 0) { n += 4;  x <<= 4;  }
    if ((x & 0xC0000000) == 0) { n += 2;  x <<= 2;  }
    if ((x & 0x80000000) == 0) { n += 1; }
    return n;
#endif
}

/* Count trailing zeros */
static inline int tbos_ctz(uint32_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return x ? __builtin_ctz(x) : 32;
#else
    if (x == 0) return 32;
    int n = 0;
    if ((x & 0x0000FFFF) == 0) { n += 16; x >>= 16; }
    if ((x & 0x000000FF) == 0) { n += 8;  x >>= 8;  }
    if ((x & 0x0000000F) == 0) { n += 4;  x >>= 4;  }
    if ((x & 0x00000003) == 0) { n += 2;  x >>= 2;  }
    if ((x & 0x00000001) == 0) { n += 1; }
    return n;
#endif
}

/* Population count (count set bits) */
static inline int tbos_popcount(uint32_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcount(x);
#else
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    return (((x + (x >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
#endif
}

/* ========================================================================= */
/* API FUNCTIONS                                                             */
/* ========================================================================= */

/**
 * @brief Initialize architecture detection
 * @return 0 on success
 */
int tbos_arch_init(void);

/**
 * @brief Get architecture information
 * @return Pointer to arch info structure
 */
const tbos_arch_info_t* tbos_arch_get_info(void);

/**
 * @brief Get current processor tier
 * @return Processor tier (0-5)
 */
tbos_processor_tier_t tbos_arch_get_tier(void);

/**
 * @brief Check if capability is available
 * @param cap Capability flag
 * @return true if available
 */
bool tbos_arch_has_capability(uint64_t cap);

/**
 * @brief Get number of CPUs/cores
 * @return Total logical processors
 */
uint32_t tbos_arch_get_cpu_count(void);

/**
 * @brief Get total memory in bytes
 * @return Total memory
 */
uint64_t tbos_arch_get_total_memory(void);

/**
 * @brief Get page size
 * @return Page size in bytes
 */
uint32_t tbos_arch_get_page_size(void);

/**
 * @brief Get optimal memory alignment for this architecture
 * @return Alignment in bytes
 */
uint32_t tbos_arch_get_alignment(void);

/**
 * @brief Check if running in virtual machine
 * @return true if virtualized
 */
bool tbos_arch_is_virtual(void);

/**
 * @brief Print architecture summary
 */
void tbos_arch_print_info(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_ARCH_H */
