#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tbos/vfs.h"
#include "fs/ucfs_driver.h"
#include "tbos/fs_drivers.h"

static void test_ucfs_basic_operations() {
    printf("Test: UCFS basic read/write operations...\n");

    // Initialize VFS
    vfs_init();

    // Mount ramfs as root
    assert(vfs_mount("/", &ramfs_driver) == 0);

    // Initialize UCFS driver
    void* ucfs_ctx = ucfs_driver.init();
    assert(ucfs_ctx != NULL);

    // Configure UCFS to use ramfs as backing
    void* ramfs_ctx = ramfs_driver.init();
    assert(ramfs_ctx != NULL);
    assert(ucfs_set_backing_driver(ucfs_ctx, &ramfs_driver, ramfs_ctx, "/ucfs") == 0);

    // Mount UCFS at /ucfs
    assert(vfs_mount_with_context("/ucfs", &ucfs_driver, ucfs_ctx) == 0);

    // Test 1: Write file using UCFS path with emoji delimiter
    const char* test_path = "[🕉️]music[🕉️]chants[🕉️]108.mp3";
    const char* test_data = "Om Namah Shivaya";
    size_t test_data_len = strlen(test_data);

    printf("  Writing to UCFS path: %s\n", test_path);
    int write_result = ucfs_driver.write_file(ucfs_ctx, test_path, test_data, test_data_len, false);
    assert(write_result == 0);

    // Test 2: Read the file back
    char read_buffer[256] = {0};
    size_t read_size = 0;
    printf("  Reading from UCFS path: %s\n", test_path);
    int read_result = ucfs_driver.read_file(ucfs_ctx, test_path, read_buffer, sizeof(read_buffer), &read_size);
    assert(read_result == 0);
    assert(read_size == test_data_len);
    assert(memcmp(read_buffer, test_data, test_data_len) == 0);
    printf("  ✓ Read back: %s\n", read_buffer);

    // Test 3: Check existence
    assert(ucfs_driver.exists(ucfs_ctx, test_path) == true);
    printf("  ✓ File exists check passed\n");

    // Test 4: Different delimiter (a)
    const char* test_path2 = "[a]Home[a]Docs[a]resume.txt";
    const char* test_data2 = "Professional Resume";
    printf("  Writing to UCFS path: %s\n", test_path2);
    assert(ucfs_driver.write_file(ucfs_ctx, test_path2, test_data2, strlen(test_data2), false) == 0);

    memset(read_buffer, 0, sizeof(read_buffer));
    assert(ucfs_driver.read_file(ucfs_ctx, test_path2, read_buffer, sizeof(read_buffer), &read_size) == 0);
    assert(strcmp(read_buffer, test_data2) == 0);
    printf("  ✓ Read back: %s\n", read_buffer);

    // Test 5: Globe emoji delimiter
    const char* test_path3 = "[🌍]🏠[🌍]👤[🌍]documents[🌍]resume.pdf";
    const char* test_data3 = "PDF content here";
    printf("  Writing to UCFS path: %s\n", test_path3);
    assert(ucfs_driver.write_file(ucfs_ctx, test_path3, test_data3, strlen(test_data3), false) == 0);

    memset(read_buffer, 0, sizeof(read_buffer));
    assert(ucfs_driver.read_file(ucfs_ctx, test_path3, read_buffer, sizeof(read_buffer), &read_size) == 0);
    assert(strcmp(read_buffer, test_data3) == 0);
    printf("  ✓ Read back: %s\n", read_buffer);

    printf("✅ UCFS basic operations test passed\n\n");
}

static void test_ucfs_directory_operations() {
    printf("Test: UCFS directory operations...\n");

    vfs_init();
    assert(vfs_mount("/", &ramfs_driver) == 0);

    void* ucfs_ctx = ucfs_driver.init();
    assert(ucfs_ctx != NULL);

    void* ramfs_ctx = ramfs_driver.init();
    assert(ucfs_set_backing_driver(ucfs_ctx, &ramfs_driver, ramfs_ctx, "/ucfs") == 0);
    assert(vfs_mount_with_context("/ucfs", &ucfs_driver, ucfs_ctx) == 0);

    // Create directory using UCFS path
    const char* dir_path = "[📁]projects[📁]tbos";
    printf("  Creating directory: %s\n", dir_path);
    int mkdir_result = ucfs_driver.mkdir(ucfs_ctx, dir_path);
    assert(mkdir_result == 0);

    // Create a file in that directory
    const char* file_path = "[📁]projects[📁]tbos[📁]README.md";
    const char* file_data = "# TernaryBit OS\nUnicode filesystem test";
    printf("  Writing file: %s\n", file_path);
    assert(ucfs_driver.write_file(ucfs_ctx, file_path, file_data, strlen(file_data), false) == 0);

    // Read it back
    char buffer[256] = {0};
    size_t size = 0;
    assert(ucfs_driver.read_file(ucfs_ctx, file_path, buffer, sizeof(buffer), &size) == 0);
    printf("  ✓ Read back %zu bytes\n", size);

    printf("✅ UCFS directory operations test passed\n\n");
}

static void test_ucfs_non_ucfs_paths() {
    printf("Test: UCFS handling of non-UCFS paths...\n");

    vfs_init();
    assert(vfs_mount("/", &ramfs_driver) == 0);

    void* ucfs_ctx = ucfs_driver.init();
    void* ramfs_ctx = ramfs_driver.init();
    assert(ucfs_set_backing_driver(ucfs_ctx, &ramfs_driver, ramfs_ctx, "/ucfs") == 0);
    assert(vfs_mount_with_context("/ucfs", &ucfs_driver, ucfs_ctx) == 0);

    // Regular POSIX path should also work (passthrough)
    const char* posix_path = "/ucfs/regular/path/file.txt";
    const char* posix_data = "Regular POSIX data";
    printf("  Writing to regular path: %s\n", posix_path);
    assert(ucfs_driver.write_file(ucfs_ctx, posix_path, posix_data, strlen(posix_data), false) == 0);

    char buffer[256] = {0};
    size_t size = 0;
    assert(ucfs_driver.read_file(ucfs_ctx, posix_path, buffer, sizeof(buffer), &size) == 0);
    assert(strcmp(buffer, posix_data) == 0);
    printf("  ✓ POSIX path passthrough works\n");

    printf("✅ UCFS non-UCFS path handling test passed\n\n");
}

int main(void) {
    printf("=== UCFS VFS Driver Integration Tests ===\n\n");

    test_ucfs_basic_operations();
    test_ucfs_directory_operations();
    test_ucfs_non_ucfs_paths();

    printf("=== All UCFS driver tests passed! ===\n");
    return 0;
}
