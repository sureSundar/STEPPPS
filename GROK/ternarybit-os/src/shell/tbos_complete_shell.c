/**
 * TBOS Complete Shell - Runs on ANY Operating System
 *
 * The full TernaryBit OS experience running on top of:
 * - Linux (native, WSL, Docker)
 * - macOS (Intel & Apple Silicon)
 * - Any POSIX-compliant system
 *
 * Features:
 * - 112+ TBOS commands
 * - Karma & Consciousness tracking
 * - PXFS compression filesystem
 * - WOW universal protocol
 * - Host shell integration
 * - Full pipe/redirect support
 *
 * Build:
 *   ./scripts/build_tbos_shell.sh
 *
 * Or manually:
 *   gcc -DTBOS_HOSTED -o tbos src/shell/tbos_complete_shell.c -Iinclude -Isrc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>

/* Cross-platform support */
#include "tbos/platform.h"

/* STEPPPS integration */
#ifdef STEPPPS_INTEGRATED
#include "steppps/v2/steppps.h"
#endif

/* UCFS integration */
#ifdef UCFS_INTEGRATED
#include "fs/ucfs_codec.h"
#endif

/* Virtual filesystem mount point */
#define TBOS_VFS_PREFIX "/tbos"
#define VFS_MAX_NODES 256
#define VFS_MAX_NAME 256
#define VFS_MAX_DATA 65536

#ifdef TBOS_PLATFORM_WINDOWS
    #include <io.h>
    #include <process.h>
    #define popen _popen
    #define pclose _pclose
    /* Windows compatibility shims */
    #define fork() (-1)  /* fork not supported, will fall back to system() */
    #define waitpid(pid, status, opts) ((void)(status), 0)
    #define WEXITSTATUS(status) (status)
    #define pid_t int
    #define mkdir(path, mode) _mkdir(path)
    #define gethostname(buf, len) tbos_gethostname(buf, len)
    #define environ _environ
#else
    #include <unistd.h>
    #include <signal.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <termios.h>
    #include <sys/ioctl.h>
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * CONFIGURATION
 * ═══════════════════════════════════════════════════════════════════════════ */

#define TBOS_VERSION        "1.0.0"
#define MAX_INPUT           4096
#define MAX_ARGS            256
#ifndef MAX_PATH
#define MAX_PATH            4096
#endif
#define TBOS_MAX_PATH       4096
#define MAX_COMMANDS        256
#define HISTORY_SIZE        100

/* ═══════════════════════════════════════════════════════════════════════════
 * TYPES & STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum {
    CMD_CAT_CORE = 0,
    CMD_CAT_FILE,
    CMD_CAT_TEXT,
    CMD_CAT_PROCESS,
    CMD_CAT_NETWORK,
    CMD_CAT_SYSTEM,
    CMD_CAT_PXFS,
    CMD_CAT_CONSCIOUSNESS,
    CMD_CAT_HOST
} cmd_category_t;

typedef int (*cmd_handler_t)(int argc, char** argv);

typedef struct {
    const char* name;
    const char* description;
    cmd_handler_t handler;
    cmd_category_t category;
    int karma_reward;
} tbos_command_t;

typedef enum {
    HOST_UNKNOWN, HOST_LINUX, HOST_UBUNTU, HOST_MACOS, HOST_WSL
} host_os_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * GLOBAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

static int g_running = 1;
static int g_karma = 100;
static char g_cwd[MAX_PATH];
static char g_hostname[256];
static char g_username[256];
static host_os_t g_host_os;
static uint64_t g_commands_executed = 0;
static time_t g_session_start;

/* Command registry */
static tbos_command_t g_commands[MAX_COMMANDS];
static int g_command_count = 0;

/* History */
static char g_history[HISTORY_SIZE][MAX_INPUT];
static int g_history_count = 0;

/* Forward declarations */
static int execute_via_host_shell(const char* line);

/* ═══════════════════════════════════════════════════════════════════════════
 * SIMPLE IN-MEMORY VIRTUAL FILESYSTEM
 * ═══════════════════════════════════════════════════════════════════════════ */

typedef enum { VFS_DIR, VFS_FILE } vfs_type_t;

typedef struct vfs_node {
    char name[VFS_MAX_NAME];
    vfs_type_t type;
    unsigned int mode;      /* Unix permissions (0755, 0644, etc.) */
    unsigned int uid;       /* Owner user ID */
    unsigned int gid;       /* Owner group ID */
    time_t mtime;           /* Modification time */
    char* data;
    size_t size;
    size_t capacity;
    struct vfs_node* parent;
    struct vfs_node* children[64];
    int child_count;
} vfs_node_t;

static vfs_node_t* g_vfs_root = NULL;
static int g_vfs_initialized = 0;

static vfs_node_t* vfs_create_node(const char* name, vfs_type_t type, vfs_node_t* parent) {
    vfs_node_t* node = (vfs_node_t*)calloc(1, sizeof(vfs_node_t));
    if (!node) return NULL;
    strncpy(node->name, name, VFS_MAX_NAME - 1);
    node->type = type;
    node->mode = (type == VFS_DIR) ? 0755 : 0644;  /* Default permissions */
    node->uid = 0;  /* root */
    node->gid = 0;  /* root */
    node->mtime = time(NULL);
    node->parent = parent;
    if (parent && parent->child_count < 64) {
        parent->children[parent->child_count++] = node;
    }
    return node;
}

static vfs_node_t* vfs_find_node(const char* path) {
    if (!g_vfs_root || !path) return NULL;

    /* Skip prefix */
    if (strncmp(path, TBOS_VFS_PREFIX, strlen(TBOS_VFS_PREFIX)) == 0) {
        path += strlen(TBOS_VFS_PREFIX);
    }
    if (*path == '/') path++;
    if (*path == '\0') return g_vfs_root;

    vfs_node_t* current = g_vfs_root;
    char component[VFS_MAX_NAME];

    while (*path && current) {
        const char* slash = strchr(path, '/');
        size_t len = slash ? (size_t)(slash - path) : strlen(path);
        if (len >= VFS_MAX_NAME) return NULL;
        strncpy(component, path, len);
        component[len] = '\0';

        vfs_node_t* found = NULL;
        for (int i = 0; i < current->child_count; i++) {
            if (strcmp(current->children[i]->name, component) == 0) {
                found = current->children[i];
                break;
            }
        }
        if (!found) return NULL;
        current = found;
        path = slash ? slash + 1 : path + len;
    }
    return current;
}

static int vfs_mkdir(const char* path) {
    if (!g_vfs_root) return -1;

    /* Find parent */
    char parent_path[MAX_PATH];
    strncpy(parent_path, path, MAX_PATH - 1);
    char* last_slash = strrchr(parent_path, '/');
    if (!last_slash) return -1;

    const char* name = last_slash + 1;
    *last_slash = '\0';

    vfs_node_t* parent = vfs_find_node(parent_path);
    if (!parent || parent->type != VFS_DIR) return -1;

    /* Check if already exists */
    for (int i = 0; i < parent->child_count; i++) {
        if (strcmp(parent->children[i]->name, name) == 0) return 0;
    }

    vfs_node_t* dir = vfs_create_node(name, VFS_DIR, parent);
    return dir ? 0 : -1;
}

static int vfs_write_file(const char* path, const void* data, size_t size) {
    if (!g_vfs_root) return -1;

    vfs_node_t* node = vfs_find_node(path);
    if (node && node->type == VFS_FILE) {
        /* Overwrite */
        if (size > node->capacity) {
            char* new_data = (char*)realloc(node->data, size + 1);
            if (!new_data) return -1;
            node->data = new_data;
            node->capacity = size;
        }
        memcpy(node->data, data, size);
        node->data[size] = '\0';
        node->size = size;
        return 0;
    }

    /* Create new file */
    char parent_path[MAX_PATH];
    strncpy(parent_path, path, MAX_PATH - 1);
    char* last_slash = strrchr(parent_path, '/');
    if (!last_slash) return -1;

    const char* name = last_slash + 1;
    *last_slash = '\0';

    vfs_node_t* parent = vfs_find_node(parent_path);
    if (!parent || parent->type != VFS_DIR) return -1;

    node = vfs_create_node(name, VFS_FILE, parent);
    if (!node) return -1;

    if (size > 0) {
        node->data = (char*)malloc(size + 1);
        if (!node->data) return -1;
        memcpy(node->data, data, size);
        node->data[size] = '\0';
        node->size = size;
        node->capacity = size;
    }
    return 0;
}

static const void* vfs_read_file_cstr(const char* path, size_t* out_size) {
    vfs_node_t* node = vfs_find_node(path);
    if (!node || node->type != VFS_FILE) return NULL;
    if (out_size) *out_size = node->size;
    return node->data ? node->data : "";
}

static int vfs_remove(const char* path, int recursive) {
    (void)recursive;
    vfs_node_t* node = vfs_find_node(path);
    if (!node || node == g_vfs_root) return -1;

    /* Remove from parent */
    vfs_node_t* parent = node->parent;
    if (parent) {
        for (int i = 0; i < parent->child_count; i++) {
            if (parent->children[i] == node) {
                memmove(&parent->children[i], &parent->children[i+1],
                        (parent->child_count - i - 1) * sizeof(vfs_node_t*));
                parent->child_count--;
                break;
            }
        }
    }
    if (node->data) free(node->data);
    free(node);
    return 0;
}

typedef int (*vfs_list_cb)(const char* name, int is_dir, void* user);

static int vfs_list_dir(const char* path, vfs_list_cb cb, void* user) {
    vfs_node_t* node = vfs_find_node(path);
    if (!node || node->type != VFS_DIR) return -1;

    for (int i = 0; i < node->child_count; i++) {
        cb(node->children[i]->name, node->children[i]->type == VFS_DIR, user);
    }
    return 0;
}

