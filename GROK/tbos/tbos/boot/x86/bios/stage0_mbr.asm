; TernaryBit OS - Minimal x86 MBR Bootloader
; Fits in 512 bytes, loads Stage 2
; BIOS boots this code from sector 0

[BITS 16]
[ORG 0x7C00]

start:
    ; Setup segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Print boot message
    mov si, msg_boot
    call print_string

    ; Load stage 2 from disk (sectors 2-10, 8 sectors = 4KB)
    mov ah, 0x02        ; BIOS read sectors
    mov al, 8           ; Number of sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Start from sector 2
    mov dh, 0           ; Head 0
    mov bx, 0x8000      ; Load to 0x8000
    int 0x13
    jc disk_error

    ; Print success
    mov si, msg_loaded
    call print_string

    ; Jump to stage 2
    jmp 0x0000:0x8000

disk_error:
    mov si, msg_error
    call print_string
    jmp $

; Print string function
; SI = pointer to null-terminated string
print_string:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Messages
msg_boot    db 'TernaryBit OS Bootloader', 13, 10, 0
msg_loaded  db 'Loading Stage 2...', 13, 10, 0
msg_error   db 'Disk Error!', 13, 10, 0

; Pad to 510 bytes
times 510-($-$$) db 0

; Boot signature
dw 0xAA55
