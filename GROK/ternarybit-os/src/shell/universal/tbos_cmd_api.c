/**
 * @file tbos_cmd_api.c
 * @brief TBOS Command API Implementation
 *
 * Provides clean command execution with structured output that can be
 * rendered to different formats (ANSI, JSON, HTML, plain text).
 */

#include "tbos_cmd_api.h"
#include "tbos_universal_shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Platform-specific includes */
#if defined(_WIN32) || defined(TBOS_ARCH_WINDOWS)
    #define TBOS_PLATFORM_WINDOWS 1
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
    #define pipe(fds) _pipe(fds, 4096, _O_BINARY)
    #define dup _dup
    #define dup2 _dup2
    #define fileno _fileno
    #define read _read
    #define close _close
#else
    #include <unistd.h>
#endif

/* ═══════════════════════════════════════════════════════════════════════════
 * INTERNAL HELPERS
 * ═══════════════════════════════════════════════════════════════════════════ */

static char* str_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

static void str_append(char** dest, size_t* len, size_t* cap, const char* src) {
    if (!src) return;
    size_t src_len = strlen(src);
    if (*len + src_len >= *cap) {
        *cap = (*cap + src_len) * 2;
        *dest = (char*)realloc(*dest, *cap);
    }
    memcpy(*dest + *len, src, src_len + 1);
    *len += src_len;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RESULT CREATION
 * ═══════════════════════════════════════════════════════════════════════════ */

static tbos_api_result_t* result_alloc(void) {
    tbos_api_result_t* r = (tbos_api_result_t*)calloc(1, sizeof(tbos_api_result_t));
    return r;
}

tbos_api_result_t* tbos_api_result_text(const char* text) {
    tbos_api_result_t* r = result_alloc();
    if (!r) return NULL;
    r->status = TBOS_OK;
    r->type = TBOS_TYPE_TEXT;
    r->text = str_dup(text);
    return r;
}

tbos_api_result_t* tbos_result_error(tbos_status_t status, const char* message) {
    tbos_api_result_t* r = result_alloc();
    if (!r) return NULL;
    r->status = status;
    r->type = TBOS_TYPE_TEXT;
    r->error = str_dup(message);
    return r;
}

tbos_api_result_t* tbos_result_ok(void) {
    tbos_api_result_t* r = result_alloc();
    if (!r) return NULL;
    r->status = TBOS_OK;
    r->type = TBOS_TYPE_EMPTY;
    return r;
}

tbos_api_result_t* tbos_api_result_table(char** headers, size_t header_count) {
    tbos_api_result_t* r = result_alloc();
    if (!r) return NULL;
    r->status = TBOS_OK;
    r->type = TBOS_TYPE_TABLE;
    r->header_count = header_count;
    r->headers = (char**)calloc(header_count, sizeof(char*));
    for (size_t i = 0; i < header_count; i++) {
        r->headers[i] = str_dup(headers[i]);
    }
    return r;
}

int tbos_result_add_row(tbos_api_result_t* result, tbos_cell_t* cells, size_t count) {
    if (!result || result->type != TBOS_TYPE_TABLE) return -1;

    result->rows = (tbos_row_t*)realloc(result->rows,
        (result->row_count + 1) * sizeof(tbos_row_t));

    tbos_row_t* row = &result->rows[result->row_count];
    row->count = count;
    row->cells = (tbos_cell_t*)calloc(count, sizeof(tbos_cell_t));

    for (size_t i = 0; i < count; i++) {
        row->cells[i].value = str_dup(cells[i].value);
        row->cells[i].color = cells[i].color ? str_dup(cells[i].color) : NULL;
        row->cells[i].bold = cells[i].bold;
    }

    result->row_count++;
    return 0;
}

tbos_api_result_t* tbos_result_list(void) {
    tbos_api_result_t* r = result_alloc();
    if (!r) return NULL;
    r->status = TBOS_OK;
    r->type = TBOS_TYPE_LIST;
    return r;
}

int tbos_result_add_item(tbos_api_result_t* result, const char* item) {
    if (!result || result->type != TBOS_TYPE_LIST) return -1;

    result->items = (char**)realloc(result->items,
        (result->item_count + 1) * sizeof(char*));
    result->items[result->item_count] = str_dup(item);
    result->item_count++;
    return 0;
}

tbos_api_result_t* tbos_result_keyval(void) {
    tbos_api_result_t* r = result_alloc();
    if (!r) return NULL;
    r->status = TBOS_OK;
    r->type = TBOS_TYPE_KEYVAL;
    return r;
}

int tbos_result_add_kv(tbos_api_result_t* result, const char* key, const char* value) {
    if (!result || result->type != TBOS_TYPE_KEYVAL) return -1;

    result->keys = (char**)realloc(result->keys,
        (result->kv_count + 1) * sizeof(char*));
    result->values = (char**)realloc(result->values,
        (result->kv_count + 1) * sizeof(char*));

    result->keys[result->kv_count] = str_dup(key);
    result->values[result->kv_count] = str_dup(value);
    result->kv_count++;
    return 0;
}

void tbos_result_free(tbos_api_result_t* result) {
    if (!result) return;

    free(result->text);
    free(result->error);

    for (size_t i = 0; i < result->header_count; i++) {
        free(result->headers[i]);
    }
    free(result->headers);

    for (size_t i = 0; i < result->row_count; i++) {
        for (size_t j = 0; j < result->rows[i].count; j++) {
            free(result->rows[i].cells[j].value);
            free(result->rows[i].cells[j].color);
        }
        free(result->rows[i].cells);
    }
    free(result->rows);

    for (size_t i = 0; i < result->item_count; i++) {
        free(result->items[i]);
    }
    free(result->items);

    for (size_t i = 0; i < result->kv_count; i++) {
        free(result->keys[i]);
        free(result->values[i]);
    }
    free(result->keys);
    free(result->values);

    free(result);
}

/* ═══════════════════════════════════════════════════════════════════════════
 * CONTEXT MANAGEMENT
 * ═══════════════════════════════════════════════════════════════════════════ */

tbos_api_context_t* tbos_api_context_create(tbos_output_format_t format) {
    tbos_api_context_t* ctx = (tbos_api_context_t*)calloc(1, sizeof(tbos_api_context_t));
    if (!ctx) return NULL;
    ctx->format = format;

    /* Get current directory */
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        ctx->cwd = str_dup(cwd);
    } else {
        ctx->cwd = str_dup("/");
    }

    return ctx;
}

