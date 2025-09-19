/**
 * @file tbos_hardware_affinity.c
 * @brief TernaryBit OS Hardware Affinity and NUMA Management
 *
 * Implements intelligent hardware-aware task scheduling and memory allocation
 * to achieve >95% CPU efficiency and >90% multi-threaded efficiency (NFRD compliance)
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>

#ifdef __linux__
#include <sched.h>
#ifdef HAVE_NUMA
#include <numa.h>
#endif
#include <sys/syscall.h>
#endif

// ============================================
// Hardware Topology Structures
// ============================================

typedef struct {
    uint32_t cpu_id;
    uint32_t core_id;
    uint32_t socket_id;
    uint32_t numa_node;
    uint32_t l1_cache_size;
    uint32_t l2_cache_size;
    uint32_t l3_cache_size;
    uint32_t capabilities;      // Bitfield: FPU, AVX, etc.
    uint32_t current_load;      // 0-100%
    uint64_t last_update_time;
} tbos_cpu_info_t;

typedef struct {
    uint32_t node_id;
    uint64_t total_memory;
    uint64_t free_memory;
    uint32_t cpu_count;
    uint32_t cpu_list[64];      // CPUs in this NUMA node
    float memory_bandwidth;     // GB/s
    uint32_t access_latency;    // nanoseconds
} tbos_numa_node_t;

typedef struct {
    uint32_t cpu_count;
    uint32_t numa_node_count;
    uint32_t socket_count;
    tbos_cpu_info_t cpus[256];
    tbos_numa_node_t numa_nodes[16];
    uint32_t cache_coherency_domains[64];
    uint64_t system_memory;
    uint32_t memory_channels;
} tbos_hardware_topology_t;

// Global hardware topology
static tbos_hardware_topology_t g_hw_topology = {0};

// ============================================
// Task Affinity Structures
// ============================================

typedef enum {
    TASK_TYPE_COMPUTE_INTENSIVE,
    TASK_TYPE_MEMORY_INTENSIVE,
    TASK_TYPE_IO_INTENSIVE,
    TASK_TYPE_NETWORK_INTENSIVE,
    TASK_TYPE_REALTIME,
    TASK_TYPE_BACKGROUND
} tbos_task_type_t;

typedef struct {
    uint32_t task_id;
    tbos_task_type_t type;
    uint32_t preferred_cpu;
    uint32_t preferred_numa_node;
    uint32_t cpu_mask;          // Bitmask of allowed CPUs
    uint32_t memory_requirement; // KB
    uint32_t cache_requirement;  // KB
    uint32_t priority;          // 0-100
    uint32_t deadline;          // microseconds (for RT tasks)
    float cpu_utilization;      // Current usage %
} tbos_task_affinity_t;

// ============================================
// Hardware Detection Implementation
// ============================================

/**
 * Detect CPU capabilities using CPUID
 */
static uint32_t detect_cpu_capabilities(uint32_t cpu_id) {
    uint32_t capabilities = 0;

#ifdef __x86_64__
    uint32_t eax, ebx, ecx, edx;

    // Check for FPU
    __asm__ volatile ("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1));

    if (edx & (1 << 0)) capabilities |= 0x01; // FPU
    if (edx & (1 << 15)) capabilities |= 0x02; // CMOV
    if (edx & (1 << 23)) capabilities |= 0x04; // MMX
    if (edx & (1 << 25)) capabilities |= 0x08; // SSE
    if (edx & (1 << 26)) capabilities |= 0x10; // SSE2
    if (ecx & (1 << 0)) capabilities |= 0x20; // SSE3
    if (ecx & (1 << 28)) capabilities |= 0x40; // AVX

    // Check for AVX2
    __asm__ volatile ("cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(7), "c"(0));

    if (ebx & (1 << 5)) capabilities |= 0x80; // AVX2
#endif

    return capabilities;
}

/**
 * Read cache sizes from /proc/cpuinfo or sysfs
 */
static void detect_cache_sizes(uint32_t cpu_id, tbos_cpu_info_t* cpu_info) {
    char path[256];
    FILE* fp;

    // Try to read from sysfs first
    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cache/index0/size", cpu_id);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%dK", &cpu_info->l1_cache_size);
        fclose(fp);
    } else {
        cpu_info->l1_cache_size = 32; // Default 32KB L1
    }

    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cache/index2/size", cpu_id);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%dK", &cpu_info->l2_cache_size);
        fclose(fp);
    } else {
        cpu_info->l2_cache_size = 256; // Default 256KB L2
    }

    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%d/cache/index3/size", cpu_id);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%dK", &cpu_info->l3_cache_size);
        fclose(fp);
    } else {
        cpu_info->l3_cache_size = 8192; // Default 8MB L3
    }
}

