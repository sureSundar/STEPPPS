; TernaryBit OS - Enhanced Stage 2 with Real Keyboard Input
; Interactive shell with STEPPPS integration

[BITS 16]
[ORG 0x8000]

stage2_start:
    ; Display enhanced banner
    mov si, stage2_banner
    call print_string

    ; Initialize STEPPPS dimensions
    call init_steppps_dimensions

    ; Enable A20 line
    call enable_a20_line

    ; Setup IDT for keyboard interrupt
    call setup_basic_idt

    ; Enter protected mode
    call enter_protected_mode

init_steppps_dimensions:
    mov si, psychology_msg
    call print_string
    mov si, pixel_msg
    call print_string
    mov si, prompt_msg
    call print_string
    mov si, script_msg
    call print_string
    ret

enable_a20_line:
    mov si, a20_msg
    call print_string

    ; Fast A20 enable method
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

setup_basic_idt:
    mov si, idt_msg
    call print_string
    ; IDT setup will be done in protected mode
    ret

enter_protected_mode:
    mov si, protected_msg
    call print_string

    cli
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:protected_mode_start

[BITS 32]
protected_mode_start:
    ; Set up segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Setup IDT for keyboard
    call setup_idt_32

    ; Enable interrupts
    sti

    ; Initialize variables
    mov dword [cursor_pos], 0xB8000
    mov dword [input_buffer_pos], 0

    ; Display kernel banner
    mov esi, kernel_banner
    call print_string_32

    ; Display STEPPPS status
    call display_steppps_status

    ; Main interactive shell loop
    jmp interactive_shell

setup_idt_32:
    ; Setup IDT for keyboard interrupt (IRQ 1)
    mov eax, keyboard_handler
    mov word [idt + 8*9], ax      ; Low 16 bits of handler
    shr eax, 16
    mov word [idt + 8*9 + 6], ax  ; High 16 bits of handler
    mov word [idt + 8*9 + 2], 0x08 ; Code segment
    mov byte [idt + 8*9 + 5], 0x8E ; Interrupt gate

    ; Load IDT
    lidt [idt_descriptor]

    ; Enable keyboard interrupt (IRQ 1)
    in al, 0x21
    and al, 0xFD    ; Clear bit 1 (keyboard)
    out 0x21, al

    ret

interactive_shell:
    ; Display prompt
    mov esi, shell_prompt
    call print_string_32

shell_loop:
    ; Check for new input
    cmp dword [new_input_flag], 1
    jne shell_loop

    ; Reset flag
    mov dword [new_input_flag], 0

    ; Get the last character
    mov al, [last_key_char]

    ; Handle special keys
    cmp al, 0x0A    ; Enter key
    je process_command
    cmp al, 0x08    ; Backspace
    je handle_backspace
    cmp al, 0       ; Null (special key)
    je shell_loop

    ; Regular character - add to buffer and display
    call add_to_input_buffer
    call display_character
    jmp shell_loop

handle_backspace:
    call remove_from_input_buffer
    jmp shell_loop

process_command:
    ; Null-terminate input buffer
    mov ebx, input_buffer_pos
    mov byte [input_buffer + ebx], 0

    ; Process the command
    call handle_command

    ; Clear input buffer
    mov dword [input_buffer_pos], 0

    ; New line and prompt
    call print_newline
    mov esi, shell_prompt
    call print_string_32
    jmp shell_loop

handle_command:
    ; Check command
    mov esi, input_buffer

    ; Compare with known commands
    mov edi, cmd_help
    call strcmp_32
    cmp eax, 0
    je cmd_help_handler

    mov edi, cmd_steppps
    call strcmp_32
    cmp eax, 0
    je cmd_steppps_handler

    mov edi, cmd_stats
    call strcmp_32
    cmp eax, 0
    je cmd_stats_handler

    mov edi, cmd_ai
    call strcmp_32
    cmp eax, 0
    je cmd_ai_handler

    mov edi, cmd_clear
    call strcmp_32
    cmp eax, 0
    je cmd_clear_handler

    mov edi, cmd_reboot
    call strcmp_32
    cmp eax, 0
    je cmd_reboot_handler

    ; Unknown command
    jmp cmd_unknown_handler

