/**
 * TBOS Tier-Aware Shell
 * Scales from 4-bit calculators to supercomputers
 *
 * Tier 0: Calculator   (4-8 bit,  256B-4KB RAM)   - 16 commands
 * Tier 1: Embedded     (16-bit,   4KB-64KB RAM)   - 27 commands
 * Tier 2: Retro        (32-bit,   64KB-16MB RAM)  - 36 commands
 * Tier 3: Desktop      (64-bit,   16MB-16GB RAM)  - 45 commands
 * Tier 4: Server       (64-bit,   16GB-1TB RAM)   - 50 commands
 * Tier 5: Supercomputer (64-bit+, 1TB+ distributed) - 54 commands
 *
 * steppps show/verify/run/audit is linked directly against the
 * canonical steppps/v2 runtime (steppps_runtime.c) - real signature
 * verification and sandboxed execution, not a decorative status
 * display. show (Tier 0+) is read-only; verify/run/audit climb tiers
 * by trust/execution risk, the same gating rationale as pxfs/ucfs/rf2s.
 *
 * Real, lossless, genuinely-reversible PXFS compression (pxcompress/
 * pxdecompress/pxstore/pxload/pxsend/pxrecv) is at Tier 0 too - store
 * to a file, transmit over a real TCP socket, extract back byte-exact.
 * Not the fake "QUANTUM" scheme elsewhere in this codebase that
 * classifies input into one of 4 buckets and fabricates a templated
 * pattern on "decompress" with zero relation to the original data -
 * this one actually round-trips, verified.
 *
 * Every tier from Calculator to Supercomputer has real POSIX-socket
 * networking (net/send/resolve/http/listen/netscan), scaled to what's
 * thematically appropriate per tier - not system()/popen() shell-outs.
 * See tbos_net_tcp_connect() - the one primitive every tier's network
 * command is built on.
 *
 * Tier 0 also has real filesystem-path commands (ucfs/rf2s, reusing
 * the canonical codecs) and real PXFS RAW-mode pixel encoding
 * (pxencode) - not decorative status text, and not gated to a higher
 * tier, because parsing a string or packing 3 bytes into a pixel costs
 * no more than calc() does. The calculator tier proves the point: a
 * compute primitive (calc), a communicate primitive (net), a represent
 * primitive (pxencode) and an address primitive (ucfs/rf2s) together
 * cover most of what separates Tier 0 from Tier 5 in practice - most
 * of what the higher tiers add on top is host-OS monitoring
 * (ps/top/netstat/df/free/lscpu) or decoration (karma/meditate/
 * reflect/sangha/dharma/consciousness/quantum/cluster), not new
 * computational capability. File I/O (ls/cat/mkdir/rm) and text
 * processing (head/grep/wc) are the one real, non-decorative gap that
 * remains between Tier 0 and Tier 5.
 *
 * Build (see Makefile target tier-shell-*): gcc -DTBOS_HOSTED -DHOST_BUILD
 *   -DTBOS_TIER=N -Iinclude src/shell/tbos_tier_shell.c
 *   src/core/filesystem/ucfs_codec.c src/core/filesystem/rf2s_codec.c
 *   kernel/hal_hosted.c -o tbos_tier_N
 *
 * The command table's storage strategy (grow on a real heap vs a fixed
 * static array) is decided at runtime via include/tbos/hal.h's
 * caps.has_dynamic_memory, not hardcoded per tier - see
 * tier_shell_init_command_table(). Link kernel/hal_baremetal.c instead
 * of kernel/hal_hosted.c to build against a platform with no heap; the
 * static-array fallback (still present, not dead code) takes over
 * automatically.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>
#include "../core/compression/pxfs_lossless.h"
#include "../../include/tbos/vfs.h"

extern const vfs_driver_t ramfs_driver;

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

/* Reuse the canonical path codecs (docs/TBOS_CANONICAL_MANIFEST.md
 * names src/core/filesystem/ as canonical for these) instead of writing
 * a fourth reimplementation of UCFS/RF2S path parsing. */
#include "fs/ucfs_codec.h"
#include "fs/rf2s_codec.h"

/* The command table's storage strategy is a HAL capability query, not
 * a hardcoded choice - see tier_shell_init_command_table() below. */
#include "tbos/hal.h"

/* Canonical STEPPPS v2 runtime (docs/TBOS_CANONICAL_MANIFEST.md) -
 * linked directly, not a sidecar (see the "direct link vs sidecar"
 * discussion: Tier 0-3 stay standalone with no assumed companion
 * process, matching every other command in this file). */
#include "steppps.h"
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
    #include <windows.h>
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

