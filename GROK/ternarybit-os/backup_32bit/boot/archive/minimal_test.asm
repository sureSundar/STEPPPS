[BITS 16]
[ORG 0x7C00]

start:
    cli
    mov ax, 0x0000
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Print 'A' to screen
    mov ah, 0x0E
    mov al, 'A'
    int 0x10

    ; Halt
    cli
    hlt

times 510-($-$$) db 0
dw 0xAA55
