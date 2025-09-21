; TBOS Kernel Entry Point - Sprint 6
; 🕉️ Kaliyugavaradhan Saranam 🕉️
; Assembly entry point that calls kernel_main()

[BITS 32]
[GLOBAL _start]
[EXTERN kernel_main]

section .text
_start:
    ; We arrive here from stage2 bootloader
    ; Stack is already set up at 0x90000

    ; Clear interrupt flag
    cli

    ; Setup segment registers
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Setup stack pointer properly
    mov esp, 0x90000
    mov ebp, esp

    ; Clear BSS section (uninitialized data)
    extern __bss_start
    extern __bss_end
    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    ; Push magic number and multiboot info (if any)
    push 0              ; Multiboot info pointer (null for now)
    push 0xDEADBEEF    ; Magic number

    ; Call kernel main
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

; Multiboot header for future GRUB compatibility
section .multiboot
align 4
    dd 0x1BADB002           ; Magic number
    dd 0x00000003           ; Flags
    dd -(0x1BADB002+0x3)    ; Checksum