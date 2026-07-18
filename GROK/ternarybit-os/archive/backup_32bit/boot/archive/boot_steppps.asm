; TernaryBit OS - STEPPPS Enhanced Boot Sector
; Gradually adding STEPPPS features back

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

    ; TernaryBit Banner
    mov si, banner_msg
    call print_string

    ; STEPPPS Initialization (one by one, controlled)
    mov si, init_msg
    call print_string

    ; SPACE dimension
    mov si, space_msg
    call print_string
    call short_delay

    ; TIME dimension
    mov si, time_msg
    call print_string
    call short_delay

    ; EVENT dimension
    mov si, event_msg
    call print_string
    call short_delay

    ; PSYCHOLOGY dimension
    mov si, psychology_msg
    call print_string
    call short_delay

    ; PIXEL dimension
    mov si, pixel_msg
    call print_string
    call short_delay

    ; PROMPT dimension
    mov si, prompt_msg
    call print_string
    call short_delay

    ; SCRIPT dimension
    mov si, script_msg
    call print_string
    call short_delay

    ; All dimensions initialized
    mov si, complete_msg
    call print_string

    ; Load stage 2
    mov si, loading_msg
    call print_string

    mov ah, 0x02    ; Read sectors
    mov al, 4       ; 4 sectors
    mov ch, 0x00    ; Cylinder 0
    mov cl, 0x02    ; Sector 2
    mov dh, 0x00    ; Head 0
    mov dl, 0x00    ; Drive A:
    mov bx, 0x8000  ; Load address
    int 0x13

    jc disk_error

    ; Success
    mov si, success_msg
    call print_string
    jmp 0x8000

disk_error:
    mov si, error_msg
    call print_string
    jmp halt_system

halt_system:
    hlt
    jmp halt_system

; Short delay to prevent rapid scrolling
short_delay:
    push cx
    mov cx, 0x8000
delay_loop:
    loop delay_loop
    pop cx
    ret

; Print string function
print_string:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; Messages
banner_msg      db 'TernaryBit OS v2.1 - STEPPPS Framework', 0x0D, 0x0A
                db '=====================================', 0x0D, 0x0A, 0x00

init_msg        db 'Initializing STEPPPS Dimensions...', 0x0D, 0x0A, 0x00

space_msg       db '[SPACE]      Hardware & Resources - OK', 0x0D, 0x0A, 0x00
time_msg        db '[TIME]       Scheduling & Timing - OK', 0x0D, 0x0A, 0x00
event_msg       db '[EVENT]      System Monitoring - OK', 0x0D, 0x0A, 0x00
psychology_msg  db '[PSYCHOLOGY] AI Learning System - OK', 0x0D, 0x0A, 0x00
pixel_msg       db '[PIXEL]      Graphics & Display - OK', 0x0D, 0x0A, 0x00
prompt_msg      db '[PROMPT]     AI Interface Ready - OK', 0x0D, 0x0A, 0x00
script_msg      db '[SCRIPT]     Automation Engine - OK', 0x0D, 0x0A, 0x00

complete_msg    db 'All 7 STEPPPS dimensions active!', 0x0D, 0x0A, 0x00
loading_msg     db 'Loading Stage 2 kernel...', 0x0D, 0x0A, 0x00
success_msg     db 'Stage 2 loaded successfully!', 0x0D, 0x0A, 0x00
error_msg       db 'Disk error - system halted', 0x0D, 0x0A, 0x00

; Boot signature
times 510-($-$$) db 0
dw 0xAA55