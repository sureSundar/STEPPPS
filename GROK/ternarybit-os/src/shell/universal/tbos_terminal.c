/**
 * @file tbos_terminal.c
 * @brief TBOS Terminal Handling Implementation
 *
 * Implements raw terminal mode, key reading with escape sequence parsing,
 * cursor control, and screen manipulation.
 *
 * Platform support:
 * - POSIX (Linux, macOS, BSD): Uses termios
 * - Windows: Uses Windows Console API
 * - WebAssembly: Stub implementation
 */

#include "tbos_terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * PLATFORM-SPECIFIC INCLUDES
 * ═══════════════════════════════════════════════════════════════════════════ */

#if defined(_WIN32) || defined(TBOS_ARCH_WINDOWS)
    /* Windows Console API */
    #define TBOS_PLATFORM_WINDOWS 1
    #include <windows.h>
    #include <conio.h>
#elif defined(__EMSCRIPTEN__) || defined(TBOS_ARCH_WASM)
    /* WebAssembly - minimal stub */
    #define TBOS_PLATFORM_WASM 1
#else
    /* POSIX (Linux, macOS, BSD) */
    #define TBOS_PLATFORM_POSIX 1
    #include <unistd.h>
    #include <termios.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
    #include <errno.h>
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * TERMINAL STATE
 * ═══════════════════════════════════════════════════════════════════════════ */

#if defined(TBOS_PLATFORM_POSIX)

struct tbos_term_state {
    struct termios orig_termios;
    bool raw_mode_active;
};

static bool g_raw_mode = false;

#elif defined(TBOS_PLATFORM_WINDOWS)

struct tbos_term_state {
    DWORD orig_mode_in;
    DWORD orig_mode_out;
    HANDLE h_stdin;
    HANDLE h_stdout;
    bool raw_mode_active;
};

static bool g_raw_mode = false;

#else /* WASM / stub */

struct tbos_term_state {
    bool raw_mode_active;
};

static bool g_raw_mode = false;

#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * RAW MODE FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

#if defined(TBOS_PLATFORM_POSIX)

int tbos_term_raw_enable(tbos_term_state_t** state) {
    if (!state) return -1;

    /* Allocate state structure */
    *state = (tbos_term_state_t*)malloc(sizeof(tbos_term_state_t));
    if (!*state) return -1;

    /* Get current terminal settings */
    if (tcgetattr(STDIN_FILENO, &(*state)->orig_termios) == -1) {
        free(*state);
        *state = NULL;
        return -1;
    }

    /* Configure raw mode */
    struct termios raw = (*state)->orig_termios;

    /* Input flags: disable break, parity, strip, flow control */
    /* Keep ICRNL so CR is converted to NL on input */
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON);

    /* Output flags: KEEP output processing enabled for proper newline handling */
    /* raw.c_oflag &= ~(OPOST); -- Don't disable this! */

    /* Control flags: set 8 bits per char */
    raw.c_cflag |= (CS8);

    /* Local flags: disable echo, canonical mode, extended input */
    /* Keep ISIG for Ctrl+C handling */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN);

    /* Control characters: read returns after 1 byte, no timeout */
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    /* Apply settings */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        free(*state);
        *state = NULL;
        return -1;
    }

    (*state)->raw_mode_active = true;
    g_raw_mode = true;

    return 0;
}

int tbos_term_raw_disable(tbos_term_state_t* state) {
    if (!state) return -1;

    /* Restore original settings */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &state->orig_termios) == -1) {
        free(state);
        return -1;
    }

    state->raw_mode_active = false;
    g_raw_mode = false;

    free(state);
    return 0;
}

