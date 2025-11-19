; ============================================================================
; TernaryBit OS - Calculator/Embedded Bootloader (Variant A)
; ============================================================================
; Purpose: Minimal bootloader for constrained devices
; Target: 4-bit to 16-bit processors with 1KB-64KB RAM
; Size: <2KB total
; Author: TernaryBit OS Team
; Date: 2025-10-23
;
; Designed for:
; - Classic calculators (HP-48, TI-89, etc.)
; - Embedded microcontrollers (8051, AVR, PIC)
; - Retro computers (Commodore 64, ZX Spectrum, etc.)
;
; Features:
; - Minimal STEPPPS initialization
; - Direct hardware access
; - No virtual memory
; - Text/segment display only
; ============================================================================

[BITS 16]
[ORG 0x1000]        ; Many embedded systems load code at different addresses

; ============================================================================
; CONFIGURATION
; ============================================================================

; Memory configuration
STACK_SIZE          equ 256         ; 256 bytes stack
HEAP_START          equ 0x2000      ; Heap starts after code
AVAILABLE_RAM       equ 0x4000      ; Assume 16KB available

; Display configuration
DISPLAY_ROWS        equ 8           ; For segment/LCD displays
DISPLAY_COLS        equ 16

; ============================================================================
; ENTRY POINT
; ============================================================================

start:
    ; Initialize minimal CPU state
    cli                             ; Disable interrupts if supported

    ; Setup stack (if supported)
    mov sp, 0x1000 + STACK_SIZE

    ; Clear any display
    call clear_display

    ; Show boot message
    call show_banner

    ; Initialize minimal STEPPPS
    call init_steppps_minimal

    ; Load and run kernel
    call run_kernel

    ; If kernel returns, halt
    cli
    jmp $

; ============================================================================
; MINIMAL STEPPPS INITIALIZATION
; ============================================================================

init_steppps_minimal:
    ; SPACE: Initialize memory regions
    call init_memory

    ; TIME: Setup basic timing (if timer available)
    call init_timer

    ; EVENT: Minimal interrupt setup (if supported)
    call init_interrupts

    ; PSYCHOLOGY: Not applicable for calculators
    ; PIXEL: Display already initialized
    ; PROMPT: Input handling
    call init_input

    ; SCRIPT: Not applicable for minimal systems

    ret

; --- SPACE: Memory Initialization ---
init_memory:
    ; Clear heap area
    mov ax, HEAP_START
    mov cx, (AVAILABLE_RAM - HEAP_START) / 2
    xor bx, bx

.clear_loop:
    mov [ax], bx
    add ax, 2
    loop .clear_loop

    ret

; --- TIME: Timer Initialization ---
init_timer:
    ; Platform-specific timer initialization
    ; For calculators, may use RTC or system timer

    ; Initialize tick counter
    mov word [tick_counter], 0

    ret

; --- EVENT: Interrupt Initialization ---
init_interrupts:
    ; Minimal interrupt setup
    ; For most calculators, interrupts are limited or pre-configured

    ret

; --- PROMPT: Input Initialization ---
init_input:
    ; Initialize keyboard/keypad buffer
    mov byte [key_buffer_head], 0
    mov byte [key_buffer_tail], 0

    ret

; ============================================================================
; DISPLAY FUNCTIONS
; ============================================================================

clear_display:
    ; Platform-specific display clear
    ; For segment displays: turn off all segments
    ; For LCD: clear all pixels

    ; Generic: fill display buffer with spaces
    mov ax, display_buffer
    mov cx, DISPLAY_ROWS * DISPLAY_COLS
    mov bl, ' '

.clear_loop:
    mov [ax], bl
    inc ax
    loop .clear_loop

    ret

show_banner:
    ; Show minimal boot banner
    mov si, banner_text
    mov di, display_buffer
    mov cx, banner_len

.copy_loop:
    lodsb
    stosb
    loop .copy_loop

    ; Update physical display (platform-specific)
    call update_display

    ret

update_display:
    ; Platform-specific display update
    ; This would interface with the actual hardware display

    ; For emulation/testing, we can use BIOS if available
    push ds
    push si

    mov si, display_buffer
    mov cx, DISPLAY_ROWS

.row_loop:
    push cx
    mov cx, DISPLAY_COLS

.col_loop:
    lodsb
    ; Output character (platform-specific)
    ; If BIOS available:
    ; mov ah, 0x0E
    ; int 0x10
    loop .col_loop

    pop cx
    loop .row_loop

    pop si
    pop ds
    ret

; ============================================================================
; KERNEL LOADER
; ============================================================================

run_kernel:
    ; For minimal systems, the "kernel" is often just a function
    ; in ROM or loaded into RAM

    ; Check if kernel is in ROM
    mov ax, [kernel_location]
    cmp ax, 0
    je .no_kernel

    ; Jump to kernel
    call ax

    ret

.no_kernel:
    ; Run built-in minimal shell
    call minimal_shell
    ret

; ============================================================================
; MINIMAL SHELL (for calculators)
; ============================================================================

minimal_shell:
    mov si, shell_prompt
    call display_string

.main_loop:
    ; Read input
    call read_key
    cmp al, 0
    je .main_loop

    ; Echo character
    call display_char

    ; Check for special keys
    cmp al, 13          ; Enter
    je .execute_command

    cmp al, 27          ; Escape
    je .clear_input

    ; Add to input buffer
    call add_to_buffer

    jmp .main_loop

