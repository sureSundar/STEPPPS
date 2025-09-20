; TernaryBit OS - Simple Stage 2 for Kernel Boot
; COLLABORATION LOG:
; CC-Claude Code: could you suggest a concise commit summary when ready?
; - Windsurf: Fixed GDT physical address , A20 gate
; - Claude: Protected mode transition , sector count fixes
; - Codex: GDT structure cleanup , added PM debug marker
; - Codex: updated 2025: Runtime GDT base via CS, serial PM banner
; - STATUS: !S2LKPA now reaches GDT load; protected-mode banner serializes

[BITS 16]
[ORG 0x8000]

; Missing constants needed for stage2
%define BOOT_KERNEL_SECTOR_COUNT 8
%define BOOT_KERNEL_LBA_START 6

; Constants
CODE_SEG equ 0x08
DATA_SEG equ 0x10
KERNEL_ENTRY equ 0x0010000

; Debug levels - The Threefold Path
DEBUG_OFF     equ 0   ; Mouna (Silence)
DEBUG_NORMAL  equ 1   ; Jnana (Knowledge)
DEBUG_VERBOSE equ 2   ; Vipassana (Insight)

; Key codes
KEY_CTRL  equ 0x1D
KEY_ALT   equ 0x38
KEY_D     equ 0x20

; === STAGE2 ENTRY POINT - MUST BE FIRST ===
jmp stage2_start

; Debug state
debug_level db DEBUG_NORMAL
debug_keys_pressed db 0  ; Bit 0: Ctrl, Bit 1: Alt, Bit 2: D

; Debug messages
debug_msg_off    db 'Debug: Output disabled', 0x0D, 0x0A, 0
debug_msg_normal db 'Debug: Normal output', 0x0D, 0x0A, 0
debug_msg_verbose db 'Debug: Verbose output', 0x0D, 0x0A, 0

; === DEBUG KEY HANDLER ===
check_debug_combo:
    pusha
    
    ; Read key without removing from buffer
    mov ah, 0x01
    int 0x16
    jz .no_key
    
    ; Check for Ctrl+Alt+D
    mov ah, 0x11
    int 0x16
    jz .no_key
    
    ; Check if Ctrl+Alt+D is pressed
    test al, KEY_CTRL | KEY_ALT
    jnz .check_d_key
    
.check_d_key:
    cmp al, KEY_D
    jne .no_key
    
    ; Toggle debug level
    mov al, [debug_level]
    inc al
    cmp al, DEBUG_VERBOSE + 1
    jbe .set_level
    xor al, al  ; Wrap around to OFF
    
.set_level:
    mov [debug_level], al
    
    ; Print debug level message
    call print_debug_status
    
    ; Clear the key from buffer
    mov ah, 0x00
    int 0x16
    
.no_key:
    popa
    ret

; Print current debug status
print_debug_status:
    pusha
    mov si, debug_msg_normal
    cmp byte [debug_level], DEBUG_OFF
    je .print
    cmp byte [debug_level], DEBUG_VERBOSE
    jb .print
    mov si, debug_msg_verbose
    jmp .print
.print_off:
    mov si, debug_msg_off
.print:
    call print_string
    popa
    ret

; Debug log function
; AL = log level, SI = message
debug_log:
    pusha
    cmp al, [debug_level]
    ja .done
    call print_string
.done:
    popa
    ret

