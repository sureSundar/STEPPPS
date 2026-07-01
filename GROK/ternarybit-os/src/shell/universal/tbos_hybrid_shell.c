/**
 * TBOS Hybrid Shell
 * Combines TBOS features with full bash capabilities
 *
 * Features:
 *   - TBOS builtin commands (karma, wow, etc.)
 *   - Transparent bash delegation for complex commands
 *   - Pipe support via bash
 *   - Environment variable sharing
 *   - Persistent karma across shell sessions
 *
 * Build:
 *   gcc -DTBOS_HOSTED -o tbos_hybrid src/shell/universal/tbos_hybrid_shell.c -Iinclude
 *
 * Run:
 *   ./tbos_hybrid
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_INPUT 4096
#define MAX_ARGS  256

/* ═══════════════════════════════════════════════════════════════════════════
 * TBOS STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_running = 1;
static int g_karma = 100;
static char g_cwd[1024];
static char g_consciousness[32] = "Aware";

static void update_consciousness(void) {
    if (g_karma > 150) strcpy(g_consciousness, "Enlightened");
    else if (g_karma > 100) strcpy(g_consciousness, "Aware");
    else if (g_karma > 50) strcpy(g_consciousness, "Awakening");
    else strcpy(g_consciousness, "Learning");
}

static void add_karma(int amount, const char* reason) {
    g_karma += amount;
    update_consciousness();
    if (reason && amount > 0) {
        printf("  +%d karma: %s\n", amount, reason);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOST OS DETECTION
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    HOST_UNKNOWN, HOST_LINUX, HOST_UBUNTU, HOST_MACOS, HOST_WSL
} host_os_t;

static host_os_t g_host_os;

static host_os_t detect_host_os(void) {
#ifdef __APPLE__
    return HOST_MACOS;
#elif defined(__linux__)
    FILE* f = fopen("/etc/os-release", "r");
    if (f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), f)) {
            if (strstr(buf, "Ubuntu")) { fclose(f); return HOST_UBUNTU; }
        }
        fclose(f);
    }
    f = fopen("/proc/version", "r");
    if (f) {
        char buf[256];
        if (fgets(buf, sizeof(buf), f) && strstr(buf, "Microsoft")) {
            fclose(f);
            return HOST_WSL;
        }
        fclose(f);
    }
    return HOST_LINUX;
#else
    return HOST_UNKNOWN;
#endif
}

static const char* host_name(void) {
    switch (g_host_os) {
        case HOST_UBUNTU: return "Ubuntu";
        case HOST_LINUX:  return "Linux";
        case HOST_MACOS:  return "macOS";
        case HOST_WSL:    return "WSL";
        default:          return "Unknown";
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BUILTIN COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║              TBOS Hybrid Shell Commands                       ║\n");
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  TBOS Builtins:                                               ║\n");
    printf("║    help       - Show this help                                ║\n");
    printf("║    exit       - Exit shell                                    ║\n");
    printf("║    karma      - Show karma status                             ║\n");
    printf("║    hostinfo   - Show host system info                         ║\n");
    printf("║    meditate   - Take a mindful break                          ║\n");
    printf("║                                                               ║\n");
    printf("║  Shell Escape:                                                ║\n");
    printf("║    ubuntu     - Enter Ubuntu/bash shell                       ║\n");
    printf("║    bash       - Enter bash shell                              ║\n");
    printf("║                                                               ║\n");
    printf("║  All other commands are passed to bash!                       ║\n");
    printf("║  Pipes, redirects, variables, globbing all work.              ║\n");
    printf("║                                                               ║\n");
    printf("║  Examples:                                                    ║\n");
    printf("║    ls -la | grep '.c'                                         ║\n");
    printf("║    for i in *.txt; do echo $i; done                           ║\n");
    printf("║    cat file.txt | awk '{print $1}'                            ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    return 0;
}

static int cmd_exit(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Exiting TBOS shell... Namaste\n");
    g_running = 0;
    return 0;
}

static int cmd_karma(int argc, char** argv) {
    (void)argc; (void)argv;
    update_consciousness();
    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  TBOS Karma Status                         ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  Karma Balance:   %-24d ║\n", g_karma);
    printf("║  Consciousness:   %-24s ║\n", g_consciousness);
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
    return 0;
}

static int cmd_hostinfo(int argc, char** argv) {
    (void)argc; (void)argv;

    char hostname[256] = "unknown";
    gethostname(hostname, sizeof(hostname));

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Host System Information                   ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  Host OS:     %-28s ║\n", host_name());
    printf("║  Hostname:    %-28s ║\n", hostname);
    printf("║  User:        %-28s ║\n", getenv("USER") ? getenv("USER") : "unknown");
    printf("║  Shell:       %-28s ║\n", getenv("SHELL") ? getenv("SHELL") : "unknown");
    printf("║  TBOS Karma:  %-28d ║\n", g_karma);
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");
    return 0;
}

static int cmd_meditate(int argc, char** argv) {
    (void)argc; (void)argv;
    int duration = 3;
    if (argc > 1) duration = atoi(argv[1]);
    if (duration < 1) duration = 1;
    if (duration > 60) duration = 60;

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  TBOS Meditation                           ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\nTaking a mindful moment...\n\n");

    for (int i = 0; i < duration; i++) {
        printf("  \xE0\xA5\x90 ");  /* Om symbol in UTF-8 */
        fflush(stdout);
        sleep(1);
    }

    printf("\n\nMeditation complete.\n");
    add_karma(10, "Mindful computing practice");
    printf("\n");
    return 0;
}

