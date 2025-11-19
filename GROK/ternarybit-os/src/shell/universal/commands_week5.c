/**
 * @file commands_week5.c
 * @brief Week 5 File Comparison & Compression (Days 29-35)
 *
 * Commands:
 *  - diff    : Compare files line by line
 *  - cmp     : Compare files byte by byte
 *  - file    : Determine file type
 *  - tar     : Archive files (simplified create/extract)
 *  - gzip    : Compress files using zlib (if available)
 *  - gunzip  : Decompress .gz files
 *  - base64  : Encode/decode base64 data
 */

#include "tbos_universal_shell.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

typedef struct {
    FILE* file;
    bool close;
    const char* name;
} stream_t;

static FILE* open_stream(const char* path, const char* mode, stream_t* stream) {
    stream->close = false;
    stream->name = path ? path : "(stdin)";
    if (!path || strcmp(path, "-") == 0) {
        stream->file = (mode[0] == 'r') ? stdin : stdout;
        return stream->file;
    }

    stream->file = fopen(path, mode);
    if (!stream->file) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return NULL;
    }
    stream->close = true;
    return stream->file;
}

static void close_stream(stream_t* stream) {
    if (stream->close && stream->file) {
        fclose(stream->file);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * diff
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_diff(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: diff FILE1 FILE2\n");
        return 1;
    }

    stream_t a, b;
    FILE* f1 = open_stream(argv[1], "rb", &a);
    if (!f1) return 1;
    FILE* f2 = open_stream(argv[2], "rb", &b);
    if (!f2) {
        close_stream(&a);
        return 1;
    }

    char* line1 = NULL;
    char* line2 = NULL;
    size_t len1 = 0, len2 = 0;
    ssize_t n1, n2;
    int line_no = 1;
    int status = 0;

    while (1) {
        n1 = getline(&line1, &len1, f1);
        n2 = getline(&line2, &len2, f2);

        if (n1 == -1 && n2 == -1) {
            break; /* both ended */
        }

        if (n1 == -1 || n2 == -1 || strcmp(line1, line2) != 0) {
            printf("%s:%d %s", argv[1], line_no, n1 == -1 ? "(EOF)\n" : line1);
            printf("%s:%d %s", argv[2], line_no, n2 == -1 ? "(EOF)\n" : line2);
            status = 1;
        }
        line_no++;
    }

    free(line1);
    free(line2);
    close_stream(&a);
    close_stream(&b);

    if (status == 0) {
        printf("Files %s and %s are identical.\n", argv[1], argv[2]);
    }

    universal_add_karma(status == 0 ? 1 : 2, "Compared truths (diff)");
    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * cmp
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_cmp(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: cmp FILE1 FILE2\n");
        return 1;
    }

    stream_t a, b;
    FILE* f1 = open_stream(argv[1], "rb", &a);
    if (!f1) return 1;
    FILE* f2 = open_stream(argv[2], "rb", &b);
    if (!f2) {
        close_stream(&a);
        return 1;
    }

    int ch1, ch2;
    size_t byte = 1;
    int status = 0;
    while ((ch1 = fgetc(f1)) != EOF && (ch2 = fgetc(f2)) != EOF) {
        if (ch1 != ch2) {
            printf("%s %s differ: byte %zu\n", argv[1], argv[2], byte);
            status = 1;
            break;
        }
        byte++;
    }

    if (status == 0) {
        if (ch1 != ch2) {
            printf("EOF mismatch at byte %zu\n", byte);
            status = 1;
        } else {
            printf("%s %s: identical bytes\n", argv[1], argv[2]);
        }
    }

    close_stream(&a);
    close_stream(&b);

    universal_add_karma(status == 0 ? 1 : 2, "Verified byte equality (cmp)");
    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * file
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* detect_file_type(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return strerror(errno);
    }
    if (S_ISDIR(st.st_mode)) return "directory";
    if (S_ISLNK(st.st_mode)) return "symbolic link";
    if (S_ISCHR(st.st_mode)) return "character device";
    if (S_ISBLK(st.st_mode)) return "block device";
    if (S_ISFIFO(st.st_mode)) return "FIFO/pipe";
    if (S_ISSOCK(st.st_mode)) return "socket";

    FILE* fp = fopen(path, "rb");
    if (!fp) return strerror(errno);
    unsigned char header[4] = {0};
    size_t n = fread(header, 1, sizeof(header), fp);
    fclose(fp);

    if (n >= 3 && header[0] == 0x1F && header[1] == 0x8B) return "gzip compressed data";
    if (n >= 4 && header[0] == 0x50 && header[1] == 0x4B) return "zip archive";
    if (n >= 4 && header[0] == 0x7F && header[1] == 'E' && header[2] == 'L' && header[3] == 'F') return "ELF executable";
    return "regular file";
}

static int cmd_file(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: file PATH...\n");
        return 1;
    }
    int status = 0;
    for (int i = 1; i < argc; i++) {
        const char* type = detect_file_type(argv[i]);
        if (!type) {
            status = 1;
        } else {
            printf("%s: %s\n", argv[i], type);
        }
    }
    universal_add_karma(1, "Perceived essence (file)");
    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * tar (minimal)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_tar(int argc, char** argv) {
    bool create = false;
    bool extract = false;
    const char* archive = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "cf:x")) != -1) {
        switch (opt) {
            case 'c': create = true; break;
            case 'x': extract = true; break;
            case 'f': archive = optarg; break;
            default:
                fprintf(stderr, "Usage: tar -c|-x -f archive [file...]\n");
                return 1;
        }
    }

    if ((create && extract) || (!create && !extract) || !archive) {
        fprintf(stderr, "Usage: tar -c|-x -f archive [file...]\n");
        return 1;
    }

    if (create) {
        FILE* out = fopen(archive, "wb");
        if (!out) {
            fprintf(stderr, "%s: %s\n", archive, strerror(errno));
            return 1;
        }

        for (int i = optind; i < argc; i++) {
            FILE* in = fopen(argv[i], "rb");
            if (!in) {
                fprintf(stderr, "%s: %s\n", argv[i], strerror(errno));
                fclose(out);
                return 1;
            }
            fprintf(out, "FILE %s\n", argv[i]);
            char buffer[4096];
            size_t n;
            while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
                fwrite(buffer, 1, n, out);
            }
            fprintf(out, "\nEND\n");
            fclose(in);
        }
        fclose(out);
        universal_add_karma(2, "Archived memories (tar)");
    } else if (extract) {
        FILE* in = fopen(archive, "rb");
        if (!in) {
            fprintf(stderr, "%s: %s\n", archive, strerror(errno));
            return 1;
        }
        char line[4096];
        FILE* current = NULL;
        while (fgets(line, sizeof(line), in)) {
            if (strncmp(line, "FILE ", 5) == 0) {
                if (current) fclose(current);
                char filename[256];
                sscanf(line + 5, "%255s", filename);
                current = fopen(filename, "wb");
                if (!current) {
                    fprintf(stderr, "%s: %s\n", filename, strerror(errno));
                    fclose(in);
                    return 1;
                }
            } else if (strcmp(line, "END\n") == 0) {
                if (current) {
                    fclose(current);
                    current = NULL;
                }
            } else if (current) {
                fputs(line, current);
            }
        }
        if (current) fclose(current);
        fclose(in);
        universal_add_karma(2, "Released archives (tar)");
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * gzip / gunzip (simple wrappers)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_gzip(int argc, char** argv) {
#ifndef HAVE_ZLIB
    (void)argc;
    (void)argv;
    fprintf(stderr, "gzip: zlib support not built-in\n");
    return 1;
#else
    if (argc < 2) {
        fprintf(stderr, "Usage: gzip FILE\n");
        return 1;
    }

    const char* input = argv[1];
    char output[512];
    snprintf(output, sizeof(output), "%s.gz", input);

    FILE* in = fopen(input, "rb");
    if (!in) {
        fprintf(stderr, "%s: %s\n", input, strerror(errno));
        return 1;
    }
    gzFile out = gzopen(output, "wb");
    if (!out) {
        fprintf(stderr, "%s.gz: cannot open\n", input);
        fclose(in);
        return 1;
    }

    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), in)) > 0) {
        if (gzwrite(out, buffer, (unsigned)n) != (int)n) {
            fprintf(stderr, "gzwrite failed\n");
            fclose(in);
            gzclose(out);
            return 1;
        }
    }

    fclose(in);
    gzclose(out);
    universal_add_karma(2, "Compressed experiences (gzip)");
    return 0;
