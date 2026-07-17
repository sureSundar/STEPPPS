/**
 * TBOS Tier-Aware Shell
 * Scales from 4-bit calculators to supercomputers
 *
 * Tier 0: Calculator   (4-8 bit,  256B-4KB RAM)   - 6 commands
 * Tier 1: Embedded     (16-bit,   4KB-64KB RAM)   - 17 commands
 * Tier 2: Retro        (32-bit,   64KB-16MB RAM)  - 26 commands
 * Tier 3: Desktop      (64-bit,   16MB-16GB RAM)  - 36 commands
 * Tier 4: Server       (64-bit,   16GB-1TB RAM)   - 41 commands
 * Tier 5: Supercomputer (64-bit+, 1TB+ distributed) - 45 commands
 *
 * Every tier from Calculator to Supercomputer has real POSIX-socket
 * networking (net/send/resolve/http/listen/netscan), scaled to what's
 * thematically appropriate per tier - not system()/popen() shell-outs.
 * See tbos_net_tcp_connect() - the one primitive every tier's network
 * command is built on.
 *
 * Build: gcc -DTBOS_TIER=N -o tbos_tier_N src/shell/tbos_tier_shell.c -Iinclude
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#ifndef _WIN32
/* Real POSIX sockets, available at every tier that can compile them.
 * Windows needs winsock2 instead - network commands are excluded there,
 * same as the existing ls/cd/cat/ping/etc POSIX-only commands. */
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

#ifndef TBOS_TIER
#define TBOS_TIER 3  /* Default to Desktop tier */
#endif

/* STEPPPS dimensions enabled per tier */
#if TBOS_TIER == 0
    #define TBOS_TIER_NAME    "Calculator"
    #define TBOS_TIER_BITS    8
    #define TBOS_TIER_RAM     "256B"
    #define STEPPPS_SPACE     1
    #define STEPPPS_TIME      1
    #define STEPPPS_EVENT     0
    #define STEPPPS_PSYCH     0
    #define STEPPPS_PIXEL     0
    #define STEPPPS_PROMPT    0
    #define STEPPPS_SCRIPT    0
#elif TBOS_TIER == 1
    #define TBOS_TIER_NAME    "Embedded"
    #define TBOS_TIER_BITS    16
    #define TBOS_TIER_RAM     "4KB"
    #define STEPPPS_SPACE     1
    #define STEPPPS_TIME      1
    #define STEPPPS_EVENT     1
    #define STEPPPS_PSYCH     0
    #define STEPPPS_PIXEL     0
    #define STEPPPS_PROMPT    1
    #define STEPPPS_SCRIPT    0
#elif TBOS_TIER == 2
    #define TBOS_TIER_NAME    "Retro"
    #define TBOS_TIER_BITS    32
    #define TBOS_TIER_RAM     "64KB"
    #define STEPPPS_SPACE     1
    #define STEPPPS_TIME      1
    #define STEPPPS_EVENT     1
    #define STEPPPS_PSYCH     0
    #define STEPPPS_PIXEL     1
    #define STEPPPS_PROMPT    1
    #define STEPPPS_SCRIPT    1
#elif TBOS_TIER == 3
    #define TBOS_TIER_NAME    "Desktop"
    #define TBOS_TIER_BITS    64
    #define TBOS_TIER_RAM     "1MB"
    #define STEPPPS_SPACE     1
    #define STEPPPS_TIME      1
    #define STEPPPS_EVENT     1
    #define STEPPPS_PSYCH     1
    #define STEPPPS_PIXEL     1
    #define STEPPPS_PROMPT    1
    #define STEPPPS_SCRIPT    1
#elif TBOS_TIER == 4
    #define TBOS_TIER_NAME    "Server"
    #define TBOS_TIER_BITS    64
    #define TBOS_TIER_RAM     "4MB"
    #define STEPPPS_SPACE     1
    #define STEPPPS_TIME      1
    #define STEPPPS_EVENT     1
    #define STEPPPS_PSYCH     1
    #define STEPPPS_PIXEL     1
    #define STEPPPS_PROMPT    1
    #define STEPPPS_SCRIPT    1
