; TernaryBit OS Kernel Entry Point
; Bridge between bootloader and C kernel

[BITS 32]
[EXTERN kernel_main]
[EXTERN g_tbds_pointer]
[EXTERN g_tbds_length]

section .text
global _start

_start:
    ; Preserve TBDS pointer/length provided via EAX/EBX
    ; Codex: Doing this before touching AX so kernel_main sees real metadata.
    mov [g_tbds_pointer], eax
    mov [g_tbds_length], ebx

    ; Serial bring-up disabled during kernel smoke-test

    ; VGA writes disabled during bring-up to avoid side-effects
    ; mov edi, 0xB8000
    ; mov ax, 0x1F45  ; 'E' white on blue
    ; mov [edi], ax
    ; mov ax, 0x1F4E  ; 'N' white on blue
    ; mov [edi+2], ax
    ; mov ax, 0x1F54  ; 'T' white on blue
    ; mov [edi+4], ax
    ; mov ax, 0x1F52  ; 'R' white on blue
    ; mov [edi+6], ax
    ; mov ax, 0x1F59  ; 'Y' white on blue
    ; mov [edi+8], ax
    ; mov ax, 0x1F21  ; '!' white on blue
    ; mov [edi+10], ax

    ; Set up kernel stack
    mov esp, 0x90000

    ; Clear direction flag for string operations
    cld

    ; Call main C kernel function
    ; Pair-Programming: CC enabling kernel_main call
    call kernel_main

    ; If kernel returns, halt
halt_loop:
    hlt
    jmp halt_loop

; ----------------------
; Minimal COM1 serial I/O
; ----------------------
serial_init:
    push eax
    push edx
    mov dx, 0x3F9          ; COM1 interrupt enable
    xor eax, eax
    out dx, al             ; Disable interrupts
    mov dx, 0x3FB          ; Line control register
    mov al, 0x80           ; Enable DLAB
    out dx, al
    mov dx, 0x3F8          ; Divisor low byte
    mov al, 0x03           ; 115200/3 = 38400 baud
    out dx, al
    mov dx, 0x3F9          ; Divisor high byte
    xor eax, eax
    out dx, al
    mov dx, 0x3FB
    mov al, 0x03           ; 8N1
    out dx, al
    mov dx, 0x3FA          ; FIFO control
    mov al, 0xC7           ; Enable FIFO, clear queues
    out dx, al
    mov dx, 0x3FC          ; Modem control
    mov al, 0x0B           ; IRQs disabled, RTS/DSR set
    out dx, al
    pop edx
    pop eax
    ret

serial_write_char:
    push edx
    push eax
.wait:
    mov dx, 0x3FD          ; Line status
    in al, dx
    test al, 0x20          ; THR empty?
    jz .wait
    mov dx, 0x3F8          ; THR
    pop eax                ; char in AL
    out dx, al
    pop edx
    ret

serial_print:
    push esi
    push eax
.loop:
    lodsb
    test al, al
    jz .done
    call serial_write_char
    jmp .loop
.done:
    pop eax
    pop esi
    ret

section .rodata
serial_banner db "[TBOS] Kernel entry active (serial)\r\n", 0
