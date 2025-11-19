; Minimal Working Shell - Starting from proven Step 3A
[BITS 32]
[GLOBAL _start]

section .text
_start:
    call vga_clear_screen

    mov esi, banner
    mov bl, 0x0B
    call vga_print_string

    call setup_idt
    call setup_pic
    sti

    mov esi, prompt
    mov bl, 0x0E
    call vga_print_string

.loop:
    hlt
    jmp .loop

keyboard_handler:
    pusha
    in al, 0x60
    test al, 0x80
    jnz .done
    call scancode_to_ascii
    test al, al
    jz .done

    ; Display character
    mov edi, 0xB8000
    add edi, [cursor_pos]
    mov ah, 0x0F
    stosw
    add dword [cursor_pos], 2

    ; Newline on Enter
    cmp al, 10
    jne .done
    push eax
    mov eax, [cursor_pos]
    xor edx, edx
    mov ecx, 160
    div ecx
    inc eax
    mul ecx
    mov [cursor_pos], eax
    pop eax

.done:
    mov al, 0x20
    out 0x20, al
    popa
    iretd

scancode_to_ascii:
    cmp al, 0x1C
    jne .not_enter
    mov al, 10
    ret
.not_enter:
    cmp al, 58
    jge .no_key
    movzx ebx, al
    lea ecx, [scancode_table]
    mov al, [ecx + ebx]
    ret
.no_key:
    xor al, al
    ret

setup_idt:
    mov eax, keyboard_handler
    mov word [idt + 33*8], ax
    shr eax, 16
    mov word [idt + 33*8 + 6], ax
    mov word [idt + 33*8 + 2], 0x08
    mov byte [idt + 33*8 + 5], 0x8E
    lidt [idt_ptr]
    ret

setup_pic:
    mov al, 0x11
    out 0x20, al
    out 0xA0, al
    mov al, 0x20
    out 0x21, al
    mov al, 0x28
    out 0xA1, al
    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xA1, al
    mov al, 0x01
    out 0x21, al
    out 0xA1, al
    mov al, 0xFD
    out 0x21, al
    mov al, 0xFF
    out 0xA1, al
    ret

vga_clear_screen:
    pusha
    mov edi, 0xB8000
    mov ecx, 80 * 25
    mov ax, 0x0F20
    rep stosw
    mov dword [cursor_pos], 0
    popa
    ret

vga_print_string:
    pusha
    mov edi, 0xB8000
    add edi, [cursor_pos]
    mov bh, bl
.loop:
    lodsb
    test al, al
    jz .done
    cmp al, 10
    je .newline
    mov ah, bh
    stosw
    add dword [cursor_pos], 2
    jmp .loop
.newline:
    push eax
    mov eax, [cursor_pos]
    xor edx, edx
    mov ecx, 160
    div ecx
    inc eax
    mul ecx
    mov [cursor_pos], eax
    mov edi, 0xB8000
    add edi, eax
    pop eax
    jmp .loop
.done:
    popa
    ret

section .data
cursor_pos: dd 0

banner: db '=== TernaryBit OS - Minimal Shell ===', 10
        db 'Keyboard echo enabled', 10, 10, 0

prompt: db 'TBOS> ', 0

idt:
    times 256*8 db 0

idt_ptr:
    dw 256*8 - 1
    dd idt

scancode_table:
    db 0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0
    db 'q','w','e','r','t','y','u','i','o','p','[',']',0,0
    db 'a','s','d','f','g','h','j','k','l',';',39,96,0,92
    db 'z','x','c','v','b','n','m',',','.','/',0,'*',0,' '

section .bss
