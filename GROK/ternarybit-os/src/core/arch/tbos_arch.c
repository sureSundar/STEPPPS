/**
 * @file tbos_arch.c
 * @brief TernaryBit OS - Universal Architecture Implementation
 *
 * Runtime detection and configuration for all processor tiers.
 *
 * @version 1.0 (Production)
 * @date 2025-11-03
 */

#include "tbos_arch.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

#ifdef __APPLE__
#include <sys/sysctl.h>
#include <mach/mach.h>
#endif

/* ========================================================================= */
/* GLOBAL STATE                                                              */
/* ========================================================================= */

static tbos_arch_info_t g_arch_info;
static bool g_arch_initialized = false;

/* ========================================================================= */
/* CPUID SUPPORT (x86/x86-64)                                                */
/* ========================================================================= */

#if defined(TBOS_ARCH_X86_64) || defined(TBOS_ARCH_X86)

static void cpuid(uint32_t leaf, uint32_t subleaf,
                  uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf), "c"(subleaf)
    );
}

static void detect_x86_capabilities(void) {
    uint32_t eax, ebx, ecx, edx;

    /* Get vendor string */
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    memcpy(g_arch_info.cpu_vendor, &ebx, 4);
    memcpy(g_arch_info.cpu_vendor + 4, &edx, 4);
    memcpy(g_arch_info.cpu_vendor + 8, &ecx, 4);
    g_arch_info.cpu_vendor[12] = '\0';

    /* Get brand string */
    uint32_t max_extended;
    cpuid(0x80000000, 0, &max_extended, &ebx, &ecx, &edx);

    if (max_extended >= 0x80000004) {
        for (int i = 0; i < 3; i++) {
            cpuid(0x80000002 + i, 0, &eax, &ebx, &ecx, &edx);
            memcpy(g_arch_info.cpu_model + i * 16, &eax, 4);
            memcpy(g_arch_info.cpu_model + i * 16 + 4, &ebx, 4);
            memcpy(g_arch_info.cpu_model + i * 16 + 8, &ecx, 4);
            memcpy(g_arch_info.cpu_model + i * 16 + 12, &edx, 4);
        }
        g_arch_info.cpu_model[48] = '\0';
    }

    /* Feature detection (leaf 1) */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);

    /* EDX features */
    if (edx & (1 << 0))  g_arch_info.capabilities |= TBOS_CAP_FPU;
    if (edx & (1 << 4))  g_arch_info.capabilities |= TBOS_CAP_TSC;
    if (edx & (1 << 25)) g_arch_info.capabilities |= TBOS_CAP_SSE;
    if (edx & (1 << 26)) g_arch_info.capabilities |= TBOS_CAP_SSE2;
    if (edx & (1 << 28)) g_arch_info.capabilities |= TBOS_CAP_HYPERTHREADING;

    /* ECX features */
    if (ecx & (1 << 0))  g_arch_info.capabilities |= TBOS_CAP_SSE3;
    if (ecx & (1 << 9))  g_arch_info.capabilities |= TBOS_CAP_SSSE3;
    if (ecx & (1 << 19)) g_arch_info.capabilities |= TBOS_CAP_SSE41;
    if (ecx & (1 << 20)) g_arch_info.capabilities |= TBOS_CAP_SSE42;
    if (ecx & (1 << 25)) g_arch_info.capabilities |= TBOS_CAP_AES;
    if (ecx & (1 << 28)) g_arch_info.capabilities |= TBOS_CAP_AVX;
    if (ecx & (1 << 30)) g_arch_info.capabilities |= TBOS_CAP_RDRAND;

    /* Extended features (leaf 7) */
    cpuid(7, 0, &eax, &ebx, &ecx, &edx);
    if (ebx & (1 << 5))  g_arch_info.capabilities |= TBOS_CAP_AVX2;
    if (ebx & (1 << 16)) g_arch_info.capabilities |= TBOS_CAP_AVX512;
    if (ebx & (1 << 29)) g_arch_info.capabilities |= TBOS_CAP_SHA;

    /* Check for hypervisor */
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    if (ecx & (1 << 31)) {
        g_arch_info.is_virtual = true;

        /* Get hypervisor vendor */
        cpuid(0x40000000, 0, &eax, &ebx, &ecx, &edx);
        char hypervisor[13];
        memcpy(hypervisor, &ebx, 4);
        memcpy(hypervisor + 4, &ecx, 4);
        memcpy(hypervisor + 8, &edx, 4);
        hypervisor[12] = '\0';

        if (strstr(hypervisor, "KVMKVMKVM")) {
            g_arch_info.hypervisor = "KVM";
        } else if (strstr(hypervisor, "VMwareVMware")) {
            g_arch_info.hypervisor = "VMware";
        } else if (strstr(hypervisor, "VBoxVBoxVBox")) {
            g_arch_info.hypervisor = "VirtualBox";
        } else if (strstr(hypervisor, "Microsoft Hv")) {
            g_arch_info.hypervisor = "Hyper-V";
        } else if (strstr(hypervisor, "XenVMMXenVMM")) {
            g_arch_info.hypervisor = "Xen";
        } else {
            g_arch_info.hypervisor = "Unknown";
        }
    }

    /* Cache info */
    cpuid(0x80000006, 0, &eax, &ebx, &ecx, &edx);
    g_arch_info.l2_cache_kb = (ecx >> 16) & 0xFFFF;
    g_arch_info.cache_line_size = ecx & 0xFF;

    /* Set capabilities based on cache */
    g_arch_info.capabilities |= TBOS_CAP_CACHE_L1;
    if (g_arch_info.l2_cache_kb > 0) {
        g_arch_info.capabilities |= TBOS_CAP_CACHE_L2;
    }
}