static int vfs_chmod(const char* path, unsigned int mode) {
    vfs_node_t* node = vfs_find_node(path);
    if (!node) return -1;
    node->mode = mode & 07777;  /* Only permission bits */
    return 0;
}

static int vfs_chown(const char* path, unsigned int uid, unsigned int gid) {
    vfs_node_t* node = vfs_find_node(path);
    if (!node) return -1;
    node->uid = uid;
    node->gid = gid;
    return 0;
}

typedef struct {
    vfs_type_t type;
    unsigned int mode;
    unsigned int uid;
    unsigned int gid;
    size_t size;
    time_t mtime;
} vfs_stat_t;

static int vfs_stat(const char* path, vfs_stat_t* st) {
    vfs_node_t* node = vfs_find_node(path);
    if (!node || !st) return -1;
    st->type = node->type;
    st->mode = node->mode;
    st->uid = node->uid;
    st->gid = node->gid;
    st->size = node->size;
    st->mtime = node->mtime;
    return 0;
}

/* Check if path is in virtual filesystem */
static int is_vfs_path(const char* path) {
    if (!path) return 0;
    size_t prefix_len = strlen(TBOS_VFS_PREFIX);
    return (strncmp(path, TBOS_VFS_PREFIX, prefix_len) == 0 &&
            (path[prefix_len] == '/' || path[prefix_len] == '\0'));
}

/* Initialize in-memory VFS */
static void init_vfs(void) {
    if (g_vfs_initialized) return;

    g_vfs_root = vfs_create_node("tbos", VFS_DIR, NULL);
    if (!g_vfs_root) return;

    g_vfs_initialized = 1;

    /* Create default directories */
    vfs_mkdir(TBOS_VFS_PREFIX "/home");
    vfs_mkdir(TBOS_VFS_PREFIX "/tmp");
    vfs_mkdir(TBOS_VFS_PREFIX "/bin");
    vfs_mkdir(TBOS_VFS_PREFIX "/etc");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LINE EDITOR WITH ESCAPE KEY HANDLING
 * ═══════════════════════════════════════════════════════════════════════════ */

#ifndef TBOS_PLATFORM_WINDOWS

/* Terminal state */
static struct termios g_orig_termios;
static int g_raw_mode = 0;

/* Key codes */
#define KEY_NULL     0
#define KEY_CTRL_A   1
#define KEY_CTRL_B   2
#define KEY_CTRL_C   3
#define KEY_CTRL_D   4
#define KEY_CTRL_E   5
#define KEY_CTRL_F   6
#define KEY_CTRL_H   8
#define KEY_TAB      9
#define KEY_CTRL_K   11
#define KEY_CTRL_L   12
#define KEY_ENTER    13
#define KEY_CTRL_N   14
#define KEY_CTRL_P   16
#define KEY_CTRL_U   21
#define KEY_CTRL_W   23
#define KEY_ESC      27
#define KEY_BACKSPACE 127

/* Special keys (values > 256 to distinguish from regular chars) */
#define KEY_ARROW_UP    1000
#define KEY_ARROW_DOWN  1001
#define KEY_ARROW_RIGHT 1002
#define KEY_ARROW_LEFT  1003
#define KEY_HOME        1004
#define KEY_END         1005
#define KEY_DELETE      1006
#define KEY_PAGE_UP     1007
#define KEY_PAGE_DOWN   1008

static void disable_raw_mode(void) {
    if (g_raw_mode) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
        g_raw_mode = 0;
    }
}

static int g_force_line_mode = 1;  /* Default to line mode for compatibility */

static int enable_raw_mode(void) {
    /* Skip raw mode if forced to line mode or not a real TTY */
    if (g_force_line_mode) return -1;
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) return -1;

    if (g_raw_mode) return 0;

    /* Try to get current terminal settings - if this fails, not a real terminal */
    if (tcgetattr(STDIN_FILENO, &g_orig_termios) == -1) return -1;

    /* Check if canonical mode is already off (unlikely for real terminal) */
    if (!(g_orig_termios.c_lflag & ICANON)) return -1;

    struct termios raw = g_orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) return -1;
    g_raw_mode = 1;
    return 0;
}

static int read_key(void) {
    char c;
    ssize_t nread;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) return -1;
    }

    if (c == KEY_ESC) {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_ESC;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDIN_FILENO, &seq[2], 1) != 1) return KEY_ESC;
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return KEY_HOME;
                        case '3': return KEY_DELETE;
                        case '4': return KEY_END;
                        case '5': return KEY_PAGE_UP;
                        case '6': return KEY_PAGE_DOWN;
                        case '7': return KEY_HOME;
                        case '8': return KEY_END;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return KEY_ARROW_UP;
                    case 'B': return KEY_ARROW_DOWN;
                    case 'C': return KEY_ARROW_RIGHT;
                    case 'D': return KEY_ARROW_LEFT;
                    case 'H': return KEY_HOME;
                    case 'F': return KEY_END;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return KEY_HOME;
                case 'F': return KEY_END;
            }
        }
        return KEY_ESC;
    }
    return (unsigned char)c;
}

static void refresh_line(const char* prompt, char* buf, size_t len, size_t pos) {
    char seq[64];
    size_t plen = strlen(prompt);

    /* Move cursor to left edge */
    snprintf(seq, sizeof(seq), "\r");
    write(STDOUT_FILENO, seq, strlen(seq));

    /* Write prompt */
    write(STDOUT_FILENO, prompt, plen);

    /* Write buffer */
    write(STDOUT_FILENO, buf, len);

    /* Erase to right */
    snprintf(seq, sizeof(seq), "\033[K");
    write(STDOUT_FILENO, seq, strlen(seq));

    /* Move cursor to position */
    snprintf(seq, sizeof(seq), "\r\033[%zuC", plen + pos);
    write(STDOUT_FILENO, seq, strlen(seq));
}

static int tbos_readline(const char* prompt, char* buf, size_t buflen) {
    size_t len = 0;
    size_t pos = 0;
    int history_index = g_history_count;
    char history_save[MAX_INPUT] = {0};

    buf[0] = '\0';

    if (enable_raw_mode() == -1) {
        /* Fallback to fgets if raw mode fails */
        write(STDOUT_FILENO, prompt, strlen(prompt));
        if (!fgets(buf, buflen, stdin)) return -1;
        size_t slen = strlen(buf);
        if (slen > 0 && buf[slen-1] == '\n') buf[slen-1] = '\0';
        return strlen(buf);
    }

    write(STDOUT_FILENO, prompt, strlen(prompt));

    while (1) {
        int c = read_key();

        switch (c) {
            case KEY_ENTER:
            case '\n':  /* Handle both CR and LF as enter */
                disable_raw_mode();
                write(STDOUT_FILENO, "\n", 1);
                return len;

            case KEY_CTRL_C:
                disable_raw_mode();
                write(STDOUT_FILENO, "^C\n", 3);
                buf[0] = '\0';
                return 0;

            case KEY_CTRL_D:
                if (len == 0) {
                    disable_raw_mode();
                    return -1;  /* EOF */
                }
                break;

            case KEY_BACKSPACE:
            case KEY_CTRL_H:
                if (pos > 0) {
                    memmove(buf + pos - 1, buf + pos, len - pos);
                    pos--;
                    len--;
                    buf[len] = '\0';
                    refresh_line(prompt, buf, len, pos);
                }
                break;

            case KEY_DELETE:
                if (pos < len) {
                    memmove(buf + pos, buf + pos + 1, len - pos - 1);
                    len--;
                    buf[len] = '\0';
                    refresh_line(prompt, buf, len, pos);
                }
                break;

            case KEY_ARROW_LEFT:
            case KEY_CTRL_B:
                if (pos > 0) {
                    pos--;
                    refresh_line(prompt, buf, len, pos);
                }
                break;

            case KEY_ARROW_RIGHT:
            case KEY_CTRL_F:
                if (pos < len) {
                    pos++;
                    refresh_line(prompt, buf, len, pos);
                }
                break;

            case KEY_HOME:
            case KEY_CTRL_A:
                pos = 0;
                refresh_line(prompt, buf, len, pos);
                break;

            case KEY_END:
            case KEY_CTRL_E:
                pos = len;
                refresh_line(prompt, buf, len, pos);
                break;

            case KEY_ARROW_UP:
            case KEY_CTRL_P:
                if (history_index > 0) {
                    if (history_index == g_history_count) {
                        strncpy(history_save, buf, MAX_INPUT - 1);
                    }
                    history_index--;
                    strncpy(buf, g_history[history_index], buflen - 1);
                    len = strlen(buf);
                    pos = len;
                    refresh_line(prompt, buf, len, pos);
                }
                break;

            case KEY_ARROW_DOWN:
            case KEY_CTRL_N:
                if (history_index < g_history_count) {
                    history_index++;
                    if (history_index == g_history_count) {
                        strncpy(buf, history_save, buflen - 1);
                    } else {
                        strncpy(buf, g_history[history_index], buflen - 1);
                    }
                    len = strlen(buf);
                    pos = len;
                    refresh_line(prompt, buf, len, pos);
                }
                break;

            case KEY_CTRL_U:
                /* Clear line before cursor */
                memmove(buf, buf + pos, len - pos);
                len -= pos;
                pos = 0;
                buf[len] = '\0';
                refresh_line(prompt, buf, len, pos);
                break;

            case KEY_CTRL_K:
                /* Clear line after cursor */
                len = pos;
                buf[len] = '\0';
                refresh_line(prompt, buf, len, pos);
                break;

            case KEY_CTRL_W:
                /* Delete word before cursor */
                if (pos > 0) {
                    size_t old_pos = pos;
                    while (pos > 0 && buf[pos-1] == ' ') pos--;
                    while (pos > 0 && buf[pos-1] != ' ') pos--;
                    memmove(buf + pos, buf + old_pos, len - old_pos);
                    len -= (old_pos - pos);
                    buf[len] = '\0';
                    refresh_line(prompt, buf, len, pos);
                }
                break;

            case KEY_CTRL_L:
                /* Clear screen */
                write(STDOUT_FILENO, "\033[H\033[2J", 7);
                refresh_line(prompt, buf, len, pos);
                break;

            case KEY_TAB:
                /* Tab completion placeholder */
                break;

            default:
                if (c >= 32 && c < 127) {
                    if (len < buflen - 1) {
                        if (pos == len) {
                            buf[len] = c;
                            len++;
                            pos++;
                            buf[len] = '\0';
                            write(STDOUT_FILENO, &(char){c}, 1);
                        } else {
                            memmove(buf + pos + 1, buf + pos, len - pos);
                            buf[pos] = c;
                            len++;
                            pos++;
                            buf[len] = '\0';
                            refresh_line(prompt, buf, len, pos);
                        }
                    }
                }
                break;
        }
    }
}

