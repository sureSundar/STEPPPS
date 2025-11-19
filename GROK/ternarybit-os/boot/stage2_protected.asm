; TernaryBit OS - Protected Mode Stage 2 Bootloader
; Implements proper protected mode transition and loads custom TBOS kernel
; Pair-Programming: ternarybit@gmail.com + Claude Code
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 16]
[ORG 0x8000]

; === CONSTANTS ===
KERNEL_LOAD_ADDR    equ 0x10000    ; Load kernel at 64KB (1MB boundary would be 0x100000)
KERNEL_SECTORS      equ 50         ; Load 50 sectors (~25KB) - adjust as needed
KERNEL_START_SECTOR equ 10         ; Kernel starts at sector 10 (after boot + stage2)
BOOT_DRIVE_ADDR     equ 0x0500     ; Where stage1 saved boot drive

; GDT segment selectors (index * 8)
CODE_SEG equ 0x08  ; Code segment selector
DATA_SEG equ 0x10  ; Data segment selector

; === ENTRY POINT ===
stage2_start:
    ; Setup segments for real mode
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000      ; Stack just below stage2
    sti

    ; Display banner
    mov si, msg_banner
    call print_string

    ; Initialize STEPPPS framework (display messages)
    call init_steppps

    ; Enable A20 line
    call enable_a20_line

    ; Load kernel into memory
    call load_kernel

    ; Setup GDT
    call setup_gdt

    ; Switch to protected mode
    call enter_protected_mode

    ; Should never reach here
    jmp halt_system

;==========================================
; STEPPPS Framework Initialization
;==========================================
init_steppps:
    mov si, msg_steppps_init
    call print_string

    ; Display each dimension
    mov si, msg_space
    call print_string
    mov si, msg_time
    call print_string
    mov si, msg_event
    call print_string
    mov si, msg_psychology
    call print_string
    mov si, msg_pixel
    call print_string
    mov si, msg_prompt
    call print_string
    mov si, msg_script
    call print_string

    mov si, msg_steppps_ready
    call print_string
    ret

;==========================================
; Enable A20 Line (Fast method)
;==========================================
enable_a20_line:
    mov si, msg_a20
    call print_string

    ; Try fast A20 gate (System Control Port A)
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Verify A20 is enabled (simplified check)
    ; In production, you'd verify by testing memory wraparound

    mov si, msg_a20_done
    call print_string
    ret

;==========================================
; Load Kernel from Disk
;==========================================
load_kernel:
    mov si, msg_loading_kernel
    call print_string

    ; Setup buffer to load kernel
    mov ax, KERNEL_LOAD_ADDR >> 4
    mov es, ax
    xor bx, bx

    ; Get boot drive
    mov dl, [BOOT_DRIVE_ADDR]

    ; Load kernel sectors
    mov si, KERNEL_START_SECTOR  ; Starting LBA
    mov cx, KERNEL_SECTORS       ; Number of sectors

.load_loop:
    cmp cx, 0
    jz .load_done

    ; Save registers
    push cx
    push si

    ; Convert LBA to CHS
    call lba_to_chs

    ; Read sector
    mov ah, 0x02    ; Read sectors function
    mov al, 1       ; Read 1 sector
    int 0x13        ; BIOS disk service
    jc .load_error

    ; Restore and advance
    pop si
    pop cx
    inc si          ; Next LBA
    add bx, 512     ; Next buffer position

    ; Handle segment wraparound if needed
    cmp bx, 0
    jne .no_wrap
    mov ax, es
    add ax, 0x1000
    mov es, ax
.no_wrap:

    dec cx
    jmp .load_loop

.load_done:
    mov si, msg_kernel_loaded
    call print_string
    ret

.load_error:
    mov si, msg_load_error
    call print_string
    jmp halt_system

;==========================================
; Setup GDT (Global Descriptor Table)
;==========================================
setup_gdt:
    mov si, msg_setup_gdt
    call print_string

    ; Reset ES to 0 (it was set to 0x1000 for kernel loading)
    xor ax, ax
    mov es, ax

    ; Load GDT descriptor
    lgdt [gdt_descriptor]

    mov si, msg_gdt_done
    call print_string
    ret

;==========================================
; Enter Protected Mode
;==========================================
enter_protected_mode:
    mov si, msg_protected_mode
    call print_string

    ; Disable interrupts
    cli

    ; Enable protected mode bit in CR0
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush pipeline and load CS with code selector
    ; Calculate physical address: ORG + offset from start
    jmp CODE_SEG:(0x8000 + (protected_mode_entry - $$))