/**
 * Initialize hardware topology detection
 */
int tbos_init_hardware_topology(void) {
    printf("🔧 Detecting Hardware Topology...\n");

    // Get CPU count
    g_hw_topology.cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
    printf("   ✅ CPUs: %d\n", g_hw_topology.cpu_count);

    // Detect each CPU
    for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
        tbos_cpu_info_t* cpu = &g_hw_topology.cpus[i];

        cpu->cpu_id = i;
        cpu->capabilities = detect_cpu_capabilities(i);
        detect_cache_sizes(i, cpu);
        cpu->current_load = 0;

        // Try to detect NUMA topology
#ifdef __linux__
#ifdef HAVE_NUMA
        if (numa_available() >= 0) {
            cpu->numa_node = numa_node_of_cpu(i);
        } else {
            cpu->numa_node = 0; // Single NUMA node
        }
#else
        cpu->numa_node = 0;
#endif
#else
        cpu->numa_node = 0;
#endif

        printf("   📊 CPU %d: NUMA=%d, L1=%dKB, L2=%dKB, L3=%dKB, CAP=0x%X\n",
               i, cpu->numa_node, cpu->l1_cache_size, cpu->l2_cache_size,
               cpu->l3_cache_size, cpu->capabilities);
    }

    // Detect NUMA nodes
#ifdef __linux__
#ifdef HAVE_NUMA
    if (numa_available() >= 0) {
        g_hw_topology.numa_node_count = numa_max_node() + 1;

        for (uint32_t i = 0; i < g_hw_topology.numa_node_count; i++) {
            tbos_numa_node_t* node = &g_hw_topology.numa_nodes[i];
            node->node_id = i;
            node->total_memory = numa_node_size64(i, &node->free_memory);

            // Count CPUs in this node
            node->cpu_count = 0;
            for (uint32_t j = 0; j < g_hw_topology.cpu_count; j++) {
                if (g_hw_topology.cpus[j].numa_node == i) {
                    node->cpu_list[node->cpu_count++] = j;
                }
            }

            printf("   🏠 NUMA Node %d: %d CPUs, %ld MB memory\n",
                   i, node->cpu_count, node->total_memory / (1024*1024));
        }
    } else {
        g_hw_topology.numa_node_count = 1;
        g_hw_topology.numa_nodes[0].node_id = 0;
        g_hw_topology.numa_nodes[0].cpu_count = g_hw_topology.cpu_count;
        for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
            g_hw_topology.numa_nodes[0].cpu_list[i] = i;
        }
    }
#else
    g_hw_topology.numa_node_count = 1;
    g_hw_topology.numa_nodes[0].node_id = 0;
    g_hw_topology.numa_nodes[0].cpu_count = g_hw_topology.cpu_count;
    for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
        g_hw_topology.numa_nodes[0].cpu_list[i] = i;
    }
#endif
#else
    g_hw_topology.numa_node_count = 1;
    g_hw_topology.numa_nodes[0].node_id = 0;
    g_hw_topology.numa_nodes[0].cpu_count = g_hw_topology.cpu_count;
    for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
        g_hw_topology.numa_nodes[0].cpu_list[i] = i;
    }
#endif

    printf("   ✅ Hardware topology detected: %d CPUs, %d NUMA nodes\n",
           g_hw_topology.cpu_count, g_hw_topology.numa_node_count);

    return 0;
}

// ============================================
// CPU Load Monitoring
// ============================================

/**
 * Update CPU load statistics
 */