#else /* TBOS_TIER >= 5 */
    #define TBOS_TIER_NAME    "Supercomputer"
    #define TBOS_TIER_BITS    64
    #define TBOS_TIER_RAM     "16MB"
    #define STEPPPS_SPACE     1
    #define STEPPPS_TIME      1
    #define STEPPPS_EVENT     1
    #define STEPPPS_PSYCH     1
    #define STEPPPS_PIXEL     1
    #define STEPPPS_PROMPT    1
    #define STEPPPS_SCRIPT    1
#endif

/* Platform includes */
#ifdef _WIN32
    #include <io.h>
    #define popen _popen
    #define pclose _pclose
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <dirent.h>
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONFIGURATION
 * ═══════════════════════════════════════════════════════════════════════════ */

#define VERSION "1.0.0"

/* Scale buffer sizes by tier */
#if TBOS_TIER <= 1
    #define MAX_INPUT    64
    #define MAX_ARGS     8
    #define MAX_PATH     64
    #define MAX_COMMANDS 20
    #define HISTORY_SIZE 5
#elif TBOS_TIER == 2
    #define MAX_INPUT    256
    #define MAX_ARGS     16
    #define MAX_PATH     256
    #define MAX_COMMANDS 50
    #define HISTORY_SIZE 20
#else
    #define MAX_INPUT    1024
    #define MAX_ARGS     64
    #define MAX_PATH     1024
    #define MAX_COMMANDS 150
    #define HISTORY_SIZE 50
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef int (*cmd_handler_t)(int argc, char** argv);

typedef struct {
    const char* name;
    const char* desc;
    cmd_handler_t handler;
    int min_tier;  /* Minimum tier required for this command */
} tbos_cmd_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * GLOBAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_running = 1;
static int g_karma = 100;
static char g_cwd[MAX_PATH];

#if TBOS_TIER >= 2
static char g_history[HISTORY_SIZE][MAX_INPUT];
static int g_history_count = 0;
#endif

static tbos_cmd_t g_commands[MAX_COMMANDS];
static int g_command_count = 0;

/* ═══════════════════════════════════════════════════════════════════════════
 * UTILITY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* consciousness_level(void) {
    if (g_karma >= 500) return "Enlightened";
    if (g_karma >= 300) return "Compassionate";
    if (g_karma >= 150) return "Aware";
    if (g_karma >= 50)  return "Awakening";
    return "Dormant";
}

static void add_karma(int amount) {
    g_karma += amount;
    if (g_karma < 0) g_karma = 0;
}

#ifndef _WIN32
/* ═══════════════════════════════════════════════════════════════════════════
 * SHARED NETWORK PRIMITIVE
 * Every tier's network command builds on this one real TCP connect - no
 * system()/popen() shelling out anywhere in this file's network code.
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Connects to host:port over TCP. Returns a connected fd (caller must
 * close() it) or -1 on failure. timeout_sec bounds both connect and any
 * later read/write on the returned fd. */
static int tbos_net_tcp_connect(const char* host, int port, int timeout_sec) {
    struct addrinfo hints, *res = NULL, *rp;
    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%d", port);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port_str, &hints, &res) != 0 || !res) return -1;

    int fd = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0) continue;

        struct timeval tv;
        tv.tv_sec = timeout_sec;
        tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        if (connect(fd, rp->ai_addr, rp->ai_addrlen) == 0) break;
        close(fd);
        fd = -1;
    }
    freeaddrinfo(res);
    return fd;
}
#endif /* !_WIN32 */

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER 0 COMMANDS (Calculator: 5 commands, +1 network = 6)
 * Basic operations only: help, echo, calc, karma, exit, net
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_help(int argc, char** argv);
static int cmd_exit(int argc, char** argv);

#ifndef _WIN32
static int cmd_net(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: net <host> <port>\n");
        printf("Real TCP connect probe - reports open or closed/unreachable.\n");
        printf("The smallest network primitive; every higher tier's network\n");
        printf("command is built on this same connect.\n");
        return 1;
    }
    int port = atoi(argv[2]);
    int fd = tbos_net_tcp_connect(argv[1], port, 3);
    if (fd >= 0) {
        printf("%s:%d - OPEN (connected)\n", argv[1], port);
        close(fd);
        return 0;
    }
    printf("%s:%d - closed or unreachable\n", argv[1], port);
    return 1;
}
#endif

static int cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], (i < argc-1) ? " " : "");
    }
    printf("\n");
    return 0;
}

