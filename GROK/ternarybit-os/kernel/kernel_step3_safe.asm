; TernaryBit OS - Kernel Step 3: Shell (Ultra-Safe Version)
; Minimal interrupt handler, all complex logic in main loop
; 🕉️ Universal: From 2-bit to Supercomputer 🕉️

[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; Initialize BSS variables
    mov dword [cmd_buffer_pos], 0
    mov byte [command_ready], 0
    mov byte [last_char], 0

    call vga_clear_screen

    ; Display banner
    mov esi, banner
    mov bl, 0x0B
    call vga_print_string

    ; Setup interrupts
    call setup_idt
    call setup_pic
    sti

    ; Show prompt
    call show_prompt

    ; Main loop
.loop:
    ; Check if we have a character to process
    mov al, [last_char]
    test al, al
    jz .check_command

    ; Process character in main loop (not interrupt)
    mov byte [last_char], 0

    ; Handle Enter
    cmp al, 10
    je .handle_enter

    ; Handle Backspace
    cmp al, 8
    je .handle_backspace

    ; Regular character - store and display
    cmp dword [cmd_buffer_pos], 79
    jge .check_command

    mov ebx, [cmd_buffer_pos]
    lea ecx, [cmd_buffer]
    mov byte [ecx + ebx], al
    inc dword [cmd_buffer_pos]

    ; Display
    mov edi, 0xB8000
    add edi, [cursor_pos]
    mov ah, 0x0F
    stosw
    add dword [cursor_pos], 2
    jmp .check_command

.handle_enter:
    ; Newline
    mov esi, newline_str
    mov bl, 0x0F
    call vga_print_string
    mov byte [command_ready], 1
    jmp .check_command

.handle_backspace:
    cmp dword [cmd_buffer_pos], 0
    je .check_command
    dec dword [cmd_buffer_pos]
    sub dword [cursor_pos], 2
    mov edi, 0xB8000
    add edi, [cursor_pos]
    mov ax, 0x0F20
    stosw
    jmp .check_command

.check_command:
    ; Process command if ready
    cmp byte [command_ready], 0
    je .wait

    mov byte [command_ready], 0
    call process_command

.wait:
    hlt
    jmp .loop

;==========================================
; Shell Functions
;==========================================

show_prompt:
    mov esi, prompt_str
    mov bl, 0x0E
    call vga_print_string
    mov dword [cmd_buffer_pos], 0
    ret

process_command:
    ; Null-terminate
    mov eax, [cmd_buffer_pos]
    lea ebx, [cmd_buffer]
    mov byte [ebx + eax], 0

    ; Check "help"
    lea esi, [cmd_buffer]
    lea edi, [cmd_help]
    call strcmp
    test eax, eax
    jz .do_help

    ; Check "clear"
    lea esi, [cmd_buffer]
    lea edi, [cmd_clear]
    call strcmp
    test eax, eax
    jz .do_clear

    ; Check "about"
    lea esi, [cmd_buffer]
    lea edi, [cmd_about]
    call strcmp
    test eax, eax
    jz .do_about

    ; Unknown
    mov esi, unknown_msg
    mov bl, 0x0C
    call vga_print_string
    jmp .done

.do_help:
    mov esi, help_msg
    mov bl, 0x0F
    call vga_print_string
    jmp .done

.do_clear:
    call vga_clear_screen
    jmp .done

.do_about:
    mov esi, about_msg
    mov bl, 0x0B
    call vga_print_string
    jmp .done

.done:
    call show_prompt
    ret

strcmp:
    push esi
    push edi
.loop:
    lodsb
    mov dl, byte [edi]
    inc edi
    cmp al, dl
    jne .not_equal
    test al, al
    jnz .loop
    xor eax, eax
    pop edi
    pop esi
    ret
.not_equal:
    mov eax, 1
    pop edi
    pop esi
    ret

;==========================================
; ULTRA-MINIMAL Keyboard Handler
;==========================================

keyboard_handler:
    pusha

    in al, 0x60

    ; Ignore key release
    test al, 0x80
    jnz .done

    ; Convert to ASCII
    call scancode_to_ascii
    test al, al
    jz .done

    ; Just store character - let main loop handle it
    mov byte [last_char], al

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
    cmp al, 0x0E
    jne .not_backspace
    mov al, 8
    ret
.not_backspace:
    cmp al, 58
    jge .no_key

    movzx ebx, al
    lea ecx, [scancode_table]
    mov al, [ecx + ebx]
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

banner:         db '=== TernaryBit OS - Universal Shell ===', 10
                db 'Type "help" for commands', 10, 10, 0

prompt_str:     db 'TBOS> ', 0
newline_str:    db 10, 0

cmd_help:       db 'help', 0
cmd_clear:      db 'clear', 0
cmd_about:      db 'about', 0

help_msg:       db 'Available commands:', 10
                db '  help  - Show this help', 10
                db '  clear - Clear screen', 10
                db '  about - System info', 10, 0

about_msg:      db 'TernaryBit OS v1.0', 10
                db 'Universal Bootloader System', 10
                db 'From 2-bit to Supercomputer', 10, 0

unknown_msg:    db 'Unknown command. Type "help"', 10, 0

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
cmd_buffer:     resb 80
cmd_buffer_pos: resd 1
command_ready:  resb 1
last_char:      resb 1
