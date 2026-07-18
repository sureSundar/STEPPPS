; TernaryBit OS - Boot Sector with STEPPPS Framework
; First stage bootloader (512 bytes)
; Implements the very first STEPPPS initialization at hardware level

[BITS 16]
[ORG 0x7C00]

start:
    ; Clear screen and initialize display (PIXEL dimension)
    mov ah, 0x00
    mov al, 0x03    ; 80x25 text mode
    int 0x10

    ; Set cursor position
    mov ah, 0x02
    mov bh, 0x00
    mov dx, 0x0000
    int 0x10

    ; Display TernaryBit boot message (PROMPT dimension)
    mov si, boot_message
    call print_string

    ; Initialize basic STEPPPS dimensions
    call init_steppps_space     ; SPACE: Hardware detection
    call init_steppps_time      ; TIME: Timer setup
    call init_steppps_event     ; EVENT: Interrupt preparation

    ; Load second stage bootloader
    mov si, loading_message
    call print_string

    ; Read second stage from disk (SCRIPT dimension)
    mov ah, 0x02    ; Read sectors
    mov al, 4       ; 4 sectors (2KB)
    mov ch, 0x00    ; Cylinder 0
    mov cl, 0x02    ; Sector 2 (after boot sector)
    mov dh, 0x00    ; Head 0
    mov bx, 0x8000  ; Load to 0x8000
    int 0x13

    jc disk_error   ; Jump if carry flag set (error)

    ; Jump to second stage
    jmp 0x8000

; STEPPPS Dimension Initializers
init_steppps_space:
    ; SPACE: Basic hardware detection
    push ax
    push si

    mov si, space_msg
    call print_string

    ; Detect memory (basic)
    int 0x12        ; Get low memory size in KB
    mov [memory_size], ax

    ; Detect CPU type (basic)
    pushf
    pop ax
    mov cx, ax
    xor ax, 0x1000  ; Try to toggle ID flag
    push ax
    popf
    pushf
    pop ax
    xor ax, cx      ; Check if flag changed
    jz cpu_8086     ; If no change, it's 8086

    mov byte [cpu_type], 1  ; 386+ CPU
    jmp space_done

cpu_8086:
    mov byte [cpu_type], 0  ; 8086 CPU

space_done:
    pop si
    pop ax
    ret

init_steppps_time:
    ; TIME: Initialize system timer
    push ax
    push si

    mov si, time_msg
    call print_string

    ; Set up timer tick counter
    mov word [timer_ticks], 0

    ; Get current time from RTC
    mov ah, 0x02
    int 0x1A        ; Get RTC time

    pop si
    pop ax
    ret

init_steppps_event:
    ; EVENT: Prepare interrupt handling
    push ax
    push si

    mov si, event_msg
    call print_string

    ; Initialize interrupt count
    mov word [interrupt_count], 0

    pop si
    pop ax
    ret

; Utility functions
print_string:
    push ax
    push bx

print_loop:
    lodsb           ; Load byte from SI into AL
    cmp al, 0       ; Check for null terminator
    je print_done

    mov ah, 0x0E    ; BIOS teletype function
    mov bx, 0x0007  ; Page 0, light gray on black
    int 0x10        ; Call BIOS interrupt

    jmp print_loop

print_done:
    pop bx
    pop ax
    ret

disk_error:
    mov si, disk_error_msg
    call print_string
    hlt             ; Halt system

; Data section
boot_message    db 'TernaryBit OS v1.0 - STEPPPS Framework', 0x0D, 0x0A
                db '==========================================', 0x0D, 0x0A
                db 'Initializing STEPPPS dimensions...', 0x0D, 0x0A, 0x00

space_msg       db '[SPACE] Hardware detection...', 0x0D, 0x0A, 0x00
time_msg        db '[TIME]  Timer initialization...', 0x0D, 0x0A, 0x00
event_msg       db '[EVENT] Interrupt preparation...', 0x0D, 0x0A, 0x00
loading_message db 'Loading stage 2 bootloader...', 0x0D, 0x0A, 0x00
disk_error_msg  db 'Disk read error! System halted.', 0x0D, 0x0A, 0x00

; STEPPPS state variables
memory_size     dw 0        ; Memory size in KB
cpu_type        db 0        ; CPU type (0=8086, 1=386+)
timer_ticks     dw 0        ; Timer tick counter
interrupt_count dw 0        ; Interrupt counter

; Boot signature
times 510-($-$$) db 0       ; Pad to 510 bytes
dw 0xAA55                   ; Boot sector signature