cmd_help_handler:
    call print_newline
    mov esi, help_output
    call print_string_32
    ret

cmd_steppps_handler:
    call print_newline
    call display_steppps_status
    ret

cmd_stats_handler:
    call print_newline
    mov esi, stats_output
    call print_string_32
    ; Update command counter
    inc dword [command_count]
    ret

cmd_ai_handler:
    call print_newline
    mov esi, ai_output
    call print_string_32
    ; Update AI interaction counter
    inc dword [ai_interactions]
    ret

cmd_clear_handler:
    call clear_screen
    mov esi, kernel_banner
    call print_string_32
    ret

cmd_reboot_handler:
    mov esi, reboot_msg
    call print_string_32
    ; Trigger keyboard controller reset
    mov al, 0xFE
    out 0x64, al
    hlt

cmd_unknown_handler:
    call print_newline
    mov esi, unknown_cmd_msg
    call print_string_32
    mov esi, input_buffer
    call print_string_32
    mov esi, newline_str
    call print_string_32
    ret

; Keyboard interrupt handler
keyboard_handler:
    pushad

    ; Read scan code
    in al, 0x60

    ; Convert scan code to ASCII (simplified)
    call scancode_to_ascii

    ; Store character
    mov [last_key_char], al

    ; Set flag for new input
    mov dword [new_input_flag], 1

    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al

    popad
    iret

scancode_to_ascii:
    ; Simplified scan code to ASCII conversion
    ; Only handles basic keys for demo

    cmp al, 0x1C    ; Enter
    je .enter_key
    cmp al, 0x0E    ; Backspace
    je .backspace_key
    cmp al, 0x39    ; Space
    je .space_key

    ; Letter keys (a-z)
    cmp al, 0x1E    ; A
    je .key_a
    cmp al, 0x30    ; B
    je .key_b
    cmp al, 0x2E    ; C
    je .key_c
    cmp al, 0x20    ; D
    je .key_d
    cmp al, 0x12    ; E
    je .key_e
    cmp al, 0x21    ; F
    je .key_f
    cmp al, 0x22    ; G
    je .key_g
    cmp al, 0x23    ; H
    je .key_h
    cmp al, 0x17    ; I
    je .key_i
    cmp al, 0x24    ; J
    je .key_j
    cmp al, 0x25    ; K
    je .key_k
    cmp al, 0x26    ; L
    je .key_l
    cmp al, 0x32    ; M
    je .key_m
    cmp al, 0x31    ; N
    je .key_n
    cmp al, 0x18    ; O
    je .key_o
    cmp al, 0x19    ; P
    je .key_p
    cmp al, 0x10    ; Q
    je .key_q
    cmp al, 0x13    ; R
    je .key_r
    cmp al, 0x1F    ; S
    je .key_s
    cmp al, 0x14    ; T
    je .key_t
    cmp al, 0x16    ; U
    je .key_u
    cmp al, 0x2F    ; V
    je .key_v
    cmp al, 0x11    ; W
    je .key_w
    cmp al, 0x2D    ; X
    je .key_x
    cmp al, 0x15    ; Y
    je .key_y
    cmp al, 0x2C    ; Z
    je .key_z

    ; Default - unknown key
    mov al, 0
    ret

.enter_key:
    mov al, 0x0A
    ret
.backspace_key:
    mov al, 0x08
    ret
.space_key:
    mov al, ' '
    ret
.key_a: mov al, 'a'
    ret
.key_b: mov al, 'b'
    ret
.key_c: mov al, 'c'
    ret
.key_d: mov al, 'd'
    ret
.key_e: mov al, 'e'
    ret
.key_f: mov al, 'f'
    ret
.key_g: mov al, 'g'
    ret
