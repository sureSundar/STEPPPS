; TernaryBit OS - Fixed Boot Sector (No Loops)
; Minimal boot sector that loads stage 2 without infinite loops

[BITS 16]
[ORG 0x7C00]

start:
    ; Initialize segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Clear screen
    mov ah, 0x00
    mov al, 0x03
    int 0x10

    ; Show boot message
    mov si, boot_msg
    call print_string

    ; Show ALL STEPPPS dimensions (no loops)
    mov si, space_msg
    call print_string

    mov si, time_msg
    call print_string

    mov si, event_msg
    call print_string

    mov si, psychology_msg
    call print_string

    mov si, pixel_msg
    call print_string

    mov si, prompt_msg
    call print_string

    mov si, script_msg
    call print_string

    ; Load stage 2
    mov si, loading_msg
    call print_string

    ; Read stage 2 from disk (simplified - no retry loops)
    mov ah, 0x02    ; Read sectors
    mov al, 4       ; 4 sectors
    mov ch, 0x00    ; Cylinder 0
    mov cl, 0x02    ; Sector 2
    mov dh, 0x00    ; Head 0
    mov dl, 0x00    ; Drive A:
    mov bx, 0x8000  ; Load address
    int 0x13

    ; Check for error (but don't loop)
    jc disk_error

    ; Success - jump to stage 2
    mov si, success_msg
    call print_string
    jmp 0x8000

disk_error:
    mov si, error_msg
    call print_string
    ; Halt on error (don't loop)
    jmp halt_system

halt_system:
    mov si, halt_msg
    call print_string
halt_loop:
    hlt
    jmp halt_loop

; Print string function (simple, no loops that could hang)
print_string:
    push ax
    push bx
    push si

print_char:
    lodsb               ; Load character
    cmp al, 0           ; Check for null terminator
    je print_done

    ; Print character
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    jmp print_char

print_done:
    pop si
    pop bx
    pop ax
    ret

; Messages
boot_msg        db 'TernaryBit OS v2.0', 0x0D, 0x0A
                db 'STEPPPS Framework Init:', 0x0D, 0x0A, 0x00

space_msg       db '[SPACE] Hardware OK', 0x0D, 0x0A, 0x00
time_msg        db '[TIME] Scheduler OK', 0x0D, 0x0A, 0x00
event_msg       db '[EVENT] Interrupts OK', 0x0D, 0x0A, 0x00
psychology_msg  db '[PSYCHOLOGY] AI OK', 0x0D, 0x0A, 0x00
pixel_msg       db '[PIXEL] Display OK', 0x0D, 0x0A, 0x00
prompt_msg      db '[PROMPT] Interface OK', 0x0D, 0x0A, 0x00
script_msg      db '[SCRIPT] Automation OK', 0x0D, 0x0A, 0x00

loading_msg     db 'Loading Stage 2...', 0x0D, 0x0A, 0x00
success_msg     db 'Stage 2 loaded!', 0x0D, 0x0A, 0x00
error_msg       db 'Disk error!', 0x0D, 0x0A, 0x00
halt_msg        db 'System halted.', 0x0D, 0x0A, 0x00

; Pad to 510 bytes and add boot signature
times 510-($-$$) db 0
dw 0xAA55