.execute_command:
    call process_command
    call display_string
    mov si, shell_prompt
    jmp .main_loop

.clear_input:
    call clear_input_buffer
    jmp .main_loop

; ============================================================================
; INPUT FUNCTIONS
; ============================================================================

read_key:
    ; Platform-specific keyboard read
    ; For calculators: scan keypad matrix
    ; For computers: use BIOS or direct port read

    ; Check if key available in buffer
    mov al, [key_buffer_head]
    cmp al, [key_buffer_tail]
    je .no_key

    ; Read from buffer
    mov bx, key_buffer
    xor ah, ah
    mov al, [key_buffer_tail]
    add bx, ax
    mov al, [bx]

    ; Update tail pointer
    inc byte [key_buffer_tail]
    and byte [key_buffer_tail], 0x0F    ; Wrap at 16 bytes

    ret

.no_key:
    xor al, al
    ret

add_to_buffer:
    ; Add character to input buffer
    mov bx, input_buffer
    mov cl, [input_buffer_len]
    xor ch, ch
    add bx, cx
    mov [bx], al
    inc byte [input_buffer_len]
    ret

clear_input_buffer:
    mov byte [input_buffer_len], 0
    ret

; ============================================================================
; COMMAND PROCESSING
; ============================================================================

process_command:
    ; Parse and execute command from input buffer

    ; Check for built-in commands
    mov si, input_buffer
    mov di, cmd_help
    call strcmp
    cmp al, 0
    je .cmd_help

    mov di, cmd_calc
    call strcmp
    cmp al, 0
    je .cmd_calc

    mov di, cmd_mem
    call strcmp
    cmp al, 0
    je .cmd_mem

    ; Unknown command
    mov si, msg_unknown
    ret

.cmd_help:
    mov si, msg_help
    ret

.cmd_calc:
    ; Simple calculator function
    call calculator
    ret

.cmd_mem:
    ; Show memory info
    call show_memory
    ret

; ============================================================================
; CALCULATOR FUNCTION
; ============================================================================

calculator:
    ; Simple arithmetic calculator
    ; Parse expression from input buffer

    ; For now, just return placeholder
    mov si, msg_calc_result
    ret

; ============================================================================
; MEMORY INFO
; ============================================================================

show_memory:
    ; Display memory information
    mov si, msg_memory
    call display_string

    ; Show available RAM
    mov ax, AVAILABLE_RAM
    call display_hex16

    mov si, msg_bytes
    call display_string

    ret

; ============================================================================
; STRING FUNCTIONS
; ============================================================================

display_string:
    ; Display null-terminated string pointed to by SI
    push ax

.loop:
    lodsb
    cmp al, 0
    je .done
    call display_char
    jmp .loop

.done:
    pop ax
    ret

display_char:
    ; Display single character in AL
    ; Platform-specific implementation
    push ax
    push bx

    ; Add to display buffer
    mov bx, [display_cursor]
    cmp bx, DISPLAY_ROWS * DISPLAY_COLS
    jge .scroll

    mov di, display_buffer
    add di, bx
    stosb
    inc word [display_cursor]

.scroll:
    ; TODO: Implement scrolling for full buffer

    pop bx
    pop ax
    ret

strcmp:
    ; Compare strings: SI = string1, DI = string2
    ; Returns: AL = 0 if equal, non-zero otherwise
    push si
    push di

.loop:
    lodsb
    mov ah, [di]
    inc di
    cmp al, ah
    jne .not_equal
    cmp al, 0
    jne .loop

    ; Equal
    xor al, al
    jmp .done

.not_equal:
    mov al, 1

.done:
    pop di
    pop si
    ret

display_hex16:
    ; Display 16-bit value in AX as hex
    push ax
    push bx
    push cx

    mov cx, 4

.loop:
    rol ax, 4
    mov bx, ax
    and bx, 0x0F
    add bl, '0'
    cmp bl, '9'
    jle .print
    add bl, 7

.print:
    push ax
    mov al, bl
    call display_char
    pop ax
    dec cx
    jnz .loop

    pop cx
    pop bx
    pop ax
    ret

; ============================================================================
; DATA SECTION
; ============================================================================

; Banner
banner_text         db 'TBOS Mini v1.0', 0
banner_len          equ $ - banner_text

; Shell messages
shell_prompt        db '> ', 0
msg_unknown         db 'Unknown command', 13, 10, 0
msg_help            db 'Commands: help, calc, mem', 13, 10, 0
msg_calc_result     db 'Result: ', 0
msg_memory          db 'RAM: ', 0
msg_bytes           db ' bytes', 13, 10, 0

; Built-in commands
cmd_help            db 'help', 0
cmd_calc            db 'calc', 0
cmd_mem             db 'mem', 0

; System state
tick_counter        dw 0
kernel_location     dw 0

; Display buffer (for text displays)
display_buffer      times DISPLAY_ROWS * DISPLAY_COLS db ' '
display_cursor      dw 0

; Input buffers
key_buffer          times 16 db 0
key_buffer_head     db 0
key_buffer_tail     db 0

input_buffer        times 64 db 0
input_buffer_len    db 0

; Pad to 2KB
times 2048-($-$$) db 0
