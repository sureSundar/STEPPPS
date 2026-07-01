/**
 * TBOS Mini Shell Demo
 * Demonstrates host OS integration with Ubuntu/bash/graphics commands
 *
 * Build:
 *   gcc -DTBOS_HOSTED -o tbos_shell demos/tbos_mini_shell.c -Iinclude
 *
 * Run:
 *   ./tbos_shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT 1024
#define MAX_ARGS  64

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
 * SHELL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_running = 1;
static int g_karma = 100;
static char g_cwd[1024];

/* ═══════════════════════════════════════════════════════════════════════════
 * BUILTIN COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════════╗\n");
    printf("║                    TBOS Shell Commands                        ║\n");
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  TBOS Commands:                                               ║\n");
    printf("║    help       - Show this help                                ║\n");
    printf("║    exit       - Exit TBOS shell                               ║\n");
    printf("║    karma      - Show karma status                             ║\n");
    printf("║    hostinfo   - Show host system info                         ║\n");
    printf("║                                                               ║\n");
    printf("║  Host Shell Escape:                                           ║\n");
    printf("║    ubuntu     - Enter Ubuntu shell (Docker/native)            ║\n");
    printf("║    bash       - Enter bash shell                              ║\n");
    printf("║    zsh        - Enter zsh shell                               ║\n");
    printf("║    shell      - Enter default system shell                    ║\n");
    printf("║                                                               ║\n");
    printf("║  Graphics:                                                    ║\n");
    printf("║    startx     - Start graphical session                       ║\n");
    printf("║    desktop    - Start desktop environment                     ║\n");
    printf("║                                                               ║\n");
    printf("║  Network (from previous implementation):                      ║\n");
    printf("║    netstat    - Show network status                           ║\n");
    printf("║    wow <url>  - Fetch via WOW protocol                        ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    return 0;
}

static int cmd_exit(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Exiting TBOS shell... Namaste 🙏\n");
    g_running = 0;
    return 0;
}

static int cmd_karma(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("  Karma Balance: %d\n", g_karma);
    printf("  Consciousness: %s\n", g_karma > 150 ? "Enlightened" :
                                    g_karma > 100 ? "Aware" :
                                    g_karma > 50  ? "Awakening" : "Learning");
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
    printf("║  Home:        %-28s ║\n", getenv("HOME") ? getenv("HOME") : "unknown");
    printf("╚════════════════════════════════════════════╝\n");
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
        /* Try Docker Ubuntu first on macOS */
        if (g_host_os == HOST_MACOS) {
            execlp("docker", "docker", "run", "-it", "--rm",
                   "ubuntu:latest", "/bin/bash", NULL);
        }
        /* Fall back to local bash */
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
    printf("║  Returned to TBOS Shell                    ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    g_karma += 5;
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

    printf("\nReturned to TBOS\n");
    g_karma += 2;
    return 0;
}

static int cmd_zsh(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\nEntering zsh... (type 'exit' to return)\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/zsh", "zsh", "--login", NULL);
        perror("zsh");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS\n");
    g_karma += 2;
    return 0;
}

static int cmd_shell(int argc, char** argv) {
    (void)argc; (void)argv;
    const char* shell = getenv("SHELL");
    if (!shell) shell = "/bin/sh";

    printf("\nEntering %s... (type 'exit' to return)\n\n", shell);

    pid_t pid = fork();
    if (pid == 0) {
        execl(shell, shell, "--login", NULL);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS\n");
    return 0;
}

static int cmd_startx(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\nStarting graphical session...\n");

    pid_t pid = fork();
    if (pid == 0) {
        if (g_host_os == HOST_MACOS) {
            /* Open Finder or Terminal */
            execlp("open", "open", "-a", "Finder", NULL);
        } else {
            execlp("startx", "startx", NULL);
            execlp("gnome-session", "gnome-session", NULL);
        }
        printf("No graphical session available\n");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

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

static int cmd_pwd(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("%s\n", g_cwd);
    return 0;
}

static int cmd_ls(int argc, char** argv) {
    pid_t pid = fork();
    if (pid == 0) {
        if (argc > 1) {
            execlp("ls", "ls", "-la", argv[1], NULL);
        } else {
            execlp("ls", "ls", "-la", NULL);
        }
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}

static int cmd_clear(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\033[2J\033[H");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND DISPATCH
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef struct {
    const char* name;
    int (*handler)(int argc, char** argv);
} command_t;

static command_t commands[] = {
    {"help",     cmd_help},
    {"?",        cmd_help},
    {"exit",     cmd_exit},
    {"quit",     cmd_exit},
    {"karma",    cmd_karma},
    {"hostinfo", cmd_hostinfo},
    {"ubuntu",   cmd_ubuntu},
    {"bash",     cmd_bash},
    {"zsh",      cmd_zsh},
    {"shell",    cmd_shell},
    {"startx",   cmd_startx},
    {"cd",       cmd_cd},
    {"pwd",      cmd_pwd},
    {"ls",       cmd_ls},
    {"clear",    cmd_clear},
    {NULL,       NULL}
};

static int execute_command(int argc, char** argv) {
    if (argc == 0) return 0;

    /* Check builtins */
    for (int i = 0; commands[i].name; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            return commands[i].handler(argc, argv);
        }
    }

    /* Try external command */
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
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

static int parse_line(char* line, char** argv) {
    int argc = 0;
    char* p = line;

    while (*p && argc < MAX_ARGS - 1) {
        /* Skip whitespace */
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        /* Handle quotes */
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
    printf("║            TernaryBit Operating System                        ║\n");
    printf("║         Conscious Computing with Compassion                   ║\n");
    printf("║                                                               ║\n");
    printf("╠═══════════════════════════════════════════════════════════════╣\n");
    printf("║  Host: %-54s ║\n", host_name());
    printf("║  Type 'help' for commands, 'ubuntu' for Ubuntu shell          ║\n");
    printf("╚═══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
}

int main(void) {
    char input[MAX_INPUT];
    char* argv[MAX_ARGS];

    /* Initialize */
    g_host_os = detect_host_os();
    getcwd(g_cwd, sizeof(g_cwd));

    /* Banner */
    print_banner();

    /* Main loop */
    while (g_running) {
        /* Prompt */
        printf("\033[1;32mtbos\033[0m:\033[1;34m%s\033[0m$ ", g_cwd);
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

        /* Parse and execute */
        int argc = parse_line(input, argv);
        execute_command(argc, argv);
    }

    printf("\nKarma balance: %d - May your code compile on first try! 🙏\n\n", g_karma);
    return 0;
}