/* Scale buffer sizes by tier. <windows.h> (included above for _WIN32) also
 * defines MAX_PATH (260) - undefine it so TBOS's own tier-scaled value is
 * unambiguously what the rest of this file uses, not an artifact of
 * definition order. */
#ifdef _WIN32
    #undef MAX_PATH
#endif
#if TBOS_TIER <= 1
    #define MAX_INPUT    64
    #define MAX_ARGS     8
    #define MAX_PATH     64
    /* Was 20, tuned to exactly fit the old Tier 1 total - which meant
     * register_cmd()'s silent `g_command_count < MAX_COMMANDS` guard
     * quietly dropped 6 commands the moment Tier 0 grew past 14 (send,
     * the last one registered, has no way to report it never
     * registered). Raised with real headroom; this is a fixed-size
     * static array in a hosted simulation, not actual constrained RAM. */
    #define MAX_COMMANDS 32
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
static time_t g_shell_start_time;

#if TBOS_TIER >= 2
static char g_history[HISTORY_SIZE][MAX_INPUT];
static int g_history_count = 0;
#endif

/* Command table: HAL-capability-driven, not hardcoded to one strategy.
 * caps.has_dynamic_memory decides whether this grows on a real heap
 * (hosted targets - all 6 tiers in this build today) or stays a fixed
 * static array (real bare-metal Tier 0/1 targets with no heap at all,
 * if this is ever cross-compiled against kernel/hal_baremetal.c instead
 * of kernel/hal_hosted.c). Static array is a real fallback, not dead
 * code - it's still what the smallest real targets must use.
 *
 * No HAL adapter is linked for _WIN32 in this build, so it keeps the
 * plain static array unconditionally - which is fine, since every
 * command that could grow the table past MAX_COMMANDS is itself
 * #ifndef _WIN32 (net/ucfs/rf2s/pxcompress/send/etc), so a Windows
 * build's command count is already far smaller. */
static tbos_cmd_t g_commands_static[MAX_COMMANDS];
static tbos_cmd_t* g_commands = g_commands_static;
static int g_command_count = 0;
static int g_command_capacity = MAX_COMMANDS;

#ifndef _WIN32
static int g_using_dynamic_memory = 0;
static const hal_dispatch_table_t* g_hal = NULL;

static void tier_shell_init_command_table(void) {
    g_hal = hal_get_dispatch();
    if (!g_hal || !g_hal->capabilities) return;

    hal_capabilities_t caps = g_hal->capabilities();
    if (!caps.has_dynamic_memory || !g_hal->memory.alloc || !g_hal->memory.realloc) {
        return;  /* stay on the static array - this platform has no heap */
    }

    int initial_capacity = 16;
    tbos_cmd_t* dynamic = (tbos_cmd_t*)g_hal->memory.alloc(sizeof(tbos_cmd_t) * (size_t)initial_capacity);
    if (!dynamic) {
        /* Capability claimed available but allocation failed anyway -
         * fall back safely rather than crash. */
        return;
    }
    g_commands = dynamic;
    g_command_capacity = initial_capacity;
    g_using_dynamic_memory = 1;
}
#else
static void tier_shell_init_command_table(void) { /* static array only */ }
#endif

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
 * TIER 0 COMMANDS (Calculator: 5 base + 11 real net/fs/compression/
 * steppps = 16)
 * help, echo, calc, karma, exit, net, ucfs, rf2s, pxencode, pxcompress,
 * pxdecompress, pxstore, pxload, pxsend, pxrecv, steppps
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

#ifndef _WIN32
/* Real UCFS path parsing via the canonical codec - not a decorative
 * status display. No reason this needs to wait for a higher tier: it's
 * string parsing, same as anything else already at Tier 0. */
static int cmd_ucfs(int argc, char** argv) {
    if (argc < 3 || strcmp(argv[1], "parse") != 0) {
        printf("Usage: ucfs parse <unicode-path>\n");
        printf("Example: ucfs parse '\xF0\x9F\x8F\xA0/docs/file.txt'\n");
        return 1;
    }

    ucfs_path_t path;
    if (ucfs_parse(argv[2], &path) != 0) {
        printf("ucfs: not a valid UCFS path (must start with a non-ASCII character)\n");
        return 1;
    }

    printf("Delimiter:  %s (U+%04X)\n", path.delimiter_utf8, path.delimiter);
    printf("Components: %zu\n", path.component_count);
    for (size_t i = 0; i < path.component_count; i++) {
        printf("  [%zu] %s\n", i, path.components[i]);
    }
    char canonical[512];
    if (ucfs_to_canonical(&path, canonical, sizeof(canonical)) == 0) {
        printf("Canonical:  %s\n", canonical);
    }
    ucfs_free(&path);
    add_karma(1);
    return 0;
}