static int cmd_ubuntu(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Entering Ubuntu/Bash Shell                ║\n");
    printf("║  Type 'exit' to return to TBOS             ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    pid_t pid = fork();
    if (pid == 0) {
        if (g_host_os == HOST_MACOS) {
            execlp("docker", "docker", "run", "-it", "--rm",
                   "ubuntu:latest", "/bin/bash", NULL);
        }
        execl("/bin/bash", "bash", "--login", NULL);
        execl("/bin/sh", "sh", NULL);
        perror("No shell available");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Returned to TBOS Hybrid Shell             ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    add_karma(5, "Explored host system");
    return 0;
}

static int cmd_bash(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\nEntering bash... (type 'exit' to return)\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/bash", "bash", "--login", NULL);
        perror("bash");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS Hybrid Shell\n");
    add_karma(2, "Shell exploration");
    return 0;
}

static int cmd_cd(int argc, char** argv) {
    const char* path = argc > 1 ? argv[1] : getenv("HOME");
    if (chdir(path) != 0) {
        perror("cd");
        return 1;
    }
    getcwd(g_cwd, sizeof(g_cwd));
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND DISPATCH
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* name;
    int (*handler)(int argc, char** argv);
} builtin_t;

static builtin_t builtins[] = {
    {"help",     cmd_help},
    {"?",        cmd_help},
    {"exit",     cmd_exit},
    {"quit",     cmd_exit},
    {"karma",    cmd_karma},
    {"hostinfo", cmd_hostinfo},
    {"meditate", cmd_meditate},
    {"ubuntu",   cmd_ubuntu},
    {"bash",     cmd_bash},
    {"cd",       cmd_cd},
    {NULL,       NULL}
};

static int is_complex_command(const char* line) {
    /* Check if command has shell metacharacters */
    return (strchr(line, '|') != NULL ||
            strchr(line, '>') != NULL ||
            strchr(line, '<') != NULL ||
            strchr(line, '&') != NULL ||
            strchr(line, ';') != NULL ||
            strchr(line, '$') != NULL ||
            strchr(line, '`') != NULL ||
            strchr(line, '*') != NULL ||
            strchr(line, '?') != NULL ||
            strstr(line, "for ") != NULL ||
            strstr(line, "while ") != NULL ||
            strstr(line, "if ") != NULL);
}

static int execute_via_bash(const char* line) {
    /* Pass entire line to bash for execution */
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/bash", "bash", "-c", line, NULL);
        execl("/bin/sh", "sh", "-c", line, NULL);
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }
    return 1;
}

static int execute_command(const char* line, int argc, char** argv) {
    if (argc == 0) return 0;

    /* Check TBOS builtins first */
    for (int i = 0; builtins[i].name; i++) {
        if (strcmp(argv[0], builtins[i].name) == 0) {
            return builtins[i].handler(argc, argv);
        }
    }

    /* If command has shell metacharacters, use bash */
    if (is_complex_command(line)) {
        return execute_via_bash(line);
    }

    /* Try simple external command */
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        /* If execvp fails, try via bash */
        execl("/bin/bash", "bash", "-c", line, NULL);
        printf("tbos: command not found: %s\n", argv[0]);
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PARSER
 * ═══════════════════════════════════════════════════════════════════════════ */

static int parse_simple(char* line, char** argv) {
    int argc = 0;
    char* p = line;

    while (*p && argc < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            argv[argc++] = p;
            while (*p && *p != quote) p++;
            if (*p) *p++ = '\0';
        } else {
            argv[argc++] = p;
            while (*p && *p != ' ' && *p != '\t') p++;
            if (*p) *p++ = '\0';
        }
    }

    argv[argc] = NULL;
    return argc;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN SHELL LOOP
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_banner(void) {
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                               ║\n");
    printf("║   ████████╗██████╗  ██████╗ ███████╗                          ║\n");
    printf("║   ╚══██╔══╝██╔══██╗██╔═══██╗██╔════╝                          ║\n");
    printf("║      ██║   ██████╔╝██║   ██║███████╗                          ║\n");
    printf("║      ██║   ██╔══██╗██║   ██║╚════██║                          ║\n");
    printf("║      ██║   ██████╔╝╚██████╔╝███████║                          ║\n");
    printf("║      ╚═╝   ╚═════╝  ╚═════╝ ╚══════╝                          ║\n");
    printf("║                                                               ║\n");
    printf("║            TernaryBit OS - Hybrid Shell                       ║\n");
    printf("║      Full bash power + Conscious computing                    ║\n");
    printf("║                                                               ║\n");
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  Host: %-54s ║\n", host_name());
    printf("║  Type 'help' for TBOS commands                                ║\n");
    printf("║  All bash commands, pipes, and scripts work!                  ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int main(void) {
    char input[MAX_INPUT];
    char input_copy[MAX_INPUT];
    char* argv[MAX_ARGS];

    /* Initialize */
    g_host_os = detect_host_os();
    getcwd(g_cwd, sizeof(g_cwd));

    /* Banner */
    print_banner();

    /* Main loop */
    while (g_running) {
        /* Prompt with karma indicator */
        const char* karma_color = g_karma > 100 ? "\033[1;32m" :
                                  g_karma > 50 ? "\033[1;33m" : "\033[1;31m";
        printf("\033[1;36mtbos\033[0m:%s[%d]\033[0m:\033[1;34m%s\033[0m$ ",
               karma_color, g_karma, g_cwd);
        fflush(stdout);

        /* Read input */
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }

        /* Remove newline */
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }

        /* Skip empty lines */
        if (input[0] == '\0') continue;

        /* Keep original for bash delegation */
        strcpy(input_copy, input);

        /* Parse for builtin check */
        int argc = parse_simple(input, argv);

        /* Execute */
        execute_command(input_copy, argc, argv);
    }

    printf("\nFinal karma: %d - May your code compile on first try!\n\n", g_karma);
    return 0;
}
