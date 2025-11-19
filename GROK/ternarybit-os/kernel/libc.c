#include "tbos/libc.h"
#include "tbos/errno.h"
#include "tbos/stdio.h"
#include "kernel.h"
#include <stdarg.h>

#define HEAP_SIZE (64 * 1024)  // 64KB heap for bare-metal
#define ALIGNMENT 16

// Simple bump allocator - no free() support, but no initialization needed
static uint8_t heap_area[HEAP_SIZE];
static size_t heap_offset = 0;
int errno = 0;

static size_t align_up(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

// No-op for compatibility - bump allocator doesn't need initialization
void libc_init(void) {
    // Bump allocator is always ready
}

// Simple bump allocator - no complex block management
void* malloc(size_t size) {
    if (size == 0) return NULL;

    size = align_up(size);

    if (heap_offset + size > HEAP_SIZE) {
        errno = ENOMEM;
        return NULL;
    }

    void* ptr = &heap_area[heap_offset];
    heap_offset += size;
    return ptr;
}

// No-op - bump allocator doesn't support free
void free(void* ptr) {
    (void)ptr;
}

void* calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* ptr = malloc(total);
    if (ptr) {
        memset(ptr, 0, total);
    }
    return ptr;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) return NULL;

    // Bump allocator: just allocate new and copy
    void* new_ptr = malloc(size);
    if (!new_ptr) return NULL;

    // Copy old data (we don't track size, so copy requested amount)
    if (ptr) {
        memcpy(new_ptr, ptr, size);
    }
    return new_ptr;
}