bool tbos_term_is_raw(void) {
    return g_raw_mode;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * KEY READING
 * ═══════════════════════════════════════════════════════════════════════════ */

/**
 * @brief Read a single byte with optional timeout
 */
static int read_byte_timeout(int timeout_ms) {
    if (timeout_ms > 0) {
        fd_set fds;
        struct timeval tv;

        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);

        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
        if (ret <= 0) return KEY_EOF;  /* Timeout or error */
    }

    unsigned char c;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    if (n == -1) return KEY_ERROR;
    if (n == 0) return KEY_EOF;

    return c;
}

/**
 * @brief Parse escape sequence after ESC [
 */
static int parse_csi_sequence(void) {
    int c = read_byte_timeout(50);
    if (c == KEY_EOF || c == KEY_ERROR) return KEY_ESCAPE;

    /* Handle arrow keys and simple sequences */
    switch (c) {
        case 'A': return KEY_UP;
        case 'B': return KEY_DOWN;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        case 'H': return KEY_HOME;
        case 'F': return KEY_END;
        case '~': return KEY_UNKNOWN;  /* Should not happen */
    }

    /* Handle sequences like ESC [ 1 ~ (Home), ESC [ 4 ~ (End), etc. */
    if (c >= '0' && c <= '9') {
        int num = c - '0';

        /* Read more digits */
        while (1) {
            c = read_byte_timeout(50);
            if (c == KEY_EOF || c == KEY_ERROR) return KEY_UNKNOWN;

            if (c >= '0' && c <= '9') {
                num = num * 10 + (c - '0');
            } else {
                break;
            }
        }

        /* Handle tilde-terminated sequences */
        if (c == '~') {
            switch (num) {
                case 1: return KEY_HOME;
                case 2: return KEY_INSERT;
                case 3: return KEY_DELETE;
                case 4: return KEY_END;
                case 5: return KEY_PAGE_UP;
                case 6: return KEY_PAGE_DOWN;
                case 7: return KEY_HOME;
                case 8: return KEY_END;
            }
        }
    }

    return KEY_UNKNOWN;
}

/**
 * @brief Parse escape sequence after ESC O
 */
static int parse_ss3_sequence(void) {
    int c = read_byte_timeout(50);
    if (c == KEY_EOF || c == KEY_ERROR) return KEY_ESCAPE;

    switch (c) {
        case 'A': return KEY_UP;
        case 'B': return KEY_DOWN;
        case 'C': return KEY_RIGHT;
        case 'D': return KEY_LEFT;
        case 'H': return KEY_HOME;
        case 'F': return KEY_END;
        case 'P': return KEY_UNKNOWN;  /* F1 */
        case 'Q': return KEY_UNKNOWN;  /* F2 */
        case 'R': return KEY_UNKNOWN;  /* F3 */
        case 'S': return KEY_UNKNOWN;  /* F4 */
    }

    return KEY_UNKNOWN;
}

int tbos_term_read_key(void) {
    int c = read_byte_timeout(-1);  /* Blocking read */

    if (c == KEY_EOF || c == KEY_ERROR) return c;

    /* Handle escape sequences */
    if (c == KEY_ESCAPE) {
        int next = read_byte_timeout(50);

        if (next == KEY_EOF) {
            /* Just ESC key pressed */
            return KEY_ESCAPE;
        }

        if (next == '[') {
            return parse_csi_sequence();
        }

        if (next == 'O') {
            return parse_ss3_sequence();
        }

        /* Unknown escape sequence, return ESC */
        return KEY_ESCAPE;
    }

    /* Handle backspace variants */
    if (c == 127 || c == 8) {
        return KEY_BACKSPACE;
    }

    return c;
}

int tbos_term_read_key_timeout(int timeout_ms) {
    int c = read_byte_timeout(timeout_ms);

    if (c == KEY_EOF || c == KEY_ERROR) return c;

    /* Handle escape sequences */
    if (c == KEY_ESCAPE) {
        int next = read_byte_timeout(50);

        if (next == KEY_EOF) {
            return KEY_ESCAPE;
        }

        if (next == '[') {
            return parse_csi_sequence();
        }

        if (next == 'O') {
            return parse_ss3_sequence();
        }

        return KEY_ESCAPE;
    }

    if (c == 127 || c == 8) {
        return KEY_BACKSPACE;
    }

    return c;
}

