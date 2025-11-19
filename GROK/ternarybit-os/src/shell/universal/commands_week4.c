/**
 * @file commands_week4.c
 * @brief Week 4 Process Management Commands (Days 26-33)
 *
 * GO MODE: Full speed ahead!
 * Week 4: Process Control & Job Management
 *
 * Commands 26-33:
 * - Day 26: ps      (process status)
 * - Day 27: top     (dynamic process viewer)
 * - Day 28: kill    (send signals to processes)
 * - Day 29: bg      (background jobs)
 * - Day 30: fg      (foreground jobs)
 * - Day 31: jobs    (list jobs)
 * - Day 32: nice    (run with priority)
 * - Day 33: renice  (change priority)
 *
 * @date 2025-11-05
 * @author TernaryBit OS Team (GO SPEED MODE)
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <dirent.h>
#include <ctype.h>

#ifdef __linux__
#include <sys/resource.h>
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * JOB MANAGEMENT STRUCTURES
 * ═══════════════════════════════════════════════════════════════════════════ */

#define MAX_JOBS 100

typedef enum {
    JOB_RUNNING,
    JOB_STOPPED,
    JOB_DONE
} job_state_t;

typedef struct {
    int job_id;
    pid_t pid;
    job_state_t state;
    char command[256];
    bool in_use;
} job_t;

static job_t g_jobs[MAX_JOBS];
static int g_next_job_id = 1;

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void init_jobs(void) {
    static bool initialized = false;
    if (!initialized) {
        memset(g_jobs, 0, sizeof(g_jobs));
        initialized = true;
    }
}

static int add_job(pid_t pid, const char* command, job_state_t state) {
    init_jobs();

    for (int i = 0; i < MAX_JOBS; i++) {
        if (!g_jobs[i].in_use) {
            g_jobs[i].job_id = g_next_job_id++;
            g_jobs[i].pid = pid;
            g_jobs[i].state = state;
            g_jobs[i].in_use = true;
            strncpy(g_jobs[i].command, command, sizeof(g_jobs[i].command) - 1);
            return g_jobs[i].job_id;
        }
    }
    return -1;
}

static job_t* find_job(int job_id) {
    init_jobs();
    for (int i = 0; i < MAX_JOBS; i++) {
        if (g_jobs[i].in_use && g_jobs[i].job_id == job_id) {
            return &g_jobs[i];
        }
    }
    return NULL;
}

static void remove_job(int job_id) {
    job_t* job = find_job(job_id);
    if (job) {
        job->in_use = false;
    }
}

