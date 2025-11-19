; TernaryBit OS - Kernel Step 2: Keyboard Input
; Adds keyboard interrupt handling
; 🕉️ Universal: Modular and testable 🕉️

[BITS 32]
[GLOBAL _start]

section .text
_start:
    ; Initialize VGA
    call vga_clear_screen

    ; Display banner
    mov esi, banner
    mov bl, 0x0F
    call vga_print_string

    ; Setup interrupts
    call setup_idt
    call setup_pic

    ; Enable interrupts
    sti

    ; Display ready message
    mov esi, ready_msg
    mov bl, 0x0A
    call vga_print_string

    ; Infinite loop - wait for keyboard interrupts
.loop:
    hlt                     ; Wait for interrupt
    jmp .loop

;==========================================
; Interrupt Setup
;==========================================

setup_idt:
    ; Install keyboard handler at IDT entry 33 (IRQ1)
    mov eax, keyboard_handler
    mov word [idt + 33*8], ax       ; Lower offset
    shr eax, 16
    mov word [idt + 33*8 + 6], ax   ; Upper offset
    mov word [idt + 33*8 + 2], 0x08 ; Code segment
    mov byte [idt + 33*8 + 5], 0x8E ; Present, DPL=0, 32-bit interrupt gate

    ; Load IDT
    lidt [idt_ptr]
    ret

setup_pic:
    ; Remap PIC
    mov al, 0x11
    out 0x20, al            ; Init PIC1
    out 0xA0, al            ; Init PIC2

    mov al, 0x20
    out 0x21, al            ; PIC1 vector offset 32
    mov al, 0x28
    out 0xA1, al            ; PIC2 vector offset 40

    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xA1, al

    mov al, 0x01
    out 0x21, al
    out 0xA1, al

    ; Unmask keyboard IRQ (IRQ1)
    mov al, 0xFD            ; All masked except IRQ1
    out 0x21, al
    mov al, 0xFF
    out 0xA1, al
    ret

;==========================================
; Keyboard Handler
;==========================================

keyboard_handler:
    pusha

    ; Read scan code from keyboard
    in al, 0x60

    ; Simple test: display scan code as hex
    mov edi, 0xB8000
    add edi, [cursor_pos]

    ; Display '['
    mov ah, 0x0E
    mov al, '['
    stosw

    ; Get scan code again
    in al, 0x60
    call display_hex_byte

    ; Display ']'
    mov ah, 0x0E
    mov al, ']'
    stosw

    ; Space
    mov al, ' '
    stosw

    add dword [cursor_pos], 10

    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al

    popa
    iretd

; Display AL as hex
display_hex_byte:
    push ax
    shr al, 4
    call display_hex_nibble
    pop ax
    and al, 0x0F
    call display_hex_nibble
    ret

display_hex_nibble:
    cmp al, 10
    jb .digit
    add al, 'A' - 10
    jmp .show
.digit:
    add al, '0'
.show:
    mov ah, 0x0E
    stosw
    ret

;==========================================
; VGA Functions (from Step 1)
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
    inc dword [cursor_pos]
    inc dword [cursor_pos]
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

banner:         db '=== TernaryBit OS - Step 2: Keyboard ===', 10, 0
ready_msg:      db '[OK] Keyboard enabled. Press keys to test!', 10
                db 'Scan codes will appear as: [XX] ', 10, 0

; IDT (simplified - only entry 33 used)
idt:
    times 256*8 db 0

idt_ptr:
    dw 256*8 - 1            ; Limit
    dd idt                  ; Base

section .bss
