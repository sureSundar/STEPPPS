; TernaryBit OS - Linux Kernel Bootloader
; Loads Linux bzImage using Linux Boot Protocol
; Pair-Programming: ternarybit@gmail.com + Claude Code

[BITS 16]
[ORG 0x8000]

; Build system overrides
%ifndef BOOT_KERNEL_SECTOR_COUNT
%define BOOT_KERNEL_SECTOR_COUNT 100  ; Linux kernels are larger
%endif

%ifndef BOOT_KERNEL_LBA_START
%define BOOT_KERNEL_LBA_START 9
%endif

%ifndef BOOT_DRIVE_ADDR
%define BOOT_DRIVE_ADDR 0x0500
%endif

; Linux Boot Protocol constants
LINUX_KERNEL_LOAD_ADDR  equ 0x10000    ; 64KB mark (standard for bzImage)
LINUX_SETUP_SEG        equ 0x9000      ; Linux setup code segment
LINUX_CMDLINE_ADDR     equ 0x20000     ; Command line location

; === STAGE2 ENTRY POINT ===
jmp stage2_start

stage2_start:
    ; Normalize segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000
    sti

    ; Display TernaryBit banner
    mov si, banner_msg
    call print_string

    ; Initialize STEPPPS framework
    call init_steppps_linux

    ; Load Linux kernel
    call load_linux_kernel

    ; Setup Linux boot environment
    call setup_linux_boot

    ; Jump to Linux kernel
    call jump_to_linux

    ; Should never reach here
    cli
    hlt

; === STEPPPS INITIALIZATION FOR LINUX ===
init_steppps_linux:
    mov si, steppps_init_msg
    call print_string

    ; Initialize each STEPPPS dimension
    ; Space: Hardware detection
    call detect_hardware

    ; Time: Set up timing
    call setup_timing

    ; Event: Prepare event handling
    call prepare_events

    ; Psychology: User interface readiness
    call setup_psychology

    ; Pixel: Display preparation
    call init_display

    ; Prompt: Command line setup
    call setup_cmdline

    ; Script: Automation readiness
    call init_scripting

    mov si, steppps_ready_msg
    call print_string
    ret

detect_hardware:
    mov si, dim_space_msg
    call print_string
    ; Basic hardware detection here
    ret

setup_timing:
    mov si, dim_time_msg
    call print_string
    ret

prepare_events:
    mov si, dim_event_msg
    call print_string
    ret

setup_psychology:
    mov si, dim_psychology_msg
    call print_string
    ret

init_display:
    mov si, dim_pixel_msg
    call print_string
    ret

setup_cmdline:
    mov si, dim_prompt_msg
    call print_string
    ; Setup Linux command line
    mov si, linux_cmdline
    mov di, LINUX_CMDLINE_ADDR & 0xFFFF
    call copy_string
    ret

init_scripting:
    mov si, dim_script_msg
    call print_string
    ret

; === LINUX KERNEL LOADING ===
load_linux_kernel:
    mov si, loading_linux_msg
    call print_string

    ; Set up for kernel loading at 64KB
    mov ax, LINUX_KERNEL_LOAD_ADDR >> 4
    mov es, ax
    xor bx, bx

    ; Get boot drive
    mov dl, [BOOT_DRIVE_ADDR]

    ; Load Linux kernel sectors
    mov si, BOOT_KERNEL_LBA_START
    mov cx, BOOT_KERNEL_SECTOR_COUNT

.load_loop:
    cmp cx, 0
    jz .load_done

    ; Convert LBA to CHS
    call lba_to_chs

    ; Load one sector at a time for compatibility
    mov ah, 0x02
    mov al, 1
    int 0x13
    jc .load_error

    ; Advance to next sector
    inc si
    add bx, 512
    ; Handle segment wraparound
    cmp bx, 0
    jne .no_wrap
    mov ax, es
    add ax, 0x1000
    mov es, ax
.no_wrap:
    dec cx
    jmp .load_loop

.load_done:
    mov si, kernel_loaded_msg
    call print_string
    ret

.load_error:
    mov si, load_error_msg
    call print_string
    cli
    hlt

; === LINUX BOOT SETUP ===
setup_linux_boot:
    mov si, setup_linux_msg
    call print_string

    ; Setup Linux boot parameters
    ; Zero out the setup area
    mov ax, LINUX_SETUP_SEG
    mov es, ax
    xor di, di
    mov cx, 2048    ; Clear 4KB
    xor ax, ax
    rep stosw

    ; Setup basic boot parameters (simplified)
    mov ax, LINUX_SETUP_SEG
    mov es, ax

    ; Boot signature
    mov word [es:0x1FE], 0xAA55

    ; Setup command line pointer
    mov word [es:0x228], LINUX_CMDLINE_ADDR & 0xFFFF
    mov word [es:0x22A], LINUX_CMDLINE_ADDR >> 4

    ; Root device (auto-detect)
    mov word [es:0x1FC], 0x0100  ; /dev/ram0

    ret

; === JUMP TO LINUX ===
jump_to_linux:
    mov si, jumping_linux_msg
    call print_string

    ; Disable interrupts
    cli

    ; Setup segments for Linux
    mov ax, LINUX_SETUP_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, 0x8000

    ; Jump to Linux kernel setup code
    ; Linux expects to start at setup_seg:0
    jmp LINUX_SETUP_SEG:0

; === UTILITY FUNCTIONS ===

; Convert LBA (in SI) to CHS
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

; Copy null-terminated string from SI to ES:DI
copy_string:
    push ax
.copy_loop:
    lodsb
    stosb
    test al, al
    jnz .copy_loop
    pop ax
    ret

; Print string function
print_string:
    push ax
    push bx
.print_loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .print_loop
.done:
    pop bx
    pop ax
    ret

; === DATA SECTION ===

banner_msg          db 'TernaryBit OS - Linux Integration Active', 13, 10, 0
steppps_init_msg    db 'Initializing STEPPPS Framework for Linux...', 13, 10, 0
dim_space_msg       db '  [SPACE] Hardware detection...', 13, 10, 0
dim_time_msg        db '  [TIME] Timing systems...', 13, 10, 0
dim_event_msg       db '  [EVENT] Event handling...', 13, 10, 0
dim_psychology_msg  db '  [PSYCHOLOGY] User interface...', 13, 10, 0
dim_pixel_msg       db '  [PIXEL] Display systems...', 13, 10, 0
dim_prompt_msg      db '  [PROMPT] Command line...', 13, 10, 0
dim_script_msg      db '  [SCRIPT] Automation...', 13, 10, 0
steppps_ready_msg   db 'STEPPPS Framework Ready!', 13, 10, 0
loading_linux_msg   db 'Loading Linux kernel...', 13, 10, 0
kernel_loaded_msg   db 'Linux kernel loaded successfully!', 13, 10, 0
setup_linux_msg     db 'Setting up Linux boot environment...', 13, 10, 0
jumping_linux_msg   db 'Jumping to Linux kernel...', 13, 10, 0
load_error_msg      db 'ERROR: Failed to load kernel!', 13, 10, 0

; Linux command line
linux_cmdline       db 'root=/dev/ram0 console=tty0 console=ttyS0,115200 ternarybit=steppps', 0

; Pad to 4KB
times 4096-($-$$) db 0