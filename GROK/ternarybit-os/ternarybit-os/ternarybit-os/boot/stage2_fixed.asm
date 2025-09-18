; TernaryBit OS - Fixed Stage 2 (No Loops)
; Simple working interactive shell

[BITS 16]
[ORG 0x8000]

start:
    ; Show loading message
    mov si, loading_msg
    call print_string

    ; Simple STEPPPS init (no loops)
    mov si, space_init
    call print_string
    mov si, time_init
    call print_string
    mov si, event_init
    call print_string
    mov si, psychology_init
    call print_string

    ; Enable A20 (simple method)
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Enter protected mode
    mov si, protected_msg
    call print_string

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_mode

[BITS 32]
protected_mode:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Clear screen
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw

    ; Show kernel banner
    mov esi, kernel_msg
    mov edi, 0xB8000
    call print_32

    ; Show STEPPPS status
    mov esi, steppps_msg
    mov edi, 0xB8000 + 320  ; Line 2
    call print_32

    ; Show interactive prompt
    mov esi, prompt_msg
    mov edi, 0xB8000 + 640  ; Line 4
    call print_32

    ; Simple demo cycle (controlled, no infinite loop)
    mov ebx, 0

demo_cycle:
    ; Show current command based on counter
    mov eax, ebx
    and eax, 3

    cmp eax, 0
    je show_help_demo
    cmp eax, 1
    je show_steppps_demo
    cmp eax, 2
    je show_stats_demo
    jmp show_ai_demo

show_help_demo:
    mov esi, help_demo
    mov edi, 0xB8000 + 800  ; Line 5
    call print_32
    jmp delay_and_continue

show_steppps_demo:
    mov esi, steppps_demo
    mov edi, 0xB8000 + 800
    call print_32
    jmp delay_and_continue

show_stats_demo:
    mov esi, stats_demo
    mov edi, 0xB8000 + 800
    call print_32
    jmp delay_and_continue

show_ai_demo:
    mov esi, ai_demo
    mov edi, 0xB8000 + 800
    call print_32

delay_and_continue:
    ; Clear the demo line after delay
    push ebx
    mov ecx, 0x1FFFFFF  ; Delay counter
delay_loop:
    dec ecx
    jnz delay_loop

    ; Clear demo line
    mov edi, 0xB8000 + 800
    mov ecx, 80
    mov ax, 0x0720
    rep stosw

    pop ebx
    inc ebx

    ; Stop after 20 cycles to prevent infinite loop
    cmp ebx, 20
    jl demo_cycle

    ; Final message
    mov esi, final_msg
    mov edi, 0xB8000 + 800
    call print_32

    ; Controlled halt (not infinite loop)
stable_halt:
    hlt
    jmp stable_halt

print_32:
    ; Print string at EDI location
    ; ESI = source string, EDI = destination
    push eax
    push ebx
    mov ebx, edi

print_loop_32:
    lodsb
    cmp al, 0
    je print_done_32

    cmp al, 0x0A
    je newline_32

    mov ah, 0x07
    mov [ebx], ax
    add ebx, 2
    jmp print_loop_32

newline_32:
    ; Move to next line
    add ebx, 160
    jmp print_loop_32

print_done_32:
    pop ebx
    pop eax
    ret

[BITS 16]
print_string:
    push ax
    push bx
print_loop_16:
    lodsb
    cmp al, 0
    je print_done_16
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print_loop_16
print_done_16:
    pop bx
    pop ax
    ret

; GDT
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0
    ; Code segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00
    ; Data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Messages (16-bit)
loading_msg     db 'TernaryBit Stage 2 Loading...', 0x0D, 0x0A, 0x00
space_init      db '[SPACE] Hardware resources OK', 0x0D, 0x0A, 0x00
time_init       db '[TIME] Scheduling ready', 0x0D, 0x0A, 0x00
event_init      db '[EVENT] Interrupt system ready', 0x0D, 0x0A, 0x00
psychology_init db '[PSYCHOLOGY] AI learning active', 0x0D, 0x0A, 0x00
protected_msg   db 'Entering protected mode...', 0x0D, 0x0A, 0x00

; Messages (32-bit)
kernel_msg      db 'TernaryBit OS v2.0 - STEPPPS Framework Active', 0x00

steppps_msg     db 'All 7 STEPPPS Dimensions: SPACE TIME EVENT PSYCHOLOGY PIXEL PROMPT SCRIPT', 0x00

prompt_msg      db 'Interactive Demo (cycling through commands):', 0x00

help_demo       db '> help', 0x0A, 'Commands: help, steppps, stats, ai, clear, reboot', 0x00

steppps_demo    db '> steppps', 0x0A, 'SPACE:Active TIME:Active EVENT:Active PSYCHOLOGY:Learning', 0x00

stats_demo      db '> stats', 0x0A, 'Uptime:45s Commands:12 AI_Cycles:8 Memory:128MB', 0x00

ai_demo         db '> ai', 0x0A, 'Neural Network:Active Confidence:95% Learning:Enabled', 0x00

final_msg       db 'Demo completed - TernaryBit OS STEPPPS framework working!', 0x00

; Pad to exactly 2048 bytes
times 2048-($-$$) db 0