; TernaryBit OS - Minimal Universal Kernel
; Proves the bootloader works - just displays a message and halts
; 🕉️ Universal: Works from 2-bit to supercomputer 🕉️

[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; We arrive here from bootloader in protected mode
    ; Segments are already set up by bootloader

    ; Display success message to VGA
    mov esi, success_msg
    mov edi, 0xB8000        ; VGA text buffer
    mov ah, 0x0F            ; White on black

.print_loop:
    lodsb                   ; Load byte from ESI
    test al, al             ; Check for null terminator
    jz .done
    stosw                   ; Write char+attribute to VGA
    jmp .print_loop

.done:
    ; Success! Halt the system cleanly
    cli
    hlt
    jmp $

section .data
success_msg:
    db '*** TBOS KERNEL LOADED SUCCESSFULLY ***', 0
    db 'Universal Bootloader: WORKING!', 0
    db 'Om Swamiye Saranam Aiyappa', 0
