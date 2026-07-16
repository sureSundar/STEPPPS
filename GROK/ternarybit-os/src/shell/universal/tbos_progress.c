/**
 * @file tbos_progress.c
 * @brief TBOS Progress Indicators Implementation
 *
 * Visual feedback for long-running operations.
 */

#include "tbos_progress.h"
#include "tbos_terminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ═══════════════════════════════════════════════════════════════════════════
 * SPINNER FRAMES
 * ═══════════════════════════════════════════════════════════════════════════ */

/* Braille spinner (default) */
static const char* spinner_braille[] = {
    "\xe2\xa0\x8b", "\xe2\xa0\x99", "\xe2\xa0\xb9", "\xe2\xa0\xb8",
    "\xe2\xa0\xbc", "\xe2\xa0\xb4", "\xe2\xa0\xa6", "\xe2\xa0\xa7",
    "\xe2\xa0\x87", "\xe2\xa0\x8f"
};
static const size_t spinner_braille_count = 10;

/* Line spinner */
static const char* spinner_line[] = { "|", "/", "-", "\\" };
static const size_t spinner_line_count = 4;

/* Dots spinner */
static const char* spinner_dots[] = {
    "\xe2\xa3\xbe", "\xe2\xa3\xbd", "\xe2\xa3\xbb", "\xe2\xa2\xbf",
    "\xe2\xa1\xbf", "\xe2\xa3\x9f", "\xe2\xa3\xaf", "\xe2\xa3\xb7"
};
static const size_t spinner_dots_count = 8;

/* Arrow spinner */
static const char* spinner_arrow[] = {
    "\xe2\x86\x90", "\xe2\x86\x96", "\xe2\x86\x91", "\xe2\x86\x97",
    "\xe2\x86\x92", "\xe2\x86\x98", "\xe2\x86\x93", "\xe2\x86\x99"
};
static const size_t spinner_arrow_count = 8;

/* Bounce spinner */
static const char* spinner_bounce[] = {
    "\xe2\xa0\x81", "\xe2\xa0\x82", "\xe2\xa0\x84", "\xe2\xa0\x82"
};
static const size_t spinner_bounce_count = 4;

/* ═══════════════════════════════════════════════════════════════════════════
 * PROGRESS STRUCTURE
 * ═══════════════════════════════════════════════════════════════════════════ */

struct tbos_progress {
    tbos_progress_type_t type;
    char* message;
    size_t total;
    size_t current;
    size_t frame;
    tbos_spinner_style_t style;
    const char** frames;
    size_t frame_count;
};

/* ═══════════════════════════════════════════════════════════════════════════
 * HELPER FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

static void get_spinner_frames(tbos_spinner_style_t style,
                                const char*** frames, size_t* count) {
    switch (style) {
        case SPINNER_LINE:
            *frames = spinner_line;
            *count = spinner_line_count;
            break;
        case SPINNER_DOTS:
            *frames = spinner_dots;
            *count = spinner_dots_count;
            break;
        case SPINNER_ARROW:
            *frames = spinner_arrow;
            *count = spinner_arrow_count;
            break;
        case SPINNER_BOUNCE:
            *frames = spinner_bounce;
            *count = spinner_bounce_count;
            break;
        case SPINNER_BRAILLE:
        default:
            *frames = spinner_braille;
            *count = spinner_braille_count;
            break;
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * SPINNER API
 * ═══════════════════════════════════════════════════════════════════════════ */

tbos_progress_t* tbos_spinner_start(const char* message) {
    return tbos_spinner_start_styled(message, SPINNER_BRAILLE);
}

tbos_progress_t* tbos_spinner_start_styled(const char* message,
                                            tbos_spinner_style_t style) {
    tbos_progress_t* p = (tbos_progress_t*)calloc(1, sizeof(tbos_progress_t));
    if (!p) return NULL;

    p->type = PROGRESS_SPINNER;
    p->message = message ? strdup(message) : NULL;
    p->style = style;
    p->frame = 0;

    get_spinner_frames(style, &p->frames, &p->frame_count);

    /* Initial render */
    printf("\r%s %s", p->frames[0], p->message ? p->message : "");
    fflush(stdout);

    return p;
}

void tbos_spinner_update(tbos_progress_t* p, const char* message) {
    if (!p || p->type != PROGRESS_SPINNER) return;

    free(p->message);
    p->message = message ? strdup(message) : NULL;

    /* Redraw */
    printf("\r\033[K%s %s", p->frames[p->frame], p->message ? p->message : "");
    fflush(stdout);
}

void tbos_spinner_tick(tbos_progress_t* p) {
    if (!p || p->type != PROGRESS_SPINNER) return;

    p->frame = (p->frame + 1) % p->frame_count;

    printf("\r%s %s", p->frames[p->frame], p->message ? p->message : "");
    fflush(stdout);
}

void tbos_spinner_stop(tbos_progress_t* p, bool success) {
    if (!p) return;

    const char* symbol = success ?
        TERM_FG_GREEN "\xe2\x9c\x94" TERM_RESET :  /* Green checkmark */
        TERM_FG_RED "\xe2\x9c\x98" TERM_RESET;     /* Red X */

    printf("\r\033[K%s %s\n", symbol, p->message ? p->message : "");
    fflush(stdout);

    free(p->message);
    free(p);
}

