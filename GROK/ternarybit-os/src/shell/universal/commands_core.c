/**
 * @file commands_core.c
 * @brief Core shell commands - Day 1
 *
 * Commands implemented:
 * - Day 1: help
 *
 * @date 2025-11-04
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND IMPLEMENTATIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * Day 1: help command
 * Shows all available commands organized by category
 */
static int cmd_help(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Universal Shell Commands                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("📖 Core Commands (Week 0): 1 command\n");
    printf("  help                   Show this help message\n");
    printf("\n");

    printf("✅ Implemented Command Categories:\n");
    printf("  \033[1m📁 Week 1: File Operations (8)\033[0m     - ls, cd, pwd, cp, mv, rm, mkdir, rmdir\n");
    printf("  \033[1m📝 Week 2: Text Processing (8)\033[0m     - cat, grep, find, head, tail, wc, more, less\n");
    printf("  \033[1m🔐 Week 3: Permissions (8)\033[0m         - chmod, chown, chgrp, touch, ln, stat, du, df\n");
    printf("  \033[1m⚙️  Week 4: Process Management (8)\033[0m  - ps, top, kill, jobs, bg, fg, nice, renice\n");
    printf("  \033[1m📊 Week 5: System Info (8)\033[0m         - System information commands\n");
    printf("  \033[1m✂️  Week 6: Text Transform (7)\033[0m      - cut, paste, join, sort, uniq, tr, fmt\n");
    printf("  \033[1m📐 Week 7: Text Layout (7)\033[0m         - expand, unexpand, col, colrm, column, rev, tac\n");
    printf("  \033[1m🔢 Week 8: Binary/Encoding (7)\033[0m     - strings, od, hexdump, xxd, iconv, dos2unix, unix2dos\n");
    printf("  \033[1m🌐 Week 9: Network Basics (7)\033[0m      - ping, traceroute, curl, wget, nslookup, dig\n");
    printf("  \033[1m🖥️  Week 10: System Info (8)\033[0m        - uname, hostname, date, uptime, whoami, id, groups, cal\n");
    printf("\n");

    printf("⏳ Command Categories (Planned):\n");
    printf("  📦 Week 11+: Advanced Compression  - zip, pxfs enhancements...\n");
    printf("  🕉️  Week 12+: Consciousness         - karma, meditate, sangha...\n");
    printf("  🔧 Week 13+: System Admin          - mount, fsck, systemctl...\n");
    printf("\n");

    printf("🗓️  Implementation Progress:\n");
    printf("  Total Commands: 400\n");
    printf("  Implemented: 76 (19.00%%)\n");
    printf("  Weeks Complete: 10/50\n");
    printf("  Contributors: CC (Weeks 1-4), CX (Weeks 5-10)\n");
    printf("\n");

    printf("📚 Syntax Support:\n");
    printf("  ✓ POSIX/Linux (bash, sh, zsh)\n");
    printf("  ✓ Windows (cmd.exe, PowerShell)\n");
    printf("  ✓ macOS/BSD (zsh, csh)\n");
    printf("  ✓ Auto-translation between syntaxes\n");
    printf("\n");

    printf("💾 Filesystem Integration:\n");
    printf("  ✓ PXFS - Pixel-based compression (4KB → 3 bytes)\n");
    printf("  ✓ UCFS - Universal filesystem\n");
    printf("  ✓ RF2S - Rock-solid storage\n");
    printf("\n");

    printf("🕉️  Consciousness Features:\n");
    printf("  • Karma tracking (current: %ld)\n", (long)universal_get_session()->karma);
    printf("  • Consciousness evolution (level: %d)\n", universal_get_session()->consciousness);
    printf("  • Digital Sangha connectivity\n");
    printf("  • Mindful computing metrics\n");
    printf("\n");

    printf("ℹ️  For detailed help on a specific command:\n");
    printf("  help <command>         (e.g., help ls)\n");
    printf("  <command> --help       (e.g., ls --help)\n");
    printf("  man <command>          (e.g., man grep)\n");
    printf("\n");

    printf("🌟 Special Features:\n");
    printf("  • Cross-platform command translation\n");
    printf("  • Automatic PXFS compression/decompression\n");
    printf("  • Universal path handling (Windows/POSIX/macOS)\n");
    printf("  • Consciousness-aware command execution\n");
    printf("  • Distributed shell via Digital Sangha\n");
    printf("\n");

    printf("॥ तत् सत् ॥ - Truth is One, Paths are Many\n\n");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_core_commands(void) {
    /* Day 1: help command */
    static const universal_command_t help_cmd = {
        .name = "help",
        .aliases = {"?", "h", "--help", "-h", NULL, NULL, NULL, NULL},
        .description = "Display help information about shell commands",
        .usage = "help [command]",
        .examples = "help\n  help ls\n  help --all",
        .handler = cmd_help,
        .os_support = OS_SUPPORT_ALL,
        .category = CMD_CAT_SHELL_BUILTIN,
        .karma_cost = 0,
        .karma_reward = 2,
        .requires_enlightenment = false,
        .requires_filesystem = false,
        .requires_network = false,
        .requires_root = false,
        .is_destructive = false,
        .supports_pipes = true,
        .supports_redirects = true,
        .is_builtin = true
    };

    universal_register_command(&help_cmd);
}
