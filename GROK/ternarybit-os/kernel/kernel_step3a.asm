; TernaryBit OS - Kernel Step 3A: Simple Character Echo
; Simplified version to test keyboard without command processing
; 🕉️ Universal: Debug step - just echo characters 🕉️

[BITS 32]
[GLOBAL _start]

section .text
_start:
    call vga_clear_screen

    ; Display banner
    mov esi, banner
    mov bl, 0x0B
    call vga_print_string

    ; Setup interrupts
    call setup_idt
    call setup_pic
    sti

    ; Main loop
.loop:
    hlt
    jmp .loop

;==========================================
; Keyboard Handler - SIMPLIFIED
;==========================================

keyboard_handler:
    pusha

    in al, 0x60             ; Read scan code

    ; Check for key release (bit 7 set)
    test al, 0x80
    jnz .done

    ; Convert to ASCII
    call scancode_to_ascii
    test al, al
    jz .done

    ; Just display the character (no buffering, no commands)
    mov edi, 0xB8000
    add edi, [cursor_pos]
    mov ah, 0x0F
    stosw
    add dword [cursor_pos], 2

    ; Simple newline handling
    cmp al, 10
    jne .done

    ; Move to next line
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
    out 0x20, al            ; EOI
    popa
    iretd

; Simple scan code to ASCII
scancode_to_ascii:
    cmp al, 0x1C            ; Enter
    jne .not_enter
    mov al, 10
    ret
.not_enter:
    cmp al, 58
    jge .no_key

    movzx ebx, al
    mov al, [scancode_table + ebx]
    ret
.no_key:
    xor al, al
    ret

;==========================================
; Interrupt Setup
;==========================================

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

;==========================================
; VGA Functions
;==========================================

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
cursor_pos:     dd 0

banner:         db '=== TernaryBit OS - Step 3A: Echo Test ===', 10
                db 'Type to see characters echoed', 10, 10, 0

; IDT must come before scancode table
idt:
    times 256*8 db 0

idt_ptr:
    dw 256*8 - 1
    dd idt

; Simplified US keyboard scan code table - AFTER IDT
scancode_table:
    db 0,0,'1','2','3','4','5','6','7','8','9','0','-','=',0,0
    db 'q','w','e','r','t','y','u','i','o','p','[',']',0,0
    db 'a','s','d','f','g','h','j','k','l',';',39,96,0,92
    db 'z','x','c','v','b','n','m',',','.','/',0,'*',0,' '

section .bss
