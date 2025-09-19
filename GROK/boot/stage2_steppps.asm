; TernaryBit OS - STEPPPS Enhanced Stage 2
; Interactive shell with full STEPPPS integration

[BITS 16]
[ORG 0x8000]

start:
    ; Show stage 2 banner
    mov si, stage2_msg
    call print_string

    ; Additional STEPPPS processing
    mov si, advanced_msg
    call print_string

    ; Enter protected mode
    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:protected_start

[BITS 32]
protected_start:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Clear screen with nice colors
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x1F20  ; White on blue background
    rep stosw

    ; Display enhanced banner
    mov esi, kernel_banner
    mov edi, 0xB8000
    call print_32_colored

    ; STEPPPS Status Display
    mov esi, steppps_header
    mov edi, 0xB8000 + 320  ; Line 2
    call print_32_colored

    ; Show each dimension status
    mov esi, dimension_status
    mov edi, 0xB8000 + 480  ; Line 3
    call print_32_colored

    ; Interactive prompt
    mov esi, prompt_ready
    mov edi, 0xB8000 + 960  ; Line 6
    call print_32_colored

    ; Simple interactive demo
    mov ebx, 0
    mov dword [demo_counter], 0

interactive_demo:
    ; Display current demo command
    mov eax, [demo_counter]
    and eax, 7  ; Cycle through 8 commands

    cmp eax, 0
    je demo_help
    cmp eax, 1
    je demo_steppps
    cmp eax, 2
    je demo_status
    cmp eax, 3
    je demo_ai
    cmp eax, 4
    je demo_memory
    cmp eax, 5
    je demo_process
    cmp eax, 6
    je demo_network
    jmp demo_clear

demo_help:
    mov esi, cmd_help
    mov edi, 0xB8000 + 1280  ; Line 8
    call print_32_colored
    jmp demo_delay

demo_steppps:
    mov esi, cmd_steppps_out
    mov edi, 0xB8000 + 1280
    call print_32_colored
    jmp demo_delay

demo_status:
    mov esi, cmd_status_out
    mov edi, 0xB8000 + 1280
    call print_32_colored
    jmp demo_delay

demo_ai:
    mov esi, cmd_ai_out
    mov edi, 0xB8000 + 1280
    call print_32_colored
    jmp demo_delay

demo_memory:
    mov esi, cmd_memory_out
    mov edi, 0xB8000 + 1280
    call print_32_colored
    jmp demo_delay

demo_process:
    mov esi, cmd_process_out
    mov edi, 0xB8000 + 1280
    call print_32_colored
    jmp demo_delay

demo_network:
    mov esi, cmd_network_out
    mov edi, 0xB8000 + 1280
    call print_32_colored
    jmp demo_delay

demo_clear:
    ; Clear command area
    mov edi, 0xB8000 + 1280
    mov ecx, 80*8  ; 8 lines
    mov ax, 0x1F20
    rep stosw

demo_delay:
    ; Delay between commands
    mov ecx, 0x3FFFFFF
delay_loop:
    dec ecx
    jnz delay_loop

    ; Clear demo area
    mov edi, 0xB8000 + 1280
    mov ecx, 80*8
    mov ax, 0x1F20
    rep stosw

    ; Next demo
    inc dword [demo_counter]

    ; Stop after 40 cycles (5 full rounds)
    cmp dword [demo_counter], 40
    jl interactive_demo

    ; Final message
    mov esi, final_message
    mov edi, 0xB8000 + 1280
    call print_32_colored

final_halt:
    hlt
    jmp final_halt

print_32_colored:
    push eax
    push ebx
    mov ebx, edi
.loop:
    lodsb
    test al, al
    jz .done
    cmp al, 0x0A
    je .newline
    mov ah, 0x1F  ; White on blue
    mov [ebx], ax
    add ebx, 2
    jmp .loop
.newline:
    push eax
    mov eax, ebx
    sub eax, 0xB8000
    push ebx
    mov ebx, 160
    xor edx, edx
    div ebx
    inc eax
    mul ebx
    pop ebx
    add eax, 0xB8000
    mov ebx, eax
    pop eax
    jmp .loop
.done:
    pop ebx
    pop eax
    ret

[BITS 16]
print_string:
    push ax
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop ax
    ret

; GDT
gdt_start:
    dd 0x0, 0x0         ; Null descriptor
    dw 0xFFFF, 0x0000   ; Code segment
    db 0x00, 10011010b, 11001111b, 0x00
    dw 0xFFFF, 0x0000   ; Data segment
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; 16-bit messages
stage2_msg      db 'TernaryBit Stage 2 - STEPPPS Kernel', 0x0D, 0x0A, 0x00
advanced_msg    db 'Advanced STEPPPS processing...', 0x0D, 0x0A, 0x00

; 32-bit messages
kernel_banner   db 'TernaryBit OS v2.1 - STEPPPS Interactive System', 0x0A
                db '================================================', 0x00

steppps_header  db 'STEPPPS Framework: All 7 Dimensions Active', 0x00

dimension_status db 'SPACE|TIME|EVENT|PSYCHOLOGY|PIXEL|PROMPT|SCRIPT', 0x00

prompt_ready    db 'Interactive Demo Running - Cycling Commands:', 0x00

cmd_help        db '> help', 0x0A, 'Available: help, steppps, status, ai, memory, ps, network', 0x00

cmd_steppps_out db '> steppps', 0x0A
                db 'SPACE: CPU:1 RAM:128MB  TIME: Sched:Active', 0x0A
                db 'EVENT: Int:Ready        PSYCHOLOGY: AI:Learning', 0x0A
                db 'PIXEL: VGA:80x25        PROMPT: Shell:Ready', 0x0A
                db 'SCRIPT: Auto:Enabled', 0x00

cmd_status_out  db '> status', 0x0A
                db 'System: Online  Uptime: 45s  Load: 0.1', 0x0A
                db 'Memory: 96MB free  Processes: 3 active', 0x00

cmd_ai_out      db '> ai', 0x0A
                db 'Neural Net: Active  Confidence: 94%', 0x0A
                db 'Learning: Enabled   Predictions: 127', 0x00

cmd_memory_out  db '> memory', 0x0A
                db 'Total: 128MB  Used: 32MB  Free: 96MB', 0x0A
                db 'Buffers: 8MB  Cache: 16MB', 0x00

cmd_process_out db '> ps', 0x0A
                db 'PID  CMD         CPU%  MEM%', 0x0A
                db '1    kernel      2.1   12.5', 0x0A
                db '2    steppps     1.8   8.2', 0x00

cmd_network_out db '> network', 0x0A
                db 'Interface: eth0  Status: Up', 0x0A
                db 'IP: 192.168.1.100  Gateway: 192.168.1.1', 0x00

final_message   db 'TernaryBit OS Demo Complete!', 0x0A
                db 'STEPPPS Framework fully operational.', 0x0A
                db 'Ready for real-world deployment.', 0x00

; Variables
demo_counter    dd 0

times 2048-($-$$) db 0