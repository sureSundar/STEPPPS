/*
 * RF2S Driver Unit Tests
 * Tests VFS driver operations
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Host build compatibility */
#ifdef HOST_BUILD
#include <errno.h>
#define EINVAL 22
#define ENOSPC 28
#define ENOMEM 12
#define ENOENT 2
#define ENOSYS 38

/* Stubs for kernel functions */
void kernel_print(const char* s) { (void)s; }
void kernel_print_hex(unsigned int n) { (void)n; }
#endif

#include "tbos/vfs.h"
#include "fs/rf2s_codec.h"
#include "fs/rf2s_driver.h"

/* External driver declarations */
extern const vfs_driver_t ramfs_driver;
extern const vfs_driver_t rf2s_driver;
extern int rf2s_set_backing_driver(void*, const vfs_driver_t*, void*, const char*);

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    tests_run++; \
    test_##name(); \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAIL: %s != %s (%d != %d)\n", #a, #b, (int)(a), (int)(b)); \
        return; \
    } \
} while(0)

#define ASSERT_TRUE(a) do { \
    if (!(a)) { \
        printf("FAIL: %s is false\n", #a); \
        return; \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        printf("FAIL: %s != %s (\"%s\" != \"%s\")\n", #a, #b, (a), (b)); \
        return; \
    } \
} while(0)

/* Shared driver context */
static void* rf2s_ctx = NULL;
static void* ramfs_ctx = NULL;

static void setup(void) {
    /* Initialize VFS */
    vfs_init();

    /* Initialize RAMFS as backing */
    ramfs_ctx = ramfs_driver.init();
    vfs_mount_with_context("/", &ramfs_driver, ramfs_ctx);

    /* Create rf2s backing directory */
    vfs_mkdir("/rf2s");

    /* Initialize RF2S driver */
    rf2s_ctx = rf2s_driver.init();
    rf2s_set_backing_driver(rf2s_ctx, &ramfs_driver, ramfs_ctx, "/rf2s");

    /* Mount RF2S */
    vfs_mount_with_context("/rf2s", &rf2s_driver, rf2s_ctx);
}

/* Test driver initialization */
TEST(init) {
    ASSERT_TRUE(rf2s_ctx != NULL);
}

/* Test mkdir with frequency path */
TEST(mkdir_frequency) {
    int rc = vfs_mkdir("/rf2s/432MHz");
    ASSERT_EQ(rc, 0);

    /* Verify directory exists (via backing) */
    ASSERT_TRUE(vfs_exists("/rf2s/432000000"));
}

/* Test mkdir with nested path */
TEST(mkdir_nested) {
    int rc = vfs_mkdir("/rf2s/432MHz/sensors");
    ASSERT_EQ(rc, 0);

    ASSERT_TRUE(vfs_exists("/rf2s/432000000/sensors"));
}

/* Test write and read file */
TEST(write_read_file) {
    const char* test_data = "RF2S test data";
    size_t data_len = strlen(test_data);

    /* Write file via RF2S path */
    int rc = vfs_write_file("/rf2s/432MHz/test.dat", test_data, data_len);
    ASSERT_EQ(rc, 0);

    /* Read it back */
    char buffer[256] = {0};
    size_t read_size = 0;
    rc = vfs_read_file("/rf2s/432MHz/test.dat", buffer, sizeof(buffer), &read_size);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(read_size, data_len);
    ASSERT_STR_EQ(buffer, test_data);
}

/* Test file exists */
TEST(exists) {
    vfs_write_file("/rf2s/432MHz/exists.txt", "hello", 5);

    ASSERT_TRUE(vfs_exists("/rf2s/432MHz/exists.txt"));
    ASSERT_TRUE(!vfs_exists("/rf2s/432MHz/nonexistent.txt"));
}

/* Test file type */
TEST(type) {
    vfs_mkdir("/rf2s/2GHz");
    vfs_write_file("/rf2s/2GHz/file.bin", "data", 4);

    ASSERT_EQ(vfs_type("/rf2s/2GHz"), VFS_NODE_DIR);
    ASSERT_EQ(vfs_type("/rf2s/2GHz/file.bin"), VFS_NODE_FILE);
}

/* Test remove file */
TEST(remove_file) {
    vfs_write_file("/rf2s/432MHz/to_remove.txt", "bye", 3);
    ASSERT_TRUE(vfs_exists("/rf2s/432MHz/to_remove.txt"));

    int rc = vfs_remove("/rf2s/432MHz/to_remove.txt", 0);
    ASSERT_EQ(rc, 0);

    ASSERT_TRUE(!vfs_exists("/rf2s/432MHz/to_remove.txt"));
}

/* Test with GHz frequency */
TEST(ghz_frequency) {
    const char* data = "WiFi beacon";
    int rc = vfs_write_file("/rf2s/2.4GHz/beacon.dat", data, strlen(data));
    ASSERT_EQ(rc, 0);

    /* Verify via backing path */
    ASSERT_TRUE(vfs_exists("/rf2s/2400000000/beacon.dat"));
}

/* Test with kHz frequency */
TEST(khz_frequency) {
    vfs_mkdir("/rf2s/432kHz");
    ASSERT_TRUE(vfs_exists("/rf2s/432000"));
}

/* Test list directory callback */
static int list_count = 0;
static int list_callback(const char* name, vfs_node_type_t type, void* user) {
    (void)name;
    (void)type;
    (void)user;
    list_count++;
    return 0;
}

TEST(list_dir) {
    /* Create some files in a frequency dir */
    vfs_mkdir("/rf2s/100MHz");
    vfs_write_file("/rf2s/100MHz/a.txt", "a", 1);
    vfs_write_file("/rf2s/100MHz/b.txt", "b", 1);
    vfs_write_file("/rf2s/100MHz/c.txt", "c", 1);

    list_count = 0;
    int rc = vfs_list_dir("/rf2s/100MHz", list_callback, NULL);
    ASSERT_EQ(rc, 0);
    ASSERT_EQ(list_count, 3);
}

/* Test auto-create parent directories */
TEST(auto_create_parents) {
    /* Write to a path where parent doesn't exist yet */
    int rc = vfs_write_file("/rf2s/500MHz/sub/deep/file.txt", "deep", 4);
    ASSERT_EQ(rc, 0);

    /* Parent directories should have been created */
    ASSERT_TRUE(vfs_exists("/rf2s/500000000"));
    ASSERT_TRUE(vfs_exists("/rf2s/500000000/sub"));
    ASSERT_TRUE(vfs_exists("/rf2s/500000000/sub/deep"));
}

int main(void) {
    printf("RF2S Driver Unit Tests\n");
    printf("======================\n\n");

    /* Setup VFS and drivers */
    setup();

    printf("Driver Tests:\n");
    RUN_TEST(init);
    RUN_TEST(mkdir_frequency);
    RUN_TEST(mkdir_nested);
    RUN_TEST(write_read_file);
    RUN_TEST(exists);
    RUN_TEST(type);
    RUN_TEST(remove_file);

    printf("\nFrequency Unit Tests:\n");
    RUN_TEST(ghz_frequency);
    RUN_TEST(khz_frequency);

    printf("\nAdvanced Tests:\n");
    RUN_TEST(list_dir);
    RUN_TEST(auto_create_parents);

    printf("\n======================\n");
    printf("Results: %d/%d tests passed\n", tests_passed, tests_run);

    return (tests_passed == tests_run) ? 0 : 1;
}