void tbos_context_free(tbos_api_context_t* ctx) {
    if (!ctx) return;
    free(ctx->cwd);
    for (size_t i = 0; i < ctx->env_count; i++) {
        free(ctx->env[i]);
    }
    free(ctx->env);
    free(ctx);
}

tbos_output_format_t tbos_format_from_env(void) {
    const char* fmt = getenv("TBOS_OUTPUT_FORMAT");
    if (!fmt) return TBOS_FMT_ANSI;

    if (strcmp(fmt, "json") == 0) return TBOS_FMT_JSON;
    if (strcmp(fmt, "html") == 0) return TBOS_FMT_HTML;
    if (strcmp(fmt, "plain") == 0) return TBOS_FMT_PLAIN;
    return TBOS_FMT_ANSI;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING - JSON
 * ═══════════════════════════════════════════════════════════════════════════ */

static char* escape_json_string(const char* s) {
    if (!s) return str_dup("null");

    /* Calculate needed length - control chars need \uXXXX (6 chars) */
    size_t len = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; p++) {
        if (*p == '"' || *p == '\\') {
            len += 2;
        } else if (*p == '\n' || *p == '\r' || *p == '\t') {
            len += 2;
        } else if (*p < 0x20) {
            len += 6;  /* \uXXXX */
        } else {
            len++;
        }
    }

    char* out = (char*)malloc(len + 3);  /* quotes + null */
    char* q = out;
    *q++ = '"';
    for (const unsigned char* p = (const unsigned char*)s; *p; p++) {
        if (*p == '"') {
            *q++ = '\\'; *q++ = '"';
        } else if (*p == '\\') {
            *q++ = '\\'; *q++ = '\\';
        } else if (*p == '\n') {
            *q++ = '\\'; *q++ = 'n';
        } else if (*p == '\r') {
            *q++ = '\\'; *q++ = 'r';
        } else if (*p == '\t') {
            *q++ = '\\'; *q++ = 't';
        } else if (*p < 0x20) {
            /* Escape control characters as \uXXXX */
            q += sprintf(q, "\\u%04x", *p);
        } else {
            *q++ = *p;
        }
    }
    *q++ = '"';
    *q = '\0';
    return out;
}

