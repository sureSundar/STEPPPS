; TBOS Sacred Bootloader - Sprint 1
; 🕉️ Swamiye Saranam Aiyappa 🕉️
; Minimal working boot sector

%ifndef STAGE2_SECTOR_COUNT
%define STAGE2_SECTOR_COUNT 8
%endif

[BITS 16]
[ORG 0x7C00]

start:
    ; Setup segments for real mode
    cli                     ; Disable interrupts
    xor ax, ax             ; Zero out AX
    mov ds, ax             ; Data segment = 0
    mov es, ax             ; Extra segment = 0
    mov ss, ax             ; Stack segment = 0
    mov sp, 0x7C00         ; Stack grows down from boot sector
    sti                     ; Re-enable interrupts

    ; Save boot drive to known location for stage2
    mov [0x0500], dl       ; Save to 0x0500 where stage2 expects it
    mov [boot_drive], dl   ; Also save locally

    ; Clear screen (PIXEL dimension initialization)
    mov ax, 0x0003         ; Text mode 80x25, 16 colors
    int 0x10               ; BIOS video interrupt

    ; Display sacred message (PROMPT dimension)
    mov si, msg_sacred
    call print_string

    ; Display boot progress
    mov si, msg_boot
    call print_string

    ; Load Stage 2 from disk (SCRIPT dimension)
    call load_stage2

    ; Display success
    mov si, msg_loaded
    call print_string

    ; Jump to Stage 2
    jmp 0x0000:0x8000

;==========================================
; Load Stage 2 Bootloader
;==========================================
load_stage2:
    push ax
    push bx
    push cx
    push dx

    ; Setup for INT 13h disk read
    mov ah, 0x02           ; Read sectors function
    mov al, STAGE2_SECTOR_COUNT
    mov ch, 0              ; Cylinder 0
    mov cl, 2              ; Start from sector 2
    mov dh, 0              ; Head 0
    mov dl, [boot_drive]   ; Drive number
    mov bx, 0x8000         ; Load to 0x8000

    int 0x13               ; BIOS disk services
    jc .disk_error         ; Jump if carry (error)

    pop dx
    pop cx
    pop bx
    pop ax
    ret

.disk_error:
    mov si, msg_disk_err
    call print_string
    jmp halt_system

;==========================================
; Print String Function
;==========================================
print_string:
    push ax
    push bx
.loop:
    lodsb                  ; Load byte from [SI] to AL
    or al, al              ; Check for null terminator
    jz .done               ; If zero, we're done
    mov ah, 0x0E           ; Teletype output
    mov bx, 0x0007         ; Page 0, white on black
    int 0x10               ; BIOS video interrupt
    jmp .loop
.done:
    pop bx
    pop ax
    ret

;==========================================
; Halt System
;==========================================
halt_system:
    cli                    ; Disable interrupts
    hlt                    ; Halt processor
    jmp halt_system        ; Infinite loop

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
