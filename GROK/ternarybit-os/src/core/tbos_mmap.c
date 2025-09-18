/**
 * @file tbos_mmap.c
 * @brief TernaryBit OS Real Memory-Mapped File Operations
 *
 * Implements genuine memory-mapped file operations for TernaryBit OS.
 * This provides real shared memory, file mapping, and zero-copy I/O.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

// Memory mapping structures
typedef struct tbos_mmap_region {
    void* address;
    size_t size;
    int protection;
    int flags;
    int fd;
    off_t offset;
    char name[256];
    struct tbos_mmap_region* next;
} tbos_mmap_region_t;

typedef struct {
    tbos_mmap_region_t* regions;
    size_t total_mapped;
    size_t region_count;
} tbos_mmap_manager_t;

// Global memory map manager
static tbos_mmap_manager_t* g_mmap_manager = NULL;

/**
 * Initialize memory mapping subsystem
 */
int tbos_mmap_init(void) {
    if (g_mmap_manager) return 0;

    g_mmap_manager = malloc(sizeof(tbos_mmap_manager_t));
    if (!g_mmap_manager) return -1;

    memset(g_mmap_manager, 0, sizeof(tbos_mmap_manager_t));
    printf("✅ TBOS Memory Mapping subsystem initialized\n");
    return 0;
}

/**
 * Map a file into memory
 */
void* tbos_mmap_file(const char* filepath, size_t* size, bool writable) {
    if (!filepath || !size) return NULL;

    // Open the file
    int fd = open(filepath, writable ? O_RDWR : O_RDONLY);
    if (fd < 0) {
        printf("❌ Failed to open file %s: %s\n", filepath, strerror(errno));
        return NULL;
    }

    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        return NULL;
    }

    // Map the file into memory
    int prot = PROT_READ | (writable ? PROT_WRITE : 0);
    void* mapped = mmap(NULL, st.st_size, prot, MAP_SHARED, fd, 0);

    if (mapped == MAP_FAILED) {
        printf("❌ Failed to map file %s: %s\n", filepath, strerror(errno));
        close(fd);
        return NULL;
    }

    // Track the mapping
    tbos_mmap_region_t* region = malloc(sizeof(tbos_mmap_region_t));
    if (region) {
        region->address = mapped;
        region->size = st.st_size;
        region->protection = prot;
        region->flags = MAP_SHARED;
        region->fd = fd;
        region->offset = 0;
        strncpy(region->name, filepath, 255);

        // Add to list
        region->next = g_mmap_manager->regions;
        g_mmap_manager->regions = region;
        g_mmap_manager->total_mapped += st.st_size;
        g_mmap_manager->region_count++;
    }

    *size = st.st_size;
    printf("✅ Mapped file %s (%zu bytes) at address %p\n", filepath, st.st_size, mapped);
    return mapped;
}

/**
 * Create shared memory region
 */
