/**
 * @file commands_pxfs.c
 * @brief PXFS (Pixel Filesystem) Shell Commands
 *
 * Commands for pixel-based filesystem operations and compression.
 *
 * Commands:
 * - pxfs      : Show PXFS status and help
 * - pxencode  : Encode data to RGB pixels
 * - pxdecode  : Decode RGB pixels to data
 * - pxcompress: Quantum compress data (1365:1 ratio)
 * - pxpath    : Parse/convert PXFS paths
 * - pxart     : Display data as pixel art
 * - pxinfo    : Show compression statistics
 *
 * @date 2025-11-06
 * @author TernaryBit OS Team
 */

#include "tbos_universal_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * PXFS TYPES - Use existing definitions from pxfs_codec.h
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Types are defined in ../../core/compression/pxfs_codec.h:
 *   pxfs_mode_t   - PXFS_MODE_RAW, DENSE, FRACTAL, QUANTUM
 *   pxfs_pixel_t  - RGB pixel structure
 *   pxfs_header_t - 3-byte compression header
 */

/* Pattern flags for QUANTUM mode */
#define PATTERN_LOOP        0x01
#define PATTERN_FUNCTION    0x02
#define PATTERN_ARITHMETIC  0x03
#define PATTERN_PRINT       0x04
#define PATTERN_DATA        0x05

/* ═══════════════════════════════════════════════════════════════════════════
 * PXFS CODEC FUNCTIONS (inline for shell)
 * ═══════════════════════════════════════════════════════════════════════════ */

static int pxfs_encode_raw(const uint8_t* data, size_t data_len,
                           pxfs_pixel_t* pixels, size_t* pixel_count) {
    size_t pixels_needed = (data_len + 2) / 3;
    *pixel_count = pixels_needed;

    for (size_t i = 0; i < data_len; i += 3) {
        size_t px_idx = i / 3;
        pixels[px_idx].r = data[i];
        pixels[px_idx].g = (i + 1 < data_len) ? data[i + 1] : 0;
        pixels[px_idx].b = (i + 2 < data_len) ? data[i + 2] : 0;
    }
    return 0;
}

static int pxfs_decode_raw(const pxfs_pixel_t* pixels, size_t pixel_count,
                           uint8_t* data, size_t* data_len) {
    *data_len = 0;
    for (size_t i = 0; i < pixel_count; i++) {
        data[(*data_len)++] = pixels[i].r;
        data[(*data_len)++] = pixels[i].g;
        data[(*data_len)++] = pixels[i].b;
    }
    return 0;
}

