; Minimal startup code for second stage bootloader

section .text
global _start
extern main

_start:
    ; Set up stack pointer
    mov esp, 0x9000
    and esp, -16  ; Align stack to 16 bytes
    
    ; Clear direction flag
    cld
    
    ; Call main function
    call main
    
    ; If main returns, halt the system
    cli
.halt:
    hlt
    jmp .halt

; Simple putchar implementation for early debugging
global putchar
putchar:
    push ebp
    mov ebp, esp
    push ebx
    
    mov al, [ebp + 8]    ; Get character
    mov ah, 0x0E         ; BIOS teletype function
    mov bx, 0x0007       ; Page 0, white on black
    int 0x10             ; Call BIOS
    
    pop ebx
    pop ebp
    ret
