#ifndef TBOS_STDIO_H
#define TBOS_STDIO_H

#include <stddef.h>
#include <stdarg.h>
#include "tbos/libc.h"
#include "tbos/vfs.h"

#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

typedef struct FILE FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

FILE* fopen(const char* path, const char* mode);
int   fclose(FILE* stream);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int   fflush(FILE* stream);
int   fseek(FILE* stream, long offset, int whence);
long  ftell(FILE* stream);
int   feof(FILE* stream);
int   ferror(FILE* stream);
void  clearerr(FILE* stream);
int   fileno(FILE* stream);

int   fprintf(FILE* stream, const char* format, ...);
int   vfprintf(FILE* stream, const char* format, va_list args);

/* Directory API */
typedef struct DIR DIR;

typedef struct dirent {
    char d_name[256];
    vfs_node_type_t d_type;
} dirent;

DIR* opendir(const char* name);
struct dirent* readdir(DIR* dirp);
int closedir(DIR* dirp);

/* stat */
struct stat {
    vfs_node_type_t st_mode;
    size_t st_size;
};

int stat(const char* path, struct stat* st);

#endif /* TBOS_STDIO_H */