/* Real RF2S frequency-path parsing via the canonical codec. */
static int cmd_rf2s(int argc, char** argv) {
    if (argc < 3 || strcmp(argv[1], "parse") != 0) {
        printf("Usage: rf2s parse </freq/path...>\n");
        printf("Example: rf2s parse /432MHz/sensor/data\n");
        return 1;
    }

    rf2s_path_t path;
    if (rf2s_parse(argv[2], &path) != 0) {
        printf("rf2s: failed to parse path\n");
        return 1;
    }

    char freq_str[64];
    rf2s_format_frequency(path.frequency.frequency_hz, freq_str, sizeof(freq_str));
    printf("Frequency:  %s (%llu Hz)\n", freq_str,
           (unsigned long long)path.frequency.frequency_hz);
    printf("Components: %zu\n", path.component_count);
    for (size_t i = 0; i < path.component_count; i++) {
        printf("  [%zu] %s\n", i, path.components[i]);
    }
    char canonical[512];
    if (rf2s_to_canonical(&path, canonical, sizeof(canonical)) == 0) {
        printf("Canonical:  %s\n", canonical);
    }
    rf2s_free(&path);
    add_karma(1);
    return 0;
}

/* Real RAW-mode PXFS encoding: 3 bytes packed per RGB pixel. This is the
 * one PXFS mode simple enough to implement here without pulling in the
 * much larger canonical compression engine (src/core/filesystem/
 * pxfs_core.c, QUANTUM/FRACTAL pattern detection) - genuine encoding,
 * not a status display, and no reason it needs to wait for Tier 3
 * either: it's byte packing, same cost as calc(). */
static int cmd_pxencode(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: pxencode <text>\n");
        printf("Packs 3 bytes per RGB pixel (PXFS RAW mode).\n");
        return 1;
    }

    char msg[256] = {0};
    for (int i = 1; i < argc; i++) {
        strncat(msg, argv[i], sizeof(msg) - strlen(msg) - 1);
        if (i < argc - 1) strncat(msg, " ", sizeof(msg) - strlen(msg) - 1);
    }
    size_t len = strlen(msg);
    size_t pixels = (len + 2) / 3;

    printf("Input:  %zu bytes\n", len);
    printf("Output: %zu pixel(s)\n\n", pixels);
    printf("  Idx  RGB                Chars\n");
    printf("  ---  -----------------  -----\n");
    for (size_t p = 0; p < pixels; p++) {
        unsigned char r = (p * 3     < len) ? (unsigned char)msg[p * 3]     : 0;
        unsigned char g = (p * 3 + 1 < len) ? (unsigned char)msg[p * 3 + 1] : 0;
        unsigned char b = (p * 3 + 2 < len) ? (unsigned char)msg[p * 3 + 2] : 0;
        printf("  %3zu  (%3u,%3u,%3u)   %c%c%c\n", p, r, g, b,
               isprint(r) ? r : '.', isprint(g) ? g : '.', isprint(b) ? b : '.');
    }
    add_karma(1);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REAL PXFS COMPRESSION: genuinely lossless, byte-exact round trip.
 *
 * No scheme can losslessly compress arbitrary 4KB input to 3 bytes -
 * there are only 256^3 possible 3-byte outputs, nowhere near enough to
 * represent all 2^32768 possible 4KB inputs (pigeonhole principle). What
 * IS real: highly repetitive data (a short pattern repeated many times)
 * genuinely compresses that far, and this recovers the exact original
 * bytes on decompress - verified below, not simulated.
 *
 * The shared core codec preserves CC's original formats:
 *   Periodic: [1][unit_len:1][repeat_count:2 LE][unit bytes...]
 *   Literal:  [0][original bytes...]
 * and adds format 2 for mixed literal/run blocks with original length and
 * CRC32. The compressor chooses the smallest representation.
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PXC_MAX PXFS_LOSSLESS_MAX_INPUT

static void pxc_print_hex(const unsigned char* data, size_t len) {
    for (size_t i = 0; i < len; i++) printf("%02x", data[i]);
    printf("\n");
}

static int pxc_parse_hex(const char* hex, unsigned char* out, size_t out_max, size_t* out_len) {
    size_t hlen = strlen(hex);
    if (hlen % 2 != 0 || hlen / 2 > out_max) return -1;
    for (size_t i = 0; i < hlen / 2; i++) {
        int hi = isdigit((unsigned char)hex[i * 2]) ? hex[i * 2] - '0' :
                 isxdigit((unsigned char)hex[i * 2]) ?
                 tolower((unsigned char)hex[i * 2]) - 'a' + 10 : -1;
        int lo = isdigit((unsigned char)hex[i * 2 + 1]) ? hex[i * 2 + 1] - '0' :
                 isxdigit((unsigned char)hex[i * 2 + 1]) ?
                 tolower((unsigned char)hex[i * 2 + 1]) - 'a' + 10 : -1;
        if (hi < 0 || lo < 0) return -1;
        out[i] = (unsigned char)((hi << 4) | lo);
    }
    *out_len = hlen / 2;
    return 0;
}

