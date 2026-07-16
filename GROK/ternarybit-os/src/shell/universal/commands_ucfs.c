/**
 * @file commands_ucfs.c
 * @brief UCFS (Unicode Character File System) and RF2S (Radio Frequency File System) Commands
 *
 * UCFS: Uses Unicode characters as path roots
 *   Example: 🏠/documents/file.txt -> /ucfs/U+1F3E0/documents/file.txt
 *
 * RF2S: Uses radio frequencies as path roots
 *   Example: /432MHz/sensor/data -> /rf2s/432000000/sensor/data
 */

#include "tbos_universal_shell.h"
#include "fs/ucfs_codec.h"
#include "fs/rf2s_codec.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * UCFS COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief ucfs - Unicode Character File System operations
 */
static int cmd_ucfs(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ucfs <command> [args]\n");
        printf("\n");
        printf("Commands:\n");
        printf("  parse <path>     Parse a UCFS path (emoji-based)\n");
        printf("  canon <path>     Convert to canonical path\n");
        printf("  info             Show UCFS information\n");
        printf("  roots            List common Unicode roots\n");
        printf("\n");
        printf("UCFS uses Unicode characters as path delimiters:\n");
        printf("  \xF0\x9F\x8F\xA0/docs/file.txt  -> /ucfs/U+1F3E0/docs/file.txt\n");
        printf("  \xE2\x98\x80/weather/today    -> /ucfs/U+2600/weather/today\n");
        return 1;
    }

    const char* subcmd = argv[1];

    if (strcmp(subcmd, "parse") == 0) {
        if (argc < 3) {
            printf("Usage: ucfs parse <unicode-path>\n");
            printf("Example: ucfs parse '\xF0\x9F\x8F\xA0/documents/file.txt'\n");
            return 1;
        }

        ucfs_path_t path;
        int result = ucfs_parse(argv[2], &path);

        if (result != 0) {
            printf("ucfs: failed to parse path (not a valid UCFS path)\n");
            printf("UCFS paths must start with a non-ASCII Unicode character.\n");
            return 1;
        }

        printf("\n");
        printf("UCFS Path Analysis\n");
        printf("══════════════════════════════════════\n");
        printf("  Input:       %s\n", argv[2]);
        printf("  Delimiter:   %s (U+%04X)\n", path.delimiter_utf8, path.delimiter);
        printf("  Components:  %zu\n", path.component_count);

        for (size_t i = 0; i < path.component_count; i++) {
            printf("    [%zu] %s\n", i, path.components[i]);
        }

        char canonical[512];
        if (ucfs_to_canonical(&path, canonical, sizeof(canonical)) == 0) {
            printf("  Canonical:   %s\n", canonical);
        }
        printf("\n");

        ucfs_free(&path);
        universal_add_karma(1, "Parsed UCFS path");
    }
    else if (strcmp(subcmd, "canon") == 0 || strcmp(subcmd, "canonical") == 0) {
        if (argc < 3) {
            printf("Usage: ucfs canon <unicode-path>\n");
            return 1;
        }

        ucfs_path_t path;
        if (ucfs_parse(argv[2], &path) != 0) {
            printf("ucfs: invalid UCFS path\n");
            return 1;
        }

        char canonical[512];
        if (ucfs_to_canonical(&path, canonical, sizeof(canonical)) == 0) {
            printf("%s\n", canonical);
        } else {
            printf("ucfs: path too long\n");
            ucfs_free(&path);
            return 1;
        }

        ucfs_free(&path);
        universal_add_karma(1, "Converted UCFS path");
    }
    else if (strcmp(subcmd, "info") == 0) {
        printf("\n");
        printf("UCFS - Unicode Character File System\n");
        printf("═════════════════════════════════════════════════════\n");
        printf("\n");
        printf("UCFS enables Unicode characters as path roots, allowing\n");
        printf("intuitive organization using visual symbols.\n");
        printf("\n");
        printf("Path Format:\n");
        printf("  <unicode-char>/<path-components>\n");
        printf("\n");
        printf("Examples:\n");
        printf("  \xF0\x9F\x8F\xA0/documents      Home documents\n");
        printf("  \xF0\x9F\x93\xB8/vacation/2024  Photo album\n");
        printf("  \xE2\x98\x80/weather/today     Weather data\n");
        printf("  \xF0\x9F\x8E\xB5/music/jazz     Music collection\n");
        printf("  \xF0\x9F\x93\x9A/books/fiction  Book library\n");
        printf("\n");
        printf("Storage Mapping:\n");
        printf("  \xF0\x9F\x8F\xA0/file.txt  ->  /ucfs/U+1F3E0/file.txt\n");
        printf("\n");
    }
    else if (strcmp(subcmd, "roots") == 0) {
        printf("\n");
        printf("Common UCFS Roots\n");
        printf("═════════════════════════════════════════════════════\n");
        printf("  Symbol  Code       Suggested Use\n");
        printf("  ──────  ─────────  ──────────────────────────\n");
        printf("  \xF0\x9F\x8F\xA0      U+1F3E0    Home directory\n");
        printf("  \xF0\x9F\x93\x81      U+1F4C1    Documents\n");
        printf("  \xF0\x9F\x93\xB8      U+1F4F8    Photos\n");
        printf("  \xF0\x9F\x8E\xB5      U+1F3B5    Music\n");
        printf("  \xF0\x9F\x8E\xAC      U+1F3AC    Videos\n");
        printf("  \xF0\x9F\x93\x9A      U+1F4DA    Books/Library\n");
        printf("  \xF0\x9F\x92\xBB      U+1F4BB    Code/Projects\n");
        printf("  \xE2\x98\x80      U+2600     Weather/Sensors\n");
        printf("  \xF0\x9F\x8C\x8D      U+1F30D    Network/Web\n");
        printf("  \xF0\x9F\x94\x92      U+1F512    Secure/Private\n");
        printf("  \xF0\x9F\x97\x91      U+1F5D1    Trash\n");
        printf("  \xE2\xAD\x90      U+2B50     Favorites\n");
        printf("\n");
    }
    else {
        printf("ucfs: unknown command '%s'\n", subcmd);
        printf("Use 'ucfs' without arguments to see available commands.\n");
        return 1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RF2S COMMANDS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief rf2s - Radio Frequency File System operations
 */
static int cmd_rf2s(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: rf2s <command> [args]\n");
        printf("\n");
        printf("Commands:\n");
        printf("  parse <path>     Parse an RF2S path\n");
        printf("  canon <path>     Convert to canonical path\n");
        printf("  freq <value>     Parse a frequency value\n");
        printf("  info             Show RF2S information\n");
        printf("  bands            List common frequency bands\n");
        printf("\n");
        printf("RF2S uses radio frequencies as path roots:\n");
        printf("  /432MHz/sensor/temp  -> /rf2s/432000000/sensor/temp\n");
        printf("  /2.4GHz/wifi/beacon  -> /rf2s/2400000000/wifi/beacon\n");
        return 1;
    }

    const char* subcmd = argv[1];

    if (strcmp(subcmd, "parse") == 0) {
        if (argc < 3) {
            printf("Usage: rf2s parse <path>\n");
            printf("Example: rf2s parse '/432MHz/sensor/data'\n");
            return 1;
        }

        rf2s_path_t path;
        int result = rf2s_parse(argv[2], &path);

        if (result != 0) {
            printf("rf2s: failed to parse path\n");
            return 1;
        }

        char freq_str[64];
        rf2s_format_frequency(path.frequency.frequency_hz, freq_str, sizeof(freq_str));

        printf("\n");
        printf("RF2S Path Analysis\n");
        printf("══════════════════════════════════════\n");
        printf("  Input:       %s\n", argv[2]);
        printf("  Frequency:   %s (%llu Hz)\n", freq_str,
               (unsigned long long)path.frequency.frequency_hz);
        printf("  Components:  %zu\n", path.component_count);

        for (size_t i = 0; i < path.component_count; i++) {
            printf("    [%zu] %s\n", i, path.components[i]);
        }

        char canonical[512];
        if (rf2s_to_canonical(&path, canonical, sizeof(canonical)) == 0) {
            printf("  Canonical:   %s\n", canonical);
        }
        printf("\n");

        rf2s_free(&path);
        universal_add_karma(1, "Parsed RF2S path");
    }
    else if (strcmp(subcmd, "canon") == 0 || strcmp(subcmd, "canonical") == 0) {
        if (argc < 3) {
            printf("Usage: rf2s canon <path>\n");
            return 1;
        }

        rf2s_path_t path;
        if (rf2s_parse(argv[2], &path) != 0) {
            printf("rf2s: invalid RF2S path\n");
            return 1;
        }

        char canonical[512];
        if (rf2s_to_canonical(&path, canonical, sizeof(canonical)) == 0) {
            printf("%s\n", canonical);
        } else {
            printf("rf2s: path too long\n");
            rf2s_free(&path);
            return 1;
        }

        rf2s_free(&path);
        universal_add_karma(1, "Converted RF2S path");
    }
    else if (strcmp(subcmd, "freq") == 0) {
        if (argc < 3) {
            printf("Usage: rf2s freq <frequency>\n");
            printf("Examples:\n");
            printf("  rf2s freq 432MHz\n");
            printf("  rf2s freq 2.4GHz\n");
            printf("  rf2s freq 915kHz\n");
            return 1;
        }

        rf2s_freq_t freq;
        if (rf2s_parse_frequency(argv[2], &freq) != 0) {
            printf("rf2s: invalid frequency format\n");
            return 1;
        }

        char formatted[64];
        rf2s_format_frequency(freq.frequency_hz, formatted, sizeof(formatted));

        printf("\n");
        printf("Frequency: %s\n", argv[2]);
        printf("  Normalized: %llu Hz\n", (unsigned long long)freq.frequency_hz);
        printf("  Formatted:  %s\n", formatted);
        printf("\n");

        universal_add_karma(1, "Parsed frequency");
    }
    else if (strcmp(subcmd, "info") == 0) {
        printf("\n");
        printf("RF2S - Radio Frequency File System\n");
        printf("═════════════════════════════════════════════════════\n");
        printf("\n");
        printf("RF2S organizes data by radio frequency, enabling:\n");
        printf("  - SDR (Software Defined Radio) data storage\n");
        printf("  - Sensor networks organized by frequency\n");
        printf("  - Spectrum analysis archives\n");
        printf("  - IoT device data by communication band\n");
        printf("\n");
        printf("Path Format:\n");
        printf("  /<frequency>/<path-components>\n");
        printf("\n");
        printf("Frequency Formats:\n");
        printf("  432000000     Raw Hz\n");
        printf("  432Hz         With unit\n");
        printf("  432kHz        Kilohertz\n");
        printf("  432MHz        Megahertz\n");
        printf("  2.4GHz        Gigahertz (decimal)\n");
        printf("\n");
        printf("Examples:\n");
        printf("  /433.92MHz/garage/door    ISM band sensor\n");
        printf("  /2.4GHz/wifi/ssid_list    WiFi data\n");
        printf("  /915MHz/lora/node_01      LoRa sensor\n");
        printf("  /5.8GHz/video/drone       Video feed\n");
        printf("\n");
    }
    else if (strcmp(subcmd, "bands") == 0) {
        printf("\n");
        printf("Common RF2S Frequency Bands\n");
        printf("═════════════════════════════════════════════════════\n");
        printf("  Frequency     Use Case\n");
        printf("  ──────────    ──────────────────────────────\n");
        printf("  433.92MHz     ISM band (EU), garage doors\n");
        printf("  868MHz        ISM band (EU), LoRa, Zigbee\n");
        printf("  915MHz        ISM band (US), LoRa\n");
        printf("  2.4GHz        WiFi, Bluetooth, Zigbee\n");
        printf("  5GHz          WiFi 5/6\n");
        printf("  5.8GHz        Video transmitters, drones\n");
        printf("  27MHz         CB radio\n");
        printf("  144MHz        VHF amateur (2m)\n");
        printf("  432MHz        UHF amateur (70cm)\n");
        printf("  1.2GHz        Amateur, ATV\n");
        printf("  1575.42MHz    GPS L1\n");
        printf("  1090MHz       ADS-B (aircraft)\n");
        printf("\n");
    }
    else {
        printf("rf2s: unknown command '%s'\n", subcmd);
        printf("Use 'rf2s' without arguments to see available commands.\n");
        return 1;
    }

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * REGISTRATION
 * ═══════════════════════════════════════════════════════════════════════════ */

void register_ucfs_commands(void) {
    universal_shell_register_command(
        "ucfs", cmd_ucfs,
        CMD_CAT_FILESYSTEM, OS_SUPPORT_ALL,
        "Unicode Character File System",
        "ucfs <command> [args]\n"
        "Commands: parse, canon, info, roots\n"
        "Use Unicode characters as path roots.\n"
        "Example: ucfs parse '\xF0\x9F\x8F\xA0/docs/file.txt'");

    universal_shell_register_command(
        "rf2s", cmd_rf2s,
        CMD_CAT_FILESYSTEM, OS_SUPPORT_ALL,
        "Radio Frequency File System",
        "rf2s <command> [args]\n"
        "Commands: parse, canon, freq, info, bands\n"
        "Use radio frequencies as path roots.\n"
        "Example: rf2s parse '/432MHz/sensor/data'");
}
