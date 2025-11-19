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
    
    ; Clear screen
    mov edi, 0xB8000  ; Video memory
    mov ecx, 80*25    ; Screen size (80x25)
    mov eax, 0x07000700  ; Black background, white foreground, space character
    rep stosd
    
    ; Call kernel main
    call kmain
    
    ; Halt if kernel returns
    cli
    hlt
    jmp $
