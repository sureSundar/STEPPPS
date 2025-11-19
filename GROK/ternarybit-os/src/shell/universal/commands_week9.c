/**
 * @file commands_week9.c
 * @brief Week 9 Network Utilities (Days 57-63)
 *
 * Commands:
 *  - ping       : Send ICMP echo requests via system ping
 *  - ping6      : IPv6 ping (falls back to ping -6)
 *  - traceroute : Trace network route (best-effort wrapper)
 *  - curl       : Transfer data from URLs (wrapper)
 *  - wget       : Retrieve files via HTTP/FTP (wrapper)
 *  - nslookup   : Query DNS records
 *  - dig        : Detailed DNS lookup
 *
 * These commands delegate to host system utilities when available.
 * Graceful messages are shown if the binary is missing or execution fails.
 */

#include "tbos_universal_shell.h"

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * GENERIC EXTERNAL COMMAND RUNNER
 * ═══════════════════════════════════════════════════════════════════════════ */

static int run_external_command(const char* program, int argc, char** argv) {
    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "%s: fork failed: %s\n", program, strerror(errno));
        return 1;
    }

    if (pid == 0) {
        /* Child */
        char** args = malloc(sizeof(char*) * (argc + 1));
        if (!args) {
            fprintf(stderr, "%s: out of memory\n", program);
            _exit(1);
        }
        args[0] = (char*)program;
        for (int i = 1; i < argc; i++) {
            args[i] = argv[i];
        }
        args[argc] = NULL;
        execvp(program, args);
        fprintf(stderr, "%s: %s\n", program, strerror(errno));
        _exit(127);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) < 0) {
        fprintf(stderr, "%s: waitpid failed: %s\n", program, strerror(errno));
        return 1;
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        if (code == 0) {
            universal_add_karma(1, "Reached out mindfully");
            return 0;
        }
        return code;
    }

    if (WIFSIGNALED(status)) {
        fprintf(stderr, "%s terminated by signal %d\n", program, WTERMSIG(status));
    }
    return 1;
}

static int run_external_or_message(const char* program,
                                   int argc,
                                   char** argv,
                                   const char* guidance) {
    int result = run_external_command(program, argc, argv);
    if (result == 127) {
        printf("%s not available on this system.\n", program);
        if (guidance) {
            printf("%s\n", guidance);
        }
        universal_add_karma(0, "Network tool unavailable");
        return 1;
    }
    return result;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * NETWORK COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_ping(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ping <host> [options]\n");
        return 1;
    }
    return run_external_or_message("ping", argc, argv,
                                   "Install iputils-ping to enable this command.");
}

static int cmd_ping6(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ping6 <host> [options]\n");
        return 1;
    }

    int result = run_external_command("ping6", argc, argv);
    if (result == 127) {
        /* fallback to ping -6 */
        int new_argc = argc + 1;
        char** new_argv = malloc(sizeof(char*) * (new_argc));
        if (!new_argv) {
            fprintf(stderr, "ping6: out of memory\n");
            return 1;
        }
        new_argv[0] = argv[0];
        new_argv[1] = "-6";
        for (int i = 1; i < argc; i++) {
            new_argv[i + 1] = argv[i];
        }
        int fallback = run_external_or_message("ping", new_argc, new_argv,
                                               "Install iputils-ping to enable IPv6 ping.");
        free(new_argv);
        return fallback;
    }
    return result;
}

static int cmd_traceroute(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: traceroute <host> [options]\n");
        return 1;
    }
    return run_external_or_message("traceroute", argc, argv,
                                   "Install traceroute to trace network routes.");
}

static int cmd_curl(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: curl <url> [options]\n");
        return 1;
    }
    return run_external_or_message("curl", argc, argv,
                                   "Install curl to enable HTTP transfers.");
}

static int cmd_wget(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: wget <url> [options]\n");
        return 1;
    }
    return run_external_or_message("wget", argc, argv,
                                   "Install wget to enable file downloads.");
}

static int cmd_nslookup(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: nslookup <host>\n");
        return 1;
    }
    return run_external_or_message("nslookup", argc, argv,
                                   "Install dnsutils/bind-tools for nslookup.");
}

static int cmd_dig(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: dig <host> [record]\n");
        return 1;
    }
    return run_external_or_message("dig", argc, argv,
                                   "Install dnsutils/bind-tools for dig.");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 9 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week9_commands(void) {
    universal_shell_register_command(
        "ping", cmd_ping,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "send ICMP echo requests",
        "ping <host> [options]\n"
        "Send ICMP echo requests via system ping.");

    universal_shell_register_command(
        "ping6", cmd_ping6,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "send ICMPv6 echo requests",
        "ping6 <host> [options]\n"
        "Send IPv6 ICMP echo requests.");

    universal_shell_register_command(
        "traceroute", cmd_traceroute,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "trace route to host",
        "traceroute <host> [options]\n"
        "Display route packets take to network host.");

    universal_shell_register_command(
        "curl", cmd_curl,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "transfer data from URLs",
        "curl <url> [options]\n"
        "HTTP/FTP transfer via curl if available.");

    universal_shell_register_command(
        "wget", cmd_wget,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "retrieve files via HTTP/FTP",
        "wget <url> [options]\n"
        "Download files using wget when available.");

    universal_shell_register_command(
        "nslookup", cmd_nslookup,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "query DNS records",
        "nslookup <host>\n"
        "Simple DNS lookup using nslookup.");

    universal_shell_register_command(
        "dig", cmd_dig,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "perform DNS queries",
        "dig <host> [type]\n"
        "Detailed DNS lookup using dig.");
}
