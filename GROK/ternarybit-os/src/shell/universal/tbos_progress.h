/**
 * @file tbos_progress.h
 * @brief TBOS Progress Indicators
 *
 * Provides visual feedback for long-running operations:
 * - Spinners (indeterminate progress)
 * - Progress bars (determinate progress)
 * - Status messages (OK, FAIL, WARN, INFO)
 */

#ifndef TBOS_PROGRESS_H
#define TBOS_PROGRESS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * PROGRESS TYPES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Progress indicator types
 */
typedef enum {
    PROGRESS_SPINNER,    /* Spinning animation (indeterminate) */
    PROGRESS_BAR,        /* Progress bar with percentage */
    PROGRESS_DOTS,       /* Loading... dots animation */
    PROGRESS_PERCENT     /* Simple percentage display */
} tbos_progress_type_t;

/**
 * @brief Spinner style
 */
typedef enum {
    SPINNER_BRAILLE,     /* ⠋⠙⠹⠸⠼⠴⠦⠧⠇⠏ */
    SPINNER_LINE,        /* |/-\ */
    SPINNER_DOTS,        /* ⣾⣽⣻⢿⡿⣟⣯⣷ */
    SPINNER_ARROW,       /* ←↖↑↗→↘↓↙ */
    SPINNER_BOUNCE       /* ⠁⠂⠄⠂ */
} tbos_spinner_style_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * PROGRESS HANDLE
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Opaque progress indicator handle
 */
typedef struct tbos_progress tbos_progress_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * SPINNER API (Indeterminate Progress)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Start a spinner with message
 *
 * @param message  Message to display (e.g., "Downloading...")
 * @return Progress handle (for updates)
 */
tbos_progress_t* tbos_spinner_start(const char* message);

/**
 * @brief Start spinner with custom style
 *
 * @param message  Message to display
 * @param style    Spinner animation style
 * @return Progress handle
 */
tbos_progress_t* tbos_spinner_start_styled(const char* message,
                                            tbos_spinner_style_t style);

/**
 * @brief Update spinner message
 *
 * @param p        Progress handle
 * @param message  New message
 */
void tbos_spinner_update(tbos_progress_t* p, const char* message);

/**
 * @brief Advance spinner animation (call periodically)
 *
 * @param p  Progress handle
 */
void tbos_spinner_tick(tbos_progress_t* p);

/**
 * @brief Stop spinner with success/failure indication
 *
 * @param p        Progress handle
 * @param success  true = show checkmark, false = show X
 */
void tbos_spinner_stop(tbos_progress_t* p, bool success);

/**
 * @brief Stop spinner with custom final message
 *
 * @param p        Progress handle
 * @param message  Final message
 * @param success  true = show checkmark, false = show X
 */
void tbos_spinner_stop_with_message(tbos_progress_t* p, const char* message,
                                     bool success);

/* ═══════════════════════════════════════════════════════════════════════════
 * PROGRESS BAR API (Determinate Progress)
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Start a progress bar
 *
 * @param message  Message to display
 * @param total    Total value (100%)
 * @return Progress handle
 */
tbos_progress_t* tbos_progress_start(const char* message, size_t total);

/**
 * @brief Update progress bar
 *
 * @param p        Progress handle
 * @param current  Current value
 */
void tbos_progress_update(tbos_progress_t* p, size_t current);

/**
 * @brief Update progress bar with message
 *
 * @param p        Progress handle
 * @param current  Current value
 * @param message  Updated message
 */
void tbos_progress_update_msg(tbos_progress_t* p, size_t current,
                               const char* message);

/**
 * @brief Increment progress by delta
 *
 * @param p      Progress handle
 * @param delta  Amount to increment
 */
void tbos_progress_increment(tbos_progress_t* p, size_t delta);

/**
 * @brief Finish progress bar
 *
 * @param p  Progress handle (freed after call)
 */
void tbos_progress_finish(tbos_progress_t* p);

/* ═══════════════════════════════════════════════════════════════════════════
 * STATUS MESSAGES
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Print OK status message
 *
 * Format: [OK] message
 *
 * @param message  Message to display
 */
void tbos_status_ok(const char* message);

/**
 * @brief Print FAIL status message
 *
 * Format: [FAIL] message
 *
 * @param message  Message to display
 */
void tbos_status_fail(const char* message);

/**
 * @brief Print WARN status message
 *
 * Format: [WARN] message
 *
 * @param message  Message to display
 */
void tbos_status_warn(const char* message);

/**
 * @brief Print INFO status message
 *
 * Format: [INFO] message
 *
 * @param message  Message to display
 */
void tbos_status_info(const char* message);

/**
 * @brief Print SKIP status message
 *
 * Format: [SKIP] message
 *
 * @param message  Message to display
 */
void tbos_status_skip(const char* message);

/* ═══════════════════════════════════════════════════════════════════════════
 * UTILITY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Format bytes as human-readable size
 *
 * @param bytes  Number of bytes
 * @param buf    Output buffer
 * @param size   Buffer size
 * @return Pointer to buf
 */
const char* tbos_format_bytes(uint64_t bytes, char* buf, size_t size);

/**
 * @brief Format duration as human-readable time
 *
 * @param seconds  Duration in seconds
 * @param buf      Output buffer
 * @param size     Buffer size
 * @return Pointer to buf
 */
const char* tbos_format_duration(double seconds, char* buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_PROGRESS_H */