static void update_cpu_loads(void) {
    FILE* fp = fopen("/proc/stat", "r");
    if (!fp) return;

    char line[256];
    uint32_t cpu_idx = 0;

    while (fgets(line, sizeof(line), fp) && cpu_idx < g_hw_topology.cpu_count) {
        if (strncmp(line, "cpu", 3) == 0 && line[3] >= '0' && line[3] <= '9') {
            unsigned long user, nice, system, idle, iowait, irq, softirq;
            sscanf(line, "cpu%*d %lu %lu %lu %lu %lu %lu %lu",
                   &user, &nice, &system, &idle, &iowait, &irq, &softirq);

            unsigned long total = user + nice + system + idle + iowait + irq + softirq;
            unsigned long active = total - idle - iowait;

            if (total > 0) {
                g_hw_topology.cpus[cpu_idx].current_load = (active * 100) / total;
            }
            cpu_idx++;
        }
    }

    fclose(fp);
}

// ============================================
// Intelligent Task Placement
// ============================================

/**
 * Find best CPU for compute-intensive task
 */
static uint32_t find_best_cpu_for_compute(tbos_task_affinity_t* task) {
    uint32_t best_cpu = 0;
    uint32_t best_score = 0;

    for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
        if (!(task->cpu_mask & (1 << i))) continue; // Not allowed

        tbos_cpu_info_t* cpu = &g_hw_topology.cpus[i];
        uint32_t score = 0;

        // Prefer CPUs with higher capabilities
        if (cpu->capabilities & 0x40) score += 50; // AVX
        if (cpu->capabilities & 0x80) score += 50; // AVX2
        if (cpu->capabilities & 0x08) score += 20; // SSE

        // Prefer larger caches
        score += cpu->l3_cache_size / 1024; // Points per MB
        score += cpu->l2_cache_size / 64;   // Points per 64KB

        // Prefer less loaded CPUs
        score += (100 - cpu->current_load);

        if (score > best_score) {
            best_score = score;
            best_cpu = i;
        }
    }

    return best_cpu;
}

/**
 * Find best CPU for memory-intensive task
 */
static uint32_t find_best_cpu_for_memory(tbos_task_affinity_t* task) {
    uint32_t best_cpu = 0;
    uint32_t best_numa_node = 0;
    uint64_t most_free_memory = 0;

    // Find NUMA node with most free memory
    for (uint32_t i = 0; i < g_hw_topology.numa_node_count; i++) {
        if (g_hw_topology.numa_nodes[i].free_memory > most_free_memory) {
            most_free_memory = g_hw_topology.numa_nodes[i].free_memory;
            best_numa_node = i;
        }
    }

    // Find least loaded CPU in that NUMA node
    uint32_t min_load = 100;
    tbos_numa_node_t* node = &g_hw_topology.numa_nodes[best_numa_node];

    for (uint32_t i = 0; i < node->cpu_count; i++) {
        uint32_t cpu_id = node->cpu_list[i];
        if (!(task->cpu_mask & (1 << cpu_id))) continue;

        if (g_hw_topology.cpus[cpu_id].current_load < min_load) {
            min_load = g_hw_topology.cpus[cpu_id].current_load;
            best_cpu = cpu_id;
        }
    }

    return best_cpu;
}

/**
 * Assign task to optimal CPU based on task characteristics
 */
uint32_t tbos_assign_task_affinity(tbos_task_affinity_t* task) {
    update_cpu_loads();

    uint32_t assigned_cpu = 0;

    switch (task->type) {
        case TASK_TYPE_COMPUTE_INTENSIVE:
            assigned_cpu = find_best_cpu_for_compute(task);
            break;

        case TASK_TYPE_MEMORY_INTENSIVE:
            assigned_cpu = find_best_cpu_for_memory(task);
            break;

        case TASK_TYPE_REALTIME:
            // For RT tasks, prefer dedicated CPU with lowest load
            assigned_cpu = 0;
            uint32_t min_load = 100;
            for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
                if (g_hw_topology.cpus[i].current_load < min_load) {
                    min_load = g_hw_topology.cpus[i].current_load;
                    assigned_cpu = i;
                }
            }
            break;

        default:
            // Default: round-robin assignment
            assigned_cpu = task->task_id % g_hw_topology.cpu_count;
            break;
    }

    task->preferred_cpu = assigned_cpu;
    task->preferred_numa_node = g_hw_topology.cpus[assigned_cpu].numa_node;

    printf("📍 Task %d (%s) → CPU %d (NUMA %d, Load %d%%)\n",
           task->task_id,
           task->type == TASK_TYPE_COMPUTE_INTENSIVE ? "COMPUTE" :
           task->type == TASK_TYPE_MEMORY_INTENSIVE ? "MEMORY" :
           task->type == TASK_TYPE_REALTIME ? "REALTIME" : "DEFAULT",
           assigned_cpu,
           task->preferred_numa_node,
           g_hw_topology.cpus[assigned_cpu].current_load);

    return assigned_cpu;
}

