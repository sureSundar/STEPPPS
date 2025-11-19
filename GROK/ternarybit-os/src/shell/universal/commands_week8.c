/**
 * @file commands_week8.c
 * @brief Week 8 Binary & Encoding Utilities (Days 50-56)
 *
 * Commands:
 *  - strings   : Extract printable character runs
 *  - od        : Octal dump of binary data
 *  - hexdump   : Hexadecimal dump with ASCII gutter
 *  - xxd       : Vim-style hex dump
 *  - iconv     : Charset conversion utility
 *  - dos2unix  : Convert CRLF line endings to LF
 *  - unix2dos  : Convert LF line endings to CRLF
 *
 * These commands focus on bridging raw binary data with human readability,
 * reinforcing STEPPPS dimensions of Pixel (visualisation) and Script
 * (automation), while keeping mindfulness metrics via karma tracking.
 */

#include "tbos_universal_shell.h"

#include <ctype.h>
#include <errno.h>
#include <iconv.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define STRINGS_DEFAULT_MIN 4
#define HEX_BYTES_PER_LINE 16

typedef enum {
    DUMP_MODE_OCTAL,
    DUMP_MODE_HEX,
    DUMP_MODE_XXD
} dump_mode_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * STREAM HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static FILE* open_input_stream(const char* path, bool* need_close) {
    if (!path || strcmp(path, "-") == 0) {
        *need_close = false;
        return stdin;
    }

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return NULL;
    }

    *need_close = true;
    return fp;
}

static FILE* open_output_stream(const char* path, bool* need_close) {
    if (!path || strcmp(path, "-") == 0) {
        *need_close = false;
        return stdout;
    }

    FILE* fp = fopen(path, "wb");
    if (!fp) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return NULL;
    }

    *need_close = true;
    return fp;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * strings COMMAND
 * ═══════════════════════════════════════════════════════════════════════════ */

static int emit_strings(FILE* fp, const char* label, size_t min_len) {
    size_t capacity = 128;
    char* buffer = malloc(capacity);
    if (!buffer) {
        fprintf(stderr, "strings: out of memory\n");
        return 1;
    }

    size_t length = 0;
    buffer[0] = '\0';
    int ch;
    int status = 0;

    while ((ch = fgetc(fp)) != EOF) {
        if (isprint((unsigned char)ch) || ch == '\t') {
            if (length + 2 >= capacity) {
                size_t new_cap = capacity * 2;
                char* tmp = realloc(buffer, new_cap);
                if (!tmp) {
                    fprintf(stderr, "strings: out of memory\n");
                    status = 1;
                    break;
                }
                buffer = tmp;
                capacity = new_cap;
            }
            buffer[length++] = (char)ch;
            buffer[length] = '\0';
        } else {
            if (length >= min_len) {
                if (label) {
                    printf("%s: %s\n", label, buffer);
                } else {
                    printf("%s\n", buffer);
                }
            }
            length = 0;
            buffer[0] = '\0';
        }
    }

    if (length >= min_len) {
        if (label) {
            printf("%s: %s\n", label, buffer);
        } else {
            printf("%s\n", buffer);
        }
    }

    free(buffer);
    return status;
}

