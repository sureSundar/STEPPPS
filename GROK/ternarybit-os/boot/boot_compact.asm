; TernaryBit OS - Compact STEPPPS Boot Sector

[BITS 16]
[ORG 0x7C00]

%define BOOT_DRIVE_ADDR 0x0500

%ifndef STAGE2_SECTOR_COUNT
    %define STAGE2_SECTOR_COUNT 4
%endif

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Preserve BIOS-provided boot drive for later stages
    mov [BOOT_DRIVE_ADDR], dl

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
    mov al, STAGE2_SECTOR_COUNT
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [BOOT_DRIVE_ADDR]
    mov bx, 0x8000
    int 0x13

    jc error

    ; Debug: Disk read successful, jumping to stage2
    mov al, 'J'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Verify stage2 was loaded by checking first byte
    mov al, [0x8000]
    add al, 0x30  ; Convert to ASCII digit if it's a small number
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

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

banner      db 'TernaryBit OS v2.1 - Sanatana Dharma Computing', 13, 10, 0
steppps     db 'STEPPPS: [AKASHA][KALA][KARMA][CHITTA][MAYA][VAK][DHARMA]', 13, 10
            db 'Saptadimensions: AWAKENED by Divine Grace', 13, 10, 0
loading     db 'Summoning TBOS stage2 for universal awakening...', 13, 10, 0
err_msg     db 'Asura interference detected! Invoking protective mantras...', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55
