; TernaryBit OS - Stage 2 with Hardware Detection Test
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 16]
[ORG 0x8000]

; === ENTRY POINT ===
stage2_start:
    ; Setup segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000
    sti

    ; Display banner
    mov si, msg_banner
    call print_string

    ; === MAIN TEST: Hardware Detection ===
    call hardware_detect_init

    ; Display completion message
    mov si, msg_complete
    call print_string

    ; Halt system
    jmp halt_system

;==========================================
; Print String Function
;==========================================
print_string:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

;==========================================
; Halt System
;==========================================
halt_system:
    mov si, msg_halt
    call print_string
    cli
    hlt
    jmp halt_system

;==========================================
; Messages
;==========================================
msg_banner:         db 13, 10
                    db '╔══════════════════════════════════════════╗', 13, 10
                    db '║  TBOS Universal Hardware Detection Test ║', 13, 10
                    db '║     Swamiye Saranam Aiyappa            ║', 13, 10
                    db '╚══════════════════════════════════════════╝', 13, 10
                    db 13, 10, 0

msg_complete:       db 13, 10
                    db '=== Hardware Detection Complete! ===', 13, 10
                    db 0

msg_halt:           db 13, 10
                    db 'System halted. Press Ctrl+Alt+Del to reboot.', 13, 10, 0

;==========================================
; Include Hardware Detection Module
;==========================================
%include "boot/hardware_detect.asm"

; Pad to 4KB
times 4096-($-$$) db 0
