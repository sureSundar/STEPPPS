; Universal Stage 1 Bootloader (MBR, 512 bytes)
; Loads stage2 from disk into 0x0000:0x8000 and jumps there.
; Designed for BIOS/MBR boot on bare metal.

%define STAGE2_LOAD_SEG     0x0000
%define STAGE2_LOAD_OFF     0x8000
%ifndef STAGE2_START_SECTOR
%define STAGE2_START_SECTOR 2          ; sector numbering starts at 1
%endif
%ifndef STAGE2_SECTORS
%define STAGE2_SECTORS      8          ; keep in sync with build script
%endif
%define BOOT_DRIVE_ADDR     0x0500     ; handoff to stage2

[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; stack grows down inside boot sector
    sti

    ; remember boot drive for stage2
    mov [boot_drive], dl
    mov [BOOT_DRIVE_ADDR], dl

    ; simple banner
    mov si, msg_banner
    call print_string

    ; load stage2
    mov si, msg_load_stage2
    call print_string
    call load_stage2

    mov si, msg_ok
    call print_string

    jmp STAGE2_LOAD_SEG:STAGE2_LOAD_OFF

; ---------------------------------------------------------------------------
; load STAGE2_SECTORS from disk starting at STAGE2_START_SECTOR to 0x0000:0x8000
; ---------------------------------------------------------------------------
load_stage2:
    push ax
    push bx
    push cx
    push dx

    mov ah, 0x02            ; BIOS read sectors
    mov al, STAGE2_SECTORS
    mov ch, 0               ; cylinder
    mov cl, STAGE2_START_SECTOR
    mov dh, 0               ; head
    mov dl, [boot_drive]
    mov bx, STAGE2_LOAD_OFF
    mov ax, STAGE2_LOAD_SEG
    mov es, ax
    int 0x13
    jc .error

    pop dx
    pop cx
    pop bx
    pop ax
    ret
.error:
    mov si, msg_disk_error
    call print_string
    jmp halt

; ---------------------------------------------------------------------------
; print_string: SI -> null-terminated string
; ---------------------------------------------------------------------------
print_string:
    pusha
.next:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .next
.done:
    popa
    ret

halt:
    cli
    hlt
    jmp halt

; ---------------------------------------------------------------------------
; data
; ---------------------------------------------------------------------------
boot_drive:       db 0

msg_banner:       db 13,10,'[TBOS] Universal Stage1',13,10,0
msg_load_stage2:  db '[TBOS] Loading stage2...',13,10,0
msg_ok:           db '[TBOS] Jumping to stage2',13,10,0
msg_disk_error:   db '[ERR] Disk read failed - halt',13,10,0

times 510-($-$$) db 0
dw 0xAA55
