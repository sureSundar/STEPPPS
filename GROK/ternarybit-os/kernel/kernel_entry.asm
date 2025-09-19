; TernaryBit OS Kernel Entry Point
; Bridge between bootloader and C kernel

[BITS 32]
[EXTERN kernel_main]
[EXTERN g_tbds_pointer]
[EXTERN g_tbds_length]

section .text
global _start

_start:
    ; Write "ENTRY!" to screen to debug
    mov edi, 0xB8000
    mov eax, 0x1F45  ; 'E' white on blue
    mov [edi], eax
    mov eax, 0x1F4E  ; 'N' white on blue
    mov [edi+2], eax
    mov eax, 0x1F54  ; 'T' white on blue
    mov [edi+4], eax
    mov eax, 0x1F52  ; 'R' white on blue
    mov [edi+6], eax
    mov eax, 0x1F59  ; 'Y' white on blue
    mov [edi+8], eax
    mov eax, 0x1F21  ; '!' white on blue
    mov [edi+10], eax

    ; Preserve TBDS pointer/length provided via EAX/EBX
    mov [g_tbds_pointer], eax
    mov [g_tbds_length], ebx

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
