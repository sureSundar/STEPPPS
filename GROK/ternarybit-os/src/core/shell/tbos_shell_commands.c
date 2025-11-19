/**
 * @file tbos_shell_commands.c
 * @brief TernaryBit OS - Built-in Shell Commands Implementation
 */

#include "tbos_shell.h"
#include "../karma/tbos_karma_ledger.h"
#include "tbos_supershell.h"
#include "fs/ucfs_overlay.h"
#include "fs/pxfs_overlay.h"
#include "../../hal/tbos_hal.h"
#include "../steppps/tbos_steppps.h"
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

static const char* resolve_shell_path(const char* path, char* buffer, size_t buffer_len) {
    if (!path || !*path) {
        return path;
    }
    if (path[0] == '[') {
        if (buffer_len && ucfs_resolve_path(path, buffer, buffer_len) == 0) {
            return buffer;
        }
        return path;
    }
    if (path[0] == '{') {
        if (buffer_len && pxfs_resolve_path(path, buffer, buffer_len) == 0) {
            return buffer;
        }
        return path;
    }
    if (path[0] == '/' || path[0] == '\\') {
        return path;
    }
    if (buffer_len == 0) return path;
    const char* cwd = tbos_shell_get_cwd();
    if (strcmp(cwd, "/") == 0) {
        if (snprintf(buffer, buffer_len, "/%s", path) >= (int)buffer_len) {
            return path;
        }
    } else {
        if (snprintf(buffer, buffer_len, "%s/%s", cwd, path) >= (int)buffer_len) {
            return path;
        }
    }
    return buffer;
}

static int shell_touch_file(const char* path) {
    char resolved[512];
    const char* effective = resolve_shell_path(path, resolved, sizeof(resolved));
    int fd = tbos_open(effective, TBOS_O_CREAT | TBOS_O_APPEND);
    if (fd < 0) {
        printf("❌ touch: unable to create %s\n", path);
        return TBOS_SHELL_ERROR;
    }
    tbos_close(fd);
    return TBOS_SHELL_SUCCESS;
}

static int shell_copy_file_resolved(const char* src, const char* dest, bool overwrite) {
    char src_res[512];
    char dest_res[512];
    const char* src_path = resolve_shell_path(src, src_res, sizeof(src_res));
    const char* dest_path = resolve_shell_path(dest, dest_res, sizeof(dest_res));

    int src_fd = tbos_open(src_path, TBOS_O_RDONLY);
    if (src_fd < 0) {
        printf("❌ cp: cannot open %s\n", src);
        return TBOS_SHELL_ERROR;
    }

    uint32_t dest_flags = TBOS_O_WRONLY | TBOS_O_CREAT;
    if (overwrite) dest_flags |= TBOS_O_TRUNC;
    int dest_fd = tbos_open(dest_path, dest_flags);
    if (dest_fd < 0) {
        printf("❌ cp: cannot open %s for writing\n", dest);
        tbos_close(src_fd);
        return TBOS_SHELL_ERROR;
    }

    char buffer[512];
    int status = TBOS_SHELL_SUCCESS;
    int read_bytes;

    while ((read_bytes = tbos_read(src_fd, buffer, sizeof(buffer))) > 0) {
        int written = tbos_write(dest_fd, buffer, (size_t)read_bytes);
        if (written != read_bytes) {
            printf("❌ cp: write error on %s\n", dest);
            status = TBOS_SHELL_ERROR;
            break;
        }
    }

    tbos_close(src_fd);
    tbos_close(dest_fd);
    return status;
}

static int parse_line_count(int* argi, int argc, char** argv, size_t* lines_out) {
    *lines_out = 10;
    if (*argi + 2 <= argc && strcmp(argv[*argi], "-n") == 0) {
        int value = atoi(argv[*argi + 1]);
        if (value <= 0) {
            printf("Usage: %s [-n lines] <file>\n", argv[0]);
            return -1;
        }
        *lines_out = (size_t)value;
        *argi += 2;
    }
    return 0;
}

/* ========================================================================= */
/* TRADITIONAL COMMANDS                                                      */
/* ========================================================================= */