static int cmd_calc(int argc, char** argv) {
    if (argc < 4) {
        printf("Usage: calc <num1> <op> <num2>\n");
        printf("  ops: + - * /\n");
        return 1;
    }

    int a = atoi(argv[1]);
    char op = argv[2][0];
    int b = atoi(argv[3]);
    int result = 0;

    switch (op) {
        case '+': result = a + b; break;
        case '-': result = a - b; break;
        case '*': result = a * b; break;
        case '/':
            if (b == 0) { printf("Error: Division by zero\n"); return 1; }
            result = a / b;
            break;
        default:
            printf("Unknown operator: %c\n", op);
            return 1;
    }

    printf("%d %c %d = %d\n", a, op, b, result);
    add_karma(1);
    return 0;
}

static int cmd_karma(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Karma: %d (%s)\n", g_karma, consciousness_level());
    return 0;
}

static int cmd_exit(int argc, char** argv) {
    (void)argc; (void)argv;
    g_running = 0;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER 1 COMMANDS (Embedded: +11 commands = 17 total)
 * Adds: ls, cd, pwd, cat, mkdir, rm, clear, date, whoami, tier, send
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER >= 1

static int cmd_pwd(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("%s\n", g_cwd);
    return 0;
}

static int cmd_clear(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\033[2J\033[H");
    return 0;
}

static int cmd_date(int argc, char** argv) {
    (void)argc; (void)argv;
    time_t now = time(NULL);
    printf("%s", ctime(&now));
    return 0;
}

static int cmd_whoami(int argc, char** argv) {
    (void)argc; (void)argv;
#ifdef _WIN32
    printf("%s\n", getenv("USERNAME") ? getenv("USERNAME") : "user");
#else
    printf("%s\n", getenv("USER") ? getenv("USER") : "user");
#endif
    return 0;
}

static int cmd_tier(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Hardware Tier: %d (%s)\n", TBOS_TIER, TBOS_TIER_NAME);
    printf("  Bits: %d-bit\n", TBOS_TIER_BITS);
    printf("  RAM:  %s\n", TBOS_TIER_RAM);
    printf("  STEPPPS: ");
    if (STEPPPS_SPACE) printf("S");
    if (STEPPPS_TIME) printf("T");
    if (STEPPPS_EVENT) printf("E");
    if (STEPPPS_PSYCH) printf("P");
    if (STEPPPS_PIXEL) printf("P");
    if (STEPPPS_PROMPT) printf("P");
    if (STEPPPS_SCRIPT) printf("S");
    printf("\n");
    printf("  Commands: %d available\n", g_command_count);
    return 0;
}

#ifndef _WIN32
static int cmd_ls(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : ".";
    DIR* dir = opendir(path);
    if (!dir) {
        perror("ls");
        return 1;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' && argc <= 1) continue;
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
    return 0;
}

static int cmd_cd(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : getenv("HOME");
    if (!path) path = "/";

    if (chdir(path) != 0) {
        perror("cd");
        return 1;
    }

    getcwd(g_cwd, sizeof(g_cwd));
    return 0;
}

static int cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cat <file>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("cat");
        return 1;
    }

    char buf[256];
    while (fgets(buf, sizeof(buf), f)) {
        printf("%s", buf);
    }

    fclose(f);
    add_karma(1);
    return 0;
}

static int cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: mkdir <dir>\n");
        return 1;
    }

    if (mkdir(argv[1], 0755) != 0) {
        perror("mkdir");
        return 1;
    }

    add_karma(2);
    return 0;
}

static int cmd_rm(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: rm <file>\n");
        return 1;
    }

    if (unlink(argv[1]) != 0) {
        perror("rm");
        return 1;
    }

    return 0;
}

static int cmd_send(int argc, char** argv) {
    if (argc < 4) {
        printf("Usage: send <host> <port> <text...>\n");
        printf("Connects, writes text terminated with CRLF, prints any reply.\n");
        return 1;
    }
    int port = atoi(argv[2]);
    int fd = tbos_net_tcp_connect(argv[1], port, 3);
    if (fd < 0) {
        printf("send: could not connect to %s:%d\n", argv[1], port);
        return 1;
    }

    char msg[512] = {0};
    for (int i = 3; i < argc; i++) {
        strncat(msg, argv[i], sizeof(msg) - strlen(msg) - 1);
        if (i < argc - 1) strncat(msg, " ", sizeof(msg) - strlen(msg) - 1);
    }
    strncat(msg, "\r\n", sizeof(msg) - strlen(msg) - 1);

    ssize_t sent = write(fd, msg, strlen(msg));
    if (sent < 0) { perror("send"); close(fd); return 1; }

    char buf[1024];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("Reply (%zd bytes):\n%s\n", n, buf);
    } else {
        printf("(no reply within timeout)\n");
    }
    close(fd);
    return 0;
}
#endif /* !_WIN32 */