#endif
}

static int cmd_gunzip(int argc, char** argv) {
#ifndef HAVE_ZLIB
    (void)argc;
    (void)argv;
    fprintf(stderr, "gunzip: zlib support not built-in\n");
    return 1;
#else
    if (argc < 2) {
        fprintf(stderr, "Usage: gunzip FILE.gz\n");
        return 1;
    }

    const char* input = argv[1];
    char output[512];
    snprintf(output, sizeof(output), "%.*s", (int)(strlen(input) - 3), input);

    gzFile in = gzopen(input, "rb");
    if (!in) {
        fprintf(stderr, "%s: cannot open\n", input);
        return 1;
    }
    FILE* out = fopen(output, "wb");
    if (!out) {
        fprintf(stderr, "%s: %s\n", output, strerror(errno));
        gzclose(in);
        return 1;
    }

    char buffer[4096];
    int n;
    while ((n = gzread(in, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, (size_t)n, out) != (size_t)n) {
            fprintf(stderr, "write failed\n");
            gzclose(in);
            fclose(out);
            return 1;
        }
    }

    gzclose(in);
    fclose(out);
    universal_add_karma(2, "Expanded insights (gunzip)");
    return 0;
#endif
}

/* ═══════════════════════════════════════════════════════════════════════════
 * base64
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode_stream(FILE* in, FILE* out) {
    unsigned char input[3];
    size_t n;
    while ((n = fread(input, 1, 3, in)) > 0) {
        unsigned char output[4];
        output[0] = base64_table[input[0] >> 2];
        output[1] = base64_table[((input[0] & 0x03) << 4) | (n > 1 ? (input[1] >> 4) : 0)];
        output[2] = (n > 1) ? base64_table[((input[1] & 0x0F) << 2) | (n > 2 ? (input[2] >> 6) : 0)] : '=';
        output[3] = (n > 2) ? base64_table[input[2] & 0x3F] : '=';
        fwrite(output, 1, 4, out);
    }
}

static int base64_decode_char(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    if (c == '=') return -1;
    return -2; /* invalid */
}

