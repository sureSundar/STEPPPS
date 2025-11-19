/**
 * @file commands_week1.c
 * @brief Week 1 File Operation Commands (Days 2-9)
 *
 * SPEED & SCALE MODE: 8 commands/week
 * Week 1: Core File Operations
 *
 * Commands 2-9:
 * - Day 2: ls   (list directory)
 * - Day 3: cd   (change directory)
 * - Day 4: pwd  (print working directory)
 * - Day 5: mkdir (make directory)
 * - Day 6: rmdir (remove empty directory)
 * - Day 7: rm   (remove files/directories)
 * - Day 8: cp   (copy files)
 * - Day 9: mv   (move/rename files)
 *
 * @date 2025-11-05
 * @author TernaryBit OS Team (FULL SPEED MODE ACTIVATED)
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* get_file_type_symbol(mode_t mode) {
    if (S_ISDIR(mode)) return "d";
    if (S_ISLNK(mode)) return "l";
    return "-";
}

static void format_size_human(off_t size, char* buf, size_t bufsize) {
    if (size < 1024) {
        snprintf(buf, bufsize, "%ldB", (long)size);
    } else if (size < 1024 * 1024) {
        snprintf(buf, bufsize, "%.1fK", size / 1024.0);
    } else if (size < 1024 * 1024 * 1024) {
        snprintf(buf, bufsize, "%.1fM", size / (1024.0 * 1024));
    } else {
        snprintf(buf, bufsize, "%.1fG", size / (1024.0 * 1024 * 1024));
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 2: ls - List Directory Contents
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_ls(int argc, char** argv) {
    const char* path = ".";
    bool long_format = false;
    bool show_all = false;
    bool human_readable = false;

    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            for (const char* p = &argv[i][1]; *p; p++) {
                switch (*p) {
                    case 'l': long_format = true; break;
                    case 'a': show_all = true; break;
                    case 'h': human_readable = true; break;
                    default:
                        fprintf(stderr, "ls: invalid option -- '%c'\n", *p);
                        return 1;
                }
            }
        } else {
            path = argv[i];
        }
    }

    /* Open directory */
    DIR* dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "ls: cannot access '%s': %s\n", path, strerror(errno));
        return 1;
    }

    struct dirent* entry;
    int count = 0;

    /* List entries */
    while ((entry = readdir(dir)) != NULL) {
        /* Skip hidden files unless -a */
        if (!show_all && entry->d_name[0] == '.') {
            continue;
        }

        count++;

        if (long_format) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            struct stat st;
            if (stat(full_path, &st) == 0) {
                char size_buf[16];
                if (human_readable) {
                    format_size_human(st.st_size, size_buf, sizeof(size_buf));
                } else {
                    snprintf(size_buf, sizeof(size_buf), "%10ld", (long)st.st_size);
                }

                char time_buf[64];
                struct tm* tm = localtime(&st.st_mtime);
                strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm);

                printf("%srwxr-xr-x %s %s %s\n",
                       get_file_type_symbol(st.st_mode),
                       size_buf,
                       time_buf,
                       entry->d_name);
            }
        } else {
            printf("%s  ", entry->d_name);
        }
    }

    if (!long_format) {
        printf("\n");
    }

    closedir(dir);

    /* Karma: listing directories is mindful observation */
    universal_add_karma(1, "Observed directory contents mindfully");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 3: cd - Change Directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_cd(int argc, char** argv) {
    const char* path = getenv("HOME");

    if (argc > 1) {
        path = argv[1];
    }

    if (!path) {
        path = "/";  /* Fallback to root */
    }

    if (chdir(path) != 0) {
        fprintf(stderr, "cd: %s: %s\n", path, strerror(errno));
        return 1;
    }

    /* Update PWD */
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd))) {
        setenv("PWD", cwd, 1);
        /* Update shell session */
        universal_shell_session_t* session = universal_get_session();
        strncpy(session->current_directory, cwd, sizeof(session->current_directory) - 1);
    }

    /* Karma: changing perspective */
    universal_add_karma(1, "Changed perspective (directory)");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 4: pwd - Print Working Directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pwd(int argc, char** argv) {
    (void)argc;
    (void)argv;

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fprintf(stderr, "pwd: %s\n", strerror(errno));
        return 1;
    }

    printf("%s\n", cwd);

    /* Karma: knowing where you are is mindfulness */
    universal_add_karma(1, "Aware of current location");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 5: mkdir - Make Directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_mkdir(int argc, char** argv) {
    bool make_parents = false;
    mode_t mode = 0755;

    int first_arg = 1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            make_parents = true;
            first_arg++;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "mkdir: invalid option '%s'\n", argv[i]);
            return 1;
        } else {
            break;
        }
    }

    if (first_arg >= argc) {
        fprintf(stderr, "mkdir: missing operand\n");
        return 1;
    }

    /* Create directories */
    for (int i = first_arg; i < argc; i++) {
        const char* path = argv[i];

        if (make_parents) {
            /* Create parent directories */
            char tmp[PATH_MAX];
            snprintf(tmp, sizeof(tmp), "%s", path);

            for (char* p = tmp + 1; *p; p++) {
                if (*p == '/') {
                    *p = '\0';
                    mkdir(tmp, mode);
                    *p = '/';
                }
            }
        }

        if (mkdir(path, mode) != 0 && errno != EEXIST) {
            fprintf(stderr, "mkdir: cannot create directory '%s': %s\n",
                    path, strerror(errno));
            return 1;
        }
    }

    /* Karma: creating space for others */
    universal_add_karma(2, "Created space (directory)");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 6: rmdir - Remove Empty Directory
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_rmdir(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "rmdir: missing operand\n");
        return 1;
    }

    int ret = 0;
    for (int i = 1; i < argc; i++) {
        if (rmdir(argv[i]) != 0) {
            fprintf(stderr, "rmdir: failed to remove '%s': %s\n",
                    argv[i], strerror(errno));
            ret = 1;
        }
    }

    if (ret == 0) {
        /* Karma: mindful cleanup */
        universal_add_karma(1, "Mindfully cleaned empty space");
    }

    return ret;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 7: rm - Remove Files/Directories
 * ═══════════════════════════════════════════════════════════════════════════ */