#endif /* TBOS_TIER >= 1 */

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER 2 COMMANDS (Retro: +9 commands = 26 total)
 * Adds: head, wc, grep, touch, uname, uptime, history, env, resolve
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER >= 2

static int cmd_history(int argc, char** argv) {
    (void)argc; (void)argv;
    for (int i = 0; i < g_history_count; i++) {
        printf("%3d  %s\n", i + 1, g_history[i]);
    }
    return 0;
}

static int cmd_env(int argc, char** argv) {
    (void)argc; (void)argv;
    extern char** environ;
    for (char** env = environ; *env; env++) {
        printf("%s\n", *env);
    }
    return 0;
}

#ifndef _WIN32
static int cmd_head(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: head <file> [lines]\n");
        return 1;
    }

    int lines = (argc > 2) ? atoi(argv[2]) : 10;
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("head");
        return 1;
    }

    char buf[512];
    int count = 0;
    while (count < lines && fgets(buf, sizeof(buf), f)) {
        printf("%s", buf);
        count++;
    }

    fclose(f);
    return 0;
}

static int cmd_wc(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: wc <file>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("wc");
        return 1;
    }

    int lines = 0, words = 0, chars = 0;
    int in_word = 0, c;

    while ((c = fgetc(f)) != EOF) {
        chars++;
        if (c == '\n') lines++;
        if (isspace(c)) {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            words++;
        }
    }

    printf("%d %d %d %s\n", lines, words, chars, argv[1]);
    fclose(f);
    return 0;
}

static int cmd_grep(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: grep <pattern> <file>\n");
        return 1;
    }

    FILE* f = fopen(argv[2], "r");
    if (!f) {
        perror("grep");
        return 1;
    }

    char buf[512];
    int line = 0;
    while (fgets(buf, sizeof(buf), f)) {
        line++;
        if (strstr(buf, argv[1])) {
            printf("%d:%s", line, buf);
        }
    }

    fclose(f);
    add_karma(1);
    return 0;
}

static int cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: touch <file>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "a");
    if (!f) {
        perror("touch");
        return 1;
    }
    fclose(f);
    return 0;
}

static int cmd_uname(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("TBOS %s Tier-%d %s %d-bit\n",
           VERSION, TBOS_TIER, TBOS_TIER_NAME, TBOS_TIER_BITS);
    return 0;
}

static int cmd_uptime(int argc, char** argv) {
    (void)argc; (void)argv;
    /* Simplified - just show system time */
    time_t now = time(NULL);
    printf("System time: %s", ctime(&now));
    return 0;
}

static int cmd_resolve(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: resolve <hostname>\n");
        return 1;
    }

    struct addrinfo hints, *res = NULL, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rc = getaddrinfo(argv[1], NULL, &hints, &res);
    if (rc != 0) {
        printf("resolve: %s\n", gai_strerror(rc));
        return 1;
    }

    printf("Addresses for %s:\n", argv[1]);
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        char ipstr[INET6_ADDRSTRLEN];
        void* addr;
        if (rp->ai_family == AF_INET) {
            addr = &((struct sockaddr_in*)rp->ai_addr)->sin_addr;
        } else {
            addr = &((struct sockaddr_in6*)rp->ai_addr)->sin6_addr;
        }
        inet_ntop(rp->ai_family, addr, ipstr, sizeof(ipstr));
        printf("  %s\n", ipstr);
    }
    freeaddrinfo(res);
    add_karma(1);
    return 0;
}
#endif /* !_WIN32 */

#endif /* TBOS_TIER >= 2 */

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER 3+ COMMANDS (Desktop: +10 commands = 36 total)
 * Adds: meditate, reflect, sangha, dharma, pxfs, steppps, ping, df, ps, http
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER >= 3

