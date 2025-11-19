/**
 * @file test_tbos_memory.c
 * @brief Test TBOS memory manager
 */

#include "../../src/core/tbos_memory.c"
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║      TBOS Memory Manager Test                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    /* Test 1: Initialization */
    printf("[TEST 1] Memory initialization...\n");
    int result = tbos_memory_init(1024 * 1024);  /* 1MB heap */
    if (result != 0) {
        printf("❌ Init failed\n");
        return 1;
    }
    printf("✅ Memory initialized\n\n");

    /* Test 2: Simple allocation */
    printf("[TEST 2] Simple allocation...\n");
    void* ptr1 = tbos_malloc(100);
    if (!ptr1) {
        printf("❌ Allocation failed\n");
        return 1;
    }
    printf("✅ Allocated 100 bytes at %p\n", ptr1);

    /* Test 3: Write and verify */
    printf("[TEST 3] Write and verify...\n");
    strcpy((char*)ptr1, "Hello, TBOS!");
    if (strcmp((char*)ptr1, "Hello, TBOS!") != 0) {
        printf("❌ Data mismatch\n");
        return 1;
    }
    printf("✅ Data verified: \"%s\"\n", (char*)ptr1);

    /* Test 4: Multiple allocations */
    printf("[TEST 4] Multiple allocations...\n");
    void* ptr2 = tbos_malloc(200);
    void* ptr3 = tbos_malloc(300);
    void* ptr4 = tbos_malloc(400);
    if (!ptr2 || !ptr3 || !ptr4) {
        printf("❌ Multiple allocations failed\n");
        return 1;
    }
    printf("✅ Allocated 4 blocks successfully\n");

    /* Test 5: Calloc (zero-initialized) */
    printf("[TEST 5] Calloc (zero memory)...\n");
    int* array = (int*)tbos_calloc(10, sizeof(int));
    if (!array) {
        printf("❌ Calloc failed\n");
        return 1;
    }
    bool all_zero = true;
    for (int i = 0; i < 10; i++) {
        if (array[i] != 0) {
            all_zero = false;
            break;
        }
    }
    if (!all_zero) {
        printf("❌ Memory not zeroed\n");
        return 1;
    }
    printf("✅ Calloc returned zeroed memory\n");

    /* Test 6: Realloc */
    printf("[TEST 6] Realloc...\n");
    char* str = (char*)tbos_malloc(10);
    strcpy(str, "Short");
    str = (char*)tbos_realloc(str, 100);
    if (!str || strcmp(str, "Short") != 0) {
        printf("❌ Realloc failed or data lost\n");
        return 1;
    }
    strcat(str, " but now much longer!");
    printf("✅ Realloc successful: \"%s\"\n", str);

    /* Test 7: Free */
    printf("[TEST 7] Free...\n");
    tbos_free(ptr1);
    tbos_free(ptr2);
    tbos_free(ptr3);
    tbos_free(ptr4);
    tbos_free(array);
    tbos_free(str);
    printf("✅ All pointers freed\n");

    /* Test 8: Statistics */
    printf("[TEST 8] Memory statistics...\n");
    tbos_memory_print_stats();

    /* Test 9: Large allocation */
    printf("[TEST 9] Large allocation (100KB)...\n");
    void* large = tbos_malloc(100 * 1024);
    if (!large) {
        printf("❌ Large allocation failed\n");
        return 1;
    }
    printf("✅ Allocated 100KB at %p\n", large);
    tbos_free(large);

    /* Final statistics */
    printf("\n[FINAL] Memory statistics:\n");
    tbos_memory_print_stats();

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║      ✅ ALL MEMORY TESTS PASSED                          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n\n");

    return 0;
}
