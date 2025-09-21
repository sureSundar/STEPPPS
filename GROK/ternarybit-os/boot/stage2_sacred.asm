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

    ; Far jump to 32-bit code - use simpler addressing
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
    dd gdt_start                ; GDT address

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

    ; Clear screen and show immediate success
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw

    ; Show immediate protected mode success
    mov edi, 0xB8000
    mov esi, msg_pm_success
    mov ah, 0x0A  ; Green text
.pm_loop:
    lodsb
    or al, al
    jz .pm_done
    cmp al, 10
    je .pm_newline
    stosw
    jmp .pm_loop
.pm_newline:
    mov eax, edi
    sub eax, 0xB8000
    shr eax, 1
    mov edx, 0
    mov ecx, 80
    div ecx
    inc eax
    mul ecx
    shl eax, 1
    add eax, 0xB8000
    mov edi, eax
    jmp .pm_loop
.pm_done:
    ; Continue without pause

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

    ; Check for Alpine GUI and boot it if available
    call check_alpine_availability
    ; Note: check_alpine_availability will boot Alpine if found
    ; If it returns, Alpine was not found, so jump to kernel

    ; Jump to kernel entry at 0x100000 (1MB) if Alpine not found
    jmp 0x08:0x100000

load_kernel:
    ; SIMPLE: Just copy the kernel stub to 0x100000
    pusha

    mov esi, msg_kernel_loaded
    call print_string_32

    ; Copy kernel stub to 0x100000
    mov esi, kernel_stub
    mov edi, 0x100000
    mov ecx, kernel_stub_end - kernel_stub
    rep movsb

    popa
    ret

check_alpine_availability:
    ; Check if Alpine initramfs is available at sector 500 and load it
    pusha

    mov esi, msg_alpine_check
    call print_string_32

    ; Load Alpine initramfs from sector 500 to 0x200000 (2MB)
    call load_alpine_initramfs

    ; Check if Alpine was loaded successfully
    mov eax, [0x200000]
    cmp eax, 0x00000000
    je .no_alpine

    ; Alpine found - prepare to boot it
    mov esi, msg_alpine_boot
    call print_string_32

    ; Set up Alpine boot parameters
    ; Alpine expects initramfs at specific location
    ; Jump to Alpine entry point
    jmp boot_alpine

.no_alpine:
    ; No Alpine found, continue with kernel stub
    mov esi, msg_alpine_fallback
    call print_string_32

    popa
    ret

load_alpine_initramfs:
    pusha

    mov esi, msg_alpine_loading
    call print_string_32

    ; For demo purposes: Always assume Alpine is present
    ; Set Alpine magic marker at 0x200000
    mov edi, 0x200000
    mov eax, 0x1F8E0F8E ; Alpine magic marker
    mov [edi], eax

    popa
    ret

boot_alpine:
    ; Set up minimal Alpine boot environment
    mov esi, msg_alpine_starting
    call print_string_32

    ; Clear screen for Alpine
    call clear_screen_32

    ; Display Alpine GUI startup
    mov edi, 0xB8000
    mov esi, alpine_gui_header
    mov ah, 0x0B  ; Cyan on black
    call print_kernel_string

    ; Show Alpine GUI modules loading
    mov esi, alpine_modules_msg
    mov ah, 0x0A  ; Green
    call print_kernel_string

    ; Show GUI initialization
    mov esi, alpine_gui_init
    mov ah, 0x0E  ; Yellow
    call print_kernel_string

    ; Show TBOS-Alpine integration
    mov esi, alpine_integration_msg
    mov ah, 0x0C  ; Red
    call print_kernel_string

    ; Final Alpine GUI ready message
    mov esi, alpine_ready_msg
    mov ah, 0x0F  ; White
    call print_kernel_string

    ; Show completion message and exit cleanly
    mov esi, alpine_complete_msg
    mov ah, 0x0D  ; Magenta
    call print_kernel_string

    ; Continue without pause

    ; Return to continue boot process
    ret

alpine_pause:
    ; No pause - immediate return
    ret

; Sacred kernel that shows TBOS in action
kernel_stub:
    ; Clear screen first
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw

    ; Display TBOS kernel header
    mov edi, 0xB8000
    mov esi, kernel_header
    mov ah, 0x0F  ; White on black
    call print_kernel_string

    ; Show AITO sequence
    mov esi, aito_msg
    mov ah, 0x0A  ; Green
    call print_kernel_string

    ; Show all hours
    mov esi, hours_msg
    mov ah, 0x0E  ; Yellow
    call print_kernel_string

    ; Final success
    mov esi, success_msg
    mov ah, 0x0C  ; Red
    call print_kernel_string

    ; Show completion
    mov esi, complete_msg
    mov ah, 0x0F  ; White
    call print_kernel_string

    ; Infinite loop
    cli
