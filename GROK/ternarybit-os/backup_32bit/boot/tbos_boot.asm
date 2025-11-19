; TBOS Sacred Bootloader - Sprint 1
; 🕉️ Swamiye Saranam Aiyappa 🕉️
; Minimal working boot sector

[BITS 16]
[ORG 0x7C00]

%define BOOT_HAL_SECTORS_PER_TRACK 18
%define BOOT_HAL_HEADS_PER_CYLINDER 2

%include "boot_hal.inc"

%ifndef STAGE2_SECTOR_COUNT
%define STAGE2_SECTOR_COUNT 8
%endif

%ifndef STAGE2_LBA_START
%define STAGE2_LBA_START 1
%endif

%ifndef STAGE2_LOAD_OFFSET
%define STAGE2_LOAD_OFFSET 0x8000
%endif

%ifndef BOOT_DRIVE_ADDR
%define BOOT_DRIVE_ADDR 0x0500
%endif

start:
    ; Setup segments for real mode
    cli
    xor ax, ax
    mov ds, ax             ; Data segment = 0
    mov es, ax             ; Extra segment = 0
    mov ss, ax             ; Stack segment = 0
    mov sp, 0x7C00         ; Stack grows down from boot sector
    sti

    ; Save boot drive to known location for stage2
    mov [BOOT_DRIVE_ADDR], dl
    mov [boot_drive], dl   ; Also save locally

    ; Initialize console (PIXEL dimension initialization)
    call boot_hal_init_console

    ; Display sacred message (PROMPT dimension)
    mov si, msg_sacred
    call boot_hal_print_string

    ; Display boot progress
    mov si, msg_boot
    call boot_hal_print_string

    ; Load Stage 2 from disk (SCRIPT dimension)
    call load_stage2

    ; Display success
    mov si, msg_loaded
    call boot_hal_print_string

    ; Jump to Stage 2
    jmp 0x0000:0x8000

;==========================================
; Load Stage 2 Bootloader
;==========================================
load_stage2:
    pusha
    xor ax, ax
    mov es, ax
    mov bx, STAGE2_LOAD_OFFSET
    mov eax, STAGE2_LBA_START
    xor edx, edx
    mov cx, STAGE2_SECTOR_COUNT
    mov dl, [boot_drive]
    call boot_hal_read_lba
    jnc .ok

    mov si, msg_disk_err
    call boot_hal_print_string
    call boot_hal_halt

.ok:
    popa
    ret

;==========================================
; Print String Function
;==========================================
;==========================================
; Data Section
;==========================================
boot_drive:     db 0

msg_sacred:     db '=====================================', 13, 10
                db '     TBOS x STEPPPS Framework', 13, 10
                db '   Swamiye Saranam Aiyappa', 13, 10
                db '=====================================', 13, 10, 0

msg_boot:       db '[BOOT] Initializing Sacred System...', 13, 10, 0
msg_loaded:     db '[BOOT] Stage 2 loaded successfully', 13, 10
                db '[BOOT] Transferring control...', 13, 10, 0
msg_disk_err:   db '[ERROR] Disk read failed!', 13, 10
                db 'System halted.', 13, 10, 0

;==========================================
; Boot Sector Signature
;==========================================
times 510-($-$$) db 0      ; Pad to 510 bytes
dw 0xAA55                  ; Boot signature