#endif /* x86/x86-64 */

/* ========================================================================= */
/* ARM64 DETECTION                                                           */
/* ========================================================================= */

#if defined(TBOS_ARCH_ARM64)

static void detect_arm64_capabilities(void) {
    strcpy(g_arch_info.cpu_vendor, "ARM");

    /* Read MIDR_EL1 for CPU identification */
    uint64_t midr;
    __asm__ __volatile__("mrs %0, midr_el1" : "=r"(midr));

    uint32_t implementer = (midr >> 24) & 0xFF;
    uint32_t variant = (midr >> 20) & 0xF;
    uint32_t part = (midr >> 4) & 0xFFF;
    uint32_t revision = midr & 0xF;

    snprintf(g_arch_info.cpu_model, sizeof(g_arch_info.cpu_model),
             "ARM Impl:0x%02X Part:0x%03X r%up%u",
             implementer, part, variant, revision);

    /* Read feature registers */
    uint64_t isar0;
    __asm__ __volatile__("mrs %0, id_aa64isar0_el1" : "=r"(isar0));

    /* Check for AES */
    if (((isar0 >> 4) & 0xF) >= 1) {
        g_arch_info.capabilities |= TBOS_CAP_AES;
    }

    /* Check for SHA */
    if (((isar0 >> 12) & 0xF) >= 1) {
        g_arch_info.capabilities |= TBOS_CAP_SHA;
    }

    /* Check for CRC32 */
    if (((isar0 >> 16) & 0xF) >= 1) {
        g_arch_info.capabilities |= TBOS_CAP_CRC32;
    }

    /* Read PFR0 for SIMD/FP */
    uint64_t pfr0;
    __asm__ __volatile__("mrs %0, id_aa64pfr0_el1" : "=r"(pfr0));

    /* Advanced SIMD (NEON) */
    if (((pfr0 >> 20) & 0xF) != 0xF) {
        g_arch_info.capabilities |= TBOS_CAP_NEON;
        g_arch_info.capabilities |= TBOS_CAP_SIMD;
    }

    /* Floating point */
    if (((pfr0 >> 16) & 0xF) != 0xF) {
        g_arch_info.capabilities |= TBOS_CAP_FPU;
    }

    /* SVE detection */
    uint64_t sve_check;
    __asm__ __volatile__("mrs %0, id_aa64pfr0_el1" : "=r"(sve_check));
    if (((sve_check >> 32) & 0xF) >= 1) {
        g_arch_info.capabilities |= TBOS_CAP_SVE;
    }

    /* ARM64 always has these */
    g_arch_info.capabilities |= TBOS_CAP_MMU;
    g_arch_info.capabilities |= TBOS_CAP_ATOMIC64;
    g_arch_info.capabilities |= TBOS_CAP_CACHE_L1;
    g_arch_info.capabilities |= TBOS_CAP_CACHE_L2;

    g_arch_info.cache_line_size = 64;  /* Typical for ARM64 */
}

#endif /* ARM64 */

/* ========================================================================= */
/* RISC-V DETECTION                                                          */
/* ========================================================================= */

