; ============================================================================
; TernaryBit OS - x86 BIOS Stage 2 Bootloader
; ============================================================================
; Purpose: Load and initialize kernel on x86 BIOS systems
; Loaded at: 0x8000
; Size: ~4KB
; Author: TernaryBit OS Team
; Date: 2025-10-23
;
; Responsibilities:
; 1. Initialize STEPPPS framework
; 2. Switch to protected mode (32-bit)
; 3. Optionally switch to long mode (64-bit)
; 4. Load kernel from disk
; 5. Transfer control to kernel
; ============================================================================

[BITS 16]
[ORG 0x8000]

; ============================================================================
; CONSTANTS
; ============================================================================

KERNEL_LOAD_ADDR    equ 0x100000    ; Load kernel at 1MB
KERNEL_SECTOR       equ 10          ; Kernel starts at sector 10
KERNEL_SECTORS      equ 100         ; Load 100 sectors (50KB)

; ============================================================================
; ENTRY POINT
; ============================================================================

stage2_start:
    ; Normalize segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000
    sti

    ; Display Stage 2 banner
    mov si, stage2_banner
    call print_string

    ; Initialize STEPPPS Framework
    call init_steppps

    ; Enable A20 line (access memory > 1MB)
    call enable_a20

    ; Load kernel into memory
    call load_kernel

    ; Switch to protected mode
    call enter_protected_mode

    ; Should never return here
    cli
    hlt

; ============================================================================
; STEPPPS FRAMEWORK INITIALIZATION
; ============================================================================

init_steppps:
    mov si, steppps_msg
    call print_string

    ; SPACE: Hardware and memory initialization
    call steppps_space

    ; TIME: Timer and timing initialization
    call steppps_time

    ; EVENT: Interrupt preparation
    call steppps_event

    ; PSYCHOLOGY: User interface setup
    call steppps_psychology

    ; PIXEL: Display initialization
    call steppps_pixel

    ; PROMPT: Command line setup
    call steppps_prompt

    ; SCRIPT: Automation initialization
    call steppps_script

    mov si, steppps_ready
    call print_string
    ret

; --- SPACE Dimension ---
steppps_space:
    mov si, space_msg
    call print_string

    ; Detect extended memory using INT 15h, E820h
    call detect_memory_e820

    ret

; --- TIME Dimension ---
steppps_time:
    mov si, time_msg
    call print_string

    ; Get system time from BIOS
    mov ah, 0x02
    int 0x1A

    ret

; --- EVENT Dimension ---
steppps_event:
    mov si, event_msg
    call print_string
    ; Event system will be initialized in protected mode
    ret

; --- PSYCHOLOGY Dimension ---
steppps_psychology:
    mov si, psychology_msg
    call print_string
    ; AI/ML initialization happens in kernel
    ret

; --- PIXEL Dimension ---
steppps_pixel:
    mov si, pixel_msg
    call print_string

    ; Detect video mode
    mov ah, 0x0F
    int 0x10
    ; AL = current video mode

    ret

; --- PROMPT Dimension ---
steppps_prompt:
    mov si, prompt_msg
    call print_string
    ; Command line interface starts in kernel
    ret

; --- SCRIPT Dimension ---
steppps_script:
    mov si, script_msg
    call print_string
    ; Scripting engine loads with kernel
    ret

; ============================================================================
; MEMORY DETECTION (E820)
; ============================================================================

detect_memory_e820:
    push es
    push di

    xor ebx, ebx
    xor bp, bp              ; Entry count
    mov di, 0x5000          ; Store memory map at 0x5000
    mov edx, 0x534D4150     ; 'SMAP' signature

.loop:
    mov eax, 0xE820
    mov ecx, 24             ; Request 24 bytes
    int 0x15

    jc .done                ; If carry, we're done
    cmp eax, 0x534D4150     ; Verify 'SMAP'
    jne .done

    ; Valid entry
    inc bp
    add di, 24

    test ebx, ebx
    jz .done

    jmp .loop

.done:
    mov [memory_entries], bp
    pop di
    pop es
    ret

; ============================================================================
; ENABLE A20 LINE
; ============================================================================

enable_a20:
    mov si, a20_msg
    call print_string

    ; Try BIOS method first
    mov ax, 0x2401
    int 0x15
    jnc .done

    ; Try keyboard controller method
    call .wait_8042
    mov al, 0xAD            ; Disable keyboard
    out 0x64, al

    call .wait_8042
    mov al, 0xD0            ; Read output port
    out 0x64, al

    call .wait_8042_data
    in al, 0x60
    push ax

    call .wait_8042
    mov al, 0xD1            ; Write output port
    out 0x64, al

    call .wait_8042
    pop ax
    or al, 2                ; Set A20 bit
    out 0x60, al

    call .wait_8042
    mov al, 0xAE            ; Enable keyboard
    out 0x64, al

    call .wait_8042