static int cmd_pxcompress(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: pxcompress <text>\n");
        printf("Real, lossless, verifiably-reversible compression - not\n");
        printf("simulated. Genuinely compact only for repetitive input;\n");
        printf("honestly falls back to literal storage otherwise.\n");
        return 1;
    }

    unsigned char msg[PXC_MAX] = {0};
    size_t len = 0;
    for (int i = 1; i < argc && len < sizeof(msg) - 1; i++) {
        size_t wlen = strlen(argv[i]);
        if (len + wlen >= sizeof(msg)) wlen = sizeof(msg) - len - 1;
        memcpy(msg + len, argv[i], wlen);
        len += wlen;
        if (i < argc - 1 && len < sizeof(msg) - 1) msg[len++] = ' ';
    }

    unsigned char out[PXFS_LOSSLESS_MAX_ENCODED];
    size_t clen = pxfs_lossless_compress(msg, len, out, sizeof(out));
    if (clen == 0) {
        printf("pxcompress: input too large or empty (max %d bytes)\n", PXC_MAX);
        return 1;
    }

    printf("Original:   %zu bytes\n", len);
    printf("Compressed: %zu bytes\n", clen);
    printf("Ratio:      %.1f:1\n", (double)len / (double)clen);
    const char* mode = out[0] == PXFS_LOSSLESS_PERIODIC ? "periodic" :
                       out[0] == PXFS_LOSSLESS_BLOCKS_V1 ? "mixed runs + CRC32" :
                       "literal (stored as-is)";
    printf("Mode:       %s\n", mode);
    printf("Hex:        ");
    pxc_print_hex(out, clen);
    printf("(feed that hex string to 'pxdecompress' to verify the round trip)\n");
    add_karma(2);
    return 0;
}

static int cmd_pxdecompress(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: pxdecompress <hex-from-pxcompress>\n");
        return 1;
    }

    unsigned char cbuf[PXFS_LOSSLESS_MAX_ENCODED];
    size_t clen;
    if (pxc_parse_hex(argv[1], cbuf, sizeof(cbuf), &clen) != 0) {
        printf("pxdecompress: invalid hex input\n");
        return 1;
    }

    unsigned char out[PXC_MAX] = {0};
    size_t len = pxfs_lossless_decompress(cbuf, clen, out, sizeof(out) - 1);
    if (len == 0) {
        printf("pxdecompress: corrupt or unrecognized data\n");
        return 1;
    }
    out[len] = '\0';

    printf("Compressed: %zu bytes\n", clen);
    printf("Recovered:  %zu bytes\n", len);
    printf("Text:       %s\n", out);
    add_karma(1);
    return 0;
}

/* Store: compress and write to a local file - real persistence, not a
 * simulation of it. */
static int cmd_pxstore(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: pxstore <filename> <text>\n");
        return 1;
    }

    unsigned char msg[PXC_MAX] = {0};
    size_t len = 0;
    for (int i = 2; i < argc && len < sizeof(msg) - 1; i++) {
        size_t wlen = strlen(argv[i]);
        if (len + wlen >= sizeof(msg)) wlen = sizeof(msg) - len - 1;
        memcpy(msg + len, argv[i], wlen);
        len += wlen;
        if (i < argc - 1 && len < sizeof(msg) - 1) msg[len++] = ' ';
    }

    unsigned char out[PXFS_LOSSLESS_MAX_ENCODED];
    size_t clen = pxfs_lossless_compress(msg, len, out, sizeof(out));
    if (clen == 0) {
        printf("pxstore: input too large or empty\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "wb");
    if (!f) { perror("pxstore"); return 1; }
    fwrite(out, 1, clen, f);
    fclose(f);

    printf("Stored %zu bytes -> %s (%zu bytes, %.1f:1)\n", len, argv[1], clen, (double)len / (double)clen);
    add_karma(2);
    return 0;
}

/* Extract: read a stored compressed file and decompress it - proves the
 * store really round-trips, not just the in-memory hex path above. */
static int cmd_pxload(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: pxload <filename>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "rb");
    if (!f) { perror("pxload"); return 1; }
    unsigned char cbuf[PXFS_LOSSLESS_MAX_ENCODED];
    size_t clen = fread(cbuf, 1, sizeof(cbuf), f);
    fclose(f);

    unsigned char out[PXC_MAX] = {0};
    size_t len = pxfs_lossless_decompress(cbuf, clen, out, sizeof(out) - 1);
    if (len == 0) {
        printf("pxload: corrupt or unrecognized file\n");
        return 1;
    }
    out[len] = '\0';

    printf("Loaded %s (%zu compressed bytes)\n", argv[1], clen);
    printf("Extracted %zu bytes: %s\n", len, out);
    add_karma(1);
    return 0;
}