void* memset(void* dest, int value, size_t len) {
    uint8_t* d = (uint8_t*)dest;
    for (size_t i = 0; i < len; i++) {
        d[i] = (uint8_t)value;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t len) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (size_t i = 0; i < len; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memmove(void* dest, const void* src, size_t len) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    if (d == s) return dest;

    if (d < s) {
        for (size_t i = 0; i < len; i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = len; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t len) {
    const uint8_t* a = (const uint8_t*)s1;
    const uint8_t* b = (const uint8_t*)s2;
    for (size_t i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
    }
    return 0;
}

size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len]) len++;
    return len;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0' || s2[i] == '\0') {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest + strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

char* strchr(const char* str, int c) {
    while (*str) {
        if (*str == (char)c) return (char*)str;
        str++;
    }
    return (c == '\0') ? (char*)str : NULL;
}

char* strrchr(const char* str, int c) {
    const char* last = NULL;
    while (*str) {
        if (*str == (char)c) last = str;
        str++;
    }
    if (c == '\0') return (char*)str;
    return (char*)last;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    size_t needle_len = strlen(needle);
    for (; *haystack; haystack++) {
        if (strncmp(haystack, needle, needle_len) == 0) {
            return (char*)haystack;
        }
    }
    return NULL;
}

char* strpbrk(const char* s, const char* accept) {
    while (*s) {
        const char* a = accept;
        while (*a) {
            if (*s == *a) return (char*)s;
            a++;
        }
        s++;
    }
    return NULL;
}

size_t strspn(const char* s, const char* accept) {
    size_t count = 0;
    while (*s) {
        const char* a = accept;
        bool match = false;
        while (*a) {
            if (*s == *a) {
                match = true;
                break;
            }
            a++;
        }
        if (!match) break;
        count++;
        s++;
    }
    return count;
}

size_t strcspn(const char* s, const char* reject) {
    size_t count = 0;
    while (*s) {
        const char* r = reject;
        while (*r) {
            if (*s == *r) return count;
            r++;
        }
        count++;
        s++;
    }
    return count;
}

char* strdup(const char* str) {
    size_t len = strlen(str);
    char* copy = malloc(len + 1);
    if (!copy) return NULL;
    memcpy(copy, str, len + 1);
    return copy;
}

char* strtok_r(char* str, const char* delim, char** saveptr) {
    if (!saveptr) return NULL;

    char* token;
    if (str) {
        token = str;
    } else {
        token = *saveptr;
    }

    if (!token) return NULL;

    token += strspn(token, delim);
    if (*token == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    char* end = token + strcspn(token, delim);
    if (*end == '\0') {
        *saveptr = NULL;
    } else {
        *end = '\0';
        *saveptr = end + 1;
    }
    return token;
}

char* strtok(char* str, const char* delim) {
    static char* saveptr;
    return strtok_r(str, delim, &saveptr);
}

int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isalpha(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int tolower(int c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    return c;
}

int toupper(int c) {
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

int atoi(const char* str) {
    return (int)strtol(str, NULL, 10);
}

long strtol(const char* str, char** endptr, int base) {
    if (base == 0) base = 10;

    while (isspace(*str)) str++;

    int sign = 1;
    if (*str == '+') {
        str++;
    } else if (*str == '-') {
        sign = -1;
        str++;
    }

    long result = 0;
    while (*str) {
        int digit;
        if (*str >= '0' && *str <= '9') {
            digit = *str - '0';
        } else if (*str >= 'A' && *str <= 'Z') {
            digit = *str - 'A' + 10;
        } else if (*str >= 'a' && *str <= 'z') {
            digit = *str - 'a' + 10;
        } else {
            break;
        }

        if (digit >= base) break;

        result = result * base + digit;
        str++;
    }

    if (endptr) *endptr = (char*)str;
    return sign * result;
}

unsigned long strtoul(const char* str, char** endptr, int base) {
    return (unsigned long)strtol(str, endptr, base);
}

static void swap_bytes(uint8_t* a, uint8_t* b, size_t size) {
    for (size_t i = 0; i < size; i++) {
        uint8_t tmp = a[i];
        a[i] = b[i];
        b[i] = tmp;
    }
}

static void quicksort(uint8_t* base, size_t left, size_t right, size_t size,
                      int (*compar)(const void*, const void*)) {
    if (left >= right) return;

    size_t pivot = left;
    size_t i = left + 1;
    size_t j = right;

    while (i <= j) {
        while (i <= right && compar(base + i * size, base + pivot * size) < 0) i++;
        while (j > left && compar(base + j * size, base + pivot * size) > 0) j--;
        if (i < j) {
            swap_bytes(base + i * size, base + j * size, size);
        }
        i++;
        if (j > 0) j--;
    }

    swap_bytes(base + pivot * size, base + j * size, size);

    if (j > 0) quicksort(base, left, j - 1, size, compar);
    quicksort(base, j + 1, right, size, compar);
}

void qsort(void* base, size_t nmemb, size_t size,
           int (*compar)(const void*, const void*)) {
    if (!base || nmemb < 2) return;
    quicksort((uint8_t*)base, 0, nmemb - 1, size, compar);
}

static void append_char(char** buffer, size_t* remaining, int c, int* count) {
    if (*remaining > 1) {
        **buffer = (char)c;
        (*buffer)++;
        (*remaining)--;
        **buffer = '\0';
    }
    (*count)++;
}

static void format_integer(char* temp, size_t temp_size, long value, int base, bool uppercase) {
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    char buf[32];
    int idx = 0;
    bool negative = false;

    if (value < 0 && base == 10) {
        negative = true;
        value = -value;
    }

    do {
        buf[idx++] = digits[value % base];
        value /= base;
    } while (value && idx < (int)sizeof(buf));

    if (negative) buf[idx++] = '-';

    int pos = 0;
    while (--idx >= 0 && pos < (int)temp_size - 1) {
        temp[pos++] = buf[idx];
    }
    temp[pos] = '\0';
}

static int kvsnprintf(char* buffer, size_t size, const char* format, va_list args) {
    char* ptr = buffer;
    size_t remaining = size;
    int count = 0;

    while (*format) {
        if (*format != '%') {
            append_char(&ptr, &remaining, *format++, &count);
            continue;
        }

        format++;
        char spec = *format++;

        switch (spec) {
            case 'd':
            case 'i': {
                int value = va_arg(args, int);
                char temp[32];
                format_integer(temp, sizeof(temp), value, 10, false);
                for (char* s = temp; *s; s++) {
                    append_char(&ptr, &remaining, *s, &count);
                }
                break;
            }
            case 'u': {
                unsigned int value = va_arg(args, unsigned int);
                char temp[32];
                format_integer(temp, sizeof(temp), value, 10, false);
                for (char* s = temp; *s; s++) {
                    append_char(&ptr, &remaining, *s, &count);
                }
                break;
            }
            case 'x':
            case 'X': {
                unsigned int value = va_arg(args, unsigned int);
                char temp[32];
                format_integer(temp, sizeof(temp), value, 16, spec == 'X');
                for (char* s = temp; *s; s++) {
                    append_char(&ptr, &remaining, *s, &count);
                }
                break;
            }
            case 'c': {
                int ch = va_arg(args, int);
                append_char(&ptr, &remaining, ch, &count);
                break;
            }
            case 's': {
                const char* str = va_arg(args, const char*);
                if (!str) str = "(null)";
                while (*str) {
                    append_char(&ptr, &remaining, *str++, &count);
                }
                break;
            }
            case 'f': {
                double value = va_arg(args, double);
                long whole = (long)value;
                long frac = (long)((value - whole) * 1000.0);
                if (frac < 0) frac = -frac;
                char temp[32];
                format_integer(temp, sizeof(temp), whole, 10, false);
                for (char* s = temp; *s; s++) append_char(&ptr, &remaining, *s, &count);
                append_char(&ptr, &remaining, '.', &count);
                format_integer(temp, sizeof(temp), frac, 10, false);
                for (char* s = temp; *s && s - temp < 3; s++) append_char(&ptr, &remaining, *s, &count);
                break;
            }
            case '%': {
                append_char(&ptr, &remaining, '%', &count);
                break;
            }
            default: {
                append_char(&ptr, &remaining, '%', &count);
                append_char(&ptr, &remaining, spec, &count);
                break;
            }
        }
    }

    if (size > 0) {
        *ptr = '\0';
    }
    return count;
}

int vsnprintf(char* buffer, size_t size, const char* format, va_list args) {
    return kvsnprintf(buffer, size, format, args);
}

int snprintf(char* buffer, size_t size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

int vprintf(const char* format, va_list args) {
    char temp[1024];
    int len = vsnprintf(temp, sizeof(temp), format, args);
    for (int i = 0; i < len; i++) {
        putchar(temp[i]);
    }
    return len;
}

int printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vprintf(format, args);
    va_end(args);
    return result;
}

int puts(const char* str) {
    if (!str) return -1;
    while (*str) {
        putchar(*str++);
    }
    putchar('\n');
    return 0;
}

int putchar(int c) {
    extern void kernel_putchar(char c);
    kernel_putchar((char)c);
    return c;
}

typedef struct {
    int code;
    const char* message;
} errno_entry_t;

static const errno_entry_t errno_table[] = {
    { EPERM,        "Operation not permitted" },
    { ENOENT,       "No such file or directory" },
    { ESRCH,        "No such process" },
    { EINTR,        "Interrupted system call" },
    { EIO,          "I/O error" },
    { ENXIO,        "No such device or address" },
    { E2BIG,        "Argument list too long" },
    { ENOEXEC,      "Exec format error" },
    { EBADF,        "Bad file descriptor" },
    { ECHILD,       "No child processes" },
    { EAGAIN,       "Resource temporarily unavailable" },
    { ENOMEM,       "Out of memory" },
    { EACCES,       "Permission denied" },
    { EFAULT,       "Bad address" },
    { EBUSY,        "Resource busy" },
    { EEXIST,       "File exists" },
    { EXDEV,        "Invalid cross-device link" },
    { ENODEV,       "No such device" },
    { ENOTDIR,      "Not a directory" },
    { EISDIR,       "Is a directory" },
    { EINVAL,       "Invalid argument" },
    { ENFILE,       "Too many open files in system" },
    { EMFILE,       "Too many open files" },
    { ENOTTY,       "Inappropriate ioctl for device" },
    { EFBIG,        "File too large" },
    { ENOSPC,       "No space left on device" },
    { ESPIPE,       "Illegal seek" },
    { EROFS,        "Read-only file system" },
    { EMLINK,       "Too many links" },
    { EPIPE,        "Broken pipe" },
    { EDOM,         "Math argument out of domain" },
    { ERANGE,       "Math result out of range" },
    { ENOSYS,       "Function not implemented" },
    { ENOTSUP,      "Operation not supported" },
};

const char* strerror(int errnum) {
    for (size_t i = 0; i < sizeof(errno_table) / sizeof(errno_entry_t); i++) {
        if (errno_table[i].code == errnum) {
            return errno_table[i].message;
        }
    }
    return "Unknown error";
}

void perror(const char* s) {
    if (s && *s) {
        printf("%s: %s\n", s, strerror(errno));
    } else {
        printf("%s\n", strerror(errno));
    }
}

/* Stdio placeholders – real VFS-backed implementation pending */
typedef enum {
    FILE_KIND_CONSOLE,
    FILE_KIND_VFS
} file_kind_t;

struct FILE {
    file_kind_t kind;
    char* path;
    uint8_t* buffer;
    size_t size;
    size_t capacity;
    size_t pos;
    bool readable;
    bool writable;
    bool dirty;
};

static FILE console_out = {
    .kind = FILE_KIND_CONSOLE,
    .path = NULL,
    .buffer = NULL,
    .size = 0,
    .capacity = 0,
    .pos = 0,
    .readable = false,
    .writable = true,
    .dirty = false
};

FILE* stdin = NULL;
FILE* stdout = &console_out;
FILE* stderr = &console_out;

static bool file_ensure_capacity(FILE* stream, size_t desired) {
    if (stream->capacity >= desired) return true;
    size_t new_cap = stream->capacity ? stream->capacity * 2 : desired;
    if (new_cap < desired) new_cap = desired;
    uint8_t* new_buf = realloc(stream->buffer, new_cap);
    if (!new_buf) {
        errno = ENOMEM;
        return false;
    }
    stream->buffer = new_buf;
    stream->capacity = new_cap;
    return true;
}

static FILE* file_open_console(void) {
    return &console_out;
}

static FILE* file_open_vfs(const char* path, const char* mode_flags) {
    FILE* stream = malloc(sizeof(FILE));
    if (!stream) {
        errno = ENOMEM;
        return NULL;
    }
    stream->kind = FILE_KIND_VFS;
    stream->path = strdup(path);
    stream->buffer = NULL;
    stream->size = 0;
    stream->capacity = 0;
    stream->pos = 0;
    stream->dirty = false;
    stream->readable = false;
    stream->writable = false;

    bool read = false;
    bool write = false;
    bool append = false;

    for (const char* p = mode_flags; *p; p++) {
        switch (*p) {
            case 'r': read = true; break;
            case 'w': write = true; break;
            case 'a': append = true; write = true; break;
            default: break;
        }
    }

    if (read) stream->readable = true;
    if (write) stream->writable = true;

    if (read || append) {
        size_t file_size = 0;
        const void* data = vfs_read_file_cstr(path, &file_size);
        if (!data) {
            if (read) {
                free(stream->path);
                free(stream);
                errno = ENOENT;
                return NULL;
            }
        } else {
            if (!file_ensure_capacity(stream, file_size)) {
                free(stream->path);
                free(stream);
                return NULL;
            }
            memcpy(stream->buffer, data, file_size);
            stream->size = file_size;
            stream->capacity = file_size;
            stream->pos = append ? file_size : 0;
        }
    }

    if (write && !read && !append) {
        if (!file_ensure_capacity(stream, 0)) {
            free(stream->path);
            free(stream);
            return NULL;
        }
        stream->size = 0;
        stream->pos = 0;
        stream->dirty = true;
    }

    return stream;
}

FILE* fopen(const char* path, const char* mode) {
    if (!path || !mode) {
        errno = EINVAL;
        return NULL;
    }
    if (strcmp(path, "stdout") == 0 || strcmp(path, "/dev/stdout") == 0) {
        return file_open_console();
    }
    return file_open_vfs(path, mode);
}

static int file_flush(FILE* stream) {
    if (!stream || stream->kind != FILE_KIND_VFS) return 0;
    if (!stream->dirty || !stream->writable) return 0;
    int rc = vfs_write_file(stream->path, stream->buffer, stream->size);
    if (rc != 0) {
        errno = -rc;
        return -1;
    }
    stream->dirty = false;
    return 0;
}

int fclose(FILE* stream) {
    if (!stream || stream == &console_out) {
        return 0;
    }
    file_flush(stream);
    free(stream->buffer);
    free(stream->path);
    free(stream);
    return 0;
}

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream || !stream->readable) {
        errno = EBADF;
        return 0;
    }
    size_t bytes = size * nmemb;
    size_t remaining = stream->size > stream->pos ? stream->size - stream->pos : 0;
    if (bytes > remaining) bytes = remaining;
    memcpy(ptr, stream->buffer + stream->pos, bytes);
    stream->pos += bytes;
    return bytes / size;
}

size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream) {
    if (!stream) {
        errno = EBADF;
        return 0;
    }
    if (stream->kind == FILE_KIND_CONSOLE) {
        const uint8_t* p = ptr;
        size_t bytes = size * nmemb;
        for (size_t i = 0; i < bytes; i++) {
            putchar(p[i]);
        }
        return nmemb;
    }
    if (!stream->writable) {
        errno = EBADF;
        return 0;
    }
    size_t bytes = size * nmemb;
    size_t new_pos = stream->pos + bytes;
    if (!file_ensure_capacity(stream, new_pos)) {
        return 0;
    }
    memcpy(stream->buffer + stream->pos, ptr, bytes);
    stream->pos = new_pos;
    if (stream->pos > stream->size) {
        stream->size = stream->pos;
    }
    stream->dirty = true;
    return nmemb;
}

int fflush(FILE* stream) {
    if (!stream) return 0;
    if (stream->kind == FILE_KIND_CONSOLE) return 0;
    return file_flush(stream);
}

int fseek(FILE* stream, long offset, int whence) {
    if (!stream || stream->kind == FILE_KIND_CONSOLE) {
        errno = ENOSYS;
        return -1;
    }
    long base = 0;
    switch (whence) {
        case SEEK_SET: base = 0; break;
        case SEEK_CUR: base = (long)stream->pos; break;
        case SEEK_END: base = (long)stream->size; break;
        default: errno = EINVAL; return -1;
    }
    long new_pos = base + offset;
    if (new_pos < 0) {
        errno = EINVAL;
        return -1;
    }
    if ((size_t)new_pos > stream->size && stream->writable) {
        if (!file_ensure_capacity(stream, (size_t)new_pos)) return -1;
        memset(stream->buffer + stream->size, 0, (size_t)new_pos - stream->size);
        stream->size = (size_t)new_pos;
        stream->dirty = true;
    } else if ((size_t)new_pos > stream->size) {
        errno = EINVAL;
        return -1;
    }
    stream->pos = (size_t)new_pos;
    return 0;
}

long ftell(FILE* stream) {
    if (!stream || stream->kind == FILE_KIND_CONSOLE) {
        errno = ENOSYS;
        return -1;
    }
    return (long)stream->pos;
}

int feof(FILE* stream) {
    if (!stream || stream->kind == FILE_KIND_CONSOLE) return 1;
    return stream->pos >= stream->size;
}

int ferror(FILE* stream) {
    (void)stream;
    return errno;
}

void clearerr(FILE* stream) {
    (void)stream;
    errno = 0;
}

int fileno(FILE* stream) {
    (void)stream;
    errno = ENOSYS;
    return -1;
}

int vfprintf(FILE* stream, const char* format, va_list args) {
    if (!stream || stream->kind == FILE_KIND_CONSOLE) {
        return vprintf(format, args);
    }
    va_list copy;
    va_copy(copy, args);
    char temp[256];
    int needed = kvsnprintf(temp, sizeof(temp), format, copy);
    va_end(copy);

    if ((size_t)needed < sizeof(temp)) {
        return fwrite(temp, 1, needed, stream);
    }

    char* buf = malloc(needed + 1);
    if (!buf) {
        errno = ENOMEM;
        return -1;
    }
    va_copy(copy, args);
    kvsnprintf(buf, needed + 1, format, copy);
    va_end(copy);
    size_t written = fwrite(buf, 1, needed, stream);
    free(buf);
    return written;
}

int fprintf(FILE* stream, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int result = vfprintf(stream, format, args);
    va_end(args);
    return result;
}

/* Directory support */
struct DIR {
    char* path;
    size_t index;
    size_t count;
    struct dirent* entries;
};

static int dir_collect_cb(const char* name, vfs_node_type_t type, void* user) {
    DIR* dir = (DIR*)user;
    struct dirent* new_entries = realloc(dir->entries, (dir->count + 1) * sizeof(struct dirent));
    if (!new_entries) {
        errno = ENOMEM;
        return -ENOMEM;
    }
    dir->entries = new_entries;
    struct dirent* entry = &dir->entries[dir->count++];
    strncpy(entry->d_name, name, sizeof(entry->d_name) - 1);
    entry->d_name[sizeof(entry->d_name) - 1] = '\0';
    entry->d_type = type;
    return 0;
}

DIR* opendir(const char* name) {
    if (!name) {
        errno = EINVAL;
        return NULL;
    }
    DIR* dir = malloc(sizeof(DIR));
    if (!dir) {
        errno = ENOMEM;
        return NULL;
    }
    dir->path = strdup(name);
    dir->index = 0;
    dir->count = 0;
    dir->entries = NULL;

    int rc = vfs_list_dir(name, dir_collect_cb, dir);
    if (rc != 0) {
        free(dir->entries);
        free(dir->path);
        free(dir);
        errno = -rc;
        return NULL;
    }
    return dir;
}

struct dirent* readdir(DIR* dirp) {
    if (!dirp) {
        errno = EBADF;
        return NULL;
    }
    if (dirp->index >= dirp->count) {
        return NULL;
    }
    return &dirp->entries[dirp->index++];
}

int closedir(DIR* dirp) {
    if (!dirp) {
        errno = EBADF;
        return -1;
    }
    free(dirp->entries);
    free(dirp->path);
    free(dirp);
    return 0;
}

int stat(const char* path, struct stat* st) {
    if (!path || !st) {
        errno = EINVAL;
        return -1;
    }
    if (!vfs_exists(path)) {
        errno = ENOENT;
        return -1;
    }
    st->st_mode = vfs_type(path);
    st->st_size = 0;
    if (st->st_mode == VFS_NODE_FILE) {
        size_t size = 0;
        const void* data = vfs_read_file_cstr(path, &size);
        if (data) {
            st->st_size = size;
        } else {
            errno = ENOSYS;
            return -1;
        }
    }
    return 0;
}
