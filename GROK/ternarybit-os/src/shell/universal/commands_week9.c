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
 *  - netstat    : Show active network connections (TBOS native)
 *  - nc         : Netcat-like TCP/UDP tool (TBOS native)
 *  - http       : Simple HTTP client (TBOS native)
 *  - serve      : Simple HTTP server (TBOS native)
 *  - wow        : WOW universal protocol fetch (TBOS native)
 *
 * These commands delegate to host system utilities when available.
 * Graceful messages are shown if the binary is missing or execution fails.
 */

#include "tbos_universal_shell.h"
#include "tbos/net.h"
#include "tbos/wow.h"

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
 * TBOS NATIVE NETWORK COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_netstat(int argc, char** argv) {
    (void)argc;
    (void)argv;

    uint64_t sent, recv;
    int active;
    tbos_net_stats(&sent, &recv, &active);

    char local_ip[64];
    tbos_net_get_local_ip(local_ip, sizeof(local_ip));

    printf("\nTBOS Network Status\n");
    printf("═══════════════════════════════════════\n");
    printf("Local IP:        %s\n", local_ip);
    printf("Active sockets:  %d\n", active);
    printf("Bytes sent:      %lu\n", sent);
    printf("Bytes received:  %lu\n", recv);
    printf("═══════════════════════════════════════\n\n");

    universal_add_karma(1, "Network status check");
    return 0;
}

static int cmd_nc(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: nc [-l] <host> <port>\n");
        printf("       nc -l <port>        Listen on port\n");
        printf("       nc <host> <port>    Connect to host:port\n");
        return 1;
    }

    int listen_mode = 0;
    const char* host = NULL;
    uint16_t port = 0;
    int arg_idx = 1;

    /* Parse options */
    if (strcmp(argv[arg_idx], "-l") == 0) {
        listen_mode = 1;
        arg_idx++;
    }

    if (listen_mode) {
        if (arg_idx >= argc) {
            printf("nc: port required for listen mode\n");
            return 1;
        }
        port = (uint16_t)atoi(argv[arg_idx]);
    } else {
        if (arg_idx + 1 >= argc) {
            printf("nc: host and port required\n");
            return 1;
        }
        host = argv[arg_idx];
        port = (uint16_t)atoi(argv[arg_idx + 1]);
    }

    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (sock < 0) {
        printf("nc: failed to create socket\n");
        return 1;
    }

    if (listen_mode) {
        if (tbos_net_bind(sock, port) != 0) {
            printf("nc: failed to bind to port %u\n", port);
            tbos_net_close(sock);
            return 1;
        }
        if (tbos_net_listen(sock, 1) != 0) {
            printf("nc: failed to listen\n");
            tbos_net_close(sock);
            return 1;
        }
        printf("Listening on port %u...\n", port);

        int client = tbos_net_accept(sock);
        if (client < 0) {
            printf("nc: accept failed\n");
            tbos_net_close(sock);
            return 1;
        }
        printf("Connection accepted\n");

        /* Echo received data */
        char buf[1024];
        ssize_t n;
        while ((n = tbos_net_recv(client, buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }

        tbos_net_close(client);
        tbos_net_close(sock);
    } else {
        if (tbos_net_connect(sock, host, port) != 0) {
            printf("nc: failed to connect to %s:%u\n", host, port);
            tbos_net_close(sock);
            return 1;
        }
        printf("Connected to %s:%u\n", host, port);

        /* Simple send/recv loop - send stdin, print response */
        char buf[1024];
        ssize_t n;
        while ((n = tbos_net_recv(sock, buf, sizeof(buf) - 1)) > 0) {
            buf[n] = '\0';
            printf("%s", buf);
            fflush(stdout);
        }

        tbos_net_close(sock);
    }

    universal_add_karma(2, "Network connection");
    return 0;
}

static int cmd_http(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: http <get|post> <url>\n");
        printf("       http get http://example.com/path\n");
        return 1;
    }

    const char* method = argv[1];
    const char* url = argv[2];

    if (strcmp(method, "get") != 0 && strcmp(method, "GET") != 0) {
        printf("http: only GET method supported currently\n");
        return 1;
    }

    /* Use wow:// if not already prefixed */
    char wow_url[2048];
    if (strncmp(url, "wow://", 6) == 0) {
        strncpy(wow_url, url, sizeof(wow_url) - 1);
    } else {
        snprintf(wow_url, sizeof(wow_url), "wow://%s", url);
    }

    wow_response_t response;
    if (wow_fetch(wow_url, &response) != WOW_SUCCESS) {
        printf("http: request failed\n");
        return 1;
    }

    printf("HTTP %d\n", response.status);
    if (response.data) {
        printf("%s\n", (char*)response.data);
    }

    universal_add_karma(response.karma, "HTTP request");
    wow_response_free(&response);
    return 0;
}