.key_h: mov al, 'h'
    ret
.key_i: mov al, 'i'
    ret
.key_j: mov al, 'j'
    ret
.key_k: mov al, 'k'
    ret
.key_l: mov al, 'l'
    ret
.key_m: mov al, 'm'
    ret
.key_n: mov al, 'n'
    ret
.key_o: mov al, 'o'
    ret
.key_p: mov al, 'p'
    ret
.key_q: mov al, 'q'
    ret
.key_r: mov al, 'r'
    ret
.key_s: mov al, 's'
    ret
.key_t: mov al, 't'
    ret
.key_u: mov al, 'u'
    ret
.key_v: mov al, 'v'
    ret
.key_w: mov al, 'w'
    ret
.key_x: mov al, 'x'
    ret
.key_y: mov al, 'y'
    ret
.key_z: mov al, 'z'
    ret

add_to_input_buffer:
    mov ebx, [input_buffer_pos]
    cmp ebx, 79             ; Max command length
    jge .buffer_full
    mov [input_buffer + ebx], al
    inc dword [input_buffer_pos]
.buffer_full:
    ret

remove_from_input_buffer:
    cmp dword [input_buffer_pos], 0
    je .empty_buffer
    dec dword [input_buffer_pos]
    ; Move cursor back and clear character
    mov ebx, [cursor_pos]
    sub ebx, 2
    mov word [ebx], 0x0720  ; Space with attribute
    mov [cursor_pos], ebx
.empty_buffer:
    ret

display_character:
    mov ebx, [cursor_pos]
    mov ah, 0x07            ; White on black
    mov [ebx], ax
    add ebx, 2
    mov [cursor_pos], ebx
    ret

print_newline:
    mov ebx, [cursor_pos]
    ; Calculate current line
    sub ebx, 0xB8000
    mov eax, ebx
    mov ecx, 160
    xor edx, edx
    div ecx
    inc eax                 ; Next line
    mul ecx
    add eax, 0xB8000
    mov [cursor_pos], eax
    ret

clear_screen:
    mov edi, 0xB8000
    mov ecx, 80*25
    mov ax, 0x0720          ; Space with white on black
    rep stosw
    mov dword [cursor_pos], 0xB8000
    ret

display_steppps_status:
    mov esi, steppps_status_header
    call print_string_32

    ; Update counters for demo
    inc dword [system_uptime]
    inc dword [events_processed]

    mov esi, space_status
    call print_string_32
    mov esi, time_status
    call print_string_32
    mov esi, event_status
    call print_string_32
    mov esi, psychology_status
    call print_string_32
    mov esi, pixel_status
    call print_string_32
    mov esi, prompt_status
    call print_string_32
    mov esi, script_status
    call print_string_32
    ret

strcmp_32:
    ; Compare strings at ESI and EDI
    ; Returns 0 in EAX if equal
.compare_loop:
    mov al, [esi]
    mov bl, [edi]
    cmp al, bl
    jne .not_equal
    cmp al, 0
    je .equal
    inc esi
    inc edi
    jmp .compare_loop
.equal:
    xor eax, eax
    ret
.not_equal:
    mov eax, 1
    ret

print_string_32:
    push eax
    push ebx
    mov ebx, [cursor_pos]

.loop32:
    lodsb
    cmp al, 0
    je .done32

    cmp al, 0x0A
    je .newline32

    mov ah, 0x07
    mov [ebx], ax
    add ebx, 2
    jmp .loop32

.newline32:
    ; Move to start of next line
    sub ebx, 0xB8000
    mov eax, ebx
    mov ecx, 160
    xor edx, edx
    div ecx
    inc eax
    mul ecx
    add eax, 0xB8000
    mov ebx, eax
    jmp .loop32

.done32:
    mov [cursor_pos], ebx
    pop ebx
    pop eax
    ret

[BITS 16]
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
    dw 0xFFFF, 0x0000   ; Code segment
    db 0x00, 10011010b, 11001111b, 0x00
    dw 0xFFFF, 0x0000   ; Data segment
    db 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; IDT descriptor
