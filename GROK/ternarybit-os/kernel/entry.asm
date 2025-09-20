; Kernel entry point for TBOS

[bits 32]
[extern kmain]    ; Defined in kernel.c

global _start

section .text
_start:
    ; Set up stack
    mov esp, 0x90000  ; Set up stack pointer
    
    ; Clear direction flag
    cld
    
    ; Call kernel main
    call kmain
    
    ; Halt if kernel returns
    cli
    hlt
    jmp $
