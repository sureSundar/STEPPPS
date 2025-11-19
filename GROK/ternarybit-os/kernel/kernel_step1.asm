; TernaryBit OS - Kernel Step 1: VGA Text Functions
; Expands minimal kernel with proper VGA text mode support
; 🕉️ Universal: Modular and testable 🕉️

[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; Initialize VGA
    call vga_clear_screen

    ; Display welcome banner
    mov esi, banner_line1
    mov bl, 0x0F        ; White on black
    call vga_print_string

    mov esi, banner_line2
    mov bl, 0x0B        ; Cyan
    call vga_print_string

    mov esi, banner_line3
    mov bl, 0x0F
    call vga_print_string

    call vga_newline

    ; Test scrolling with multiple lines
    mov esi, test_msg1
    mov bl, 0x0A        ; Green
    call vga_print_string

    mov esi, test_msg2
    mov bl, 0x0E        ; Yellow
    call vga_print_string

    mov esi, success_msg
    mov bl, 0x0F
    call vga_print_string

    ; Halt cleanly
    cli
    hlt
    jmp $

;==========================================
; VGA Functions
;==========================================

; Clear screen - fill with spaces
vga_clear_screen:
    pusha
    mov edi, 0xB8000
    mov ecx, 80 * 25        ; 80 columns x 25 rows
    mov ax, 0x0F20          ; White on black space
    rep stosw

    ; Reset cursor position
    mov dword [cursor_pos], 0
    popa
    ret

; Print string pointed to by ESI with color in BL
vga_print_string:
    pusha
    mov edi, 0xB8000
    add edi, [cursor_pos]
    mov bh, bl              ; Save color

.loop:
    lodsb                   ; Load character
    test al, al             ; Check for null
    jz .done

    ; Handle newline
    cmp al, 10
    je .newline

    ; Store character + color
    mov ah, bh
    stosw
    inc dword [cursor_pos]
    inc dword [cursor_pos]
    jmp .loop

.newline:
    ; Move to start of next line
    push eax
    mov eax, [cursor_pos]
    xor edx, edx
    mov ecx, 160            ; 80 chars * 2 bytes
    div ecx                 ; EAX = line number
    inc eax                 ; Next line
    mul ecx                 ; EAX = offset of next line
    mov [cursor_pos], eax
    mov edi, 0xB8000
    add edi, eax
    pop eax
    jmp .loop

.done:
    popa
    ret

; Move to new line
vga_newline:
    push eax
    push ecx
    push edx

    mov eax, [cursor_pos]
    xor edx, edx
    mov ecx, 160            ; Bytes per line
    div ecx
    inc eax                 ; Next line
    mul ecx
    mov [cursor_pos], eax

    pop edx
    pop ecx
    pop eax
    ret

section .data
cursor_pos:     dd 0

banner_line1:   db '===================================', 10, 0
banner_line2:   db ' TernaryBit OS - Kernel Step 1', 10, 0
banner_line3:   db '===================================', 10, 0

test_msg1:      db '[VGA] Text mode initialized', 10, 0
test_msg2:      db '[VGA] Color support working', 10, 0
success_msg:    db '[OK] Kernel Step 1 complete!', 10, 0

section .bss