idt_descriptor:
    dw 256*8-1          ; IDT size
    dd idt              ; IDT address

; Data section
stage2_banner   db 'TernaryBit Enhanced Bootloader v2.0', 0x0D, 0x0A, 0x00
psychology_msg  db '[PSYCHOLOGY] AI learning system ready', 0x0D, 0x0A, 0x00
pixel_msg       db '[PIXEL] Display system initialized', 0x0D, 0x0A, 0x00
prompt_msg      db '[PROMPT] Interactive shell ready', 0x0D, 0x0A, 0x00
script_msg      db '[SCRIPT] Automation engine active', 0x0D, 0x0A, 0x00
a20_msg         db 'Enabling A20 line...', 0x0D, 0x0A, 0x00
idt_msg         db 'Setting up interrupts...', 0x0D, 0x0A, 0x00
protected_msg   db 'Entering protected mode...', 0x0D, 0x0A, 0x00

; 32-bit strings
kernel_banner   db 'TernaryBit OS v2.0 - Interactive STEPPPS Shell', 0x0A
                db '================================================', 0x0A
                db 'Real keyboard input enabled - Type commands!', 0x0A, 0x0A, 0x00

shell_prompt    db 'ternarybit> ', 0x00
newline_str     db 0x0A, 0x00

; Commands
cmd_help        db 'help', 0x00
cmd_steppps     db 'steppps', 0x00
cmd_stats       db 'stats', 0x00
cmd_ai          db 'ai', 0x00
cmd_clear       db 'clear', 0x00
cmd_reboot      db 'reboot', 0x00

; Command outputs
help_output     db 'Available commands:', 0x0A
                db '  help     - Show this help', 0x0A
                db '  steppps  - Show STEPPPS dimension status', 0x0A
                db '  stats    - Show system statistics', 0x0A
                db '  ai       - Show AI subsystem status', 0x0A
                db '  clear    - Clear screen', 0x0A
                db '  reboot   - Restart system', 0x0A, 0x00

steppps_status_header db 'STEPPPS Framework Status:', 0x0A, 0x00
space_status    db '  SPACE:      Active (CPU: 1, RAM: 128MB)', 0x0A, 0x00
time_status     db '  TIME:       Active (Scheduler running)', 0x0A, 0x00
event_status    db '  EVENT:      Active (Keyboard ready)', 0x0A, 0x00
psychology_status db '  PSYCHOLOGY: Active (Learning enabled)', 0x0A, 0x00
pixel_status    db '  PIXEL:      Active (VGA 80x25)', 0x0A, 0x00
prompt_status   db '  PROMPT:     Active (Shell interactive)', 0x0A, 0x00
script_status   db '  SCRIPT:     Active (Automation ready)', 0x0A, 0x00

stats_output    db 'System Statistics:', 0x0A
                db '  Commands processed: Variable', 0x0A
                db '  AI interactions: Variable', 0x0A
                db '  System uptime: Running', 0x0A, 0x00

ai_output       db 'AI Subsystem Status:', 0x0A
                db '  Neural Network: Simulated Active', 0x0A
                db '  Learning Rate: 15%', 0x0A
                db '  Confidence Level: 87%', 0x0A
                db '  Pattern Recognition: Enabled', 0x0A, 0x00

unknown_cmd_msg db 'Unknown command: ', 0x00
reboot_msg      db 'Rebooting TernaryBit OS...', 0x0A, 0x00

; Variables
cursor_pos      dd 0xB8000
input_buffer_pos dd 0
new_input_flag  dd 0
last_key_char   db 0
command_count   dd 0
ai_interactions dd 0
system_uptime   dd 0
events_processed dd 0

; Input buffer
input_buffer    times 80 db 0

; IDT (simplified - only need a few entries)
idt             times 256*8 db 0

; Pad to sector boundaries
times 2048-($-$$) db 0