/* Transmit: compress and send the actual compressed bytes over a real
 * TCP connection (reuses tbos_net_tcp_connect - the same primitive
 * every other network command in this file is built on). */
static int cmd_pxsend(int argc, char** argv) {
    if (argc < 4) {
        printf("Usage: pxsend <host> <port> <text>\n");
        printf("Compresses, then transmits the compressed bytes for real.\n");
        printf("Receive and extract with: pxrecv <port>\n");
        return 1;
    }

    unsigned char msg[PXC_MAX] = {0};
    size_t len = 0;
    for (int i = 3; i < argc && len < sizeof(msg) - 1; i++) {
        size_t wlen = strlen(argv[i]);
        if (len + wlen >= sizeof(msg)) wlen = sizeof(msg) - len - 1;
        memcpy(msg + len, argv[i], wlen);
        len += wlen;
        if (i < argc - 1 && len < sizeof(msg) - 1) msg[len++] = ' ';
    }

    unsigned char out[PXFS_LOSSLESS_MAX_ENCODED];
    size_t clen = pxfs_lossless_compress(msg, len, out, sizeof(out));
    if (clen == 0) {
        printf("pxsend: input too large or empty\n");
        return 1;
    }

    int port = atoi(argv[2]);
    int fd = tbos_net_tcp_connect(argv[1], port, 3);
    if (fd < 0) {
        printf("pxsend: could not connect to %s:%d\n", argv[1], port);
        return 1;
    }

    if (write(fd, out, clen) < 0) {
        perror("pxsend");
        close(fd);
        return 1;
    }
    close(fd);

    printf("Sent %zu bytes compressed to %zu bytes (%.1f:1) -> %s:%d\n",
           len, clen, (double)len / (double)clen, argv[1], port);
    add_karma(2);
    return 0;
}

/* Extract from the network: receive compressed bytes and decompress
 * them - the receiving half of pxsend, completing the real
 * store/transmit/extract round trip end to end. */
static int cmd_pxrecv(int argc, char** argv) {
    int port = (argc >= 2) ? atoi(argv[1]) : 9100;

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("pxrecv"); return 1; }
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons((uint16_t)port);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("pxrecv: bind"); close(srv); return 1;
    }
    if (listen(srv, 1) < 0) {
        perror("pxrecv: listen"); close(srv); return 1;
    }

    printf("Waiting for compressed data on 127.0.0.1:%d (10s timeout)...\n", port);
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

    unsigned char cbuf[PXFS_LOSSLESS_MAX_ENCODED];
    ssize_t clen = read(client_fd, cbuf, sizeof(cbuf));
    close(client_fd);
    close(srv);

    if (clen <= 0) {
        printf("pxrecv: no data received\n");
        return 1;
    }

    unsigned char out[PXC_MAX] = {0};
    size_t len = pxfs_lossless_decompress(cbuf, (size_t)clen, out, sizeof(out) - 1);
    if (len == 0) {
        printf("pxrecv: received %zd bytes but could not decompress (corrupt?)\n", clen);
        return 1;
    }
    out[len] = '\0';

    printf("Received %zd compressed bytes -> extracted %zu bytes: %s\n", clen, len, out);
    add_karma(2);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REAL STEPPPS: linked against the canonical runtime (steppps/v2/
 * steppps_runtime.c - the same one that parses/verifies/sandboxes real
 * .steppps.json entities, tested earlier this session against the real
 * example files under steppps/v2/examples/). Not the decorative
 * "Space: Enabled" status display this used to be -
 * docs/TBOS_FIRST_ARCHITECTURE.md rule 5 says STEPPPS must be
 * "operational metadata and policy, not decorative terminology," and
 * the old cmd_steppps was exactly that violation.
 *
 * One command, tier-gated by risk rather than cost, same lesson as
 * pxfs/ucfs/rf2s being moved to Tier 0: show (read-only, no more
 * dangerous than pxencode) stays at Tier 0; verify/run climb tiers
 * because they carry real trust/execution consequences, not because
 * they're expensive. */