.done:
    ret

.wait_8042:
    in al, 0x64
    test al, 2
    jnz .wait_8042
    ret

.wait_8042_data:
    in al, 0x64
    test al, 1
    jz .wait_8042_data
    ret

; ============================================================================
; LOAD KERNEL
; ============================================================================

load_kernel:
    mov si, kernel_msg
    call print_string

    ; We need to load kernel above 1MB, which requires protected mode
    ; For now, load to conventional memory then copy later

    ; Load to 0x10000 (64KB mark) temporarily
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    mov ax, KERNEL_SECTOR
    mov cx, KERNEL_SECTORS

.load_loop:
    cmp cx, 0
    je .done

    push ax
    push cx

    ; Convert LBA to CHS
    call lba_to_chs

    ; Read sector
    mov ah, 0x02
    mov al, 1
    mov dl, [0x0500]        ; Boot drive
    int 0x13
    jc .error

    pop cx
    pop ax

    inc ax
    add bx, 512
    ; Handle segment wrap
    cmp bx, 0
    jne .no_wrap
    mov dx, es
    add dx, 0x1000
    mov es, dx
    xor bx, bx
.no_wrap:
    dec cx
    jmp .load_loop

.done:
    mov si, kernel_loaded
    call print_string
    ret

.error:
    mov si, kernel_error
    call print_string
    cli
    hlt

; ============================================================================
; ENTER PROTECTED MODE
; ============================================================================

enter_protected_mode:
    mov si, pmode_msg
    call print_string

    ; Disable interrupts
    cli

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode (set PE bit in CR0)
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to flush pipeline and load CS
    jmp 0x08:protected_mode_entry

[BITS 32]
protected_mode_entry:
    ; Setup protected mode segments
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000    ; Stack at 576KB

    ; Now in 32-bit protected mode!

    ; Copy kernel from conventional memory to 1MB
    call copy_kernel_high

    ; Jump to kernel entry point
    mov eax, KERNEL_LOAD_ADDR
    jmp eax

    ; Should never reach here
    cli
    hlt

; ============================================================================
; COPY KERNEL TO HIGH MEMORY
; ============================================================================

copy_kernel_high:
    mov esi, 0x10000            ; Source: 64KB
    mov edi, KERNEL_LOAD_ADDR   ; Dest: 1MB
    mov ecx, KERNEL_SECTORS * 512 / 4  ; Copy DWORDs
    rep movsd
    ret

; ============================================================================
; UTILITY FUNCTIONS (16-bit)
; ============================================================================

[BITS 16]

; Print string
print_string:
    push ax
    push bx
.loop:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; LBA to CHS conversion
lba_to_chs:
    push ax
    push bx
    push dx

    xor dx, dx
    mov bx, 18
    div bx

    mov cl, dl
    inc cl

    xor dx, dx
    mov bx, 2
    div bx

    mov ch, al
    mov dh, dl

    pop dx
    pop bx
    pop ax
    ret

; ============================================================================
; GLOBAL DESCRIPTOR TABLE (GDT)
; ============================================================================

align 16
gdt_start:
    ; Null descriptor
    dq 0

    ; Code segment (selector 0x08)
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (middle)
    db 10011010b    ; Access: present, ring 0, code, execute/read
    db 11001111b    ; Granularity: 4KB, 32-bit
    db 0x00         ; Base (high)

    ; Data segment (selector 0x10)
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (middle)
    db 10010010b    ; Access: present, ring 0, data, read/write
    db 11001111b    ; Granularity: 4KB, 32-bit
    db 0x00         ; Base (high)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Offset

; ============================================================================
; DATA SECTION
; ============================================================================

stage2_banner       db 'TernaryBit Stage 2 - x86 BIOS', 13, 10, 0
steppps_msg         db 'Initializing STEPPPS Framework...', 13, 10, 0
space_msg           db '  [SPACE] Memory detection', 13, 10, 0
time_msg            db '  [TIME] System timing', 13, 10, 0
event_msg           db '  [EVENT] Interrupt prep', 13, 10, 0
psychology_msg      db '  [PSYCHOLOGY] UI init', 13, 10, 0
pixel_msg           db '  [PIXEL] Display setup', 13, 10, 0
prompt_msg          db '  [PROMPT] Command line', 13, 10, 0
script_msg          db '  [SCRIPT] Automation', 13, 10, 0
steppps_ready       db 'STEPPPS Ready!', 13, 10, 0
a20_msg             db 'Enabling A20 line...', 13, 10, 0
kernel_msg          db 'Loading kernel...', 13, 10, 0
kernel_loaded       db 'Kernel loaded.', 13, 10, 0
kernel_error        db 'ERROR: Kernel load failed!', 13, 10, 0
pmode_msg           db 'Entering protected mode...', 13, 10, 0

; Memory detection results
memory_entries      dw 0

; Pad to 4KB
times 4096-($-$$) db 0
