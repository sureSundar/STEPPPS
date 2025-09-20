; TBOS Bootloader
; Loads and executes the TBOS kernel

[bits 16]
[org 0x7C00]

; Constants
KERNEL_LOAD_SEG  equ 0x1000   ; Segment where we'll load the kernel (0x1000:0x0000 = 0x10000)
KERNEL_LOAD_OFF  equ 0x0000   ; Offset where we'll load the kernel
KERNEL_SECTOR    equ 1        ; Kernel starts at sector 1 (right after boot sector)
KERNEL_SECTORS   equ 32       ; Number of sectors to load (16KB)

; Disk Address Packet (DAP)
dap:
    db 0x10           ; Size of DAP (16 bytes)
    db 0              ; Always 0
    dw KERNEL_SECTORS ; Number of sectors to read
    dw KERNEL_LOAD_OFF; Offset
    dw KERNEL_LOAD_SEG; Segment (0x1000:0x0000 = 0x10000)
    dd KERNEL_SECTOR  ; Starting LBA (sector 1)
    dd 0              ; Upper 32 bits of LBA (0 for disks < 2TB)

; Entry point
start:
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Set up stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Clear screen
    mov ax, 0x0003  ; 80x25 text mode
    int 0x10
    
    ; Print loading message
    mov si, loading_msg
    call print_string
    
    ; Load kernel from disk
    call load_kernel
    
    ; Switch to protected mode
    call switch_to_pm
    
    ; This point will never be reached
    jmp $

; Load kernel from disk using LBA with retries
load_kernel:
    pusha
    
    ; Print loading message
    mov si, load_kernel_msg
    call print_string
    
    ; Reset disk system
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    jc disk_error
    
    ; Check for LBA extensions
    mov ah, 0x41
    mov bx, 0x55AA
    mov dl, [boot_drive]
    int 0x13
    jc disk_error  ; No LBA support
    
    ; Try loading up to 3 times
    mov cx, 3
    
.load_retry:
    push cx
    
    ; Load kernel using LBA
    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jnc .load_success
    
    ; Reset disk on error
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13
    
    pop cx
    loop .load_retry
    
    ; If we get here, all retries failed
    jmp disk_error
    
.load_success:
    pop cx
    
    ; Verify the first few bytes were loaded
    mov ax, [KERNEL_LOAD_SEG]
    mov es, ax
    mov di, 0
    mov cx, 4
    xor ax, ax
    repz scasw
    jnz disk_error  ; First 8 bytes are zero, something's wrong
    
    ; Print success message
    mov si, load_ok_msg
    call print_string
    
    popa
    ret

; Switch to 32-bit protected mode
switch_to_pm:
    cli                     ; Disable interrupts
    
    ; Load GDT
    lgdt [gdt_descriptor]
    
    ; Enable A20 line (fast method)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Set protected mode bit
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; Far jump to clear pipeline and enter 32-bit mode
    jmp CODE_SEG:init_pm

[bits 32]
init_pm:
    ; Set up segment registers
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Set up stack
    mov esp, 0x90000
    
    ; Jump to kernel at 0x10000
    jmp 0x10000
    
    ; Halt if kernel returns
    cli
    hlt

; 16-bit print string function
[bits 16]
print_string:
    pusha
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
.next_char:
    lodsb
    or al, al
    jz .done
    int 0x10
    jmp .next_char
.done:
    popa
    ret

; Error handling
disk_error:
    pusha
    mov si, disk_error_msg
    call print_string
    
    ; Print error code in AH
    mov al, ah      ; Error code is in AH
    call print_hex_byte
    
    ; Print newline
    mov al, 0x0A
    call print_char
    mov al, 0x0D
    call print_char
    
    popa
    jmp $

; Print AL as 2-digit hex
print_hex_byte:
    pusha
    mov cx, 2
    mov bl, al
    
.next_digit:
    rol bl, 4
    mov al, bl
    and al, 0x0F
    cmp al, 0x0A
    jb .is_digit
    add al, 7       ; 'A' - '0' - 0xA
.is_digit:
    add al, '0'
    call print_char
    loop .next_digit
    popa
    ret
    
; Print character in AL
print_char:
    pusha
    mov ah, 0x0E
    int 0x10
    popa
    ret

; Data
boot_drive      db 0
loading_msg     db 'TBOS Bootloader', 0x0D, 0x0A, 0
load_kernel_msg db 'Loading kernel... ', 0
load_ok_msg     db 'OK', 0x0D, 0x0A, 0
disk_error_msg  db 'Disk error!', 0x0D, 0x0A, 0

; GDT
gdt_start:
    ; Null segment (required)
    dq 0x0000000000000000
    
    ; Code segment (base=0, limit=4GB, type=0x9A, flags=0xCF)
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 0x9A         ; Access byte (present, ring 0, code, exec/read)
    db 0xCF         ; Flags (4KB granularity, 32-bit) + Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)
    
    ; Data segment (base=0, limit=4GB, type=0x92, flags=0xCF)
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 0x92         ; Access byte (present, ring 0, data, read/write)
    db 0xCF         ; Flags (4KB granularity, 32-bit) + Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size of GDT
    dd gdt_start                ; Start of GDT

; Segment selectors
CODE_SEG equ 0x08  ; Offset of code segment in GDT (0x08 = 8 bytes, after null descriptor)
DATA_SEG equ 0x10  ; Offset of data segment in GDT (0x10 = 16 bytes, after code segment)

; Fill with zeros to make 512 bytes
times 510-($-$$) db 0

dw 0xAA55  ; Boot signature