static int cmd_strings(int argc, char** argv) {
    size_t min_len = STRINGS_DEFAULT_MIN;
    int argi = 1;

    while (argi < argc) {
        if (strcmp(argv[argi], "-n") == 0 && argi + 1 < argc) {
            min_len = (size_t)strtoul(argv[argi + 1], NULL, 10);
            argi += 2;
        } else if (argv[argi][0] == '-' && strcmp(argv[argi], "-") != 0) {
            fprintf(stderr, "strings: unknown option '%s'\n", argv[argi]);
            return 1;
        } else {
            break;
        }
    }

    if (min_len == 0) min_len = STRINGS_DEFAULT_MIN;

    if (argi >= argc) {
        bool need_close = false;
        FILE* fp = open_input_stream(NULL, &need_close);
        int status = emit_strings(fp, NULL, min_len);
        if (need_close && fp) fclose(fp);
        if (status == 0) {
            universal_add_karma(2, "Revealed hidden messages (strings)");
        }
        return status;
    }

    int overall_status = 0;
    for (int i = argi; i < argc; i++) {
        bool need_close = false;
        FILE* fp = open_input_stream(argv[i], &need_close);
        if (!fp) {
            overall_status = 1;
            continue;
        }

        int status = emit_strings(fp, argv[i], min_len);
        if (need_close) fclose(fp);
        if (status != 0) {
            overall_status = status;
        }
    }

    if (overall_status == 0) {
        universal_add_karma(2, "Revealed hidden messages (strings)");
    }

    return overall_status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * HEX/OD DUMP HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_dump_oct_line(uint64_t offset,
                                const unsigned char* data,
                                size_t count) {
    printf("%07llo ", (unsigned long long)offset);

    for (size_t i = 0; i < count; i++) {
        printf("%03o ", data[i]);
    }

    printf("\n");
}

static void print_dump_hex_line(uint64_t offset,
                                const unsigned char* data,
                                size_t count,
                                bool vim_style) {
    if (vim_style) {
        printf("%08llx: ", (unsigned long long)offset);
    } else {
        printf("%08llx  ", (unsigned long long)offset);
    }

    for (size_t i = 0; i < HEX_BYTES_PER_LINE; i++) {
        if (i < count) {
            printf("%02x", data[i]);
        } else {
            printf("  ");
        }

        if (i % 2 == 1) {
            putchar(' ');
        }
    }

    printf(" ");
    putchar(' ');

    putchar('|');
    for (size_t i = 0; i < count; i++) {
        unsigned char c = data[i];
        putchar(isprint(c) ? c : '.');
    }
    for (size_t i = count; i < HEX_BYTES_PER_LINE; i++) {
        putchar(' ');
    }
    putchar('|');
    putchar('\n');
}

static int perform_dump(FILE* fp, dump_mode_t mode) {
    unsigned char buffer[HEX_BYTES_PER_LINE];
    size_t offset = 0;
    size_t nread;

    while ((nread = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        switch (mode) {
            case DUMP_MODE_OCTAL:
                print_dump_oct_line(offset, buffer, nread);
                break;
            case DUMP_MODE_HEX:
                print_dump_hex_line(offset, buffer, nread, false);
                break;
            case DUMP_MODE_XXD:
                print_dump_hex_line(offset, buffer, nread, true);
                break;
        }
        offset += nread;
    }

    if (ferror(fp)) {
        return 1;
    }

    return 0;
}

static int dump_command(int argc, char** argv, dump_mode_t mode) {
    int argi = 1;
    if (argi >= argc) {
        bool need_close = false;
        FILE* fp = open_input_stream(NULL, &need_close);
        int status = perform_dump(fp, mode);
        if (need_close && fp) fclose(fp);
        return status;
    }

    int status = 0;
    for (int i = argi; i < argc; i++) {
        bool need_close = false;
        FILE* fp = open_input_stream(argv[i], &need_close);
        if (!fp) {
            status = 1;
            continue;
        }
        int rc = perform_dump(fp, mode);
        if (need_close) fclose(fp);
        if (rc != 0) status = rc;
    }
    return status;
}

static int cmd_od(int argc, char** argv) {
    int status = dump_command(argc, argv, DUMP_MODE_OCTAL);
    if (status == 0) {
        universal_add_karma(2, "Viewed truth in octal light (od)");
    }
    return status;
}

static int cmd_hexdump(int argc, char** argv) {
    int status = dump_command(argc, argv, DUMP_MODE_HEX);
    if (status == 0) {
        universal_add_karma(2, "Illuminated bytes in hex (hexdump)");
    }
    return status;
}

static int cmd_xxd(int argc, char** argv) {
    int status = dump_command(argc, argv, DUMP_MODE_XXD);
    if (status == 0) {
        universal_add_karma(2, "Transcribed bytes mindfully (xxd)");
    }
    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * iconv COMMAND
 * ═══════════════════════════════════════════════════════════════════════════ */

static int convert_with_iconv(iconv_t cd, FILE* in, FILE* out) {
    unsigned char inbuf[4096];
    unsigned char outbuf[8192];

    while (1) {
        size_t inbytes = fread(inbuf, 1, sizeof(inbuf), in);
        if (inbytes == 0 && ferror(in)) {
            fprintf(stderr, "iconv: read error: %s\n", strerror(errno));
            return 1;
        }

        unsigned char* pin = inbuf;
        size_t inleft = inbytes;

        do {
            unsigned char* pout = outbuf;
            size_t outleft = sizeof(outbuf);
            size_t res = iconv(cd, (char**)&pin, &inleft, (char**)&pout, &outleft);
            size_t produced = sizeof(outbuf) - outleft;
            if (produced > 0 && fwrite(outbuf, 1, produced, out) != produced) {
                fprintf(stderr, "iconv: write error: %s\n", strerror(errno));
                return 1;
            }

            if (res == (size_t)-1) {
                if (errno == E2BIG) {
                    continue; /* Need more output space */
                } else if (errno == EILSEQ || errno == EINVAL) {
                    /* Skip invalid sequence */
                    pin++;
                    inleft--;
                    continue;
                } else {
                    fprintf(stderr, "iconv: conversion error: %s\n", strerror(errno));
                    return 1;
                }
            }
        } while (inleft > 0);

        if (inbytes == 0) {
            break; /* EOF */
        }
    }

    /* Flush shift state */
    while (1) {
        unsigned char* pout = outbuf;
        size_t outleft = sizeof(outbuf);
        size_t res = iconv(cd, NULL, NULL, (char**)&pout, &outleft);
        size_t produced = sizeof(outbuf) - outleft;
        if (produced > 0 && fwrite(outbuf, 1, produced, out) != produced) {
            fprintf(stderr, "iconv: write error: %s\n", strerror(errno));
            return 1;
        }
        if (res != (size_t)-1 || errno != E2BIG) {
            break;
        }
    }

    return 0;
}

static int cmd_iconv(int argc, char** argv) {
    const char* from_encoding = "UTF-8";
    const char* to_encoding = "UTF-8";
    const char* input_path = NULL;
    const char* output_path = NULL;

    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            from_encoding = argv[i + 1];
            i += 2;
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            to_encoding = argv[i + 1];
            i += 2;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[i + 1];
            i += 2;
        } else {
            break;
        }
    }

    if (i < argc) {
        input_path = argv[i++];
    }
    if (i < argc && !output_path) {
        output_path = argv[i++];
    }

    bool close_in = false;
    bool close_out = false;

    FILE* in = open_input_stream(input_path, &close_in);
    if (!in) return 1;

    FILE* out = open_output_stream(output_path, &close_out);
    if (!out) {
        if (close_in) fclose(in);
        return 1;
    }
    /* CX REVIEW: Opening the output stream before verifying iconv_open succeeds
     * can leave behind an empty/truncated file when the conversion setup fails.
     * Maybe defer creating/truncating the target until after iconv_open passes? */

    iconv_t cd = iconv_open(to_encoding, from_encoding);
    if (cd == (iconv_t)-1) {
        fprintf(stderr, "iconv: cannot convert from %s to %s: %s\n",
                from_encoding, to_encoding, strerror(errno));
        if (close_in) fclose(in);
        if (close_out) fclose(out);
        return 1;
    }

    int status = convert_with_iconv(cd, in, out);
    iconv_close(cd);

    if (close_in) fclose(in);
    if (close_out) fclose(out);

    if (status == 0) {
        universal_add_karma(3, "Bridged worlds via encoding (iconv)");
    }

    return status;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * LINE ENDING CONVERSIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static int convert_dos_to_unix(FILE* in, FILE* out) {
    int ch;
    while ((ch = fgetc(in)) != EOF) {
        if (ch == '\r') {
            int next = fgetc(in);
            if (next == '\n') {
                if (fputc('\n', out) == EOF) return 1;
            } else {
                if (fputc('\r', out) == EOF) return 1;
                if (next != EOF) {
                    if (ungetc(next, in) == EOF) return 1;
                }
            }
        } else {
            if (fputc(ch, out) == EOF) return 1;
        }
    }
    return ferror(in) ? 1 : 0;
}

static int convert_unix_to_dos(FILE* in, FILE* out) {
    int ch;
    int prev = -1;
    while ((ch = fgetc(in)) != EOF) {
        if (ch == '\n' && prev != '\r') {
            if (fputc('\r', out) == EOF) return 1;
        }
        if (fputc(ch, out) == EOF) return 1;
        prev = ch;
    }
    return ferror(in) ? 1 : 0;
}

static int line_endings_command(int argc, char** argv, bool to_unix) {
    const char* input_path = NULL;
    const char* output_path = NULL;

    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_path = argv[i + 1];
            i += 2;
        } else {
            input_path = argv[i++];
        }
    }

    bool close_in = false;
    bool close_out = false;

    FILE* in = open_input_stream(input_path, &close_in);
    if (!in) return 1;

    FILE* out = open_output_stream(output_path, &close_out);
    if (!out) {
        if (close_in) fclose(in);
        return 1;
    }
    /* CX REVIEW: When input_path == output_path we truncate the source before
     * reading it. Should we guard against in-place conversion or use a temp
     * file to avoid data loss on error? */

    int status = to_unix ? convert_dos_to_unix(in, out)
                         : convert_unix_to_dos(in, out);

    if (close_in) fclose(in);
    if (close_out) fclose(out);

    if (status == 0) {
        if (to_unix) {
            universal_add_karma(1, "Harmonised endings to simplicity (dos2unix)");
        } else {
            universal_add_karma(1, "Prepared message for broader audience (unix2dos)");
        }
    }

    return status;
}