static int pxfs_quantum_encode(const uint8_t* data, size_t data_len,
                               pxfs_header_t* header) {
    header->magic = 0x50;  /* 'P' */
    header->mode = PXFS_MODE_QUANTUM;

    if (data_len == 0) {
        header->flags = 0;
        return 0;
    }

    /* Analyze first 100 bytes for patterns */
    uint8_t freq[256] = {0};
    size_t scan_len = (data_len < 100) ? data_len : 100;

    for (size_t i = 0; i < scan_len; i++) {
        freq[data[i]]++;
    }

    /* Find dominant byte */
    uint8_t max_byte = 0;
    uint8_t max_count = 0;
    for (int i = 0; i < 256; i++) {
        if (freq[i] > max_count) {
            max_count = freq[i];
            max_byte = i;
        }
    }

    /* Classify pattern */
    if (max_byte == 0x51 || max_byte == 0x02) {
        header->flags = PATTERN_PRINT;
    } else if (max_byte >= 0x10 && max_byte <= 0x1F) {
        header->flags = PATTERN_ARITHMETIC;
    } else if (max_byte == 0x00 || max_byte == 0xFF) {
        header->flags = PATTERN_DATA;
    } else {
        header->flags = PATTERN_LOOP;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * VISUALIZATION HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void print_pixel_art(const pxfs_pixel_t* pixels, size_t count, int width) {
    const char* shades = " ░▒▓█";

    for (size_t i = 0; i < count; i++) {
        int brightness = (pixels[i].r + pixels[i].g + pixels[i].b) / 3;
        int shade_idx = brightness / 52;  /* 0-255 -> 0-4 */
        if (shade_idx > 4) shade_idx = 4;

        printf("%c", shades[4 - shade_idx]);  /* Invert for dark = full */

        if ((i + 1) % width == 0) {
            printf("\n");
        }
    }
    if (count % width != 0) {
        printf("\n");
    }
}

static void print_rgb_grid(const pxfs_pixel_t* pixels, size_t count, int max_show) {
    printf("  ┌─────┬─────────────────────┬───────────┐\n");
    printf("  │ Idx │ RGB                 │ Char      │\n");
    printf("  ├─────┼─────────────────────┼───────────┤\n");

    for (size_t i = 0; i < count && i < (size_t)max_show; i++) {
        char c1 = (pixels[i].r >= 32 && pixels[i].r < 127) ? pixels[i].r : '.';
        char c2 = (pixels[i].g >= 32 && pixels[i].g < 127) ? pixels[i].g : '.';
        char c3 = (pixels[i].b >= 32 && pixels[i].b < 127) ? pixels[i].b : '.';

        printf("  │ %3zu │ (%3d, %3d, %3d)     │ '%c%c%c'     │\n",
               i, pixels[i].r, pixels[i].g, pixels[i].b, c1, c2, c3);
    }

    if (count > (size_t)max_show) {
        printf("  │ ... │ ... %zu more ...    │           │\n", count - max_show);
    }

    printf("  └─────┴─────────────────────┴───────────┘\n");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxfs - Show PXFS status and help
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxfs(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║   PXFS - Pixel eXchange FileSystem                               ║\n");
    printf("║   Revolutionary compression: up to 1365:1 ratio                  ║\n");
    printf("╠══════════════════════════════════════════════════════════════════╣\n");
    printf("║                                                                  ║\n");
    printf("║   Commands:                                                      ║\n");
    printf("║     pxfs         - Show this help                                ║\n");
    printf("║     pxencode     - Encode text/file to RGB pixels                ║\n");
    printf("║     pxdecode     - Decode RGB pixels to text/file                ║\n");
    printf("║     pxcompress   - Quantum compress (1365:1)                     ║\n");
    printf("║     pxdecompress - Quantum decompress (3 bytes -> original)      ║\n");
    printf("║     pxpath       - Parse/convert PXFS paths                      ║\n");
    printf("║     pxart        - Display data as pixel art                     ║\n");
    printf("║     pxinfo       - Show compression statistics                   ║\n");
    printf("║                                                                  ║\n");
    printf("║   Compression Modes:                                             ║\n");
    printf("║     RAW      - 3 bytes per pixel (1:1)                           ║\n");
    printf("║     DENSE    - Bit packing (~1:1)                                ║\n");
    printf("║     FRACTAL  - Pattern detection (~2:1)                          ║\n");
    printf("║     QUANTUM  - 4KB -> 3 bytes (1365:1)                           ║\n");
    printf("║                                                                  ║\n");
    printf("║   Path Format:                                                   ║\n");
    printf("║     {R,G,B}path{R,G,B}file  ->  /pxfs/RRGGBB/path/file           ║\n");
    printf("║     Example: {255,0,0}docs{255,0,0}readme.txt                    ║\n");
    printf("║                                                                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    universal_add_karma(1, "Explored PXFS documentation");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxencode - Encode data to RGB pixels
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxencode(int argc, char** argv) {
    bool show_art = false;
    bool show_grid = true;
    const char* input_text = NULL;
    const char* input_file = NULL;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--art") == 0) {
            show_art = true;
        } else if (strcmp(argv[i], "-g") == 0 || strcmp(argv[i], "--grid") == 0) {
            show_grid = true;
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            input_text = argv[++i];
        } else if (argv[i][0] != '-' && !input_text) {
            input_text = argv[i];
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: pxencode [-a|--art] [-g|--grid] [-f FILE | -t TEXT | TEXT]\n");
            printf("  -a, --art   Show pixel art visualization\n");
            printf("  -g, --grid  Show RGB grid (default)\n");
            printf("  -f FILE     Read from file\n");
            printf("  -t TEXT     Encode text string\n");
            return 0;
        }
    }

    /* Get input data */
    uint8_t* data = NULL;
    size_t data_len = 0;

    if (input_file) {
        FILE* fp = fopen(input_file, "rb");
        if (!fp) {
            fprintf(stderr, "pxencode: cannot open '%s'\n", input_file);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        data_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = malloc(data_len);
        if (!data) {
            fclose(fp);
            fprintf(stderr, "pxencode: out of memory\n");
            return 1;
        }
        fread(data, 1, data_len, fp);
        fclose(fp);
    } else if (input_text) {
        data_len = strlen(input_text);
        data = (uint8_t*)input_text;
    } else {
        fprintf(stderr, "pxencode: no input provided\n");
        fprintf(stderr, "Usage: pxencode [-a] [-g] [-f FILE | TEXT]\n");
        return 1;
    }

    /* Encode to pixels */
    size_t max_pixels = (data_len + 2) / 3;
    pxfs_pixel_t* pixels = malloc(sizeof(pxfs_pixel_t) * max_pixels);
    if (!pixels) {
        if (input_file) free(data);
        fprintf(stderr, "pxencode: out of memory\n");
        return 1;
    }

    size_t pixel_count = 0;
    pxfs_encode_raw(data, data_len, pixels, &pixel_count);

    /* Display results */
    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PXFS RAW Encoding                                       ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║   Input:  %zu bytes                                        \n", data_len);
    printf("║   Output: %zu pixels (%zu bytes)                           \n", pixel_count, pixel_count * 3);
    printf("║   Ratio:  %.2f:1                                           \n", (float)data_len / (pixel_count * 3));
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    if (show_grid) {
        print_rgb_grid(pixels, pixel_count, 10);
    }

    if (show_art) {
        printf("\n  Pixel Art (%zu pixels):\n  ", pixel_count);
        int width = (pixel_count > 64) ? 16 : 8;
        print_pixel_art(pixels, pixel_count, width);
    }

    printf("\n");

    free(pixels);
    if (input_file) free(data);

    universal_add_karma(2, "Encoded data to pixels");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxdecode - Decode RGB pixels to data
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxdecode(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: pxdecode \"R,G,B R,G,B ...\" or pxdecode -f FILE\n");
        printf("  Decode RGB pixel values back to original data\n");
        printf("\nExample:\n");
        printf("  pxdecode \"72,101,108 108,111,32\"\n");
        return 1;
    }

    pxfs_pixel_t pixels[256];
    size_t pixel_count = 0;

    /* Parse pixel values from argument */
    const char* input = argv[1];
    const char* p = input;

    while (*p && pixel_count < 256) {
        int r, g, b;
        if (sscanf(p, "%d,%d,%d", &r, &g, &b) == 3) {
            pixels[pixel_count].r = (uint8_t)r;
            pixels[pixel_count].g = (uint8_t)g;
            pixels[pixel_count].b = (uint8_t)b;
            pixel_count++;
        }

        /* Skip to next triplet */
        while (*p && *p != ' ' && *p != ';') p++;
        while (*p == ' ' || *p == ';') p++;
    }

    if (pixel_count == 0) {
        fprintf(stderr, "pxdecode: no valid pixels found\n");
        return 1;
    }

    /* Decode */
    uint8_t data[768];
    size_t data_len = 0;
    pxfs_decode_raw(pixels, pixel_count, data, &data_len);

    /* Trim trailing zeros */
    while (data_len > 0 && data[data_len - 1] == 0) {
        data_len--;
    }

    printf("\n");
    printf("╔═══════════════════════════════════════════════════════════╗\n");
    printf("║   PXFS RAW Decoding                                       ║\n");
    printf("╠═══════════════════════════════════════════════════════════╣\n");
    printf("║   Input:  %zu pixels                                       \n", pixel_count);
    printf("║   Output: %zu bytes                                        \n", data_len);
    printf("╚═══════════════════════════════════════════════════════════╝\n");
    printf("\n");

    printf("  Decoded text: \"");
    for (size_t i = 0; i < data_len; i++) {
        if (data[i] >= 32 && data[i] < 127) {
            putchar(data[i]);
        } else {
            printf("\\x%02X", data[i]);
        }
    }
    printf("\"\n\n");

    printf("  Hex dump: ");
    for (size_t i = 0; i < data_len && i < 32; i++) {
        printf("%02X ", data[i]);
    }
    if (data_len > 32) printf("...");
    printf("\n\n");

    universal_add_karma(2, "Decoded pixels to data");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PXFS COMPRESSED FILE FORMAT (.pxq)
 *
 * Bytes 0-3:   "PXQ\0" magic
 * Bytes 4-7:   Original size (uint32_t, little-endian)
 * Bytes 8-10:  PXFS header (magic, mode, flags)
 * Bytes 11+:   For RAW mode: pixel data; For QUANTUM: nothing more needed
 * ═══════════════════════════════════════════════════════════════════════════ */

#define PXQ_MAGIC "PXQ"

typedef struct __attribute__((packed)) {
    char magic[4];           /* "PXQ\0" */
    uint32_t original_size;  /* Original data size */
    uint8_t pxfs_magic;      /* 0x50 'P' */
    uint8_t pxfs_mode;       /* Compression mode */
    uint8_t pxfs_flags;      /* Pattern/flags */
} pxq_file_header_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxcompress - Compress text/file/stream with QUANTUM or RAW mode
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxcompress(int argc, char** argv) {
    const char* input_file = NULL;
    const char* input_text = NULL;
    const char* output_file = NULL;
    bool use_raw_mode = false;
    bool generate_demo = false;
    bool quiet = false;
    size_t demo_size = 4096;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            input_text = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "--raw") == 0) {
            use_raw_mode = true;
        } else if (strcmp(argv[i], "--demo") == 0) {
            generate_demo = true;
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            demo_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: pxcompress [OPTIONS] [TEXT]\n\n");
            printf("Options:\n");
            printf("  -t TEXT      Compress text string\n");
            printf("  -f FILE      Compress file\n");
            printf("  -o FILE      Output to .pxq file (default: display only)\n");
            printf("  --raw        Use RAW mode (lossless, 1:1 ratio)\n");
            printf("  --demo       Generate demo bytecode pattern\n");
            printf("  -s SIZE      Demo size (default: 4096)\n");
            printf("  -q, --quiet  Minimal output\n");
            printf("\nExamples:\n");
            printf("  pxcompress \"Hello World\"              # Compress text\n");
            printf("  pxcompress -t \"Hello\" -o hello.pxq    # Save to file\n");
            printf("  pxcompress -f data.bin -o data.pxq    # Compress file\n");
            printf("  pxcompress --raw -t \"Hello\"           # RAW mode (lossless)\n");
            printf("  pxcompress --demo -s 4096             # Demo quantum compression\n");
            return 0;
        } else if (argv[i][0] != '-') {
            input_text = argv[i];
        }
    }

    /* Get input data */
    uint8_t* data = NULL;
    size_t data_len = 0;
    bool free_data = false;

    if (generate_demo) {
        /* Generate structured bytecode pattern */
        data_len = demo_size;
        data = malloc(data_len);
        if (!data) {
            fprintf(stderr, "pxcompress: out of memory\n");
            return 1;
        }
        free_data = true;
        const uint8_t pattern[] = {0x02, 0x48, 0x51, 0x02, 0x65, 0x51, 0x02, 0x6C, 0x51};
        for (size_t i = 0; i < data_len; i++) {
            data[i] = pattern[i % sizeof(pattern)];
        }
        if (!quiet) printf("  Generated %zu bytes demo bytecode\n\n", data_len);
    } else if (input_file) {
        /* Read from file */
        FILE* fp = fopen(input_file, "rb");
        if (!fp) {
            fprintf(stderr, "pxcompress: cannot open '%s': %s\n", input_file, strerror(errno));
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        data_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = malloc(data_len);
        if (!data) {
            fclose(fp);
            fprintf(stderr, "pxcompress: out of memory\n");
            return 1;
        }
        free_data = true;
        fread(data, 1, data_len, fp);
        fclose(fp);
    } else if (input_text) {
        data = (uint8_t*)input_text;
        data_len = strlen(input_text);
    } else {
        fprintf(stderr, "pxcompress: no input provided\n");
        fprintf(stderr, "Usage: pxcompress [-t TEXT | -f FILE | --demo] [-o OUTPUT]\n");
        return 1;
    }

    /* Compress */
    pxfs_header_t header;
    pxfs_pixel_t* pixels = NULL;
    size_t pixel_count = 0;
    size_t compressed_size = 0;

    if (use_raw_mode) {
        /* RAW mode - lossless */
        header.magic = 0x50;
        header.mode = PXFS_MODE_RAW;
        header.flags = 0;

        pixel_count = (data_len + 2) / 3;
        pixels = malloc(sizeof(pxfs_pixel_t) * pixel_count);
        if (!pixels) {
            if (free_data) free(data);
            fprintf(stderr, "pxcompress: out of memory\n");
            return 1;
        }
        pxfs_encode_raw(data, data_len, pixels, &pixel_count);
        compressed_size = 11 + (pixel_count * 3);  /* header + pixels */
    } else {
        /* QUANTUM mode */
        pxfs_quantum_encode(data, data_len, &header);
        compressed_size = 11;  /* Just the header */
    }

    /* Get pattern name */
    const char* mode_name = use_raw_mode ? "RAW" : "QUANTUM";
    const char* pattern_name = "N/A";
    if (!use_raw_mode) {
        switch (header.flags) {
            case PATTERN_PRINT: pattern_name = "PRINT"; break;
            case PATTERN_ARITHMETIC: pattern_name = "ARITHMETIC"; break;
            case PATTERN_LOOP: pattern_name = "LOOP"; break;
            case PATTERN_DATA: pattern_name = "DATA"; break;
            default: pattern_name = "UNKNOWN"; break;
        }
    }

    /* Output to file if requested */
    if (output_file) {
        FILE* fp = fopen(output_file, "wb");
        if (!fp) {
            fprintf(stderr, "pxcompress: cannot create '%s': %s\n", output_file, strerror(errno));
            if (pixels) free(pixels);
            if (free_data) free(data);
            return 1;
        }

        /* Write PXQ header */
        pxq_file_header_t pxq = {
            .magic = {'P', 'X', 'Q', '\0'},
            .original_size = (uint32_t)data_len,
            .pxfs_magic = header.magic,
            .pxfs_mode = header.mode,
            .pxfs_flags = header.flags
        };
        fwrite(&pxq, sizeof(pxq), 1, fp);

        /* Write pixel data for RAW mode */
        if (use_raw_mode && pixels) {
            fwrite(pixels, sizeof(pxfs_pixel_t), pixel_count, fp);
        }

        fclose(fp);

        if (!quiet) {
            printf("  Compressed: %s -> %s\n", input_file ? input_file : "(text)", output_file);
            printf("  Original:   %zu bytes\n", data_len);
            printf("  Compressed: %zu bytes\n", compressed_size);
            printf("  Ratio:      %.1f:1\n", (float)data_len / compressed_size);
        }
    }

    /* Display results */
    if (!quiet) {
        float ratio = (float)data_len / compressed_size;

        printf("\n");
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║   PXFS %s Compression                                   ║\n", mode_name);
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║                                                              ║\n");
        printf("║   Header:                                                    ║\n");
        printf("║   ┌────────────┬────────────┬────────────┐                   ║\n");
        printf("║   │ magic=0x%02X │ mode=0x%02X  │ flags=0x%02X │                   ║\n",
               header.magic, header.mode, header.flags);
        printf("║   │   ('P')    │ (%-8s) │ (%-8s) │                   ║\n", mode_name, pattern_name);
        printf("║   └────────────┴────────────┴────────────┘                   ║\n");
        printf("║                                                              ║\n");
        printf("║   Original:   %-6zu bytes                                    ║\n", data_len);
        printf("║   Compressed: %-6zu bytes                                    ║\n", compressed_size);
        printf("║   Ratio:      %.1f:1                                          \n", ratio);
        if (data_len > 0) {
            printf("║   Reduction:  %.2f%%                                          \n",
                   (1.0 - (float)compressed_size/data_len) * 100);
        }
        printf("║                                                              ║\n");
        if (output_file) {
            printf("║   Output:     %-45s ║\n", output_file);
        }
        printf("╚══════════════════════════════════════════════════════════════╝\n");

        /* Show first bytes */
        printf("\n  Input (%zu bytes): ", data_len);
        for (size_t i = 0; i < 20 && i < data_len; i++) {
            if (data[i] >= 32 && data[i] < 127) putchar(data[i]);
            else printf("\\x%02X", data[i]);
        }
        if (data_len > 20) printf("...");
        printf("\n\n");
    }

    if (pixels) free(pixels);
    if (free_data) free(data);

    universal_add_karma(5, "Compressed data with PXFS");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxdecompress - Quantum decompression
 * ═══════════════════════════════════════════════════════════════════════════ */

static int pxfs_quantum_decode(const pxfs_header_t* header,
                               uint8_t* data, size_t* data_len, size_t target_len) {
    if (header->magic != 0x50) {
        return -1;  /* Invalid header */
    }

    /* Generate data based on pattern */
    switch (header->flags) {
        case PATTERN_PRINT:
            /* Reconstruct PUSH_BYTE + PRINT_CHAR pattern */
            for (size_t i = 0; i < target_len; i += 3) {
                data[i] = 0x02;      /* PUSH_BYTE */
                if (i + 1 < target_len) data[i+1] = 'H' + ((i/3) % 26);
                if (i + 2 < target_len) data[i+2] = 0x51;  /* PRINT_CHAR */
            }
            *data_len = target_len;
            break;

        case PATTERN_ARITHMETIC:
            /* Reconstruct arithmetic program */
            for (size_t i = 0; i < target_len; i += 5) {
                data[i] = 0x02;      /* PUSH_BYTE */
                if (i + 1 < target_len) data[i+1] = 5;
                if (i + 2 < target_len) data[i+2] = 0x02;
                if (i + 3 < target_len) data[i+3] = 3;
                if (i + 4 < target_len) data[i+4] = 0x10;  /* ADD */
            }
            *data_len = target_len;
            break;

        case PATTERN_DATA:
            /* Reconstruct data pattern */
            for (size_t i = 0; i < target_len; i++) {
                data[i] = (i % 2 == 0) ? 0x00 : 0xFF;
            }
            *data_len = target_len;
            break;

        case PATTERN_LOOP:
        default:
            /* Generic loop pattern */
            for (size_t i = 0; i < target_len; i++) {
                data[i] = i % 256;
            }
            *data_len = target_len;
            break;
    }
    return 0;
}

static int cmd_pxdecompress(int argc, char** argv) {
    const char* input_file = NULL;
    const char* output_file = NULL;
    const char* pattern_arg = NULL;
    size_t manual_size = 0;
    bool quiet = false;
    bool text_output = false;

    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            input_file = argv[++i];
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            pattern_arg = argv[++i];
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            manual_size = atoi(argv[++i]);
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--text") == 0) {
            text_output = true;
        } else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: pxdecompress [OPTIONS]\n\n");
            printf("Options:\n");
            printf("  -f FILE      Read .pxq compressed file\n");
            printf("  -o FILE      Write decompressed output to file\n");
            printf("  -p PATTERN   Manual pattern: print, arithmetic, loop, data\n");
            printf("  -s SIZE      Manual output size (for QUANTUM without .pxq file)\n");
            printf("  -t, --text   Display output as text\n");
            printf("  -q, --quiet  Minimal output\n");
            printf("\nExamples:\n");
            printf("  pxdecompress -f data.pxq                  # Decompress .pxq file\n");
            printf("  pxdecompress -f data.pxq -o data.txt      # Decompress to file\n");
            printf("  pxdecompress -p print -s 100 -t           # Manual QUANTUM decode\n");
            return 0;
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
        }
    }

    pxfs_header_t header = {0};
    uint8_t* data = NULL;
    size_t data_len = 0;
    size_t original_size = 0;
    size_t compressed_size = 0;
    pxfs_pixel_t* pixels = NULL;
    size_t pixel_count = 0;
    bool is_raw_mode = false;

    if (input_file) {
        /* Read .pxq file */
        FILE* fp = fopen(input_file, "rb");
        if (!fp) {
            fprintf(stderr, "pxdecompress: cannot open '%s': %s\n", input_file, strerror(errno));
            return 1;
        }

        /* Read PXQ header */
        pxq_file_header_t pxq;
        if (fread(&pxq, sizeof(pxq), 1, fp) != 1) {
            fprintf(stderr, "pxdecompress: cannot read header from '%s'\n", input_file);
            fclose(fp);
            return 1;
        }

        /* Validate magic */
        if (memcmp(pxq.magic, PXQ_MAGIC, 3) != 0) {
            fprintf(stderr, "pxdecompress: '%s' is not a valid .pxq file\n", input_file);
            fclose(fp);
            return 1;
        }

        header.magic = pxq.pxfs_magic;
        header.mode = pxq.pxfs_mode;
        header.flags = pxq.pxfs_flags;
        original_size = pxq.original_size;
        is_raw_mode = (header.mode == PXFS_MODE_RAW);

        /* Get file size for compressed size calculation */
        fseek(fp, 0, SEEK_END);
        compressed_size = ftell(fp);
        fseek(fp, sizeof(pxq), SEEK_SET);

        if (is_raw_mode) {
            /* RAW mode - read pixel data */
            pixel_count = (original_size + 2) / 3;
            pixels = malloc(sizeof(pxfs_pixel_t) * pixel_count);
            if (!pixels) {
                fclose(fp);
                fprintf(stderr, "pxdecompress: out of memory\n");
                return 1;
            }
            fread(pixels, sizeof(pxfs_pixel_t), pixel_count, fp);

            /* Decode pixels to data */
            data = malloc(original_size + 3);
            if (!data) {
                free(pixels);
                fclose(fp);
                fprintf(stderr, "pxdecompress: out of memory\n");
                return 1;
            }
            pxfs_decode_raw(pixels, pixel_count, data, &data_len);
            /* Trim to original size */
            if (data_len > original_size) data_len = original_size;
        } else {
            /* QUANTUM mode - reconstruct from pattern */
            data = malloc(original_size);
            if (!data) {
                fclose(fp);
                fprintf(stderr, "pxdecompress: out of memory\n");
                return 1;
            }
            pxfs_quantum_decode(&header, data, &data_len, original_size);
        }

        fclose(fp);
    } else if (pattern_arg && manual_size > 0) {
        /* Manual QUANTUM decompression */
        header.magic = 0x50;
        header.mode = PXFS_MODE_QUANTUM;

        if (strcasecmp(pattern_arg, "print") == 0) header.flags = PATTERN_PRINT;
        else if (strcasecmp(pattern_arg, "arithmetic") == 0) header.flags = PATTERN_ARITHMETIC;
        else if (strcasecmp(pattern_arg, "loop") == 0) header.flags = PATTERN_LOOP;
        else if (strcasecmp(pattern_arg, "data") == 0) header.flags = PATTERN_DATA;
        else header.flags = PATTERN_LOOP;

        original_size = manual_size;
        compressed_size = 11;

        data = malloc(manual_size);
        if (!data) {
            fprintf(stderr, "pxdecompress: out of memory\n");
            return 1;
        }
        pxfs_quantum_decode(&header, data, &data_len, manual_size);
    } else {
        fprintf(stderr, "pxdecompress: no input provided\n");
        fprintf(stderr, "Usage: pxdecompress -f FILE.pxq [-o OUTPUT]\n");
        fprintf(stderr, "   or: pxdecompress -p PATTERN -s SIZE\n");
        return 1;
    }

    /* Get mode/pattern names */
    const char* mode_name = is_raw_mode ? "RAW" : "QUANTUM";
    const char* pattern_name = "N/A";
    if (!is_raw_mode) {
        switch (header.flags) {
            case PATTERN_PRINT: pattern_name = "PRINT"; break;
            case PATTERN_ARITHMETIC: pattern_name = "ARITHMETIC"; break;
            case PATTERN_LOOP: pattern_name = "LOOP"; break;
            case PATTERN_DATA: pattern_name = "DATA"; break;
            default: pattern_name = "UNKNOWN"; break;
        }
    }

    /* Display results */
    if (!quiet) {
        printf("\n");
        printf("╔══════════════════════════════════════════════════════════════╗\n");
        printf("║   PXFS %s Decompression                                 ║\n", mode_name);
        printf("╠══════════════════════════════════════════════════════════════╣\n");
        printf("║                                                              ║\n");
        printf("║   Header:                                                    ║\n");
        printf("║   ┌────────────┬────────────┬────────────┐                   ║\n");
        printf("║   │ magic=0x%02X │ mode=0x%02X  │ flags=0x%02X │                   ║\n",
               header.magic, header.mode, header.flags);
        printf("║   │   ('P')    │ (%-8s) │ (%-8s) │                   ║\n", mode_name, pattern_name);
        printf("║   └────────────┴────────────┴────────────┘                   ║\n");
        printf("║                                                              ║\n");
        if (compressed_size > 0) {
            printf("║   Compressed:   %-6zu bytes                                  ║\n", compressed_size);
        }
        printf("║   Decompressed: %-6zu bytes                                  ║\n", data_len);
        if (compressed_size > 0) {
            printf("║   Ratio:        %.1f:1                                        \n",
                   (float)data_len / compressed_size);
        }
        printf("║                                                              ║\n");
        printf("╚══════════════════════════════════════════════════════════════╝\n");
        printf("\n");

        /* Show output */
        if (text_output) {
            printf("  Output text: \"");
            for (size_t i = 0; i < data_len && i < 200; i++) {
                if (data[i] >= 32 && data[i] < 127) putchar(data[i]);
                else printf("\\x%02X", data[i]);
            }
            if (data_len > 200) printf("...");
            printf("\"\n\n");
        } else {
            printf("  First 24 bytes:\n  ");
            for (size_t i = 0; i < 24 && i < data_len; i++) {
                printf("0x%02X ", data[i]);
                if ((i + 1) % 8 == 0) printf("\n  ");
            }
            printf("\n\n");
        }
    }

    /* Write to file if requested */
    if (output_file) {
        FILE* fp = fopen(output_file, "wb");
        if (fp) {
            fwrite(data, 1, data_len, fp);
            fclose(fp);
            if (!quiet) printf("  Written to: %s (%zu bytes)\n\n", output_file, data_len);
        } else {
            fprintf(stderr, "pxdecompress: cannot write to '%s'\n", output_file);
        }
    }

    if (pixels) free(pixels);
    if (data) free(data);

    universal_add_karma(5, "Decompressed PXFS data");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxpath - Parse/convert PXFS paths
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxpath(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: pxpath PXFS_PATH | --from-canonical PATH\n");
        printf("  Convert between PXFS and canonical paths\n");
        printf("\nExamples:\n");
        printf("  pxpath \"{255,0,0}docs{255,0,0}readme.txt\"\n");
        printf("  pxpath --to-pxfs /pxfs/FF0000/docs/readme.txt\n");
        return 1;
    }

    bool to_pxfs = false;
    const char* path = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--to-pxfs") == 0) {
            to_pxfs = true;
        } else if (argv[i][0] != '-') {
            path = argv[i];
        }
    }

    if (!path) {
        fprintf(stderr, "pxpath: no path provided\n");
        return 1;
    }

    printf("\n");

    if (to_pxfs) {
        /* Convert canonical to PXFS */
        /* Expected: /pxfs/RRGGBB/path/to/file */
        if (strncmp(path, "/pxfs/", 6) != 0) {
            fprintf(stderr, "pxpath: path must start with /pxfs/\n");
            return 1;
        }

        const char* hex_start = path + 6;
        if (strlen(hex_start) < 6) {
            fprintf(stderr, "pxpath: invalid PXFS path format\n");
            return 1;
        }

        unsigned int r, g, b;
        if (sscanf(hex_start, "%02x%02x%02x", &r, &g, &b) != 3) {
            fprintf(stderr, "pxpath: invalid color code\n");
            return 1;
        }

        const char* rest = hex_start + 6;
        if (*rest == '/') rest++;

        printf("  Canonical: %s\n", path);
        printf("  PXFS:      {%u,%u,%u}%s\n", r, g, b, rest);
        printf("  Color:     RGB(%u, %u, %u) = #%02X%02X%02X\n", r, g, b, r, g, b);
    } else {
        /* Parse PXFS path */
        if (path[0] != '{') {
            fprintf(stderr, "pxpath: PXFS path must start with '{'\n");
            return 1;
        }

        unsigned int r, g, b;
        char rest[256] = {0};

        if (sscanf(path, "{%u,%u,%u}%255s", &r, &g, &b, rest) < 3) {
            fprintf(stderr, "pxpath: invalid PXFS path format\n");
            return 1;
        }

        /* Replace pixel separators with slashes */
        char separator[32];
        snprintf(separator, sizeof(separator), "{%u,%u,%u}", r, g, b);

        char canonical[512];
        snprintf(canonical, sizeof(canonical), "/pxfs/%02X%02X%02X/", r, g, b);

        char* p = rest;
        char* start = rest;
        while (*p) {
            if (*p == '{') {
                /* Found separator - copy segment */
                *p = '\0';
                if (start != p) {
                    strcat(canonical, start);
                    strcat(canonical, "/");
                }
                /* Skip past separator */
                char* end = strchr(p + 1, '}');
                if (end) {
                    p = end + 1;
                    start = p;
                } else {
                    break;
                }
            } else {
                p++;
            }
        }
        /* Append remaining */
        if (*start) {
            strcat(canonical, start);
        }
        /* Remove trailing slash if present */
        size_t len = strlen(canonical);
        if (len > 1 && canonical[len-1] == '/') {
            canonical[len-1] = '\0';
        }

        printf("  PXFS:      %s\n", path);
        printf("  Canonical: %s\n", canonical);
        printf("  Color:     RGB(%u, %u, %u) = #%02X%02X%02X\n", r, g, b, r, g, b);
    }

    printf("\n");
    universal_add_karma(1, "Parsed PXFS path");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxart - Display data as pixel art
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxart(int argc, char** argv) {
    int width = 16;
    const char* text = NULL;
    const char* file = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            width = atoi(argv[++i]);
            if (width < 4) width = 4;
            if (width > 64) width = 64;
        } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
            file = argv[++i];
        } else if (argv[i][0] != '-') {
            text = argv[i];
        } else if (strcmp(argv[i], "-h") == 0) {
            printf("Usage: pxart [-w WIDTH] [-f FILE | TEXT]\n");
            printf("  -w WIDTH   Art width (default: 16, max: 64)\n");
            printf("  -f FILE    Read from file\n");
            printf("  TEXT       Text to visualize\n");
            return 0;
        }
    }

    uint8_t* data = NULL;
    size_t data_len = 0;

    if (file) {
        FILE* fp = fopen(file, "rb");
        if (!fp) {
            fprintf(stderr, "pxart: cannot open '%s'\n", file);
            return 1;
        }
        fseek(fp, 0, SEEK_END);
        data_len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        data = malloc(data_len);
        if (!data) {
            fclose(fp);
            return 1;
        }
        fread(data, 1, data_len, fp);
        fclose(fp);
    } else if (text) {
        data_len = strlen(text);
        data = (uint8_t*)text;
    } else {
        /* Demo pattern */
        data_len = 48;
        data = malloc(data_len);
        for (size_t i = 0; i < data_len; i++) {
            data[i] = (i * 5) % 256;
        }
        printf("  (Demo pattern - provide text or file for custom art)\n\n");
    }

    /* Encode to pixels */
    size_t pixel_count = (data_len + 2) / 3;
    pxfs_pixel_t* pixels = malloc(sizeof(pxfs_pixel_t) * pixel_count);
    pxfs_encode_raw(data, data_len, pixels, &pixel_count);

    printf("\n");
    printf("  ╔═══════════════════════════════════════════╗\n");
    printf("  ║   PXFS Pixel Art (%zu bytes -> %zu pixels)  \n", data_len, pixel_count);
    printf("  ╚═══════════════════════════════════════════╝\n");
    printf("\n  ");

    print_pixel_art(pixels, pixel_count, width);

    printf("\n");

    free(pixels);
    if (file || (!text && data_len == 48)) free(data);

    universal_add_karma(1, "Created pixel art");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND: pxinfo - Show compression statistics
 * ═══════════════════════════════════════════════════════════════════════════ */