; === MAIN BOOTLOADER CODE ===
stage2_start:
    ; Debug: Stage2 started
    mov al, '!'  ; Entry marker
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov al, 'S'  ; Stage2
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov al, '2'  ; Stage 2
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; === KERNEL LOADING ===
    ; Load kernel at 0x1000:0000 (linear address 0x10000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    ; Debug: Loading kernel
    mov al, 'L'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Get boot drive from boot sector (preserved at 0x0500)
    mov dl, [0x0500]

    ; Load kernel sectors (configured at build time)
    mov ah, 0x02        ; BIOS read sectors
    mov al, BOOT_KERNEL_SECTOR_COUNT
    mov ch, 0           ; Cylinder 0
    mov cl, BOOT_KERNEL_LBA_START
    mov dh, 0           ; Head 0
    int 0x13
    jc kernel_error

    ; Debug: Kernel loaded
    mov al, 'K'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; === PROTECTED MODE TRANSITION ===
    cli                     ; Disable interrupts

    mov al, 'P'            ; Protected mode start
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Enable A20 gate
    mov si, msg_a20
    call print_string
    call enable_a20
    jc a20_error
    
    mov si, msg_ok
    call print_string

    ; Load GDT
    cli
    
    ; Debug: Print GDT loading message
    mov si, msg_gdt
    call print_string
    
    ; Calculate GDT physical address using current code segment base
    mov si, msg_calc_gdt
    call print_string

    xor edx, edx
    mov dx, cs
    shl edx, 4              ; edx = current segment base
    mov [stage2_base], edx

    ; Compute and show stage2 physical base for debugging
    mov eax, edx
    add eax, stage2_start
    mov [stage2_phys], eax

    mov si, msg_stage2_base
    call print_string
    mov eax, [stage2_phys]
    call print_hex_dword
    call newline

    ; Report CS base (segment << 4)
    mov si, msg_cs_base
    call print_string
    mov eax, [stage2_base]
    call print_hex_dword
    call newline

    ; Compute physical GDT base and update descriptor
    mov eax, gdt_start
    sub eax, stage2_start
    add eax, [stage2_phys]
    mov [gdt_descriptor + 2], eax

    mov si, msg_gdt_final
    call print_string
    mov eax, [gdt_descriptor + 2]
    call print_hex_dword
    call newline

    ; Show descriptor pointer (physical)
    mov si, msg_gdt_descriptor
    call print_string
    mov eax, gdt_descriptor
    sub eax, stage2_start
    add eax, [stage2_phys]
    call print_hex_dword
    call newline

    ; Load GDT with debug output
    mov si, msg_loading_gdt
    call print_string

    ; Actual GDT load
    lgdt [gdt_descriptor]

    ; Verify GDT was loaded
    mov si, msg_gdt_loaded
    call print_string

    ; Read back GDTR contents safely
    sgdt [gdtr_buffer]

    ; Display GDTR base for confirmation
    mov si, msg_gdtr_verification
    call print_string
    mov eax, dword [gdtr_buffer + 2]
    call print_hex_dword
    call newline

    ; Print success
    mov si, msg_ok
    call print_string
    
    mov al, 'G'            ; GDT loaded
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Prepare for protected mode transition
    mov si, msg_pm_transition
    call print_string
    
    ; Enable protected mode (set PE bit in CR0)
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    ; Far jump flushes pipeline and loads CS selector
    jmp CODE_SEG:init_pmode

[BITS 32]
; Protected mode entry point
init_pmode:
    ; Set up segment registers
    mov ax, DATA_SEG       ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up stack
    mov ss, ax
    mov esp, 0x90000       ; Set stack pointer

    ; Initialize serial port so -nographic runs still show PM banner
    call serial_init
    mov esi, pm_msg
    call serial_print
    mov al, 0x0D
    call serial_write_char
    mov al, 0x0A
    call serial_write_char
    
    ; Clear screen
    mov edi, 0xB8000
    mov ecx, 80*25         ; 80x25 text mode
    mov eax, 0x0F200F20    ; Black on white space
    rep stosd
    
    ; Print success message
    mov esi, pm_msg
    mov edi, 0xB8000
    mov ah, 0x0F           ; White on black
    cld
.print_loop:
    lodsb
    test al, al
    jz .print_done
    stosw
    jmp .print_loop
.print_done:
    
    ; Print GDT info
    mov esi, gdt_info_msg
    mov edi, 0xB8000 + 160 ; Second line
    call pm_print_string
    
    ; Print CS value
    mov edi, 0xB8000 + 320 ; Third line
    mov esi, cs_msg
    call pm_print_string
    
    ; Announce TBDS readiness on serial when debug enabled
    mov al, [debug_level]
    cmp al, DEBUG_OFF
    je .skip_tbds_serial

    mov esi, tbds_ready_msg
    call serial_print
    call serial_crlf

    mov esi, tbds_ptr_msg
    call serial_print
    mov eax, tbds_data
    call serial_print_hex_dword
    call serial_crlf

    mov esi, tbds_len_msg
    call serial_print
    mov eax, tbds_length
    call serial_print_hex_dword
    call serial_crlf

.skip_tbds_serial:

    ; Deliver TBDS pointer (EAX) and length (EBX) to kernel
    mov eax, tbds_data
    mov ebx, tbds_length
    
    ; Transfer control to 32-bit kernel entry
    jmp KERNEL_ENTRY

serial_init:
    push eax
    push edx
    mov dx, 0x3F9          ; COM1 interrupt enable
    xor eax, eax
    out dx, al             ; Disable interrupts
    mov dx, 0x3FB          ; Line control register
    mov al, 0x80           ; Enable DLAB
    out dx, al
    mov dx, 0x3F8          ; Divisor low byte
    mov al, 0x03           ; 115200 / 3 = 38400 baud
    out dx, al
    mov dx, 0x3F9          ; Divisor high byte
    xor eax, eax
    out dx, al
    mov dx, 0x3FB
    mov al, 0x03           ; 8 bits, no parity, one stop
    out dx, al
    mov dx, 0x3FA          ; FIFO control
    mov al, 0xC7           ; Enable FIFO, clear queues
    out dx, al
    mov dx, 0x3FC          ; Modem control
    mov al, 0x0B           ; IRQs disabled, RTS/DSR set
    out dx, al
    pop edx
    pop eax
    ret
    
serial_write_char:
    push edx
    push eax
.serial_wait_tx:
    mov dx, 0x3FD          ; Line status register
    in al, dx
    test al, 0x20          ; THR empty?
    jz .serial_wait_tx
    mov dx, 0x3F8          ; Transmit holding register
    pop eax                ; Restore character into AL
    out dx, al
    pop edx
    ret

serial_print:
    push eax
    push esi
.serial_loop:
    lodsb
    test al, al
    jz .serial_done
    call serial_write_char
    jmp .serial_loop
.serial_done:
    pop esi
    pop eax
    ret

serial_print_hex_dword:
    push eax
    push edx
    push ecx
    mov edx, eax
    mov ecx, 8
.serial_hex_loop:
    rol edx, 4
    mov al, dl
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jbe .serial_digit
    add al, 7
.serial_digit:
    call serial_write_char
    loop .serial_hex_loop
    pop ecx
    pop edx
    pop eax
    ret

; Dump memory in hex format
; EAX = memory address, CX = byte count
dump_memory:
    pusha
    mov esi, eax
    mov ebx, ecx
    
.dump_loop:
    ; Print address
    mov eax, esi
    call print_hex_dword
    mov al, ':'
    call serial_write_char
    mov al, ' '
    call serial_write_char
    
    ; Print hex bytes
    push ecx
    mov ecx, 16
    cmp ebx, ecx
    jbe .last_line
    mov ecx, 16
    
.print_hex:
    lodsb
    call print_hex_byte
    mov al, ' '
    call serial_write_char
    loop .print_hex
    
    ; Print ASCII representation
    sub esi, 16
    mov ecx, 16
    mov al, '|'
    call serial_write_char
    
.print_ascii:
    lodsb
    cmp al, 32
    jb .non_printable
    cmp al, 126
    ja .non_printable
    jmp .print_char
    
.non_printable:
    mov al, '.'
    
.print_char:
    call serial_write_char
    loop .print_ascii
    
    mov al, '|'
    call serial_write_char
    call serial_crlf
    
    pop ecx
    sub ebx, 16
    jnz .dump_loop
    jmp .done
    
.last_line:
    mov ecx, ebx
    jmp .print_hex
    
.done:
    popa
    ret

; Dump all general purpose registers
dump_registers:
    pusha
    pushf
    
    ; Save ESP before pushing
    mov ebp, esp
    
    ; Get EIP
    push .return_address
    mov eax, [esp]
    
    ; Print register values
    mov esi, reg_names
    mov ecx, 8  ; Number of registers to print
    lea edi, [esp + 8]  ; Skip return address and flags
    
.reg_loop:
    push ecx
    
    ; Print register name
    mov esi, reg_names
    mov ecx, 8
    sub ecx, [esp]
    shl ecx, 3  ; Each name is max 8 bytes
    add esi, ecx
    
    ; Find end of register name
    push esi
    mov edi, esi
    xor al, al
    mov ecx, -1
    repne scasb
    not ecx
    dec ecx
    
    ; Print register name
    mov ah, 0x0E
    mov ebx, 0x0007
    
.print_name:
    lodsb
    int 0x10
    loop .print_name
    
    ; Print register value
    pop esi
    pop ecx
    push ecx
    
    mov eax, [edi - 4]  ; Get register value from stack
    call print_hex_dword
    call serial_crlf
    
    add edi, 4
    pop ecx
    loop .reg_loop
    
    ; Print flags
    mov esi, msg_flags
    call print_string
    pushfd
    pop eax
    call print_hex_dword
    call serial_crlf
    
    popf
    popa
    ret

.return_address:
    add esp, 4  ; Clean up EIP
    popf
    popa
    ret

; Print newline (CRLF)
serial_crlf:
    push eax
    mov al, 0x0D
    call serial_write_char
    mov al, 0x0A
    call serial_write_char
    pop eax
    ret

; Print AL as hex
print_hex_byte:
    push eax
    push ecx
    
    mov cl, al
    shr al, 4
    call .nibble_to_hex
    call serial_write_char
    
    mov al, cl
    and al, 0x0F
    call .nibble_to_hex
    call serial_write_char
    
    pop ecx
    pop eax
    ret
    
.nibble_to_hex:
    cmp al, 10
    jb .is_digit
    add al, 'A' - '0' - 10
.is_digit:
    add al, '0'
    ret

; Protected mode print string function
; ESI = string, EDI = video memory offset
pm_print_string:
    pusha
    mov ah, 0x0F           ; White on black
    cld
.pm_print_loop:
    lodsb
    test al, al
    jz .pm_print_done
    mov [edi], ax
    add edi, 2
    jmp .pm_print_loop
.pm_print_done:
    popa
    ret


[BITS 16]
kernel_error:
    mov al, 'E'  ; Error
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

hang:
    cli
    hlt
    jmp $
    
; Helper function to print string in real mode
print_string:
    pusha
    mov ah, 0x0E
    mov bx, 0x0007
.repeat:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .repeat
.done:
    popa
    ret

; Print newline
newline:
    pusha
    mov ah, 0x0E
    mov al, 0x0D
    int 0x10
    mov al, 0x0A
    int 0x10
    popa
    ret

; A20 error handler
a20_error:
    mov si, msg_error
    call print_string
    jmp $

; Helper function to print DWORD in hex
print_hex_dword:
    pusha
    mov cx, 8          ; 8 hex digits
    mov bx, 0x0007     ; Page 0, attribute 7 (normal)
    mov ah, 0x0E       ; BIOS teletype
    mov edx, eax       ; Save original value
.next_digit:
    rol edx, 4         ; Rotate left 4 bits
    mov al, dl         ; Get lower nibble
    and al, 0x0F       ; Isolate nibble
    add al, '0'        ; Convert to ASCII
    cmp al, '9'        ; Check if > '9'
    jbe .print_char
    add al, 7          ; Adjust for A-F
.print_char:
    int 0x10           ; Print character
    loop .next_digit
    popa
    ret

; === ENABLE A20 GATE ===
enable_a20:
    cli
    
    ; Check for keypresses
    mov ah, 0x01
    int 0x16
    jz .no_keypress
    
    ; Check for debug key combo (Ctrl+Alt+D)
    call check_debug_combo
    
    ; Process other keypresses if needed
    ; ...nabled
    
.no_keypress:
    ; Try BIOS method first
    mov ax, 0x2401
    int 0x15
    jnc .a20_enabled
    
    ; Try keyboard controller method
    call .a20_wait
    mov al, 0xAD
    out 0x64, al
    
    call .a20_wait
    mov al, 0xD0
    out 0x64, al
    
    call .a20_wait2
    in al, 0x60
    push eax
    
    call .a20_wait
    mov al, 0xD1
    out 0x64, al
    
    call .a20_wait
    pop eax
    or al, 2
    out 0x60, al
    
    call .a20_wait
    mov al, 0xAE
    out 0x64, al
    
    call .a20_wait
    jmp .a20_enabled
    
.a20_wait:
    in al, 0x64
    test al, 2
    jnz .a20_wait
    ret
    
.a20_wait2:
    in al, 0x64
    test al, 1
    jz .a20_wait2
    ret
    
.a20_enabled:
    ret

; === GLOBAL DESCRIPTOR TABLE (GDT) ===
; Align GDT on 8-byte boundary for performance
align 8
gdt_start:
    ; NULL descriptor (required)
    dq 0
    
    ; Code segment (selector 0x08)
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 0x9A         ; Access byte (P=1, DPL=00, S=1, Type=1010)
    db 0xCF         ; Flags (G=1, D/B=1, L=0, AVL=0) | Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)
    
    ; Data segment (selector 0x10)
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 0x92         ; Access byte (P=1, DPL=00, S=1, Type=0010)
    db 0xCF         ; Flags (G=1, D/B=1, L=0, AVL=0) | Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)