;==========================================
; Protected Mode Entry (32-bit code)
;==========================================
[BITS 32]
protected_mode_entry:
    ; We are now in 32-bit protected mode!

    ; CRITICAL: Disable interrupts (no IDT set up yet!)
    cli

    ; Setup all segment registers to point to data segment
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Setup stack at 0x90000 (as kernel expects)
    mov esp, 0x90000
    mov ebp, esp

    ; Debug: Write to serial before jumping to kernel
    mov dx, 0x3F8
    mov al, '['
    out dx, al
    mov al, 'S'
    out dx, al
    mov al, '2'
    out dx, al
    mov al, 'P'
    out dx, al
    mov al, 'M'
    out dx, al
    mov al, ']'
    out dx, al
    mov al, 10
    out dx, al

    ; Jump to kernel at 0x10000 (absolute jump via register)
    mov eax, KERNEL_LOAD_ADDR
    jmp eax

    ; Should never reach here
    jmp $

;==========================================
; Print String in Protected Mode (VGA)
;==========================================
print_string_pm:
    push eax
    push ebx
    mov ebx, 0xB8000    ; VGA text buffer
    add ebx, (24 * 80 * 2)  ; Line 24

.loop:
    lodsb                   ; Load byte from ESI
    test al, al
    jz .done
    mov ah, 0x0F           ; White on black
    mov [ebx], ax
    add ebx, 2
    jmp .loop

.done:
    pop ebx
    pop eax
    ret

;==========================================
; Utility Functions (16-bit Real Mode)
;==========================================
[BITS 16]

; Convert LBA (in SI) to CHS
; Assumes standard floppy/disk geometry: 18 sectors/track, 2 heads
lba_to_chs:
    push ax
    push bx
    push dx

    mov ax, si
    mov bx, 18          ; Sectors per track
    xor dx, dx
    div bx              ; AX = track, DX = sector

    mov cl, dl
    inc cl              ; Sectors are 1-based

    mov bx, 2           ; Heads per cylinder
    xor dx, dx
    div bx              ; AX = cylinder, DX = head

    mov ch, al          ; Cylinder
    mov dh, dl          ; Head

    pop dx
    pop bx
    pop ax
    ret

; Print string (SI points to null-terminated string)
print_string:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E        ; Teletype output
    mov bx, 0x0007      ; Page 0, white on black
    int 0x10            ; BIOS video interrupt
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; Halt system
halt_system:
    cli
    hlt
    jmp halt_system

;==========================================
; GDT Definition
;==========================================
align 8
gdt_start:

; Null descriptor (required)
gdt_null:
    dd 0x0
    dd 0x0

; Code segment descriptor
; Base = 0x0, Limit = 0xFFFFF (4GB)
; Access: Present, Ring 0, Code, Executable, Readable
; Flags: 4KB granularity, 32-bit
gdt_code:
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10011010b    ; Access: Present, Ring 0, Code, Executable, Readable
    db 11001111b    ; Flags: Granularity 4KB, 32-bit, Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

; Data segment descriptor
; Base = 0x0, Limit = 0xFFFFF (4GB)
; Access: Present, Ring 0, Data, Writable
; Flags: 4KB granularity, 32-bit
gdt_data:
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0x0000       ; Base (bits 0-15)
    db 0x00         ; Base (bits 16-23)
    db 10010010b    ; Access: Present, Ring 0, Data, Writable
    db 11001111b    ; Flags: Granularity 4KB, 32-bit, Limit (bits 16-19)
    db 0x00         ; Base (bits 24-31)

gdt_end:

; GDT Descriptor
gdt_descriptor:
    dw gdt_end - gdt_start - 1          ; Size of GDT
    dd 0x8000 + (gdt_start - $$)        ; Physical address = ORG + offset

;==========================================
; Data Section
;==========================================
msg_banner:         db '=== TernaryBit OS Stage 2 Bootloader ===', 13, 10, 0
msg_steppps_init:   db 'Initializing STEPPPS Framework...', 13, 10, 0
msg_space:          db '  [SPACE] Hardware detection', 13, 10, 0
msg_time:           db '  [TIME] Timing systems', 13, 10, 0
msg_event:          db '  [EVENT] Event handling', 13, 10, 0
msg_psychology:     db '  [PSYCHOLOGY] AI framework', 13, 10, 0
msg_pixel:          db '  [PIXEL] Display systems', 13, 10, 0
msg_prompt:         db '  [PROMPT] Command interface', 13, 10, 0
msg_script:         db '  [SCRIPT] Automation', 13, 10, 0
msg_steppps_ready:  db 'STEPPPS Framework Ready!', 13, 10, 0
msg_a20:            db 'Enabling A20 line...', 13, 10, 0
msg_a20_done:       db 'A20 enabled', 13, 10, 0
msg_loading_kernel: db 'Loading TBOS kernel...', 13, 10, 0
msg_kernel_loaded:  db 'Kernel loaded successfully!', 13, 10, 0
msg_setup_gdt:      db 'Setting up GDT...', 13, 10, 0
msg_gdt_done:       db 'GDT configured', 13, 10, 0
msg_protected_mode: db 'Entering protected mode...', 13, 10, 0
msg_load_error:     db 'ERROR: Failed to load kernel!', 13, 10, 0

[BITS 32]
msg_pm_success:     db 'Protected mode active! ', 0
msg_jumping_kernel: db 'Jumping to kernel...', 0

; Pad to 4KB
[BITS 16]
times 4096-($-$$) db 0
