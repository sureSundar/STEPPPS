/**
 * @file baremetal_compat.h
 * @brief Compatibility layer for compiling hosted TBOS code in bare-metal
 */

#ifndef TBOS_BAREMETAL_COMPAT_H
#define TBOS_BAREMETAL_COMPAT_H

/* For bare-metal builds, redirect standard library functions to kernel equivalents */
#ifndef HOST_BUILD

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Basic libc functions */
extern void* memset(void* s, int c, size_t n);
extern void* memcpy(void* dest, const void* src, size_t n);
extern void* memmove(void* dest, const void* src, size_t n);
extern int memcmp(const void* s1, const void* s2, size_t n);
extern char* strcpy(char* dest, const char* src);
extern char* strncpy(char* dest, const char* src, size_t n);
extern size_t strlen(const char* s);
extern int strcmp(const char* s1, const char* s2);
extern int strncmp(const char* s1, const char* s2, size_t n);
extern char* strcat(char* dest, const char* src);
extern char* strchr(const char* s, int c);
extern char* strstr(const char* haystack, const char* needle);

/* Redirect printf to kernel_printf (defined in kernel_main.c) */
extern void kernel_printf(const char* fmt, ...);
#define printf kernel_printf
#define fprintf(stream, fmt, ...) kernel_printf(fmt, ##__VA_ARGS__)
#define vprintf(fmt, args) kernel_printf(fmt)
#define sprintf(buf, fmt, ...) sprintf_minimal(buf, fmt, ##__VA_ARGS__)
#define snprintf(buf, size, fmt, ...) snprintf_minimal(buf, size, fmt, ##__VA_ARGS__)

/* Minimal sprintf/snprintf implementations */
int sprintf_minimal(char* buf, const char* fmt, ...);
int snprintf_minimal(char* buf, size_t size, const char* fmt, ...);

/* Time functions - use simple counter */
typedef long time_t;
struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

static inline time_t time(time_t* t) {
    /* Simple uptime counter */
    static time_t counter = 0;
    counter++;
    if (t) *t = counter;
    return counter;
}

static inline struct tm* localtime(const time_t* timep) {
    static struct tm tm_buf = {0};
    (void)timep;
    return &tm_buf;
}

/* File operations - no-op stubs */
typedef struct { int dummy; } FILE;
#define stdin  ((FILE*)0)
#define stdout ((FILE*)1)
#define stderr ((FILE*)2)

static inline FILE* fopen(const char* path, const char* mode) {
    (void)path; (void)mode;
    return NULL;
}

static inline int fclose(FILE* fp) {
    (void)fp;
    return 0;
}

static inline char* fgets(char* s, int size, FILE* stream) {
    (void)s; (void)size; (void)stream;
    return NULL;
}

static inline int feof(FILE* stream) {
    (void)stream;
    return 1;
}

static inline size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    (void)ptr; (void)size; (void)nmemb; (void)stream;
    return 0;
}

static inline size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    (void)ptr; (void)size; (void)nmemb; (void)stream;
    return 0;
}

/* Character classification */
static inline int isspace(int c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v');
}

static inline int isdigit(int c) {
    return (c >= '0' && c <= '9');
}

static inline int isalpha(int c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

static inline int isalnum(int c) {
    return (isalpha(c) || isdigit(c));
}

static inline int tolower(int c) {
    return (c >= 'A' && c <= 'Z') ? (c + 32) : c;
}

static inline int toupper(int c) {
    return (c >= 'a' && c <= 'z') ? (c - 32) : c;
}

/* stdlib */
#ifndef NULL
#define NULL ((void*)0)
#endif

static inline void* malloc(size_t size) {
    (void)size;
    return NULL; /* No dynamic allocation in bare-metal */
}

static inline void* calloc(size_t nmemb, size_t size) {
    (void)nmemb; (void)size;
    return NULL;
}

static inline void* realloc(void* ptr, size_t size) {
    (void)ptr; (void)size;
    return NULL;
}

static inline void free(void* ptr) {
    (void)ptr;
}

static inline int atoi(const char* nptr) {
    int result = 0;
    int sign = 1;
    if (*nptr == '-') {
        sign = -1;
        nptr++;
    }
    while (*nptr >= '0' && *nptr <= '9') {
        result = result * 10 + (*nptr - '0');
        nptr++;
    }
    return sign * result;
}

static inline long strtol(const char* nptr, char** endptr, int base) {
    (void)base; /* Simplified: only handle base 10 */
    if (endptr) *endptr = (char*)nptr;
    return (long)atoi(nptr);
}

static inline unsigned long strtoul(const char* nptr, char** endptr, int base) {
    (void)base;
    if (endptr) *endptr = (char*)nptr;
    return (unsigned long)atoi(nptr);
}

/* Exit/abort */
static inline void exit(int status) {
    (void)status;
    while(1) { __asm__("hlt"); }
}

static inline void abort(void) {
    while(1) { __asm__("hlt"); }
}

#else /* HOST_BUILD */

/* In hosted mode, use real standard library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#endif /* HOST_BUILD */

#endif /* TBOS_BAREMETAL_COMPAT_H */
