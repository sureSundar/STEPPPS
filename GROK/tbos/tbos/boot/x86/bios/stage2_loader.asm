; TernaryBit OS - Stage 2 Loader
; Loaded by MBR at 0x8000
; Transitions to protected mode and loads VM

[BITS 16]
[ORG 0x8000]

start:
    ; Print Stage 2 message
    mov si, msg_stage2
    call print_string

    ; Enable A20 line (required for protected mode)
    call enable_a20

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:protected_mode

; Enable A20 line
enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

; Print string (16-bit real mode)
print_string:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    popa
    ret

; Messages
msg_stage2 db 'Stage 2 Loaded', 13, 10, 'Entering Protected Mode...', 13, 10, 0

; GDT
gdt_start:
    ; Null descriptor
    dq 0

    ; Code segment descriptor
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10011010b    ; Access: present, ring 0, code, executable, readable
    db 11001111b    ; Flags + Limit high: 4KB granularity, 32-bit
    db 0x00         ; Base high

    ; Data segment descriptor
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10010010b    ; Access: present, ring 0, data, writable
    db 11001111b    ; Flags + Limit high
    db 0x00         ; Base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                 ; Offset

[BITS 32]
protected_mode:
    ; Setup segments
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Set stack

    ; Print message to VGA (protected mode)
    mov edi, 0xB8000    ; VGA text mode buffer
    mov esi, msg_pmode
    call print_string_pmode

    ; Halt (for now - would load VM here)
    hlt

; Print string in protected mode (VGA text)
print_string_pmode:
    mov ah, 0x0F        ; White on black
.loop:
    lodsb
    cmp al, 0
    je .done
    stosw
    jmp .loop
.done:
    ret

msg_pmode db 'TernaryBit OS - Protected Mode Active', 0

; Pad to 4096 bytes (8 sectors)
times 4096-($-$$) db 0
