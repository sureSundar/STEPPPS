// TEST_DEPS: shell/argparse.c
#include <stdio.h>
#include <string.h>
#include "tbos/argparse.h"

void kernel_print(const char* str) {
    (void)str;
}

static int tests_passed = 0;
static int tests_failed = 0;

static void assert_true(int condition, const char* message) {
    if (condition) {
        printf("  ✅ %s\n", message);
        tests_passed++;
    } else {
        printf("  ❌ %s\n", message);
        tests_failed++;
    }
}

static void test_long_flag_with_value(void) {
    printf("\n[TEST] Long flag parsing\n");
    argparse_result_t result;
    int rc = argparse_parse("--mode=sh /tmp", &result);

    assert_true(rc == 0, "Parsing succeeded");
    assert_true(result.flag_count == 1, "One flag parsed");
    assert_true(strcmp(result.flags[0].flag, "mode") == 0, "Flag name captured");
    assert_true(result.flags[0].has_value, "Flag value detected");
    assert_true(strcmp(result.flags[0].value, "sh") == 0, "Flag value recorded");
    assert_true(result.positional_count == 1, "One positional argument");
    assert_true(strcmp(result.positional[0], "/tmp") == 0, "Positional argument preserved");
}

static void test_unterminated_quote_error(void) {
    printf("\n[TEST] Unterminated quote detection\n");
    argparse_result_t result;
    int rc = argparse_parse("echo \"unfinished", &result);

    assert_true(rc == -1, "Parsing failed for unterminated quote");
    assert_true(result.error == ARGPARSE_ERROR_INVALID_SYNTAX, "Error code set to invalid syntax");
    assert_true(strstr(result.error_msg, "Unterminated") != NULL, "Error message populated");
}

int main(void) {
    printf("\n=== argparse Tests ===\n");
    test_long_flag_with_value();
    test_unterminated_quote_error();

    printf("\nTest summary: %d passed, %d failed\n", tests_passed, tests_failed);
    return (tests_failed == 0) ? 0 : 1;
}
