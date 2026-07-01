/**
 * Test host shell integration
 *
 * Build & run:
 *   gcc -DTBOS_HOSTED -o test_host tests/test_host_shell.c -Iinclude && ./test_host
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/* Detect host OS */
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
        case HOST_WSL:    return "WSL";
        default:          return "Unknown";
    }
}

int main(void) {
    printf("\n");
    printf("═══════════════════════════════════════════════════════════\n");
    printf("  TBOS Host Shell Integration Test\n");
    printf("═══════════════════════════════════════════════════════════\n\n");

    host_os_t host = detect_host_os();
    printf("1. Detected Host OS: %s\n", host_os_name(host));

    /* Check available shells */
    printf("\n2. Available shells:\n");

    const char* shells[] = {"/bin/bash", "/bin/zsh", "/bin/sh", NULL};
    for (int i = 0; shells[i]; i++) {
        if (access(shells[i], X_OK) == 0) {
            printf("   ✅ %s\n", shells[i]);
        } else {
            printf("   ❌ %s (not found)\n", shells[i]);
        }
    }

    /* Check Docker */
    printf("\n3. Docker availability:\n");
    int docker_available = (system("docker --version > /dev/null 2>&1") == 0);
    if (docker_available) {
        printf("   ✅ Docker installed\n");

        int docker_running = (system("docker info > /dev/null 2>&1") == 0);
        if (docker_running) {
            printf("   ✅ Docker daemon running\n");
        } else {
            printf("   ⚠️  Docker daemon not running\n");
        }
    } else {
        printf("   ❌ Docker not installed\n");
    }

    /* Check environment */
    printf("\n4. Environment:\n");
    printf("   SHELL: %s\n", getenv("SHELL") ? getenv("SHELL") : "(not set)");
    printf("   USER:  %s\n", getenv("USER") ? getenv("USER") : "(not set)");
    printf("   HOME:  %s\n", getenv("HOME") ? getenv("HOME") : "(not set)");

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        printf("   HOST:  %s\n", hostname);
    }

    /* Test shell invocation */
    printf("\n5. Quick shell test (echo 'hello from bash'):\n");
    fflush(stdout);

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/bash", "bash", "-c", "echo '   Output: hello from bash'", NULL);
        execl("/bin/sh", "sh", "-c", "echo '   Output: hello from sh'", NULL);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            printf("   ✅ Shell execution works\n");
        }
    }

    printf("\n═══════════════════════════════════════════════════════════\n");
    printf("  Recommendation:\n");

    if (host == HOST_UBUNTU || host == HOST_WSL) {
        printf("  You're on %s - native Ubuntu shell available!\n", host_os_name(host));
        printf("  Use 'ubuntu' command to enter bash.\n");
    } else if (docker_available) {
        printf("  Use 'ubuntu' or 'docker-ubuntu' for Ubuntu shell.\n");
        printf("  Run: ./scripts/setup_ubuntu.sh for full setup.\n");
    } else {
        printf("  Install Docker for Ubuntu container support:\n");
        printf("  https://docs.docker.com/get-docker/\n");
    }

    printf("═══════════════════════════════════════════════════════════\n\n");

    return 0;
}
