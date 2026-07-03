/**
 * steppps.c - Simple STEPPPS Runtime
 *
 * A minimal runtime to execute STEPPPS JSON files.
 * Each STEPPPS is self-contained with its own code, metadata, and context.
 *
 * Usage: steppps <file.steppps.json> [action]
 *
 * Build: gcc -o steppps steppps.c -DHOST_BUILD
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#define MAX_JSON_SIZE (1024 * 1024)  /* 1MB max */
#define MAX_FIELD_SIZE 4096

/* Simple JSON value extraction (no external deps) */
static char* json_get_string(const char* json, const char* key, char* buf, size_t len) {
    char search[256];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char* pos = strstr(json, search);
    if (!pos) return NULL;

    pos = strchr(pos + strlen(search), ':');
    if (!pos) return NULL;

    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t' || *pos == '\n')) pos++;

    if (*pos == '"') {
        pos++;
        size_t i = 0;
        while (*pos && *pos != '"' && i < len - 1) {
            if (*pos == '\\' && *(pos + 1)) {
                pos++;
                switch (*pos) {
                    case 'n': buf[i++] = '\n'; break;
                    case 't': buf[i++] = '\t'; break;
                    case '\\': buf[i++] = '\\'; break;
                    case '"': buf[i++] = '"'; break;
                    default: buf[i++] = *pos;
                }
            } else {
                buf[i++] = *pos;
            }
            pos++;
        }
        buf[i] = '\0';
        return buf;
    }

    return NULL;
}

static int json_get_int(const char* json, const char* key, int def) {
    char search[256];
    snprintf(search, sizeof(search), "\"%s\"", key);

    const char* pos = strstr(json, search);
    if (!pos) return def;

    pos = strchr(pos + strlen(search), ':');
    if (!pos) return def;

    while (*pos && (*pos == ':' || *pos == ' ' || *pos == '\t' || *pos == '\n')) pos++;

    return atoi(pos);
}

/* STEPPPS structure (parsed from JSON) */
typedef struct {
    char id[128];
    char name[256];

    /* Space */
    char space_universe[64];
    char space_device[128];
    char space_path[256];

    /* Time */
    char time_utc[64];
    char time_created[64];

    /* Event */
    char event_type[32];
    char event_description[512];
    char event_severity[16];

    /* Psychology */
    int psych_health;
    int psych_karma;
    char psych_consciousness[32];
    char psych_intent[256];

    /* Prompt */
    char prompt_system[1024];
    char prompt_user[1024];
    char prompt_next[1024];

    /* Pixel */
    char pixel_type[32];
    char pixel_template[2048];

    /* Script */
    char script_lang[16];
    char script_code[MAX_FIELD_SIZE];
    char script_entry[64];
    bool script_sandbox;

    /* Meta */
    char meta_author[64];
    int meta_karma_cost;
    int meta_karma_reward;
} steppps_t;