static int rm_recursive(const char* path) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        return -1;
    }

    if (S_ISDIR(st.st_mode)) {
        DIR* dir = opendir(path);
        if (!dir) return -1;

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            if (rm_recursive(full_path) != 0) {
                closedir(dir);
                return -1;
            }
        }
        closedir(dir);

        return rmdir(path);
    } else {
        return unlink(path);
    }
}

static int cmd_rm(int argc, char** argv) {
    bool recursive = false;
    bool force = false;

    int first_arg = 1;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            for (const char* p = &argv[i][1]; *p; p++) {
                switch (*p) {
                    case 'r':
                    case 'R': recursive = true; break;
                    case 'f': force = true; break;
                    default:
                        fprintf(stderr, "rm: invalid option -- '%c'\n", *p);
                        return 1;
                }
            }
            first_arg++;
        } else {
            break;
        }
    }

    if (first_arg >= argc) {
        fprintf(stderr, "rm: missing operand\n");
        return 1;
    }

    int ret = 0;
    for (int i = first_arg; i < argc; i++) {
        const char* path = argv[i];

        if (recursive) {
            if (rm_recursive(path) != 0 && !force) {
                fprintf(stderr, "rm: cannot remove '%s': %s\n", path, strerror(errno));
                ret = 1;
            }
        } else {
            if (unlink(path) != 0 && !force) {
                fprintf(stderr, "rm: cannot remove '%s': %s\n", path, strerror(errno));
                ret = 1;
            }
        }
    }

    /* Karma: letting go (but responsibly) */
    if (ret == 0) {
        universal_add_karma(1, "Released attachment (deleted file)");
    }

    return ret;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 8: cp - Copy Files
 * ═══════════════════════════════════════════════════════════════════════════ */

static int copy_file(const char* src, const char* dst) {
    FILE* src_file = fopen(src, "rb");
    if (!src_file) {
        return -1;
    }

    FILE* dst_file = fopen(dst, "wb");
    if (!dst_file) {
        fclose(src_file);
        return -1;
    }

    char buffer[8192];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0) {
        if (fwrite(buffer, 1, bytes, dst_file) != bytes) {
            fclose(src_file);
            fclose(dst_file);
            return -1;
        }
    }

    fclose(src_file);
    fclose(dst_file);

    /* Copy permissions */
    struct stat st;
    if (stat(src, &st) == 0) {
        chmod(dst, st.st_mode);
    }

    return 0;
}

