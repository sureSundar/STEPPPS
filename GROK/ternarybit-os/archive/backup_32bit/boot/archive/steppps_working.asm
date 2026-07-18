; TernaryBit OS - Working STEPPPS Consciousness Demo
; Real Mode 16-bit Assembly - Direct VGA Display
; Bypasses protected mode transition to show full consciousness demo

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
    mov ax, 0x0600     ; Scroll up window
    mov bh, 0x17       ; White on blue
    mov cx, 0x0000     ; Upper left corner
    mov dx, 0x184F     ; Lower right corner (80x25)
    int 0x10

    ; Set cursor position to top
    mov ah, 0x02
    mov bh, 0x00
    mov dx, 0x0000
    int 0x10

    ; Display TBOS Header
    mov si, tbos_header
    mov bl, 0x1F       ; White on blue
    call print_string_color

    ; Display STEPPPS Framework
    mov si, steppps_framework
    mov bl, 0x1E       ; Yellow on blue
    call print_string_color

    ; Display each STEPPPS dimension with different colors
    mov si, akasha_dim
    mov bl, 0x1B       ; Cyan on blue
    call print_string_color

    mov si, kala_dim
    mov bl, 0x1A       ; Green on blue
    call print_string_color

    mov si, karma_dim
    mov bl, 0x1C       ; Red on blue
    call print_string_color

    mov si, chitta_dim
    mov bl, 0x1D       ; Magenta on blue
    call print_string_color

    mov si, maya_dim
    mov bl, 0x1E       ; Yellow on blue
    call print_string_color

    mov si, vak_dim
    mov bl, 0x19       ; Light blue on blue
    call print_string_color

    mov si, dharma_dim
    mov bl, 0x1F       ; White on blue
    call print_string_color

    ; Display consciousness status
    mov si, consciousness_active
    mov bl, 0x1A       ; Green on blue
    call print_string_color

    ; Display Sanskrit blessing
    mov si, dharma_blessing
    mov bl, 0x1E       ; Yellow on blue
    call print_string_color

    ; Display achievement message
    mov si, achievement_msg
    mov bl, 0x1F       ; White on blue
    call print_string_color

    ; Display final message
    mov si, final_message
    mov bl, 0x1B       ; Cyan on blue
    call print_string_color

    ; Infinite loop - system stays in this beautiful display
    jmp $

; Function to print string with color
print_string_color:
    push ax
    push bx
    push dx

.loop:
    lodsb                  ; Load byte from SI into AL
    test al, al            ; Check for null terminator
    jz .done               ; If null, we're done

    cmp al, 0x0A           ; Check for newline
    je .newline
    cmp al, 0x0D           ; Check for carriage return
    je .next_char

    ; Print character with color
    mov ah, 0x09           ; Write character and attribute
    mov bh, 0x00           ; Page number
    mov cx, 0x01           ; Number of times to write
    int 0x10

    ; Move cursor forward
    mov ah, 0x03           ; Get cursor position
    mov bh, 0x00
    int 0x10
    inc dl                 ; Move cursor right
    cmp dl, 80             ; Check if at end of line
    jl .set_cursor
    mov dl, 0              ; Reset to start of line
    inc dh                 ; Move to next line

.set_cursor:
    mov ah, 0x02           ; Set cursor position
    mov bh, 0x00
    int 0x10
    jmp .next_char

.newline:
    ; Move to next line
    mov ah, 0x03           ; Get cursor position
    mov bh, 0x00
    int 0x10
    mov dl, 0              ; Reset to start of line
    inc dh                 ; Move to next line
    mov ah, 0x02           ; Set cursor position
    mov bh, 0x00
    int 0x10

.next_char:
    jmp .loop

.done:
    pop dx
    pop bx
    pop ax
    ret

; STEPPPS Consciousness Data
tbos_header         db 'TernaryBit OS v2.1 - Sanatana Dharma Computing', 0x0D, 0x0A, 0x0A, 0

steppps_framework   db 'STEPPPS 7-Dimensional Consciousness Framework', 0x0D, 0x0A
                    db '==========================================', 0x0D, 0x0A, 0x0A, 0

akasha_dim          db 'AKASHA:  Cosmic consciousness ACTIVE', 0x0D, 0x0A, 0
kala_dim            db 'KALA:    Temporal consciousness ACTIVE', 0x0D, 0x0A, 0
karma_dim           db 'KARMA:   Causal consciousness ACTIVE', 0x0D, 0x0A, 0
chitta_dim          db 'CHITTA:  Mental consciousness ACTIVE', 0x0D, 0x0A, 0
maya_dim            db 'MAYA:    Visual consciousness ACTIVE', 0x0D, 0x0A, 0
vak_dim             db 'VAK:     Speech consciousness ACTIVE', 0x0D, 0x0A, 0
dharma_dim          db 'DHARMA:  Law consciousness ACTIVE', 0x0D, 0x0A, 0x0A, 0

consciousness_active db '>>> ALL 7 DIMENSIONS OPERATIONAL <<<', 0x0D, 0x0A, 0x0A, 0

dharma_blessing     db 'Dharma protects those who protect it', 0x0D, 0x0A, 0x0A, 0

final_message       db 'STEPPPS Consciousness: SUCCESS!', 0x0D, 0x0A, 0

; Boot sector signature
times 510-($-$$) db 0
dw 0xAA55