#if defined(TBOS_ARCH_RISCV64) || defined(TBOS_ARCH_RISCV32)

static void detect_riscv_capabilities(void) {
    strcpy(g_arch_info.cpu_vendor, "RISC-V");

    /* Read misa for extensions */
    /* Note: This may require M-mode or be emulated */
#ifdef __riscv_m
    uint64_t misa;
    __asm__ __volatile__("csrr %0, misa" : "=r"(misa));

    /* Check extensions */
    if (misa & (1 << ('F' - 'A'))) {
        g_arch_info.capabilities |= TBOS_CAP_FPU;
    }
    if (misa & (1 << ('D' - 'A'))) {
        g_arch_info.capabilities |= TBOS_CAP_FPU;  /* Double precision */
    }
    if (misa & (1 << ('V' - 'A'))) {
        g_arch_info.capabilities |= TBOS_CAP_SIMD;  /* Vector extension */
    }
    if (misa & (1 << ('A' - 'A'))) {
        g_arch_info.capabilities |= TBOS_CAP_ATOMIC64;  /* Atomics */
    }

    snprintf(g_arch_info.cpu_model, sizeof(g_arch_info.cpu_model),
             "RISC-V (MISA: 0x%lx)", (unsigned long)misa);
#else
    strcpy(g_arch_info.cpu_model, "RISC-V (Extensions unknown)");

    /* Assume common extensions on modern RISC-V */
    g_arch_info.capabilities |= TBOS_CAP_FPU;
    g_arch_info.capabilities |= TBOS_CAP_ATOMIC64;
#endif

    g_arch_info.cache_line_size = 64;  /* Common default */
}

#endif /* RISC-V */

/* ========================================================================= */
/* SYSTEM INFO DETECTION                                                     */
/* ========================================================================= */

static void detect_system_info(void) {
#ifdef __linux__
    /* CPU count */
    g_arch_info.cpu_count = 1;
    g_arch_info.total_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (g_arch_info.total_threads <= 0) {
        g_arch_info.total_threads = 1;
    }

    /* Memory */
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages > 0 && page_size > 0) {
        g_arch_info.total_memory = (uint64_t)pages * page_size;
        g_arch_info.page_size = page_size;
    } else {
        g_arch_info.total_memory = 256 * 1024 * 1024;  /* Assume 256MB */
        g_arch_info.page_size = 4096;
    }

    /* Available memory */
    long avail_pages = sysconf(_SC_AVPHYS_PAGES);
    if (avail_pages > 0) {
        g_arch_info.available_memory = (uint64_t)avail_pages * page_size;
    } else {
        g_arch_info.available_memory = g_arch_info.total_memory / 2;
    }

    /* NUMA detection */
    FILE *fp = fopen("/sys/devices/system/node/online", "r");
    if (fp) {
        char buf[64];
        if (fgets(buf, sizeof(buf), fp)) {
            /* Parse node range like "0-3" or "0" */
            int start = 0, end = 0;
            if (sscanf(buf, "%d-%d", &start, &end) == 2) {
                g_arch_info.numa_nodes = end - start + 1;
            } else if (sscanf(buf, "%d", &start) == 1) {
                g_arch_info.numa_nodes = 1;
            }
            if (g_arch_info.numa_nodes > 1) {
                g_arch_info.capabilities |= TBOS_CAP_NUMA;
            }
        }
        fclose(fp);
    }

    /* Container detection */
    FILE *cgroup = fopen("/proc/1/cgroup", "r");
    if (cgroup) {
        char buf[512];
        while (fgets(buf, sizeof(buf), cgroup)) {
            if (strstr(buf, "docker") || strstr(buf, "lxc") ||
                strstr(buf, "kubepods")) {
                g_arch_info.is_container = true;
                break;
            }
        }
        fclose(cgroup);
    }

