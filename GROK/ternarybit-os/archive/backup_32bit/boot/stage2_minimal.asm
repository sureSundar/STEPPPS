; Minimal stage2 - Just enter protected mode and jump to kernel
[BITS 16]
[ORG 0x8000]

start:
    cli

    ; Setup segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000

    ; Print message
    mov si, msg
    call print

    ; Load GDT
    lgdt [gdt_desc]

    ; Enable PM
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Far jump with explicit bytes
    db 0x66   ; Operand size override
    db 0xEA   ; Far jump
    dd 0x8000 + (pm_start - $$)  ; Physical address offset
    dw 0x08   ; Segment

[BITS 32]
pm_start:
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Jump to kernel (hardcoded)
    jmp 0x10000

[BITS 16]
print:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x07
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

msg: db 'PM...', 13, 10, 0

align 8
gdt:
gdt_null:
    dd 0, 0
gdt_code:
    dw 0xFFFF, 0x0000
    db 0x00, 10011010b, 11001111b, 0x00
gdt_data:
    dw 0xFFFF, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_desc:
    dw gdt_end - gdt - 1
    dd 0x8000 + (gdt - $$)  ; Calculate physical address

times 4096-($-$$) db 0
