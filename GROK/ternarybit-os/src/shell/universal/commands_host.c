/**
 * @file commands_host.c
 * @brief Host OS Integration Commands
 *
 * Commands to escape to host shell and graphics:
 *   - ubuntu/bash  : Launch Ubuntu/Linux bash shell
 *   - shell        : Launch default system shell
 *   - startx       : Start graphical desktop
 *   - docker       : Run Ubuntu in Docker container
 */

#include "tbos_universal_shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * HOST DETECTION
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    HOST_UNKNOWN,
    HOST_LINUX,
    HOST_UBUNTU,
    HOST_MACOS,
    HOST_WSL
} host_os_t;

static host_os_t detect_host_os(void) {
#ifdef __APPLE__
    return HOST_MACOS;
#elif defined(__linux__)
    /* Check for Ubuntu */
    FILE* f = fopen("/etc/os-release", "r");
    if (f) {
        char buf[256];
        while (fgets(buf, sizeof(buf), f)) {
            if (strstr(buf, "Ubuntu") || strstr(buf, "ubuntu")) {
                fclose(f);
                return HOST_UBUNTU;
            }
        }
        fclose(f);
    }

    /* Check for WSL */
    f = fopen("/proc/version", "r");
    if (f) {
        char buf[256];
        if (fgets(buf, sizeof(buf), f)) {
            if (strstr(buf, "Microsoft") || strstr(buf, "WSL")) {
                fclose(f);
                return HOST_WSL;
            }
        }
        fclose(f);
    }

    return HOST_LINUX;
#else
    return HOST_UNKNOWN;
#endif
}