void tbos_spinner_stop_with_message(tbos_progress_t* p, const char* message,
                                     bool success) {
    if (!p) return;

    const char* symbol = success ?
        TERM_FG_GREEN "\xe2\x9c\x94" TERM_RESET :
        TERM_FG_RED "\xe2\x9c\x98" TERM_RESET;

    printf("\r\033[K%s %s\n", symbol, message ? message : "");
    fflush(stdout);

    free(p->message);
    free(p);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PROGRESS BAR API
 * ═══════════════════════════════════════════════════════════════════════════ */

tbos_progress_t* tbos_progress_start(const char* message, size_t total) {
    tbos_progress_t* p = (tbos_progress_t*)calloc(1, sizeof(tbos_progress_t));
    if (!p) return NULL;

    p->type = PROGRESS_BAR;
    p->message = message ? strdup(message) : NULL;
    p->total = total > 0 ? total : 1;
    p->current = 0;

    /* Initial render */
    printf("\r%s [                    ] 0%%", p->message ? p->message : "");
    fflush(stdout);

    return p;
}

void tbos_progress_update(tbos_progress_t* p, size_t current) {
    if (!p || p->type != PROGRESS_BAR) return;

    p->current = current;

    /* Calculate percentage */
    int percent = (int)((p->current * 100) / p->total);
    if (percent > 100) percent = 100;

    /* Calculate bar width (20 chars) */
    int filled = (percent * 20) / 100;

    /* Build bar */
    char bar[22];
    for (int i = 0; i < 20; i++) {
        if (i < filled) {
            bar[i] = '\xe2' < 128 ? '#' : '#';  /* Use # for ASCII safety */
        } else {
            bar[i] = ' ';
        }
    }
    bar[20] = '\0';

    /* Use Unicode block characters if terminal supports it */
    printf("\r\033[K%s [", p->message ? p->message : "");

    /* Print filled portion in green */
    printf(TERM_FG_GREEN);
    for (int i = 0; i < filled; i++) {
        printf("\xe2\x96\x88");  /* Full block █ */
    }
    printf(TERM_RESET);

    /* Print empty portion */
    printf(TERM_FG_BRIGHT_BLACK);
    for (int i = filled; i < 20; i++) {
        printf("\xe2\x96\x91");  /* Light shade ░ */
    }
    printf(TERM_RESET);

    printf("] %d%%", percent);
    fflush(stdout);
}

void tbos_progress_update_msg(tbos_progress_t* p, size_t current,
                               const char* message) {
    if (!p || p->type != PROGRESS_BAR) return;

    free(p->message);
    p->message = message ? strdup(message) : NULL;

    tbos_progress_update(p, current);
}

void tbos_progress_increment(tbos_progress_t* p, size_t delta) {
    if (!p || p->type != PROGRESS_BAR) return;
    tbos_progress_update(p, p->current + delta);
}

void tbos_progress_finish(tbos_progress_t* p) {
    if (!p) return;

    if (p->type == PROGRESS_BAR) {
        /* Force 100% */
        tbos_progress_update(p, p->total);
        printf("\n");
    }

    free(p->message);
    free(p);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * STATUS MESSAGES
 * ═══════════════════════════════════════════════════════════════════════════ */

void tbos_status_ok(const char* message) {
    printf(TERM_FG_GREEN "[" TERM_BOLD "OK" TERM_RESET TERM_FG_GREEN "]" TERM_RESET " %s\n",
           message ? message : "");
}

void tbos_status_fail(const char* message) {
    printf(TERM_FG_RED "[" TERM_BOLD "FAIL" TERM_RESET TERM_FG_RED "]" TERM_RESET " %s\n",
           message ? message : "");
}

void tbos_status_warn(const char* message) {
    printf(TERM_FG_YELLOW "[" TERM_BOLD "WARN" TERM_RESET TERM_FG_YELLOW "]" TERM_RESET " %s\n",
           message ? message : "");
}

void tbos_status_info(const char* message) {
    printf(TERM_FG_BLUE "[" TERM_BOLD "INFO" TERM_RESET TERM_FG_BLUE "]" TERM_RESET " %s\n",
           message ? message : "");
}

void tbos_status_skip(const char* message) {
    printf(TERM_FG_BRIGHT_BLACK "[" TERM_BOLD "SKIP" TERM_RESET TERM_FG_BRIGHT_BLACK "]" TERM_RESET " %s\n",
           message ? message : "");
}

/* ═══════════════════════════════════════════════════════════════════════════
 * UTILITY FUNCTIONS
 * ═══════════════════════════════════════════════════════════════════════════ */

const char* tbos_format_bytes(uint64_t bytes, char* buf, size_t size) {
    const char* units[] = { "B", "KB", "MB", "GB", "TB", "PB" };
    int unit = 0;
    double value = (double)bytes;

    while (value >= 1024.0 && unit < 5) {
        value /= 1024.0;
        unit++;
    }

    if (unit == 0) {
        snprintf(buf, size, "%llu %s", (unsigned long long)bytes, units[unit]);
    } else {
        snprintf(buf, size, "%.1f %s", value, units[unit]);
    }

    return buf;
}

const char* tbos_format_duration(double seconds, char* buf, size_t size) {
    if (seconds < 1.0) {
        snprintf(buf, size, "%.0f ms", seconds * 1000);
    } else if (seconds < 60.0) {
        snprintf(buf, size, "%.1f s", seconds);
    } else if (seconds < 3600.0) {
        int mins = (int)(seconds / 60);
        int secs = (int)seconds % 60;
        snprintf(buf, size, "%dm %ds", mins, secs);
    } else {
        int hours = (int)(seconds / 3600);
        int mins = ((int)seconds % 3600) / 60;
        snprintf(buf, size, "%dh %dm", hours, mins);
    }

    return buf;
}