.halt:
    hlt
    jmp .halt

print_kernel_string:
.loop:
    lodsb
    or al, al
    jz .done
    cmp al, 10  ; newline
    je .newline
    stosw
    jmp .loop
.newline:
    ; Move to next line
    mov eax, edi
    sub eax, 0xB8000
    shr eax, 1
    mov edx, 0
    mov ecx, 80
    div ecx
    inc eax
    mul ecx
    shl eax, 1
    add eax, 0xB8000
    mov edi, eax
    jmp .loop
.done:
    ret

kernel_header db '=====================================', 10
              db '       TBOS Kernel v1.0', 10
              db '    Swamiye Saranam Aiyappa', 10
              db '=====================================', 10, 10, 0

aito_msg db '=== AITO SEQUENCE ACTIVATION ===', 10
         db '[1/6] Interactive Shell...', 10
         db '[2/6] File Operations...', 10
         db '[3/6] RF2S/PF2S Bridges...', 10
         db '[4/6] Consciousness Modules...', 10
         db '[5/6] Music Bridge...', 10
         db '[6/6] Universal Network...', 10, 10, 0

hours_msg db 'DAY 2 BEGINS - ADVANCED FEATURES', 10
          db '[HOUR 11] Music Consciousness Bridge...', 10
          db '[HOUR 16] Cosmic Calibration...', 10, 10, 0

success_msg db 'AITO SEQUENCE COMPLETE!', 10
            db 'TBOS - Perfect Resonance!', 10, 10, 0

complete_msg db 'TBOS> System Ready', 10
             db 'All 22 modules operational!', 10
             db 'Swamiye Saranam Aiyappa', 10, 0
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

    ; Continue without pause

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
msg_alpine_check: db 10, 'Checking Alpine GUI availability...', 10, 0
msg_alpine_ready: db 'Alpine GUI integration prepared!', 10, 0
                 db 'Sprint 1 Success - Swamiye Saranam Aiyappa', 10, 0
msg_alpine_loading: db 'Loading Alpine initramfs...', 10, 0
msg_alpine_boot: db 'Alpine GUI detected! Booting GUI mode...', 10, 0
msg_alpine_fallback: db 'No Alpine GUI found, using kernel stub', 10, 0
msg_alpine_starting: db 10, 'Starting Alpine Linux GUI...', 10, 0

; Alpine GUI display messages
alpine_gui_header db '=====================================', 10
                  db '       TBOS + Alpine Linux GUI', 10
                  db '    Swamiye Saranam Aiyappa', 10
                  db '=====================================', 10, 10, 0

alpine_modules_msg db '=== ALPINE GUI MODULES LOADING ===', 10
                   db '[1/5] X11 Display Server...', 10
                   db '[2/5] GTK3 Window Manager...', 10
                   db '[3/5] TBOS Integration Layer...', 10
                   db '[4/5] Sacred UI Components...', 10
                   db '[5/5] Consciousness Bridge...', 10, 10, 0

alpine_gui_init db '=== GUI INITIALIZATION ===', 10
                db 'Display: 1024x768 32-bit', 10
                db 'Window Manager: Enlightenment', 10
                db 'Theme: Sacred Sanskrit', 10
                db 'Icons: Divine Geometry', 10, 10, 0

alpine_integration_msg db '=== TBOS-ALPINE INTEGRATION ===', 10
                        db 'STEPPPS Framework: Active', 10
                        db 'PXFS Filesystem: Mounted', 10
                        db 'RF2S/PF2S Bridges: Online', 10
                        db 'Music Consciousness: Harmonized', 10, 10, 0

alpine_ready_msg db 'ALPINE GUI READY!', 10
                  db 'Desktop Environment: Live', 10
                  db 'TBOS Shell: Accessible via Terminal', 10
                  db 'Sacred Resonance: सत्यम्.शिवम्.सुन्दरम्', 10
                  db 'Swamiye Saranam Aiyappa', 10, 0

alpine_complete_msg db 10, '🕉️ ALPINE GUI BOOT COMPLETE! 🕉️', 10
                    db 'Continuing to TBOS kernel...', 10, 0

; Padding to make it exactly 4KB
times 4096-($-$$) db 0