/* Parse STEPPPS from JSON */
static int parse_steppps(const char* json, steppps_t* s) {
    memset(s, 0, sizeof(steppps_t));

    json_get_string(json, "id", s->id, sizeof(s->id));
    json_get_string(json, "name", s->name, sizeof(s->name));

    /* Find nested objects and extract */
    const char* space = strstr(json, "\"space\"");
    if (space) {
        json_get_string(space, "universe", s->space_universe, sizeof(s->space_universe));
        json_get_string(space, "device", s->space_device, sizeof(s->space_device));
        json_get_string(space, "path", s->space_path, sizeof(s->space_path));
    }

    const char* time_obj = strstr(json, "\"time\"");
    if (time_obj) {
        json_get_string(time_obj, "utc", s->time_utc, sizeof(s->time_utc));
        json_get_string(time_obj, "created", s->time_created, sizeof(s->time_created));
    }

    const char* event = strstr(json, "\"event\"");
    if (event) {
        json_get_string(event, "type", s->event_type, sizeof(s->event_type));
        json_get_string(event, "description", s->event_description, sizeof(s->event_description));
        json_get_string(event, "severity", s->event_severity, sizeof(s->event_severity));
    }

    const char* psych = strstr(json, "\"psychology\"");
    if (psych) {
        const char* sys = strstr(psych, "\"system\"");
        if (sys) {
            s->psych_health = json_get_int(sys, "health", 100);
            s->psych_karma = json_get_int(sys, "karma", 0);
            json_get_string(sys, "consciousness", s->psych_consciousness, sizeof(s->psych_consciousness));
        }
        json_get_string(psych, "intent", s->psych_intent, sizeof(s->psych_intent));
    }

    const char* prompt = strstr(json, "\"prompt\"");
    if (prompt) {
        json_get_string(prompt, "system", s->prompt_system, sizeof(s->prompt_system));
        json_get_string(prompt, "user", s->prompt_user, sizeof(s->prompt_user));
        json_get_string(prompt, "next", s->prompt_next, sizeof(s->prompt_next));
    }

    const char* pixel = strstr(json, "\"pixel\"");
    if (pixel) {
        json_get_string(pixel, "type", s->pixel_type, sizeof(s->pixel_type));
        json_get_string(pixel, "template", s->pixel_template, sizeof(s->pixel_template));
    }

    const char* script = strstr(json, "\"script\"");
    if (script) {
        json_get_string(script, "lang", s->script_lang, sizeof(s->script_lang));
        json_get_string(script, "code", s->script_code, sizeof(s->script_code));
        json_get_string(script, "entry", s->script_entry, sizeof(s->script_entry));
        s->script_sandbox = (strstr(script, "\"sandbox\": true") != NULL ||
                            strstr(script, "\"sandbox\":true") != NULL);
    }

    const char* meta = strstr(json, "\"meta\"");
    if (meta) {
        json_get_string(meta, "author", s->meta_author, sizeof(s->meta_author));
        s->meta_karma_cost = json_get_int(meta, "karma_cost", 0);
        s->meta_karma_reward = json_get_int(meta, "karma_reward", 1);
    }

    return (s->id[0] != '\0') ? 0 : -1;
}

/* Display STEPPPS info */
static void display_steppps(const steppps_t* s) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║  STEPPPS: %-50s  ║\n", s->name);
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  ID: %-55s  ║\n", s->id);
    printf("╠══════════════════════════════════════════════════════════════╣\n");

    printf("║  S (Space)                                                   ║\n");
    printf("║    Universe: %-47s  ║\n", s->space_universe[0] ? s->space_universe : "earth");
    printf("║    Device:   %-47s  ║\n", s->space_device[0] ? s->space_device : "(local)");

    printf("║  T (Time)                                                    ║\n");
    printf("║    UTC:      %-47s  ║\n", s->time_utc[0] ? s->time_utc : "(now)");

    printf("║  E (Event)                                                   ║\n");
    printf("║    Type:     %-47s  ║\n", s->event_type);
    printf("║    Severity: %-47s  ║\n", s->event_severity);

    printf("║  P (Psychology)                                              ║\n");
    printf("║    Health: %d%%  Karma: %d  Consciousness: %-18s  ║\n",
           s->psych_health, s->psych_karma, s->psych_consciousness);

    printf("║  P (Prompt)                                                  ║\n");
    if (s->prompt_user[0]) {
        printf("║    User: %-50.50s  ║\n", s->prompt_user);
    }

    printf("║  P (Pixel)                                                   ║\n");
    printf("║    Type: %-50s  ║\n", s->pixel_type);

    printf("║  S (Script)                                                  ║\n");
    printf("║    Lang: %-50s  ║\n", s->script_lang);
    printf("║    Sandbox: %-48s  ║\n", s->script_sandbox ? "Yes" : "No");

    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  Meta: Author=%s  Karma: -%d/+%d                           ║\n",
           s->meta_author[0] ? s->meta_author : "unknown",
           s->meta_karma_cost, s->meta_karma_reward);
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

