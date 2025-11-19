#ifndef TBOS_FS_UCFS_CONFIG_H
#define TBOS_FS_UCFS_CONFIG_H

#include <stddef.h>
#include <stdint.h>

#define UCFS_MAX_DELIMITER_MAPPINGS 64
#define UCFS_MAX_PATH_LENGTH 256

/**
 * Configuration for a UCFS delimiter mapping
 */
typedef struct {
    uint32_t delimiter;              /* Unicode codepoint */
    char delimiter_utf8[8];          /* UTF-8 representation */
    size_t delimiter_len;            /* Length of UTF-8 bytes */
    char backing_path[UCFS_MAX_PATH_LENGTH];  /* Backing storage path */
    char culture[32];                /* Cultural context (optional) */
    char description[128];           /* Human-readable description */
    int active;                      /* 1 if active, 0 if disabled */
} ucfs_delimiter_config_t;

/**
 * Global UCFS configuration
 */
typedef struct {
    char default_backing[UCFS_MAX_PATH_LENGTH];  /* Default backing path */
    ucfs_delimiter_config_t delimiters[UCFS_MAX_DELIMITER_MAPPINGS];
    size_t delimiter_count;
} ucfs_config_t;

/**
 * Load UCFS configuration from file
 * @param config_path Path to config file (default: /etc/tbos/ucfs.conf)
 * @return 0 on success, negative error code on failure
 */
int ucfs_config_load(const char* config_path, ucfs_config_t* config);

/**
 * Save UCFS configuration to file
 * @param config_path Path to config file
 * @param config Configuration to save
 * @return 0 on success, negative error code on failure
 */
int ucfs_config_save(const char* config_path, const ucfs_config_t* config);

/**
 * Get delimiter configuration by codepoint
 * @param config Configuration object
 * @param delimiter Unicode codepoint
 * @return Pointer to config or NULL if not found
 */
const ucfs_delimiter_config_t* ucfs_config_get_delimiter(
    const ucfs_config_t* config, uint32_t delimiter);

/**
 * Add or update delimiter configuration
 * @param config Configuration object
 * @param delim_config Delimiter configuration to add/update
 * @return 0 on success, negative error code on failure
 */
int ucfs_config_set_delimiter(ucfs_config_t* config,
                               const ucfs_delimiter_config_t* delim_config);

/**
 * Initialize default UCFS configuration
 * @param config Configuration object to initialize
 */
void ucfs_config_init_defaults(ucfs_config_t* config);

#endif /* TBOS_FS_UCFS_CONFIG_H */