static int base64_decode_stream(FILE* in, FILE* out) {
    char input[4];
    size_t n;
    while ((n = fread(input, 1, 4, in)) == 4) {
        int vals[4];
        for (int i = 0; i < 4; i++) {
            vals[i] = base64_decode_char(input[i]);
            if (vals[i] == -2) return 1;
        }
        unsigned char output[3];
        output[0] = (unsigned char)((vals[0] << 2) | (vals[1] >> 4));
        fwrite(output, 1, 1, out);
        if (vals[2] >= 0) {
            output[1] = (unsigned char)(((vals[1] & 0x0F) << 4) | (vals[2] >> 2));
            fwrite(output + 1, 1, 1, out);
            if (vals[3] >= 0) {
                output[2] = (unsigned char)(((vals[2] & 0x03) << 6) | vals[3]);
                fwrite(output + 2, 1, 1, out);
            }
        }
    }
    return 0;
}

static int cmd_base64(int argc, char** argv) {
    bool decode = false;
    const char* input_path = NULL;
    const char* output_path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--decode") == 0) {
            decode = true;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[++i];
        } else {
            input_path = argv[i];
        }
    }

    stream_t in_stream, out_stream;
    FILE* in = open_stream(input_path, "rb", &in_stream);
    if (!in) return 1;
    FILE* out = open_stream(output_path, "wb", &out_stream);
    if (!out) {
        close_stream(&in_stream);
        return 1;
    }

    int status = 0;
    if (decode) {
        status = base64_decode_stream(in, out);
        if (status != 0) fprintf(stderr, "base64: invalid input\n");
    } else {
        base64_encode_stream(in, out);
    }

    close_stream(&in_stream);
    close_stream(&out_stream);

    if (status == 0) {
        universal_add_karma(2, decode ? "Decoded hidden wisdom (base64)" : "Encoded messages (base64)");
    }
    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 5 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week5_commands(void) {
    universal_shell_register_command(
        "diff", cmd_diff,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "compare files line by line",
        "diff FILE1 FILE2\n"
        "Show differing lines between files.");

    universal_shell_register_command(
        "cmp", cmd_cmp,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "compare files byte by byte",
        "cmp FILE1 FILE2\n"
        "Show first differing byte between files.");

    universal_shell_register_command(
        "file", cmd_file,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "determine file type",
        "file PATH...\n"
        "Display basic file type information.");

    universal_shell_register_command(
        "tar", cmd_tar,
        CMD_CAT_COMPRESSION, OS_SUPPORT_ALL,
        "archive files (simplified)",
        "tar -c -f archive files...\n"
        "tar -x -f archive\n"
        "Create or extract simple archives.");

    universal_shell_register_command(
        "gzip", cmd_gzip,
        CMD_CAT_COMPRESSION, OS_SUPPORT_ALL,
        "compress files (requires zlib)",
        "gzip FILE\n"
        "Compress FILE to FILE.gz (if zlib available).");

    universal_shell_register_command(
        "gunzip", cmd_gunzip,
        CMD_CAT_COMPRESSION, OS_SUPPORT_ALL,
        "decompress gzip files",
        "gunzip FILE.gz\n"
        "Decompress FILE.gz (if zlib available).");

    universal_shell_register_command(
        "base64", cmd_base64,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "base64 encode/decode data",
        "base64 [-d] [-o output] [file]\n"
        "Encode or decode data using Base64.");
}