// ============================================
// NUMA-Aware Memory Allocation
// ============================================

/**
 * Allocate memory on specific NUMA node
 */
void* tbos_numa_alloc(size_t size, uint32_t numa_node) {
    void* ptr = NULL;

#ifdef __linux__
#ifdef HAVE_NUMA
    if (numa_available() >= 0 && numa_node < g_hw_topology.numa_node_count) {
        ptr = numa_alloc_onnode(size, numa_node);
        if (ptr) {
            printf("🧠 Allocated %zu bytes on NUMA node %d\n", size, numa_node);
            return ptr;
        }
    }
#endif
#endif

    // Fallback to regular allocation
    ptr = malloc(size);
    if (ptr) {
        printf("🧠 Allocated %zu bytes (fallback allocation)\n", size);
    }

    return ptr;
}

/**
 * Free NUMA-allocated memory
 */
void tbos_numa_free(void* ptr, size_t size) {
#ifdef __linux__
#ifdef HAVE_NUMA
    if (numa_available() >= 0) {
        numa_free(ptr, size);
        return;
    }
#endif
#endif
    free(ptr);
}

// ============================================
// Hardware Affinity API
// ============================================

/**
 * Set CPU affinity for current thread
 */
int tbos_set_thread_affinity(uint32_t cpu_id) {
#ifdef __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_id, &cpuset);

    int result = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
    if (result == 0) {
        printf("⚡ Thread bound to CPU %d\n", cpu_id);
    }
    return result;
#else
    printf("⚡ CPU affinity not supported on this platform\n");
    return -1;
#endif
}

/**
 * Get current CPU ID
 */
uint32_t tbos_get_current_cpu(void) {
#ifdef __linux__
    return sched_getcpu();
#else
    return 0;
#endif
}

// ============================================
// Performance Monitoring
// ============================================

/**
 * Calculate achieved CPU efficiency
 */
float tbos_calculate_cpu_efficiency(void) {
    update_cpu_loads();

    uint32_t total_load = 0;
    uint32_t active_cpus = 0;

    for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
        if (g_hw_topology.cpus[i].current_load > 5) { // Consider >5% as active
            total_load += g_hw_topology.cpus[i].current_load;
            active_cpus++;
        }
    }

    if (active_cpus == 0) return 0.0f;

    float efficiency = (float)total_load / active_cpus;
    return efficiency;
}

/**
 * Print hardware affinity statistics
 */
void tbos_print_affinity_stats(void) {
    printf("\n📊 Hardware Affinity Statistics:\n");
    printf("════════════════════════════════\n");

    float cpu_efficiency = tbos_calculate_cpu_efficiency();
    printf("CPU Efficiency: %.1f%% ", cpu_efficiency);

    if (cpu_efficiency >= 95.0f) {
        printf("✅ EXCELLENT (NFRD Target: >95%%)\n");
    } else if (cpu_efficiency >= 90.0f) {
        printf("✅ GOOD (Above 90%%)\n");
    } else if (cpu_efficiency >= 80.0f) {
        printf("⚠️ ACCEPTABLE (Above 80%%)\n");
    } else {
        printf("❌ NEEDS OPTIMIZATION (Below 80%%)\n");
    }

    printf("\nPer-CPU Load Distribution:\n");
    for (uint32_t i = 0; i < g_hw_topology.cpu_count; i++) {
        printf("CPU %2d: %3d%% [", i, g_hw_topology.cpus[i].current_load);
        uint32_t bars = g_hw_topology.cpus[i].current_load / 5;
        for (uint32_t j = 0; j < 20; j++) {
            printf("%c", j < bars ? '█' : '░');
        }
        printf("]\n");
    }

    printf("\nNUMA Memory Distribution:\n");
    for (uint32_t i = 0; i < g_hw_topology.numa_node_count; i++) {
        printf("NUMA %d: %ld MB free\n",
               i, g_hw_topology.numa_nodes[i].free_memory / (1024*1024));
    }
}

