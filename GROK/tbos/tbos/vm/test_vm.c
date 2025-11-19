/*
 * TBLVM Nano Test Program
 * Demonstrates the bytecode interpreter
 */

#include <stdio.h>
#include <stdlib.h>
#include "tblvm_nano.h"

/* ========================================================================= */
/* I/O CALLBACKS (for desktop testing)                                      */
/* ========================================================================= */

static void my_putchar(char c) {
    putchar(c);
    fflush(stdout);
}

static int my_getchar(void) {
    return getchar();
}

/* ========================================================================= */
/* TEST PROGRAMS (hand-coded bytecode)                                      */
/* ========================================================================= */

/* Test 1: Hello World (prints "Hello!" using ASCII values) */
static const uint8_t test_hello[] = {
    OP_PUSH_BYTE, 'H',      /* Push 'H' */
    OP_PRINT_CHAR,          /* Print it */
    OP_PUSH_BYTE, 'e',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, 'l',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, 'l',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, 'o',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, '!',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, '\n',
    OP_PRINT_CHAR,
    OP_HALT
};

/* Test 2: Simple arithmetic (5 + 3 = 8) */
static const uint8_t test_arithmetic[] = {
    OP_PUSH_BYTE, 5,        /* Push 5 */
    OP_PUSH_BYTE, 3,        /* Push 3 */
    OP_ADD,                 /* Add them */
    OP_PRINT,               /* Print result (8) */
    OP_PUSH_BYTE, '\n',
    OP_PRINT_CHAR,
    OP_HALT
};

/* Test 3: Loop (count from 1 to 10) */
static const uint8_t test_loop[] = {
    /* Initialize counter in local[0] */
    OP_PUSH_BYTE, 1,        /* 0: Push 1 */
    OP_STORE, 0,            /* 2: Store to local[0] */

    /* Loop start (PC = 4) */
    OP_LOAD, 0,             /* 4: Load counter */
    OP_DUP,                 /* 6: Duplicate it */
    OP_PRINT,               /* 7: Print counter */
    OP_PUSH_BYTE, '\n',     /* 8: Newline */
    OP_PRINT_CHAR,          /* 10: Print it */

    /* Increment counter */
    OP_LOAD, 0,             /* 11: Load counter */
    OP_INC,                 /* 13: Increment */
    OP_STORE, 0,            /* 14: Store back */

    /* Check if counter <= 10 */
    OP_LOAD, 0,             /* 16: Load counter */
    OP_PUSH_BYTE, 10,       /* 18: Push 10 */
    OP_LE,                  /* 20: counter <= 10? */
    OP_JNZ, 0x00, 0x04,     /* 21: Jump to loop start if not zero */

    OP_HALT                 /* 24: Done */
};

/* Test 4: Function call (factorial-like) */
static const uint8_t test_function[] = {
    /* Main: */
    OP_PUSH_BYTE, 5,        /* 0: Push argument (5) */
    OP_CALL, 0x00, 0x08,    /* 2: Call function at PC=8 */
    OP_PRINT,               /* 5: Print result */
    OP_HALT,                /* 6: Exit */
    OP_NOP,                 /* 7: Padding */

    /* Function (double the input): PC=8 */
    OP_PUSH_BYTE, 2,        /* 8: Push 2 */
    OP_MUL,                 /* 10: Multiply */
    OP_RET                  /* 11: Return */
};

/* Test 5: Calculator (read two numbers, add, print result) */
static const uint8_t test_calculator[] = {
    /* Print prompt */
    OP_PUSH_BYTE, 'A',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, '=',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, ' ',
    OP_PRINT_CHAR,

    /* Read first number */
    OP_READ,
    OP_STORE, 0,            /* Store in local[0] */

    /* Print prompt */
    OP_PUSH_BYTE, 'B',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, '=',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, ' ',
    OP_PRINT_CHAR,

    /* Read second number */
    OP_READ,
    OP_STORE, 1,            /* Store in local[1] */

    /* Calculate A + B */
    OP_LOAD, 0,
    OP_LOAD, 1,
    OP_ADD,

    /* Print result */
    OP_PUSH_BYTE, 'R',
    OP_PRINT_CHAR,
    OP_PUSH_BYTE, '=',
    OP_PRINT_CHAR,
    OP_PRINT,
    OP_PUSH_BYTE, '\n',
    OP_PRINT_CHAR,

    OP_HALT
};

/* ========================================================================= */
/* TEST RUNNER                                                              */
/* ========================================================================= */

static void run_test(const char* name, const uint8_t* bytecode, uint32_t size) {
    printf("\n=== Test: %s ===\n", name);

    tblvm_t vm;
    tblvm_init(&vm);
    tblvm_set_putchar(&vm, my_putchar);
    tblvm_set_getchar(&vm, my_getchar);

    int result = tblvm_load(&vm, bytecode, size);
    if (result != TBLVM_OK) {
        printf("ERROR: Failed to load bytecode: %s\n", tblvm_error_str(result));
        return;
    }

    result = tblvm_exec(&vm);
    if (result != TBLVM_OK) {
        printf("ERROR: Execution failed: %s\n", tblvm_error_str(result));
        printf("PC=%u, SP=%u\n", vm.pc, vm.sp);
        return;
    }

    printf("Exit code: %d\n", vm.exit_code);
}

/* ========================================================================= */
/* MAIN                                                                     */
/* ========================================================================= */

int main(int argc, char** argv) {
    printf("TernaryBit Lean VM - Nano Edition Test Suite\n");
    printf("VM Size: Stack=%d, Locals=%d, Globals=%d, Heap=%d bytes\n",
           TBLVM_STACK_SIZE * 4,
           TBLVM_LOCALS_SIZE * 4,
           TBLVM_GLOBALS_SIZE * 4,
           TBLVM_HEAP_SIZE);

    /* Run tests */
    run_test("Hello World", test_hello, sizeof(test_hello));
    run_test("Arithmetic (5+3)", test_arithmetic, sizeof(test_arithmetic));
    run_test("Loop (1 to 10)", test_loop, sizeof(test_loop));
    run_test("Function Call", test_function, sizeof(test_function));

    /* Interactive test */
    if (argc > 1 && argv[1][0] == 'i') {
        printf("\n=== Interactive Calculator Test ===\n");
        printf("Enter two numbers (separated by newline):\n");
        run_test("Calculator", test_calculator, sizeof(test_calculator));
    }

    printf("\n=== All tests complete ===\n");
    return 0;
}
