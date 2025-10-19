; Minimal bootloader test
[BITS 16]
[ORG 0x7C00]

start:
    ; Set up segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Print message
    mov si, msg
    call print_string
    
    ; Halt
    cli
    hlt

print_string:
    pusha
    mov ah, 0x0E
    mov bh, 0x00
.next_char:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .next_char
.done:
    popa
    ret

msg db 'TBOS Minimal Bootloader', 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
