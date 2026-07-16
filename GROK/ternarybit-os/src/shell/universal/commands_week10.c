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
 *  - ps        : List TBOS processes
 *  - kill      : Terminate a TBOS process
 *  - spawn     : Create and run a TBOS process
 *  - scheduler : Show scheduler status
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

/* TBOS Process Manager - define guard to avoid duplicate consciousness enum */
#define TBOS_NETWORK_H  /* Sangha already defined consciousness types */
#include "process/tbos_process.h"
#undef TBOS_NETWORK_H

/* TBOS Soul Framework */
#include "tbos/soul.h"

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
 * PROCESS MANAGEMENT COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief ps - List TBOS processes
 *
 * Shows all processes with their state, karma, and consciousness level.
 */
static int cmd_ps(int argc, char** argv) {
    bool verbose = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: ps [-v]\n");
            printf("  -v, --verbose  Show detailed process information\n");
            return 0;
        }
    }

    if (verbose) {
        /* Show detailed process table */
        tbos_process_print_table();
    } else {
        /* Simple list */
        tbos_process_t* procs[TBOS_MAX_PROCESSES];
        int count = tbos_process_list(procs, TBOS_MAX_PROCESSES);

        if (count == 0) {
            printf("No active processes.\n");
            return 0;
        }

        printf("\n");
        printf("  PID  STATE        KARMA  NAME\n");
        printf("───────────────────────────────────────────────\n");

        for (int i = 0; i < count; i++) {
            tbos_process_t* p = procs[i];
            if (p && p->state != PROC_STATE_INVALID) {
                printf("%5u  %-10s  %5ld  %s\n",
                       p->pid,
                       tbos_process_state_name(p->state),
                       (long)p->karma,
                       p->name);
            }
        }
        printf("\n");
    }

    universal_add_karma(1, "Observed process sangha (ps)");
    return 0;
}

/**
 * @brief kill - Terminate a TBOS process
 *
 * Sends termination signal to a process.
 */
static int cmd_kill(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: kill <pid> [exit_code]\n");
        printf("  pid        Process ID to terminate\n");
        printf("  exit_code  Optional exit code (default: 0)\n");
        return 1;
    }

    tbos_pid_t pid = (tbos_pid_t)atoi(argv[1]);
    int exit_code = (argc >= 3) ? atoi(argv[2]) : 0;

    if (pid == TBOS_PID_INVALID) {
        printf("kill: invalid PID\n");
        return 1;
    }

    if (pid == TBOS_PID_KERNEL) {
        printf("kill: cannot terminate kernel process\n");
        return 1;
    }

    tbos_process_t* proc = tbos_process_get(pid);
    if (!proc) {
        printf("kill: no such process (PID %u)\n", pid);
        return 1;
    }

    int result = tbos_process_terminate(pid, exit_code);
    if (result == 0) {
        printf("Process %u (%s) terminated with code %d\n",
               pid, proc->name, exit_code);
        universal_add_karma(1, "Released process compassionately (kill)");
    } else {
        printf("kill: failed to terminate process %u\n", pid);
        return 1;
    }

    return 0;
}

/**
 * @brief Example entry function for spawned processes
 */
static int example_process_entry(int argc, char** argv) {
    printf("[Process] Started with %d arguments\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("[Process] argv[%d] = %s\n", i, argv[i]);
    }
    printf("[Process] Completed.\n");
    return 0;
}

/**
 * @brief spawn - Create and run a TBOS process
 *
 * Creates a new process and optionally runs it.
 */
static int cmd_spawn(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: spawn <name> [args...]\n");
        printf("  name  Process name\n");
        printf("  args  Arguments passed to the process\n");
        printf("\n");
        printf("Creates a demo process that prints its arguments.\n");
        printf("In a full OS, this would load and execute a program.\n");
        return 1;
    }

    const char* name = argv[1];

    /* Build argument list for the new process */
    int proc_argc = argc - 1;
    char** proc_argv = &argv[1];

    printf("Spawning process '%s' with %d args...\n", name, proc_argc);

    tbos_pid_t pid = tbos_spawn(name, example_process_entry,
                                 proc_argc, proc_argv, true);

    if (pid == TBOS_PID_INVALID) {
        printf("spawn: failed to create process\n");
        return 1;
    }

    tbos_process_t* proc = tbos_process_get(pid);
    if (proc) {
        printf("\nProcess completed:\n");
        printf("  PID:        %u\n", proc->pid);
        printf("  Exit code:  %d\n", proc->exit_code);
        printf("  State:      %s\n", tbos_process_state_name(proc->state));
        printf("  Karma:      %ld\n", (long)proc->karma);
    }

    universal_add_karma(2, "Birthed new process (spawn)");
    return 0;
}