#elif defined(__APPLE__)
    /* macOS */
    int mib[2];
    size_t len;

    /* CPU count */
    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    int ncpu;
    len = sizeof(ncpu);
    if (sysctl(mib, 2, &ncpu, &len, NULL, 0) == 0) {
        g_arch_info.total_threads = ncpu;
    } else {
        g_arch_info.total_threads = 1;
    }
    g_arch_info.cpu_count = 1;

    /* Memory */
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    uint64_t memsize;
    len = sizeof(memsize);
    if (sysctl(mib, 2, &memsize, &len, NULL, 0) == 0) {
        g_arch_info.total_memory = memsize;
    } else {
        g_arch_info.total_memory = 256 * 1024 * 1024;
    }

    /* Available memory from Mach */
    mach_port_t host = mach_host_self();
    vm_statistics64_data_t vm_stats;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    if (host_statistics64(host, HOST_VM_INFO64,
                          (host_info64_t)&vm_stats, &count) == KERN_SUCCESS) {
        g_arch_info.available_memory =
            (uint64_t)(vm_stats.free_count + vm_stats.inactive_count) * 4096;
    } else {
        g_arch_info.available_memory = g_arch_info.total_memory / 2;
    }

    g_arch_info.page_size = 4096;
    g_arch_info.numa_nodes = 1;

#else
    /* Unknown/embedded system - use defaults */
    g_arch_info.cpu_count = 1;
    g_arch_info.total_threads = 1;
    g_arch_info.page_size = 4096;
    g_arch_info.numa_nodes = 1;

    /* Estimate based on tier */
    switch (TBOS_ARCH_TIER) {
        case PROCESSOR_TIER_CALCULATOR:
            g_arch_info.total_memory = 2 * 1024;       /* 2KB */
            break;
        case PROCESSOR_TIER_EMBEDDED:
            g_arch_info.total_memory = 64 * 1024;      /* 64KB */
            break;
        case PROCESSOR_TIER_RETRO:
            g_arch_info.total_memory = 16 * 1024 * 1024; /* 16MB */
            break;
        default:
            g_arch_info.total_memory = 256 * 1024 * 1024; /* 256MB */
            break;
    }
    g_arch_info.available_memory = g_arch_info.total_memory / 2;
#endif
}

/* ========================================================================= */
/* TIER DETECTION                                                            */
/* ========================================================================= */

static tbos_processor_tier_t detect_tier(void) {
    uint64_t mem_mb = g_arch_info.total_memory / (1024 * 1024);
    uint32_t threads = g_arch_info.total_threads;

    /* Override with compile-time tier if 8/16-bit */
#if TBOS_ARCH_BITS <= 8
    return PROCESSOR_TIER_CALCULATOR;
#elif TBOS_ARCH_BITS == 16
    return PROCESSOR_TIER_EMBEDDED;
#endif

    /* Runtime detection for 32/64-bit */
    if (mem_mb > 256 * 1024 && threads > 64) {  /* >256GB, >64 cores */
        return PROCESSOR_TIER_SUPERCOMPUTER;
    } else if (mem_mb > 16 * 1024 && threads > 8) {  /* >16GB, >8 cores */
        return PROCESSOR_TIER_SERVER;
    } else if (mem_mb > 1024) {  /* >1GB */
        return PROCESSOR_TIER_DESKTOP;
    } else if (mem_mb > 64) {  /* >64MB */
        return PROCESSOR_TIER_RETRO;
    } else if (mem_mb > 1) {  /* >1MB */
        return PROCESSOR_TIER_EMBEDDED;
    } else {
        return PROCESSOR_TIER_CALCULATOR;
    }
}

/* ========================================================================= */
/* PUBLIC API                                                                */
/* ========================================================================= */

int tbos_arch_init(void) {
    if (g_arch_initialized) {
        return 0;
    }

    memset(&g_arch_info, 0, sizeof(g_arch_info));

    /* Set compile-time known values */
    g_arch_info.arch_name = TBOS_ARCH_NAME;
    g_arch_info.arch_bits = TBOS_ARCH_BITS;
#ifdef TBOS_LITTLE_ENDIAN
    g_arch_info.is_little_endian = true;
#else
    g_arch_info.is_little_endian = false;
#endif

    /* Architecture-specific detection */
#if defined(TBOS_ARCH_X86_64) || defined(TBOS_ARCH_X86)
    detect_x86_capabilities();
#elif defined(TBOS_ARCH_ARM64)
    detect_arm64_capabilities();
#elif defined(TBOS_ARCH_RISCV64) || defined(TBOS_ARCH_RISCV32)
    detect_riscv_capabilities();
#else
    strcpy(g_arch_info.cpu_vendor, "Unknown");
    strcpy(g_arch_info.cpu_model, "Generic CPU");
#endif

    /* System info detection */
    detect_system_info();

    /* Detect tier */
    g_arch_info.tier = detect_tier();

    /* Common capabilities based on bits */
#if TBOS_ARCH_BITS >= 32
    g_arch_info.capabilities |= TBOS_CAP_MMU;
#endif
#if TBOS_ARCH_BITS >= 64
    g_arch_info.capabilities |= TBOS_CAP_ATOMIC64;
#endif

    g_arch_initialized = true;
    return 0;
}