#else /* TBOS_PLATFORM_WINDOWS */

static int tbos_readline(const char* prompt, char* buf, size_t buflen) {
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, buflen, stdin)) return -1;
    size_t len = strlen(buf);
    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
    return strlen(buf);
}

#endif /* TBOS_PLATFORM_WINDOWS */

/* ═══════════════════════════════════════════════════════════════════════════
 * CONSCIOUSNESS SYSTEM
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* get_consciousness_level(void) {
    if (g_karma >= 500) return "Enlightened";
    if (g_karma >= 300) return "Compassionate";
    if (g_karma >= 150) return "Aware";
    if (g_karma >= 50)  return "Awakening";
    return "Dormant";
}

static const char* get_karma_color(void) {
    if (g_karma >= 200) return "\033[1;32m";  /* Bright green */
    if (g_karma >= 100) return "\033[1;33m";  /* Yellow */
    return "\033[1;31m";                       /* Red */
}

static void add_karma(int amount, const char* reason) {
    g_karma += amount;
    if (g_karma < 0) g_karma = 0;
    if (reason && amount != 0) {
        printf("  %s%+d karma\033[0m: %s\n",
               amount > 0 ? "\033[32m" : "\033[31m", amount, reason);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HOST OS DETECTION
 * ═══════════════════════════════════════════════════════════════════════════ */

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

static const char* host_os_name(void) {
    switch (g_host_os) {
        case HOST_UBUNTU: return "Ubuntu";
        case HOST_LINUX:  return "Linux";
        case HOST_MACOS:  return "macOS";
        case HOST_WSL:    return "WSL";
        default:          return "Unknown";
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

static void register_command(const char* name, const char* desc,
                            cmd_handler_t handler, cmd_category_t cat, int karma) {
    if (g_command_count >= MAX_COMMANDS) return;
    g_commands[g_command_count].name = name;
    g_commands[g_command_count].description = desc;
    g_commands[g_command_count].handler = handler;
    g_commands[g_command_count].category = cat;
    g_commands[g_command_count].karma_reward = karma;
    g_command_count++;
}

static tbos_command_t* find_command(const char* name) {
    for (int i = 0; i < g_command_count; i++) {
        if (strcmp(g_commands[i].name, name) == 0) {
            return &g_commands[i];
        }
    }
    return NULL;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CORE COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n");
    printf("\033[1;36m╔═══════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║               TBOS Complete Shell v%s                      ║\033[0m\n", TBOS_VERSION);
    printf("\033[1;36m║          Conscious Computing on Any Platform                  ║\033[0m\n");
    printf("\033[1;36m╠═══════════════════════════════════════════════════════════════╣\033[0m\n");

    const char* categories[] = {
        "Core", "File", "Text", "Process", "Network", "System", "PXFS", "Consciousness", "Host"
    };

    for (int cat = 0; cat <= CMD_CAT_HOST; cat++) {
        printf("\033[1;33m║  %s Commands:\033[0m\n", categories[cat]);
        for (int i = 0; i < g_command_count; i++) {
            if (g_commands[i].category == cat) {
                printf("║    \033[1;32m%-12s\033[0m - %s\n",
                       g_commands[i].name, g_commands[i].description);
            }
        }
    }

    printf("\033[1;36m╠═══════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;36m║  Any command not found is passed to the host shell            ║\033[0m\n");
    printf("\033[1;36m║  Pipes (|) and redirects (>, <) work via host shell           ║\033[0m\n");
    printf("\033[1;36m╚═══════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");

    return 0;
}

static int cmd_exit(int argc, char** argv) {
    (void)argc; (void)argv;
    g_running = 0;
    return 0;
}

static int cmd_karma(int argc, char** argv) {
    (void)argc; (void)argv;

    time_t now = time(NULL);
    int session_mins = (int)((now - g_session_start) / 60);

    printf("\n");
    printf("\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;35m║  Consciousness Status                                      ║\033[0m\n");
    printf("\033[1;35m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Karma Balance:     %s%-6d\033[0m                               ║\n",
           get_karma_color(), g_karma);
    printf("║  Consciousness:     %-20s                   ║\n", get_consciousness_level());
    printf("║  Commands Executed: %-6lu                               ║\n",
           (unsigned long)g_commands_executed);
    printf("║  Session Duration:  %d minutes                            ║\n", session_mins);
    printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");

    return 0;
}

static int cmd_meditate(int argc, char** argv) {
    int seconds = (argc > 1) ? atoi(argv[1]) : 5;
    if (seconds < 1) seconds = 1;
    if (seconds > 60) seconds = 60;

    printf("\n");
    printf("\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;35m║  TBOS Meditation                                           ║\033[0m\n");
    printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\nTaking a mindful moment...\n\n");

    for (int i = 0; i < seconds; i++) {
        printf("  \xE0\xA5\x90 ");  /* Om symbol */
        fflush(stdout);
        tbos_sleep_ms(1000);
    }

    printf("\n\nMeditation complete.\n");
    add_karma(seconds * 2, "Mindful computing practice");
    printf("\n");

    return 0;
}

static int cmd_hostinfo(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n");
    printf("\033[1;34m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;34m║  Host System Information                                   ║\033[0m\n");
    printf("\033[1;34m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Host OS:      %-44s ║\n", host_os_name());
    printf("║  Hostname:     %-44s ║\n", g_hostname);
    printf("║  User:         %-44s ║\n", g_username);
    printf("║  Working Dir:  %-44s ║\n", g_cwd);
    printf("║  TBOS Karma:   %-6d                                      ║\n", g_karma);
    printf("\033[1;34m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * FILE COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Format mode string like ls -l */
static void format_mode(unsigned int mode, int is_dir, char* buf) {
    buf[0] = is_dir ? 'd' : '-';
    buf[1] = (mode & 0400) ? 'r' : '-';
    buf[2] = (mode & 0200) ? 'w' : '-';
    buf[3] = (mode & 0100) ? 'x' : '-';
    buf[4] = (mode & 0040) ? 'r' : '-';
    buf[5] = (mode & 0020) ? 'w' : '-';
    buf[6] = (mode & 0010) ? 'x' : '-';
    buf[7] = (mode & 0004) ? 'r' : '-';
    buf[8] = (mode & 0002) ? 'w' : '-';
    buf[9] = (mode & 0001) ? 'x' : '-';
    buf[10] = '\0';
}

/* VFS list callback for cmd_ls */
static int vfs_ls_callback(const char* name, int is_dir, void* user) {
    (void)user;
    if (is_dir) {
        printf("\033[1;34m%s/\033[0m\n", name);
    } else {
        printf("%s\n", name);
    }
    return 0;
}

/* Long listing callback with permissions */
static int vfs_ls_long_callback(const char* name, int is_dir, void* user) {
    const char* base_path = (const char*)user;
    char full_path[MAX_PATH];
    snprintf(full_path, sizeof(full_path), "%s/%s", base_path, name);

    vfs_stat_t st;
    if (vfs_stat(full_path, &st) == 0) {
        char mode_str[12];
        format_mode(st.mode, is_dir, mode_str);
        if (is_dir) {
            printf("%s %3u %3u %6zu  \033[1;34m%s/\033[0m\n",
                   mode_str, st.uid, st.gid, st.size, name);
        } else {
            printf("%s %3u %3u %6zu  %s\n",
                   mode_str, st.uid, st.gid, st.size, name);
        }
    }
    return 0;
}

static int cmd_ls(int argc, char** argv) {
    int long_format = 0;
    const char* path = ".";

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "-la") == 0 ||
            strcmp(argv[i], "-al") == 0) {
            long_format = 1;
        } else if (argv[i][0] != '-') {
            path = argv[i];
        }
    }

    /* Check for VFS path */
    if (is_vfs_path(path)) {
        if (long_format) {
            if (vfs_list_dir(path, vfs_ls_long_callback, (void*)path) != 0) {
                fprintf(stderr, "ls: cannot access '%s'\n", path);
                return 1;
            }
        } else {
            if (vfs_list_dir(path, vfs_ls_callback, NULL) != 0) {
                fprintf(stderr, "ls: cannot access '%s'\n", path);
                return 1;
            }
        }
        return 0;
    }

    /* Host filesystem */
    tbos_dir_t* dir = tbos_opendir(path);
    if (!dir) {
        perror("ls");
        return 1;
    }

    tbos_dirent_t* entry;
    while ((entry = tbos_readdir(dir)) != NULL) {
        if (entry->name[0] == '.' && argc <= 1) continue;

        if (entry->is_dir) {
            printf("\033[1;34m%s/\033[0m\n", entry->name);
        } else {
            printf("%s\n", entry->name);
        }
    }

    tbos_closedir(dir);
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

static int cmd_pwd(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("%s\n", g_cwd);
    return 0;
}

static int cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: cat <file>\n");
        return 1;
    }

    /* Check for VFS path */
    if (is_vfs_path(argv[1])) {
        size_t size = 0;
        const void* data = vfs_read_file_cstr(argv[1], &size);
        if (!data) {
            fprintf(stderr, "cat: cannot read '%s'\n", argv[1]);
            return 1;
        }
        fwrite(data, 1, size, stdout);
        return 0;
    }

    /* Host filesystem */
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("cat");
        return 1;
    }

    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        printf("%s", buf);
    }

    fclose(f);
    return 0;
}

