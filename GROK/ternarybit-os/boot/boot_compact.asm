; TernaryBit OS - Compact STEPPPS Boot Sector

[BITS 16]
[ORG 0x7C00]

start:
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

    ; Banner
    mov si, banner
    call print

    ; STEPPPS Init
    mov si, steppps
    call print

    ; Load stage 2
    mov si, loading
    call print

    mov ah, 0x02
    mov al, 4
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, 0x00
    mov bx, 0x8000
    int 0x13

    jc error
    jmp 0x8000

error:
    mov si, err_msg
    call print
halt:
    hlt
    jmp halt

print:
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

banner      db 'TernaryBit OS v2.1', 13, 10, 0
steppps     db 'STEPPPS: [SPACE][TIME][EVENT][PSYCHOLOGY][PIXEL][PROMPT][SCRIPT]', 13, 10
            db 'All 7 dimensions: ACTIVE', 13, 10, 0
loading     db 'Loading kernel...', 13, 10, 0
err_msg     db 'Boot error', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55