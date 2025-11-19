#ifndef TBOS_LIBC_H
#define TBOS_LIBC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include "tbos/errno.h"

void libc_init(void);

/* Memory management */
void* malloc(size_t size);
void  free(void* ptr);
void* calloc(size_t nmemb, size_t size);
void* realloc(void* ptr, size_t size);

/* Memory operations */
void* memset(void* dest, int value, size_t len);
void* memcpy(void* dest, const void* src, size_t len);
void* memmove(void* dest, const void* src, size_t len);
int   memcmp(const void* s1, const void* s2, size_t len);

/* String operations */
size_t strlen(const char* str);
int    strcmp(const char* s1, const char* s2);
int    strncmp(const char* s1, const char* s2, size_t n);
char*  strcpy(char* dest, const char* src);
char*  strncpy(char* dest, const char* src, size_t n);
char*  strcat(char* dest, const char* src);
char*  strchr(const char* str, int c);
char*  strrchr(const char* str, int c);
char*  strstr(const char* haystack, const char* needle);
char*  strpbrk(const char* s, const char* accept);
char*  strdup(const char* str);
char*  strtok_r(char* str, const char* delim, char** saveptr);
char*  strtok(char* str, const char* delim);
size_t strspn(const char* s, const char* accept);
size_t strcspn(const char* s, const char* reject);

/* Character helpers */
int isspace(int c);
int isdigit(int c);
int isalpha(int c);
int isalnum(int c);
int tolower(int c);
int toupper(int c);

/* Numeric conversions */
int  atoi(const char* str);
long strtol(const char* str, char** endptr, int base);
unsigned long strtoul(const char* str, char** endptr, int base);

/* Sorting */
void qsort(void* base, size_t nmemb, size_t size,
           int (*compar)(const void*, const void*));

/* I/O */
int putchar(int c);
int puts(const char* str);
int printf(const char* format, ...);
int vprintf(const char* format, va_list args);
int snprintf(char* buffer, size_t size, const char* format, ...);
int vsnprintf(char* buffer, size_t size, const char* format, va_list args);
const char* strerror(int errnum);
void perror(const char* s);

extern int errno;

#endif /* TBOS_LIBC_H */
