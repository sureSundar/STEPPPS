/*
 * TernaryBit Lean Virtual Machine - Nano Edition
 * Optimized implementation: 2.9KB
 */

#include "tblvm_nano.h"
#include <stdio.h>
#include <string.h>

/* ========================================================================= */
/* MACROS                                                                    */
/* ========================================================================= */

#define FETCH_BYTE() (vm->bytecode[vm->pc++])
#define FETCH_UINT16() (vm->pc += 2, \
    (uint16_t)((vm->bytecode[vm->pc-2] << 8) | vm->bytecode[vm->pc-1]))
#define FETCH_UINT32() (vm->pc += 4, \
    (uint32_t)((vm->bytecode[vm->pc-4] << 24) | \
               (vm->bytecode[vm->pc-3] << 16) | \
               (vm->bytecode[vm->pc-2] << 8) | \
                vm->bytecode[vm->pc-1]))

#define STACK_PUSH(val) do { \
    if (vm->sp >= TBLVM_STACK_SIZE) return TBLVM_ERR_STACK_OVERFLOW; \
    vm->stack[vm->sp++] = (val); \
} while(0)

#define STACK_POP() (vm->sp == 0 ? (exit_code = TBLVM_ERR_STACK_UNDERFLOW, 0) : vm->stack[--vm->sp])

#define STACK_TOP() (vm->sp == 0 ? 0 : vm->stack[vm->sp-1])

/* ========================================================================= */
/* API IMPLEMENTATION                                                        */
/* ========================================================================= */

void tblvm_init(tblvm_t* vm) {
    memset(vm, 0, sizeof(tblvm_t));
    vm->running = 0;
}

int tblvm_load(tblvm_t* vm, const uint8_t* bytecode, uint32_t size) {
    if (!vm) return TBLVM_ERR_NULL_VM;
    if (!bytecode) return TBLVM_ERR_NULL_CODE;

    vm->bytecode = bytecode;
    vm->bytecode_size = size;
    vm->pc = 0;
    vm->sp = 0;
    vm->fp = 0;
    vm->callsp = 0;
    vm->running = 1;
    vm->exit_code = 0;

    return TBLVM_OK;
}

int tblvm_exec(tblvm_t* vm) {
    if (!vm) return TBLVM_ERR_NULL_VM;

    int result;
    while (vm->running && vm->pc < vm->bytecode_size) {
        result = tblvm_step(vm);
        if (result != TBLVM_OK) return result;
    }

    return TBLVM_OK;
}

