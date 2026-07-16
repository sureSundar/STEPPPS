/**
 * @file tbos_terminal.h
 * @brief TBOS Terminal Handling - Raw mode, key reading, cursor control
 *
 * Provides low-level terminal control for the TBOS shell:
 * - Raw mode enable/disable for character-by-character input
 * - Special key code reading (arrows, function keys, etc.)
 * - Cursor positioning and screen control
 * - Terminal size detection
 */

#ifndef TBOS_TERMINAL_H
#define TBOS_TERMINAL_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * KEY CODES
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Special key codes (above 255 to avoid ASCII conflicts) */
#define KEY_UP        1001
#define KEY_DOWN      1002
#define KEY_LEFT      1003
#define KEY_RIGHT     1004
#define KEY_HOME      1005
#define KEY_END       1006
#define KEY_DELETE    1007
#define KEY_PAGE_UP   1008
#define KEY_PAGE_DOWN 1009
#define KEY_INSERT    1010

/* Control keys */
#define KEY_BACKSPACE 127
#define KEY_TAB       9
#define KEY_ENTER     10
#define KEY_ESCAPE    27

/* Ctrl+key combinations (Ctrl+A = 1, Ctrl+B = 2, etc.) */
#define KEY_CTRL_A    1   /* Beginning of line */
#define KEY_CTRL_B    2   /* Back one character */
#define KEY_CTRL_C    3   /* Interrupt */
#define KEY_CTRL_D    4   /* EOF / Delete forward */
#define KEY_CTRL_E    5   /* End of line */
#define KEY_CTRL_F    6   /* Forward one character */
#define KEY_CTRL_G    7   /* Bell / Cancel */
#define KEY_CTRL_H    8   /* Backspace (alternative) */
#define KEY_CTRL_K    11  /* Kill to end of line */
#define KEY_CTRL_L    12  /* Clear screen */
#define KEY_CTRL_N    14  /* Next history */
#define KEY_CTRL_P    16  /* Previous history */
#define KEY_CTRL_R    18  /* Reverse search */
#define KEY_CTRL_T    20  /* Transpose characters */
#define KEY_CTRL_U    21  /* Kill whole line */
#define KEY_CTRL_W    23  /* Kill word backward */
#define KEY_CTRL_Y    25  /* Yank (paste) */
#define KEY_CTRL_Z    26  /* Suspend */

/* Error/special return values */
#define KEY_EOF       -1
#define KEY_ERROR     -2
#define KEY_UNKNOWN   -3

/* ═══════════════════════════════════════════════════════════════════════════
 * TERMINAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Opaque terminal state structure
 * Stores original terminal settings for restoration
 */
typedef struct tbos_term_state tbos_term_state_t;

/* ═══════════════════════════════════════════════════════════════════════════
 * RAW MODE FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Enable raw terminal mode
 *
 * Disables:
 * - Line buffering (ICANON)
 * - Echo (ECHO)
 * - Signal generation (ISIG)
 * - Input processing (IXON, ICRNL, etc.)
 *
 * @param state  Pointer to store original state (for later restoration)
 * @return 0 on success, -1 on error
 */
int tbos_term_raw_enable(tbos_term_state_t** state);

/**
 * @brief Disable raw mode and restore original terminal settings
 *
 * @param state  State saved by tbos_term_raw_enable()
 * @return 0 on success, -1 on error
 */
int tbos_term_raw_disable(tbos_term_state_t* state);

/**
 * @brief Check if terminal is in raw mode
 *
 * @return true if raw mode is active, false otherwise
 */