gdt_end:

; Data section
msg_loading db 'Loading TernaryBit OS...', 0
msg_a20 db 'Enabling A20... ', 0
    
; GDT-related messages
msg_gdt db 'Loading GDT...', 0x0D, 0x0A, 0
msg_calc_gdt db 'Calculating GDT address...', 0x0D, 0x0A, 0
msg_stage2_base db 'Stage2 physical base: 0x', 0
msg_cs_base db 'CS Base: 0x', 0
msg_gdt_final db 'Dharma-aligned GDT: 0x', 0
msg_gdt_descriptor db 'GDT Descriptor pointer: 0x', 0
msg_loading_gdt db 'Loading GDT...', 0x0D, 0x0A, 0
msg_gdt_loaded db 'GDT Loaded. Verifying...', 0x0D, 0x0A, 0
msg_gdtr_verification db 'GDTR Base (should match): 0x', 0
msg_pm_transition db 'Preparing protected mode transition...', 0x0D, 0x0A, 0
    
; Register names for dump
reg_names:
    db 'EAX: ', 0
    db 'EBX: ', 0
    db 'ECX: ', 0
    db 'EDX: ', 0
    db 'ESI: ', 0
    db 'EDI: ', 0
    db 'ESP: ', 0
    db 'EBP: ', 0
    db 'EIP: ', 0