// ============================================
// Hardware Affinity Test Suite
// ============================================

/**
 * Test CPU-intensive workload
 */
void* cpu_intensive_worker(void* arg) {
    tbos_task_affinity_t task = {
        .task_id = *(uint32_t*)arg,
        .type = TASK_TYPE_COMPUTE_INTENSIVE,
        .cpu_mask = 0xFFFFFFFF, // Allow any CPU
        .priority = 80
    };

    uint32_t cpu = tbos_assign_task_affinity(&task);
    tbos_set_thread_affinity(cpu);

    // Simulate CPU-intensive work
    double result = 0.0;
    for (int i = 0; i < 10000000; i++) {
        result += i * 3.14159;
    }

    printf("🔥 CPU task %d completed on CPU %d, result: %f\n",
           task.task_id, tbos_get_current_cpu(), result);

    return NULL;
}

/**
 * Test memory-intensive workload
 */
void* memory_intensive_worker(void* arg) {
    tbos_task_affinity_t task = {
        .task_id = *(uint32_t*)arg + 100,
        .type = TASK_TYPE_MEMORY_INTENSIVE,
        .cpu_mask = 0xFFFFFFFF,
        .memory_requirement = 1024, // 1MB
        .priority = 70
    };

    uint32_t cpu = tbos_assign_task_affinity(&task);
    tbos_set_thread_affinity(cpu);

    // Allocate NUMA-aware memory
    void* buffer = tbos_numa_alloc(task.memory_requirement * 1024, task.preferred_numa_node);
    if (buffer) {
        // Simulate memory-intensive work
        memset(buffer, 0xAA, task.memory_requirement * 1024);

        for (int i = 0; i < 1000; i++) {
            volatile char* ptr = (char*)buffer;
            for (size_t j = 0; j < task.memory_requirement * 1024; j += 64) {
                ptr[j] = i & 0xFF; // Cache line access pattern
            }
        }

        tbos_numa_free(buffer, task.memory_requirement * 1024);
    }

    printf("🧠 Memory task %d completed on CPU %d (NUMA %d)\n",
           task.task_id, tbos_get_current_cpu(), task.preferred_numa_node);

    return NULL;
}

/**
 * Comprehensive hardware affinity test
 */
int tbos_test_hardware_affinity(void) {
    printf("\n🧪 Testing Hardware Affinity System...\n");
    printf("═══════════════════════════════════════\n");

    // Initialize hardware detection
    if (tbos_init_hardware_topology() != 0) {
        printf("❌ Hardware topology detection failed\n");
        return -1;
    }

    // Create mixed workload
    pthread_t threads[16];
    uint32_t thread_ids[16];

    printf("\n🚀 Starting mixed workload test...\n");

    // Start CPU-intensive threads
    for (int i = 0; i < 4; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, cpu_intensive_worker, &thread_ids[i]);
    }

    // Start memory-intensive threads
    for (int i = 4; i < 8; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, memory_intensive_worker, &thread_ids[i]);
    }

    // Wait for completion
    for (int i = 0; i < 8; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n✅ Workload test completed\n");

    // Print final statistics
    tbos_print_affinity_stats();

    printf("\n🎯 NFRD Compliance Check:\n");
    float efficiency = tbos_calculate_cpu_efficiency();
    if (efficiency >= 95.0f) {
        printf("✅ NFR-PFM-004: CPU Efficiency >95%% ✓\n");
    } else {
        printf("⚠️ NFR-PFM-004: CPU Efficiency %.1f%% (target >95%%)\n", efficiency);
    }

    return 0;
}