static void format_time(unsigned long seconds, char* buf, size_t bufsize) {
    unsigned long hours = seconds / 3600;
    unsigned long mins = (seconds % 3600) / 60;
    unsigned long secs = seconds % 60;

    if (hours > 0) {
        snprintf(buf, bufsize, "%lu:%02lu:%02lu", hours, mins, secs);
    } else {
        snprintf(buf, bufsize, "%lu:%02lu", mins, secs);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 26: ps - Process Status
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_ps(int argc, char** argv) {
    bool show_all = false;
    bool full_format = false;

    /* Parse options */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-e") == 0) {
            show_all = true;
        } else if (strcmp(argv[i], "-f") == 0) {
            full_format = true;
        } else if (strcmp(argv[i], "aux") == 0) {
            show_all = true;
            full_format = true;
        }
    }

#ifdef __linux__
    /* Read from /proc on Linux */
    DIR* proc = opendir("/proc");
    if (!proc) {
        fprintf(stderr, "ps: cannot open /proc: %s\n", strerror(errno));
        return 1;
    }

    printf("  PID TTY          TIME CMD\n");

    struct dirent* entry;
    while ((entry = readdir(proc)) != NULL) {
        /* Check if directory name is a number (PID) */
        if (!isdigit(entry->d_name[0])) continue;

        pid_t pid = atoi(entry->d_name);

        /* Read process info */
        char stat_path[256];
        snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

        FILE* stat_file = fopen(stat_path, "r");
        if (!stat_file) continue;

        int pid_read;
        char comm[256];
        char state;
        unsigned long utime, stime;

        if (fscanf(stat_file, "%d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu",
                   &pid_read, comm, &state, &utime, &stime) < 5) {
            fclose(stat_file);
            continue;
        }
        fclose(stat_file);

        /* Remove parentheses from command name */
        if (comm[0] == '(') {
            size_t len = strlen(comm);
            if (len > 2 && comm[len-1] == ')') {
                comm[len-1] = '\0';
                memmove(comm, comm + 1, len - 1);
            }
        }

        /* Calculate CPU time */
        unsigned long total_time = (utime + stime) / sysconf(_SC_CLK_TCK);
        char time_str[32];
        format_time(total_time, time_str, sizeof(time_str));

        /* Show process */
        if (show_all || pid == getpid()) {
            printf("%5d %-12s %8s %s\n", pid, "?", time_str, comm);
        }
    }

    closedir(proc);
#else
    /* Fallback: just show current process */
    printf("  PID TTY          TIME CMD\n");
    printf("%5d %-12s %8s %s\n", getpid(), "?", "0:00", "tbos_shell");
#endif

    universal_add_karma(1, "Observed processes mindfully");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 27: top - Dynamic Process Viewer
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_top(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("TernaryBit OS - Process Monitor\n");
    printf("(Simplified version - press Ctrl+C to exit)\n\n");

#ifdef __linux__
    /* Get system info */
    long uptime = 0;
    FILE* uptime_file = fopen("/proc/uptime", "r");
    if (uptime_file) {
        double uptime_secs;
        if (fscanf(uptime_file, "%lf", &uptime_secs) == 1) {
            uptime = (long)uptime_secs;
        }
        fclose(uptime_file);
    }

    printf("System uptime: %ld seconds\n\n", uptime);
    printf("  PID USER      PR  NI    VIRT    RES  S  %%CPU %%MEM     TIME+ COMMAND\n");

    /* Read top processes */
    DIR* proc = opendir("/proc");
    if (!proc) {
        fprintf(stderr, "top: cannot open /proc\n");
        return 1;
    }

    struct dirent* entry;
    int count = 0;
    while ((entry = readdir(proc)) != NULL && count < 20) {
        if (!isdigit(entry->d_name[0])) continue;

        pid_t pid = atoi(entry->d_name);

        char stat_path[256];
        snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);

        FILE* stat_file = fopen(stat_path, "r");
        if (!stat_file) continue;

        int pid_read;
        char comm[256];
        char state;
        unsigned long vsize, rss;

        if (fscanf(stat_file, "%d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %*u %lu %lu",
                   &pid_read, comm, &state, &vsize, &rss) < 5) {
            fclose(stat_file);
            continue;
        }
        fclose(stat_file);

        /* Remove parentheses */
        if (comm[0] == '(') {
            size_t len = strlen(comm);
            if (len > 2 && comm[len-1] == ')') {
                comm[len-1] = '\0';
                memmove(comm, comm + 1, len - 1);
            }
        }

        printf("%5d %-8s %3d %3d %7lu %6lu %c %5.1f %4.1f %9s %s\n",
               pid, "user", 20, 0, vsize / 1024, rss * 4, state,
               0.0, 0.0, "0:00.00", comm);

        count++;
    }

    closedir(proc);
#else
    printf("  PID COMMAND\n");
    printf("%5d %s\n", getpid(), "tbos_shell");
#endif

    universal_add_karma(2, "Monitored system consciousness");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 28: kill - Send Signals to Processes
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_kill(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "kill: usage: kill [-SIGNAL] PID...\n");
        return 1;
    }

    int sig = SIGTERM;  /* Default signal */
    int first_pid_arg = 1;

    /* Parse signal option */
    if (argv[1][0] == '-') {
        if (isdigit(argv[1][1])) {
            sig = atoi(argv[1] + 1);
        } else {
            /* Named signals */
            if (strcmp(argv[1], "-TERM") == 0) sig = SIGTERM;
            else if (strcmp(argv[1], "-KILL") == 0) sig = SIGKILL;
            else if (strcmp(argv[1], "-INT") == 0) sig = SIGINT;
            else if (strcmp(argv[1], "-HUP") == 0) sig = SIGHUP;
            else if (strcmp(argv[1], "-STOP") == 0) sig = SIGSTOP;
            else if (strcmp(argv[1], "-CONT") == 0) sig = SIGCONT;
            else {
                fprintf(stderr, "kill: unknown signal: %s\n", argv[1]);
                return 1;
            }
        }
        first_pid_arg = 2;
    }

    if (first_pid_arg >= argc) {
        fprintf(stderr, "kill: no process ID specified\n");
        return 1;
    }

    /* Send signal to each PID */
    for (int i = first_pid_arg; i < argc; i++) {
        pid_t pid = atoi(argv[i]);

        if (pid <= 0) {
            fprintf(stderr, "kill: invalid PID: %s\n", argv[i]);
            continue;
        }

        if (kill(pid, sig) != 0) {
            fprintf(stderr, "kill: (%d) - %s\n", pid, strerror(errno));
            return 1;
        }
    }

    universal_add_karma(1, "Sent signal (mindful termination)");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 29: bg - Background Jobs
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_bg(int argc, char** argv) {
    init_jobs();

    if (argc < 2) {
        fprintf(stderr, "bg: usage: bg [job_id]\n");
        return 1;
    }

    int job_id = atoi(argv[1]);
    job_t* job = find_job(job_id);

    if (!job) {
        fprintf(stderr, "bg: no such job: %d\n", job_id);
        return 1;
    }

    /* Send SIGCONT to resume stopped process */
    if (kill(job->pid, SIGCONT) != 0) {
        fprintf(stderr, "bg: failed to continue job %d: %s\n", job_id, strerror(errno));
        return 1;
    }

    job->state = JOB_RUNNING;
    printf("[%d] %s &\n", job->job_id, job->command);

    universal_add_karma(1, "Resumed background work");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 30: fg - Foreground Jobs
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_fg(int argc, char** argv) {
    init_jobs();

    if (argc < 2) {
        fprintf(stderr, "fg: usage: fg [job_id]\n");
        return 1;
    }

    int job_id = atoi(argv[1]);
    job_t* job = find_job(job_id);

    if (!job) {
        fprintf(stderr, "fg: no such job: %d\n", job_id);
        return 1;
    }

    printf("[%d] %s\n", job->job_id, job->command);

    /* Send SIGCONT if stopped */
    if (job->state == JOB_STOPPED) {
        kill(job->pid, SIGCONT);
    }

    job->state = JOB_RUNNING;

    /* Wait for job to complete */
    int status;
    if (waitpid(job->pid, &status, 0) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            remove_job(job_id);
        }
    }

    universal_add_karma(1, "Brought work to foreground");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 31: jobs - List Jobs
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_jobs(int argc, char** argv) {
    (void)argc;
    (void)argv;

    init_jobs();

    bool found_any = false;
    for (int i = 0; i < MAX_JOBS; i++) {
        if (g_jobs[i].in_use) {
            found_any = true;

            const char* state_str;
            switch (g_jobs[i].state) {
                case JOB_RUNNING: state_str = "Running"; break;
                case JOB_STOPPED: state_str = "Stopped"; break;
                case JOB_DONE:    state_str = "Done"; break;
                default:          state_str = "Unknown"; break;
            }

            printf("[%d]  %s\t\t%s\n", g_jobs[i].job_id, state_str, g_jobs[i].command);
        }
    }

    if (!found_any) {
        printf("(no jobs)\n");
    }

    universal_add_karma(1, "Observed work in progress");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 32: nice - Run with Priority
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_nice(int argc, char** argv) {
    if (argc < 2) {
        /* Show current niceness */
        int niceness = getpriority(PRIO_PROCESS, 0);
        printf("%d\n", niceness);
        return 0;
    }

    int niceness = 10;  /* Default nice value */
    int first_cmd_arg = 1;

    /* Parse -n option */
    if (strcmp(argv[1], "-n") == 0) {
        if (argc < 4) {
            fprintf(stderr, "nice: usage: nice [-n adjustment] command [args]\n");
            return 1;
        }
        niceness = atoi(argv[2]);
        first_cmd_arg = 3;
    }

    /* Set priority and execute command */
    if (setpriority(PRIO_PROCESS, 0, niceness) != 0) {
        fprintf(stderr, "nice: cannot set priority: %s\n", strerror(errno));
        return 1;
    }

    /* In a real shell, we would fork and exec the command here */
    printf("nice: would execute '%s' with niceness %d\n", argv[first_cmd_arg], niceness);
    printf("(Full execution not implemented in minimal shell)\n");

    universal_add_karma(2, "Set mindful priority");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * DAY 33: renice - Change Priority
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_renice(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "renice: usage: renice priority pid...\n");
        return 1;
    }

    int priority = atoi(argv[1]);

    /* Change priority for each PID */
    for (int i = 2; i < argc; i++) {
        pid_t pid = atoi(argv[i]);

        if (pid <= 0) {
            fprintf(stderr, "renice: invalid PID: %s\n", argv[i]);
            continue;
        }

        if (setpriority(PRIO_PROCESS, pid, priority) != 0) {
            fprintf(stderr, "renice: failed for PID %d: %s\n", pid, strerror(errno));
            return 1;
        }

        printf("%d: old priority 0, new priority %d\n", pid, priority);
    }

    universal_add_karma(1, "Adjusted priorities mindfully");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 4 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week4_commands(void) {
    universal_shell_register_command("ps", cmd_ps,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "report process status",
        "ps [OPTIONS]\n\n"
        "Show process information.\n\n"
        "Options:\n"
        "  -a, -e    show all processes\n"
        "  -f        full format\n"
        "  aux       BSD-style (all + full)");

    universal_shell_register_command("top", cmd_top,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "dynamic process viewer",
        "top\n\n"
        "Display system processes dynamically.\n"
        "Press Ctrl+C to exit.");

    universal_shell_register_command("kill", cmd_kill,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "send signal to process",
        "kill [-SIGNAL] PID...\n\n"
        "Send signal to process(es).\n\n"
        "Signals:\n"
        "  -TERM    terminate (default)\n"
        "  -KILL    force kill\n"
        "  -INT     interrupt\n"
        "  -STOP    stop process\n"
        "  -CONT    continue process");

    universal_shell_register_command("bg", cmd_bg,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "background jobs",
        "bg [job_id]\n\n"
        "Resume stopped job in background.");

    universal_shell_register_command("fg", cmd_fg,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "foreground jobs",
        "fg [job_id]\n\n"
        "Bring job to foreground.");

    universal_shell_register_command("jobs", cmd_jobs,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "list jobs",
        "jobs\n\n"
        "List active jobs.");

    universal_shell_register_command("nice", cmd_nice,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "run with modified priority",
        "nice [-n ADJUSTMENT] COMMAND [ARGS]\n\n"
        "Run command with adjusted priority.\n"
        "Default adjustment: 10\n"
        "Range: -20 (high) to 19 (low)");

    universal_shell_register_command("renice", cmd_renice,
        CMD_CAT_PROCESS_MGMT, OS_SUPPORT_ALL,
        "alter priority of running process",
        "renice PRIORITY PID...\n\n"
        "Change priority of running process(es).\n"
        "Range: -20 (high) to 19 (low)");

    printf("🚀 [Week 4] Registered 8 process management commands\n");
    printf("   Commands 26-33: ps, top, kill, bg, fg, jobs, nice, renice\n");
}