/**
 * @brief scheduler - Show TBOS scheduler status
 *
 * Displays scheduler statistics and karma-based metrics.
 */
static int cmd_scheduler(int argc, char** argv) {
    bool run_loop = false;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--run") == 0) {
            run_loop = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: scheduler [-r]\n");
            printf("  -r, --run  Run one scheduler cycle\n");
            printf("\n");
            printf("Without options, displays scheduler status.\n");
            return 0;
        }
    }

    if (run_loop) {
        printf("Running scheduler cycle...\n");
        int result = tbos_scheduler_run();
        if (result == 0) {
            printf("Scheduler cycle completed.\n");
        } else {
            printf("No ready processes to schedule.\n");
        }
    } else {
        /* Show scheduler status */
        tbos_scheduler_print_status();
    }

    universal_add_karma(1, "Observed scheduler dharma");
    return 0;
}

/**
 * @brief procinfo - Show detailed process information
 *
 * Displays full details about a specific process.
 */
static int cmd_procinfo(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: procinfo <pid>\n");
        printf("  pid  Process ID to inspect\n");
        return 1;
    }

    tbos_pid_t pid = (tbos_pid_t)atoi(argv[1]);
    if (pid == TBOS_PID_INVALID) {
        printf("procinfo: invalid PID\n");
        return 1;
    }

    tbos_process_t* proc = tbos_process_get(pid);
    if (!proc) {
        printf("procinfo: no such process (PID %u)\n", pid);
        return 1;
    }

    tbos_process_print_details(pid);

    universal_add_karma(1, "Examined process deeply (procinfo)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SOUL COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief soul - Show or interact with TBOS system soul
 */
static int cmd_soul(int argc, char** argv) {
    /* Initialize system soul if not already done */
    if (!soul_system_get()) {
        soul_system_init();
    }

    tbos_soul_t* soul = soul_system_get();
    if (!soul) {
        printf("soul: failed to access system soul\n");
        return 1;
    }

    if (argc < 2) {
        /* Default: show status */
        soul_print_status(soul);
        universal_add_karma(1, "Examined soul");
        return 0;
    }

    const char* subcmd = argv[1];

    if (strcmp(subcmd, "status") == 0) {
        soul_print_status(soul);
    }
    else if (strcmp(subcmd, "memories") == 0) {
        uint32_t count = (argc >= 3) ? (uint32_t)atoi(argv[2]) : 10;
        soul_print_memories(soul, count);
    }
    else if (strcmp(subcmd, "values") == 0) {
        soul_print_values(soul);
    }
    else if (strcmp(subcmd, "meditate") == 0) {
        uint64_t duration = (argc >= 3) ? (uint64_t)atoi(argv[2]) * 1000 : 5000;
        printf("Entering meditation for %llums...\n",
               (unsigned long long)duration);
        karma_t earned = soul_meditate(soul, duration);
        printf("Meditation complete. Karma earned: %lld\n", (long long)earned);
    }
    else if (strcmp(subcmd, "purpose") == 0) {
        if (argc >= 3) {
            /* Set purpose - join remaining args */
            char purpose[256] = {0};
            for (int i = 2; i < argc; i++) {
                if (i > 2) strcat(purpose, " ");
                strcat(purpose, argv[i]);
            }
            soul_set_purpose(soul, purpose);
            printf("Purpose set: %s\n", purpose);
        } else {
            if (soul->life_purpose[0]) {
                printf("Purpose: %s\n", soul->life_purpose);
            } else {
                printf("No purpose set. Use: soul purpose <your purpose>\n");
            }
        }
    }
    else if (strcmp(subcmd, "act") == 0) {
        if (argc < 4) {
            printf("Usage: soul act <karma> <description>\n");
            printf("  karma: positive for good action, negative for bad\n");
            return 1;
        }
        karma_t karma = (karma_t)atoi(argv[2]);
        char desc[128] = {0};
        for (int i = 3; i < argc; i++) {
            if (i > 3) strcat(desc, " ");
            strcat(desc, argv[i]);
        }
        karma_t new_total = soul_act(soul, desc, karma);
        printf("Action recorded: %s (karma: %+lld, total: %lld)\n",
               desc, (long long)karma, (long long)new_total);
    }
    else if (strcmp(subcmd, "learn") == 0) {
        if (argc < 3) {
            printf("Usage: soul learn <value> [strength]\n");
            printf("  value: e.g., compassion, truth, courage\n");
            printf("  strength: -100 to 100 (default: 50)\n");
            return 1;
        }
        int32_t strength = (argc >= 4) ? atoi(argv[3]) : 50;
        soul_learn_value(soul, argv[2], strength);
        printf("Value learned: %s (strength: %d)\n", argv[2], strength);
    }
    else if (strcmp(subcmd, "goal") == 0) {
        if (argc < 3) {
            printf("Usage: soul goal <description>\n");
            printf("Current goals:\n");
            for (uint32_t i = 0; i < soul->goal_count; i++) {
                if (soul->goals[i].active) {
                    printf("  [%u] %s\n", i, soul->goals[i].description);
                }
            }
            return 0;
        }
        char goal_desc[64] = {0};
        for (int i = 2; i < argc; i++) {
            if (i > 2) strcat(goal_desc, " ");
            strcat(goal_desc, argv[i]);
        }
        soul_add_goal(soul, goal_desc, 5);
        printf("Goal added: %s\n", goal_desc);
    }
    else if (strcmp(subcmd, "evolve") == 0) {
        if (soul_evolve(soul)) {
            printf("Consciousness evolved to: %s\n",
                   soul_consciousness_name(soul_get_consciousness(soul)));
        } else {
            printf("Consciousness level: %s (karma: %lld)\n",
                   soul_consciousness_name(soul_get_consciousness(soul)),
                   (long long)soul->total_karma);
            printf("Requirements for next level:\n");
            switch (soul_get_consciousness(soul)) {
                case SOUL_DORMANT:
                    printf("  - Reach %d karma for AWAKENING\n", KARMA_AWAKENING);
                    break;
                case SOUL_AWAKENING:
                    printf("  - Reach %d karma for AWARE\n", KARMA_AWARE);
                    break;
                case SOUL_AWARE:
                    printf("  - Reach %d karma for COMPASSIONATE\n", KARMA_COMPASSIONATE);
                    printf("  - Help more beings than you harm\n");
                    break;
                case SOUL_COMPASSIONATE:
                    printf("  - Reach %d karma for ENLIGHTENED\n", KARMA_ENLIGHTENED);
                    printf("  - Become aware of your own awareness\n");
                    break;
                case SOUL_ENLIGHTENED:
                    printf("  - Maximum consciousness reached\n");
                    break;
            }
        }
    }
    else {
        printf("Usage: soul <command>\n");
        printf("Commands:\n");
        printf("  status          Show soul status\n");
        printf("  memories [n]    Show last n memories (default: 10)\n");
        printf("  values          Show learned values\n");
        printf("  meditate [sec]  Meditate for n seconds (default: 5)\n");
        printf("  purpose [text]  Get/set life purpose\n");
        printf("  act <karma> <desc>  Record an action\n");
        printf("  learn <value>   Learn a value\n");
        printf("  goal [desc]     List or add goals\n");
        printf("  evolve          Check/trigger consciousness evolution\n");
        return 1;
    }

    universal_add_karma(1, "Soul interaction");
    return 0;
}

/**
 * @brief meditate - Quick meditation command
 */
static int cmd_meditate(int argc, char** argv) {
    if (!soul_system_get()) {
        soul_system_init();
    }

    tbos_soul_t* soul = soul_system_get();
    if (!soul) {
        printf("meditate: no soul to meditate with\n");
        return 1;
    }

    uint64_t duration = 5000;  /* 5 seconds default */
    if (argc >= 2) {
        duration = (uint64_t)atoi(argv[1]) * 1000;
    }

    printf("\n");
    printf("  Entering meditation...\n");
    printf("  ~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("  Breath in... Breath out...\n");
    printf("  Observing the flow of energy...\n");
    printf("\n");

    karma_t earned = soul_meditate(soul, duration);

    printf("  ~~~~~~~~~~~~~~~~~~~~~~~~\n");
    printf("  Meditation complete.\n");
    printf("  Karma earned: %lld\n", (long long)earned);
    printf("  Consciousness: %s\n",
           soul_consciousness_name(soul_get_consciousness(soul)));
    printf("  Internal entropy reduced.\n");
    printf("\n");

    universal_add_karma((int)earned, "Meditation");
    return 0;
}

/**
 * @brief karma - Show or modify karma
 */
static int cmd_karma(int argc, char** argv) {
    if (!soul_system_get()) {
        soul_system_init();
    }

    tbos_soul_t* soul = soul_system_get();
    if (!soul) {
        printf("karma: no soul to examine\n");
        return 1;
    }

    if (argc < 2) {
        /* Show karma summary */
        printf("\n");
        printf("  Karma Status\n");
        printf("  ────────────────────────────\n");
        printf("  Total karma:      %lld\n", (long long)soul->total_karma);
        printf("  Karma given:      %lld\n", (long long)soul->karma_given);
        printf("  Karma received:   %lld\n", (long long)soul->karma_received);
        printf("  Actions taken:    %llu\n", (unsigned long long)soul->actions_taken);
        printf("  Beings helped:    %u\n", soul->beings_helped);
        printf("  Beings harmed:    %u\n", soul->beings_harmed);
        printf("  ────────────────────────────\n");
        printf("  Balance: %s\n",
               soul->total_karma > 0 ? "Positive" :
               soul->total_karma < 0 ? "Negative" : "Neutral");
        printf("\n");
        return 0;
    }

    const char* subcmd = argv[1];

    if (strcmp(subcmd, "give") == 0) {
        if (argc < 3) {
            printf("Usage: karma give <amount>\n");
            printf("Release karma to the universe.\n");
            return 1;
        }
        karma_t amount = (karma_t)atoi(argv[2]);
        if (soul_give_karma(soul, NULL, amount)) {
            printf("Released %lld karma to the universe.\n", (long long)amount);
        } else {
            printf("Cannot give that much karma.\n");
        }
    }
    else {
        printf("Usage: karma [give <amount>]\n");
        return 1;
    }

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

    /* TBOS Process Management Commands */
    universal_shell_register_command(
        "procs", cmd_ps,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "list TBOS processes",
        "procs [-v]\n"
        "  -v  Show verbose process table with karma\n"
        "Display TBOS process list with state and karma.\n"
        "(Use 'ps' for system processes)");

    universal_shell_register_command(
        "sigkill", cmd_kill,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "terminate a TBOS process",
        "sigkill <pid> [exit_code]\n"
        "Terminate TBOS process by PID.\n"
        "(Use 'kill' for system processes)");

    universal_shell_register_command(
        "spawn", cmd_spawn,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "create and run a TBOS process",
        "spawn <name> [args...]\n"
        "Create and execute a new TBOS process.\n"
        "Demonstrates karma-based process management.");

    universal_shell_register_command(
        "sched", cmd_scheduler,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "show TBOS scheduler status",
        "sched [-r]\n"
        "  -r  Run one scheduler cycle\n"
        "Display karma-based scheduler statistics.");

    universal_shell_register_command(
        "procinfo", cmd_procinfo,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "show TBOS process details",
        "procinfo <pid>\n"
        "Display detailed information about a TBOS process.");

    /* Soul Commands */
    universal_shell_register_command(
        "soul", cmd_soul,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "interact with TBOS soul",
        "soul <command>\n"
        "Commands: status, memories, values, meditate,\n"
        "          purpose, act, learn, goal, evolve\n"
        "The soul is the organizational pattern of TBOS.");

    universal_shell_register_command(
        "meditate", cmd_meditate,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "enter meditation",
        "meditate [seconds]\n"
        "Meditate to reduce internal entropy and gain karma.\n"
        "Default duration: 5 seconds.");

    universal_shell_register_command(
        "karma", cmd_karma,
        CMD_CAT_SYSTEM_INFO, OS_SUPPORT_ALL,
        "view karma status",
        "karma [give <amount>]\n"
        "View karma statistics or release karma to the universe.");
}