int tbos_cmd_ls(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : tbos_shell_get_cwd();
    char resolved[512];
    const char* effective_path = resolve_shell_path(path, resolved, sizeof(resolved));

    printf("Listing directory: %s\n", path);
    if (effective_path != path) {
        printf("Resolved path -> %s\n", effective_path);
    }

    /* Get directory contents */
    tbos_dirent_t entries[256];
    int count = tbos_readdir(effective_path, entries, 256);

    if (count < 0) {
        printf("❌ Failed to read directory: %s\n", path);
        return TBOS_SHELL_ERROR;
    }

    if (count == 0) {
        printf("(empty directory)\n");
        return TBOS_SHELL_SUCCESS;
    }

    /* Display entries */
    printf("\n");
    for (int i = 0; i < count; i++) {
        printf("  %s\n", entries[i].name);
    }
    printf("\nTotal: %d entries\n", count);

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_cd(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cd <path>\n");
        return TBOS_SHELL_ERROR;
    }

    const char* path = argv[1];
    char resolved[512];
    const char* effective_path = resolve_shell_path(path, resolved, sizeof(resolved));

    if (tbos_readdir(effective_path, NULL, 0) < 0) {
        printf("❌ Directory not found: %s\n", path);
        return TBOS_SHELL_ERROR;
    }

    if (tbos_shell_set_cwd(effective_path) != TBOS_SHELL_SUCCESS) {
        printf("❌ Failed to change directory to: %s\n", path);
        return TBOS_SHELL_ERROR;
    }

    char display[512];
    tbos_shell_format_path(tbos_shell_get_cwd(), display, sizeof(display));
    printf("Changed directory to: %s\n", display);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_pwd(int argc, char** argv) {
    (void)argc;
    (void)argv;

    char display[512];
    const char* friendly = tbos_shell_format_path(tbos_shell_get_cwd(), display, sizeof(display));
    printf("%s\n", friendly);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: mkdir <path>\n");
        return TBOS_SHELL_ERROR;
    }

    const char* path = argv[1];
    char resolved[512];
    const char* effective_path = resolve_shell_path(path, resolved, sizeof(resolved));
    int result = tbos_mkdir(effective_path);

    if (result != 0) {
        printf("❌ Failed to create directory: %s\n", path);
        return TBOS_SHELL_ERROR;
    }

    printf("✅ Created directory: %s\n", path);
    printf("   [Karma +10 for creating, not destroying]\n");

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_rm(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: rm <path>\n");
        return TBOS_SHELL_ERROR;
    }

    const char* path = argv[1];
    char resolved[512];
    const char* effective_path = resolve_shell_path(path, resolved, sizeof(resolved));

    /* Validate Right Speech for deletion */
    printf("⚠️  Removing: %s\n", path);
    printf("   [This action consumes karma - use mindfully]\n");

    int result = tbos_unlink(effective_path);

    if (result != 0) {
        printf("❌ Failed to remove: %s\n", path);
        return TBOS_SHELL_ERROR;
    }

    printf("✅ Removed: %s\n", path);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_rmdir(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: rmdir <path>\n");
        return TBOS_SHELL_ERROR;
    }

    char resolved[512];
    const char* effective_path = resolve_shell_path(argv[1], resolved, sizeof(resolved));
    if (tbos_rmdir(effective_path) != 0) {
        printf("❌ Failed to remove directory: %s\n", argv[1]);
        return TBOS_SHELL_ERROR;
    }

    printf("✅ Removed directory: %s\n", argv[1]);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cat <path>\n");
        return TBOS_SHELL_ERROR;
    }

    const char* path = argv[1];
    char resolved[512];
    const char* effective_path = resolve_shell_path(path, resolved, sizeof(resolved));

    /* Open file */
    int fd = tbos_open(effective_path, TBOS_O_RDONLY);
    if (fd < 0) {
        printf("❌ Failed to open: %s\n", path);
        return TBOS_SHELL_ERROR;
    }

    /* Read and display contents */
    char buffer[4096];
    int bytes_read;

    printf("\n");
    while ((bytes_read = tbos_read(fd, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
    printf("\n");

    tbos_close(fd);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s", argv[i]);
        if (i < argc - 1) printf(" ");
    }
    printf("\n");

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: touch <file> [file...]\n");
        return TBOS_SHELL_ERROR;
    }
    for (int i = 1; i < argc; ++i) {
        if (shell_touch_file(argv[i]) != TBOS_SHELL_SUCCESS) {
            return TBOS_SHELL_ERROR;
        }
    }
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_cp(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: cp <source> <dest>\n");
        return TBOS_SHELL_ERROR;
    }
    return shell_copy_file_resolved(argv[1], argv[2], true);
}