void* tbos_create_shared_memory(const char* name, size_t size) {
    if (!name || size == 0) return NULL;

    // Create shared memory using anonymous mapping
    void* shared = mmap(NULL, size, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (shared == MAP_FAILED) {
        printf("❌ Failed to create shared memory: %s\n", strerror(errno));
        return NULL;
    }

    // Track the shared memory region
    tbos_mmap_region_t* region = malloc(sizeof(tbos_mmap_region_t));
    if (region) {
        region->address = shared;
        region->size = size;
        region->protection = PROT_READ | PROT_WRITE;
        region->flags = MAP_SHARED | MAP_ANONYMOUS;
        region->fd = -1;
        region->offset = 0;
        strncpy(region->name, name, 255);

        region->next = g_mmap_manager->regions;
        g_mmap_manager->regions = region;
        g_mmap_manager->total_mapped += size;
        g_mmap_manager->region_count++;
    }

    printf("✅ Created shared memory '%s' (%zu bytes) at address %p\n", name, size, shared);
    return shared;
}

/**
 * Synchronize mapped memory to disk
 */
int tbos_mmap_sync(void* addr, size_t size) {
    if (!addr || size == 0) return -1;

    if (msync(addr, size, MS_SYNC) == 0) {
        printf("✅ Synchronized %zu bytes at %p to disk\n", size, addr);
        return 0;
    }

    printf("❌ Failed to sync memory: %s\n", strerror(errno));
    return -1;
}

/**
 * Unmap memory region
 */
int tbos_mmap_unmap(void* addr, size_t size) {
    if (!addr || size == 0) return -1;

    // Find and remove from tracking
    tbos_mmap_region_t** current = &g_mmap_manager->regions;
    while (*current) {
        if ((*current)->address == addr) {
            tbos_mmap_region_t* region = *current;
            *current = region->next;

            // Close file descriptor if any
            if (region->fd >= 0) {
                close(region->fd);
            }

            g_mmap_manager->total_mapped -= region->size;
            g_mmap_manager->region_count--;

            printf("✅ Unmapping region '%s' at %p\n", region->name, addr);
            free(region);
            break;
        }
        current = &(*current)->next;
    }

    // Unmap the memory
    if (munmap(addr, size) == 0) {
        return 0;
    }

    printf("❌ Failed to unmap memory: %s\n", strerror(errno));
    return -1;
}

/**
 * Change memory protection
 */
int tbos_mmap_protect(void* addr, size_t size, int prot) {
    if (!addr || size == 0) return -1;

    if (mprotect(addr, size, prot) == 0) {
        printf("✅ Changed protection for %zu bytes at %p\n", size, addr);

        // Update tracking
        tbos_mmap_region_t* region = g_mmap_manager->regions;
        while (region) {
            if (region->address == addr) {
                region->protection = prot;
                break;
            }
            region = region->next;
        }
        return 0;
    }

    printf("❌ Failed to change protection: %s\n", strerror(errno));
    return -1;
}

/**
 * Lock memory pages in RAM
 */
int tbos_mmap_lock(void* addr, size_t size) {
    if (!addr || size == 0) return -1;

    if (mlock(addr, size) == 0) {
        printf("✅ Locked %zu bytes at %p in RAM\n", size, addr);
        return 0;
    }

    printf("⚠️  Failed to lock memory (may need privileges): %s\n", strerror(errno));
    return -1;
}

/**
 * Get memory mapping statistics
 */
void tbos_mmap_stats(void) {
    if (!g_mmap_manager) {
        printf("Memory mapping not initialized\n");
        return;
    }

    printf("\n📊 Memory Mapping Statistics:\n");
    printf("   Total mapped: %zu bytes\n", g_mmap_manager->total_mapped);
    printf("   Active regions: %zu\n", g_mmap_manager->region_count);

    if (g_mmap_manager->regions) {
        printf("   Mapped regions:\n");
        tbos_mmap_region_t* region = g_mmap_manager->regions;
        while (region) {
            printf("     - %s: %zu bytes at %p (fd=%d)\n",
                   region->name, region->size, region->address, region->fd);
            region = region->next;
        }
    }
}

/**
 * Cleanup memory mapping subsystem
 */
void tbos_mmap_cleanup(void) {
    if (!g_mmap_manager) return;

    // Unmap all regions
    while (g_mmap_manager->regions) {
        tbos_mmap_region_t* region = g_mmap_manager->regions;
        g_mmap_manager->regions = region->next;

        munmap(region->address, region->size);
        if (region->fd >= 0) {
            close(region->fd);
        }
        free(region);
    }

    free(g_mmap_manager);
    g_mmap_manager = NULL;
    printf("✅ Memory mapping subsystem cleaned up\n");
}

/**
 * Demonstration of memory-mapped operations
 */
void tbos_mmap_demo(void) {
    printf("\n🗺️  === TBOS Memory-Mapped File Operations Demo ===\n");

    // Initialize
    tbos_mmap_init();

    // Create a test file
    const char* test_file = "tbos_mmap_test.dat";
    int fd = open(test_file, O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (fd >= 0) {
        // Write initial data
        const char* initial = "Initial data for memory mapping test\n";
        write(fd, initial, strlen(initial));
        close(fd);

        // Map the file
        size_t file_size;
        char* mapped = tbos_mmap_file(test_file, &file_size, true);

        if (mapped) {
            printf("📝 Original content: %.*s", (int)file_size, mapped);

            // Modify through memory mapping
            strcpy(mapped, "Modified via memory mapping!\n");

            // Sync to disk
            tbos_mmap_sync(mapped, file_size);

            // Unmap
            tbos_mmap_unmap(mapped, file_size);

            // Verify changes persisted
            fd = open(test_file, O_RDONLY);
            char buffer[256];
            ssize_t read_bytes = read(fd, buffer, sizeof(buffer));
            buffer[read_bytes] = '\0';
            printf("📝 After mapping: %s", buffer);
            close(fd);
        }

        unlink(test_file);
    }

    // Create shared memory
    size_t shared_size = 4096;
    void* shared = tbos_create_shared_memory("tbos_shared_region", shared_size);
    if (shared) {
        // Write to shared memory
        strcpy(shared, "Data in shared memory region!");
        printf("📝 Shared memory content: %s\n", (char*)shared);

        // Lock in RAM (may fail without privileges)
        tbos_mmap_lock(shared, shared_size);

        // Change protection to read-only
        tbos_mmap_protect(shared, shared_size, PROT_READ);

        // Unmap shared memory
        tbos_mmap_unmap(shared, shared_size);
    }

    // Show statistics
    tbos_mmap_stats();

    // Cleanup
    tbos_mmap_cleanup();

    printf("✅ Memory-mapped operations demonstration complete!\n");
}