const tbos_arch_info_t* tbos_arch_get_info(void) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return &g_arch_info;
}

tbos_processor_tier_t tbos_arch_get_tier(void) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return g_arch_info.tier;
}

bool tbos_arch_has_capability(uint64_t cap) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return (g_arch_info.capabilities & cap) == cap;
}

uint32_t tbos_arch_get_cpu_count(void) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return g_arch_info.total_threads;
}

uint64_t tbos_arch_get_total_memory(void) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return g_arch_info.total_memory;
}

uint32_t tbos_arch_get_page_size(void) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return g_arch_info.page_size;
}

uint32_t tbos_arch_get_alignment(void) {
    /* Return optimal alignment based on architecture */
#if TBOS_ARCH_BITS <= 8
    return 1;
#elif TBOS_ARCH_BITS == 16
    return 2;
#elif TBOS_ARCH_BITS == 32
    return 4;
#else
    /* 64-bit: prefer cache line alignment for performance */
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return g_arch_info.cache_line_size > 0 ? g_arch_info.cache_line_size : 8;
#endif
}

bool tbos_arch_is_virtual(void) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }
    return g_arch_info.is_virtual;
}

void tbos_arch_print_info(void) {
    if (!g_arch_initialized) {
        tbos_arch_init();
    }

    const char* tier_names[] = {
        "Calculator (8-bit)",
        "Embedded (16-bit)",
        "Retro/Embedded (32-bit)",
        "Desktop (64-bit)",
        "Server/HPC",
        "Supercomputer"
    };

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║      TernaryBit OS - Architecture Information                ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Architecture:    %-42s ║\n", g_arch_info.arch_name);
    printf("║ Bits:            %-42d ║\n", g_arch_info.arch_bits);
    printf("║ Endianness:      %-42s ║\n",
           g_arch_info.is_little_endian ? "Little Endian" : "Big Endian");
    printf("║ Processor Tier:  %-42s ║\n", tier_names[g_arch_info.tier]);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ CPU Vendor:      %-42s ║\n", g_arch_info.cpu_vendor);
    printf("║ CPU Model:       %-42.42s ║\n", g_arch_info.cpu_model);
    printf("║ Logical CPUs:    %-42u ║\n", g_arch_info.total_threads);
    printf("║ NUMA Nodes:      %-42u ║\n", g_arch_info.numa_nodes);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Total Memory:    %-39llu MB ║\n",
           (unsigned long long)(g_arch_info.total_memory / (1024 * 1024)));
    printf("║ Available:       %-39llu MB ║\n",
           (unsigned long long)(g_arch_info.available_memory / (1024 * 1024)));
    printf("║ Page Size:       %-39u KB ║\n", g_arch_info.page_size / 1024);
    printf("║ Cache Line:      %-39u B  ║\n", g_arch_info.cache_line_size);
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    /* Print capabilities */
    printf("║ Capabilities:                                                ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_FPU)
        printf("║   - Floating Point Unit (FPU)                               ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_MMU)
        printf("║   - Memory Management Unit (MMU)                            ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_SIMD)
        printf("║   - SIMD Instructions                                       ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_AVX2)
        printf("║   - AVX2 Vector Extensions                                  ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_AVX512)
        printf("║   - AVX-512 Vector Extensions                               ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_NEON)
        printf("║   - ARM NEON                                                ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_SVE)
        printf("║   - ARM SVE                                                 ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_NUMA)
        printf("║   - NUMA Support                                            ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_AES)
        printf("║   - Hardware AES                                            ║\n");
    if (g_arch_info.capabilities & TBOS_CAP_RDRAND)
        printf("║   - Hardware RNG                                            ║\n");

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    if (g_arch_info.is_virtual) {
        printf("║ Virtual Machine: Yes (%-36s) ║\n",
               g_arch_info.hypervisor ? g_arch_info.hypervisor : "Unknown");
    } else {
        printf("║ Virtual Machine: No (Bare Metal/Native)                     ║\n");
    }
    if (g_arch_info.is_container) {
        printf("║ Container:       Yes                                        ║\n");
    }
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
}