static const char* host_os_name(host_os_t os) {
    switch (os) {
        case HOST_UBUNTU: return "Ubuntu";
        case HOST_LINUX:  return "Linux";
        case HOST_MACOS:  return "macOS";
        case HOST_WSL:    return "WSL (Windows Subsystem for Linux)";
        default:          return "Unknown";
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SHELL COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_ubuntu(int argc, char** argv) {
    (void)argc; (void)argv;

    host_os_t host = detect_host_os();

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Entering Ubuntu Shell                     ║\n");
    printf("║  Type 'exit' to return to TBOS             ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    if (host == HOST_UBUNTU || host == HOST_WSL) {
        /* Native Ubuntu or WSL - just exec bash */
        pid_t pid = fork();
        if (pid == 0) {
            execl("/bin/bash", "bash", "--login", NULL);
            perror("exec failed");
            _exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        }
    } else if (host == HOST_MACOS || host == HOST_LINUX) {
        /* Try Docker first */
        printf("Host OS: %s\n", host_os_name(host));
        printf("Attempting to launch Ubuntu via Docker...\n\n");

        pid_t pid = fork();
        if (pid == 0) {
            execlp("docker", "docker", "run", "-it", "--rm",
                   "-v", "/:/host:ro",
                   "-w", "/host",
                   "ubuntu:latest",
                   "/bin/bash", NULL);
            /* If docker fails, try local bash */
            execl("/bin/bash", "bash", "--login", NULL);
            execl("/bin/zsh", "zsh", "--login", NULL);
            perror("No shell available");
            _exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        }
    } else {
        printf("Cannot determine host OS. Trying /bin/bash...\n");
        pid_t pid = fork();
        if (pid == 0) {
            execl("/bin/bash", "bash", NULL);
            execl("/bin/sh", "sh", NULL);
            _exit(1);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        }
    }

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Returned to TBOS Shell                    ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    universal_add_karma(5, "Explored host system");
    return 0;
}

static int cmd_shell(int argc, char** argv) {
    (void)argc; (void)argv;

    const char* shell = getenv("SHELL");
    if (!shell) shell = "/bin/sh";

    printf("\n");
    printf("Launching system shell: %s\n", shell);
    printf("Type 'exit' to return to TBOS\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        execl(shell, shell, "--login", NULL);
        execl("/bin/sh", "sh", NULL);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS Shell\n");
    return 0;
}

static int cmd_bash(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\nLaunching bash... (type 'exit' to return)\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/bash", "bash", "--login", NULL);
        perror("bash not found");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS Shell\n");
    return 0;
}

static int cmd_zsh(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\nLaunching zsh... (type 'exit' to return)\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/zsh", "zsh", "--login", NULL);
        execl("/usr/bin/zsh", "zsh", "--login", NULL);
        perror("zsh not found");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS Shell\n");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * GRAPHICS COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_startx(int argc, char** argv) {
    (void)argc; (void)argv;

    host_os_t host = detect_host_os();

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Starting Graphical Session                ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    pid_t pid = fork();
    if (pid == 0) {
        if (host == HOST_MACOS) {
            /* macOS - open new Terminal or launch app */
            execlp("open", "open", "-a", "Terminal", NULL);
        } else if (host == HOST_UBUNTU || host == HOST_LINUX) {
            /* Try various display managers */
            execlp("startx", "startx", NULL);
            execlp("gnome-session", "gnome-session", NULL);
            execlp("startkde", "startkde", NULL);
            execlp("startxfce4", "startxfce4", NULL);
            execlp("i3", "i3", NULL);
        } else if (host == HOST_WSL) {
            /* WSL with WSLg */
            printf("WSL detected. Use 'wsl --shutdown' and restart for GUI.\n");
            printf("Or install: sudo apt install ubuntu-desktop\n");
            _exit(0);
        }
        perror("Could not start graphical session");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}

static int cmd_desktop(int argc, char** argv) {
    (void)argc; (void)argv;

    host_os_t host = detect_host_os();

    printf("Starting desktop environment...\n");

    pid_t pid = fork();
    if (pid == 0) {
        if (host == HOST_UBUNTU || host == HOST_LINUX) {
            /* Try GNOME first, then others */
            execlp("gnome-shell", "gnome-shell", NULL);
            execlp("plasmashell", "plasmashell", NULL);
            execlp("xfce4-session", "xfce4-session", NULL);
        }
        printf("No desktop environment found.\n");
        printf("Install with: sudo apt install ubuntu-desktop\n");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SYSTEM INFO
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_hostinfo(int argc, char** argv) {
    (void)argc; (void)argv;

    host_os_t host = detect_host_os();

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Host System Information                   ║\n");
    printf("╠════════════════════════════════════════════╣\n");
    printf("║  Host OS:     %-28s ║\n", host_os_name(host));

    /* Get more info */
    char hostname[256] = "unknown";
    gethostname(hostname, sizeof(hostname));
    printf("║  Hostname:    %-28s ║\n", hostname);

    const char* user = getenv("USER");
    printf("║  User:        %-28s ║\n", user ? user : "unknown");

    const char* shell = getenv("SHELL");
    printf("║  Shell:       %-28s ║\n", shell ? shell : "unknown");

    const char* home = getenv("HOME");
    printf("║  Home:        %-28s ║\n", home ? home : "unknown");

    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    /* Show /etc/os-release on Linux */
    if (host == HOST_UBUNTU || host == HOST_LINUX || host == HOST_WSL) {
        printf("OS Release:\n");
        FILE* f = fopen("/etc/os-release", "r");
        if (f) {
            char buf[256];
            while (fgets(buf, sizeof(buf), f)) {
                if (strncmp(buf, "PRETTY_NAME", 11) == 0 ||
                    strncmp(buf, "VERSION", 7) == 0) {
                    printf("  %s", buf);
                }
            }
            fclose(f);
        }
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DOCKER INTEGRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_docker_ubuntu(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n");
    printf("╔════════════════════════════════════════════╗\n");
    printf("║  Ubuntu Container (Docker)                 ║\n");
    printf("║  Type 'exit' to return to TBOS             ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    printf("\n");

    printf("Pulling Ubuntu image if needed...\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        execlp("docker", "docker", "run", "-it", "--rm",
               "--name", "tbos-ubuntu",
               "-v", "/:/host:ro",
               "-w", "/root",
               "ubuntu:latest",
               "/bin/bash", NULL);
        perror("Docker not available");
        printf("\nInstall Docker: https://docs.docker.com/get-docker/\n");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS Shell\n");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_host_commands(void) {
    /* Shell escape commands */
    universal_shell_register_command(
        "ubuntu", cmd_ubuntu,
        CMD_CAT_SHELL_BUILTIN, OS_SUPPORT_ALL,
        "enter Ubuntu shell",
        "ubuntu\n"
        "Launch Ubuntu bash shell (native or Docker).\n"
        "Type 'exit' to return to TBOS.");

    universal_shell_register_command(
        "shell", cmd_shell,
        CMD_CAT_SHELL_BUILTIN, OS_SUPPORT_ALL,
        "enter system shell",
        "shell\n"
        "Launch default system shell ($SHELL).");

    universal_shell_register_command(
        "bash", cmd_bash,
        CMD_CAT_SHELL_BUILTIN, OS_SUPPORT_ALL,
        "enter bash shell",
        "bash\n"
        "Launch bash shell.");

    universal_shell_register_command(
        "zsh", cmd_zsh,
        CMD_CAT_SHELL_BUILTIN, OS_SUPPORT_ALL,
        "enter zsh shell",
        "zsh\n"
        "Launch zsh shell.");

    /* Graphics commands */
    universal_shell_register_command(
        "startx", cmd_startx,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "start graphical session",
        "startx\n"
        "Start X11/graphical desktop session.");

    universal_shell_register_command(
        "desktop", cmd_desktop,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "start desktop environment",
        "desktop\n"
        "Launch desktop environment (GNOME, KDE, XFCE).");

    /* System info */
    universal_shell_register_command(
        "hostinfo", cmd_hostinfo,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "show host system info",
        "hostinfo\n"
        "Display information about the host operating system.");

    /* Docker integration */
    universal_shell_register_command(
        "docker-ubuntu", cmd_docker_ubuntu,
        CMD_CAT_SHELL_BUILTIN, OS_SUPPORT_ALL,
        "run Ubuntu in Docker",
        "docker-ubuntu\n"
        "Launch Ubuntu container via Docker.");
}