bool tbos_term_is_raw(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * KEY READING
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Read a single key press (blocking)
 *
 * Handles escape sequences for special keys (arrows, function keys, etc.)
 *
 * @return Key code (ASCII or KEY_* constant), KEY_EOF on end, KEY_ERROR on error
 */
int tbos_term_read_key(void);

/**
 * @brief Read a key with timeout
 *
 * @param timeout_ms  Timeout in milliseconds (0 = non-blocking)
 * @return Key code, KEY_EOF on timeout, KEY_ERROR on error
 */
int tbos_term_read_key_timeout(int timeout_ms);

/**
 * @brief Check if a key is available to read (non-blocking)
 *
 * @return true if a key is ready, false otherwise
 */
bool tbos_term_key_available(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * TERMINAL SIZE
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Get terminal dimensions
 *
 * @param rows  Output: number of rows (lines)
 * @param cols  Output: number of columns
 * @return 0 on success, -1 on error
 */
int tbos_term_get_size(int* rows, int* cols);

/* ═══════════════════════════════════════════════════════════════════════════
 * CURSOR CONTROL
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Move cursor to specified position (1-indexed)
 *
 * @param row  Row (1 = top)
 * @param col  Column (1 = left)
 */
void tbos_term_cursor_move(int row, int col);

/**
 * @brief Move cursor relative to current position
 *
 * @param row_delta  Rows to move (positive = down, negative = up)
 * @param col_delta  Columns to move (positive = right, negative = left)
 */
void tbos_term_cursor_move_rel(int row_delta, int col_delta);

/**
 * @brief Save current cursor position
 */
void tbos_term_cursor_save(void);

/**
 * @brief Restore previously saved cursor position
 */
void tbos_term_cursor_restore(void);

/**
 * @brief Hide cursor
 */
void tbos_term_cursor_hide(void);

/**
 * @brief Show cursor
 */
void tbos_term_cursor_show(void);

/**
 * @brief Get current cursor position
 *
 * @param row  Output: current row
 * @param col  Output: current column
 * @return 0 on success, -1 on error
 */
int tbos_term_cursor_get(int* row, int* col);

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN CONTROL
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Clear the entire screen
 */
void tbos_term_clear_screen(void);

/**
 * @brief Clear from cursor to end of screen
 */
void tbos_term_clear_to_eos(void);

/**
 * @brief Clear the current line
 */
void tbos_term_clear_line(void);

/**
 * @brief Clear from cursor to end of line
 */
void tbos_term_clear_to_eol(void);

/**
 * @brief Clear from beginning of line to cursor
 */
void tbos_term_clear_to_bol(void);

/**
 * @brief Ring the terminal bell
 */
void tbos_term_bell(void);

/* ═══════════════════════════════════════════════════════════════════════════
 * ANSI COLOR CODES
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Foreground colors */
#define TERM_FG_BLACK   "\033[30m"
#define TERM_FG_RED     "\033[31m"
#define TERM_FG_GREEN   "\033[32m"
#define TERM_FG_YELLOW  "\033[33m"
#define TERM_FG_BLUE    "\033[34m"
#define TERM_FG_MAGENTA "\033[35m"
#define TERM_FG_CYAN    "\033[36m"
#define TERM_FG_WHITE   "\033[37m"

/* Bright foreground colors */
#define TERM_FG_BRIGHT_BLACK   "\033[90m"
#define TERM_FG_BRIGHT_RED     "\033[91m"
#define TERM_FG_BRIGHT_GREEN   "\033[92m"
#define TERM_FG_BRIGHT_YELLOW  "\033[93m"
#define TERM_FG_BRIGHT_BLUE    "\033[94m"
#define TERM_FG_BRIGHT_MAGENTA "\033[95m"
#define TERM_FG_BRIGHT_CYAN    "\033[96m"
#define TERM_FG_BRIGHT_WHITE   "\033[97m"

/* Background colors */
#define TERM_BG_BLACK   "\033[40m"
#define TERM_BG_RED     "\033[41m"
#define TERM_BG_GREEN   "\033[42m"
#define TERM_BG_YELLOW  "\033[43m"
#define TERM_BG_BLUE    "\033[44m"
#define TERM_BG_MAGENTA "\033[45m"
#define TERM_BG_CYAN    "\033[46m"
#define TERM_BG_WHITE   "\033[47m"

/* Text attributes */
#define TERM_RESET      "\033[0m"
#define TERM_BOLD       "\033[1m"
#define TERM_DIM        "\033[2m"
#define TERM_ITALIC     "\033[3m"
#define TERM_UNDERLINE  "\033[4m"
#define TERM_BLINK      "\033[5m"
#define TERM_REVERSE    "\033[7m"
#define TERM_HIDDEN     "\033[8m"
#define TERM_STRIKE     "\033[9m"

/* Reset specific attributes */
#define TERM_RESET_BOLD      "\033[22m"
#define TERM_RESET_DIM       "\033[22m"
#define TERM_RESET_ITALIC    "\033[23m"
#define TERM_RESET_UNDERLINE "\033[24m"
#define TERM_RESET_BLINK     "\033[25m"
#define TERM_RESET_REVERSE   "\033[27m"

/**
 * @brief Set terminal color
 *
 * @param fg  Foreground color (0-7 for normal, 8-15 for bright, -1 for default)
 * @param bg  Background color (0-7 for normal, -1 for default)
 */
void tbos_term_set_color(int fg, int bg);

/**
 * @brief Reset terminal to default colors and attributes
 */
void tbos_term_reset_style(void);

#ifdef __cplusplus
}
#endif

#endif /* TBOS_TERMINAL_H */
