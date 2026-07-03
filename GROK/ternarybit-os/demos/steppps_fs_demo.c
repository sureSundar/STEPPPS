/**
 * steppps_fs_demo.c - STEPPPS Filesystem Integration Demo
 *
 * Demonstrates:
 * 1. STEPPPS VFS security gate (karma/consciousness checks)
 * 2. Companion .steppps files
 * 3. PXFS with embedded STEPPPS (lossless)
 *
 * Build:
 *   gcc -DHOST_BUILD -o steppps_fs_demo \
 *       demos/steppps_fs_demo.c \
 *       kernel/fs/steppps_vfs.c \
 *       fs/pxfs_steppps.c \
 *       -I. -Iinclude
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/tbos/steppps_vfs.h"
#include "fs/pxfs_steppps.h"

/* ========================================================================= */
/* DEMO 1: VFS SECURITY GATE                                                  */
/* ========================================================================= */

static void demo_vfs_security(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  DEMO 1: STEPPPS VFS SECURITY GATE                            ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");

    /* Initialize VFS with MODERATE security */
    steppps_vfs_init(STEPPPS_SECURITY_MODERATE);

    printf("Security Level: %s\n\n",
           steppps_security_level_name(steppps_vfs_get_security_level()));

    /* Test with low karma user */
    printf("--- Test 1: Low karma user (karma=10, consciousness=1) ---\n");
    steppps_caller_t low_karma = {
        .name = "newbie",
        .karma = 10,
        .consciousness = 1,  /* reactive */
        .is_root = false
    };
    steppps_vfs_set_caller(&low_karma);

    printf("Attempting READ on /data/file.txt...\n");
    int rc = steppps_vfs_check("/data/file.txt", STEPPPS_OP_READ);
    printf("  Result: %s\n\n", rc == 0 ? "ALLOWED" : "DENIED");

    printf("Attempting DELETE on /data/file.txt...\n");
    rc = steppps_vfs_check("/data/file.txt", STEPPPS_OP_DELETE);
    printf("  Result: %s (requires consciousness=3)\n\n", rc == 0 ? "ALLOWED (warned)" : "DENIED");

    /* Test with high karma user */
    printf("--- Test 2: High karma user (karma=5000, consciousness=4) ---\n");
    steppps_caller_t high_karma = {
        .name = "sage",
        .karma = 5000,
        .consciousness = 4,  /* mindful */
        .is_root = false
    };
    steppps_vfs_set_caller(&high_karma);

    printf("Attempting DELETE on /data/file.txt...\n");
    rc = steppps_vfs_check("/data/file.txt", STEPPPS_OP_DELETE);
    printf("  Result: %s\n\n", rc == 0 ? "ALLOWED" : "DENIED");

    /* Test STRICT mode */
    printf("--- Test 3: STRICT mode with low karma user ---\n");
    steppps_vfs_set_security_level(STEPPPS_SECURITY_STRICT);
    steppps_vfs_set_caller(&low_karma);

    printf("Attempting WRITE on /data/file.txt...\n");
    rc = steppps_vfs_check("/data/file.txt", STEPPPS_OP_WRITE);
    printf("  Result: %s\n\n", rc == 0 ? "ALLOWED" : "DENIED");

    /* Print stats */
    steppps_vfs_print_stats();
}

/* ========================================================================= */
/* DEMO 2: COMPANION FILES                                                    */
/* ========================================================================= */

