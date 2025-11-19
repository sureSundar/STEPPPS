/**
 * @file commands_week10.c
 * @brief Week 10 System Information Commands (Days 64-71)
 *
 * Commands:
 *  - uname     : Print kernel/system information
 *  - hostname  : Display current hostname
 *  - date      : Show current date/time (optional format)
 *  - uptime    : Display system uptime and load (GNU/Linux)
 *  - whoami    : Show effective user name
 *  - id        : Print user and group identities
 *  - groups    : List supplementary groups
 *  - cal       : Display a monthly calendar
 */

#include "tbos_universal_shell.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/utsname.h>

#ifdef __linux__
#include <sys/sysinfo.h>
#endif

#include <grp.h>
#include <pwd.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * uname
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_uname(int argc, char** argv) {
    struct utsname uts;
    if (uname(&uts) != 0) {
        fprintf(stderr, "uname: %s\n", strerror(errno));
        return 1;
    }

    bool all = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            all = true;
        } else {
            fprintf(stderr, "uname: invalid option '%s'\n", argv[i]);
            return 1;
        }
    }

    if (all) {
        printf("%s %s %s %s %s\n",
               uts.sysname,
               uts.nodename,
               uts.release,
               uts.version,
               uts.machine);
    } else {
        printf("%s\n", uts.sysname);
    }

    universal_add_karma(1, "Observed system nature (uname)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * hostname
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_hostname(int argc, char** argv) {
    (void)argc;
    (void)argv;

    char buf[256];
    if (gethostname(buf, sizeof(buf)) != 0) {
        fprintf(stderr, "hostname: %s\n", strerror(errno));
        return 1;
    }
    printf("%s\n", buf);
    universal_add_karma(1, "Recognized identity (hostname)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * date
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_date(int argc, char** argv) {
    const char* fmt = "%a %b %d %H:%M:%S %Z %Y";
    if (argc > 1) {
        fmt = argv[1];
    }

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    if (!tm_info) {
        fprintf(stderr, "date: %s\n", strerror(errno));
        return 1;
    }

    char buffer[256];
    if (strftime(buffer, sizeof(buffer), fmt, tm_info) == 0) {
        fprintf(stderr, "date: format too long or invalid\n");
        return 1;
    }

    printf("%s\n", buffer);
    universal_add_karma(1, "Noted the present moment (date)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * uptime
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_uptime(int argc, char** argv) {
    (void)argc;
    (void)argv;

#ifdef __linux__
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        fprintf(stderr, "uptime: %s\n", strerror(errno));
        return 1;
    }

    long days = info.uptime / 86400;
    long hours = (info.uptime % 86400) / 3600;
    long minutes = (info.uptime % 3600) / 60;

    printf("uptime: ");
    if (days > 0) {
        printf("%ld day%s, ", days, days == 1 ? "" : "s");
    }
    printf("%ld:%02ld up", hours, minutes);

    if (info.loads[0] || info.loads[1] || info.loads[2]) {
        double load1 = info.loads[0] / 65536.0;
        double load5 = info.loads[1] / 65536.0;
        double load15 = info.loads[2] / 65536.0;
        printf(", load average: %.2f %.2f %.2f", load1, load5, load15);
    }
    printf("\n");
    universal_add_karma(1, "Measured uptime (uptime)");
    return 0;
#else
    printf("uptime: not supported on this platform\n");
    return 1;
#endif
}

/* ═══════════════════════════════════════════════════════════════════════════
 * whoami
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_whoami(int argc, char** argv) {
    (void)argc;
    (void)argv;

    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (!pw) {
        fprintf(stderr, "whoami: cannot determine user (uid=%d)\n", (int)uid);
        return 1;
    }
    printf("%s\n", pw->pw_name);
    universal_add_karma(1, "Affirmed self (whoami)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * id
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_group_list(gid_t* groups, int count) {
    for (int i = 0; i < count; i++) {
        struct group* gr = getgrgid(groups[i]);
        if (gr) {
            printf("%s", gr->gr_name);
        } else {
            printf("%d", groups[i]);
        }
        if (i != count - 1) {
            putchar(',');
        }
    }
}

static int cmd_id(int argc, char** argv) {
    (void)argc;
    (void)argv;

    uid_t uid = geteuid();
    gid_t gid = getegid();

    struct passwd* pw = getpwuid(uid);
    struct group* gr = getgrgid(gid);

    printf("uid=%d(%s) gid=%d(%s)", (int)uid, pw ? pw->pw_name : "?", (int)gid, gr ? gr->gr_name : "?");

    gid_t groups[128];
    int ngroups = getgroups(128, groups);
    if (ngroups >= 0) {
        printf(" groups=");
        print_group_list(groups, ngroups);
    }
    printf("\n");

    universal_add_karma(1, "Understood identity (id)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * groups
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_groups(int argc, char** argv) {
    (void)argc;
    (void)argv;

    gid_t groups[128];
    int ngroups = getgroups(128, groups);
    if (ngroups < 0) {
        fprintf(stderr, "groups: %s\n", strerror(errno));
        return 1;
    }

    print_group_list(groups, ngroups);
    printf("\n");
    universal_add_karma(1, "Acknowledged community (groups)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * cal
 * ═══════════════════════════════════════════════════════════════════════════ */

static int days_in_month(int month, int year) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2) {
        bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        return leap ? 29 : 28;
    }
    return days[month - 1];
}

static int first_weekday_of_month(int month, int year) {
    struct tm tm_info = {0};
    tm_info.tm_year = year - 1900;
    tm_info.tm_mon = month - 1;
    tm_info.tm_mday = 1;
    if (mktime(&tm_info) == (time_t)-1) {
        return -1;
    }
    return tm_info.tm_wday; /* 0 = Sunday */
}

static int cmd_cal(int argc, char** argv) {
    time_t now = time(NULL);
    struct tm* tm_now = localtime(&now);
    if (!tm_now) {
        fprintf(stderr, "cal: %s\n", strerror(errno));
        return 1;
    }

    int month = tm_now->tm_mon + 1;
    int year = tm_now->tm_year + 1900;

    if (argc == 2) {
        year = atoi(argv[1]);
    } else if (argc >= 3) {
        month = atoi(argv[1]);
        year = atoi(argv[2]);
    }

    if (month < 1 || month > 12) {
        fprintf(stderr, "cal: invalid month '%d'\n", month);
        return 1;
    }
    if (year < 1) {
        fprintf(stderr, "cal: invalid year '%d'\n", year);
        return 1;
    }

    static const char* month_names[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    int first = first_weekday_of_month(month, year);
    if (first < 0) {
        fprintf(stderr, "cal: unable to compute calendar\n");
        return 1;
    }

    int days = days_in_month(month, year);

    printf("     %s %d\n", month_names[month - 1], year);
    printf("Su Mo Tu We Th Fr Sa\n");

    int current_col = 0;
    for (int i = 0; i < first; i++) {
        printf("   ");
        current_col++;
    }
    for (int day = 1; day <= days; day++) {
        printf("%2d", day);
        current_col++;
        if (current_col % 7 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
    if (current_col % 7 != 0) {
        printf("\n");
    }

    universal_add_karma(1, "Aligned time (cal)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 10 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week10_commands(void) {
    universal_shell_register_command(
        "uname", cmd_uname,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "print system information",
        "uname [-a]\n"
        "Show kernel and system information.");

    universal_shell_register_command(
        "hostname", cmd_hostname,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "display system hostname",
        "hostname\n"
        "Show the current system hostname.");

    universal_shell_register_command(
        "date", cmd_date,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "display current date and time",
        "date [format]\n"
        "Show date/time using optional strftime format.");

    universal_shell_register_command(
        "uptime", cmd_uptime,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "show how long the system has been running",
        "uptime\n"
        "Display system uptime and load averages.");

    universal_shell_register_command(
        "whoami", cmd_whoami,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "print effective user name",
        "whoami\n"
        "Display effective user name.");

    universal_shell_register_command(
        "id", cmd_id,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "print user and group information",
        "id\n"
        "Display user, group and supplementary group IDs.");

    universal_shell_register_command(
        "groups", cmd_groups,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "print group memberships",
        "groups\n"
        "List supplementary groups for the current user.");

    universal_shell_register_command(
        "cal", cmd_cal,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "display a calendar",
        "cal [month] [year]\n"
        "Show a monthly calendar (defaults to current month).");
}
