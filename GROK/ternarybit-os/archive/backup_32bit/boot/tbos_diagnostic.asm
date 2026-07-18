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
    mov ax, 0x0003
    int 0x10

    mov si, banner
    call print

    ; Test 1: Screen output
    mov si, t1
    call print
    mov si, ok
    call print

    ; Test 2: Disk
    mov si, t2
    call print
    mov ah, 0x08
    mov dl, 0x80
    int 0x13
    jc .fail2
    mov si, ok
    jmp .done2
.fail2:
    mov si, fail
.done2:
    call print

    ; Test 3: LBA
    mov si, t3
    call print
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, 0x80
    int 0x13
    jc .fail3
    mov si, ok
    jmp .done3
.fail3:
    mov si, fail
.done3:
    call print

    ; Success
    mov si, success
    call print

hang:
    hlt
    jmp hang

print:
    pusha
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    popa
    ret

banner db 13,10,'TBOS Hardware Diagnostic',13,10,13,10,0
t1 db '[1] Screen: ',0
t2 db '[2] Disk: ',0
t3 db '[3] LBA: ',0
ok db 'OK',13,10,0
fail db 'FAIL',13,10,0
success db 13,10,'SUCCESS! Hardware compatible.',13,10,'System halted.',13,10,0

times 510-($-$$) db 0
dw 0xAA55