static int cmd_meditate(int argc, char** argv) {
    int secs = (argc > 1) ? atoi(argv[1]) : 3;
    if (secs < 1) secs = 1;
    if (secs > 30) secs = 30;

    printf("\nMeditating");
    fflush(stdout);
    for (int i = 0; i < secs; i++) {
        printf(".");
        fflush(stdout);
#ifdef _WIN32
        _sleep(1000);
#else
        sleep(1);
#endif
    }
    printf("\n\nMeditation complete. +%d karma\n", secs * 2);
    add_karma(secs * 2);
    return 0;
}

static int cmd_reflect(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("  \"Code with compassion,\n");
    printf("   Debug with patience,\n");
    printf("   Deploy with wisdom.\"\n");
    printf("\n");
    printf("  Karma: %d (%s)\n", g_karma, consciousness_level());
    printf("\n");
    add_karma(5);
    return 0;
}

static int cmd_sangha(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Digital Sangha Status:\n");
    printf("  Mode: Standalone\n");
    printf("  Port: 7805\n");
    printf("  Discovery: mDNS, UDP broadcast\n");
    printf("\nUse 'sangha join <name>' to join a device community\n");
    return 0;
}

static int cmd_dharma(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\nThe Dharma of Computing:\n");
    printf("  1. Right Code     - Write with intention\n");
    printf("  2. Right Data     - Handle with care\n");
    printf("  3. Right Process  - Execute mindfully\n");
    printf("  4. Right Memory   - Use wisely\n");
    printf("  5. Right Network  - Connect compassionately\n");
    printf("  6. Right Storage  - Preserve with integrity\n");
    printf("  7. Right Interface - Serve with kindness\n");
    printf("  8. Right System   - Harmonize all\n\n");
    add_karma(10);
    return 0;
}

static int cmd_pxfs(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("PXFS - PixelXpress File System\n");
    printf("  Status: Available\n");
    printf("  Compression: RGB pixel encoding\n");
    printf("  Ratio: Up to 1365:1\n");
    return 0;
}

static int cmd_steppps(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("STEPPPS Framework Status:\n");
    printf("  Space:      %s\n", STEPPPS_SPACE  ? "Enabled" : "Disabled");
    printf("  Time:       %s\n", STEPPPS_TIME   ? "Enabled" : "Disabled");
    printf("  Event:      %s\n", STEPPPS_EVENT  ? "Enabled" : "Disabled");
    printf("  Psychology: %s\n", STEPPPS_PSYCH  ? "Enabled" : "Disabled");
    printf("  Pixel:      %s\n", STEPPPS_PIXEL  ? "Enabled" : "Disabled");
    printf("  Prompt:     %s\n", STEPPPS_PROMPT ? "Enabled" : "Disabled");
    printf("  Script:     %s\n", STEPPPS_SCRIPT ? "Enabled" : "Disabled");
    return 0;
}

#ifndef _WIN32
static int cmd_ping(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ping <host>\n");
        return 1;
    }

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ping -c 4 %s", argv[1]);
    return system(cmd);
}

static int cmd_df(int argc, char** argv) {
    (void)argc; (void)argv;
    return system("df -h");
}

static int cmd_ps(int argc, char** argv) {
    (void)argc; (void)argv;
    return system("ps aux | head -20");
}

static int cmd_http(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: http <host> [path]\n");
        printf("Real HTTP/1.0 GET over a raw TCP socket to port 80 - no\n");
        printf("curl/wget subprocess involved.\n");
        return 1;
    }
    const char* path = (argc >= 3) ? argv[2] : "/";

    int fd = tbos_net_tcp_connect(argv[1], 80, 5);
    if (fd < 0) {
        printf("http: could not connect to %s:80\n", argv[1]);
        return 1;
    }

    char req[512];
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.0\r\nHost: %s\r\nConnection: close\r\n\r\n",
             path, argv[1]);
    if (write(fd, req, strlen(req)) < 0) {
        perror("http");
        close(fd);
        return 1;
    }

    char buf[4096];
    ssize_t n;
    size_t total = 0;
    printf("--- response (first 2KB) ---\n");
    while ((n = read(fd, buf, sizeof(buf) - 1)) > 0 && total < 2048) {
        buf[n] = '\0';
        printf("%s", buf);
        total += (size_t)n;
    }
    printf("\n--- %zu bytes shown ---\n", total);
    close(fd);
    add_karma(1);
    return 0;
}
#endif

#endif /* TBOS_TIER >= 3 */

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER 4+ COMMANDS (Server: +5 commands = 41 total)
 * Adds: top, netstat, free, lscpu, listen
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER >= 4

