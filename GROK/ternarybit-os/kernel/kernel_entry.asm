; TernaryBit OS Kernel Entry Point
; Bridge between bootloader and C kernel

[BITS 32]
[EXTERN kernel_main]

section .text
global _start

_start:
    ; Set up kernel stack
    mov esp, 0x90000

    ; Clear direction flag for string operations
    cld

    ; Call main C kernel function
    call kernel_main

    ; If kernel returns, halt
halt_loop:
    hlt
    jmp halt_loop