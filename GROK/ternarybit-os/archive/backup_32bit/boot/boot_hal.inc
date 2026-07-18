; Boot HAL - BIOS-backed hardware abstraction for Stage 1/2
; Provides uniform console, disk, and halt primitives during real-mode boot.
; All routines assume real-mode execution.

%ifndef BOOT_HAL_INC
%define BOOT_HAL_INC

%ifndef BOOT_HAL_SECTORS_PER_TRACK
%define BOOT_HAL_SECTORS_PER_TRACK 18
%endif

%ifndef BOOT_HAL_HEADS_PER_CYLINDER
%define BOOT_HAL_HEADS_PER_CYLINDER 2
%endif

%ifndef BOOT_HAL_MAX_TRANSFER
%define BOOT_HAL_MAX_TRANSFER 127
%endif

; -----------------------------------------------------------------------------
; Console primitives
; -----------------------------------------------------------------------------

boot_hal_init_console:
    mov ax, 0x0003            ; 80x25 text mode
    int 0x10
    ret

; DS:SI -> null-terminated string
boot_hal_print_string:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007            ; page 0, white on black
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; -----------------------------------------------------------------------------
; Disk access (EDD 1.1, INT 13h extensions). Expects:
;   DL  = BIOS drive number
;   EDX:EAX = starting LBA (64-bit, upper 32 bits in EDX)
;   CX  = sector count (1..127)
;   ES:BX = destination buffer
; On return: CF clear = success, CF set = failure.
; Registers restored to caller state.
; -----------------------------------------------------------------------------

boot_hal_read_lba:
    push ax
    push bx
    push cx
    push dx
    push si

    mov word [boot_hal_dap + 2], cx      ; sector count
    mov word [boot_hal_dap + 4], bx      ; offset
    mov dword [boot_hal_dap + 8], eax    ; LBA low
    mov dword [boot_hal_dap + 12], edx   ; LBA high
    mov ax, es
    mov word [boot_hal_dap + 6], ax      ; segment

    mov si, boot_hal_dap
    mov ah, 0x42
    int 0x13

    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; -----------------------------------------------------------------------------
; Halt helper (used for unrecoverable failures)
; -----------------------------------------------------------------------------
boot_hal_halt:
    cli
    hlt
    jmp boot_hal_halt

; -----------------------------------------------------------------------------
; Data - Disk Address Packet (16 bytes)
; -----------------------------------------------------------------------------
boot_hal_dap:
    db 0x10        ; size of packet
    db 0x00        ; reserved
    dw 0x0000      ; sector count (patched at runtime)
    dw 0x0000      ; buffer offset
    dw 0x0000      ; buffer segment
    dd 0x00000000  ; starting LBA (low 32 bits)
    dd 0x00000000  ; starting LBA (high 32 bits)

%endif ; BOOT_HAL_INC
