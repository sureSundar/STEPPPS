/**
 * @file commands_week3.c
 * @brief Week 3 File Permissions & Info Commands (Days 18-25)
 *
 * GO MODE: Full speed ahead!
 * Week 3: Permissions, Links, and File Information
 *
 * Commands 18-25:
 * - Day 18: touch  (create/update file timestamps)
 * - Day 19: chmod  (change file permissions)
 * - Day 20: chown  (change file owner)
 * - Day 21: chgrp  (change file group)
 * - Day 22: ln     (create links)
 * - Day 23: stat   (file status/info)
 * - Day 24: du     (disk usage)
 * - Day 25: df     (disk free space)
 *
 * @date 2025-11-05
 * @author TernaryBit OS Team (GO SPEED MODE)
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <utime.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void format_size(off_t size, char* buf, size_t bufsize, bool human) {
    if (!human) {
        snprintf(buf, bufsize, "%ld", (long)size);
        return;
    }

    if (size < 1024) {
        snprintf(buf, bufsize, "%ldB", (long)size);
    } else if (size < 1024 * 1024) {
        snprintf(buf, bufsize, "%.1fK", size / 1024.0);
    } else if (size < 1024L * 1024 * 1024) {
        snprintf(buf, bufsize, "%.1fM", size / (1024.0 * 1024));
    } else {
        snprintf(buf, bufsize, "%.1fG", size / (1024.0 * 1024 * 1024));
    }
}

static void format_permissions(mode_t mode, char* buf) {
    sprintf(buf, "%c%c%c%c%c%c%c%c%c",
        (mode & S_IRUSR) ? 'r' : '-',
        (mode & S_IWUSR) ? 'w' : '-',
        (mode & S_IXUSR) ? 'x' : '-',
        (mode & S_IRGRP) ? 'r' : '-',
        (mode & S_IWGRP) ? 'w' : '-',
        (mode & S_IXGRP) ? 'x' : '-',
        (mode & S_IROTH) ? 'r' : '-',
        (mode & S_IWOTH) ? 'w' : '-',
        (mode & S_IXOTH) ? 'x' : '-');
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 18: touch - Create/Update File Timestamps
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "touch: missing file operand\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        const char* path = argv[i];

        /* Try to update timestamp */
        if (utime(path, NULL) == 0) {
            continue; /* Success */
        }

        /* If file doesn't exist, create it */
        if (errno == ENOENT) {
            FILE* fp = fopen(path, "w");
            if (!fp) {
                fprintf(stderr, "touch: cannot touch '%s': %s\n", path, strerror(errno));
                return 1;
            }
            fclose(fp);
        } else {
            fprintf(stderr, "touch: cannot touch '%s': %s\n", path, strerror(errno));
            return 1;
        }
    }

    universal_add_karma(1, "Created presence (file)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 19: chmod - Change File Permissions
 * ═══════════════════════════════════════════════════════════════════════════ */

static mode_t parse_mode(const char* mode_str) {
    mode_t mode = 0;

    /* Octal mode: 644, 755, etc */
    if (mode_str[0] >= '0' && mode_str[0] <= '7') {
        return (mode_t)strtol(mode_str, NULL, 8);
    }

    /* Symbolic mode: simplified version */
    /* u+x, g+w, o+r, etc - not fully implemented */
    return 0644; /* Default fallback */
}

static int cmd_chmod(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "chmod: missing operand\n");
        fprintf(stderr, "usage: chmod MODE FILE...\n");
        return 1;
    }

    mode_t mode = parse_mode(argv[1]);

    for (int i = 2; i < argc; i++) {
        if (chmod(argv[i], mode) != 0) {
            fprintf(stderr, "chmod: cannot change '%s': %s\n", argv[i], strerror(errno));
            return 1;
        }
    }

    universal_add_karma(2, "Set boundaries (permissions)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 20: chown - Change File Owner
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_chown(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "chown: missing operand\n");
        fprintf(stderr, "usage: chown USER[:GROUP] FILE...\n");
        return 1;
    }

    /* Parse owner:group */
    char* owner_str = strdup(argv[1]);
    char* group_str = strchr(owner_str, ':');
    if (group_str) {
        *group_str++ = '\0';
    }

    uid_t uid = -1;
    gid_t gid = -1;

    /* Lookup user */
    if (owner_str && *owner_str) {
        struct passwd* pw = getpwnam(owner_str);
        if (pw) {
            uid = pw->pw_uid;
        } else {
            uid = atoi(owner_str); /* Try numeric UID */
        }
    }

    /* Lookup group */
    if (group_str && *group_str) {
        struct group* gr = getgrnam(group_str);
        if (gr) {
            gid = gr->gr_gid;
        } else {
            gid = atoi(group_str); /* Try numeric GID */
        }
    }

    /* Change ownership */
    for (int i = 2; i < argc; i++) {
        if (chown(argv[i], uid, gid) != 0) {
            fprintf(stderr, "chown: cannot change '%s': %s\n", argv[i], strerror(errno));
            free(owner_str);
            return 1;
        }
    }

    free(owner_str);
    universal_add_karma(2, "Transferred stewardship (ownership)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 21: chgrp - Change File Group
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_chgrp(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "chgrp: missing operand\n");
        fprintf(stderr, "usage: chgrp GROUP FILE...\n");
        return 1;
    }

    /* Lookup group */
    gid_t gid;
    struct group* gr = getgrnam(argv[1]);
    if (gr) {
        gid = gr->gr_gid;
    } else {
        gid = atoi(argv[1]); /* Try numeric GID */
    }

    /* Change group */
    for (int i = 2; i < argc; i++) {
        if (chown(argv[i], -1, gid) != 0) {
            fprintf(stderr, "chgrp: cannot change '%s': %s\n", argv[i], strerror(errno));
            return 1;
        }
    }

    universal_add_karma(1, "Changed community (group)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 22: ln - Create Links
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_ln(int argc, char** argv) {
    bool symbolic = false;
    int first_arg = 1;

    /* Parse -s option */
    if (argc > 1 && strcmp(argv[1], "-s") == 0) {
        symbolic = true;
        first_arg = 2;
    }

    if (argc - first_arg < 2) {
        fprintf(stderr, "ln: missing file operand\n");
        fprintf(stderr, "usage: ln [-s] TARGET LINK_NAME\n");
        return 1;
    }

    const char* target = argv[first_arg];
    const char* linkname = argv[first_arg + 1];

    int result;
    if (symbolic) {
        result = symlink(target, linkname);
    } else {
        result = link(target, linkname);
    }

    if (result != 0) {
        fprintf(stderr, "ln: cannot create link '%s': %s\n", linkname, strerror(errno));
        return 1;
    }

    universal_add_karma(2, "Created connection (link)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 23: stat - File Status/Info
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* file_type(mode_t mode) {
    if (S_ISREG(mode)) return "regular file";
    if (S_ISDIR(mode)) return "directory";
    if (S_ISLNK(mode)) return "symbolic link";
    if (S_ISBLK(mode)) return "block device";
    if (S_ISCHR(mode)) return "character device";
    if (S_ISFIFO(mode)) return "FIFO/pipe";
    if (S_ISSOCK(mode)) return "socket";
    return "unknown";
}

static int cmd_stat(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "stat: missing operand\n");
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        struct stat st;
        if (lstat(argv[i], &st) != 0) {
            fprintf(stderr, "stat: cannot stat '%s': %s\n", argv[i], strerror(errno));
            continue;
        }

        printf("  File: %s\n", argv[i]);
        printf("  Size: %-15ld Blocks: %-10ld IO Block: %ld\n",
               (long)st.st_size, (long)st.st_blocks, (long)st.st_blksize);

        char perms[10];
        format_permissions(st.st_mode, perms);
        printf("Device: %lxh/%ldd  Inode: %-10ld Links: %ld\n",
               (unsigned long)st.st_dev, (unsigned long)st.st_dev,
               (long)st.st_ino, (long)st.st_nlink);

        printf("Access: (%04o/%s)  Uid: (%5d)  Gid: (%5d)\n",
               st.st_mode & 07777, perms, st.st_uid, st.st_gid);

        char timebuf[64];
        struct tm* tm;

        tm = localtime(&st.st_atime);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);
        printf("Access: %s\n", timebuf);

        tm = localtime(&st.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);
        printf("Modify: %s\n", timebuf);

        tm = localtime(&st.st_ctime);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);
        printf("Change: %s\n", timebuf);

        printf(" Birth: -\n");

        if (i < argc - 1) printf("\n");
    }

    universal_add_karma(1, "Examined deeply (stat)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 24: du - Disk Usage
 * ═══════════════════════════════════════════════════════════════════════════ */

static off_t du_recursive(const char* path, bool human, bool summarize) {
    struct stat st;
    if (lstat(path, &st) != 0) {
        return 0;
    }

    off_t total = st.st_blocks * 512; /* blocks are 512 bytes */

    if (S_ISDIR(st.st_mode)) {
        DIR* dir = opendir(path);
        if (!dir) return total;

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char full_path[4096];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            off_t subtotal = du_recursive(full_path, human, summarize);
            total += subtotal;

            if (!summarize) {
                char size_str[32];
                format_size(subtotal, size_str, sizeof(size_str), human);
                printf("%s\t%s\n", size_str, full_path);
            }
        }
        closedir(dir);
    }

    return total;
}

