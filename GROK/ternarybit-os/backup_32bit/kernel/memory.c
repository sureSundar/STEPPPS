// TBOS Memory Manager - Sprint 8
// 🕉️ Satyameva Jayate 🕉️
// Simple physical memory allocator

#include <stdint.h>
#include <stddef.h>

// Memory constants
#define PAGE_SIZE 4096
#define MEMORY_START 0x200000  // Start after 2MB (kernel space)
#define MEMORY_SIZE  0x1000000 // 16MB total for now

// Memory bitmap (1 bit per page)
#define BITMAP_SIZE (MEMORY_SIZE / PAGE_SIZE / 8)
static uint8_t memory_bitmap[BITMAP_SIZE];
static uint32_t free_pages = MEMORY_SIZE / PAGE_SIZE;

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);

// Initialize memory manager
void memory_init(void) {
    // Mark all pages as free (0 = free, 1 = used)
    for (int i = 0; i < BITMAP_SIZE; i++) {
        memory_bitmap[i] = 0;
    }

    // Mark kernel pages as used (0-2MB)
    int kernel_pages = MEMORY_START / PAGE_SIZE;
    for (int i = 0; i < kernel_pages / 8; i++) {
        memory_bitmap[i] = 0xFF;
    }

    free_pages -= kernel_pages;

    kernel_print("  Memory initialized: ");
    kernel_print_hex(free_pages * PAGE_SIZE);
    kernel_print(" bytes free\n");
}

// Find first free page
static int find_free_page(void) {
    for (int i = 0; i < BITMAP_SIZE; i++) {
        if (memory_bitmap[i] != 0xFF) {
            // Found byte with free page
            for (int bit = 0; bit < 8; bit++) {
                if (!(memory_bitmap[i] & (1 << bit))) {
                    return i * 8 + bit;
                }
            }
        }
    }
    return -1; // No free pages
}

// Allocate a page
void* page_alloc(void) {
    int page = find_free_page();
    if (page == -1) {
        kernel_print("[MEMORY] Out of memory!\n");
        return NULL;
    }

    // Mark page as used
    memory_bitmap[page / 8] |= (1 << (page % 8));
    free_pages--;

    // Return physical address
    return (void*)(page * PAGE_SIZE);
}

// Free a page
void page_free(void* addr) {
    uint32_t page = (uint32_t)addr / PAGE_SIZE;

    // Mark page as free
    memory_bitmap[page / 8] &= ~(1 << (page % 8));
    free_pages++;
}

// Simple kmalloc (allocates whole pages for now)
void* kmalloc(size_t size) {
    if (size == 0) return NULL;

    // Round up to page size
    size_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    // For now, just allocate one page
    // TODO: Implement proper heap allocator
    return page_alloc();
}

// Simple kfree
void kfree(void* ptr) {
    if (ptr == NULL) return;
    page_free(ptr);
}

// Get free memory
uint32_t get_free_memory(void) {
    return free_pages * PAGE_SIZE;
}

// Get used memory
uint32_t get_used_memory(void) {
    return (MEMORY_SIZE / PAGE_SIZE - free_pages) * PAGE_SIZE;
}