; debug.inc - Debugging utilities for TBOS

; Debug levels
%define DEBUG_OFF     0
%define DEBUG_NORMAL  1
%define DEBUG_VERBOSE 2

; Debug state
debug_level db DEBUG_NORMAL

; Debug messages
debug_header  db 0x0D, 0x0A, 'TBOS Debug: ', 0
debug_off    db 'Debug OFF', 0x0D, 0x0A, 0
debug_normal db 'Debug NORMAL', 0x0D, 0x0A, 0
debug_verbose db 'Debug VERBOSE', 0x0D, 0x0A, 0

; Debug buffer
debug_buffer times 16 db 0

; Check for Ctrl+Alt+D debug key combo
; Modifies: AX, BX, CX, DX
check_debug_combo:
    pushf
    pusha
    
    ; Check keyboard status for Ctrl+Alt
    mov ah, 0x02
    int 0x16        ; Get keyboard status
    and al, 0x0C    ; Check Ctrl+Alt bits (00001100b)
    cmp al, 0x0C    ; Both Ctrl+Alt pressed?
    jne .done
    
    ; Check for 'D' key
    mov ah, 0x01
    int 0x16
    jz .done        ; No key available
    
    ; Check if it's 'D' or 'd'
    cmp al, 'D'
    je .debug_key
    cmp al, 'd'
    jne .done
    
.debug_key:
    ; Consume the key
    mov ah, 0x00
    int 0x16
    
    ; Toggle debug level
    mov al, [debug_level]
    inc al
    cmp al, DEBUG_VERBOSE + 1
    jbe .set_level
    mov al, DEBUG_OFF
    
.set_level:
    mov [debug_level], al
    
    ; Print debug level
    call debug_print_level
    
.done:
    popa
    popf
    ret

; Print current debug level to serial
; Preserves all registers
debug_print_level:
    pusha
    mov si, debug_header
    call serial_print
    
    mov al, [debug_level]
    cmp al, DEBUG_OFF
    je .print_off
    cmp al, DEBUG_NORMAL
    je .print_normal
    
    ; DEBUG_VERBOSE
    mov si, debug_verbose
    jmp .print_done
    
.print_normal:
    mov si, debug_normal
    jmp .print_done
    
.print_off:
    mov si, debug_off
    
.print_done:
    call serial_print
    popa
    ret

; Print debug message if level is sufficient
; AL = required level, SI = message
debug_print:
    pusha
    cmp al, [debug_level]
    ja .done
    call serial_print
.done:
    popa
    ret

; Dump registers to serial port
; Preserves all registers
debug_dump_regs:
    pusha
    
    ; Save registers to stack
    push eax
    push ebx
    push ecx
    push edx
    push esi
    push edi
    push ebp
    push esp
    
    ; Print header
    mov si, .header
    call serial_print
    
    ; Point to saved registers
    mov ebp, esp
    mov ecx, 8  ; Number of registers
    mov edi, .reg_names
    
.reg_loop:
    ; Print register name
    mov si, di
    call serial_print
    
    ; Print ' = 0x'
    mov al, ' '
    call serial_write_char
    mov al, '='
    call serial_write_char
    mov al, ' '
    call serial_write_char
    mov al, '0'
    call serial_write_char
    mov al, 'x'
    call serial_write_char
    
    ; Get register value and print it
    mov eax, [ebp + ecx*4 - 4]
    call serial_print_hex_dword
    
    ; Newline
    mov al, 0x0D
    call serial_write_char
    mov al, 0x0A
    call serial_write_char
    
    ; Next register
    add edi, 4
    loop .reg_loop
    
    ; Clean up
    add esp, 32 ; Remove saved registers from stack
    popa
    ret

.header:    db 0x0D, 0x0A, 'Register Dump:', 0x0D, 0x0A, 0
.reg_names: db 'EAX', 0, 'EBX', 0, 'ECX', 0, 'EDX', 0,
            db 'ESI', 0, 'EDI', 0, 'EBP', 0, 'ESP', 0