#ifndef _WIN32
static int cmd_top(int argc, char** argv) {
    (void)argc; (void)argv;
    return system("top -n 1 -b | head -30");
}

static int cmd_netstat(int argc, char** argv) {
    (void)argc; (void)argv;
    return system("netstat -tuln 2>/dev/null || ss -tuln");
}

static int cmd_free(int argc, char** argv) {
    (void)argc; (void)argv;
    return system("free -h 2>/dev/null || vm_stat");
}

static int cmd_lscpu(int argc, char** argv) {
    (void)argc; (void)argv;
    return system("lscpu 2>/dev/null || sysctl -a | grep cpu");
}

/* Minimal single-connection TCP echo server. A "server tier" should be
 * able to serve, not just consume, the network. Bound to localhost only
 * on principle - a shell command should not become an unintentional
 * network-reachable listener (see tbos_api_server.c's fix for why). */
static int cmd_listen(int argc, char** argv) {
    int port = (argc >= 2) ? atoi(argv[1]) : 9000;

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("listen"); return 1; }

    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons((uint16_t)port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("listen: bind");
        close(srv);
        return 1;
    }
    if (listen(srv, 1) < 0) {
        perror("listen: listen");
        close(srv);
        return 1;
    }

    printf("Listening on 127.0.0.1:%d (one connection, 10s timeout)...\n", port);
    printf("Try from another terminal: nc 127.0.0.1 %d\n", port);

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(srv, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(srv, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        printf("(no connection within timeout)\n");
        close(srv);
        return 1;
    }

    char buf[1024];
    ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("Received %zd bytes: %s\n", n, buf);
        write(client_fd, buf, (size_t)n); /* echo back */
    }
    close(client_fd);
    close(srv);
    add_karma(2);
    return 0;
}
#endif

#endif /* TBOS_TIER >= 4 */

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER 5 COMMANDS (Supercomputer: +4 commands = 45 total)
 * Adds: cluster, consciousness, quantum, netscan
 * ═══════════════════════════════════════════════════════════════════════════ */

#if TBOS_TIER >= 5

static int cmd_cluster(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("TBOS Cluster Status:\n");
    printf("  Mode: Supercomputer (Tier 5)\n");
    printf("  Nodes: 1 (local)\n");
    printf("  Distributed Memory: Ready\n");
    printf("  MPI Support: Simulated\n");
    return 0;
}

static int cmd_consciousness(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Consciousness Framework:\n");
    printf("  Level: %s\n", consciousness_level());
    printf("  Karma: %d\n", g_karma);
    printf("  AI Integration: Tier 5 Full\n");
    printf("  Psychology Module: Active\n");
    return 0;
}

static int cmd_quantum(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Quantum Simulation Status:\n");
    printf("  Qubits: 8 (simulated)\n");
    printf("  Entanglement: Disabled\n");
    printf("  Error Correction: Basic\n");
    return 0;
}

#ifndef _WIN32
/* Scans a bounded port range using the same tier-0 connect primitive,
 * in a loop - "supercomputer tier" gets the aggregate/parallel-flavored
 * operation, built from the same real socket code every other tier uses,
 * not a separate implementation. Range is capped so a demo scan stays
 * fast and doesn't become an accidental network scanner. */
static int cmd_netscan(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: netscan <host> [startport] [endport]\n");
        printf("Scans up to 200 ports using the tier-0 TCP connect probe.\n");
        return 1;
    }
    int start = (argc >= 3) ? atoi(argv[2]) : 1;
    int end = (argc >= 4) ? atoi(argv[3]) : 1024;
    if (start < 1) start = 1;
    if (end - start > 200) end = start + 200;

    printf("Scanning %s ports %d-%d...\n", argv[1], start, end);
    int found = 0;
    for (int p = start; p <= end; p++) {
        int fd = tbos_net_tcp_connect(argv[1], p, 1);
        if (fd >= 0) {
            printf("  %d/tcp open\n", p);
            close(fd);
            found++;
        }
    }
    printf("Scan complete: %d open port(s) found.\n", found);
    add_karma(1);
    return 0;
}
#endif

#endif /* TBOS_TIER >= 5 */