static int cmd_serve(int argc, char** argv) {
    uint16_t port = 8080;

    if (argc >= 2) {
        port = (uint16_t)atoi(argv[1]);
    }

    int sock = tbos_net_socket(TBOS_SOCK_TCP);
    if (sock < 0) {
        printf("serve: failed to create socket\n");
        return 1;
    }

    if (tbos_net_bind(sock, port) != 0) {
        printf("serve: failed to bind to port %u\n", port);
        tbos_net_close(sock);
        return 1;
    }

    if (tbos_net_listen(sock, 5) != 0) {
        printf("serve: failed to listen\n");
        tbos_net_close(sock);
        return 1;
    }

    printf("Serving HTTP on port %u (Ctrl+C to stop)...\n", port);

    const char* response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>TBOS Web Server</h1>"
        "<p>Serving with consciousness and compassion.</p>"
        "</body></html>\r\n";

    while (1) {
        int client = tbos_net_accept(sock);
        if (client < 0) continue;

        /* Read request (discard) */
        char buf[1024];
        tbos_net_recv(client, buf, sizeof(buf));

        /* Send response */
        tbos_net_send(client, response, strlen(response));
        tbos_net_close(client);

        universal_add_karma(1, "Served request");
    }

    tbos_net_close(sock);
    return 0;
}

static int cmd_wow(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: wow <url>\n");
        printf("       wow wow://http://example.com/path\n");
        printf("       wow wow://sangha-name/resource\n");
        return 1;
    }

    const char* url = argv[1];

    /* Ensure wow:// prefix */
    char wow_url[2048];
    if (strncmp(url, "wow://", 6) == 0) {
        strncpy(wow_url, url, sizeof(wow_url) - 1);
    } else {
        snprintf(wow_url, sizeof(wow_url), "wow://%s", url);
    }

    wow_response_t response;
    if (wow_fetch(wow_url, &response) != WOW_SUCCESS) {
        printf("wow: fetch failed\n");
        return 1;
    }

    if (response.data) {
        printf("%s\n", (char*)response.data);
    }

    printf("\n[Status: %d, Karma: %+ld]\n", response.status, response.karma);

    universal_add_karma(response.karma, "WOW fetch");
    wow_response_free(&response);
    return 0;
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

    /* TBOS Native Network Commands */
    universal_shell_register_command(
        "netstat", cmd_netstat,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "show network status",
        "netstat\n"
        "Display TBOS network statistics and active connections.");

    universal_shell_register_command(
        "nc", cmd_nc,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "netcat TCP/UDP tool",
        "nc [-l] <host> <port>\n"
        "  -l         Listen mode\n"
        "  host port  Connect to host:port");

    universal_shell_register_command(
        "http", cmd_http,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "HTTP client",
        "http <get|post> <url>\n"
        "Simple HTTP client using TBOS network stack.");

    universal_shell_register_command(
        "serve", cmd_serve,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "HTTP server",
        "serve [port]\n"
        "Start simple HTTP server (default port 8080).");

    universal_shell_register_command(
        "wow", cmd_wow,
        CMD_CAT_NETWORK, OS_SUPPORT_ALL,
        "WOW universal fetch",
        "wow <url>\n"
        "Fetch via WOW protocol:\n"
        "  wow://http://host/path\n"
        "  wow://https://host/path\n"
        "  wow://ftp://host/path\n"
        "  wow://sangha-name/resource");
}
