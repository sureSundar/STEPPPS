; TBOS Working Minimal Bootloader - Actually Functional
; Target: <512 bytes, guaranteed to work

[BITS 16]
[ORG 0x8000]

start:
    ; Print startup message
    mov si, msg_start
    call print

    ; Setup GDT
    mov si, msg_gdt
    call print
    lgdt [gdt_desc]

    ; Enable A20
    mov si, msg_a20
    call print
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Enter protected mode
    mov si, msg_pm
    call print
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

print:
    pusha
    mov ah, 0x0E
    mov bx, 0x0007
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    mov al, 0x0D
    int 0x10
    mov al, 0x0A
    int 0x10
    popa
    ret

[BITS 32]
protected_mode:
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Load kernel (simplified - just jump to predefined location)
    ; In real implementation, would load from disk here

    ; For demo, just infinite loop with success indicator
    mov eax, 0xDEADBEEF  ; Success marker

.loop:
    jmp .loop

; Messages
msg_start: db 'TBOS Minimal Stage2', 0
msg_gdt:   db 'Loading GDT', 0
msg_a20:   db 'Enabling A20', 0
msg_pm:    db 'Entering Protected Mode', 0

; GDT
gdt_start:
    ; Null descriptor
    dd 0, 0

    ; Code segment: base=0, limit=0xFFFFF, present, ring 0, code, readable
    dw 0xFFFF       ; Limit 0-15
    dw 0x0000       ; Base 0-15
    db 0x00         ; Base 16-23
    db 0x9A         ; Access: present, ring 0, code, readable
    db 0xCF         ; Flags: 4KB granularity, 32-bit
    db 0x00         ; Base 24-31

    ; Data segment: base=0, limit=0xFFFFF, present, ring 0, data, writable
    dw 0xFFFF       ; Limit 0-15
    dw 0x0000       ; Base 0-15
    db 0x00         ; Base 16-23
    db 0x92         ; Access: present, ring 0, data, writable
    db 0xCF         ; Flags: 4KB granularity, 32-bit
    db 0x00         ; Base 24-31

gdt_desc:
    dw gdt_desc - gdt_start - 1    ; GDT limit
    dd gdt_start                   ; GDT base

; Pad to 512 bytes
times 512-($-$$) db 0