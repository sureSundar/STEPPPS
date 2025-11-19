; TernaryBit OS - Minimal Stage 2 (Debug Version)
; Just shows a message and halts - no loops

[BITS 16]
[ORG 0x8000]

start:
    ; Show we got here
    mov si, stage2_msg
    call print_string

    ; Enter protected mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_start

[BITS 32]
protected_start:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Clear screen and show success message
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw

    ; Display success message
    mov esi, success_msg
    mov edi, 0xB8000
    call print_32

    ; Final halt - no loops
final_halt:
    hlt
    jmp final_halt

print_32:
    push eax
    push ebx
    mov ebx, edi
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x07
    mov [ebx], ax
    add ebx, 2
    jmp .loop
.done:
    pop ebx
    pop eax
    ret

[BITS 16]
print_string:
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

; GDT
gdt_start:
    dd 0x0, 0x0         ; Null descriptor
    dw 0xFFFF, 0x0000   ; Code segment
    db 0x00, 10011010b, 11001111b, 0x00
    dw 0xFFFF, 0x0000   ; Data segment
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Messages
stage2_msg      db 'Stage 2 running...', 0x0D, 0x0A, 0x00
success_msg     db 'TernaryBit OS - Protected Mode Success! No loops detected.', 0x00

times 2048-($-$$) db 0