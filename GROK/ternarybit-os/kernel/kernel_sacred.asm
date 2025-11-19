; TBOS Kernel Entry Point - Sprint 6
; 🕉️ Kaliyugavaradhan Saranam 🕉️
; Assembly entry point that calls kernel_main()

[BITS 64]
[GLOBAL _start]
[EXTERN kernel_main]

section .text
_start:
    ; We arrive here from stage2 bootloader in 64-bit long mode
    ; Stack and segments already set up by Stage 2

    ; CRITICAL: Ensure interrupts are disabled (defensive programming)
    cli

    ; DEBUG: Write '@' to VGA to prove we reached kernel entry
    mov rax, 0xB8000
    mov word [rax + 160], 0x0F40  ; '@' at position (1, 0)

    ; Setup stack pointer with 16-byte alignment (use 64-bit registers)
    ; Place stack well above kernel BSS
    ; Use 2MB (0x200000) as stack base for safety
    mov rax, 0x200000
    mov rsp, rax
    and rsp, 0xFFFFFFFFFFFFFFF0    ; Align to 16 bytes
    mov rbp, rsp

    ; DEBUG: Write '#' to show stack setup complete
    mov rax, 0xB8000
    mov word [rax + 162], 0x0F23  ; '#' at position (1, 1)

    ; SKIP BSS CLEARING - might be causing triple fault
    ; extern __bss_start
    ; extern __bss_end

.bss_done:
    ; DEBUG: Write '$' to show BSS was skipped
    mov rax, 0xB8000
    mov word [rax + 164], 0x0F24  ; '$' at position (1, 2)

    ; Call kernel_main with no arguments
    ; Stack is already 16-byte aligned
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

; Kernel GDT (in data section so it doesn't get overwritten)
section .data
align 8
kernel_gdt_start:
    ; Null descriptor
    dq 0

    ; 64-bit Code segment (0x08)
    dw 0xFFFF       ; Limit low (ignored in 64-bit mode)
    dw 0x0000       ; Base low (ignored in 64-bit mode)
    db 0x00         ; Base middle
    db 10011010b    ; Access: P=1, DPL=00, S=1, Type=1010 (code, execute/read)
    db 10101111b    ; Flags: G=1, L=1 (64-bit), D=0, Limit high=1111
    db 0x00         ; Base high

    ; 64-bit Data segment (0x10)
    dw 0xFFFF       ; Limit low (ignored in 64-bit mode)
    dw 0x0000       ; Base low (ignored in 64-bit mode)
    db 0x00         ; Base middle
    db 10010010b    ; Access: P=1, DPL=00, S=1, Type=0010 (data, read/write)
    db 10101111b    ; Flags: G=1, L=1, D=0, Limit high=1111
    db 0x00         ; Base high

kernel_gdt_end:

kernel_gdt_descriptor:
    dw kernel_gdt_end - kernel_gdt_start - 1
    dq kernel_gdt_start