/* ═══════════════════════════════════════════════════════════════════════════
 * HELP COMMAND
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\nTBOS Shell - Tier %d (%s)\n", TBOS_TIER, TBOS_TIER_NAME);
    printf("================================\n");
    printf("Available commands (%d):\n\n", g_command_count);

    for (int i = 0; i < g_command_count; i++) {
        printf("  %-12s - %s\n", g_commands[i].name, g_commands[i].desc);
    }

    printf("\n");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

static void register_cmd(const char* name, const char* desc, cmd_handler_t h, int tier) {
    if (TBOS_TIER >= tier && g_command_count < MAX_COMMANDS) {
        g_commands[g_command_count].name = name;
        g_commands[g_command_count].desc = desc;
        g_commands[g_command_count].handler = h;
        g_commands[g_command_count].min_tier = tier;
        g_command_count++;
    }
}

static void register_all_commands(void) {
    /* Tier 0: Calculator (5 base + net = 6 commands) */
    register_cmd("help",   "Show available commands",  cmd_help,   0);
    register_cmd("echo",   "Display text",             cmd_echo,   0);
    register_cmd("calc",   "Calculator (+ - * /)",     cmd_calc,   0);
    register_cmd("karma",  "Show karma status",        cmd_karma,  0);
    register_cmd("exit",   "Exit shell",               cmd_exit,   0);
#ifndef _WIN32
    register_cmd("net",    "TCP connectivity probe",   cmd_net,    0);
#endif

#if TBOS_TIER >= 1
    /* Tier 1: Embedded (+10 base + send = 11; cumulative 17) */
    register_cmd("pwd",    "Print working directory",  cmd_pwd,    1);
    register_cmd("clear",  "Clear screen",             cmd_clear,  1);
    register_cmd("date",   "Show current date/time",   cmd_date,   1);
    register_cmd("whoami", "Show current user",        cmd_whoami, 1);
    register_cmd("tier",   "Show hardware tier info",  cmd_tier,   1);
#ifndef _WIN32
    register_cmd("ls",     "List directory",           cmd_ls,     1);
    register_cmd("cd",     "Change directory",         cmd_cd,     1);
    register_cmd("cat",    "Display file contents",    cmd_cat,    1);
    register_cmd("mkdir",  "Create directory",         cmd_mkdir,  1);
    register_cmd("rm",     "Remove file",              cmd_rm,     1);
    register_cmd("send",   "Send raw TCP message, read reply", cmd_send, 1);
#endif
#endif

#if TBOS_TIER >= 2
    /* Tier 2: Retro (+8 base + resolve = 9; cumulative 26) */
    register_cmd("history","Show command history",     cmd_history, 2);
    register_cmd("env",    "Show environment",         cmd_env,     2);
#ifndef _WIN32
    register_cmd("head",   "Display first lines",      cmd_head,    2);
    register_cmd("wc",     "Word/line/char count",     cmd_wc,      2);
    register_cmd("grep",   "Search for pattern",       cmd_grep,    2);
    register_cmd("touch",  "Create/update file",       cmd_touch,   2);
    register_cmd("uname",  "System information",       cmd_uname,   2);
    register_cmd("uptime", "System uptime",            cmd_uptime,  2);
    register_cmd("resolve","DNS resolve a hostname",   cmd_resolve, 2);
#endif
#endif

#if TBOS_TIER >= 3
    /* Tier 3: Desktop (+9 base + http = 10; cumulative 36) */
    register_cmd("meditate","Mindful break",           cmd_meditate, 3);
    register_cmd("reflect", "Reflect on journey",      cmd_reflect,  3);
    register_cmd("sangha",  "Digital sangha network",  cmd_sangha,   3);
    register_cmd("dharma",  "Dharma of computing",     cmd_dharma,   3);
    register_cmd("pxfs",    "PXFS filesystem info",    cmd_pxfs,     3);
    register_cmd("steppps", "STEPPPS framework",       cmd_steppps,  3);
#ifndef _WIN32
    register_cmd("ping",    "Ping a host",             cmd_ping,     3);
    register_cmd("df",      "Disk free space",         cmd_df,       3);
    register_cmd("ps",      "Process status",          cmd_ps,       3);
    register_cmd("http",    "Real HTTP/1.0 GET client", cmd_http,    3);
#endif
#endif

#if TBOS_TIER >= 4
    /* Tier 4: Server (+4 base + listen = 5; cumulative 41) */
