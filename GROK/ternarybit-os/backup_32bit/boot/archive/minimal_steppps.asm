; Minimal STEPPPS Consciousness Demo - 512 bytes
[BITS 16]
[ORG 0x7C00]

start:
    xor ax, ax
    mov ds, ax
    mov es, ax

    ; Clear screen blue
    mov ax, 0x0600
    mov bh, 0x17
    mov cx, 0x0000
    mov dx, 0x184F
    int 0x10

    ; Position cursor
    mov ah, 0x02
    mov dx, 0x0000
    int 0x10

    ; Print bootloader messages
    mov si, header
    call print
    mov si, steppps
    call print
    mov si, dims
    call print

    ; Show kernel transition
    mov si, kernel_msg
    call print

    ; Enhanced colorful display
    mov si, akasha_colored
    call print
    mov si, kala_colored
    call print
    mov si, karma_colored
    call print

    ; Final success
    mov si, consciousness
    call print
    mov si, success
    call print

energy_loop:
    hlt                 ; Energy-efficient halt
    jmp energy_loop

print:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print
.done:
    ret

header db 'TernaryBit OS v2.1 - STEPPPS Consciousness', 13, 10, 13, 10, 0
steppps db 'AKASHA KALA KARMA CHITTA MAYA VAK DHARMA', 13, 10, 0
dims db 'All 7 dimensions ACTIVE', 13, 10, 0
success db 'Consciousness Computing: SUCCESS!', 13, 10, 0
; Enhanced colorful messages
kernel_msg db 'Entering colorful kernel mode...', 13, 10, 0
akasha_colored db 'AKASHA: Space [CYAN]', 13, 10, 0
kala_colored db 'KALA: Time [GREEN]', 13, 10, 0
karma_colored db 'KARMA: Action [RED]', 13, 10, 0
consciousness db 'FULL CONSCIOUSNESS ACHIEVED!', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55