static int cmd_cp(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "cp: missing file operand\n");
        return 1;
    }

    const char* dst = argv[argc - 1];
    int num_src = argc - 2;

    /* Check if destination is directory */
    struct stat dst_st;
    bool dst_is_dir = (stat(dst, &dst_st) == 0 && S_ISDIR(dst_st.st_mode));

    if (num_src > 1 && !dst_is_dir) {
        fprintf(stderr, "cp: target '%s' is not a directory\n", dst);
        return 1;
    }

    for (int i = 1; i < argc - 1; i++) {
        const char* src = argv[i];
        char dst_path[PATH_MAX];

        if (dst_is_dir) {
            const char* basename = strrchr(src, '/');
            basename = basename ? basename + 1 : src;
            snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, basename);
        } else {
            snprintf(dst_path, sizeof(dst_path), "%s", dst);
        }

        if (copy_file(src, dst_path) != 0) {
            fprintf(stderr, "cp: cannot copy '%s' to '%s': %s\n",
                    src, dst_path, strerror(errno));
            return 1;
        }
    }

    /* Karma: preserving knowledge */
    universal_add_karma(2, "Preserved data (copied file)");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 9: mv - Move/Rename Files
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_mv(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "mv: missing file operand\n");
        return 1;
    }

    const char* dst = argv[argc - 1];
    int num_src = argc - 2;

    /* Check if destination is directory */
    struct stat dst_st;
    bool dst_is_dir = (stat(dst, &dst_st) == 0 && S_ISDIR(dst_st.st_mode));

    if (num_src > 1 && !dst_is_dir) {
        fprintf(stderr, "mv: target '%s' is not a directory\n", dst);
        return 1;
    }

    for (int i = 1; i < argc - 1; i++) {
        const char* src = argv[i];
        char dst_path[PATH_MAX];

        if (dst_is_dir) {
            const char* basename = strrchr(src, '/');
            basename = basename ? basename + 1 : src;
            snprintf(dst_path, sizeof(dst_path), "%s/%s", dst, basename);
        } else {
            snprintf(dst_path, sizeof(dst_path), "%s", dst);
        }

        /* Try rename first */
        if (rename(src, dst_path) != 0) {
            /* If rename fails, try copy + delete */
            if (copy_file(src, dst_path) != 0) {
                fprintf(stderr, "mv: cannot move '%s' to '%s': %s\n",
                        src, dst_path, strerror(errno));
                return 1;
            }
            unlink(src);
        }
    }

    /* Karma: transformation */
    universal_add_karma(2, "Transformed location (moved file)");

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 1 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week1_commands(void) {
    // REVIEW(CX): universal_shell_register_command() isn't declared/defined anywhere,
    // so this registration layer won't compile until a wrapper around universal_register_command()
    // (or equivalent) is introduced.
    universal_shell_register_command("ls", cmd_ls,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "list directory contents",
        "ls [OPTION]... [FILE]...\n\n"
        "List information about files (current directory by default).\n\n"
        "Options:\n"
        "  -a    show all files (including hidden)\n"
        "  -l    long listing format\n"
        "  -h    human-readable sizes (with -l)");

    universal_shell_register_command("cd", cmd_cd,
        CMD_CAT_SHELL_BUILTIN, OS_SUPPORT_ALL,
        "change directory",
        "cd [DIRECTORY]\n\n"
        "Change working directory to DIRECTORY.\n"
        "Defaults to HOME if no directory specified.");

    universal_shell_register_command("pwd", cmd_pwd,
        CMD_CAT_SHELL_BUILTIN, OS_SUPPORT_ALL,
        "print working directory",
        "pwd\n\n"
        "Print the current working directory.");

    universal_shell_register_command("mkdir", cmd_mkdir,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "make directories",
        "mkdir [OPTION]... DIRECTORY...\n\n"
        "Create DIRECTORY(ies) if they don't exist.\n\n"
        "Options:\n"
        "  -p    make parent directories as needed");

    universal_shell_register_command("rmdir", cmd_rmdir,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "remove empty directories",
        "rmdir DIRECTORY...\n\n"
        "Remove empty DIRECTORY(ies).");

    universal_shell_register_command("rm", cmd_rm,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "remove files or directories",
        "rm [OPTION]... FILE...\n\n"
        "Remove (unlink) FILE(s).\n\n"
        "Options:\n"
        "  -r, -R    remove directories recursively\n"
        "  -f        force, ignore nonexistent files");

    universal_shell_register_command("cp", cmd_cp,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "copy files and directories",
        "cp SOURCE... DEST\n\n"
        "Copy SOURCE to DEST or multiple SOURCE(s) to DIRECTORY.");

    universal_shell_register_command("mv", cmd_mv,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "move (rename) files",
        "mv SOURCE DEST\n\n"
        "Rename SOURCE to DEST or move SOURCE(s) to DIRECTORY.");

    printf("🚀 [Week 1] Registered 8 file operation commands\n");
    printf("   Commands 2-9: ls, cd, pwd, mkdir, rmdir, rm, cp, mv\n");
}