int tblvm_step(tblvm_t* vm) {
    if (!vm->running) return TBLVM_OK;
    if (vm->pc >= vm->bytecode_size) {
        vm->running = 0;
        return TBLVM_OK;
    }

    int32_t a, b;
    int exit_code = TBLVM_OK;
    uint8_t op = FETCH_BYTE();

    switch (op) {
        /* Control Flow */
        case OP_NOP:
            break;
        case OP_HALT:
            vm->running = 0;
            break;
        case OP_EXIT:
            vm->exit_code = STACK_POP();
            vm->running = 0;
            break;

        /* Stack Operations */
        case OP_PUSH:
            STACK_PUSH((int32_t)FETCH_UINT32());
            break;
        case OP_PUSH_BYTE:
            STACK_PUSH((int32_t)FETCH_BYTE());
            break;
        case OP_POP:
            STACK_POP();
            break;
        case OP_DUP:
            a = STACK_TOP();
            STACK_PUSH(a);
            break;
        case OP_SWAP:
            a = STACK_POP();
            b = STACK_POP();
            STACK_PUSH(a);
            STACK_PUSH(b);
            break;

        /* Arithmetic */
        case OP_ADD:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a + b);
            break;
        case OP_SUB:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a - b);
            break;
        case OP_MUL:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a * b);
            break;
        case OP_DIV:
            b = STACK_POP();
            if (b == 0) return TBLVM_ERR_DIV_ZERO;
            a = STACK_POP();
            STACK_PUSH(a / b);
            break;
        case OP_MOD:
            b = STACK_POP();
            if (b == 0) return TBLVM_ERR_DIV_ZERO;
            a = STACK_POP();
            STACK_PUSH(a % b);
            break;
        case OP_NEG:
            a = STACK_POP();
            STACK_PUSH(-a);
            break;
        case OP_INC:
            a = STACK_POP();
            STACK_PUSH(a + 1);
            break;
        case OP_DEC:
            a = STACK_POP();
            STACK_PUSH(a - 1);
            break;

        /* Bitwise */
        case OP_AND:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a & b);
            break;
        case OP_OR:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a | b);
            break;
        case OP_XOR:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a ^ b);
            break;
        case OP_NOT:
            a = STACK_POP();
            STACK_PUSH(~a);
            break;
        case OP_SHL:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a << b);
            break;
        case OP_SHR:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a >> b);
            break;

        /* Comparison */
        case OP_EQ:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a == b ? 1 : 0);
            break;
        case OP_NE:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a != b ? 1 : 0);
            break;
        case OP_LT:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a < b ? 1 : 0);
            break;
        case OP_LE:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a <= b ? 1 : 0);
            break;
        case OP_GT:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a > b ? 1 : 0);
            break;
        case OP_GE:
            b = STACK_POP();
            a = STACK_POP();
            STACK_PUSH(a >= b ? 1 : 0);
            break;

        /* Jumps */
        case OP_JMP:
            vm->pc = FETCH_UINT16();
            break;
        case OP_JZ:
            a = FETCH_UINT16();
            if (STACK_POP() == 0) vm->pc = a;
            break;
        case OP_JNZ:
            a = FETCH_UINT16();
            if (STACK_POP() != 0) vm->pc = a;
            break;

        /* Memory */
        case OP_LOAD:
            a = FETCH_BYTE();
            if (a >= TBLVM_LOCALS_SIZE) return TBLVM_ERR_OUT_OF_BOUNDS;
            STACK_PUSH(vm->locals[a]);
            break;
        case OP_STORE:
            a = FETCH_BYTE();
            if (a >= TBLVM_LOCALS_SIZE) return TBLVM_ERR_OUT_OF_BOUNDS;
            vm->locals[a] = STACK_POP();
            break;
        case OP_LOAD_GLOBAL:
            a = FETCH_BYTE();
            if (a >= TBLVM_GLOBALS_SIZE) return TBLVM_ERR_OUT_OF_BOUNDS;
            STACK_PUSH(vm->globals[a]);
            break;
        case OP_STORE_GLOBAL:
            a = FETCH_BYTE();
            if (a >= TBLVM_GLOBALS_SIZE) return TBLVM_ERR_OUT_OF_BOUNDS;
            vm->globals[a] = STACK_POP();
            break;

        /* Functions */
        case OP_CALL:
            if (vm->callsp >= TBLVM_CALLSTACK_SIZE) return TBLVM_ERR_CALL_OVERFLOW;
            a = FETCH_UINT16();
            vm->callstack[vm->callsp++] = vm->pc;
            vm->pc = a;
            break;
        case OP_RET:
            if (vm->callsp == 0) {
                vm->running = 0;
            } else {
                vm->pc = vm->callstack[--vm->callsp];
            }
            break;

        /* I/O */
        case OP_PRINT:
            a = STACK_POP();
            if (vm->putchar_fn) {
                /* Print number as decimal */
                char buf[16];
                int len = 0;
                int neg = a < 0;
                if (neg) a = -a;

                do {
                    buf[len++] = '0' + (a % 10);
                    a /= 10;
                } while (a > 0 && len < 15);

                if (neg) buf[len++] = '-';

                while (len > 0) {
                    vm->putchar_fn(buf[--len]);
                }
            }
            break;
        case OP_PRINT_CHAR:
            if (vm->putchar_fn) {
                vm->putchar_fn((char)STACK_POP());
            }
            break;
        case OP_READ:
            if (vm->getchar_fn) {
                a = 0;
                int c;
                while ((c = vm->getchar_fn()) != '\n' && c != EOF) {
                    if (c >= '0' && c <= '9') {
                        a = a * 10 + (c - '0');
                    }
                }
                STACK_PUSH(a);
            } else {
                STACK_PUSH(0);
            }
            break;
        case OP_READ_CHAR:
            if (vm->getchar_fn) {
                STACK_PUSH(vm->getchar_fn());
            } else {
                STACK_PUSH(0);
            }
            break;

        default:
            return TBLVM_ERR_INVALID_OPCODE;
    }

    return exit_code;
}

void tblvm_set_putchar(tblvm_t* vm, void (*fn)(char)) {
    if (vm) vm->putchar_fn = fn;
}

void tblvm_set_getchar(tblvm_t* vm, int (*fn)(void)) {
    if (vm) vm->getchar_fn = fn;
}

const char* tblvm_error_str(int error_code) {
    switch (error_code) {
        case TBLVM_OK: return "OK";
        case TBLVM_ERR_NULL_VM: return "NULL VM";
        case TBLVM_ERR_NULL_CODE: return "NULL code";
        case TBLVM_ERR_STACK_OVERFLOW: return "Stack overflow";
        case TBLVM_ERR_STACK_UNDERFLOW: return "Stack underflow";
        case TBLVM_ERR_INVALID_OPCODE: return "Invalid opcode";
        case TBLVM_ERR_DIV_ZERO: return "Division by zero";
        case TBLVM_ERR_OUT_OF_BOUNDS: return "Out of bounds";
        case TBLVM_ERR_CALL_OVERFLOW: return "Call stack overflow";
        default: return "Unknown error";
    }
}