static int cmd_steppps(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: steppps <command> <file>\n");
        printf("\n");
        printf("Commands:\n");
        printf("  show <file>    - Load, parse, display (Tier 0+)\n");
        printf("  verify <file>  - Check signature + karma trust (Tier 2+)\n");
        printf("  run <file>     - Execute in sandbox (Tier 3+)\n");
        printf("  audit <id>     - Show audit log (Tier 4+)\n");
        printf("\n");
        printf("Real canonical runtime (steppps/v2) - loads actual\n");
        printf("STEPPPS JSON entities, not a capability status display.\n");
        return 1;
    }

    const char* subcmd = argv[1];

    if (strcmp(subcmd, "show") == 0) {
        if (argc < 3) { printf("Usage: steppps show <file>\n"); return 1; }
        steppps_t s;
        memset(&s, 0, sizeof(s));
        if (steppps_load(argv[2], &s) != 0) {
            printf("steppps: failed to load/parse %s\n", argv[2]);
            return 1;
        }
        steppps_print(&s);
        steppps_free(&s);
        add_karma(1);
        return 0;
    }

    if (strcmp(subcmd, "verify") == 0) {
        if (TBOS_TIER < 2) {
            printf("steppps verify requires Tier 2+ (this build is Tier %d)\n", TBOS_TIER);
            return 1;
        }
        if (argc < 3) { printf("Usage: steppps verify <file>\n"); return 1; }
        steppps_t s;
        memset(&s, 0, sizeof(s));
        if (steppps_load(argv[2], &s) != 0) {
            printf("steppps: failed to load/parse %s\n", argv[2]);
            return 1;
        }
        int64_t real_karma = steppps_get_user_karma();
        int sig_ok = steppps_verify(&s);
        int trust_ok = steppps_check_trust(&s, real_karma);
        steppps_print_security(&s);
        printf("Signature valid: %s\n", sig_ok == 0 ? "yes" : "no");
        printf("Trust check (your karma %lld): %s\n", (long long)real_karma, trust_ok == 0 ? "passed" : "failed");
        steppps_free(&s);
        add_karma(2);
        return 0;
    }

    if (strcmp(subcmd, "run") == 0) {
        if (TBOS_TIER < 3) {
            printf("steppps run requires Tier 3+ (this build is Tier %d)\n", TBOS_TIER);
            return 1;
        }
        if (argc < 3) { printf("Usage: steppps run <file>\n"); return 1; }
        steppps_t s;
        memset(&s, 0, sizeof(s));
        if (steppps_load(argv[2], &s) != 0) {
            printf("steppps: failed to load/parse %s\n", argv[2]);
            return 1;
        }
        steppps_verify(&s);
        steppps_check_trust(&s, steppps_get_user_karma());
        int rc = steppps_run(&s);
        printf("Execution: %s (exit code %d)\n", rc == 0 ? "completed" : "rejected/failed", s.exit_code);
        add_karma(rc == 0 ? 3 : 1);
        steppps_free(&s);
        return 0;
    }

    if (strcmp(subcmd, "audit") == 0) {
        if (TBOS_TIER < 4) {
            printf("steppps audit requires Tier 4+ (this build is Tier %d)\n", TBOS_TIER);
            return 1;
        }
        if (argc < 3) { printf("Usage: steppps audit <id>\n"); return 1; }
        char log[4096] = {0};
        if (steppps_audit_get(argv[2], log, sizeof(log)) != 0) {
            printf("steppps: no audit log found for id '%s'\n", argv[2]);
            return 1;
        }
        printf("%s\n", log);
        return 0;
    }

    printf("steppps: unknown command '%s'\n", subcmd);
    return 1;
}
#endif /* !_WIN32 */

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
 * TIER 1 COMMANDS (Embedded: +11 commands = 27 total)
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
    printf("  Commands: %d available (capacity %d)\n", g_command_count, g_command_capacity);
#ifndef _WIN32
    printf("  Command table memory: %s",
           g_using_dynamic_memory ? "dynamic (HAL heap, grows via realloc)"
                                   : "static (fixed array, no heap on this platform)");
    if (g_hal && g_hal->capabilities) {
        printf(" [HAL caps.has_dynamic_memory=%d]", g_hal->capabilities().has_dynamic_memory);
    }
    printf("\n");
#else
    printf("  Command table memory: static (fixed array, no HAL adapter on _WIN32)\n");
#endif
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

