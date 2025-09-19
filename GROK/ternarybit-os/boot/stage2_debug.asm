; TernaryBit OS - Enhanced Stage 2 with Comprehensive Debugging
[BITS 16]
[ORG 0x8000]

; Debug configuration
%define DEBUG_BASIC     0x01    ; Basic progress indicators
%define DEBUG_VERBOSE   0x02    ; Detailed operation info
%define DEBUG_REGISTERS 0x04    ; Show register values
%define DEBUG_MEMORY    0x08    ; Memory operations
%define DEBUG_DISK      0x10    ; Disk I/O details
%define DEBUG_ALL       0xFF    ; All debugging

; Global debug level (can be changed by key combo)
debug_level     db DEBUG_BASIC | DEBUG_DISK | DEBUG_VERBOSE

stage2_start:
    ; Very first debug to confirm stage2 entry
    mov al, '!'  ; Entry marker
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Immediate debug - no function calls that could cause hangs
    mov al, 'S'  ; Stage2 started
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov al, '2'  ; Stage 2
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov al, ' '  ; Space
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; === SIMPLIFIED KERNEL LOADING - NO FUNCTION CALLS ===
    ; Load kernel at 0x1000:0000 (linear address 0x10000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    mov al, 'L'  ; Loading
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Get boot drive from boot sector
    mov dl, [0x0500]

    ; Load kernel sectors
    mov ah, 0x02        ; BIOS read sectors
    mov al, 28          ; Load all 28 kernel sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 9           ; Start at sector 9 (kernel location)
    mov dh, 0           ; Head 0
    int 0x13
    jc kernel_load_error

    mov al, 'K'  ; Kernel loaded
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Test: Simple debug before SUCCESS
    mov al, 'T'  ; Test
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    jmp success_message

kernel_load_error:
    mov al, 'E'  ; Error loading kernel
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp hang

success_message:
    ; Debug: About to start protected mode transition
    mov al, 'X'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; === MINIMAL PROTECTED MODE TRANSITION ===

    ; Disable interrupts
    cli

    mov al, 'P'  ; Protected mode start
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Load GDT
    lgdt [gdt_descriptor]

    mov al, 'G'  ; GDT loaded
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code segment
    jmp 0x08:simple_protected_mode

[BITS 32]
simple_protected_mode:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack
    mov esp, 0x90000

    ; Jump to kernel at 0x10000
    jmp 0x10000

[BITS 16]
    ; Should never reach here - fallback infinite loop
.success_loop:
    hlt
    jmp .success_loop

hang:
    cli
    hlt
    jmp hang

[BITS 32]
protected_mode_32:
    ; Test: Write "PMODE" to screen to prove we're in protected mode
    mov edi, 0xB8000
    mov eax, 0x2F50     ; 'P' green on black
    mov [edi], eax
    mov eax, 0x2F4D     ; 'M' green on black
    mov [edi+2], eax
    mov eax, 0x2F4F     ; 'O' green on black
    mov [edi+4], eax
    mov eax, 0x2F44     ; 'D' green on black
    mov [edi+6], eax
    mov eax, 0x2F45     ; 'E' green on black
    mov [edi+8], eax

    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Wait a moment to show PMODE message
    mov ecx, 0x1000000
.wait:
    dec ecx
    jnz .wait

    ; Jump to kernel
    jmp 0x10000

[BITS 16]

    ; Should never reach here
    call debug_error
    jmp $

; =============================================================================
; DEBUG KEY DETECTION
; =============================================================================
check_debug_keys:
    pusha

    ; Display key combo help
    mov si, key_help_msg
    call print_string

    ; Wait for key with timeout
    mov cx, 0x0500      ; Timeout counter
.wait_key:
    mov ah, 0x01        ; Check if key available
    int 0x16
    jz .check_timeout

    ; Key available, read it
    mov ah, 0x00
    int 0x16

    ; Check for debug key combinations
    cmp al, 'd'         ; 'd' = basic debug
    je .set_basic
    cmp al, 'D'         ; 'D' = verbose debug
    je .set_verbose
    cmp al, 'v'         ; 'v' = very verbose
    je .set_very_verbose
    cmp al, 'A'         ; 'A' = all debugging
    je .set_all
    cmp al, 'q'         ; 'q' = quiet (minimal)
    je .set_quiet

    jmp .done

.check_timeout:
    loop .wait_key
    jmp .done

.set_basic:
    mov byte [debug_level], DEBUG_BASIC
    jmp .done
.set_verbose:
    mov byte [debug_level], DEBUG_BASIC | DEBUG_VERBOSE
    jmp .done
.set_very_verbose:
    mov byte [debug_level], DEBUG_BASIC | DEBUG_VERBOSE | DEBUG_REGISTERS
    jmp .done
.set_all:
    mov byte [debug_level], DEBUG_ALL
    jmp .done
.set_quiet:
    mov byte [debug_level], 0
    jmp .done

.done:
    popa
    ret

; =============================================================================
; DEBUG OUTPUT FUNCTIONS
; =============================================================================
debug_print:
    ; Input: AL = debug flag to check, SI = message
    ; Only prints if debug level includes the flag
    pusha
    test byte [debug_level], al
    jz .skip
    call print_string
.skip:
    popa
    ret

debug_char:
    ; Input: AL = debug flag, DL = character
    ; Only prints if debug level includes the flag
    pusha
    test byte [debug_level], al
    jz .skip
    mov al, dl
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
.skip:
    popa
    ret

debug_hex8:
    ; Input: AL = debug flag, DL = 8-bit value to display
    pusha
    test byte [debug_level], al
    jz .skip

    ; Display high nibble
    mov al, dl
    shr al, 4
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jle .high_digit
    add al, 'A' - '0' - 10
.high_digit:
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Display low nibble
    mov al, dl
    and al, 0x0F
    add al, '0'
    cmp al, '9'
    jle .low_digit
    add al, 'A' - '0' - 10
.low_digit:
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

.skip:
    popa
    ret

debug_hex16:
    ; Input: AL = debug flag, DX = 16-bit value to display
    push dx
    mov dl, dh          ; High byte first
    call debug_hex8
    pop dx
    push dx
    call debug_hex8     ; Low byte
    pop dx
    ret

debug_registers:
    ; Display all register values if register debugging enabled
    push ax
    push si
    mov al, DEBUG_REGISTERS
    test byte [debug_level], al
    jz .skip

    mov si, reg_dump_msg
    call print_string

    ; Show AX
    mov si, ax_msg
    call print_string
    pop ax              ; Get original AX
    push ax
    mov dx, ax
    mov al, DEBUG_REGISTERS
    call debug_hex16
    call print_newline

.skip:
    pop si
    pop ax
    ret

; =============================================================================
; KERNEL LOADING WITH DEBUGGING
; =============================================================================
debug_kernel_loading_start:
    mov al, DEBUG_BASIC
    mov si, kernel_load_start_msg
    call debug_print
    call debug_registers
    ret

load_kernel_with_debugging:
    ; Set up memory
    mov al, DEBUG_MEMORY
    mov si, setting_memory_msg
    call debug_print

    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    mov al, DEBUG_MEMORY
    mov si, memory_set_msg
    call debug_print
    mov al, DEBUG_MEMORY
    mov dx, es
    call debug_hex16
    mov al, DEBUG_MEMORY
    mov dl, ':'
    call debug_char
    mov al, DEBUG_MEMORY
    mov dx, bx
    call debug_hex16
    call print_newline

    ; Get boot drive
    mov dl, [0x0500]
    mov al, DEBUG_DISK
    mov si, boot_drive_msg
    call debug_print
    mov al, DEBUG_DISK
    push dx
    call debug_hex8
    pop dx
    call print_newline

    ; Try loading first sector
    mov al, DEBUG_DISK
    mov si, loading_sector_msg
    call debug_print

    ; Set up disk parameters
    mov ah, 0x02        ; BIOS read sectors
    mov al, 1           ; Load 1 sector
    mov ch, 0           ; Cylinder 0
    mov cl, 10          ; Start at sector 10
    mov dh, 0           ; Head 0
    mov dl, [0x0500]    ; Get boot drive again (fresh copy)

    ; Call BIOS interrupt immediately (no debug output that could corrupt registers)
    int 0x13
    jc .kernel_error

    ; Success!
    mov al, DEBUG_BASIC
    mov dl, '1'
    call debug_char

    ; Load more sectors
    mov al, DEBUG_DISK
    mov si, loading_more_msg
    call debug_print

    mov ah, 0x02
    mov al, 10
    mov ch, 0
    mov cl, 11
    mov dh, 0
    mov dl, [0x0500]
    int 0x13
    jc .kernel_error

    mov al, DEBUG_BASIC
    mov dl, '2'
    call debug_char

    jmp .success

.kernel_error:
    mov al, DEBUG_BASIC
    mov dl, 'E'
    call debug_char

    ; Detailed error analysis
    mov al, DEBUG_DISK
    mov si, disk_error_msg
    call debug_print
    mov al, DEBUG_DISK
    push ax
    mov al, ah          ; Error code in AH
    mov dl, al
    mov al, DEBUG_DISK
    call debug_hex8
    pop ax
    call print_newline

    ; Show carry flag status
    mov al, DEBUG_DISK
    mov si, carry_flag_msg
    call debug_print

    jmp .done

.success:
    mov al, DEBUG_BASIC
    mov dl, 'K'
    call debug_char

.done:
    ret

debug_kernel_loading_complete:
    ; Simple debug char only - skip the verbose debug_print call
    mov al, 'C'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Skip the debug_print call that might be corrupting something
    ; mov al, DEBUG_VERBOSE
    ; mov si, kernel_complete_msg
    ; call debug_print

    ; Debug: About to return
    mov al, 'R'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    ret

; =============================================================================
; PROTECTED MODE DEBUGGING
; =============================================================================
debug_protected_mode_start:
    mov al, DEBUG_VERBOSE
    mov si, pmode_start_msg
    call debug_print
    ret

enter_protected_mode_debug:
    mov al, DEBUG_VERBOSE
    mov si, disabling_interrupts_msg
    call debug_print
    cli

    ; Enable A20 line first
    in al, 0x92
    test al, 2
    jnz .a20_done
    or al, 2
    and al, 0xFE
    out 0x92, al
.a20_done:

    mov al, DEBUG_VERBOSE
    mov si, loading_gdt_msg
    call debug_print
    lgdt [gdt_descriptor]

    mov al, DEBUG_VERBOSE
    mov si, setting_pe_bit_msg
    call debug_print

    ; By Rama's divine grace - enter protected mode
    mov eax, cr0
    or eax, 1               ; Set PE bit
    mov cr0, eax

    ; Far jump to flush CPU pipeline and enter protected mode
    jmp 0x08:protected_mode_entry

; =============================================================================
; UTILITY FUNCTIONS
; =============================================================================
print_banner:
    mov si, banner_msg
    call print_string
    ret

show_debug_level:
    mov al, DEBUG_BASIC
    test byte [debug_level], al
    jz .check_verbose
    mov si, debug_basic_msg
    call print_string

.check_verbose:
    mov al, DEBUG_VERBOSE
    test byte [debug_level], al
    jz .done
    mov si, debug_verbose_msg
    call print_string

.done:
    ret

print_string:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

print_newline:
    push ax
    push bx
    mov ah, 0x0E
    mov al, 0x0D
    mov bx, 0x0007
    int 0x10
    mov al, 0x0A
    int 0x10
    pop bx
    pop ax
    ret

debug_error:
    mov al, DEBUG_BASIC
    mov si, fatal_error_msg
    call debug_print
    ret

; =============================================================================
; PROTECTED MODE ENTRY POINT
; =============================================================================
[BITS 32]
protected_mode_entry:
    ; Clear screen first
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ax, 0x0720     ; Space with white on black
    rep stosw

    ; Show we reached protected mode - "RAMA" message
    mov edi, 0xB8000
    mov eax, 0x4F52     ; 'R' white on red
    mov [edi], eax
    mov eax, 0x4F41     ; 'A' white on red
    mov [edi+2], eax
    mov eax, 0x4F4D     ; 'M' white on red
    mov [edi+4], eax
    mov eax, 0x4F41     ; 'A' white on red
    mov [edi+6], eax

    ; Set up data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack
    mov esp, 0x90000

    ; Pause to show "RAMA" message
    mov ecx, 0x1000000
.pause:
    dec ecx
    jnz .pause

    ; Jump to kernel
    jmp 0x10000

    ; Should never reach here
    cli
    hlt

[BITS 16]
; =============================================================================
; GDT DEFINITION - By Rama's divine guidance
; =============================================================================
align 16
gdt_start:
    ; Null descriptor (required)
    dq 0x0000000000000000

    ; Code segment descriptor (selector 0x08)
    ; Base=0, Limit=4GB, 32-bit code, privilege level 0
    dw 0xFFFF       ; Limit 0:15
    dw 0x0000       ; Base 0:15
    db 0x00         ; Base 16:23
    db 0x9A         ; Access: Present, Ring 0, Code, Executable, Readable
    db 0xCF         ; Granularity: 4KB, 32-bit, Limit 16:19
    db 0x00         ; Base 24:31

    ; Data segment descriptor (selector 0x10)
    ; Base=0, Limit=4GB, 32-bit data, privilege level 0
    dw 0xFFFF       ; Limit 0:15
    dw 0x0000       ; Base 0:15
    db 0x00         ; Base 16:23
    db 0x92         ; Access: Present, Ring 0, Data, Writable
    db 0xCF         ; Granularity: 4KB, 32-bit, Limit 16:19
    db 0x00         ; Base 24:31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1    ; GDT limit
    dd gdt_start                  ; GDT base address

; =============================================================================
; DEBUG MESSAGES
; =============================================================================
banner_msg              db 'TBOS Debug v2.1', 0x0D, 0x0A, 0x00
key_help_msg            db 'Keys: d=basic, D=verbose, v=very verbose, A=all, q=quiet [2sec]', 0x0D, 0x0A, 0x00
debug_basic_msg         db '[DEBUG: Basic] ', 0x00
debug_verbose_msg       db '[VERBOSE] ', 0x00
kernel_load_start_msg   db 'Starting kernel load...', 0x0D, 0x0A, 0x00
setting_memory_msg      db 'Setting memory segment...', 0x00
memory_set_msg          db 'Memory: ', 0x00
boot_drive_msg          db 'Boot drive: 0x', 0x00
loading_sector_msg      db 'Loading sector...', 0x0D, 0x0A, 0x00
disk_params_msg         db 'Disk params: ', 0x00
loading_more_msg        db 'Loading additional sectors...', 0x0D, 0x0A, 0x00
disk_error_msg          db 'Disk error code: 0x', 0x00
carry_flag_msg          db 'Carry flag set', 0x0D, 0x0A, 0x00
kernel_complete_msg     db 'Kernel loading complete!', 0x0D, 0x0A, 0x00
pmode_start_msg         db 'Entering protected mode...', 0x0D, 0x0A, 0x00
disabling_interrupts_msg db 'Disabling interrupts...', 0x0D, 0x0A, 0x00
loading_gdt_msg         db 'Loading GDT...', 0x0D, 0x0A, 0x00
setting_pe_bit_msg      db 'Setting PE bit...', 0x0D, 0x0A, 0x00
jumping_pmode_msg       db 'Jumping to protected mode...', 0x0D, 0x0A, 0x00
fatal_error_msg         db 'FATAL ERROR!', 0x0D, 0x0A, 0x00
reg_dump_msg            db 'Registers:', 0x0D, 0x0A, 0x00
ax_msg                  db 'AX=', 0x00

; Padding to 4KB
times 4096-($-$$) db 0