/* Execute STEPPPS script */
static int execute_steppps(const steppps_t* s) {
    if (!s->script_code[0]) {
        printf("No script to execute.\n");
        return 0;
    }

    printf("\n--- Executing Script (%s) ---\n\n", s->script_lang);

    /* Set environment variables for script context */
    setenv("STEPPPS_ID", s->id, 1);
    setenv("STEPPPS_NAME", s->name, 1);
    setenv("STEPPPS_SPACE_UNIVERSE", s->space_universe, 1);
    setenv("STEPPPS_SPACE_PATH", s->space_path, 1);
    setenv("STEPPPS_TIME_UTC", s->time_utc, 1);
    setenv("STEPPPS_EVENT_TYPE", s->event_type, 1);

    char karma_str[16];
    snprintf(karma_str, sizeof(karma_str), "%d", s->psych_karma);
    setenv("STEPPPS_KARMA", karma_str, 1);
    setenv("STEPPPS_CONSCIOUSNESS", s->psych_consciousness, 1);

    if (strcmp(s->script_lang, "sh") == 0) {
        /* Execute shell script */
        int rc = system(s->script_code);
        return WEXITSTATUS(rc);
    }
    else if (strcmp(s->script_lang, "json") == 0) {
        /* JSON is data, just display it */
        printf("%s\n", s->script_code);
        return 0;
    }
    else if (strcmp(s->script_lang, "c") == 0) {
        printf("[C code would be compiled and executed]\n");
        printf("%s\n", s->script_code);
        return 0;
    }
    else {
        printf("[%s script]\n%s\n", s->script_lang, s->script_code);
        return 0;
    }
}

/* Render pixel output */
static void render_pixel(const steppps_t* s) {
    if (!s->pixel_template[0]) return;

    printf("\n--- Pixel Output (%s) ---\n\n", s->pixel_type);

    /* Simple template rendering - replace {{var}} */
    char output[4096];
    strncpy(output, s->pixel_template, sizeof(output) - 1);

    /* Replace some known variables */
    char* pos;
    while ((pos = strstr(output, "{{name}}")) != NULL) {
        char temp[4096];
        *pos = '\0';
        snprintf(temp, sizeof(temp), "%s%s%s", output, s->name, pos + 8);
        strncpy(output, temp, sizeof(output) - 1);
    }
    while ((pos = strstr(output, "{{karma}}")) != NULL) {
        char temp[4096];
        *pos = '\0';
        snprintf(temp, sizeof(temp), "%s%d%s", output, s->psych_karma, pos + 9);
        strncpy(output, temp, sizeof(output) - 1);
    }

    printf("%s\n", output);
}

/* Main */
int main(int argc, char** argv) {
    if (argc < 2) {
        printf("STEPPPS Runtime v1.0\n");
        printf("Usage: %s <file.steppps.json> [run|show|prompt]\n", argv[0]);
        printf("\nActions:\n");
        printf("  show   - Display STEPPPS info (default)\n");
        printf("  run    - Execute the script\n");
        printf("  pixel  - Render pixel output\n");
        printf("  prompt - Show AI prompt\n");
        return 0;
    }

    /* Read STEPPPS file */
    FILE* f = fopen(argv[1], "r");
    if (!f) {
        fprintf(stderr, "Error: Cannot open %s\n", argv[1]);
        return 1;
    }

    char* json = malloc(MAX_JSON_SIZE);
    if (!json) {
        fclose(f);
        return 1;
    }

    size_t len = fread(json, 1, MAX_JSON_SIZE - 1, f);
    json[len] = '\0';
    fclose(f);

    /* Parse */
    steppps_t steppps;
    if (parse_steppps(json, &steppps) != 0) {
        fprintf(stderr, "Error: Invalid STEPPPS file\n");
        free(json);
        return 1;
    }

    /* Determine action */
    const char* action = (argc > 2) ? argv[2] : "show";

    if (strcmp(action, "show") == 0) {
        display_steppps(&steppps);
    }
    else if (strcmp(action, "run") == 0) {
        display_steppps(&steppps);
        int rc = execute_steppps(&steppps);
        printf("\n--- Script exited with code %d ---\n", rc);
        printf("Karma reward: +%d\n", steppps.meta_karma_reward);
    }
    else if (strcmp(action, "pixel") == 0) {
        render_pixel(&steppps);
    }
    else if (strcmp(action, "prompt") == 0) {
        printf("\n=== STEPPPS AI Prompt ===\n\n");
        printf("System: %s\n\n", steppps.prompt_system);
        printf("User: %s\n\n", steppps.prompt_user);
        printf("Next: %s\n", steppps.prompt_next);
    }
    else {
        fprintf(stderr, "Unknown action: %s\n", action);
    }

    free(json);
    return 0;
}