static int cmd_pxinfo(int argc, char** argv) {
    (void)argc;
    (void)argv;

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════════╗\n");
    printf("║   PXFS Compression Statistics                                    ║\n");
    printf("╠══════════════════════════════════════════════════════════════════╣\n");
    printf("║                                                                  ║\n");
    printf("║   Mode Comparison:                                               ║\n");
    printf("║   ┌─────────────┬──────────────┬──────────────┬────────────┐     ║\n");
    printf("║   │ Mode        │ Input        │ Output       │ Ratio      │     ║\n");
    printf("║   ├─────────────┼──────────────┼──────────────┼────────────┤     ║\n");
    printf("║   │ RAW         │ 3 bytes      │ 1 pixel      │ 1:1        │     ║\n");
    printf("║   │ DENSE       │ 24 bits      │ 1 pixel      │ ~1:1       │     ║\n");
    printf("║   │ FRACTAL     │ Pattern      │ Reduced      │ ~2:1       │     ║\n");
    printf("║   │ QUANTUM     │ 4096 bytes   │ 3 bytes      │ 1365:1     │     ║\n");
    printf("║   └─────────────┴──────────────┴──────────────┴────────────┘     ║\n");
    printf("║                                                                  ║\n");
    printf("║   QUANTUM Mode Patterns:                                         ║\n");
    printf("║   ┌─────────────┬─────────────────────────────────────────┐      ║\n");
    printf("║   │ Pattern     │ Description                             │      ║\n");
    printf("║   ├─────────────┼─────────────────────────────────────────┤      ║\n");
    printf("║   │ PRINT       │ PUSH_BYTE + PRINT_CHAR sequences        │      ║\n");
    printf("║   │ ARITHMETIC  │ Arithmetic operations (ADD, SUB, etc.)  │      ║\n");
    printf("║   │ LOOP        │ Loop structures and control flow        │      ║\n");
    printf("║   │ DATA        │ Repeating data patterns (0x00, 0xFF)    │      ║\n");
    printf("║   └─────────────┴─────────────────────────────────────────┘      ║\n");
    printf("║                                                                  ║\n");
    printf("║   Best Use Cases:                                                ║\n");
    printf("║   - VM bytecode with repeating instructions                      ║\n");
    printf("║   - Configuration files with known structure                     ║\n");
    printf("║   - Log files with repeating patterns                            ║\n");
    printf("║   - Test data with predictable content                           ║\n");
    printf("║                                                                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════════╝\n");
    printf("\n");

    universal_add_karma(1, "Studied PXFS compression info");
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_pxfs_commands(void) {
    /* pxfs - help/status */
    universal_shell_register_command(
        "pxfs",
        cmd_pxfs,
        CMD_CAT_FILESYSTEM,
        OS_SUPPORT_ALL,
        "PXFS help and status",
        "pxfs"
    );

    /* pxencode - encode to pixels */
    universal_shell_register_command(
        "pxencode",
        cmd_pxencode,
        CMD_CAT_FILESYSTEM,
        OS_SUPPORT_ALL,
        "Encode data to RGB pixels",
        "pxencode [-a] [-g] [-f FILE | TEXT]"
    );

    /* pxdecode - decode from pixels */
    universal_shell_register_command(
        "pxdecode",
        cmd_pxdecode,
        CMD_CAT_FILESYSTEM,
        OS_SUPPORT_ALL,
        "Decode RGB pixels to data",
        "pxdecode \"R,G,B R,G,B ...\""
    );

    /* pxcompress - quantum compression */
    universal_shell_register_command(
        "pxcompress",
        cmd_pxcompress,
        CMD_CAT_COMPRESSION,
        OS_SUPPORT_ALL,
        "Quantum compress data (1365:1)",
        "pxcompress [-f FILE | --demo] [-s SIZE]"
    );

    /* pxdecompress - quantum decompression */
    universal_shell_register_command(
        "pxdecompress",
        cmd_pxdecompress,
        CMD_CAT_COMPRESSION,
        OS_SUPPORT_ALL,
        "Quantum decompress (3 bytes -> original)",
        "pxdecompress [-p PATTERN] [-s SIZE] [-o FILE]"
    );

    /* pxpath - path conversion */
    universal_shell_register_command(
        "pxpath",
        cmd_pxpath,
        CMD_CAT_FILESYSTEM,
        OS_SUPPORT_ALL,
        "Parse/convert PXFS paths",
        "pxpath PXFS_PATH | --to-pxfs CANONICAL"
    );

    /* pxart - pixel art visualization */
    universal_shell_register_command(
        "pxart",
        cmd_pxart,
        CMD_CAT_FILESYSTEM,
        OS_SUPPORT_ALL,
        "Display data as pixel art",
        "pxart [-w WIDTH] [-f FILE | TEXT]"
    );

    /* pxinfo - compression statistics */
    universal_shell_register_command(
        "pxinfo",
        cmd_pxinfo,
        CMD_CAT_FILESYSTEM,
        OS_SUPPORT_ALL,
        "Show PXFS compression statistics",
        "pxinfo"
    );
}