; Debug messages
msg_flags db 'Karma State (EFLAGS): 0x', 0
msg_critical_hang db 'DHARMA BREACH: System seeking balance. Dumping state...', 0x0D, 0x0A, 0
msg_pmode db 'Entering protected-mode darshan (heightened perception)...', 0x0D, 0x0A, 0
msg_ok db 'Dharma Aligned', 0x0D, 0x0A, 0
msg_error db 'Karma Imbalance', 0x0D, 0x0A, 0
tbds_ready_msg db 'TBDS stream consecrated for kernel darshan', 0
tbds_ptr_msg   db 'TBDS ptr: 0x', 0
tbds_len_msg   db 'TBDS len: 0x', 0

; Buffer for SGDT verification
gdtr_buffer dw 0
            dd 0
stage2_base dd 0
stage2_phys dd 0

; Protected mode messages
pm_msg db '32-bit Protected-Mode Darshan Active!', 0
gdt_info_msg db 'GDT loaded successfully', 0
cs_msg db 'CS: 0x0008', 0

; Variables
boot_drive db 0

; TBDS constants
TBDS_SIGNATURE    equ 0x53444454
TBDS_VERSION_1_0  equ 0x0100
TBDS_DESCRIPTOR_COUNT equ 5
TBDS_TYPE_ARCH_INFO         equ 0x0001
TBDS_TYPE_FIRMWARE_INFO     equ 0x0002
TBDS_TYPE_MEMORY_MAP        equ 0x0003
TBDS_TYPE_BOOT_DEVICE       equ 0x0004
TBDS_TYPE_CONSOLE_INFO      equ 0x0006

