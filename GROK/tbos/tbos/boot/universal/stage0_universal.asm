; ============================================================================
; TernaryBit OS - Universal Bootloader Stage 0
; ============================================================================
; Purpose: Universal hardware detection and variant selection
; Size: 512 bytes (MBR compatible)
; Author: TernaryBit OS Team
; Date: 2025-10-23
;
; This is the FIRST code that runs on ANY hardware. It must:
; 1. Initialize minimal CPU state
; 2. Detect hardware capabilities
; 3. Select appropriate Stage 2 variant
; 4. Load and jump to Stage 2
; ============================================================================

[BITS 16]
[ORG 0x7C00]

; ============================================================================
; ENTRY POINT
; ============================================================================

start:
    ; Disable interrupts during initialization
    cli

    ; Normalize segment registers (x86 real mode)
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00      ; Stack grows down from bootloader

    ; Re-enable interrupts
    sti

    ; Save boot drive number (BIOS passes in DL)
    mov [boot_drive], dl

    ; Clear screen
    call clear_screen

    ; Display boot banner
    mov si, banner_msg
    call print_string

    ; === HARDWARE DETECTION ===
    call detect_hardware

    ; === VARIANT SELECTION ===
    call select_variant

    ; === LOAD STAGE 2 ===
    call load_stage2

    ; === JUMP TO STAGE 2 ===
    jmp 0x0000:0x8000

    ; Should never reach here
    cli
    hlt

; ============================================================================
; HARDWARE DETECTION
; ============================================================================

detect_hardware:
    ; Display detection message
    mov si, detect_msg
    call print_string

    ; --- Detect CPU Architecture ---
    call detect_cpu

    ; --- Detect Memory Size ---
    call detect_memory

    ; --- Detect Boot Method ---
    call detect_boot_method

    ; Display detection complete
    mov si, detect_done_msg
    call print_string

    ret

; ----------------------------------------------------------------------------
; Detect CPU Type
; ----------------------------------------------------------------------------
detect_cpu:
    ; Test for x86_64 (check for CPUID support)
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x00200000     ; Try to flip ID bit
    push eax
    popfd
    pushfd
    pop eax
    xor eax, ecx
    jz .is_8086             ; If bit didn't flip, it's 8086

    ; We have CPUID support - check for long mode (64-bit)
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .is_32bit

    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29       ; LM bit (Long Mode)
    jz .is_32bit

    ; 64-bit capable
    mov byte [cpu_bits], 64
    mov si, cpu_64_msg
    call print_string
    jmp .done

.is_32bit:
    mov byte [cpu_bits], 32
    mov si, cpu_32_msg
    call print_string
    jmp .done

.is_8086:
    mov byte [cpu_bits], 16
    mov si, cpu_16_msg
    call print_string

.done:
    ret

; ----------------------------------------------------------------------------
; Detect Memory Size
; ----------------------------------------------------------------------------
detect_memory:
    ; Try BIOS INT 12h (get conventional memory size in KB)
    int 0x12
    jc .no_memory
    cmp ax, 0
    je .no_memory

    ; Store memory size (in KB)
    mov [memory_kb], ax

    ; Display memory size
    mov si, memory_msg
    call print_string
    mov ax, [memory_kb]
    call print_hex16
    mov si, kb_msg
    call print_string

    ret

.no_memory:
    mov word [memory_kb], 64    ; Assume minimal 64KB
    ret

; ----------------------------------------------------------------------------
; Detect Boot Method (BIOS vs UEFI)
; ----------------------------------------------------------------------------
detect_boot_method:
    ; For now, assume BIOS (Stage 0 is MBR = BIOS boot)
    ; UEFI would use a different bootloader entry point
    mov byte [boot_method], 1   ; 1 = BIOS, 2 = UEFI
    mov si, bios_msg
    call print_string
    ret

; ============================================================================
; VARIANT SELECTION
; ============================================================================

select_variant:
    mov si, select_msg
    call print_string

    ; Decision tree based on detected hardware

    ; Check if very low memory (< 64KB) - Calculator/Embedded variant
    cmp word [memory_kb], 64
    jl .variant_a

    ; Check CPU bits
    mov al, [cpu_bits]

    cmp al, 16
    je .variant_b_16bit     ; 16-bit x86 = BIOS variant

    cmp al, 32
    je .variant_b_32bit     ; 32-bit x86 = BIOS variant

    cmp al, 64
    je .variant_b_64bit     ; 64-bit x86 = BIOS variant (or UEFI)

    ; Default to BIOS variant
    jmp .variant_b_16bit

.variant_a:
    mov byte [selected_variant], 'A'
    mov word [stage2_sector], 2
    mov word [stage2_count], 2
    mov si, variant_a_msg
    call print_string
    ret

.variant_b_16bit:
.variant_b_32bit:
.variant_b_64bit:
    mov byte [selected_variant], 'B'
    mov word [stage2_sector], 2
    mov word [stage2_count], 8      ; x86 BIOS loader is bigger
    mov si, variant_b_msg
    call print_string
    ret

