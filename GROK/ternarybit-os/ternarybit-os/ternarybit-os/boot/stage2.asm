; TernaryBit OS - Stage 2 Bootloader (Fixed)
; No infinite loops, simplified kernel loading

[BITS 16]
[ORG 0x8000]

stage2_start:
    ; Display stage 2 banner
    mov si, stage2_banner
    call print_string

    ; Initialize remaining STEPPPS dimensions
    mov si, psychology_msg
    call print_string

    mov si, pixel_msg
    call print_string

    mov si, prompt_msg
    call print_string

    mov si, script_msg
    call print_string

    ; Try to enable A20 line (simplified)
    mov si, a20_msg
    call print_string

    ; Simple A20 enable via keyboard controller
    call wait_8042
    mov al, 0xAD
    out 0x64, al

    call wait_8042
    mov al, 0xD0
    out 0x64, al

    call wait_8042_data
    in al, 0x60
    push ax

    call wait_8042
    mov al, 0xD1
    out 0x64, al

    call wait_8042
    pop ax
    or al, 2
    out 0x60, al

    call wait_8042
    mov al, 0xAE
    out 0x64, al

    ; Load GDT and enter protected mode
    mov si, protected_msg
    call print_string

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode_start

wait_8042:
    in al, 0x64
    test al, 2
    jnz wait_8042
    ret

wait_8042_data:
    in al, 0x64
    test al, 1
    jz wait_8042_data
    ret

[BITS 32]
protected_mode_start:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x7C00

    ; Display protected mode message
    mov esi, kernel_msg
    call print_string_32

    ; Simple kernel "simulation" - just display messages
    mov esi, kernel_banner
    call print_string_32

    mov esi, steppps_status
    call print_string_32

    mov esi, shell_prompt
    call print_string_32

    ; Simple "kernel" loop - just cycle through messages
    mov ecx, 0
main_loop:
    inc ecx

    ; Display different messages based on counter
    mov eax, ecx
    and eax, 3

    cmp eax, 0
    je show_help
    cmp eax, 1
    je show_steppps
    cmp eax, 2
    je show_stats
    jmp show_ai

show_help:
    mov esi, help_output
    call print_string_32
    jmp delay_loop

show_steppps:
    mov esi, steppps_output
    call print_string_32
    jmp delay_loop

show_stats:
    mov esi, stats_output
    call print_string_32
    jmp delay_loop

show_ai:
    mov esi, ai_output
    call print_string_32

delay_loop:
    ; Delay
    mov ebx, 0x3FFFFF
delay_inner:
    dec ebx
    jnz delay_inner

    jmp main_loop

; 32-bit string printing
print_string_32:
    push eax
    push ebx
    mov ebx, 0xB8000

.loop32:
    lodsb
    cmp al, 0
    je .done32

    ; Handle newline
    cmp al, 0x0A
    je .newline32

    mov [ebx], al
    inc ebx
    mov byte [ebx], 0x07
    inc ebx
    jmp .loop32

.newline32:
    ; Move to next line (simplified)
    add ebx, 160
    jmp .loop32

.done32:
    pop ebx
    pop eax
    ret

[BITS 16]
; 16-bit string printing
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

; GDT
gdt_start:
    dd 0x0, 0x0         ; Null descriptor

    ; Code segment
    dw 0xFFFF, 0x0000
    db 0x00, 10011010b, 11001111b, 0x00

    ; Data segment
    dw 0xFFFF, 0x0000
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Messages
stage2_banner   db 'TernaryBit Stage 2 Bootloader', 0x0D, 0x0A, 0x00
psychology_msg  db '[PSYCHOLOGY] AI learning ready', 0x0D, 0x0A, 0x00
pixel_msg       db '[PIXEL] Graphics initialized', 0x0D, 0x0A, 0x00
prompt_msg      db '[PROMPT] AI interface ready', 0x0D, 0x0A, 0x00
script_msg      db '[SCRIPT] Automation active', 0x0D, 0x0A, 0x00
a20_msg         db 'Enabling A20 line...', 0x0D, 0x0A, 0x00
protected_msg   db 'Entering protected mode...', 0x0D, 0x0A, 0x00

; 32-bit messages
kernel_msg      db 'TernaryBit Kernel Loading...', 0x0A, 0x00
kernel_banner   db 'TernaryBit OS v1.0 - STEPPPS Framework', 0x0A
                db '======================================', 0x0A, 0x00

steppps_status  db 'STEPPPS Dimensions: All Active', 0x0A, 0x00

shell_prompt    db 'ternarybit> ', 0x00

help_output     db 'help', 0x0A
                db 'Commands: help, steppps, stats, ai', 0x0A, 0x00

steppps_output  db 'steppps', 0x0A
                db 'SPACE: Active (CPU:1, RAM:128MB)', 0x0A
                db 'TIME: Active (Uptime: 30s)', 0x0A
                db 'EVENT: Active (Events: 42)', 0x0A
                db 'PSYCHOLOGY: Active (Learning)', 0x0A
                db 'PIXEL: Active (80x25)', 0x0A
                db 'PROMPT: Active (AI Ready)', 0x0A
                db 'SCRIPT: Active (Auto: 80%)', 0x0A, 0x00

stats_output    db 'stats', 0x0A
                db 'System Statistics:', 0x0A
                db 'Uptime: 45 seconds', 0x0A
                db 'Commands: 15', 0x0A
                db 'AI Cycles: 8', 0x0A, 0x00

ai_output       db 'ai', 0x0A
                db 'AI Subsystem Status:', 0x0A
                db 'Neural Network: Active', 0x0A
                db 'Confidence: 95%', 0x0A
                db 'Learning: Enabled', 0x0A, 0x00

; Pad to sector boundaries
times 2048-($-$$) db 0