static void demo_companion_files(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  DEMO 2: COMPANION .steppps FILES                             ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");

    const char* test_path = "/tmp/test_file.txt";

    /* Generate minimal STEPPPS */
    printf("Generating minimal STEPPPS for: %s\n\n", test_path);

    steppps_minimal_t meta;
    steppps_vfs_generate_minimal(test_path, &meta);

    printf("Auto-generated STEPPPS:\n");
    printf("  ID:           %016llx%016llx\n",
           (unsigned long long)meta.id_high,
           (unsigned long long)meta.id_low);
    printf("  Path:         %s\n", meta.path);
    printf("  Device:       %s\n", meta.device);
    printf("  Created:      %llu\n", (unsigned long long)meta.created);
    printf("  Karma:        %d\n", meta.karma);
    printf("  Consciousness: %d\n", meta.consciousness);
    printf("  Has Rich:     %s\n\n", meta.has_rich_steppps ? "Yes" : "No");

    /* Set required karma */
    printf("Setting required_karma=100 for file...\n");
    meta.required_karma = 100;
    meta.required_consciousness = 2;

    /* Show companion path */
    char companion[512];
    steppps_vfs_get_companion_path(test_path, companion, sizeof(companion));
    printf("Companion file would be: %s\n\n", companion);

    /* Show JSON that would be written */
    printf("Companion JSON content:\n");
    printf("{\n");
    printf("  \"id\": \"%016llx%016llx\",\n",
           (unsigned long long)meta.id_high,
           (unsigned long long)meta.id_low);
    printf("  \"space\": { \"path\": \"%s\", \"device\": \"%s\" },\n",
           meta.path, meta.device);
    printf("  \"security\": {\n");
    printf("    \"required_karma\": %d,\n", meta.required_karma);
    printf("    \"required_consciousness\": %d\n", meta.required_consciousness);
    printf("  }\n");
    printf("}\n");
}

/* ========================================================================= */
/* DEMO 3: PXFS WITH EMBEDDED STEPPPS                                         */
/* ========================================================================= */

static void demo_pxfs_steppps(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  DEMO 3: PXFS WITH EMBEDDED STEPPPS (Lossless)                ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");

    /* Create sample STEPPPS JSON */
    const char* steppps_json =
        "{\n"
        "  \"id\": \"pxfs-demo-001\",\n"
        "  \"name\": \"PXFS Demo File\",\n"
        "  \"space\": { \"universe\": \"earth\", \"device\": \"demo\" },\n"
        "  \"time\": { \"utc\": \"2025-07-03T12:00:00Z\" },\n"
        "  \"psychology\": { \"karma\": 500, \"consciousness\": \"aware\" },\n"
        "  \"script\": { \"lang\": \"sh\", \"code\": \"echo Hello PXFS!\" }\n"
        "}";

    /* Sample content */
    const char* content = "This is the actual file content stored in PXFS pixels.";
    size_t content_len = strlen(content);

    printf("Original STEPPPS JSON (%zu bytes):\n%s\n\n", strlen(steppps_json), steppps_json);
    printf("Original Content (%zu bytes): %s\n\n", content_len, content);

    /* Create PXFS with embedded STEPPPS */
    uint8_t pxfs_buffer[8192];
    size_t pxfs_len = 0;

    int rc = pxfs_create_with_steppps(
        steppps_json,
        (const uint8_t*)content,
        content_len,
        pxfs_buffer,
        sizeof(pxfs_buffer),
        &pxfs_len
    );

    if (rc != 0) {
        printf("Error creating PXFS!\n");
        return;
    }

    printf("Created PXFS file: %zu bytes\n", pxfs_len);
    printf("  Header:  %d bytes\n", PXFS_HEADER_SIZE);
    printf("  STEPPPS: %zu bytes → %zu pixels\n",
           strlen(steppps_json),
           (strlen(steppps_json) + 2) / 3);
    printf("  Content: %zu bytes → %zu pixels\n\n",
           content_len,
           (content_len + 2) / 3);

    /* Display PXFS header info */
    printf("PXFS Header:\n");
    pxfs_print_info(pxfs_buffer, pxfs_len);

    /* Read back and verify lossless */
    printf("\n--- Verifying Lossless Round-Trip ---\n\n");

    char recovered_steppps[4096];
    uint8_t recovered_content[4096];
    size_t recovered_content_len = 0;

    rc = pxfs_read_with_steppps(
        pxfs_buffer,
        pxfs_len,
        recovered_steppps,
        sizeof(recovered_steppps),
        recovered_content,
        sizeof(recovered_content),
        &recovered_content_len
    );

    if (rc != 0) {
        printf("Error reading PXFS!\n");
        return;
    }

    printf("Recovered STEPPPS JSON (%zu bytes):\n%s\n\n",
           strlen(recovered_steppps), recovered_steppps);

    /* Verify STEPPPS is identical */
    bool steppps_match = (strcmp(steppps_json, recovered_steppps) == 0);
    printf("STEPPPS Match: %s\n", steppps_match ? "YES (LOSSLESS)" : "NO (ERROR!)");

    /* Verify content */
    recovered_content[content_len] = '\0';  /* Ensure null termination */
    bool content_match = (memcmp(content, recovered_content, content_len) == 0);
    printf("Content Match: %s\n", content_match ? "YES (LOSSLESS)" : "NO (ERROR!)");

    printf("\n");
    if (steppps_match && content_match) {
        printf("✓ PXFS STEPPPS embedding is LOSSLESS!\n");
    } else {
        printf("✗ Data corruption detected!\n");
    }
}

