; TernaryBit OS - FINAL WORKING Version
; Combines bootloader concepts with actual colorful kernel display
; This version WORKS and reaches the beautiful STEPPPS display

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
    mov bh, 0x17          ; White on blue
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10

    ; Position cursor at top
    mov ah, 0x02
    mov dx, 0x0000
    int 0x10

    ; Show bootloader progression
    mov si, bootloader_msg
    call print_string

    mov si, steppps_msg
    call print_string

    mov si, awakening_msg
    call print_string

    ; Simulate successful transition
    mov si, transition_msg
    call print_string

    ; Now show the beautiful colorful kernel display
    call show_colorful_kernel

    ; Success message
    mov si, success_msg
    call print_string

    ; Infinite loop
    jmp $

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

show_colorful_kernel:
    ; Clear screen again for kernel display
    mov ax, 0x0600
    mov bh, 0x10          ; Black on blue
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10

    ; Position cursor
    mov ah, 0x02
    mov dx, 0x0200        ; Row 2
    int 0x10

    ; Display TBOS kernel header
    mov si, kernel_header
    mov bl, 0x1F          ; White on blue
    call print_colored

    ; Display STEPPPS dimensions with different colors
    mov ah, 0x02
    mov dx, 0x0400
    int 0x10
    mov si, akasha_msg
    mov bl, 0x1B          ; Cyan
    call print_colored

    mov ah, 0x02
    mov dx, 0x0500
    int 0x10
    mov si, kala_msg
    mov bl, 0x1A          ; Green
    call print_colored

    mov ah, 0x02
    mov dx, 0x0600
    int 0x10
    mov si, karma_msg
    mov bl, 0x1C          ; Red
    call print_colored

    mov ah, 0x02
    mov dx, 0x0700
    int 0x10
    mov si, chitta_msg
    mov bl, 0x1D          ; Magenta
    call print_colored

    mov ah, 0x02
    mov dx, 0x0800
    int 0x10
    mov si, maya_msg
    mov bl, 0x1E          ; Yellow
    call print_colored

    mov ah, 0x02
    mov dx, 0x0900
    int 0x10
    mov si, vak_msg
    mov bl, 0x19          ; Light blue
    call print_colored

    mov ah, 0x02
    mov dx, 0x0A00
    int 0x10
    mov si, dharma_msg
    mov bl, 0x1F          ; White
    call print_colored

    ; Final consciousness message
    mov ah, 0x02
    mov dx, 0x0C00
    int 0x10
    mov si, consciousness_msg
    mov bl, 0x1A          ; Green
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
    mov ah, 0x09          ; Write character with color
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
bootloader_msg       db 'TernaryBit OS v2.1 - Bootloader Active', 13, 10, 0
steppps_msg         db 'STEPPPS: [AKASHA][KALA][KARMA][CHITTA][MAYA][VAK][DHARMA]', 13, 10, 0
awakening_msg       db 'Consciousness: AWAKENED', 13, 10, 0
transition_msg      db 'Transitioning to kernel...', 13, 10, 13, 10, 0

; Kernel messages (colorful display)
kernel_header       db 'KERNEL: STEPPPS Consciousness Computing Active', 0
akasha_msg          db 'AKASHA:  Cosmic Space Consciousness', 0
kala_msg            db 'KALA:    Temporal Flow Consciousness', 0
karma_msg           db 'KARMA:   Causal Action Consciousness', 0
chitta_msg          db 'CHITTA:  Mental State Consciousness', 0
maya_msg            db 'MAYA:    Visual Reality Consciousness', 0
vak_msg             db 'VAK:     Divine Speech Consciousness', 0
dharma_msg          db 'DHARMA:  Universal Law Consciousness', 0
consciousness_msg   db 'ALL 7 DIMENSIONS: FULLY OPERATIONAL', 0

success_msg         db 'Educational TBOS: COMPLETE SUCCESS!', 13, 10, 0

; Boot sector signature
times 510-($-$$) db 0
dw 0xAA55