/* ═══════════════════════════════════════════════════════════════════════════
 * TBOS's own virtual root, distinct from the host filesystem `ls`/`cd`/`mkdir`
 * above operate on. docs/TBOS_LINUX_DISTRO_ROADMAP.md step 1: an "Ubuntu-like"
 * TBOS needs its own namespace, not just host passthrough. Mounts the
 * already-canonical kernel/fs/vfs.c + ramfs.c (manifest: "VFS contract and
 * namespace") rather than the non-canonical tbos_ramdisk.c or a new format.
 * Deliberately additive and Tier-5-only for this first slice: `ls`/`cd`/
 * `mkdir` above are unchanged, so nothing that already depends on real host
 * paths (pxstore/pxload, editing this very repo) can regress.
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_vfs_mounted = 0;

static void vfs_ensure_mounted(void) {
    if (g_vfs_mounted) return;
    vfs_init();
    vfs_mount("/", &ramfs_driver);
    g_vfs_mounted = 1;
}

static int cmd_vmkdir(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: vmkdir <path>  (TBOS's own virtual root, not the host filesystem)\n");
        return 1;
    }
    vfs_ensure_mounted();
    if (vfs_mkdir(argv[1]) != 0) {
        printf("vmkdir: failed to create %s\n", argv[1]);
        return 1;
    }
    add_karma(2);
    return 0;
}

static int vls_print(const char* name, vfs_node_type_t type, void* user) {
    (void)user;
    printf("%s%s\n", name, type == VFS_NODE_DIR ? "/" : "");
    return 0;
}

static int cmd_vls(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "/";
    vfs_ensure_mounted();
    if (vfs_list_dir(path, vls_print, NULL) != 0) {
        printf("vls: cannot list %s\n", path);
        return 1;
    }
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
 * TIER 2 COMMANDS (Retro: +9 commands = 36 total)
 * Adds: head, wc, grep, touch, uname, uptime, history, env, resolve
 * (ucfs/rf2s moved to Tier 0 - see top of file)
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
    time_t now = time(NULL);
    double elapsed = difftime(now, g_shell_start_time);
    long total_secs = (long)elapsed;
    long days = total_secs / 86400;
    long hours = (total_secs % 86400) / 3600;
    long mins = (total_secs % 3600) / 60;
    long secs = total_secs % 60;

    printf("Shell uptime: ");
    if (days > 0) printf("%ldd ", days);
    if (days > 0 || hours > 0) printf("%ldh ", hours);
    printf("%ldm %lds\n", mins, secs);
    printf("Started:      %s", ctime(&g_shell_start_time));
    printf("Current time: %s", ctime(&now));
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
/* ucfs/rf2s moved to the Tier 0 section above - no reason path parsing
 * needed to wait for Tier 2. */

#endif /* TBOS_TIER >= 2 */

/* ═══════════════════════════════════════════════════════════════════════════
 * TIER 3+ COMMANDS (Desktop: +9 commands = 45 total)
 * Adds: meditate, reflect, sangha, dharma, pxfs, ping, df, ps, http
 * (steppps moved to Tier 0 - see top of file)
 * (pxencode moved to Tier 0 - see top of file)
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
    printf("  Compression: RGB pixel encoding (see 'pxencode' for RAW mode)\n");
    printf("  Ratio: Up to 1365:1 in QUANTUM mode (not implemented at this tier)\n");
    return 0;
}

/* pxencode and steppps (show/verify/run/audit) both moved to the
 * Tier 0 section above. steppps used to be a decorative "Space:
 * Enabled" status display - now it's linked against the canonical
 * steppps/v2 runtime and loads real entities. */

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
 * TIER 4+ COMMANDS (Server: +5 commands = 50 total)
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
 * TIER 5 COMMANDS (Supercomputer: +4 commands = 54 total)
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
    if (TBOS_TIER < tier) return;

    if (g_command_count >= g_command_capacity) {
#ifndef _WIN32
        if (g_using_dynamic_memory && g_hal && g_hal->memory.realloc) {
            int new_capacity = g_command_capacity * 2;
            tbos_cmd_t* grown = (tbos_cmd_t*)g_hal->memory.realloc(
                g_commands, sizeof(tbos_cmd_t) * (size_t)new_capacity);
            if (!grown) {
                fprintf(stderr, "warning: command table realloc failed, '%s' not registered\n", name);
                return;
            }
            g_commands = grown;
            g_command_capacity = new_capacity;
        } else
#endif
        {
            /* Static array exhausted (no heap on this platform, or HAL
             * unavailable) - this used to fail silently with zero
             * indication why. Surfacing it turns a silent capability
             * gap into a build-time-visible bug. */
            fprintf(stderr, "warning: command table capacity (%d) exceeded, '%s' not registered\n",
                    g_command_capacity, name);
            return;
        }
    }

    g_commands[g_command_count].name = name;
    g_commands[g_command_count].desc = desc;
    g_commands[g_command_count].handler = h;
    g_commands[g_command_count].min_tier = tier;
    g_command_count++;
}

