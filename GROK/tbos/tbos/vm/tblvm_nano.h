/*
 * TernaryBit Lean Virtual Machine - Nano Edition
 * Header file defining VM opcodes and interface
 * Size: 2.9KB total implementation
 */

#ifndef TBLVM_NANO_H
#define TBLVM_NANO_H

#include <stdint.h>
#include <stddef.h>

/* ========================================================================= */
/* VM CONFIGURATION                                                          */
/* ========================================================================= */

#define TBLVM_STACK_SIZE    64      /* Stack size (32-bit values) */
#define TBLVM_LOCALS_SIZE   16      /* Local variables */
#define TBLVM_GLOBALS_SIZE  16      /* Global variables */
#define TBLVM_HEAP_SIZE     256     /* Heap memory (bytes) */
#define TBLVM_CALLSTACK_SIZE 8      /* Call stack depth */

/* ========================================================================= */
/* OPCODES (40 total)                                                        */
/* ========================================================================= */

typedef enum {
    /* Control Flow */
    OP_NOP          = 0x00,  /* No operation */
    OP_HALT         = 0x01,  /* Stop execution */
    OP_EXIT         = 0x02,  /* Exit with code */

    /* Stack Operations */
    OP_PUSH         = 0x10,  /* Push 32-bit value */
    OP_PUSH_BYTE    = 0x11,  /* Push 8-bit value */
    OP_POP          = 0x12,  /* Pop value */
    OP_DUP          = 0x13,  /* Duplicate top */
    OP_SWAP         = 0x14,  /* Swap top two */

    /* Arithmetic */
    OP_ADD          = 0x20,  /* Add */
    OP_SUB          = 0x21,  /* Subtract */
    OP_MUL          = 0x22,  /* Multiply */
    OP_DIV          = 0x23,  /* Divide */
    OP_MOD          = 0x24,  /* Modulo */
    OP_NEG          = 0x25,  /* Negate */
    OP_INC          = 0x26,  /* Increment */
    OP_DEC          = 0x27,  /* Decrement */

    /* Bitwise */
    OP_AND          = 0x30,  /* Bitwise AND */
    OP_OR           = 0x31,  /* Bitwise OR */
    OP_XOR          = 0x32,  /* Bitwise XOR */
    OP_NOT          = 0x33,  /* Bitwise NOT */
    OP_SHL          = 0x34,  /* Shift left */
    OP_SHR          = 0x35,  /* Shift right */

    /* Comparison */
    OP_EQ           = 0x40,  /* Equal */
    OP_NE           = 0x41,  /* Not equal */
    OP_LT           = 0x42,  /* Less than */
    OP_LE           = 0x43,  /* Less or equal */
    OP_GT           = 0x44,  /* Greater than */
    OP_GE           = 0x45,  /* Greater or equal */

    /* Jumps */
    OP_JMP          = 0x50,  /* Unconditional jump */
    OP_JZ           = 0x51,  /* Jump if zero */
    OP_JNZ          = 0x52,  /* Jump if not zero */

    /* Memory */
    OP_LOAD         = 0x60,  /* Load local variable */
    OP_STORE        = 0x61,  /* Store local variable */
    OP_LOAD_GLOBAL  = 0x62,  /* Load global variable */
    OP_STORE_GLOBAL = 0x63,  /* Store global variable */

    /* Functions */
    OP_CALL         = 0x70,  /* Call function */
    OP_RET          = 0x71,  /* Return from function */

    /* I/O */
    OP_PRINT        = 0x80,  /* Print number */
    OP_PRINT_CHAR   = 0x81,  /* Print character */
    OP_READ         = 0x82,  /* Read number */
    OP_READ_CHAR    = 0x83   /* Read character */
} tblvm_opcode_t;

/* ========================================================================= */
/* VM STRUCTURE                                                              */
/* ========================================================================= */

typedef struct {
    /* Registers */
    uint32_t pc;                           /* Program counter */
    uint32_t sp;                           /* Stack pointer */
    uint32_t fp;                           /* Frame pointer */
    int running;                           /* Execution flag */
    int32_t exit_code;                     /* Exit code */

    /* Memory */
    int32_t stack[TBLVM_STACK_SIZE];       /* Data stack */
    int32_t locals[TBLVM_LOCALS_SIZE];     /* Local variables */
    int32_t globals[TBLVM_GLOBALS_SIZE];   /* Global variables */
    uint8_t heap[TBLVM_HEAP_SIZE];         /* Heap memory */

    /* Call stack */
    uint32_t callstack[TBLVM_CALLSTACK_SIZE]; /* Return addresses */
    uint32_t callsp;                       /* Call stack pointer */

    /* Bytecode */
    const uint8_t* bytecode;               /* Bytecode pointer */
    uint32_t bytecode_size;                /* Bytecode size */

    /* I/O callbacks */
    void (*putchar_fn)(char);              /* Output callback */
    int (*getchar_fn)(void);               /* Input callback */
} tblvm_t;

/* ========================================================================= */
/* ERROR CODES                                                               */
/* ========================================================================= */

typedef enum {
    TBLVM_OK                = 0,
    TBLVM_ERR_NULL_VM       = -1,
    TBLVM_ERR_NULL_CODE     = -2,
    TBLVM_ERR_STACK_OVERFLOW = -3,
    TBLVM_ERR_STACK_UNDERFLOW = -4,
    TBLVM_ERR_INVALID_OPCODE = -5,
    TBLVM_ERR_DIV_ZERO      = -6,
    TBLVM_ERR_OUT_OF_BOUNDS = -7,
    TBLVM_ERR_CALL_OVERFLOW = -8
} tblvm_error_t;

/* ========================================================================= */
/* API FUNCTIONS                                                             */
/* ========================================================================= */

/* Initialize VM */
void tblvm_init(tblvm_t* vm);

/* Load bytecode */
int tblvm_load(tblvm_t* vm, const uint8_t* bytecode, uint32_t size);

/* Execute loaded bytecode */
int tblvm_exec(tblvm_t* vm);

/* Execute single instruction */
int tblvm_step(tblvm_t* vm);

/* Set I/O callbacks */
void tblvm_set_putchar(tblvm_t* vm, void (*fn)(char));
void tblvm_set_getchar(tblvm_t* vm, int (*fn)(void));

/* Get error string */
const char* tblvm_error_str(int error_code);

#endif /* TBLVM_NANO_H */
