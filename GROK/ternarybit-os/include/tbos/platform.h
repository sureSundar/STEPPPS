/**
 * platform.h - TBOS Cross-Platform Compatibility Layer
 *
 * Supports: macOS, Linux, Windows, Android, iOS
 *
 * Usage:
 *   #include "tbos/platform.h"
 *   tbos_mkdir("path");
 *   tbos_getenv("HOME");
 *   tbos_sleep_ms(100);
 */

#ifndef TBOS_PLATFORM_H
#define TBOS_PLATFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

/* ========================================================================= */
/* PLATFORM DETECTION                                                         */
/* ========================================================================= */

#if defined(_WIN32) || defined(_WIN64) || defined(__MINGW32__) || defined(__MINGW64__)
    #define TBOS_PLATFORM_WINDOWS 1
    #define TBOS_PLATFORM_NAME "Windows"
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define TBOS_PLATFORM_IOS 1
        #define TBOS_PLATFORM_NAME "iOS"
    #else
        #define TBOS_PLATFORM_MACOS 1
        #define TBOS_PLATFORM_NAME "macOS"
    #endif
#elif defined(__ANDROID__)
    #define TBOS_PLATFORM_ANDROID 1
    #define TBOS_PLATFORM_NAME "Android"
#elif defined(__linux__)
    #define TBOS_PLATFORM_LINUX 1
    #define TBOS_PLATFORM_NAME "Linux"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
    #define TBOS_PLATFORM_BSD 1
    #define TBOS_PLATFORM_NAME "BSD"
#else
    #define TBOS_PLATFORM_UNKNOWN 1
    #define TBOS_PLATFORM_NAME "Unknown"
#endif

/* POSIX-like systems */
#if defined(TBOS_PLATFORM_MACOS) || defined(TBOS_PLATFORM_LINUX) || \
    defined(TBOS_PLATFORM_BSD) || defined(TBOS_PLATFORM_ANDROID)
    #define TBOS_PLATFORM_POSIX 1
#endif

/* ========================================================================= */
/* PLATFORM-SPECIFIC INCLUDES                                                 */
/* ========================================================================= */

#ifdef TBOS_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    #include <process.h>
    #define PATH_SEPARATOR '\\'
    #define PATH_SEPARATOR_STR "\\"
    #define LINE_ENDING "\r\n"
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <dirent.h>
    #include <pwd.h>
    #define PATH_SEPARATOR '/'
    #define PATH_SEPARATOR_STR "/"
    #define LINE_ENDING "\n"
#endif

#ifdef TBOS_PLATFORM_IOS
    /* iOS has limited system access */
    #define TBOS_SANDBOXED 1
#endif

/* ========================================================================= */
/* DIRECTORY OPERATIONS                                                       */
/* ========================================================================= */

static inline int tbos_mkdir(const char* path) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

static inline int tbos_rmdir(const char* path) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _rmdir(path);
#else
    return rmdir(path);
#endif
}

static inline int tbos_chdir(const char* path) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _chdir(path);
#else
    return chdir(path);
#endif
}

static inline char* tbos_getcwd(char* buf, size_t size) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _getcwd(buf, (int)size);
#else
    return getcwd(buf, size);
#endif
}

/* ========================================================================= */
/* FILE OPERATIONS                                                            */
/* ========================================================================= */

static inline int tbos_unlink(const char* path) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _unlink(path);
#else
    return unlink(path);
#endif
}

static inline int tbos_access(const char* path, int mode) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _access(path, mode);
#else
    return access(path, mode);
#endif
}

static inline bool tbos_file_exists(const char* path) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _access(path, 0) == 0;
#else
    return access(path, F_OK) == 0;
#endif
}

/* ========================================================================= */
/* ENVIRONMENT                                                                */
/* ========================================================================= */

static inline const char* tbos_getenv(const char* name) {
    return getenv(name);
}

static inline int tbos_setenv(const char* name, const char* value) {
#ifdef TBOS_PLATFORM_WINDOWS
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s=%s", name, value);
    return _putenv(buf);
#else
    return setenv(name, value, 1);
#endif
}

static inline const char* tbos_get_home(void) {
#ifdef TBOS_PLATFORM_WINDOWS
    const char* home = getenv("USERPROFILE");
    if (!home) home = getenv("HOMEDRIVE");
    return home ? home : "C:\\Users\\Default";
#elif defined(TBOS_PLATFORM_ANDROID)
    const char* home = getenv("HOME");
    return home ? home : "/data/local/tmp";
#elif defined(TBOS_PLATFORM_IOS)
    /* iOS apps have sandboxed home */
    const char* home = getenv("HOME");
    return home ? home : "/var/mobile";
#else
    const char* home = getenv("HOME");
    if (!home) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
    return home ? home : "/tmp";
#endif
}

