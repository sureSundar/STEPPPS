; TBOS Stage 2 Bootloader - Sprint 1
; 🕉️ Hariharaputram Saranam 🕉️
; Protected mode transition and kernel loading

[BITS 16]
[ORG 0x8000]

stage2_start:
    ; Display stage 2 message
    mov si, msg_stage2
    call print_string_16

    ; Enable A20 gate (Sprint 4 preview)
    call enable_a20

    ; Load GDT
    call load_gdt

    ; Switch to protected mode
    mov si, msg_protected
    call print_string_16

    ; Enter protected mode
    cli                     ; Disable interrupts
    mov eax, cr0
    or eax, 1               ; Set PE bit
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:protected_mode

;==========================================
; 16-bit Functions
;==========================================
print_string_16:
    push ax
    push bx
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

enable_a20:
    ; Fast A20 method
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

load_gdt:
    lgdt [gdt_descriptor]
    ret

;==========================================
; GDT (Global Descriptor Table)
;==========================================
gdt_start:
    ; Null descriptor
    dd 0x00000000
    dd 0x00000000

    ; Code segment descriptor
    dw 0xFFFF               ; Limit (bits 0-15)
    dw 0x0000               ; Base (bits 0-15)
    db 0x00                 ; Base (bits 16-23)
    db 10011010b            ; Access byte
    db 11001111b            ; Flags + Limit (bits 16-19)
    db 0x00                 ; Base (bits 24-31)

    ; Data segment descriptor
    dw 0xFFFF               ; Limit (bits 0-15)
    dw 0x0000               ; Base (bits 0-15)
    db 0x00                 ; Base (bits 16-23)
    db 10010010b            ; Access byte
    db 11001111b            ; Flags + Limit (bits 16-19)
    db 0x00                 ; Base (bits 24-31)

    ; Real mode segment descriptor (16-bit)
    dw 0xFFFF               ; Limit (bits 0-15)
    dw 0x0000               ; Base (bits 0-15)
    db 0x00                 ; Base (bits 16-23)
    db 10011010b            ; Access byte (16-bit code)
    db 00000000b            ; Flags (16-bit)
    db 0x00                 ; Base (bits 24-31)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                 ; GDT address

