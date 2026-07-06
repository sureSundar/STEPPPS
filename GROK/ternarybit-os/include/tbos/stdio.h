#ifndef TBOS_STDIO_H
#define TBOS_STDIO_H

#include <stddef.h>
#include <stdarg.h>

/* In hosted mode, use system headers for file/directory operations */
#ifdef TBOS_HOSTED
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "tbos/libc.h"
#include "tbos/vfs.h"

#else /* Bare-metal mode: define our own types */

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

/* Unix-style permission types */
typedef uint32_t mode_t;
typedef uint32_t uid_t;
typedef uint32_t gid_t;
typedef int64_t  time_t;

/* File type bits (in st_mode upper bits) */
#define S_IFMT   0170000  /* Type mask */
#define S_IFREG  0100000  /* Regular file */
#define S_IFDIR  0040000  /* Directory */
#define S_IFLNK  0120000  /* Symbolic link */

/* Permission bits */
#define S_IRUSR  0000400  /* Owner read */
#define S_IWUSR  0000200  /* Owner write */
#define S_IXUSR  0000100  /* Owner execute */
#define S_IRGRP  0000040  /* Group read */
#define S_IWGRP  0000020  /* Group write */
#define S_IXGRP  0000010  /* Group execute */
#define S_IROTH  0000004  /* Other read */
#define S_IWOTH  0000002  /* Other write */
#define S_IXOTH  0000001  /* Other execute */

/* Common permission combinations */
#define S_IRWXU  (S_IRUSR | S_IWUSR | S_IXUSR)
#define S_IRWXG  (S_IRGRP | S_IWGRP | S_IXGRP)
#define S_IRWXO  (S_IROTH | S_IWOTH | S_IXOTH)

/* Type check macros */
#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)

/* stat structure */
struct stat {
    mode_t    st_mode;   /* File type + permissions (S_IFREG|0644, etc.) */
    uid_t     st_uid;    /* Owner user ID */
    gid_t     st_gid;    /* Owner group ID */
    size_t    st_size;   /* File size in bytes */
    time_t    st_mtime;  /* Last modification time */
    time_t    st_ctime;  /* Last status change time */
    uint32_t  st_ino;    /* Inode number */
    uint16_t  st_nlink;  /* Number of hard links */
};

int stat(const char* path, struct stat* st);
int chmod(const char* path, mode_t mode);
int chown(const char* path, uid_t uid, gid_t gid);
mode_t umask(mode_t mask);

#endif /* TBOS_HOSTED */

#endif /* TBOS_STDIO_H */