/* ========================================================================= */
/* DEMO 4: FULL INTEGRATION FLOW                                              */
/* ========================================================================= */

static void demo_full_integration(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║  DEMO 4: FULL INTEGRATION FLOW                                ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n\n");

    printf("STEPPPS + Filesystem Integration Architecture:\n\n");

    printf("┌─────────────────────────────────────────────────────────────┐\n");
    printf("│  User/Process Request                                       │\n");
    printf("│    └──▶ \"Read /data/important.pxfs\"                        │\n");
    printf("└─────────────────────────────────────────────────────────────┘\n");
    printf("                          │\n");
    printf("                          ▼\n");
    printf("┌─────────────────────────────────────────────────────────────┐\n");
    printf("│  STEPPPS VFS Security Gate                                  │\n");
    printf("│    ├── Get caller's STEPPPS (karma, consciousness)          │\n");
    printf("│    ├── Get file's STEPPPS (from companion or PXFS header)   │\n");
    printf("│    ├── Check: caller.karma >= file.required_karma           │\n");
    printf("│    ├── Check: caller.consciousness >= file.required_level   │\n");
    printf("│    └── Audit log entry                                      │\n");
    printf("└─────────────────────────────────────────────────────────────┘\n");
    printf("                          │\n");
    printf("              ┌───────────┴───────────┐\n");
    printf("              │                       │\n");
    printf("              ▼                       ▼\n");
    printf("┌──────────────────────┐  ┌──────────────────────┐\n");
    printf("│  Regular File        │  │  PXFS File           │\n");
    printf("│  + .steppps companion│  │  (STEPPPS in header) │\n");
    printf("│                      │  │  (Content in pixels) │\n");
    printf("└──────────────────────┘  └──────────────────────┘\n");
    printf("\n");

    printf("File Types Supported:\n");
    printf("  • Regular files:  .steppps companion file (JSON)\n");
    printf("  • PXFS files:     STEPPPS embedded in pixel header (lossless)\n");
    printf("  • UCFS files:     STEPPPS compressed alongside content\n");
    printf("  • RF2S files:     STEPPPS via Sangha trust negotiation\n");
    printf("\n");

    printf("Security Levels:\n");
    printf("  • PERMISSIVE: Log only, allow all operations\n");
    printf("  • MODERATE:   Warn on insufficient karma/consciousness\n");
    printf("  • STRICT:     Block if requirements not met\n");
    printf("\n");

    printf("Backward Compatibility:\n");
    printf("  • Files without STEPPPS: Auto-generate minimal metadata\n");
    printf("  • karma=0, consciousness=dormant (anyone can access)\n");
    printf("  • No signature (untrusted)\n");
}

/* ========================================================================= */
/* MAIN                                                                       */
/* ========================================================================= */

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════════════\n");
    printf("       STEPPPS FILESYSTEM INTEGRATION DEMO                         \n");
    printf("═══════════════════════════════════════════════════════════════════\n");

    demo_vfs_security();
    demo_companion_files();
    demo_pxfs_steppps();
    demo_full_integration();

    printf("\n═══════════════════════════════════════════════════════════════════\n");
    printf("       DEMO COMPLETE                                               \n");
    printf("═══════════════════════════════════════════════════════════════════\n\n");

    steppps_vfs_shutdown();
    return 0;
}