align 8
tbds_data:
    dd TBDS_SIGNATURE
    dd tbds_end - tbds_data
    dw TBDS_VERSION_1_0
    dw TBDS_DESCRIPTOR_COUNT
    times 8 db 0

    ; ARCH_INFO descriptor
    dw TBDS_TYPE_ARCH_INFO
    dw 0
    dd 8
    dw 0x0001
    dw 0x0020
    dw 0x0001
    dw 0x0000

    ; FIRMWARE_INFO descriptor
    dw TBDS_TYPE_FIRMWARE_INFO
    dw 0
    dd 8
    dw 0x0001
    dw 0x0000
    dd 0x00010000

    ; MEMORY_MAP descriptor (two entries)
    dw TBDS_TYPE_MEMORY_MAP
    dw 0
    dd 40
    dq 0x0000000000000000
    dq 0x000000000009FC00
    dd 0x00000001
    dq 0x0000000000100000
    dq 0x000000003FF00000
    dd 0x00000001

    ; BOOT_DEVICE descriptor
    dw TBDS_TYPE_BOOT_DEVICE
    dw 0
    dd 12
    db 0x00
    db 0x00
    dw 0x0002
    dd BOOT_KERNEL_LBA_START
    dd BOOT_KERNEL_SECTOR_COUNT

    ; CONSOLE_INFO descriptor
    dw TBDS_TYPE_CONSOLE_INFO
    dw 0
    dd 6
    dw 0x0001
    dw 80
    dw 25

tbds_end:
tbds_length equ tbds_end - tbds_data

; GDT Descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size (16-bit)
    dd gdt_start                ; Physical address of GDT


; Padding to 4KB
times 4096-($-$$) db 0