static int cmd_du(int argc, char** argv) {
    bool human = false;
    bool summarize = false;
    int first_arg = 1;

    /* Parse options */
    for (int i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-h") == 0) human = true;
        else if (strcmp(argv[i], "-s") == 0) summarize = true;
        first_arg++;
    }

    if (first_arg >= argc) {
        first_arg = argc;
        argv[argc++] = "."; /* Default to current directory */
    }

    for (int i = first_arg; i < argc; i++) {
        off_t total = du_recursive(argv[i], human, summarize);

        char size_str[32];
        format_size(total, size_str, sizeof(size_str), human);
        printf("%s\t%s\n", size_str, argv[i]);
    }

    universal_add_karma(1, "Measured consumption (disk usage)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 25: df - Disk Free Space
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_df(int argc, char** argv) {
    bool human = false;

    /* Parse options */
    for (int i = 1; i < argc && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-h") == 0) human = true;
    }

    /* Print header */
    if (human) {
        printf("Filesystem      Size  Used Avail Use%% Mounted on\n");
    } else {
        printf("Filesystem     1K-blocks    Used Available Use%% Mounted on\n");
    }

    /* Get filesystem info for / */
    struct statvfs vfs;
    if (statvfs("/", &vfs) == 0) {
        unsigned long block_size = vfs.f_frsize;
        unsigned long total = (vfs.f_blocks * block_size) / 1024;
        unsigned long avail = (vfs.f_bavail * block_size) / 1024;
        unsigned long used = total - avail;
        int use_percent = total ? (used * 100) / total : 0;

        if (human) {
            char total_str[32], used_str[32], avail_str[32];
            format_size(total * 1024, total_str, sizeof(total_str), true);
            format_size(used * 1024, used_str, sizeof(used_str), true);
            format_size(avail * 1024, avail_str, sizeof(avail_str), true);
            printf("%-15s %5s %5s %5s %3d%% /\n",
                   "rootfs", total_str, used_str, avail_str, use_percent);
        } else {
            printf("%-15s %10lu %10lu %10lu %3d%% /\n",
                   "rootfs", total, used, avail, use_percent);
        }
    }

    universal_add_karma(1, "Assessed resources (disk space)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 3 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week3_commands(void) {
    universal_shell_register_command("touch", cmd_touch,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "change file timestamps",
        "touch FILE...\n\n"
        "Update access/modification times.\n"
        "Create file if it doesn't exist.");

    universal_shell_register_command("chmod", cmd_chmod,
        CMD_CAT_PERMISSIONS, OS_SUPPORT_ALL,
        "change file mode bits",
        "chmod MODE FILE...\n\n"
        "Change file permissions.\n"
        "MODE: octal like 644, 755, etc.");

    universal_shell_register_command("chown", cmd_chown,
        CMD_CAT_PERMISSIONS, OS_SUPPORT_ALL,
        "change file owner and group",
        "chown USER[:GROUP] FILE...\n\n"
        "Change file owner and optionally group.");

    universal_shell_register_command("chgrp", cmd_chgrp,
        CMD_CAT_PERMISSIONS, OS_SUPPORT_ALL,
        "change group ownership",
        "chgrp GROUP FILE...\n\n"
        "Change file group ownership.");

    universal_shell_register_command("ln", cmd_ln,
        CMD_CAT_FILE_OPS, OS_SUPPORT_ALL,
        "make links between files",
        "ln [-s] TARGET LINK_NAME\n\n"
        "Create hard or symbolic links.\n\n"
        "Options:\n"
        "  -s    create symbolic link");

    universal_shell_register_command("stat", cmd_stat,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "display file status",
        "stat FILE...\n\n"
        "Display detailed file information.");

    universal_shell_register_command("du", cmd_du,
        CMD_CAT_DISK_STORAGE, OS_SUPPORT_ALL,
        "estimate file space usage",
        "du [OPTION]... [FILE]...\n\n"
        "Estimate disk usage.\n\n"
        "Options:\n"
        "  -h    human-readable sizes\n"
        "  -s    display only total");

    universal_shell_register_command("df", cmd_df,
        CMD_CAT_DISK_STORAGE, OS_SUPPORT_ALL,
        "report file system disk space usage",
        "df [OPTION]...\n\n"
        "Show disk space usage.\n\n"
        "Options:\n"
        "  -h    human-readable sizes");

    printf("🚀 [Week 3] Registered 8 permission & info commands\n");
    printf("   Commands 18-25: touch, chmod, chown, chgrp, ln, stat, du, df\n");
}