static int cmd_dos2unix(int argc, char** argv) {
    return line_endings_command(argc, argv, true);
}

static int cmd_unix2dos(int argc, char** argv) {
    return line_endings_command(argc, argv, false);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * WEEK 8 REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_week8_commands(void) {
    universal_shell_register_command(
        "strings", cmd_strings,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "print sequences of printable characters",
        "strings [-n MIN_LENGTH] [FILE ...]\n"
        "Extract printable character sequences (default length ≥ 4).");

    universal_shell_register_command(
        "od", cmd_od,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "dump files in octal format",
        "od [FILE ...]\n"
        "Display binary data as octal words.");

    universal_shell_register_command(
        "hexdump", cmd_hexdump,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "dump files in hexadecimal format",
        "hexdump [FILE ...]\n"
        "Display binary data with hex bytes and ASCII gutter.");

    universal_shell_register_command(
        "xxd", cmd_xxd,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "create a hex dump (vim style)",
        "xxd [FILE ...]\n"
        "Create a canonical hex dump with ASCII translation.");

    universal_shell_register_command(
        "iconv", cmd_iconv,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "convert file encoding",
        "iconv [-f from] [-t to] [-o output] [input]\n"
        "Convert character encoding using iconv.");

    universal_shell_register_command(
        "dos2unix", cmd_dos2unix,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "convert DOS line endings to UNIX",
        "dos2unix [-o output] [input]\n"
        "Convert files from CRLF to LF line endings.");

    universal_shell_register_command(
        "unix2dos", cmd_unix2dos,
        CMD_CAT_TEXT_PROC, OS_SUPPORT_ALL,
        "convert UNIX line endings to DOS",
        "unix2dos [-o output] [input]\n"
        "Convert files from LF to CRLF line endings.");
}
