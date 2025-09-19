; TernaryBit OS - Simple Stage 2 for Kernel Boot
; COLLABORATION LOG:
; - Windsurf: Fixed GDT physical address ✅, A20 gate ✅
; - Claude: Protected mode transition ✅, sector count fixes ✅
; - Codex: GDT structure cleanup ✅, added PM debug marker ✅
; - Codex: updated 2025: Runtime GDT base via CS, serial PM banner ✅
; - STATUS: !S2LKPA now reaches GDT load; protected-mode banner serializes
[BITS 16]
[ORG 0x8000]

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

    ; Load kernel sectors (28 sectors starting at sector 9)
    mov ah, 0x02        ; BIOS read sectors
    mov al, 28          ; Load all 28 kernel sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 9           ; Start at sector 9 (kernel location)
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
    xor eax, eax
    mov ax, cs
    shl eax, 4
    xor ebx, ebx
    mov bx, gdt_start
    add eax, ebx
    mov [gdt_descriptor + 2], eax

    ; Print GDT address
    call print_hex_dword
    call newline
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Print success
    mov si, msg_ok
    call print_string
    
    mov al, 'G'            ; GDT loaded
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Enter protected mode
    mov si, msg_pmode
    call print_string
    
    cli
    mov eax, cr0
    or eax, 0x00000001     ; Set PE bit
    mov cr0, eax
    
    ; Far jump to 32-bit code segment
    jmp 0x08:pm_start

[BITS 32]
; Protected mode entry point
pm_start:
    ; Set up segment registers
    mov ax, 0x10           ; Data segment selector (0x10 = 2nd entry in GDT)
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
    
    ; Halt the CPU
    cli
    hlt
    
    ; Should never reach here
    jmp $


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

; PM print string function
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
msg_loading db 'Booting TernaryBit OS...', 0x0D, 0x0A, 0
msg_a20     db 'Enabling A20 gate... ', 0
msg_gdt     db 'Loading GDT at 0x', 0
msg_pmode   db 'Entering protected mode...', 0x0D, 0x0A, 0
msg_ok      db 'OK', 0x0D, 0x0A, 0
msg_error   db 'ERROR', 0x0D, 0x0A, 0

; Protected mode messages
pm_msg       db '32-bit Protected Mode Active!', 0
gdt_info_msg db 'GDT loaded successfully', 0
cs_msg      db 'CS: 0x0008', 0

; Variables
boot_drive db 0

; GDT Descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size (16-bit)
    dd 0                        ; Patched at runtime with physical address

; Padding to 4KB
times 4096-($-$$) db 0