bool tbos_term_key_available(void) {
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * TERMINAL SIZE
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_term_get_size(int* rows, int* cols) {
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        /* Fallback: try environment variables */
        char* lines = getenv("LINES");
        char* columns = getenv("COLUMNS");

        if (lines && columns) {
            *rows = atoi(lines);
            *cols = atoi(columns);
            return 0;
        }

        /* Last resort: assume standard terminal */
        *rows = 24;
        *cols = 80;
        return -1;
    }

    *rows = ws.ws_row;
    *cols = ws.ws_col;
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CURSOR CONTROL
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_term_cursor_move(int row, int col) {
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

void tbos_term_cursor_move_rel(int row_delta, int col_delta) {
    if (row_delta > 0) {
        printf("\033[%dB", row_delta);  /* Down */
    } else if (row_delta < 0) {
        printf("\033[%dA", -row_delta);  /* Up */
    }

    if (col_delta > 0) {
        printf("\033[%dC", col_delta);  /* Right */
    } else if (col_delta < 0) {
        printf("\033[%dD", -col_delta);  /* Left */
    }

    fflush(stdout);
}

void tbos_term_cursor_save(void) {
    printf("\033[s");
    fflush(stdout);
}

void tbos_term_cursor_restore(void) {
    printf("\033[u");
    fflush(stdout);
}

void tbos_term_cursor_hide(void) {
    printf("\033[?25l");
    fflush(stdout);
}

void tbos_term_cursor_show(void) {
    printf("\033[?25h");
    fflush(stdout);
}

int tbos_term_cursor_get(int* row, int* col) {
    /* Query cursor position: ESC [ 6 n */
    printf("\033[6n");
    fflush(stdout);

    /* Response format: ESC [ row ; col R */
    char buf[32];
    int i = 0;

    while (i < (int)sizeof(buf) - 1) {
        int c = read_byte_timeout(100);
        if (c == KEY_EOF || c == KEY_ERROR) return -1;

        buf[i++] = (char)c;

        if (c == 'R') break;
    }
    buf[i] = '\0';

    /* Parse response */
    if (buf[0] != '\033' || buf[1] != '[') return -1;

    if (sscanf(buf + 2, "%d;%d", row, col) != 2) return -1;

    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SCREEN CONTROL
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_term_clear_screen(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void tbos_term_clear_to_eos(void) {
    printf("\033[J");
    fflush(stdout);
}

void tbos_term_clear_line(void) {
    printf("\033[2K\r");
    fflush(stdout);
}

void tbos_term_clear_to_eol(void) {
    printf("\033[K");
    fflush(stdout);
}

void tbos_term_clear_to_bol(void) {
    printf("\033[1K");
    fflush(stdout);
}

void tbos_term_bell(void) {
    printf("\a");
    fflush(stdout);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COLOR CONTROL
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_term_set_color(int fg, int bg) {
    if (fg >= 0 && fg < 8) {
        printf("\033[%dm", 30 + fg);
    } else if (fg >= 8 && fg < 16) {
        printf("\033[%dm", 90 + (fg - 8));
    } else if (fg == -1) {
        printf("\033[39m");  /* Default foreground */
    }

    if (bg >= 0 && bg < 8) {
        printf("\033[%dm", 40 + bg);
    } else if (bg == -1) {
        printf("\033[49m");  /* Default background */
    }

    fflush(stdout);
}

void tbos_term_reset_style(void) {
    printf("\033[0m");
    fflush(stdout);
}

#elif defined(TBOS_PLATFORM_WINDOWS)
/* ═══════════════════════════════════════════════════════════════════════════
 * WINDOWS IMPLEMENTATION
 * Uses Windows Console API for terminal control
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_term_raw_enable(tbos_term_state_t** state) {
    if (!state) return -1;

    *state = (tbos_term_state_t*)malloc(sizeof(tbos_term_state_t));
    if (!*state) return -1;

    (*state)->h_stdin = GetStdHandle(STD_INPUT_HANDLE);
    (*state)->h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);

    /* Save original modes */
    GetConsoleMode((*state)->h_stdin, &(*state)->orig_mode_in);
    GetConsoleMode((*state)->h_stdout, &(*state)->orig_mode_out);

    /* Enable raw mode */
    DWORD mode_in = (*state)->orig_mode_in;
    mode_in &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
    mode_in |= ENABLE_VIRTUAL_TERMINAL_INPUT;
    SetConsoleMode((*state)->h_stdin, mode_in);

    /* Enable VT100 processing for output */
    DWORD mode_out = (*state)->orig_mode_out;
    mode_out |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode((*state)->h_stdout, mode_out);

    (*state)->raw_mode_active = true;
    g_raw_mode = true;

    return 0;
}

int tbos_term_raw_disable(tbos_term_state_t* state) {
    if (!state) return -1;

    SetConsoleMode(state->h_stdin, state->orig_mode_in);
    SetConsoleMode(state->h_stdout, state->orig_mode_out);

    state->raw_mode_active = false;
    g_raw_mode = false;

    free(state);
    return 0;
}

bool tbos_term_is_raw(void) {
    return g_raw_mode;
}

int tbos_term_read_key(void) {
    int c = _getch();
    if (c == 0 || c == 0xE0) {
        int extended = _getch();
        switch (extended) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            case 71: return KEY_HOME;
            case 79: return KEY_END;
            case 82: return KEY_INSERT;
            case 83: return KEY_DELETE;
            case 73: return KEY_PAGE_UP;
            case 81: return KEY_PAGE_DOWN;
            default: return KEY_UNKNOWN;
        }
    }
    if (c == 8 || c == 127) return KEY_BACKSPACE;
    if (c == 27) return KEY_ESCAPE;
    return c;
}

int tbos_term_read_key_timeout(int timeout_ms) {
    DWORD start = GetTickCount();
    while (!_kbhit()) {
        if (timeout_ms > 0 && (GetTickCount() - start) >= (DWORD)timeout_ms) {
            return KEY_EOF;
        }
        Sleep(10);
    }
    return tbos_term_read_key();
}

bool tbos_term_key_available(void) {
    return _kbhit() != 0;
}

int tbos_term_get_size(int* rows, int* cols) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return 0;
    }
    *rows = 24;
    *cols = 80;
    return -1;
}

void tbos_term_cursor_move(int row, int col) {
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
}

void tbos_term_cursor_move_rel(int row_delta, int col_delta) {
    if (row_delta > 0) printf("\033[%dB", row_delta);
    else if (row_delta < 0) printf("\033[%dA", -row_delta);
    if (col_delta > 0) printf("\033[%dC", col_delta);
    else if (col_delta < 0) printf("\033[%dD", -col_delta);
    fflush(stdout);
}

void tbos_term_cursor_save(void) { printf("\033[s"); fflush(stdout); }
void tbos_term_cursor_restore(void) { printf("\033[u"); fflush(stdout); }
void tbos_term_cursor_hide(void) { printf("\033[?25l"); fflush(stdout); }
void tbos_term_cursor_show(void) { printf("\033[?25h"); fflush(stdout); }

int tbos_term_cursor_get(int* row, int* col) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *row = csbi.dwCursorPosition.Y + 1;
        *col = csbi.dwCursorPosition.X + 1;
        return 0;
    }
    return -1;
}

