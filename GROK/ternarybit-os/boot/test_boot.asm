; Minimal boot sector to load and test compressed stage2
[BITS 16]
[ORG 0x7C00]

start:
    ; Setup segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    ; Print loading message
    mov si, loading_msg
    call print_string

    ; Load stage2 from sector 16 to 0x8000
    mov ah, 0x02        ; Read sectors
    mov al, 1           ; Number of sectors
    mov ch, 0           ; Cylinder
    mov cl, 17          ; Sector (16 + 1)
    mov dh, 0           ; Head
    mov dl, 0x00        ; Floppy A: drive
    mov bx, 0x8000      ; Load address
    int 0x13

    jc load_error

    ; Print success message
    mov si, success_msg
    call print_string

    ; Jump to stage2
    jmp 0x8000

load_error:
    mov si, error_msg
    call print_string
    jmp $

print_string:
    mov ah, 0x0E
    mov bx, 0x0007
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

loading_msg: db 'Loading TBOS compressed stage2...', 13, 10, 0
success_msg: db 'Stage2 loaded, jumping...', 13, 10, 0
error_msg: db 'Load failed!', 13, 10, 0

; Boot sector signature
times 510-($-$$) db 0
dw 0xAA55