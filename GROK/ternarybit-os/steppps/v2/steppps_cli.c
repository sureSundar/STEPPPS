/**
 * steppps_cli.c - STEPPPS Command Line Interface
 *
 * Usage:
 *   steppps show <file.steppps.json>     - Display STEPPPS info
 *   steppps run <file.steppps.json>      - Execute STEPPPS
 *   steppps verify <file.steppps.json>   - Verify signature
 *   steppps prompt <file.steppps.json>   - Show AI prompt
 *   steppps audit                        - Show audit log
 *   steppps blacklist <id|hash>          - Blacklist a STEPPPS
 *
 * Build:
 *   gcc -o steppps steppps_cli.c steppps_runtime.c -DHOST_BUILD
 */

#include "steppps.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* prog) {
    printf("STEPPPS v%s - Simple JSON + Smart Secure Runtime\n\n", STEPPPS_VERSION);
    printf("Usage: %s <command> [args]\n\n", prog);
    printf("Commands:\n");
    printf("  show <file>      Display STEPPPS info\n");
    printf("  run <file>       Execute STEPPPS (with security checks)\n");
    printf("  verify <file>    Verify signature and trust\n");
    printf("  prompt <file>    Show AI prompt\n");
    printf("  pixel <file>     Render pixel output\n");
    printf("  audit            Show audit log\n");
    printf("  blacklist <id>   Blacklist a STEPPPS\n");
    printf("\nSecurity:\n");
    printf("  - Unsigned STEPPPS limited to SANDBOX_READ\n");
    printf("  - Signed STEPPPS unlock higher sandbox levels based on author karma\n");
    printf("  - All executions logged to ~/.tbos/steppps_audit.log\n");
    printf("\nSandbox Levels:\n");
    printf("  0 DISPLAY  - View only, no execution\n");
    printf("  1 PURE     - Pure computation, no I/O\n");
    printf("  2 READ     - Read local files (declared paths)\n");
    printf("  3 WRITE    - Write local files (declared paths)\n");
    printf("  4 NETWORK  - Network access (declared hosts)\n");
    printf("  5 SYSTEM   - System commands (declared cmds)\n");
    printf("  6 TRUSTED  - Unrestricted (requires karma >= 50000)\n");
}

static int cmd_show(const char* path) {
    steppps_t s;
    if (steppps_load(path, &s) != 0) {
        fprintf(stderr, "Error: Failed to load %s\n", path);
        return 1;
    }

    steppps_display(&s);
    steppps_free(&s);
    return 0;
}

static int cmd_run(const char* path) {
    steppps_t s;
    if (steppps_load(path, &s) != 0) {
        fprintf(stderr, "Error: Failed to load %s\n", path);
        return 1;
    }

    steppps_display(&s);

    int rc = steppps_run(&s);

    printf("\n--- Exit code: %d ---\n", rc);
    steppps_free(&s);
    return rc;
}

static int cmd_verify(const char* path) {
    steppps_t s;
    if (steppps_load(path, &s) != 0) {
        fprintf(stderr, "Error: Failed to load %s\n", path);
        return 1;
    }

    printf("STEPPPS: %s\n", s.id);
    printf("Hash:    %s\n", s.security.hash);

    if (steppps_verify(&s) != 0) {
        printf("Status:  FAILED (blacklisted or invalid)\n");
        steppps_free(&s);
        return 1;
    }

    printf("Signed:  %s\n", s.security.is_signed ? "Yes" : "No");
    if (s.security.is_signed) {
        printf("Author:  %s\n", s.security.author);
        printf("Karma:   %lld\n", (long long)s.security.author_karma);
        printf("Sig OK:  %s\n", s.security.sig_valid ? "Yes" : "No");
    }
    printf("Max Sandbox: %s\n", steppps_sandbox_name(s.security.max_sandbox));
    printf("Safe:    %s\n", steppps_is_safe(&s) ? "Yes" : "No");

    steppps_free(&s);
    return 0;
}