static inline int tbos_gethostname(char* buf, size_t len) {
#ifdef TBOS_PLATFORM_WINDOWS
    DWORD size = (DWORD)len;
    if (GetComputerNameA(buf, &size)) return 0;
    strncpy(buf, "windows-pc", len);
    return 0;
#elif defined(TBOS_PLATFORM_IOS)
    strncpy(buf, "ios-device", len);
    return 0;
#else
    return gethostname(buf, len);
#endif
}

static inline const char* tbos_get_username(void) {
#ifdef TBOS_PLATFORM_WINDOWS
    const char* user = getenv("USERNAME");
    return user ? user : "user";
#else
    const char* user = getenv("USER");
    if (!user) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) user = pw->pw_name;
    }
    return user ? user : "user";
#endif
}

/* ========================================================================= */
/* PROCESS / EXECUTION                                                        */
/* ========================================================================= */

static inline int tbos_system(const char* cmd) {
#ifdef TBOS_SANDBOXED
    /* iOS: Cannot execute arbitrary commands */
    (void)cmd;
    fprintf(stderr, "system() not available on this platform\n");
    return -1;
#else
    return system(cmd);
#endif
}

static inline bool tbos_can_execute(void) {
#ifdef TBOS_SANDBOXED
    return false;
#else
    return true;
#endif
}

/* ========================================================================= */
/* TIME                                                                       */
/* ========================================================================= */

static inline void tbos_sleep_ms(uint32_t ms) {
#ifdef TBOS_PLATFORM_WINDOWS
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static inline uint64_t tbos_time_ms(void) {
#ifdef TBOS_PLATFORM_WINDOWS
    return (uint64_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
#endif
}

/* ========================================================================= */
/* TERMINAL / CONSOLE                                                         */
/* ========================================================================= */

static inline bool tbos_is_terminal(void) {
#ifdef TBOS_PLATFORM_WINDOWS
    return _isatty(_fileno(stdin));
#else
    return isatty(STDIN_FILENO);
#endif
}

static inline void tbos_clear_screen(void) {
#ifdef TBOS_PLATFORM_WINDOWS
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

/* ANSI color support */
static inline bool tbos_supports_color(void) {
#ifdef TBOS_PLATFORM_WINDOWS
    /* Windows 10+ supports ANSI */
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (GetConsoleMode(hOut, &mode)) {
        SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        return true;
    }
    return false;
#else
    const char* term = getenv("TERM");
    return term && strcmp(term, "dumb") != 0;
#endif
}

/* ========================================================================= */
/* PATH UTILITIES                                                             */
/* ========================================================================= */

static inline void tbos_path_normalize(char* path) {
#ifdef TBOS_PLATFORM_WINDOWS
    /* Convert forward slashes to backslashes */
    for (char* p = path; *p; p++) {
        if (*p == '/') *p = '\\';
    }
#else
    /* Convert backslashes to forward slashes */
    for (char* p = path; *p; p++) {
        if (*p == '\\') *p = '/';
    }
#endif
}

static inline void tbos_path_join(char* dest, size_t size, const char* a, const char* b) {
    snprintf(dest, size, "%s%c%s", a, PATH_SEPARATOR, b);
}

/* ========================================================================= */
/* PLATFORM INFO                                                              */
/* ========================================================================= */

typedef struct {
    const char* name;           /* Platform name */
    const char* arch;           /* Architecture */
    bool is_posix;              /* POSIX compatible */
    bool is_sandboxed;          /* Restricted environment */
    bool can_execute;           /* Can run system commands */
    bool supports_color;        /* Terminal color support */
} tbos_platform_info_t;

static inline void tbos_get_platform_info(tbos_platform_info_t* info) {
    info->name = TBOS_PLATFORM_NAME;

#if defined(__x86_64__) || defined(_M_X64)
    info->arch = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
    info->arch = "x86";
#elif defined(__aarch64__) || defined(_M_ARM64)
    info->arch = "arm64";
#elif defined(__arm__) || defined(_M_ARM)
    info->arch = "arm";
#elif defined(__riscv)
    info->arch = "riscv";
#else
    info->arch = "unknown";
#endif

#ifdef TBOS_PLATFORM_POSIX
    info->is_posix = true;
#else
    info->is_posix = false;
#endif

#ifdef TBOS_SANDBOXED
    info->is_sandboxed = true;
#else
    info->is_sandboxed = false;
#endif

    info->can_execute = tbos_can_execute();
    info->supports_color = tbos_supports_color();
}

static inline void tbos_print_platform_info(void) {
    tbos_platform_info_t info;
    tbos_get_platform_info(&info);

    printf("Platform: %s (%s)\n", info.name, info.arch);
    printf("POSIX:    %s\n", info.is_posix ? "Yes" : "No");
    printf("Sandbox:  %s\n", info.is_sandboxed ? "Yes" : "No");
    printf("Execute:  %s\n", info.can_execute ? "Yes" : "No");
    printf("Color:    %s\n", info.supports_color ? "Yes" : "No");
}

#endif /* TBOS_PLATFORM_H */
