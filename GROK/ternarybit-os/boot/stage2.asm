; TernaryBit OS - Stage 2 Bootloader
; Advanced STEPPPS initialization and kernel loading
; Loaded at 0x8000

[BITS 16]
[ORG 0x8000]

stage2_start:
    ; Display stage 2 banner
    mov si, stage2_banner
    call print_string

    ; Advanced STEPPPS initialization
    call init_steppps_psychology   ; PSYCHOLOGY: Learning/adaptation
    call init_steppps_pixel_adv     ; PIXEL: Advanced graphics
    call init_steppps_prompt_adv    ; PROMPT: AI interface
    call init_steppps_script_adv    ; SCRIPT: Automation

    ; Enable A20 line (for >1MB memory access)
    call enable_a20

    ; Enter protected mode
    call enter_protected_mode

; Advanced STEPPPS initializers
init_steppps_psychology:
    ; PSYCHOLOGY: Basic learning and adaptation
    push ax
    push si

    mov si, psychology_msg
    call print_string

    ; Initialize adaptation counters
    mov word [boot_count], 1
    mov word [learning_rate], 100   ; 100% learning rate initially

    ; Basic "psychology" - remember boot preferences
    ; (In real implementation, this would read from disk)
    mov byte [user_preference], 0   ; Default mode

    pop si
    pop ax
    ret

init_steppps_pixel_adv:
    ; PIXEL: Advanced graphics initialization
    push ax
    push si

    mov si, pixel_msg
    call print_string

    ; Try to set VESA mode (VGA fallback)
    mov ax, 0x4F02      ; VESA set mode
    mov bx, 0x0117      ; 1024x768x16
    int 0x10

    cmp ah, 0           ; Check if successful
    je vesa_success

    ; Fallback to standard VGA
    mov ax, 0x0012      ; VGA 640x480x16
    int 0x10

vesa_success:
    ; Store graphics mode info
    mov byte [graphics_mode], 1

    pop si
    pop ax
    ret

init_steppps_prompt_adv:
    ; PROMPT: Advanced AI interface preparation
    push ax
    push si

    mov si, prompt_msg
    call print_string

    ; Initialize command buffer
    mov di, command_buffer
    mov cx, 256
    xor al, al
    rep stosb           ; Clear command buffer

    ; Set up basic command recognition patterns
    mov word [command_count], 0

    pop si
    pop ax
    ret

init_steppps_script_adv:
    ; SCRIPT: Automation and orchestration
    push ax
    push si

    mov si, script_msg
    call print_string

    ; Initialize script execution environment
    mov word [script_stack], 0x7000  ; Script execution stack
    mov byte [auto_mode], 1          ; Enable automation

    pop si
    pop ax
    ret

enable_a20:
    ; Enable A20 line through keyboard controller
    push ax

    call wait_8042_command
    mov al, 0xAD        ; Disable first PS/2 port
    out 0x64, al

    call wait_8042_command
    mov al, 0xD0        ; Read from input
    out 0x64, al

    call wait_8042_data
    in al, 0x60
    push ax

    call wait_8042_command
    mov al, 0xD1        ; Write to output
    out 0x64, al

    call wait_8042_command
    pop ax
    or al, 2            ; Enable A20
    out 0x60, al

    call wait_8042_command
    mov al, 0xAE        ; Enable first PS/2 port
    out 0x64, al

    pop ax
    ret

wait_8042_command:
    in al, 0x64
    test al, 2
    jnz wait_8042_command
    ret

wait_8042_data:
    in al, 0x64
    test al, 1
    jz wait_8042_data
    ret

enter_protected_mode:
    ; Load GDT
    cli                 ; Disable interrupts
    lgdt [gdt_descriptor]

    ; Enter protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to clear pipeline
    jmp 0x08:protected_mode_start

[BITS 32]
protected_mode_start:
    ; Set up protected mode segments
    mov ax, 0x10        ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack
    mov esp, 0x7C00     ; Stack below boot sector

    ; Display protected mode message
    mov esi, protected_msg
    call print_string_32

    ; Load and jump to kernel
    call load_kernel

    ; Should never reach here
    hlt

load_kernel:
    ; Load TernaryBit kernel from disk
    ; (Simplified - in real implementation would use proper file system)

    ; Display loading message
    mov esi, kernel_loading_msg
    call print_string_32

    ; Simulate kernel loading
    mov ecx, 1000000    ; Delay counter
delay_loop:
    dec ecx
    jnz delay_loop

    ; Jump to kernel (loaded at 0x10000)
    jmp 0x08:0x10000

; 32-bit string printing function
print_string_32:
    push eax
    push ebx
    mov ebx, 0xB8000    ; VGA text buffer

print_loop_32:
    lodsb
    cmp al, 0
    je print_done_32

    mov [ebx], al
    inc ebx
    mov byte [ebx], 0x07    ; Light gray on black
    inc ebx

    jmp print_loop_32

print_done_32:
    pop ebx
    pop eax
    ret

[BITS 16]
; 16-bit string printing (reused from stage 1)
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

; Global Descriptor Table
gdt_start:
    ; Null descriptor
    dd 0x0
    dd 0x0

    ; Code segment descriptor (0x08)
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (middle)
    db 10011010b    ; Access byte
    db 11001111b    ; Flags, limit (high)
    db 0x00         ; Base (high)

    ; Data segment descriptor (0x10)
    dw 0xFFFF       ; Limit (low)
    dw 0x0000       ; Base (low)
    db 0x00         ; Base (middle)
    db 10010010b    ; Access byte
    db 11001111b    ; Flags, limit (high)
    db 0x00         ; Base (high)

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size
    dd gdt_start                ; GDT address

; Messages
stage2_banner   db 'TernaryBit Stage 2 - Advanced STEPPPS Init', 0x0D, 0x0A
                db '==========================================', 0x0D, 0x0A, 0x00

psychology_msg  db '[PSYCHOLOGY] Learning system init...', 0x0D, 0x0A, 0x00
pixel_msg       db '[PIXEL] Graphics subsystem init...', 0x0D, 0x0A, 0x00
prompt_msg      db '[PROMPT] AI interface preparation...', 0x0D, 0x0A, 0x00
script_msg      db '[SCRIPT] Automation engine init...', 0x0D, 0x0A, 0x00

protected_msg   db 'Protected mode enabled - STEPPPS core active', 0x00
kernel_loading_msg db 'Loading TernaryBit kernel...', 0x00

; STEPPPS state variables
boot_count      dw 0
learning_rate   dw 0
user_preference db 0
graphics_mode   db 0
command_buffer  times 256 db 0
command_count   dw 0
script_stack    dw 0
auto_mode       db 0

; Pad to sector boundary
times 2048-($-$$) db 0  ; 4 sectors total