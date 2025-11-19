#include "fs/ucfs_config.h"
#include "fs/ucfs_codec.h"
#include "tbos/errno.h"
#include "tbos/libc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void ucfs_config_init_defaults(ucfs_config_t* config) {
    if (!config) return;

    memset(config, 0, sizeof(*config));
    strcpy(config->default_backing, "/ucfs");
    config->delimiter_count = 0;

    /* Add some common default delimiters */
    ucfs_delimiter_config_t defaults[] = {
        {
            .delimiter = 0x1F549,  /* 🕉️ Om symbol */
            .delimiter_utf8 = "🕉️",
            .delimiter_len = 4,
            .backing_path = "/ucfs/U+1F549",
            .culture = "Sanskrit/Hindu",
            .description = "Om symbol - spiritual/sacred content",
            .active = 1
        },
        {
            .delimiter = 0x1F4C1,  /* 📁 Folder emoji */
            .delimiter_utf8 = "📁",
            .delimiter_len = 4,
            .backing_path = "/ucfs/U+1F4C1",
            .culture = "Universal",
            .description = "Folder emoji - general files",
            .active = 1
        },
        {
            .delimiter = 0x1F30D,  /* 🌍 Earth emoji */
            .delimiter_utf8 = "🌍",
            .delimiter_len = 4,
            .backing_path = "/ucfs/U+1F30D",
            .culture = "Universal",
            .description = "Earth emoji - global content",
            .active = 1
        },
        {
            .delimiter = 0x0950,   /* ॐ Devanagari Om */
            .delimiter_utf8 = "ॐ",
            .delimiter_len = 3,
            .backing_path = "/ucfs/U+0950",
            .culture = "Devanagari/Sanskrit",
            .description = "Devanagari Om - Sanskrit content",
            .active = 1
        }
    };

    for (size_t i = 0; i < sizeof(defaults) / sizeof(defaults[0]); i++) {
        if (config->delimiter_count < UCFS_MAX_DELIMITER_MAPPINGS) {
            config->delimiters[config->delimiter_count++] = defaults[i];
        }
    }
}

const ucfs_delimiter_config_t* ucfs_config_get_delimiter(
    const ucfs_config_t* config, uint32_t delimiter) {
    if (!config) return NULL;

    for (size_t i = 0; i < config->delimiter_count; i++) {
        if (config->delimiters[i].delimiter == delimiter &&
            config->delimiters[i].active) {
            return &config->delimiters[i];
        }
    }

    return NULL;
}

int ucfs_config_set_delimiter(ucfs_config_t* config,
                               const ucfs_delimiter_config_t* delim_config) {
    if (!config || !delim_config) return -EINVAL;

    /* Check if delimiter already exists */
    for (size_t i = 0; i < config->delimiter_count; i++) {
        if (config->delimiters[i].delimiter == delim_config->delimiter) {
            /* Update existing */
            config->delimiters[i] = *delim_config;
            return 0;
        }
    }

    /* Add new delimiter */
    if (config->delimiter_count >= UCFS_MAX_DELIMITER_MAPPINGS) {
        return -ENOSPC;
    }

    config->delimiters[config->delimiter_count++] = *delim_config;
    return 0;
}

int ucfs_config_load(const char* config_path, ucfs_config_t* config) {
    if (!config_path || !config) return -EINVAL;

    /* Initialize with defaults first */
    ucfs_config_init_defaults(config);

    FILE* fp = fopen(config_path, "r");
    if (!fp) {
        /* File doesn't exist, use defaults */
        return 0;
    }

    char line[512];
    int line_num = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        /* Remove trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        /* Skip empty lines and comments */
        if (len == 0 || line[0] == '#') continue;

        /* Parse configuration line */
        if (strncmp(line, "default_backing=", 16) == 0) {
            strncpy(config->default_backing, line + 16,
                    sizeof(config->default_backing) - 1);
        }
        else if (strncmp(line, "delimiter=", 10) == 0) {
            /* Parse delimiter configuration */
            /* Format: delimiter=CODEPOINT,backing_path,culture,description */
            char* start = line + 10;
            char* comma1 = strchr(start, ',');
            if (!comma1) continue;

            *comma1 = '\0';
            uint32_t codepoint = (uint32_t)strtoul(start, NULL, 16);

            char* comma2 = strchr(comma1 + 1, ',');
            if (!comma2) continue;
            *comma2 = '\0';

            char* comma3 = strchr(comma2 + 1, ',');
            if (!comma3) continue;
            *comma3 = '\0';

            ucfs_delimiter_config_t delim;
            memset(&delim, 0, sizeof(delim));
            delim.delimiter = codepoint;
            delim.active = 1;

            /* Encode codepoint to UTF-8 */
            if (codepoint <= 0x7F) {
                delim.delimiter_utf8[0] = (char)codepoint;
                delim.delimiter_len = 1;
            } else if (codepoint <= 0x7FF) {
                delim.delimiter_utf8[0] = (char)(0xC0 | ((codepoint >> 6) & 0x1F));
                delim.delimiter_utf8[1] = (char)(0x80 | (codepoint & 0x3F));
                delim.delimiter_len = 2;
            } else if (codepoint <= 0xFFFF) {
                delim.delimiter_utf8[0] = (char)(0xE0 | ((codepoint >> 12) & 0x0F));
                delim.delimiter_utf8[1] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                delim.delimiter_utf8[2] = (char)(0x80 | (codepoint & 0x3F));
                delim.delimiter_len = 3;
            } else {
                delim.delimiter_utf8[0] = (char)(0xF0 | ((codepoint >> 18) & 0x07));
                delim.delimiter_utf8[1] = (char)(0x80 | ((codepoint >> 12) & 0x3F));
                delim.delimiter_utf8[2] = (char)(0x80 | ((codepoint >> 6) & 0x3F));
                delim.delimiter_utf8[3] = (char)(0x80 | (codepoint & 0x3F));
                delim.delimiter_len = 4;
            }

            strncpy(delim.backing_path, comma1 + 1, sizeof(delim.backing_path) - 1);
            strncpy(delim.culture, comma2 + 1, sizeof(delim.culture) - 1);
            strncpy(delim.description, comma3 + 1, sizeof(delim.description) - 1);

            ucfs_config_set_delimiter(config, &delim);
        }
    }

    fclose(fp);
    return 0;
}

int ucfs_config_save(const char* config_path, const ucfs_config_t* config) {
    if (!config_path || !config) return -EINVAL;

    FILE* fp = fopen(config_path, "w");
    if (!fp) return -errno;

    fprintf(fp, "# UCFS Configuration File\n");
    fprintf(fp, "# Generated by TernaryBit OS\n\n");

    fprintf(fp, "# Default backing storage path\n");
    fprintf(fp, "default_backing=%s\n\n", config->default_backing);

    fprintf(fp, "# Delimiter mappings\n");
    fprintf(fp, "# Format: delimiter=CODEPOINT,backing_path,culture,description\n\n");

    for (size_t i = 0; i < config->delimiter_count; i++) {
        const ucfs_delimiter_config_t* d = &config->delimiters[i];
        if (d->active) {
            fprintf(fp, "delimiter=%04X,%s,%s,%s\n",
                    d->delimiter,
                    d->backing_path,
                    d->culture,
                    d->description);
        }
    }

    fclose(fp);
    return 0;
}