static int cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: mkdir <directory>\n");
        return 1;
    }

    if (is_vfs_path(argv[1])) {
        if (vfs_mkdir(argv[1]) != 0) {
            fprintf(stderr, "mkdir: cannot create directory '%s'\n", argv[1]);
            return 1;
        }
    } else {
        if (mkdir(argv[1], 0755) != 0) {
            perror("mkdir");
            return 1;
        }
    }

    add_karma(1, "Created directory");
    return 0;
}

static int cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: touch <file>\n");
        return 1;
    }

    if (is_vfs_path(argv[1])) {
        /* Create empty file in VFS */
        if (vfs_write_file(argv[1], "", 0) != 0) {
            fprintf(stderr, "touch: cannot create file '%s'\n", argv[1]);
            return 1;
        }
    } else {
        FILE* f = fopen(argv[1], "a");
        if (!f) {
            perror("touch");
            return 1;
        }
        fclose(f);
    }

    return 0;
}

static int cmd_write(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: write <file> <content>\n");
        fprintf(stderr, "       write /tbos/myfile.txt 'Hello World'\n");
        return 1;
    }

    /* Join remaining args as content */
    char content[4096] = {0};
    size_t offset = 0;
    for (int i = 2; i < argc && offset < sizeof(content) - 1; i++) {
        if (i > 2) content[offset++] = ' ';
        size_t len = strlen(argv[i]);
        if (offset + len >= sizeof(content)) break;
        memcpy(content + offset, argv[i], len);
        offset += len;
    }
    content[offset] = '\0';

    if (is_vfs_path(argv[1])) {
        if (vfs_write_file(argv[1], content, offset) != 0) {
            fprintf(stderr, "write: cannot write to '%s'\n", argv[1]);
            return 1;
        }
        add_karma(1, "Wrote to virtual file");
        return 0;
    }

    /* Host filesystem */
    FILE* f = fopen(argv[1], "w");
    if (!f) {
        perror("write");
        return 1;
    }
    fwrite(content, 1, offset, f);
    fclose(f);
    add_karma(1, "Wrote to file");
    return 0;
}

static int cmd_rm(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: rm <file>\n");
        return 1;
    }

    if (is_vfs_path(argv[1])) {
        if (vfs_remove(argv[1], false) != 0) {
            fprintf(stderr, "rm: cannot remove '%s'\n", argv[1]);
            return 1;
        }
        return 0;
    }

    if (unlink(argv[1]) != 0) {
        perror("rm");
        return 1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TEXT COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_head(int argc, char** argv) {
    int lines = 10;
    const char* file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-n", 2) == 0) {
            lines = atoi(argv[i] + 2);
            if (lines == 0 && i + 1 < argc) {
                lines = atoi(argv[++i]);
            }
        } else {
            file = argv[i];
        }
    }

    if (!file) {
        fprintf(stderr, "Usage: head [-n lines] <file>\n");
        return 1;
    }

    FILE* f = fopen(file, "r");
    if (!f) {
        perror("head");
        return 1;
    }

    char buf[4096];
    int count = 0;
    while (count < lines && fgets(buf, sizeof(buf), f)) {
        printf("%s", buf);
        count++;
    }

    fclose(f);
    return 0;
}

static int cmd_tail(int argc, char** argv) {
    int lines = 10;
    const char* file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-n", 2) == 0) {
            lines = atoi(argv[i] + 2);
            if (lines == 0 && i + 1 < argc) {
                lines = atoi(argv[++i]);
            }
        } else {
            file = argv[i];
        }
    }

    if (!file) {
        fprintf(stderr, "Usage: tail [-n lines] <file>\n");
        return 1;
    }

    /* Simple implementation: read all lines, print last N */
    FILE* f = fopen(file, "r");
    if (!f) {
        perror("tail");
        return 1;
    }

    char** buffer = malloc(lines * sizeof(char*));
    for (int i = 0; i < lines; i++) {
        buffer[i] = malloc(4096);
        buffer[i][0] = '\0';
    }

    int idx = 0;
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        strncpy(buffer[idx % lines], buf, 4095);
        idx++;
    }

    int start = (idx > lines) ? idx % lines : 0;
    int count = (idx > lines) ? lines : idx;
    for (int i = 0; i < count; i++) {
        printf("%s", buffer[(start + i) % lines]);
    }

    for (int i = 0; i < lines; i++) free(buffer[i]);
    free(buffer);
    fclose(f);

    return 0;
}

static int cmd_wc(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: wc <file>\n");
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("wc");
        return 1;
    }

    int lines = 0, words = 0, chars = 0;
    int in_word = 0;
    int c;

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

    printf("%7d %7d %7d %s\n", lines, words, chars, argv[1]);
    fclose(f);

    return 0;
}

static int cmd_grep(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: grep <pattern> <file>\n");
        return 1;
    }

    const char* pattern = argv[1];
    FILE* f = fopen(argv[2], "r");
    if (!f) {
        perror("grep");
        return 1;
    }

    char buf[4096];
    int line_num = 0;
    while (fgets(buf, sizeof(buf), f)) {
        line_num++;
        if (strstr(buf, pattern)) {
            printf("\033[1;32m%d:\033[0m%s", line_num, buf);
        }
    }

    fclose(f);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SYSTEM COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_date(int argc, char** argv) {
    (void)argc; (void)argv;
    time_t now = time(NULL);
    printf("%s", ctime(&now));
    return 0;
}

static int cmd_whoami(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("%s\n", g_username);
    return 0;
}

static int cmd_hostname(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("%s\n", g_hostname);
    return 0;
}

static int cmd_clear(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\033[2J\033[H");
    return 0;
}

static int cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], (i < argc - 1) ? " " : "");
    }
    printf("\n");
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

/* ═══════════════════════════════════════════════════════════════════════════
 * HOST INTEGRATION COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_ubuntu(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n");
    printf("\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;35m║  Entering Ubuntu/Bash Shell                                ║\033[0m\n");
    printf("\033[1;35m║  Type 'exit' to return to TBOS                             ║\033[0m\n");
    printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");

    pid_t pid = fork();
    if (pid == 0) {
        if (g_host_os == HOST_MACOS) {
            execlp("docker", "docker", "run", "-it", "--rm",
                   "ubuntu:latest", "/bin/bash", NULL);
        }
        execl("/bin/bash", "bash", "--login", NULL);
        execl("/bin/sh", "sh", NULL);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\n\033[1;32mReturned to TBOS Shell\033[0m\n\n");
    add_karma(5, "Explored host system");

    return 0;
}

static int cmd_bash(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\nEntering bash... (type 'exit' to return)\n\n");

    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/bash", "bash", "--login", NULL);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\nReturned to TBOS\n");
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

/* ═══════════════════════════════════════════════════════════════════════════
 * NETWORK COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_wow(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: wow <url>\n");
        printf("Examples:\n");
        printf("  wow http://example.com\n");
        printf("  wow https://api.github.com\n");
        return 1;
    }

    const char* url = argv[1];

    /* Strip wow:// prefix if present */
    if (strncmp(url, "wow://", 6) == 0) {
        url += 6;
    }

    printf("\n");
    printf("\033[1;34m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;34m║  WOW Protocol Fetch                                        ║\033[0m\n");
    printf("\033[1;34m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\nFetching: %s\n\n", url);

    /* Use curl */
    pid_t pid = fork();
    if (pid == 0) {
        execlp("curl", "curl", "-sS", url, NULL);
        perror("curl");
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    printf("\n");
    add_karma(2, "Network exploration");

    return 0;
}

