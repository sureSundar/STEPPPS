; TBOS Kernel Entry Point - Sprint 6
; 🕉️ Kaliyugavaradhan Saranam 🕉️
; Assembly entry point that calls kernel_main()

[BITS 32]
[GLOBAL _start]
[EXTERN kernel_main]

section .text
_start:
    ; We arrive here from stage2 bootloader
    ; Stack and segments already set up by Stage 2
    ; Interrupts already disabled by Stage 2

    ; Debug: Write to serial at kernel entry
    mov dx, 0x3F8
    mov al, '['
    out dx, al
    mov al, 'E'
    out dx, al
    mov al, 'N'
    out dx, al
    mov al, 'T'
    out dx, al
    mov al, 'R'
    out dx, al
    mov al, 'Y'
    out dx, al
    mov al, ']'
    out dx, al
    mov al, 10
    out dx, al

    ; Setup stack pointer with 16-byte alignment
    mov esp, 0x90000
    and esp, 0xFFFFFFF0    ; Align to 16 bytes
    mov ebp, esp

    ; Debug: Before BSS clear
    mov dx, 0x3F8
    mov al, '['
    out dx, al
    mov al, 'B'
    out dx, al
    mov al, 'S'
    out dx, al
    mov al, 'S'
    out dx, al
    mov al, ']'
    out dx, al
    mov al, 10
    out dx, al

    ; Clear BSS section (uninitialized data)
    extern __bss_start
    extern __bss_end
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; Debug: After BSS clear
    mov dx, 0x3F8
    mov al, '['
    out dx, al
    mov al, 'O'
    out dx, al
    mov al, 'K'
    out dx, al
    mov al, ']'
    out dx, al
    mov al, 10
    out dx, al

    ; Call kernel_main with no arguments
    ; Stack is already 16-byte aligned at 0x8FFF0
    call kernel_main

    ; If kernel_main returns (it shouldn't), halt
    cli
.halt:
    hlt
    jmp .halt

; Stack section - 16KB kernel stack
section .bss
align 16
stack_bottom:
    resb 16384      ; 16KB stack
stack_top:

; Multiboot header removed - using custom bootloader
; section .multiboot
; align 4
;    dd 0x1BADB002           ; Magic number
;    dd 0x00000003           ; Flags
;    dd -(0x1BADB002+0x3)    ; Checksum

; Kernel GDT (in data section so it doesn't get overwritten)
section .data
align 8
kernel_gdt_start:
    ; Null descriptor
    dq 0
    ; Code segment (0x08): base=0, limit=0xFFFFF, 32-bit, readable, executable
    dw 0xFFFF       ; limit low
    dw 0x0000       ; base low
    db 0x00         ; base middle
    db 10011010b    ; access (present, ring 0, code, executable, readable)
    db 11001111b    ; flags (4KB gran, 32-bit) + limit high
    db 0x00         ; base high
    ; Data segment (0x10): base=0, limit=0xFFFFF, 32-bit, writable
    dw 0xFFFF       ; limit low
    dw 0x0000       ; base low
    db 0x00         ; base middle
    db 10010010b    ; access (present, ring 0, data, writable)
    db 11001111b    ; flags (4KB gran, 32-bit) + limit high
    db 0x00         ; base high
kernel_gdt_end:

kernel_gdt_descriptor:
    dw kernel_gdt_end - kernel_gdt_start - 1
    dd kernel_gdt_start