; ============================================================================
; LOAD STAGE 2
; ============================================================================

load_stage2:
    mov si, loading_msg
    call print_string

    ; Setup for disk read
    mov dl, [boot_drive]
    mov ax, [stage2_sector]     ; Starting sector
    mov cx, [stage2_count]      ; Number of sectors
    mov bx, 0x8000              ; Load address (0x0000:0x8000)

.read_loop:
    cmp cx, 0
    je .read_done

    ; Convert LBA to CHS
    push ax
    push cx
    call lba_to_chs

    ; Read one sector
    mov ah, 0x02        ; BIOS read sector
    mov al, 1           ; Read 1 sector
    ; CH = cylinder, CL = sector, DH = head (from lba_to_chs)
    mov dl, [boot_drive]
    int 0x13
    jc .read_error

    pop cx
    pop ax

    ; Advance to next sector
    inc ax
    add bx, 512         ; Move buffer forward
    dec cx
    jmp .read_loop

.read_done:
    mov si, loaded_msg
    call print_string
    ret

.read_error:
    mov si, error_msg
    call print_string
    cli
    hlt

; ============================================================================
; UTILITY FUNCTIONS
; ============================================================================

; ----------------------------------------------------------------------------
; Clear Screen
; ----------------------------------------------------------------------------
clear_screen:
    mov ah, 0x00
    mov al, 0x03        ; 80x25 text mode
    int 0x10
    ret

; ----------------------------------------------------------------------------
; Print String (SI = pointer to null-terminated string)
; ----------------------------------------------------------------------------
print_string:
    push ax
    push bx
.loop:
    lodsb
    cmp al, 0
    je .done
    mov ah, 0x0E        ; BIOS teletype
    mov bx, 0x0007      ; Page 0, light gray
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; ----------------------------------------------------------------------------
; Print 16-bit hex value (AX = value)
; ----------------------------------------------------------------------------
print_hex16:
    push ax
    push bx
    push cx

    mov cx, 4           ; 4 hex digits
.loop:
    rol ax, 4           ; Rotate left 4 bits
    mov bx, ax
    and bx, 0x0F        ; Mask lowest 4 bits
    add bl, '0'
    cmp bl, '9'
    jle .print
    add bl, 7           ; Convert to A-F
.print:
    push ax
    mov al, bl
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    pop ax
    dec cx
    jnz .loop

    pop cx
    pop bx
    pop ax
    ret

; ----------------------------------------------------------------------------
; Convert LBA to CHS
; Input: AX = LBA sector
; Output: CH = cylinder, CL = sector, DH = head
; ----------------------------------------------------------------------------
lba_to_chs:
    push ax
    push bx
    push dx

    xor dx, dx
    mov bx, 18          ; Sectors per track (standard floppy)
    div bx              ; AX = track, DX = sector

    mov cl, dl
    inc cl              ; Sectors are 1-based

    xor dx, dx
    mov bx, 2           ; Heads
    div bx              ; AX = cylinder, DX = head

    mov ch, al          ; Cylinder
    mov dh, dl          ; Head

    pop dx
    pop bx
    pop ax
    ret

; ============================================================================
; DATA SECTION
; ============================================================================

; Boot messages
banner_msg          db 'TernaryBit Universal Bootloader v1.0', 13, 10, 0
detect_msg          db 'Detecting hardware...', 13, 10, 0
detect_done_msg     db 'Detection complete.', 13, 10, 0
cpu_16_msg          db '  CPU: 16-bit (8086/286)', 13, 10, 0
cpu_32_msg          db '  CPU: 32-bit (386+)', 13, 10, 0
cpu_64_msg          db '  CPU: 64-bit (x86_64)', 13, 10, 0
memory_msg          db '  Memory: ', 0
kb_msg              db ' KB', 13, 10, 0
bios_msg            db '  Boot: BIOS (MBR)', 13, 10, 0
select_msg          db 'Selecting boot variant...', 13, 10, 0
variant_a_msg       db '  Variant A: Calculator/Embedded', 13, 10, 0
variant_b_msg       db '  Variant B: x86 BIOS', 13, 10, 0
loading_msg         db 'Loading Stage 2...', 13, 10, 0
loaded_msg          db 'Stage 2 loaded. Jumping...', 13, 10, 0
error_msg           db 'ERROR: Disk read failed!', 13, 10, 0

; Hardware detection results
boot_drive          db 0
cpu_bits            db 0        ; 16, 32, or 64
memory_kb           dw 0
boot_method         db 0        ; 1=BIOS, 2=UEFI

; Variant selection
selected_variant    db 0        ; 'A', 'B', 'C', 'D', 'E'
stage2_sector       dw 2        ; Starting sector of Stage 2
stage2_count        dw 8        ; Number of sectors to load

; ============================================================================
; BOOT SIGNATURE
; ============================================================================

times 510-($-$$) db 0           ; Pad to 510 bytes
dw 0xAA55                       ; Boot signature
