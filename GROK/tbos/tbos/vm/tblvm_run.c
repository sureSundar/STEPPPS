/*
 * TernaryBit VM Runtime - Bytecode Executor
 * Loads and executes .tbc bytecode files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tblvm_nano.h"

/* ========================================================================= */
/* I/O CALLBACKS                                                             */
/* ========================================================================= */

static void my_putchar(char c) {
    putchar(c);
    fflush(stdout);
}

static int my_getchar(void) {
    return getchar();
}

/* ========================================================================= */
/* FILE LOADING                                                              */
/* ========================================================================= */

static uint8_t* load_bytecode_file(const char* filename, uint32_t* size) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }

    /* Get file size */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (fsize <= 0 || fsize > 1024*1024) {
        fprintf(stderr, "Error: Invalid file size\n");
        fclose(f);
        return NULL;
    }

    /* Allocate and read */
    uint8_t* bytecode = (uint8_t*)malloc(fsize);
    if (!bytecode) {
        fprintf(stderr, "Error: Out of memory\n");
        fclose(f);
        return NULL;
    }

    size_t read = fread(bytecode, 1, fsize, f);
    fclose(f);

    if (read != (size_t)fsize) {
        fprintf(stderr, "Error: Failed to read file\n");
        free(bytecode);
        return NULL;
    }

    *size = (uint32_t)fsize;
    return bytecode;
}

/* ========================================================================= */
/* MAIN                                                                      */
/* ========================================================================= */

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <bytecode.tbc>\n", argv[0]);
        return 1;
    }

    /* Load bytecode file */
    uint32_t size;
    uint8_t* bytecode = load_bytecode_file(argv[1], &size);
    if (!bytecode) {
        return 1;
    }

    /* Initialize VM */
    tblvm_t vm;
    tblvm_init(&vm);
    tblvm_set_putchar(&vm, my_putchar);
    tblvm_set_getchar(&vm, my_getchar);

    /* Load bytecode */
    int result = tblvm_load(&vm, bytecode, size);
    if (result != TBLVM_OK) {
        fprintf(stderr, "Error loading bytecode: %s\n", tblvm_error_str(result));
        free(bytecode);
        return 1;
    }

    /* Execute */
    result = tblvm_exec(&vm);
    int exit_code = vm.exit_code;

    /* Cleanup */
    free(bytecode);

    if (result != TBLVM_OK) {
        fprintf(stderr, "\nVM Error: %s\n", tblvm_error_str(result));
        fprintf(stderr, "PC=%u, SP=%u\n", vm.pc, vm.sp);
        return 1;
    }

    return exit_code;
}