#ifndef _WIN32
    register_cmd("top",     "Process monitor",         cmd_top,      4);
    register_cmd("netstat", "Network connections",     cmd_netstat,  4);
    register_cmd("free",    "Memory usage",            cmd_free,     4);
    register_cmd("lscpu",   "CPU information",         cmd_lscpu,    4);
    register_cmd("listen",  "Minimal TCP echo server (localhost)", cmd_listen, 4);
#endif
#endif

#if TBOS_TIER >= 5
    /* Tier 5: Supercomputer (+3 base + netscan = 4; cumulative 45) */
    register_cmd("cluster",     "Cluster status",      cmd_cluster,      5);
    register_cmd("consciousness","AI consciousness",   cmd_consciousness,5);
    register_cmd("quantum",     "Quantum simulation",  cmd_quantum,      5);
#ifndef _WIN32
    register_cmd("netscan", "Scan a port range (uses TCP connect probe)", cmd_netscan, 5);
#endif
#endif
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND EXECUTION
 * ═══════════════════════════════════════════════════════════════════════════ */

static int parse_line(char* line, char** argv) {
    int argc = 0;
    char* p = line;

    while (*p && argc < MAX_ARGS - 1) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        argv[argc++] = p;
        while (*p && *p != ' ' && *p != '\t') p++;
        if (*p) *p++ = '\0';
    }

    argv[argc] = NULL;
    return argc;
}

static tbos_cmd_t* find_command(const char* name) {
    for (int i = 0; i < g_command_count; i++) {
        if (strcmp(g_commands[i].name, name) == 0) {
            return &g_commands[i];
        }
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BANNER & PROMPT
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_banner(void) {
#if TBOS_TIER == 0
    printf("\n");
    printf("TBOS Calculator Shell\n");
    printf("Type 'help' for commands\n");
    printf("\n");
#elif TBOS_TIER <= 2
    printf("\n");
    printf("TBOS %s Shell (Tier %d)\n", TBOS_TIER_NAME, TBOS_TIER);
    printf("%d-bit, %s RAM | %d commands\n", TBOS_TIER_BITS, TBOS_TIER_RAM, g_command_count);
    printf("Type 'help' for commands\n");
    printf("\n");
#else
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║  TBOS %s Shell (Tier %d)                       \n", TBOS_TIER_NAME, TBOS_TIER);
    printf("║  %d-bit, %s RAM | %d commands available            \n",
           TBOS_TIER_BITS, TBOS_TIER_RAM, g_command_count);
    printf("║  STEPPPS: ");
    if (STEPPPS_SPACE)  printf("S");
    if (STEPPPS_TIME)   printf("T");
    if (STEPPPS_EVENT)  printf("E");
    if (STEPPPS_PSYCH)  printf("P");
    if (STEPPPS_PIXEL)  printf("P");
    if (STEPPPS_PROMPT) printf("P");
    if (STEPPPS_SCRIPT) printf("S");
    printf(" | Karma: %d                              \n", g_karma);
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("\n");
#endif
}

static void print_prompt(void) {
#if TBOS_TIER == 0
    printf("> ");
#elif TBOS_TIER == 1
    printf("tbos> ");
#else
    printf("tbos[%d]:%s$ ", g_karma, g_cwd);
#endif
    fflush(stdout);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(int argc, char** argv) {
    char input[MAX_INPUT];
    char* cmd_argv[MAX_ARGS];

    (void)argc; (void)argv;

    /* Initialize */
    getcwd(g_cwd, sizeof(g_cwd));
    register_all_commands();

    /* Banner */
    print_banner();

    /* Main loop */
    while (g_running) {
        print_prompt();

        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }

        /* Remove newline */
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }

        /* Skip empty */
        if (input[0] == '\0') continue;

#if TBOS_TIER >= 2
        /* Add to history */
        if (g_history_count < HISTORY_SIZE) {
            strncpy(g_history[g_history_count++], input, MAX_INPUT - 1);
        }
#endif

        /* Parse and execute */
        int cmd_argc = parse_line(input, cmd_argv);
        if (cmd_argc == 0) continue;

        tbos_cmd_t* cmd = find_command(cmd_argv[0]);
        if (cmd) {
            cmd->handler(cmd_argc, cmd_argv);
        } else {
            printf("Unknown command: %s\n", cmd_argv[0]);
            printf("Type 'help' for available commands\n");
        }
    }

    printf("\nFinal Karma: %d (%s)\n", g_karma, consciousness_level());
    printf("Namaste.\n\n");

    return 0;
}
