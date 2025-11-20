// TEST_DEPS: shell/shell_morph.c shell/tbos_interpreter.c shell/sh_interpreter.c
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "tbos/shell_morph.h"

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT_TRUE(cond, msg) \
    do { \
        if (cond) { \
            printf("  ✅ %s\n", msg); \
            tests_passed++; \
        } else { \
            printf("  ❌ %s\n", msg); \
            tests_failed++; \
        } \
    } while (0)

/* ------------------------------------------------------------------------- */
/* Kernel stubs                                                              */
/* ------------------------------------------------------------------------- */

static char print_log[4096];

static void reset_print_log(void) {
    print_log[0] = '\0';
}

void kernel_print(const char* str) {
    if (!str) return;
    size_t current = strlen(print_log);
    size_t remaining = sizeof(print_log) - current - 1;
    if (remaining == 0) return;
    strncat(print_log, str, remaining);
}

static char last_shell_command[512];
static int shell_exec_calls = 0;

int shell_execute_command(const char* cmdline) {
    shell_exec_calls++;
    if (!cmdline) {
        last_shell_command[0] = '\0';
        return -1;
    }
    strncpy(last_shell_command, cmdline, sizeof(last_shell_command) - 1);
    last_shell_command[sizeof(last_shell_command) - 1] = '\0';
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Helpers                                                                   */
/* ------------------------------------------------------------------------- */

static void init_shells(void) {
    reset_print_log();
    shell_exec_calls = 0;
    last_shell_command[0] = '\0';

    shell_morph_init();

    int rc = shell_morph_register(&shell_tbos_interpreter);
    ASSERT_TRUE(rc == 0, "Registered TBOS interpreter");

    rc = shell_morph_register(&shell_sh_interpreter);
    ASSERT_TRUE(rc == 0, "Registered POSIX sh interpreter");
}

/* ------------------------------------------------------------------------- */
/* Tests                                                                     */
/* ------------------------------------------------------------------------- */

static void test_default_shell_is_tbos(void) {
    printf("\n[TEST] Default shell registration\n");
    init_shells();

    const shell_interpreter_t* current = shell_morph_current();
    ASSERT_TRUE(current != NULL, "Current shell is not NULL");
    ASSERT_TRUE(current->type == SHELL_TBOS, "TBOS interpreter becomes default");
    ASSERT_TRUE(strstr(print_log, "ch-sh sh") != NULL,
                "TBOS init advertises ch-sh transition hint");
}

static void test_switch_to_sh_and_back(void) {
    printf("\n[TEST] Switching between TBOS and sh\n");
    init_shells();

    reset_print_log();
    int rc = shell_morph_switch("sh");
    ASSERT_TRUE(rc == 0, "Switch to sh succeeds");
    const shell_interpreter_t* current = shell_morph_current();
    ASSERT_TRUE(current && current->type == SHELL_SH, "Active shell updated to sh");
    ASSERT_TRUE(strstr(print_log, "ch-sh tbos") != NULL,
                "sh init reminds how to return via ch-sh");

    reset_print_log();
    rc = shell_morph_switch("tbos");
    ASSERT_TRUE(rc == 0, "Switch back to tbos succeeds");
    current = shell_morph_current();
    ASSERT_TRUE(current && current->type == SHELL_TBOS, "Active shell back to TBOS");
    ASSERT_TRUE(strstr(print_log, "ch-sh sh") != NULL,
                "TBOS re-entry advertises POSIX switch");
}

static void test_shell_list_marks_current(void) {
    printf("\n[TEST] Listing shells marks current interpreter\n");
    init_shells();

    char buffer[128];
    int rc = shell_morph_list(buffer, sizeof(buffer));
    ASSERT_TRUE(rc == 0, "List shells succeeds");
    ASSERT_TRUE(strstr(buffer, "tbos*") != NULL, "Current TBOS shell marked with *");

    shell_morph_switch("sh");
    rc = shell_morph_list(buffer, sizeof(buffer));
    ASSERT_TRUE(rc == 0, "List shells after switch succeeds");
    ASSERT_TRUE(strstr(buffer, "sh*") != NULL, "sh shell marked as current after switch");
}

static void test_execute_routes_to_current_shell(void) {
    printf("\n[TEST] shell_morph_execute routes through active interpreter\n");
    init_shells();
    shell_exec_calls = 0;

    int rc = shell_morph_execute("pwd");
    ASSERT_TRUE(rc == 0, "TBOS interpreter executes command");
    ASSERT_TRUE(shell_exec_calls == 1, "TBOS execution hits kernel dispatcher");
    ASSERT_TRUE(strcmp(last_shell_command, "pwd") == 0, "TBOS command forwarded verbatim");

    shell_morph_switch("sh");
    shell_exec_calls = 0;
    rc = shell_morph_execute("pwd");
    ASSERT_TRUE(rc == 0, "POSIX sh routes non built-ins to TBOS dispatcher");
    ASSERT_TRUE(shell_exec_calls == 1, "sh interpreter calls kernel dispatcher");
    ASSERT_TRUE(strcmp(last_shell_command, "pwd") == 0, "sh forwarded pwd command");
}

static void test_switch_invalid_shell(void) {
    printf("\n[TEST] Switching to invalid shell fails gracefully\n");
    init_shells();

    int rc = shell_morph_switch("invalid");
    ASSERT_TRUE(rc == -2, "Unknown shell returns -2");
    const shell_interpreter_t* current = shell_morph_current();
    ASSERT_TRUE(current && current->type == SHELL_TBOS, "Current shell unchanged on failure");
}

int main(void) {
    printf("\n=== Shell Morphing / ch-sh Dispatcher Tests ===\n");
    test_default_shell_is_tbos();
    test_switch_to_sh_and_back();
    test_shell_list_marks_current();
    test_execute_routes_to_current_shell();
    test_switch_invalid_shell();

    printf("\nTest summary: %d passed, %d failed\n", tests_passed, tests_failed);
    return (tests_failed == 0) ? 0 : 1;
}