void tbos_term_clear_screen(void) { printf("\033[2J\033[H"); fflush(stdout); }
void tbos_term_clear_to_eos(void) { printf("\033[J"); fflush(stdout); }
void tbos_term_clear_line(void) { printf("\033[2K\r"); fflush(stdout); }
void tbos_term_clear_to_eol(void) { printf("\033[K"); fflush(stdout); }
void tbos_term_clear_to_bol(void) { printf("\033[1K"); fflush(stdout); }
void tbos_term_bell(void) { printf("\a"); fflush(stdout); }

void tbos_term_set_color(int fg, int bg) {
    if (fg >= 0 && fg < 8) printf("\033[%dm", 30 + fg);
    else if (fg >= 8 && fg < 16) printf("\033[%dm", 90 + (fg - 8));
    else if (fg == -1) printf("\033[39m");
    if (bg >= 0 && bg < 8) printf("\033[%dm", 40 + bg);
    else if (bg == -1) printf("\033[49m");
    fflush(stdout);
}

void tbos_term_reset_style(void) { printf("\033[0m"); fflush(stdout); }

#else /* WASM / Stub implementation */
/* ═══════════════════════════════════════════════════════════════════════════
 * STUB IMPLEMENTATION (WebAssembly, etc.)
 * Basic stubs that allow compilation but provide minimal functionality
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_term_raw_enable(tbos_term_state_t** state) {
    if (!state) return -1;
    *state = (tbos_term_state_t*)malloc(sizeof(tbos_term_state_t));
    if (!*state) return -1;
    (*state)->raw_mode_active = true;
    g_raw_mode = true;
    return 0;
}

int tbos_term_raw_disable(tbos_term_state_t* state) {
    if (!state) return -1;
    state->raw_mode_active = false;
    g_raw_mode = false;
    free(state);
    return 0;
}

bool tbos_term_is_raw(void) { return g_raw_mode; }

int tbos_term_read_key(void) {
    int c = getchar();
    if (c == EOF) return KEY_EOF;
    return c;
}

int tbos_term_read_key_timeout(int timeout_ms) {
    (void)timeout_ms;
    return tbos_term_read_key();
}

bool tbos_term_key_available(void) { return false; }
int tbos_term_get_size(int* rows, int* cols) { *rows = 24; *cols = 80; return 0; }

void tbos_term_cursor_move(int row, int col) { printf("\033[%d;%dH", row, col); fflush(stdout); }
void tbos_term_cursor_move_rel(int row_delta, int col_delta) {
    (void)row_delta; (void)col_delta;
}
void tbos_term_cursor_save(void) { printf("\033[s"); fflush(stdout); }
void tbos_term_cursor_restore(void) { printf("\033[u"); fflush(stdout); }
void tbos_term_cursor_hide(void) { printf("\033[?25l"); fflush(stdout); }
void tbos_term_cursor_show(void) { printf("\033[?25h"); fflush(stdout); }
int tbos_term_cursor_get(int* row, int* col) { *row = 1; *col = 1; return 0; }

void tbos_term_clear_screen(void) { printf("\033[2J\033[H"); fflush(stdout); }
void tbos_term_clear_to_eos(void) { printf("\033[J"); fflush(stdout); }
void tbos_term_clear_line(void) { printf("\033[2K\r"); fflush(stdout); }
void tbos_term_clear_to_eol(void) { printf("\033[K"); fflush(stdout); }
void tbos_term_clear_to_bol(void) { printf("\033[1K"); fflush(stdout); }
void tbos_term_bell(void) { printf("\a"); fflush(stdout); }

void tbos_term_set_color(int fg, int bg) {
    if (fg >= 0 && fg < 8) printf("\033[%dm", 30 + fg);
    else if (fg == -1) printf("\033[39m");
    if (bg >= 0 && bg < 8) printf("\033[%dm", 40 + bg);
    else if (bg == -1) printf("\033[49m");
    fflush(stdout);
}

void tbos_term_reset_style(void) { printf("\033[0m"); fflush(stdout); }

#endif /* Platform implementations */