static int cmd_ping(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ping <host>\n");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        execlp("ping", "ping", "-c", "4", argv[1], NULL);
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PXFS COMMANDS (Simulated for hosted mode)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxfs(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n");
    printf("\033[1;36m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║  PXFS - Pixel-Based Compression Filesystem                 ║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Status: Available (hosted mode)                            ║\n");
    printf("║  Compression: RGB pixel encoding (3 bytes/pixel)            ║\n");
    printf("║  Ratio: Up to 1365:1 with quantum compression               ║\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Commands:                                                  ║\n");
    printf("║    pxencode <file>    - Encode to PXFS format               ║\n");
    printf("║    pxdecode <file>    - Decode from PXFS format             ║\n");
    printf("║    pxcompress <file>  - Quantum compress                    ║\n");
    printf("║    pxinfo <file>      - Show compression stats              ║\n");
    printf("\033[1;36m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HISTORY COMMAND
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_history(int argc, char** argv) {
    (void)argc; (void)argv;

    printf("\n");
    for (int i = 0; i < g_history_count; i++) {
        printf("%4d  %s\n", i + 1, g_history[i]);
    }
    printf("\n");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * ADDITIONAL COMMANDS (delegated to host for complex operations)
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Helper: delegate command to host shell */
static int delegate_to_host(int argc, char** argv) {
    char cmd[MAX_INPUT] = "";
    for (int i = 0; i < argc; i++) {
        if (i > 0) strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }
    return execute_via_host_shell(cmd);
}

/* File operations - Week 1 */
static int cmd_cp(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_mv(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_rmdir(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_ln(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_chmod(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: chmod <mode> <file>\n");
        return 1;
    }
    if (is_vfs_path(argv[2])) {
        unsigned int mode = (unsigned int)strtol(argv[1], NULL, 8);
        if (vfs_chmod(argv[2], mode) != 0) {
            fprintf(stderr, "chmod: cannot change '%s'\n", argv[2]);
            return 1;
        }
        return 0;
    }
    return delegate_to_host(argc, argv);
}

static int cmd_chown(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: chown <uid> <file>\n");
        return 1;
    }
    if (is_vfs_path(argv[2])) {
        unsigned int uid = (unsigned int)atoi(argv[1]);
        if (vfs_chown(argv[2], uid, 0) != 0) {
            fprintf(stderr, "chown: cannot change '%s'\n", argv[2]);
            return 1;
        }
        return 0;
    }
    return delegate_to_host(argc, argv);
}

static int cmd_chgrp(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* File info - Week 3 */
static int cmd_stat(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: stat <file>\n");
        return 1;
    }
    if (is_vfs_path(argv[1])) {
        vfs_stat_t st;
        if (vfs_stat(argv[1], &st) != 0) {
            fprintf(stderr, "stat: cannot stat '%s'\n", argv[1]);
            return 1;
        }
        char mode_str[12];
        format_mode(st.mode, st.type == VFS_DIR, mode_str);
        printf("  File: %s\n", argv[1]);
        printf("  Size: %zu\n", st.size);
        printf("Access: (%04o/%s)  Uid: %u  Gid: %u\n", st.mode, mode_str, st.uid, st.gid);
        printf("Modify: %s", ctime(&st.mtime));
        return 0;
    }
    return delegate_to_host(argc, argv);
}
static int cmd_du(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_df(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_file(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Text processing - Week 2 */
static int cmd_more(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_less(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_find(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Process management - Week 4 */
static int cmd_ps(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_top(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_kill(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_jobs(int argc, char** argv) { (void)argc; (void)argv; printf("No background jobs\n"); return 0; }
static int cmd_bg(int argc, char** argv) { (void)argc; (void)argv; printf("No jobs to background\n"); return 0; }
static int cmd_fg(int argc, char** argv) { (void)argc; (void)argv; printf("No jobs to foreground\n"); return 0; }
static int cmd_nice(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_renice(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Compression - Week 5 */
static int cmd_diff(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_cmp(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_tar(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_gzip(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_gunzip(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_base64(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Text transformation - Week 6 */
static int cmd_cut(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_paste(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_join(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_sort(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_uniq(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_tr(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_fmt(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Text layout - Week 7 */
static int cmd_expand(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_unexpand(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_col(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_colrm(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_column(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_rev(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_tac(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Binary utilities - Week 8 */
static int cmd_strings(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_od(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_hexdump(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_xxd(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_iconv(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_dos2unix(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_unix2dos(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Network - Week 9 */
static int cmd_traceroute(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_curl(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_wget(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_nslookup(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_dig(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_netstat(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_ifconfig(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_ssh(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_scp(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* System info - Week 10 */
static int cmd_uname(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_uptime(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_id(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_groups(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_cal(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_free(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_lscpu(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_lsblk(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_mount(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_umount(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Development tools */
static int cmd_make(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_gcc(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_git(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_python(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_python3(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_node(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_npm(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Editors */
static int cmd_vi(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_vim(int argc, char** argv) { return delegate_to_host(argc, argv); }
static int cmd_nano(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* Host shells */
static int cmd_zsh(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\nEntering zsh... (type 'exit' to return)\n\n");
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/zsh", "zsh", "--login", NULL);
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
            execlp("open", "open", "-a", "Terminal", NULL);
        } else {
            execlp("startx", "startx", NULL);
        }
        _exit(1);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}

static int cmd_docker(int argc, char** argv) { return delegate_to_host(argc, argv); }

/* PXFS commands */
static int cmd_pxencode(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("PXFS encode: converts data to RGB pixels (3 bytes = 1 pixel)\n");
    printf("Usage: pxencode <input> <output.px>\n");
    add_karma(1, "Exploring PXFS");
    return 0;
}

static int cmd_pxdecode(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("PXFS decode: converts RGB pixels back to data\n");
    printf("Usage: pxdecode <input.px> <output>\n");
    return 0;
}

static int cmd_pxcompress(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("PXFS quantum compress: achieves up to 1365:1 compression\n");
    printf("Usage: pxcompress <input> <output.pxc>\n");
    return 0;
}

static int cmd_pxdecompress(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("PXFS decompress: restores quantum compressed data\n");
    printf("Usage: pxdecompress <input.pxc> <output>\n");
    return 0;
}

static int cmd_pxinfo(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("\033[1;36m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║  PXFS Compression Statistics                               ║\033[0m\n");
    printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Algorithm: RGB Pixel Encoding + Quantum Compression        ║\n");
    printf("║  Standard Ratio: 3:1 (3 bytes -> 1 pixel)                   ║\n");
    printf("║  Quantum Ratio: up to 1365:1                                ║\n");
    printf("║  Status: Available in hosted mode                           ║\n");
    printf("\033[1;36m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");
    return 0;
}

/* Consciousness commands */
static int cmd_reflect(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;35m║  TBOS Reflection                                           ║\033[0m\n");
    printf("\033[1;35m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║                                                              ║\n");
    printf("║  \"Code with compassion,                                      ║\n");
    printf("║   Debug with patience,                                       ║\n");
    printf("║   Deploy with wisdom.\"                                       ║\n");
    printf("║                                                              ║\n");
    printf("\033[1;35m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Karma: %-6d    Consciousness: %-20s   ║\n", g_karma, get_consciousness_level());
    printf("║  Commands: %-6lu                                           ║\n", (unsigned long)g_commands_executed);
    printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");
    add_karma(5, "Mindful reflection");
    return 0;
}

static int cmd_sangha(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;35m║  Digital Sangha - Conscious Device Network                 ║\033[0m\n");
    printf("\033[1;35m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Status: Standalone mode (no sangha connected)              ║\n");
    printf("║  Discovery: mDNS, Bluetooth, UDP broadcast                  ║\n");
    printf("║  Port: 7805 (Sangha protocol)                               ║\n");
    printf("\033[1;35m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Use 'sangha join <name>' to join a sangha                  ║\n");
    printf("║  Use 'sangha create <name>' to create a sangha              ║\n");
    printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");
    return 0;
}

static int cmd_dharma(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("\n");
    printf("\033[1;33m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;33m║  The Dharma of Computing                                   ║\033[0m\n");
    printf("\033[1;33m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║                                                              ║\n");
    printf("║  1. Right Code      - Write with intention and clarity      ║\n");
    printf("║  2. Right Data      - Handle information with care          ║\n");
    printf("║  3. Right Process   - Execute with mindfulness              ║\n");
    printf("║  4. Right Memory    - Use resources wisely                  ║\n");
    printf("║  5. Right Network   - Connect with compassion               ║\n");
    printf("║  6. Right Storage   - Preserve with integrity               ║\n");
    printf("║  7. Right Interface - Serve users with kindness             ║\n");
    printf("║  8. Right System    - Harmonize all components              ║\n");
    printf("║                                                              ║\n");
    printf("\033[1;33m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");
    add_karma(10, "Studied the Dharma of Computing");
    return 0;
}

/* STEPPPS integrated command */
#ifdef STEPPPS_INTEGRATED
static int cmd_steppps(int argc, char** argv) {
    if (argc < 2) {
        printf("\n");
        printf("\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[1;35m║  STEPPPS - Secure Trust-verified Executable Programs       ║\033[0m\n");
        printf("\033[1;35m╠════════════════════════════════════════════════════════════╣\033[0m\n");
        printf("║  Usage: steppps <command> [file]                            ║\n");
        printf("║                                                              ║\n");
        printf("║  Commands:                                                   ║\n");
        printf("║    show <file>     - Display STEPPPS info                    ║\n");
        printf("║    run <file>      - Execute with security checks            ║\n");
        printf("║    verify <file>   - Verify signature and trust              ║\n");
        printf("║    prompt <file>   - Show AI prompt                          ║\n");
        printf("║    audit           - Show execution audit log                ║\n");
        printf("║                                                              ║\n");
        printf("║  Sandbox Levels (based on karma & signature):                ║\n");
        printf("║    0 DISPLAY  - View only, no execution                      ║\n");
        printf("║    1 PURE     - Pure computation, no I/O                     ║\n");
        printf("║    2 READ     - Read local files                             ║\n");
        printf("║    3 WRITE    - Write local files                            ║\n");
        printf("║    4 NETWORK  - Network access                               ║\n");
        printf("║    5 SYSTEM   - System commands                              ║\n");
        printf("║    6 TRUSTED  - Unrestricted (karma >= 50000)                ║\n");
        printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
        return 0;
    }

    const char* cmd = argv[1];
    const char* file = argc > 2 ? argv[2] : NULL;

    if (strcmp(cmd, "show") == 0 && file) {
        steppps_t s;
        if (steppps_load(file, &s) != 0) {
            printf("Error: Failed to load %s\n", file);
            return 1;
        }
        steppps_display(&s);
        steppps_free(&s);
        add_karma(2, "Examined STEPPPS");
        return 0;
    }
    else if (strcmp(cmd, "run") == 0 && file) {
        steppps_t s;
        if (steppps_load(file, &s) != 0) {
            printf("Error: Failed to load %s\n", file);
            return 1;
        }
        steppps_display(&s);
        int rc = steppps_run(&s);
        printf("\n--- Exit code: %d ---\n", rc);
        steppps_free(&s);
        add_karma(5, "Executed STEPPPS");
        return rc;
    }
    else if (strcmp(cmd, "verify") == 0 && file) {
        steppps_t s;
        if (steppps_load(file, &s) != 0) {
            printf("Error: Failed to load %s\n", file);
            return 1;
        }
        printf("STEPPPS: %s\n", s.id);
        printf("Hash:    %s\n", s.security.hash);
        if (steppps_verify(&s) != 0) {
            printf("Status:  \033[1;31mFAILED\033[0m (blacklisted or invalid)\n");
            steppps_free(&s);
            return 1;
        }
        printf("Signed:  %s\n", s.security.is_signed ? "Yes" : "No");
        if (s.security.is_signed) {
            printf("Author:  %s\n", s.security.author);
            printf("Karma:   %lld\n", (long long)s.security.author_karma);
            printf("Sig OK:  %s\n", s.security.sig_valid ? "\033[1;32mYes\033[0m" : "\033[1;31mNo\033[0m");
        }
        printf("Max Sandbox: %s\n", steppps_sandbox_name(s.security.max_sandbox));
        printf("Safe:    %s\n", steppps_is_safe(&s) ? "\033[1;32mYes\033[0m" : "\033[1;31mNo\033[0m");
        steppps_free(&s);
        add_karma(3, "Verified STEPPPS");
        return 0;
    }
    else if (strcmp(cmd, "prompt") == 0 && file) {
        steppps_t s;
        if (steppps_load(file, &s) != 0) {
            printf("Error: Failed to load %s\n", file);
            return 1;
        }
        printf("\n\033[1;36m=== AI Prompt ===\033[0m\n\n");
        if (s.prompt.system[0]) {
            printf("\033[1;33mSystem:\033[0m\n%s\n\n", s.prompt.system);
        }
        if (s.prompt.user[0]) {
            printf("\033[1;33mUser:\033[0m\n%s\n", s.prompt.user);
        }
        if (!s.prompt.system[0] && !s.prompt.user[0]) {
            printf("(no prompt defined)\n");
        }
        steppps_free(&s);
        return 0;
    }
    else if (strcmp(cmd, "audit") == 0) {
        printf("\n\033[1;35m=== STEPPPS Audit Log ===\033[0m\n\n");
        printf("Audit logging not yet implemented.\n");
        printf("Use 'steppps verify <file>' to verify individual files.\n");
        return 0;
    }
    else {
        printf("Unknown steppps command: %s\n", cmd);
        printf("Use 'steppps' without arguments for help.\n");
        return 1;
    }
}
#endif /* STEPPPS_INTEGRATED */

/* UCFS integrated command */
#ifdef UCFS_INTEGRATED
static int cmd_ucfs(int argc, char** argv) {
    if (argc < 2) {
        printf("\n");
        printf("\033[1;36m╔════════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[1;36m║  UCFS - Unicode Character File System                      ║\033[0m\n");
        printf("\033[1;36m╠════════════════════════════════════════════════════════════╣\033[0m\n");
        printf("║  Usage: ucfs <command> [path]                               ║\n");
        printf("║                                                              ║\n");
        printf("║  Commands:                                                   ║\n");
        printf("║    parse <path>   - Parse UCFS path, show components         ║\n");
        printf("║    canon <path>   - Convert to canonical POSIX path          ║\n");
        printf("║    ls <path>      - List directory using UCFS path           ║\n");
        printf("║    cat <path>     - Read file using UCFS path                ║\n");
        printf("║    roots          - Show available UCFS roots                ║\n");
        printf("║                                                              ║\n");
        printf("║  Examples:                                                   ║\n");
        printf("║    ucfs parse '🕉️/music/chants'                              ║\n");
        printf("║    ucfs canon '🌍/docs/readme.txt'                           ║\n");
        printf("║    ucfs ls '📁/projects'                                     ║\n");
        printf("\033[1;36m╚════════════════════════════════════════════════════════════╝\033[0m\n");
        printf("\n");
        return 0;
    }

    const char* cmd = argv[1];
    const char* path = argc > 2 ? argv[2] : NULL;

    if (strcmp(cmd, "parse") == 0 && path) {
        ucfs_path_t parsed;
        int ret = ucfs_parse(path, &parsed);
        if (ret != 0) {
            printf("Error: Invalid UCFS path (code %d)\n", ret);
            printf("UCFS paths start with a Unicode character as root.\n");
            printf("Example: 🕉️/music/chants or 📁/docs/file.txt\n");
            return 1;
        }
        printf("\n\033[1;36m=== UCFS Path Parsed ===\033[0m\n\n");
        printf("Root delimiter: %s (U+%04X)\n", parsed.delimiter_utf8, parsed.delimiter);
        printf("Components (%zu):\n", parsed.component_count);
        for (size_t i = 0; i < parsed.component_count; i++) {
            printf("  [%zu] %s\n", i, parsed.components[i]);
        }
        ucfs_free(&parsed);
        add_karma(1, "Parsed UCFS path");
        return 0;
    }
    else if (strcmp(cmd, "canon") == 0 && path) {
        ucfs_path_t parsed;
        int ret = ucfs_parse(path, &parsed);
        if (ret != 0) {
            printf("Error: Invalid UCFS path\n");
            return 1;
        }
        char canonical[1024];
        ret = ucfs_to_canonical(&parsed, canonical, sizeof(canonical));
        if (ret != 0) {
            printf("Error: Failed to canonicalize path\n");
            ucfs_free(&parsed);
            return 1;
        }
        printf("%s\n", canonical);
        ucfs_free(&parsed);
        return 0;
    }
    else if (strcmp(cmd, "ls") == 0 && path) {
        ucfs_path_t parsed;
        int ret = ucfs_parse(path, &parsed);
        if (ret != 0) {
            printf("Error: Invalid UCFS path\n");
            return 1;
        }
        char canonical[1024];
        ret = ucfs_to_canonical(&parsed, canonical, sizeof(canonical));
        ucfs_free(&parsed);
        if (ret != 0) {
            printf("Error: Failed to canonicalize path\n");
            return 1;
        }
        char* ls_argv[] = {"ls", "-la", canonical, NULL};
        return delegate_to_host(3, ls_argv);
    }
    else if (strcmp(cmd, "cat") == 0 && path) {
        ucfs_path_t parsed;
        int ret = ucfs_parse(path, &parsed);
        if (ret != 0) {
            printf("Error: Invalid UCFS path\n");
            return 1;
        }
        char canonical[1024];
        ret = ucfs_to_canonical(&parsed, canonical, sizeof(canonical));
        ucfs_free(&parsed);
        if (ret != 0) {
            printf("Error: Failed to canonicalize path\n");
            return 1;
        }
        char* cat_argv[] = {"cat", canonical, NULL};
        return delegate_to_host(2, cat_argv);
    }
    else if (strcmp(cmd, "roots") == 0) {
        printf("\n\033[1;36m=== UCFS Roots ===\033[0m\n\n");
        printf("  🕉️   - Spiritual/meditation content\n");
        printf("  📁  - Documents and files\n");
        printf("  🎵  - Music and audio\n");
        printf("  🎬  - Video content\n");
        printf("  🌍  - Global/shared content\n");
        printf("  🏠  - Home directory\n");
        printf("  💼  - Work/projects\n");
        printf("  🔧  - System/config\n");
        printf("\nAny Unicode character can be a root.\n");
        printf("Example: ucfs ls '🕉️/mantras'\n");
        return 0;
    }
    else {
        printf("Unknown ucfs command: %s\n", cmd);
        printf("Use 'ucfs' without arguments for help.\n");
        return 1;
    }
}
#endif /* UCFS_INTEGRATED */

/* Alias for common commands */
static int cmd_ll(int argc, char** argv) {
    char* new_argv[] = {"ls", "-la", NULL, NULL};
    if (argc > 1) new_argv[2] = argv[1];
    return delegate_to_host(argc > 1 ? 3 : 2, new_argv);
}

static int cmd_la(int argc, char** argv) {
    char* new_argv[] = {"ls", "-a", NULL, NULL};
    if (argc > 1) new_argv[2] = argv[1];
    return delegate_to_host(argc > 1 ? 3 : 2, new_argv);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

static void register_all_commands(void) {
    /* Core */
    register_command("help",     "Show all commands",           cmd_help,     CMD_CAT_CORE, 0);
    register_command("?",        "Show all commands",           cmd_help,     CMD_CAT_CORE, 0);
    register_command("exit",     "Exit TBOS shell",             cmd_exit,     CMD_CAT_CORE, 0);
    register_command("quit",     "Exit TBOS shell",             cmd_exit,     CMD_CAT_CORE, 0);
    register_command("history",  "Show command history",        cmd_history,  CMD_CAT_CORE, 0);

    /* File operations - Week 1 */
    register_command("ls",       "List directory contents",     cmd_ls,       CMD_CAT_FILE, 1);
    register_command("ll",       "List with details (ls -la)",  cmd_ll,       CMD_CAT_FILE, 1);
    register_command("la",       "List all files (ls -a)",      cmd_la,       CMD_CAT_FILE, 1);
    register_command("cd",       "Change directory",            cmd_cd,       CMD_CAT_FILE, 1);
    register_command("pwd",      "Print working directory",     cmd_pwd,      CMD_CAT_FILE, 0);
    register_command("mkdir",    "Create directory",            cmd_mkdir,    CMD_CAT_FILE, 2);
    register_command("rmdir",    "Remove empty directory",      cmd_rmdir,    CMD_CAT_FILE, 0);
    register_command("rm",       "Remove file",                 cmd_rm,       CMD_CAT_FILE, 0);
    register_command("cp",       "Copy files",                  cmd_cp,       CMD_CAT_FILE, 1);
    register_command("mv",       "Move/rename files",           cmd_mv,       CMD_CAT_FILE, 1);
    register_command("touch",    "Create/update file",          cmd_touch,    CMD_CAT_FILE, 1);
    register_command("write",    "Write content to file",       cmd_write,    CMD_CAT_FILE, 2);
    register_command("ln",       "Create links",                cmd_ln,       CMD_CAT_FILE, 1);
    register_command("chmod",    "Change permissions",          cmd_chmod,    CMD_CAT_FILE, 1);
    register_command("chown",    "Change owner",                cmd_chown,    CMD_CAT_FILE, 1);
    register_command("chgrp",    "Change group",                cmd_chgrp,    CMD_CAT_FILE, 1);
    register_command("stat",     "File status",                 cmd_stat,     CMD_CAT_FILE, 1);

    /* Text processing - Week 2 */
    register_command("cat",      "Display file contents",       cmd_cat,      CMD_CAT_TEXT, 1);
    register_command("head",     "Display first lines",         cmd_head,     CMD_CAT_TEXT, 1);
    register_command("tail",     "Display last lines",          cmd_tail,     CMD_CAT_TEXT, 1);
    register_command("more",     "Page through text",           cmd_more,     CMD_CAT_TEXT, 1);
    register_command("less",     "Advanced pager",              cmd_less,     CMD_CAT_TEXT, 1);
    register_command("wc",       "Word/line/char count",        cmd_wc,       CMD_CAT_TEXT, 1);
    register_command("grep",     "Search for pattern",          cmd_grep,     CMD_CAT_TEXT, 2);
    register_command("find",     "Find files",                  cmd_find,     CMD_CAT_TEXT, 2);
    register_command("echo",     "Display text",                cmd_echo,     CMD_CAT_TEXT, 0);

    /* Disk/storage - Week 3 */
    register_command("du",       "Disk usage",                  cmd_du,       CMD_CAT_FILE, 1);
    register_command("df",       "Disk free space",             cmd_df,       CMD_CAT_FILE, 1);
    register_command("file",     "Determine file type",         cmd_file,     CMD_CAT_FILE, 1);
    register_command("mount",    "Mount filesystem",            cmd_mount,    CMD_CAT_FILE, 0);
    register_command("umount",   "Unmount filesystem",          cmd_umount,   CMD_CAT_FILE, 0);
    register_command("lsblk",    "List block devices",          cmd_lsblk,    CMD_CAT_FILE, 1);

    /* Process management - Week 4 */
    register_command("ps",       "Process status",              cmd_ps,       CMD_CAT_PROCESS, 1);
    register_command("top",      "Process monitor",             cmd_top,      CMD_CAT_PROCESS, 1);
    register_command("kill",     "Send signal to process",      cmd_kill,     CMD_CAT_PROCESS, 0);
    register_command("jobs",     "List background jobs",        cmd_jobs,     CMD_CAT_PROCESS, 0);
    register_command("bg",       "Background a job",            cmd_bg,       CMD_CAT_PROCESS, 0);
    register_command("fg",       "Foreground a job",            cmd_fg,       CMD_CAT_PROCESS, 0);
    register_command("nice",     "Run with priority",           cmd_nice,     CMD_CAT_PROCESS, 1);
    register_command("renice",   "Change priority",             cmd_renice,   CMD_CAT_PROCESS, 1);

    /* Compression - Week 5 */
    register_command("diff",     "Compare files",               cmd_diff,     CMD_CAT_TEXT, 2);
    register_command("cmp",      "Compare bytes",               cmd_cmp,      CMD_CAT_TEXT, 1);
    register_command("tar",      "Archive files",               cmd_tar,      CMD_CAT_FILE, 2);
    register_command("gzip",     "Compress files",              cmd_gzip,     CMD_CAT_FILE, 1);
    register_command("gunzip",   "Decompress files",            cmd_gunzip,   CMD_CAT_FILE, 1);
    register_command("base64",   "Base64 encode/decode",        cmd_base64,   CMD_CAT_TEXT, 1);

    /* Text transformation - Week 6 */
    register_command("cut",      "Extract columns",             cmd_cut,      CMD_CAT_TEXT, 1);
    register_command("paste",    "Merge lines",                 cmd_paste,    CMD_CAT_TEXT, 1);
    register_command("join",     "Join files on field",         cmd_join,     CMD_CAT_TEXT, 1);
    register_command("sort",     "Sort lines",                  cmd_sort,     CMD_CAT_TEXT, 1);
    register_command("uniq",     "Remove duplicates",           cmd_uniq,     CMD_CAT_TEXT, 1);
    register_command("tr",       "Translate characters",        cmd_tr,       CMD_CAT_TEXT, 1);
    register_command("fmt",      "Format text",                 cmd_fmt,      CMD_CAT_TEXT, 1);

    /* Text layout - Week 7 */
    register_command("expand",   "Tabs to spaces",              cmd_expand,   CMD_CAT_TEXT, 1);
    register_command("unexpand", "Spaces to tabs",              cmd_unexpand, CMD_CAT_TEXT, 1);
    register_command("col",      "Filter control chars",        cmd_col,      CMD_CAT_TEXT, 1);
    register_command("colrm",    "Remove columns",              cmd_colrm,    CMD_CAT_TEXT, 1);
    register_command("column",   "Columnize output",            cmd_column,   CMD_CAT_TEXT, 1);
    register_command("rev",      "Reverse lines",               cmd_rev,      CMD_CAT_TEXT, 1);
    register_command("tac",      "Reverse file",                cmd_tac,      CMD_CAT_TEXT, 1);

    /* Binary utilities - Week 8 */
    register_command("strings",  "Extract strings",             cmd_strings,  CMD_CAT_TEXT, 1);
    register_command("od",       "Octal dump",                  cmd_od,       CMD_CAT_TEXT, 1);
    register_command("hexdump",  "Hex dump",                    cmd_hexdump,  CMD_CAT_TEXT, 1);
    register_command("xxd",      "Hex editor dump",             cmd_xxd,      CMD_CAT_TEXT, 1);
    register_command("iconv",    "Convert encoding",            cmd_iconv,    CMD_CAT_TEXT, 1);
    register_command("dos2unix", "DOS to Unix lines",           cmd_dos2unix, CMD_CAT_TEXT, 1);
    register_command("unix2dos", "Unix to DOS lines",           cmd_unix2dos, CMD_CAT_TEXT, 1);

    /* Network - Week 9 */
    register_command("ping",     "Ping a host",                 cmd_ping,     CMD_CAT_NETWORK, 1);
    register_command("traceroute","Trace route",                cmd_traceroute,CMD_CAT_NETWORK, 1);
    register_command("curl",     "Transfer URL",                cmd_curl,     CMD_CAT_NETWORK, 2);
    register_command("wget",     "Download files",              cmd_wget,     CMD_CAT_NETWORK, 2);
    register_command("nslookup", "DNS lookup",                  cmd_nslookup, CMD_CAT_NETWORK, 1);
    register_command("dig",      "DNS query",                   cmd_dig,      CMD_CAT_NETWORK, 1);
    register_command("netstat",  "Network status",              cmd_netstat,  CMD_CAT_NETWORK, 1);
    register_command("ifconfig", "Network interfaces",          cmd_ifconfig, CMD_CAT_NETWORK, 1);
    register_command("ssh",      "Secure shell",                cmd_ssh,      CMD_CAT_NETWORK, 2);
    register_command("scp",      "Secure copy",                 cmd_scp,      CMD_CAT_NETWORK, 2);
    register_command("wow",      "WOW protocol fetch",          cmd_wow,      CMD_CAT_NETWORK, 2);

    /* System info - Week 10 */
    register_command("date",     "Show current date/time",      cmd_date,     CMD_CAT_SYSTEM, 0);
    register_command("whoami",   "Show current user",           cmd_whoami,   CMD_CAT_SYSTEM, 0);
    register_command("hostname", "Show system hostname",        cmd_hostname, CMD_CAT_SYSTEM, 0);
    register_command("uname",    "System information",          cmd_uname,    CMD_CAT_SYSTEM, 1);
    register_command("uptime",   "System uptime",               cmd_uptime,   CMD_CAT_SYSTEM, 0);
    register_command("id",       "User/group IDs",              cmd_id,       CMD_CAT_SYSTEM, 0);
    register_command("groups",   "User groups",                 cmd_groups,   CMD_CAT_SYSTEM, 0);
    register_command("cal",      "Calendar",                    cmd_cal,      CMD_CAT_SYSTEM, 0);
    register_command("clear",    "Clear screen",                cmd_clear,    CMD_CAT_SYSTEM, 0);
    register_command("env",      "Show environment",            cmd_env,      CMD_CAT_SYSTEM, 0);
    register_command("hostinfo", "Show host system info",       cmd_hostinfo, CMD_CAT_SYSTEM, 1);
    register_command("free",     "Memory usage",                cmd_free,     CMD_CAT_SYSTEM, 1);
    register_command("lscpu",    "CPU information",             cmd_lscpu,    CMD_CAT_SYSTEM, 1);

    /* Development tools */
    register_command("make",     "Build tool",                  cmd_make,     CMD_CAT_SYSTEM, 2);
    register_command("gcc",      "C compiler",                  cmd_gcc,      CMD_CAT_SYSTEM, 2);
    register_command("git",      "Version control",             cmd_git,      CMD_CAT_SYSTEM, 2);
    register_command("python",   "Python interpreter",          cmd_python,   CMD_CAT_SYSTEM, 1);
    register_command("python3",  "Python 3 interpreter",        cmd_python3,  CMD_CAT_SYSTEM, 1);
    register_command("node",     "Node.js",                     cmd_node,     CMD_CAT_SYSTEM, 1);
    register_command("npm",      "Node package manager",        cmd_npm,      CMD_CAT_SYSTEM, 1);
    register_command("docker",   "Container runtime",           cmd_docker,   CMD_CAT_SYSTEM, 2);

    /* Editors */
    register_command("vi",       "Vi editor",                   cmd_vi,       CMD_CAT_SYSTEM, 1);
    register_command("vim",      "Vim editor",                  cmd_vim,      CMD_CAT_SYSTEM, 1);
    register_command("nano",     "Nano editor",                 cmd_nano,     CMD_CAT_SYSTEM, 1);

    /* PXFS - Pixel filesystem */
    register_command("pxfs",     "PXFS filesystem info",        cmd_pxfs,     CMD_CAT_PXFS, 0);
    register_command("pxencode", "Encode to PXFS",              cmd_pxencode, CMD_CAT_PXFS, 1);
    register_command("pxdecode", "Decode from PXFS",            cmd_pxdecode, CMD_CAT_PXFS, 1);
    register_command("pxcompress","Quantum compress",           cmd_pxcompress,CMD_CAT_PXFS, 1);
    register_command("pxdecompress","Quantum decompress",       cmd_pxdecompress,CMD_CAT_PXFS, 1);
    register_command("pxinfo",   "PXFS statistics",             cmd_pxinfo,   CMD_CAT_PXFS, 0);

    /* Consciousness */
    register_command("karma",    "Show karma status",           cmd_karma,    CMD_CAT_CONSCIOUSNESS, 0);
    register_command("meditate", "Take a mindful break",        cmd_meditate, CMD_CAT_CONSCIOUSNESS, 0);
    register_command("reflect",  "Reflect on your journey",     cmd_reflect,  CMD_CAT_CONSCIOUSNESS, 0);
    register_command("sangha",   "Digital sangha network",      cmd_sangha,   CMD_CAT_CONSCIOUSNESS, 0);
    register_command("dharma",   "The Dharma of Computing",     cmd_dharma,   CMD_CAT_CONSCIOUSNESS, 0);
#ifdef STEPPPS_INTEGRATED
    register_command("steppps",  "STEPPPS secure runtime",      cmd_steppps,  CMD_CAT_CONSCIOUSNESS, 0);
#endif
#ifdef UCFS_INTEGRATED
    register_command("ucfs",     "Unicode Character FS",        cmd_ucfs,     CMD_CAT_FILE, 0);
#endif

    /* Host integration */
    register_command("ubuntu",   "Enter Ubuntu shell",          cmd_ubuntu,   CMD_CAT_HOST, 0);
    register_command("bash",     "Enter bash shell",            cmd_bash,     CMD_CAT_HOST, 0);
    register_command("zsh",      "Enter zsh shell",             cmd_zsh,      CMD_CAT_HOST, 0);
    register_command("shell",    "Enter system shell",          cmd_shell,    CMD_CAT_HOST, 0);
    register_command("startx",   "Start graphical session",     cmd_startx,   CMD_CAT_HOST, 0);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND PARSING & EXECUTION
 * ═══════════════════════════════════════════════════════════════════════════ */

static int parse_line(char* line, char** argv) {
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

static int has_shell_metachar(const char* line) {
    return (strchr(line, '|') || strchr(line, '>') || strchr(line, '<') ||
            strchr(line, '&') || strchr(line, ';') || strchr(line, '$') ||
            strchr(line, '`') || strchr(line, '*') || strchr(line, '?'));
}

static int execute_via_host_shell(const char* line) {
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

static int execute_command(const char* original_line, int argc, char** argv) {
    if (argc == 0) return 0;

    /* Find TBOS command */
    tbos_command_t* cmd = find_command(argv[0]);

    if (cmd) {
        int result = cmd->handler(argc, argv);
        if (result == 0 && cmd->karma_reward > 0) {
            add_karma(cmd->karma_reward, NULL);
        }
        return result;
    }

    /* If has shell metacharacters or not found, pass to host shell */
    if (has_shell_metachar(original_line)) {
        return execute_via_host_shell(original_line);
    }

    /* Try as external command */
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "tbos: command not found: %s\n", argv[0]);
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return WEXITSTATUS(status);
    }

    return 1;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * BANNER & PROMPT
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_banner(void) {
    printf("\n");
    printf("\033[1;36m╔═══════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;36m║                                                               ║\033[0m\n");
    printf("\033[1;36m║   \033[1;33m████████╗██████╗  ██████╗ ███████╗\033[1;36m                          ║\033[0m\n");
    printf("\033[1;36m║   \033[1;33m╚══██╔══╝██╔══██╗██╔═══██╗██╔════╝\033[1;36m                          ║\033[0m\n");
    printf("\033[1;36m║   \033[1;33m   ██║   ██████╔╝██║   ██║███████╗\033[1;36m                          ║\033[0m\n");
    printf("\033[1;36m║   \033[1;33m   ██║   ██╔══██╗██║   ██║╚════██║\033[1;36m                          ║\033[0m\n");
    printf("\033[1;36m║   \033[1;33m   ██║   ██████╔╝╚██████╔╝███████║\033[1;36m                          ║\033[0m\n");
    printf("\033[1;36m║   \033[1;33m   ╚═╝   ╚═════╝  ╚═════╝ ╚══════╝\033[1;36m                          ║\033[0m\n");
    printf("\033[1;36m║                                                               ║\033[0m\n");
    printf("\033[1;36m║           \033[1;37mTernaryBit Operating System v%s\033[1;36m                   ║\033[0m\n", TBOS_VERSION);
    printf("\033[1;36m║        \033[1;35mConscious Computing on Any Platform\033[1;36m                    ║\033[0m\n");
    printf("\033[1;36m║                                                               ║\033[0m\n");
    printf("\033[1;36m╠═══════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("\033[1;36m║  \033[0mHost: %-12s  User: %-12s  Karma: %-6d\033[1;36m     ║\033[0m\n",
           host_os_name(), g_username, g_karma);
    printf("\033[1;36m║  \033[0mType '\033[1;32mhelp\033[0m' for commands, '\033[1;32mubuntu\033[0m' for Ubuntu shell\033[1;36m          ║\033[0m\n");
    printf("\033[1;36m╚═══════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n");
}

static char g_prompt[512];

static const char* get_prompt(void) {
    snprintf(g_prompt, sizeof(g_prompt),
             "\033[1;36mtbos\033[0m:%s[%d]\033[0m:\033[1;34m%s\033[0m$ ",
             get_karma_color(), g_karma, g_cwd);
    return g_prompt;
}

/* print_prompt() removed - now using tbos_readline(get_prompt(), ...) */

/* ═══════════════════════════════════════════════════════════════════════════
 * MAIN
 * ═══════════════════════════════════════════════════════════════════════════ */

int main(int argc, char** argv) {
    char input[MAX_INPUT];
    char input_copy[MAX_INPUT];
    char* cmd_argv[MAX_ARGS];

    /* Handle script execution */
    if (argc > 1) {
        /* Execute script or single command */
        FILE* f = fopen(argv[1], "r");
        if (f) {
            while (fgets(input, sizeof(input), f)) {
                size_t len = strlen(input);
                if (len > 0 && input[len-1] == '\n') input[len-1] = '\0';
                if (input[0] == '\0' || input[0] == '#') continue;

                strcpy(input_copy, input);
                int cmd_argc = parse_line(input, cmd_argv);
                execute_command(input_copy, cmd_argc, cmd_argv);
            }
            fclose(f);
            return 0;
        }
    }

    /* Initialize */
    g_host_os = detect_host_os();
    getcwd(g_cwd, sizeof(g_cwd));
    gethostname(g_hostname, sizeof(g_hostname));
    strncpy(g_username, getenv("USER") ? getenv("USER") : "user", sizeof(g_username) - 1);
    g_session_start = time(NULL);

#ifndef TBOS_PLATFORM_WINDOWS
    /* Detect piped/redirected input - use line mode instead of raw mode */
    {
        int pending = 0;
        if (ioctl(STDIN_FILENO, FIONREAD, &pending) == 0 && pending > 0) {
            g_force_line_mode = 1;  /* Input already buffered */
        }
    }
#endif

    /* Register commands */
    register_all_commands();

    /* Initialize virtual filesystem */
    init_vfs();

    /* Banner */
    print_banner();

    /* Main loop */
    while (g_running) {
        int result = tbos_readline(get_prompt(), input, sizeof(input));

        if (result < 0) {
            printf("\n");
            break;
        }

        /* Skip empty lines */
        if (input[0] == '\0') continue;

        /* Add to history */
        if (g_history_count < HISTORY_SIZE) {
            strncpy(g_history[g_history_count++], input, MAX_INPUT - 1);
        }

        /* Keep original for shell delegation */
        strcpy(input_copy, input);

        /* Parse and execute */
        int cmd_argc = parse_line(input, cmd_argv);
        execute_command(input_copy, cmd_argc, cmd_argv);
        g_commands_executed++;
    }

    /* Exit message */
    printf("\n");
    printf("\033[1;35m╔════════════════════════════════════════════════════════════╗\033[0m\n");
    printf("\033[1;35m║  Session Complete                                          ║\033[0m\n");
    printf("\033[1;35m╠════════════════════════════════════════════════════════════╣\033[0m\n");
    printf("║  Final Karma: %-6d                                       ║\n", g_karma);
    printf("║  Consciousness: %-20s                   ║\n", get_consciousness_level());
    printf("║  Commands: %-6lu                                          ║\n",
           (unsigned long)g_commands_executed);
    printf("\033[1;35m╚════════════════════════════════════════════════════════════╝\033[0m\n");
    printf("\n\033[1;33mMay your code compile on first try! Namaste.\033[0m\n\n");

    return 0;
}