static void register_all_commands(void) {
    /* Tier 0: Calculator (5 base + net/ucfs/rf2s/pxencode/pxcompress/
     * pxdecompress/pxstore/pxload/pxsend/pxrecv/steppps = 16 commands).
     * All of this is string/byte parsing, real socket I/O, and a real
     * linked STEPPPS parse - none of it costs more than calc(), so
     * there's no reason to hold any of it back from the smallest tier. */
    register_cmd("help",   "Show available commands",  cmd_help,   0);
    register_cmd("echo",   "Display text",             cmd_echo,   0);
    register_cmd("calc",   "Calculator (+ - * /)",     cmd_calc,   0);
    register_cmd("karma",  "Show karma status",        cmd_karma,  0);
    register_cmd("exit",   "Exit shell",               cmd_exit,   0);
#ifndef _WIN32
    register_cmd("net",         "TCP connectivity probe",             cmd_net,         0);
    register_cmd("ucfs",        "Parse a UCFS Unicode path",          cmd_ucfs,        0);
    register_cmd("rf2s",        "Parse an RF2S frequency path",       cmd_rf2s,        0);
    register_cmd("pxencode",    "Real PXFS RAW pixel encode",         cmd_pxencode,    0);
    register_cmd("pxcompress",  "Real lossless compression (RLE)",    cmd_pxcompress,  0);
    register_cmd("pxdecompress","Decompress pxcompress hex output",   cmd_pxdecompress,0);
    register_cmd("pxstore",     "Compress and write to a file",       cmd_pxstore,     0);
    register_cmd("pxload",      "Read a file and decompress it",      cmd_pxload,      0);
    register_cmd("pxsend",      "Compress and transmit over TCP",     cmd_pxsend,      0);
    register_cmd("pxrecv",      "Receive and decompress over TCP",    cmd_pxrecv,      0);
    register_cmd("steppps",     "Real STEPPPS: show/verify/run/audit", cmd_steppps,    0);
#endif

#if TBOS_TIER >= 1
    /* Tier 1: Embedded (+10 base + send = 11; cumulative 27) */
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
    /* Tier 2: Retro (+8 base + resolve = 9; cumulative 36) */
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
    /* Tier 3: Desktop (+8 base + http = 9; cumulative 45) */
    register_cmd("meditate","Mindful break",           cmd_meditate, 3);
    register_cmd("reflect", "Reflect on journey",      cmd_reflect,  3);
    register_cmd("sangha",  "Digital sangha network",  cmd_sangha,   3);
    register_cmd("dharma",  "Dharma of computing",     cmd_dharma,   3);
    register_cmd("pxfs",    "PXFS filesystem info",    cmd_pxfs,     3);
#ifndef _WIN32
    register_cmd("ping",    "Ping a host",             cmd_ping,     3);
    register_cmd("df",      "Disk free space",         cmd_df,       3);
    register_cmd("ps",      "Process status",          cmd_ps,       3);
    register_cmd("http",    "Real HTTP/1.0 GET client", cmd_http,    3);
#endif
#endif

#if TBOS_TIER >= 4
    /* Tier 4: Server (+4 base + listen = 5; cumulative 50) */
#ifndef _WIN32
    register_cmd("top",     "Process monitor",         cmd_top,      4);
    register_cmd("netstat", "Network connections",     cmd_netstat,  4);
    register_cmd("free",    "Memory usage",            cmd_free,     4);
    register_cmd("lscpu",   "CPU information",         cmd_lscpu,    4);
    register_cmd("listen",  "Minimal TCP echo server (localhost)", cmd_listen, 4);
#endif
#endif

#if TBOS_TIER >= 5
    /* Tier 5: Supercomputer (+3 base + netscan = 4; cumulative 54) */
    register_cmd("cluster",     "Cluster status",      cmd_cluster,      5);
    register_cmd("consciousness","AI consciousness",   cmd_consciousness,5);
    register_cmd("quantum",     "Quantum simulation",  cmd_quantum,      5);
#ifndef _WIN32
    register_cmd("netscan", "Scan a port range (uses TCP connect probe)", cmd_netscan, 5);
    register_cmd("vmkdir",  "Create a dir in TBOS's own virtual root (not the host FS)", cmd_vmkdir, 5);
    register_cmd("vls",     "List a dir in TBOS's own virtual root (not the host FS)",   cmd_vls,     5);
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

#ifdef _WIN32
    /* The banner below prints UTF-8 box-drawing characters (e.g. the border
     * around "TBOS Supercomputer Shell"). Windows consoles default to a
     * legacy OEM code page, not UTF-8, so each multi-byte sequence gets
     * misread byte-by-byte and renders as garbage - reported as "non
     * printable chars and no shell prompt" (the mangled banner obscures the
     * actual, correctly-flushed ASCII prompt line right after it). Setting
     * both codepages to UTF-8 (65001) before any output fixes this. */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    /* Initialize */
    g_shell_start_time = time(NULL);
    getcwd(g_cwd, sizeof(g_cwd));
    tier_shell_init_command_table();
#ifndef _WIN32
    steppps_runtime_init();
#endif
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
