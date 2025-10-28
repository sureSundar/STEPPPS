; TBOS v3.0 Revolutionary Bootloader - Stage 1 (512 bytes max)
; Minimal functional bootloader that loads Stage 2

[BITS 16]               ; 16-bit real mode
[ORG 0x7C00]           ; BIOS loads us at 0x7C00

start:
    ; Initialize segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Display TBOS banner
    mov si, banner_msg
    call print_string

    ; Detect basic hardware
    call detect_hardware

    ; Display detected persona
    mov si, persona_msg
    call print_string

    ; Load Stage 2 bootloader
    mov si, loading_msg
    call print_string
    call load_stage2

    ; Jump to Stage 2 (if successfully loaded)
    jmp stage2_entry

detect_hardware:
    ; Simple CPU detection
    mov byte [cpu_type], 0x86  ; Assume x86 for now

    ; Memory detection (simplified)
    int 0x12                   ; Get conventional memory
    mov [memory_size], ax

    ; Store detected persona (x86 = 2)
    mov byte [persona_id], 2
    ret

load_stage2:
    ; Load Stage 2 from next sectors
    mov ah, 0x02               ; Read sectors
    mov al, 4                  ; Read 4 sectors (2KB)
    mov ch, 0                  ; Cylinder 0
    mov cl, 2                  ; Start from sector 2
    mov dh, 0                  ; Head 0
    mov dl, 0x80               ; Drive 0x80 (hard disk)
    mov bx, stage2_entry       ; Load to 0x7E00
    int 0x13                   ; BIOS disk interrupt

    jc disk_error              ; Jump if carry flag set (error)
    ret

disk_error:
    mov si, error_msg
    call print_string
    hlt

print_string:
    ; Print null-terminated string at DS:SI
    mov ah, 0x0E               ; BIOS teletype output
.loop:
    lodsb                      ; Load byte from SI into AL
    or al, al                  ; Check if null terminator
    jz .done
    int 0x10                   ; BIOS video interrupt
    jmp .loop
.done:
    ret

print_hex:
    ; Print AL as hexadecimal
    push ax
    shr al, 4
    call print_hex_digit
    pop ax
    and al, 0x0F
    call print_hex_digit
    ret

print_hex_digit:
    ; Print single hex digit in AL
    add al, '0'
    cmp al, '9'
    jle .digit
    add al, 7                  ; Convert A-F
.digit:
    mov ah, 0x0E
    int 0x10
    ret

; Data section
banner_msg      db 'TBOS v3.0 Revolutionary Bootloader', 13, 10, 0
persona_msg     db 'Hardware Persona: x86 Detected', 13, 10, 0
loading_msg     db 'Loading Stage 2...', 13, 10, 0
error_msg       db 'Disk Error!', 13, 10, 0

; Hardware detection data
cpu_type        db 0
memory_size     dw 0
persona_id      db 0

; Stage 2 entry point (0x7E00)
stage2_entry    equ 0x7E00

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55                      ; Boot sector signature