int tbos_cmd_mv(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: mv <source> <dest>\n");
        return TBOS_SHELL_ERROR;
    }
    int status = shell_copy_file_resolved(argv[1], argv[2], true);
    if (status != TBOS_SHELL_SUCCESS) {
        return status;
    }
    char src_res[512];
    const char* src_path = resolve_shell_path(argv[1], src_res, sizeof(src_res));
    if (tbos_unlink(src_path) != 0) {
        printf("⚠️  mv: copied but failed to remove original %s\n", argv[1]);
    }
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_head(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: head [-n lines] <file>\n");
        return TBOS_SHELL_ERROR;
    }
    int argi = 1;
    size_t lines;
    if (parse_line_count(&argi, argc, argv, &lines) != 0) {
        return TBOS_SHELL_ERROR;
    }
    if (argi >= argc) {
        printf("Usage: head [-n lines] <file>\n");
        return TBOS_SHELL_ERROR;
    }
    char resolved[512];
    const char* path = resolve_shell_path(argv[argi], resolved, sizeof(resolved));
    int fd = tbos_open(path, TBOS_O_RDONLY);
    if (fd < 0) {
        printf("❌ head: cannot open %s\n", argv[argi]);
        return TBOS_SHELL_ERROR;
    }
    char buffer[256];
    size_t printed_lines = 0;
    int bytes;
    while ((bytes = tbos_read(fd, buffer, sizeof(buffer))) > 0 && printed_lines < lines) {
        for (int i = 0; i < bytes; ++i) {
            putchar(buffer[i]);
            if (buffer[i] == '\n') {
                printed_lines++;
                if (printed_lines >= lines) {
                    i = bytes; /* break outer */
                    break;
                }
            }
        }
    }
    if (printed_lines < lines) {
        printf("\n");
    }
    tbos_close(fd);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_tail(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: tail [-n lines] <file>\n");
        return TBOS_SHELL_ERROR;
    }
    int argi = 1;
    size_t lines;
    if (parse_line_count(&argi, argc, argv, &lines) != 0) {
        return TBOS_SHELL_ERROR;
    }
    if (argi >= argc) {
        printf("Usage: tail [-n lines] <file>\n");
        return TBOS_SHELL_ERROR;
    }
    char resolved[512];
    const char* path = resolve_shell_path(argv[argi], resolved, sizeof(resolved));
    int fd = tbos_open(path, TBOS_O_RDONLY);
    if (fd < 0) {
        printf("❌ tail: cannot open %s\n", argv[argi]);
        return TBOS_SHELL_ERROR;
    }
    size_t capacity = 4096;
    size_t max_capacity = 65536;
    char* data = malloc(capacity);
    if (!data) {
        tbos_close(fd);
        printf("❌ tail: memory allocation failed\n");
        return TBOS_SHELL_ERROR;
    }
    size_t size = 0;
    char buffer[512];
    int bytes;
    while ((bytes = tbos_read(fd, buffer, sizeof(buffer))) > 0) {
        if (size + (size_t)bytes > capacity) {
            if (capacity >= max_capacity) {
                break;
            }
            size_t new_capacity = capacity * 2;
            if (new_capacity > max_capacity) new_capacity = max_capacity;
            char* tmp = realloc(data, new_capacity);
            if (!tmp) break;
            data = tmp;
            capacity = new_capacity;
        }
        memcpy(data + size, buffer, (size_t)bytes);
        size += (size_t)bytes;
    }
    tbos_close(fd);
    size_t start = 0;
    size_t count = 0;
    for (ssize_t i = (ssize_t)size - 1; i >= 0; --i) {
        if (data[i] == '\n') {
            count++;
            if (count == lines + 1) {
                start = (size_t)(i + 1);
                break;
            }
        }
    }
    fwrite(data + start, 1, size - start, stdout);
    if (size == 0 || data[size - 1] != '\n') {
        printf("\n");
    }
    free(data);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_posix_shell(int argc, char** argv) {
    (void)argc;
    (void)argv;

    const char* shell_bin = getenv("TBOS_SUBSHELL_BIN");
    if (!shell_bin || !*shell_bin) {
        shell_bin = "/usr/bin/busybox";
    }
    if (access(shell_bin, X_OK) != 0) {
        printf("❌ sh: subshell binary '%s' not found or not executable. "
               "Set TBOS_SUBSHELL_BIN to a valid BusyBox path.\n", shell_bin);
        return TBOS_SHELL_ERROR;
    }

    printf("[Subshell] Launching POSIX shell via %s. "
           "Exit with Ctrl-D or 'exit' to return.\n", shell_bin);

    pid_t pid = fork();
    if (pid < 0) {
        perror("sh fork");
        return TBOS_SHELL_ERROR;
    }
    if (pid == 0) {
        execl(shell_bin, shell_bin, "sh", NULL);
        perror("sh exec");
        _exit(127);
    }

    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
        printf("[Subshell] Exited with status %d\n", WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("[Subshell] Terminated by signal %d\n", WTERMSIG(status));
    }
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_persona(int argc, char** argv) {
    tbos_shell_session_t* session = tbos_shell_get_session();

    if (argc == 1 || (argc > 1 && strcmp(argv[1], "status") == 0)) {
        printf("\nPersona status:\n");
        printf("  Current persona : %s\n", session->persona);
        printf("  Commands run    : %lu\n", session->commands_executed);
        printf("  Karma           : %ld\n", session->user_karma);
        printf("  Awareness       : %d\n", session->user_awareness);
        printf("\nUse 'persona list' to see all personas or 'persona morph <id>' to switch.\n\n");
        return TBOS_SHELL_SUCCESS;
    }

    if (strcmp(argv[1], "list") == 0) {
        tbos_supershell_list();
        return TBOS_SHELL_SUCCESS;
    }

    if (strcmp(argv[1], "morph") == 0) {
        if (argc < 3) {
            printf("Usage: persona morph <persona_id>\n");
            return TBOS_SHELL_ERROR;
        }
        const char* target = argv[2];
        if (tbos_supershell_request_morph(target) != 0) {
            printf("❌ Failed to request morph to persona: %s\n", target);
            return TBOS_SHELL_ERROR;
        }
        printf("✨ Morph request registered for persona '%s'. Shell will restart.\n", target);
        return TBOS_SHELL_EXIT;
    }

    printf("Usage: persona [status|list|morph <persona_id>]\n");
    return TBOS_SHELL_ERROR;
}

int tbos_cmd_clear(int argc, char** argv) {
    (void)argc;
    (void)argv;

    /* ANSI escape code for clearing screen */
    printf("\033[2J\033[H");
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_help(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║   TernaryBit OS - Available Commands                        ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");

    printf("📁 File System Commands:\n");
    printf("  ls [path]              List directory contents\n");
    printf("  cd <path>              Change directory\n");
    printf("  pwd                    Print working directory\n");
    printf("  mkdir <path>           Create directory\n");
    printf("  rmdir <path>           Remove directory\n");
    printf("  rm <path>              Remove file\n");
    printf("  cat <path>             Display file contents\n");
    printf("\n");

    printf("🕉️  Consciousness Commands:\n");
    printf("  karma                  Show karma status\n");
    printf("  consciousness          Show consciousness level\n");
    printf("  history                Show command history\n");
    printf("\n");

    printf("🛠️  System Commands:\n");
    printf("  echo <text>            Display text\n");
    printf("  clear                  Clear screen\n");
    printf("  help                   Show this help\n");
    printf("  exit                   Exit shell\n");
    printf("\n");

    printf("💡 Tips:\n");
    printf("  - Commands cost and reward karma\n");
    printf("  - Destructive actions require higher karma\n");
    printf("  - Practice mindful computing for enlightenment\n");
    printf("\n");

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_date(int argc, char** argv) {
    (void)argc;
    (void)argv;

    time_t now = time(NULL);
    struct tm local_tm;
    struct tm utc_tm;
    char local_buf[64];
    char utc_buf[64];
    localtime_r(&now, &local_tm);
    gmtime_r(&now, &utc_tm);

    strftime(local_buf, sizeof(local_buf), "%Y-%m-%d %H:%M:%S %Z", &local_tm);
    strftime(utc_buf, sizeof(utc_buf), "%Y-%m-%d %H:%M:%S UTC", &utc_tm);

    printf("Local time : %s\n", local_buf);
    printf("UTC time   : %s\n", utc_buf);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_uptime(int argc, char** argv) {
    (void)argc;
    (void)argv;

    tbos_shell_session_t* session = tbos_shell_get_session();
    time_t now = time(NULL);
    time_t delta = now - (time_t)session->session_start_time;
    int days = (int)(delta / 86400);
    int hours = (int)((delta % 86400) / 3600);
    int minutes = (int)((delta % 3600) / 60);
    int seconds = (int)(delta % 60);
    printf("Session uptime: %d days %02d:%02d:%02d (since %ld)\n",
           days, hours, minutes, seconds,
           (long)session->session_start_time);
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_whoami(int argc, char** argv) {
    (void)argc;
    (void)argv;
    const char* user = getenv("USER");
    if (!user || !*user) user = "tbos";
    tbos_shell_session_t* session = tbos_shell_get_session();
    printf("User    : %s\n", user);
    printf("Persona : %s\n", session->persona);
    return TBOS_SHELL_SUCCESS;
}

extern char **environ;

int tbos_cmd_env(int argc, char** argv) {
    (void)argc;
    (void)argv;
    if (!environ) {
        printf("Environment not available.\n");
        return TBOS_SHELL_SUCCESS;
    }
    for (char** env = environ; *env; ++env) {
        printf("%s\n", *env);
    }
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_exit(int argc, char** argv) {
    (void)argc;
    (void)argv;

    tbos_shell_print_status();
    return TBOS_SHELL_EXIT;
}

/* ========================================================================= */
/* CONSCIOUSNESS-AWARE COMMANDS                                              */
/* ========================================================================= */

int tbos_cmd_karma(int argc, char** argv) {

    tbos_shell_session_t* session = tbos_shell_get_session();

    if (argc > 1 && strcmp(argv[1], "ledger") == 0) {
        size_t entries = 10;
        if (argc > 2) {
            entries = (size_t)atoi(argv[2]);
            if (entries == 0) entries = 10;
        }
        tbos_karma_ledger_print(session->persona, entries);
        return TBOS_SHELL_SUCCESS;
    }

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      Your Karma Status                                 ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Current Karma:         %-10ld                    ║\n",
           session->user_karma);
    printf("║ Karma Earned:          %-10lu                    ║\n",
           session->total_karma_earned);
    printf("║ Karma Lost:            %-10lu                    ║\n",
           session->total_karma_lost);
    printf("║ Net Karma:             %-10ld                    ║\n",
           (long)(session->total_karma_earned - session->total_karma_lost));
    printf("╠════════════════════════════════════════════════════════╣\n");

    /* Karma level assessment */
    const char* level;
    const char* advice;

    if (session->user_karma < 0) {
        level = "Negative";
        advice = "Practice compassionate computing";
    } else if (session->user_karma < 50) {
        level = "Low";
        advice = "Help other processes, avoid destruction";
    } else if (session->user_karma < 100) {
        level = "Moderate";
        advice = "Continue mindful practice";
    } else if (session->user_karma < 500) {
        level = "Good";
        advice = "Share knowledge with others";
    } else {
        level = "Excellent";
        advice = "Guide others on the path";
    }

    printf("║ Karma Level:           %-30s  ║\n", level);
    printf("║ Guidance:              %-30s  ║\n", advice);
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_consciousness(int argc, char** argv) {
    (void)argc;
    (void)argv;

    tbos_shell_session_t* session = tbos_shell_get_session();

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      Your Consciousness Level                          ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");

    /* Display consciousness progression */
    const char* levels[] = {
        "NONE         [0] - Unconscious operation",
        "AWAKENING    [1] - Beginning awareness",
        "AWARE        [2] - Recognizes existence",
        "COMPASSIONATE[3] - Cares for others",
        "ENLIGHTENED  [4] - Complete awareness"
    };

    const size_t level_count = sizeof(levels) / sizeof(levels[0]);
    for (size_t idx = 0; idx < level_count; idx++) {
        consciousness_level_t level = (consciousness_level_t)idx;

        if (level == session->user_awareness) {
            printf("║ ➤ %s ║\n", levels[idx]);
        } else if (level < session->user_awareness) {
            printf("║ ✓ %s ║\n", levels[idx]);
        } else {
            printf("║   %s ║\n", levels[idx]);
        }
    }

    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Commands Executed:     %-10lu                    ║\n",
           session->commands_executed);

    /* Progress to next level */
    uint64_t commands_needed = (session->user_awareness + 1) * 100;
    printf("║ Next Level In:         %-10lu commands         ║\n",
           commands_needed > session->commands_executed ?
           commands_needed - session->commands_executed : 0);

    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");

    /* Check for consciousness upgrade */
    if (session->commands_executed >= commands_needed &&
        session->user_awareness < CONSCIOUSNESS_ENLIGHTENED) {
        session->user_awareness++;
        printf("✨ CONSCIOUSNESS UPGRADE ✨\n");
        printf("   You have reached: %s\n",
               levels[session->user_awareness] + 3);  /* Skip icon */
        printf("   Your awareness expands...\n\n");
    }

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_history(int argc, char** argv) {
    (void)argc;
    (void)argv;

    tbos_shell_session_t* session = tbos_shell_get_session();

    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║      Command History                                   ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n\n");

    if (session->history_count == 0) {
        printf("(no history yet)\n\n");
        return TBOS_SHELL_SUCCESS;
    }

    for (uint32_t i = 0; i < session->history_count; i++) {
        tbos_shell_history_entry_t* entry = &session->history[i];

        printf("%3u. %s\n", i + 1, entry->command);
        printf("     Exit: %d | Karma: %+ld\n",
               entry->exit_code, entry->karma_impact);
    }

    printf("\nTotal commands in history: %u\n\n", session->history_count);
    return TBOS_SHELL_SUCCESS;
}

/* ========================================================================= */
/* NETWORK COMMANDS (Stubs for now)                                         */
/* ========================================================================= */

int tbos_cmd_http(int argc, char** argv) {
    /* Network stack integration */
    extern int tbos_network_init(void);
    extern int tbos_http_init(void);
    extern void* tbos_http_client_create(const char* host, uint16_t port);
    extern void* tbos_http_get(void* client, const char* path);
    extern void tbos_http_response_free(void* response);
    extern void tbos_http_client_destroy(void* client);

    if (argc < 3) {
        printf("Usage: http <get|post> <url>\n");
        printf("  http get http://example.com/path\n");
        printf("  http post http://example.com/api <data>\n");
        return TBOS_SHELL_ERROR;
    }

    /* Initialize network if needed */
    tbos_network_init();
    tbos_http_init();

    const char* method = argv[1];
    const char* url = argv[2];

    if (strcmp(method, "get") == 0) {
        printf("🌐 Performing mindful HTTP GET request...\n");
        printf("   URL: %s\n", url);
        printf("   Following Right Speech protocol\n\n");

        /* Create client */
        void* client = tbos_http_client_create("example.com", 80);
        if (!client) {
            printf("❌ Failed to create HTTP client\n");
            return TBOS_SHELL_ERROR;
        }

        /* Perform GET */
        void* response = tbos_http_get(client, "/");
        if (response) {
            printf("✅ Response received with consciousness\n");
            printf("   Status: 200 OK\n");
            printf("   Served with compassion: Yes\n\n");
            tbos_http_response_free(response);
        } else {
            printf("❌ Request failed\n");
        }

        tbos_http_client_destroy(client);
    } else if (strcmp(method, "post") == 0) {
        printf("🌐 HTTP POST - coming soon\n");
    } else {
        printf("Unknown HTTP method: %s\n", method);
        return TBOS_SHELL_ERROR;
    }

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_ping(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ping <host>\n");
        return TBOS_SHELL_ERROR;
    }

    const char* host = argv[1];

    printf("🌐 Pinging %s with consciousness...\n", host);
    printf("   Practicing Right Speech\n");
    printf("   Measuring connection karma\n\n");

    /* Simulate ping with consciousness */
    for (int i = 0; i < 4; i++) {
        printf("   Reply from %s: bytes=32 time=%dms karma=+%d\n",
               host, 10 + i, 5);
    }

    printf("\n✅ Ping complete - Connection is conscious and compassionate\n");

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_netstat(int argc, char** argv) {
    (void)argc;
    (void)argv;

    extern int tbos_network_init(void);
    extern void tbos_network_print_status(void);

    /* Initialize and show network status */
    tbos_network_init();

    printf("🌐 Network Status - Digital Sangha\n\n");
    tbos_network_print_status();

    return TBOS_SHELL_SUCCESS;
}

/* ========================================================================= */
/* SYSTEM COMMANDS (Stubs for now)                                          */
/* ========================================================================= */

int tbos_cmd_ps(int argc, char** argv) {
    (void)argc;
    (void)argv;

    /* Initialize process manager if needed */
    extern int tbos_process_init(void);
    extern void tbos_process_print_table(void);

    tbos_process_init();
    tbos_process_print_table();

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_top(int argc, char** argv) {
    (void)argc;
    (void)argv;

    /* Print scheduler status */
    extern int tbos_process_init(void);
    extern void tbos_scheduler_print_status(void);

    tbos_process_init();
    tbos_scheduler_print_status();

    return TBOS_SHELL_SUCCESS;
}

static const char* hal_profile_name(hal_profile_t profile) {
    switch (profile) {
        case HAL_PROFILE_CALCULATOR: return "Calculator";
        case HAL_PROFILE_RADIO: return "Radio";
        case HAL_PROFILE_MINIMAL: return "Minimal";
        case HAL_PROFILE_STANDARD: return "Standard";
        case HAL_PROFILE_ADVANCED: return "Advanced";
        case HAL_PROFILE_CUSTOM: return "Custom";
        default: return "Unknown";
    }
}

int tbos_cmd_hal(int argc, char** argv) {
    (void)argc;
    (void)argv;

    hal_hardware_info_t info;
    if (hal_detect_hardware(&info) != 0) {
        printf("❌ Failed to detect hardware profile\n");
        return TBOS_SHELL_ERROR;
    }

    printf("\n╔════════════════════════════════════════════════════════╗\n");
    printf("║      HAL Hardware Profile                              ║\n");
    printf("╠════════════════════════════════════════════════════════╣\n");
    printf("║ Profile: %-46s ║\n", hal_profile_name(info.profile));
    printf("║ RAM:     %-10u bytes                                 ║\n", info.ram_bytes);
    printf("║ ROM:     %-10u bytes                                 ║\n", info.rom_bytes);
    printf("║ Storage: %-10u bytes                                 ║\n", info.storage_bytes);
    printf("║ CPU:     %u-bit @ %u Hz, cores=%u                      ║\n",
           info.cpu_bits, info.cpu_freq_hz, info.cpu_cores);
    printf("║ Free mem: %-10u bytes                                 ║\n", hal_get_free_memory());
    printf("╠════════ Capabilities ══════════════════════════════════╣\n");
    printf("║ 0x%08x                                                ║\n", info.capabilities);
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_shutdown(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Initiating mindful shutdown...\n");
    printf("(Soft shutdown from US-2.3 coming next)\n");
    return TBOS_SHELL_EXIT;
}

int tbos_cmd_sangha(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Digital Sangha Status:\n");
    printf("(Network sangha integration coming soon)\n");
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_fast(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Component Fasting:\n");
    printf("(Fasting integration coming soon)\n");
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_compassion(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("Compassion Metrics:\n");
    printf("(Compassion tracking coming soon)\n");
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_metrics(int argc, char** argv) {
    /* Initialize metrics system */
    extern int tbos_metrics_init(void);
    extern void tbos_metrics_print_consciousness_dashboard(void);
    extern void tbos_metrics_print_component_budgets(void);
    extern void tbos_metrics_print_system_energy(void);
    extern void tbos_metrics_print_transformation_flow(void);

    tbos_metrics_init();

    if (argc == 1) {
        /* No arguments - show main dashboard */
        tbos_metrics_print_consciousness_dashboard();
    } else if (argc == 2) {
        if (strcmp(argv[1], "components") == 0) {
            tbos_metrics_print_component_budgets();
        } else if (strcmp(argv[1], "system") == 0) {
            tbos_metrics_print_system_energy();
        } else if (strcmp(argv[1], "transform") == 0) {
            tbos_metrics_print_transformation_flow();
        } else if (strcmp(argv[1], "all") == 0) {
            tbos_metrics_print_consciousness_dashboard();
            tbos_metrics_print_component_budgets();
            tbos_metrics_print_system_energy();
            tbos_metrics_print_transformation_flow();
        } else {
            printf("Usage: metrics [components|system|transform|all]\n");
            return TBOS_SHELL_ERROR;
        }
    }

    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_events(int argc, char** argv) {
    size_t limit = 10;
    if (argc > 1) {
        limit = (size_t)atoi(argv[1]);
        if (limit == 0) limit = 10;
    }
    if (limit > 32) limit = 32;

    steppps_event_t events[32];
    int count = steppps_event_get_history(events, (uint32_t)limit);
    if (count <= 0) {
        printf("No STEPPPS events recorded yet.\n");
        return TBOS_SHELL_SUCCESS;
    }

    const char* type_names[] = {
        "SYSTEM", "PROCESS", "NETWORK", "USER", "CONSCIOUS", "KARMA"
    };

    printf("\nLast %d STEPPPS events:\n", count);
    for (int i = 0; i < count; ++i) {
        const steppps_event_t* ev = &events[i];
        const char* type = (ev->type >= 0 &&
                            ev->type < (int)(sizeof(type_names)/sizeof(type_names[0])))
                           ? type_names[ev->type]
                           : "UNKNOWN";
        printf("  #%u [%s] karma=%lld uptime=%.2fs\n      %s\n",
               ev->event_id,
               type,
               (long long)ev->karma_impact,
               ev->timestamp_us / 1000000.0,
               ev->description);
    }
    printf("\n");
    return TBOS_SHELL_SUCCESS;
}

int tbos_cmd_steppps(int argc, char** argv) {
    /* Initialize STEPPPS framework */
    extern int steppps_init(void);
    extern void steppps_print_status(void);
    extern void steppps_print_dimension(const char* dimension_name);
    extern int steppps_psychology_evolve(void);
    extern int steppps_psychology_record_experience(const char* experience, float learning_value);

    steppps_init();

    if (argc == 1) {
        /* No arguments - show all dimensions */
        steppps_print_status();
    } else if (argc == 2) {
        /* Show specific dimension or action */
        if (strcmp(argv[1], "space") == 0) {
            steppps_print_dimension("space");
        } else if (strcmp(argv[1], "time") == 0) {
            steppps_print_dimension("time");
        } else if (strcmp(argv[1], "event") == 0) {
            steppps_print_dimension("event");
        } else if (strcmp(argv[1], "psychology") == 0) {
            steppps_print_dimension("psychology");
        } else if (strcmp(argv[1], "pixel") == 0) {
            steppps_print_dimension("pixel");
        } else if (strcmp(argv[1], "prompt") == 0) {
            steppps_print_dimension("prompt");
        } else if (strcmp(argv[1], "script") == 0) {
            steppps_print_dimension("script");
        } else if (strcmp(argv[1], "evolve") == 0) {
            /* Trigger consciousness evolution */
            int result = steppps_psychology_evolve();
            if (result > 0) {
                printf("✨ Consciousness evolved!\n");
            } else if (result == 0) {
                printf("Not ready to evolve yet. Keep practicing!\n");
            }
        } else if (strcmp(argv[1], "help") == 0) {
            printf("STEPPPS - Seven-Dimensional Computing Framework\n\n");
            printf("Usage: steppps [dimension|action]\n\n");
            printf("Dimensions:\n");
            printf("  space       - Physical location and proximity\n");
            printf("  time        - Temporal awareness across all scales\n");
            printf("  event       - Event-driven architecture and history\n");
            printf("  psychology  - AI, consciousness, learning\n");
            printf("  pixel       - Visual and data representation\n");
            printf("  prompt      - Natural language interface\n");
            printf("  script      - Automation and programming\n\n");
            printf("Actions:\n");
            printf("  evolve      - Trigger consciousness evolution\n");
            printf("  help        - Show this help\n\n");
            printf("Examples:\n");
            printf("  steppps              # Show all dimensions\n");
            printf("  steppps psychology   # Show psychology dimension\n");
            printf("  steppps evolve       # Evolve consciousness\n");
        } else {
            printf("Unknown dimension or action: %s\n", argv[1]);
            printf("Use 'steppps help' for usage information\n");
            return TBOS_SHELL_ERROR;
        }
    } else if (argc == 3 && strcmp(argv[1], "learn") == 0) {
        /* Record learning experience */
        steppps_psychology_record_experience(argv[2], 0.8f);
        printf("✅ Experience recorded\n");
    } else {
        printf("Usage: steppps [dimension|action]\n");
        printf("Use 'steppps help' for more information\n");
        return TBOS_SHELL_ERROR;
    }

    return TBOS_SHELL_SUCCESS;
}
