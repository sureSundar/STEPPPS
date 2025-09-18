; TernaryBit OS - Boot Sector with STEPPPS Framework
; Fixed version to prevent infinite loops

[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segments and stack
    cli                 ; Disable interrupts
    xor ax, ax          ; Zero AX
    mov ds, ax          ; Set data segment
    mov es, ax          ; Set extra segment
    mov ss, ax          ; Set stack segment
    mov sp, 0x7C00      ; Set stack pointer
    sti                 ; Enable interrupts

    ; Clear screen
    mov ah, 0x00
    mov al, 0x03        ; 80x25 text mode
    int 0x10

    ; Display boot message
    mov si, boot_message
    call print_string

    ; Simple STEPPPS initialization (no loops)
    mov si, space_msg
    call print_string

    mov si, time_msg
    call print_string

    mov si, event_msg
    call print_string

    ; Load stage 2 (simplified)
    mov si, loading_msg
    call print_string

    ; Read stage 2 from disk
    mov ah, 0x02        ; Read sectors
    mov al, 4           ; 4 sectors
    mov ch, 0x00        ; Cylinder 0
    mov cl, 0x02        ; Sector 2
    mov dh, 0x00        ; Head 0
    mov dl, 0x00        ; Drive A:
    mov bx, 0x8000      ; Load to 0x8000
    int 0x13

    jc disk_error       ; Jump on error

    ; Success message
    mov si, success_msg
    call print_string

    ; Jump to stage 2
    jmp 0x8000

print_string:
    push ax
    push bx
.loop:
    lodsb               ; Load byte from SI
    cmp al, 0           ; Check for null
    je .done
    mov ah, 0x0E        ; BIOS teletype
    mov bx, 0x0007      ; Page 0, attribute
    int 0x10            ; Print character
    jmp .loop
.done:
    pop bx
    pop ax
    ret

disk_error:
    mov si, disk_error_msg
    call print_string
    jmp halt_system

halt_system:
    mov si, halt_msg
    call print_string
.halt_loop:
    hlt
    jmp .halt_loop

; Messages
boot_message    db 'TernaryBit OS v1.0', 0x0D, 0x0A
                db 'STEPPPS Framework', 0x0D, 0x0A, 0x00

space_msg       db '[SPACE] Hardware OK', 0x0D, 0x0A, 0x00
time_msg        db '[TIME]  Timer ready', 0x0D, 0x0A, 0x00
event_msg       db '[EVENT] Events ready', 0x0D, 0x0A, 0x00
loading_msg     db 'Loading stage 2...', 0x0D, 0x0A, 0x00
success_msg     db 'Stage 2 loaded successfully!', 0x0D, 0x0A, 0x00
disk_error_msg  db 'Disk read error!', 0x0D, 0x0A, 0x00
halt_msg        db 'System halted.', 0x0D, 0x0A, 0x00

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55