static int cmd_prompt(const char* path) {
    steppps_t s;
    if (steppps_load(path, &s) != 0) {
        fprintf(stderr, "Error: Failed to load %s\n", path);
        return 1;
    }

    printf("\n╔═══════════════════════════════════════════════════════════════════╗\n");
    printf("║  STEPPPS AI PROMPT                                                ║\n");
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");
    printf("║  ID: %-60s  ║\n", s.id);
    printf("╠═══════════════════════════════════════════════════════════════════╣\n");

    if (s.prompt.system[0]) {
        printf("║  SYSTEM:                                                          ║\n");
        printf("║  %-.65s  ║\n", s.prompt.system);
    }

    if (s.prompt.user[0]) {
        printf("╠═══════════════════════════════════════════════════════════════════╣\n");
        printf("║  USER:                                                            ║\n");
        printf("║  %-.65s  ║\n", s.prompt.user);
    }

    if (s.prompt.next[0]) {
        printf("╠═══════════════════════════════════════════════════════════════════╣\n");
        printf("║  NEXT:                                                            ║\n");
        printf("║  %-.65s  ║\n", s.prompt.next);
    }

    if (s.prompt.interaction[0]) {
        printf("╠═══════════════════════════════════════════════════════════════════╣\n");
        printf("║  INTERACTION:                                                     ║\n");
        printf("║  %-.65s  ║\n", s.prompt.interaction);
    }

    printf("╚═══════════════════════════════════════════════════════════════════╝\n");

    steppps_free(&s);
    return 0;
}

static int cmd_pixel(const char* path) {
    steppps_t s;
    if (steppps_load(path, &s) != 0) {
        fprintf(stderr, "Error: Failed to load %s\n", path);
        return 1;
    }

    steppps_render(&s);
    steppps_free(&s);
    return 0;
}

static int cmd_audit(void) {
    const char* home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Cannot determine home directory\n");
        return 1;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/.tbos/steppps_audit.log", home);

    FILE* f = fopen(path, "r");
    if (!f) {
        printf("No audit log found at %s\n", path);
        return 0;
    }

    printf("STEPPPS Audit Log\n");
    printf("═════════════════════════════════════════════════════════════════════\n");
    printf("%-20s %-20s %-10s %-8s\n", "TIMESTAMP", "ID", "ACTION", "RESULT");
    printf("─────────────────────────────────────────────────────────────────────\n");

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char timestamp[32], id[128], hash[128], action[32], author[64];
        int result;
        long long caps;

        if (sscanf(line, "%31[^|]|%127[^|]|%127[^|]|%31[^|]|%63[^|]|%d|%lld",
                   timestamp, id, hash, action, author, &result, &caps) >= 6) {
            printf("%-20s %-20.20s %-10s %-8d\n", timestamp, id, action, result);
        }
    }

    fclose(f);
    return 0;
}

static int cmd_blacklist(const char* id_or_hash) {
    const char* home = getenv("HOME");
    if (!home) {
        fprintf(stderr, "Cannot determine home directory\n");
        return 1;
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/.tbos/steppps_blacklist", home);

    FILE* f = fopen(path, "a");
    if (!f) {
        fprintf(stderr, "Cannot open blacklist file\n");
        return 1;
    }

    fprintf(f, "%s\n", id_or_hash);
    fclose(f);

    printf("Added to blacklist: %s\n", id_or_hash);
    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 0;
    }

    /* Initialize runtime */
    steppps_runtime_init();

    int rc = 0;
    const char* cmd = argv[1];

    if (strcmp(cmd, "show") == 0 && argc > 2) {
        rc = cmd_show(argv[2]);
    }
    else if (strcmp(cmd, "run") == 0 && argc > 2) {
        rc = cmd_run(argv[2]);
    }
    else if (strcmp(cmd, "verify") == 0 && argc > 2) {
        rc = cmd_verify(argv[2]);
    }
    else if (strcmp(cmd, "prompt") == 0 && argc > 2) {
        rc = cmd_prompt(argv[2]);
    }
    else if (strcmp(cmd, "pixel") == 0 && argc > 2) {
        rc = cmd_pixel(argv[2]);
    }
    else if (strcmp(cmd, "audit") == 0) {
        rc = cmd_audit();
    }
    else if (strcmp(cmd, "blacklist") == 0 && argc > 2) {
        rc = cmd_blacklist(argv[2]);
    }
    else {
        print_usage(argv[0]);
        rc = 1;
    }

    steppps_runtime_shutdown();
    return rc;
}