static char* render_json(tbos_api_result_t* result) {
    char* out = NULL;
    size_t len = 0, cap = 256;
    out = (char*)malloc(cap);
    out[0] = '\0';

    str_append(&out, &len, &cap, "{");

    /* Status */
    str_append(&out, &len, &cap, "\"status\":");
    str_append(&out, &len, &cap, result->status == TBOS_OK ? "\"ok\"" : "\"error\"");

    /* Error message if any */
    if (result->error) {
        str_append(&out, &len, &cap, ",\"error\":");
        char* escaped = escape_json_string(result->error);
        str_append(&out, &len, &cap, escaped);
        free(escaped);
    }

    /* Type-specific output */
    switch (result->type) {
        case TBOS_TYPE_TEXT:
            if (result->text) {
                str_append(&out, &len, &cap, ",\"type\":\"text\",\"output\":");
                char* escaped = escape_json_string(result->text);
                str_append(&out, &len, &cap, escaped);
                free(escaped);
            }
            break;

        case TBOS_TYPE_TABLE:
            str_append(&out, &len, &cap, ",\"type\":\"table\",\"headers\":[");
            for (size_t i = 0; i < result->header_count; i++) {
                if (i > 0) str_append(&out, &len, &cap, ",");
                char* escaped = escape_json_string(result->headers[i]);
                str_append(&out, &len, &cap, escaped);
                free(escaped);
            }
            str_append(&out, &len, &cap, "],\"rows\":[");
            for (size_t i = 0; i < result->row_count; i++) {
                if (i > 0) str_append(&out, &len, &cap, ",");
                str_append(&out, &len, &cap, "[");
                for (size_t j = 0; j < result->rows[i].count; j++) {
                    if (j > 0) str_append(&out, &len, &cap, ",");
                    char* escaped = escape_json_string(result->rows[i].cells[j].value);
                    str_append(&out, &len, &cap, escaped);
                    free(escaped);
                }
                str_append(&out, &len, &cap, "]");
            }
            str_append(&out, &len, &cap, "]");
            break;

        case TBOS_TYPE_LIST:
            str_append(&out, &len, &cap, ",\"type\":\"list\",\"items\":[");
            for (size_t i = 0; i < result->item_count; i++) {
                if (i > 0) str_append(&out, &len, &cap, ",");
                char* escaped = escape_json_string(result->items[i]);
                str_append(&out, &len, &cap, escaped);
                free(escaped);
            }
            str_append(&out, &len, &cap, "]");
            break;

        case TBOS_TYPE_KEYVAL:
            str_append(&out, &len, &cap, ",\"type\":\"keyval\",\"data\":{");
            for (size_t i = 0; i < result->kv_count; i++) {
                if (i > 0) str_append(&out, &len, &cap, ",");
                char* key = escape_json_string(result->keys[i]);
                char* val = escape_json_string(result->values[i]);
                str_append(&out, &len, &cap, key);
                str_append(&out, &len, &cap, ":");
                str_append(&out, &len, &cap, val);
                free(key);
                free(val);
            }
            str_append(&out, &len, &cap, "}");
            break;

        case TBOS_TYPE_EMPTY:
            str_append(&out, &len, &cap, ",\"type\":\"empty\"");
            break;

        default:
            break;
    }

    str_append(&out, &len, &cap, "}");
    return out;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING - HTML
 * ═══════════════════════════════════════════════════════════════════════════ */

static char* escape_html(const char* s) {
    if (!s) return str_dup("");

    size_t len = 0;
    for (const char* p = s; *p; p++) {
        switch (*p) {
            case '<': case '>': len += 4; break;
            case '&': len += 5; break;
            case '"': len += 6; break;
            default: len++; break;
        }
    }

    char* out = (char*)malloc(len + 1);
    char* q = out;
    for (const char* p = s; *p; p++) {
        switch (*p) {
            case '<': memcpy(q, "&lt;", 4); q += 4; break;
            case '>': memcpy(q, "&gt;", 4); q += 4; break;
            case '&': memcpy(q, "&amp;", 5); q += 5; break;
            case '"': memcpy(q, "&quot;", 6); q += 6; break;
            case '\n': *q++ = '\n'; break;
            default: *q++ = *p; break;
        }
    }
    *q = '\0';
    return out;
}

static char* render_html(tbos_api_result_t* result) {
    char* out = NULL;
    size_t len = 0, cap = 256;
    out = (char*)malloc(cap);
    out[0] = '\0';

    if (result->status != TBOS_OK && result->error) {
        str_append(&out, &len, &cap, "<div class=\"error\">");
        char* escaped = escape_html(result->error);
        str_append(&out, &len, &cap, escaped);
        free(escaped);
        str_append(&out, &len, &cap, "</div>");
        return out;
    }

    switch (result->type) {
        case TBOS_TYPE_TEXT:
            str_append(&out, &len, &cap, "<pre class=\"output\">");
            if (result->text) {
                char* escaped = escape_html(result->text);
                str_append(&out, &len, &cap, escaped);
                free(escaped);
            }
            str_append(&out, &len, &cap, "</pre>");
            break;

        case TBOS_TYPE_TABLE:
            str_append(&out, &len, &cap, "<table class=\"tbos-table\"><thead><tr>");
            for (size_t i = 0; i < result->header_count; i++) {
                str_append(&out, &len, &cap, "<th>");
                char* escaped = escape_html(result->headers[i]);
                str_append(&out, &len, &cap, escaped);
                free(escaped);
                str_append(&out, &len, &cap, "</th>");
            }
            str_append(&out, &len, &cap, "</tr></thead><tbody>");
            for (size_t i = 0; i < result->row_count; i++) {
                str_append(&out, &len, &cap, "<tr>");
                for (size_t j = 0; j < result->rows[i].count; j++) {
                    tbos_cell_t* cell = &result->rows[i].cells[j];
                    str_append(&out, &len, &cap, "<td");
                    if (cell->color || cell->bold) {
                        str_append(&out, &len, &cap, " class=\"");
                        if (cell->color) {
                            str_append(&out, &len, &cap, "c-");
                            str_append(&out, &len, &cap, cell->color);
                        }
                        if (cell->bold) {
                            if (cell->color) str_append(&out, &len, &cap, " ");
                            str_append(&out, &len, &cap, "c-bold");
                        }
                        str_append(&out, &len, &cap, "\"");
                    }
                    str_append(&out, &len, &cap, ">");
                    char* escaped = escape_html(cell->value);
                    str_append(&out, &len, &cap, escaped);
                    free(escaped);
                    str_append(&out, &len, &cap, "</td>");
                }
                str_append(&out, &len, &cap, "</tr>");
            }
            str_append(&out, &len, &cap, "</tbody></table>");
            break;

        case TBOS_TYPE_LIST:
            str_append(&out, &len, &cap, "<ul class=\"tbos-list\">");
            for (size_t i = 0; i < result->item_count; i++) {
                str_append(&out, &len, &cap, "<li>");
                char* escaped = escape_html(result->items[i]);
                str_append(&out, &len, &cap, escaped);
                free(escaped);
                str_append(&out, &len, &cap, "</li>");
            }
            str_append(&out, &len, &cap, "</ul>");
            break;

        case TBOS_TYPE_KEYVAL:
            str_append(&out, &len, &cap, "<dl class=\"tbos-kv\">");
            for (size_t i = 0; i < result->kv_count; i++) {
                str_append(&out, &len, &cap, "<dt>");
                char* key = escape_html(result->keys[i]);
                str_append(&out, &len, &cap, key);
                free(key);
                str_append(&out, &len, &cap, "</dt><dd>");
                char* val = escape_html(result->values[i]);
                str_append(&out, &len, &cap, val);
                free(val);
                str_append(&out, &len, &cap, "</dd>");
            }
            str_append(&out, &len, &cap, "</dl>");
            break;

        case TBOS_TYPE_EMPTY:
            /* No output */
            break;

        default:
            break;
    }

    return out;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING - PLAIN TEXT
 * ═══════════════════════════════════════════════════════════════════════════ */

static char* render_plain(tbos_api_result_t* result) {
    char* out = NULL;
    size_t len = 0, cap = 256;
    out = (char*)malloc(cap);
    out[0] = '\0';

    if (result->status != TBOS_OK && result->error) {
        str_append(&out, &len, &cap, "Error: ");
        str_append(&out, &len, &cap, result->error);
        str_append(&out, &len, &cap, "\n");
        return out;
    }

    switch (result->type) {
        case TBOS_TYPE_TEXT:
            if (result->text) {
                str_append(&out, &len, &cap, result->text);
            }
            break;

        case TBOS_TYPE_TABLE:
            /* Simple column-aligned output */
            for (size_t i = 0; i < result->header_count; i++) {
                if (i > 0) str_append(&out, &len, &cap, "\t");
                str_append(&out, &len, &cap, result->headers[i]);
            }
            str_append(&out, &len, &cap, "\n");
            for (size_t i = 0; i < result->row_count; i++) {
                for (size_t j = 0; j < result->rows[i].count; j++) {
                    if (j > 0) str_append(&out, &len, &cap, "\t");
                    str_append(&out, &len, &cap, result->rows[i].cells[j].value);
                }
                str_append(&out, &len, &cap, "\n");
            }
            break;

        case TBOS_TYPE_LIST:
            for (size_t i = 0; i < result->item_count; i++) {
                str_append(&out, &len, &cap, result->items[i]);
                str_append(&out, &len, &cap, "\n");
            }
            break;

        case TBOS_TYPE_KEYVAL:
            for (size_t i = 0; i < result->kv_count; i++) {
                str_append(&out, &len, &cap, result->keys[i]);
                str_append(&out, &len, &cap, ": ");
                str_append(&out, &len, &cap, result->values[i]);
                str_append(&out, &len, &cap, "\n");
            }
            break;

        case TBOS_TYPE_EMPTY:
            break;

        default:
            break;
    }

    return out;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * RENDERING - ANSI (Terminal)
 * ═══════════════════════════════════════════════════════════════════════════ */

static const char* ansi_color(const char* color) {
    if (!color) return "";
    if (strcmp(color, "red") == 0) return "\033[31m";
    if (strcmp(color, "green") == 0) return "\033[32m";
    if (strcmp(color, "yellow") == 0) return "\033[33m";
    if (strcmp(color, "blue") == 0) return "\033[34m";
    if (strcmp(color, "magenta") == 0) return "\033[35m";
    if (strcmp(color, "cyan") == 0) return "\033[36m";
    if (strcmp(color, "white") == 0) return "\033[37m";
    return "";
}

static char* render_ansi(tbos_api_result_t* result) {
    char* out = NULL;
    size_t len = 0, cap = 256;
    out = (char*)malloc(cap);
    out[0] = '\0';

    if (result->status != TBOS_OK && result->error) {
        str_append(&out, &len, &cap, "\033[1;31mError:\033[0m ");
        str_append(&out, &len, &cap, result->error);
        str_append(&out, &len, &cap, "\n");
        return out;
    }

    switch (result->type) {
        case TBOS_TYPE_TEXT:
            if (result->text) {
                str_append(&out, &len, &cap, result->text);
            }
            break;

        case TBOS_TYPE_TABLE:
            /* Headers in bold */
            str_append(&out, &len, &cap, "\033[1m");
            for (size_t i = 0; i < result->header_count; i++) {
                if (i > 0) str_append(&out, &len, &cap, "\t");
                str_append(&out, &len, &cap, result->headers[i]);
            }
            str_append(&out, &len, &cap, "\033[0m\n");

            for (size_t i = 0; i < result->row_count; i++) {
                for (size_t j = 0; j < result->rows[i].count; j++) {
                    if (j > 0) str_append(&out, &len, &cap, "\t");
                    tbos_cell_t* cell = &result->rows[i].cells[j];
                    if (cell->bold) str_append(&out, &len, &cap, "\033[1m");
                    str_append(&out, &len, &cap, ansi_color(cell->color));
                    str_append(&out, &len, &cap, cell->value);
                    if (cell->color || cell->bold) {
                        str_append(&out, &len, &cap, "\033[0m");
                    }
                }
                str_append(&out, &len, &cap, "\n");
            }
            break;

        case TBOS_TYPE_LIST:
            for (size_t i = 0; i < result->item_count; i++) {
                str_append(&out, &len, &cap, "  • ");
                str_append(&out, &len, &cap, result->items[i]);
                str_append(&out, &len, &cap, "\n");
            }
            break;

        case TBOS_TYPE_KEYVAL:
            for (size_t i = 0; i < result->kv_count; i++) {
                str_append(&out, &len, &cap, "\033[1m");
                str_append(&out, &len, &cap, result->keys[i]);
                str_append(&out, &len, &cap, ":\033[0m ");
                str_append(&out, &len, &cap, result->values[i]);
                str_append(&out, &len, &cap, "\n");
            }
            break;

        case TBOS_TYPE_EMPTY:
            break;

        default:
            break;
    }

    return out;
}

/* ═══════════════════════════════════════════════════════════════════════════
 * PUBLIC RENDER FUNCTION
 * ═══════════════════════════════════════════════════════════════════════════ */

char* tbos_result_render(tbos_api_result_t* result, tbos_output_format_t format) {
    if (!result) return str_dup("");

    switch (format) {
        case TBOS_FMT_JSON:  return render_json(result);
        case TBOS_FMT_HTML:  return render_html(result);
        case TBOS_FMT_PLAIN: return render_plain(result);
        case TBOS_FMT_ANSI:
        default:             return render_ansi(result);
    }
}

/* ═══════════════════════════════════════════════════════════════════════════
 * COMMAND EXECUTION
 * ═══════════════════════════════════════════════════════════════════════════ */

int tbos_cmd_init(void) {
    /* Initialize command registry if needed */
    return 0;
}

void tbos_cmd_shutdown(void) {
    /* Cleanup */
}

/* Parse command line into argc/argv */
static int parse_line(const char* line, char*** argv_out) {
    if (!line || !*line) return 0;

    /* Count words (simple tokenization) */
    int count = 0;
    bool in_word = false;
    bool in_quote = false;
    char quote_char = 0;

    for (const char* p = line; *p; p++) {
        if (!in_quote && (*p == '"' || *p == '\'')) {
            in_quote = true;
            quote_char = *p;
            if (!in_word) { in_word = true; count++; }
        } else if (in_quote && *p == quote_char) {
            in_quote = false;
        } else if (!in_quote && isspace((unsigned char)*p)) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            count++;
        }
    }

    if (count == 0) return 0;

    /* Allocate argv */
    char** argv = (char**)calloc(count + 1, sizeof(char*));

    /* Extract words */
    int idx = 0;
    const char* start = NULL;
    in_word = false;
    in_quote = false;
    quote_char = 0;

    for (const char* p = line; ; p++) {
        if (!in_quote && (*p == '"' || *p == '\'')) {
            in_quote = true;
            quote_char = *p;
            if (!in_word) { in_word = true; start = p + 1; }
        } else if (in_quote && *p == quote_char) {
            in_quote = false;
            /* End of quoted string */
            size_t len = p - start;
            argv[idx] = (char*)malloc(len + 1);
            memcpy(argv[idx], start, len);
            argv[idx][len] = '\0';
            idx++;
            in_word = false;
        } else if (!in_quote && (isspace((unsigned char)*p) || *p == '\0')) {
            if (in_word && start) {
                size_t len = p - start;
                argv[idx] = (char*)malloc(len + 1);
                memcpy(argv[idx], start, len);
                argv[idx][len] = '\0';
                idx++;
            }
            in_word = false;
            if (*p == '\0') break;
        } else if (!in_word) {
            in_word = true;
            start = p;
        }
    }

    *argv_out = argv;
    return idx;
}

static void free_argv(int argc, char** argv) {
    for (int i = 0; i < argc; i++) {
        free(argv[i]);
    }
    free(argv);
}

tbos_api_result_t* tbos_cmd_execute(const char* line, tbos_api_context_t* ctx) {
    char** argv = NULL;
    int argc = parse_line(line, &argv);

    if (argc == 0) {
        return tbos_result_ok();
    }

    tbos_api_result_t* result = tbos_cmd_execute_argv(argc, argv, ctx);
    free_argv(argc, argv);
    return result;
}

tbos_api_result_t* tbos_cmd_execute_argv(int argc, char** argv, tbos_api_context_t* ctx) {
    if (argc == 0 || !argv || !argv[0]) {
        return tbos_result_ok();
    }

    /* Find command in registry */
    const universal_command_t* cmd = universal_find_command(argv[0]);

    if (!cmd) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Command not found: %s", argv[0]);
        return tbos_result_error(TBOS_NOT_FOUND, msg);
    }

    /*
     * For now, we capture the command's stdout output.
     * In the future, commands can be refactored to return tbos_api_result_t directly.
     */

    /* Create a pipe to capture output */
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        return tbos_result_error(TBOS_ERROR, "Failed to create pipe");
    }

    /* Save original stdout */
    int saved_stdout = dup(STDOUT_FILENO);

    /* Redirect stdout to pipe */
    dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);

    /* Execute command */
    int ret = cmd->handler(argc, argv);

    /* Flush and restore stdout */
    fflush(stdout);
    dup2(saved_stdout, STDOUT_FILENO);
    close(saved_stdout);

    /* Read captured output */
    char buffer[8192];
    ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    if (n > 0) {
        buffer[n] = '\0';
        tbos_api_result_t* result = tbos_api_result_text(buffer);
        if (ret != 0) {
            result->status = TBOS_ERROR;
        }
        return result;
    }

    return ret == 0 ? tbos_result_ok() :
                      tbos_result_error(TBOS_ERROR, "Command failed");
}
