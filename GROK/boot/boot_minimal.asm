; TernaryBit OS - Minimal Boot Sector (Debug Version)
; Absolutely minimal to identify loop source

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

    ; Show simple message
    mov si, simple_msg
    call print_string

    ; Try to load stage 2
    mov ah, 0x02    ; Read sectors
    mov al, 4       ; 4 sectors
    mov ch, 0x00    ; Cylinder 0
    mov cl, 0x02    ; Sector 2
    mov dh, 0x00    ; Head 0
    mov dl, 0x00    ; Drive A:
    mov bx, 0x8000  ; Load address
    int 0x13

    jc error_halt   ; Jump on error

    ; Success - show message and jump
    mov si, loaded_msg
    call print_string
    jmp 0x8000

error_halt:
    mov si, error_msg
    call print_string

final_halt:
    hlt
    jmp final_halt

; Simple print function
print_string:
    push ax
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop ax
    ret

; Minimal messages
simple_msg      db 'TernaryBit Boot Test', 0x0D, 0x0A, 0x00
loaded_msg      db 'Stage 2 loaded successfully', 0x0D, 0x0A, 0x00
error_msg       db 'Boot error - halting', 0x0D, 0x0A, 0x00

; Boot signature
times 510-($-$$) db 0
dw 0xAA55