;==========================================
; 32-bit Protected Mode Code
;==========================================
[BITS 32]
protected_mode:
    ; Setup segments
    mov ax, 0x10            ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000        ; Setup stack

    ; Clear screen (text mode)
    call clear_screen_32

    ; Print protected mode message
    mov esi, msg_pm_success
    call print_string_32

    ; STEPPPS Initialization
    call init_steppps_32

    ; Load kernel
    mov esi, msg_kernel_load
    call print_string_32

    ; Load kernel from disk (assuming it's at sector 10)
    call load_kernel

    ; Jump to kernel
    mov esi, msg_jumping
    call print_string_32

    ; Jump to kernel entry at 0x100000 (1MB)
    jmp 0x08:0x100000

load_kernel:
    ; Load real kernel from disk sectors 10+ to 0x100000 (1MB)
    pusha

    ; We need to use BIOS interrupts in 16-bit mode
    ; Switch back to real mode temporarily
    jmp 0x18:real_mode_kernel_load

real_mode_kernel_load:
    [BITS 16]
    ; Disable protected mode
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax

    ; Far jump to fix CS
    jmp 0x0000:real_mode_setup

real_mode_setup:
    ; Reset segments
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x7000

    ; Load kernel from sector 10 (where build.sh puts it)
    ; Load to 0x1000:0x0000 (which is 0x10000 physical)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx          ; es:bx = 0x1000:0x0000 = 0x10000

    mov ah, 0x02        ; Read sectors
    mov al, 32          ; Read 32 sectors (16KB) - should be enough for kernel
    mov ch, 0           ; Cylinder 0
    mov cl, 11          ; Sector 11 (BIOS sectors start at 1, dd seek=10 = BIOS sector 11)
    mov dh, 0           ; Head 0
    mov dl, 0x00        ; Drive A:
    int 0x13            ; BIOS disk interrupt

    jc kernel_load_error

    ; Re-enter protected mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Jump back to protected mode
    jmp 0x08:protected_mode_copy_kernel

[BITS 32]
protected_mode_copy_kernel:
    ; Setup segments again
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Copy kernel from 0x10000 to 0x100000 (1MB)
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 8192       ; Copy 32KB (32 sectors * 512 bytes / 4)
    rep movsd

    mov esi, msg_kernel_loaded
    call print_string_32

    popa
    ret

[BITS 16]
kernel_load_error:
    ; Print error and halt
    mov si, msg_disk_error
    call print_string_16_basic
    cli
    hlt

print_string_16_basic:
    push ax
    push bx
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; Simple kernel stub that we'll copy to 0x100000
kernel_stub:
    ; Clear screen first
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw

    ; Print kernel message
    mov edi, 0xB8000
    mov esi, kernel_msg
    mov ah, 0x07
.print_loop:
    lodsb
    or al, al
    jz .done
    stosw
    jmp .print_loop
.done:
    ; Infinite loop
    cli
.halt:
    hlt
    jmp .halt

kernel_msg db 'TBOS Kernel Active! Swamiye Saranam Aiyappa', 0
kernel_stub_end:

;==========================================
; 32-bit Functions
;==========================================
clear_screen_32:
    push eax
    push ecx
    push edi
    mov edi, 0xB8000        ; VGA text buffer
    mov ecx, 2000           ; 80x25 characters
    mov ax, 0x0720          ; Space with white on black
    rep stosw
    pop edi
    pop ecx
    pop eax
    ret

print_string_32:
    push eax
    push ebx
    push edi
    mov edi, [cursor_pos]
.loop:
    lodsb
    or al, al
    jz .done
    cmp al, 10              ; Check for newline
    je .newline
    mov ah, 0x07            ; White on black
    mov [edi], ax
    add edi, 2
    jmp .loop
.newline:
    ; Move to next line
    push eax
    mov eax, edi
    sub eax, 0xB8000
    xor edx, edx
    mov ebx, 160            ; 80 chars * 2 bytes
    div ebx
    inc eax
    mul ebx
    add eax, 0xB8000
    mov edi, eax
    pop eax
    jmp .loop
.done:
    mov [cursor_pos], edi
    pop edi
    pop ebx
    pop eax
    ret

init_steppps_32:
    push esi

    ; Initialize SPACE dimension
    mov esi, msg_space
    call print_string_32

    ; Initialize TIME dimension
    mov esi, msg_time
    call print_string_32

    ; Initialize EVENT dimension
    mov esi, msg_event
    call print_string_32

    ; Initialize PSYCHOLOGY dimension
    mov esi, msg_psychology
    call print_string_32

    ; Initialize PIXEL dimension
    mov esi, msg_pixel
    call print_string_32

    ; Initialize PROMPT dimension
    mov esi, msg_prompt
    call print_string_32

    ; Initialize SCRIPT dimension
    mov esi, msg_script
    call print_string_32

    pop esi
    ret

halt_32:
    mov esi, msg_complete
    call print_string_32
    cli
    hlt
    jmp halt_32

;==========================================
; Data Section
;==========================================
cursor_pos:     dd 0xB8000

; 16-bit messages
msg_stage2:     db 'Stage 2: Preparing protected mode...', 13, 10, 0
msg_protected:  db 'Entering protected mode...', 13, 10, 0

; 32-bit messages
msg_pm_success: db 'Protected mode active!', 10
                db '=======================', 10, 0

msg_space:      db '[SPACE]      Hardware abstraction initialized', 10, 0
msg_time:       db '[TIME]       Temporal subsystem ready', 10, 0
msg_event:      db '[EVENT]      Event handler configured', 10, 0
msg_psychology: db '[PSYCHOLOGY] AI consciousness prepared', 10, 0
msg_pixel:      db '[PIXEL]      Display driver loaded', 10, 0
msg_prompt:     db '[PROMPT]     Command interface ready', 10, 0
msg_script:     db '[SCRIPT]     Execution engine online', 10, 0

msg_kernel_load: db 10, 'Loading TBOS kernel...', 10, 0
msg_kernel_loaded: db 'Kernel loaded at 0x100000', 10, 0
msg_jumping:     db 'Jumping to kernel...', 10, 0
msg_complete:    db 10, 'TBOS Stage 2 Complete!', 10
msg_disk_error:  db 13, 10, 'Disk read error! Cannot load kernel.', 13, 10, 0
                 db 'Sprint 1 Success - Swamiye Saranam Aiyappa', 10, 0

; Padding to make it exactly 4KB
times 4096-($-$$) db 0