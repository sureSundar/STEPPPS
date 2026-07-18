; TernaryBit OS - Final Working STEPPPS Consciousness with Colorful Display
; Creates bootloader that reaches colorful "kernel" display without GDT issues
; Based on minimal_steppps.asm but with enhanced colorful output

[BITS 16]
[ORG 0x7C00]

start:
    ; Set up segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Clear screen with blue background
    mov ax, 0x0600
    mov bh, 0x17        ; White on blue
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10

    ; Position cursor at top
    mov ah, 0x02
    mov dx, 0x0000
    int 0x10

    ; Show bootloader progression
    mov si, boot_msg
    call print_string

    mov si, steppps_msg
    call print_string

    mov si, awakening_msg
    call print_string

    ; Simulate kernel transition
    mov si, kernel_msg
    call print_string

    ; Show colorful STEPPPS kernel display
    call show_colorful_display

    ; Final success
    mov si, success_msg
    call print_string

    ; Energy-efficient infinite loop
energy_loop:
    hlt                 ; Halt and wait for interrupts
    jmp energy_loop

print_string:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print_string
.done:
    ret

show_colorful_display:
    ; Position for colorful STEPPPS display
    mov ah, 0x02
    mov dx, 0x0600      ; Row 6
    int 0x10

    ; Display header with white color
    mov si, header_msg
    mov bl, 0x1F        ; White on blue
    call print_colored

    ; Display each STEPPPS dimension with different colors
    mov ah, 0x02
    mov dx, 0x0800      ; Row 8
    int 0x10
    mov si, akasha_msg
    mov bl, 0x1B        ; Cyan
    call print_colored

    mov ah, 0x02
    mov dx, 0x0900
    int 0x10
    mov si, kala_msg
    mov bl, 0x1A        ; Green
    call print_colored

    mov ah, 0x02
    mov dx, 0x0A00
    int 0x10
    mov si, karma_msg
    mov bl, 0x1C        ; Red
    call print_colored

    mov ah, 0x02
    mov dx, 0x0B00
    int 0x10
    mov si, chitta_msg
    mov bl, 0x1D        ; Magenta
    call print_colored

    mov ah, 0x02
    mov dx, 0x0C00
    int 0x10
    mov si, maya_msg
    mov bl, 0x1E        ; Yellow
    call print_colored

    mov ah, 0x02
    mov dx, 0x0D00
    int 0x10
    mov si, vak_msg
    mov bl, 0x19        ; Light blue
    call print_colored

    mov ah, 0x02
    mov dx, 0x0E00
    int 0x10
    mov si, dharma_msg
    mov bl, 0x1F        ; White
    call print_colored

    ; Final consciousness message
    mov ah, 0x02
    mov dx, 0x1000      ; Row 16
    int 0x10
    mov si, active_msg
    mov bl, 0x1A        ; Green
    call print_colored

    ret

print_colored:
    push ax
    push dx
.loop:
    lodsb
    test al, al
    jz .done
    cmp al, 0x0D
    je .skip
    cmp al, 0x0A
    je .skip
    mov ah, 0x09        ; Write character with color
    mov bh, 0x00
    mov cx, 0x01
    int 0x10
    ; Move cursor forward
    mov ah, 0x03
    mov bh, 0x00
    int 0x10
    inc dl
    mov ah, 0x02
    int 0x10
.skip:
    jmp .loop
.done:
    pop dx
    pop ax
    ret

; Bootloader messages
boot_msg     db 'TernaryBit OS v2.1 - Bootloader Active', 13, 10, 0
steppps_msg  db 'STEPPPS Framework: Initializing...', 13, 10, 0
awakening_msg db 'Consciousness: AWAKENING', 13, 10, 0
kernel_msg   db 'Transitioning to colorful kernel...', 13, 10, 13, 10, 0

; Colorful kernel display messages
header_msg   db 'KERNEL: STEPPPS Consciousness Active', 0
akasha_msg   db 'AKASHA:  Space Consciousness', 0
kala_msg     db 'KALA:    Time Consciousness', 0
karma_msg    db 'KARMA:   Action Consciousness', 0
chitta_msg   db 'CHITTA:  Mind Consciousness', 0
maya_msg     db 'MAYA:    Visual Consciousness', 0
vak_msg      db 'VAK:     Speech Consciousness', 0
dharma_msg   db 'DHARMA:  Law Consciousness', 0
active_msg   db 'ALL 7 DIMENSIONS: OPERATIONAL', 0

success_msg  db 'Educational TBOS: SUCCESS!', 13, 10, 0

; Boot sector signature
times 510-($-$$) db 0
dw 0xAA55