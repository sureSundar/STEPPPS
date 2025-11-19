; Universal Bootloader - Stage 2
; Design: Works from minimal hardware to supercomputers
; Phase: Protected Mode Transition
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 16]
[ORG 0x8000]

jmp stage2_entry          ; skip HAL helpers at file start

%define BOOT_HAL_SECTORS_PER_TRACK 18
%define BOOT_HAL_HEADS_PER_CYLINDER 2

%include "boot_hal.inc"

%ifndef BOOT_KERNEL_LBA_START
%define BOOT_KERNEL_LBA_START 10
%endif

%ifndef BOOT_KERNEL_SECTOR_COUNT
%define BOOT_KERNEL_SECTOR_COUNT 32
%endif

%ifndef STAGE2_LBA_START
%define STAGE2_LBA_START 1
%endif

%ifndef BOOT_DRIVE_ADDR
%define BOOT_DRIVE_ADDR 0x0500
%endif

%define TBDS_BASE           0x7000
%define TBDS_HEADER_SIZE    20
%define TBDS_SIGNATURE      0x53444454
%define TBDS_VERSION_1_0    0x0100
%define TBDS_TYPE_BOOT_DEVICE   0x0004
%define TBDS_TYPE_CONSOLE_INFO  0x0006
%define TBDS_TYPE_FIRMWARE_INFO 0x0002

%define KERNEL_SEGMENT (KERNEL_ADDR >> 4)

; === CONSTANTS ===
KERNEL_ADDR equ 0x10000

stage2_entry:
    ; Stage1 jumps here with CS=0, IP=0x8000
    ; First: Establish known segment state
    cli
    xor ax, ax
    mov ds, ax              ; DS = 0
    mov es, ax              ; ES = 0
    mov ss, ax              ; SS = 0
    mov sp, 0x8000          ; Stack below stage2
    sti

    ; Print banner
    mov si, banner
    call boot_hal_print_string

    ; Load kernel from disk
    call load_kernel

    ; Build TBDS handoff structure
    call build_tbds

    ; Enable A20 line (fast method via port 0x92)
    call enable_a20

    ; Enter protected mode
    cli

    ; Load GDT - ES MUST be 0 here
    lgdt [gdt_ptr]

    ; Enable protected mode bit
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; Far jump to protected mode code
    ; Need physical address = ORG + offset
    db 0x66                         ; 32-bit operand size prefix
    db 0xEA                         ; Far jump opcode
    dd 0x8000 + (pm_entry - $$)     ; 32-bit offset
    dw 0x08                         ; Code segment

; === 16-BIT FUNCTIONS ===

load_kernel:
    pusha
    mov si, loading_msg
    call boot_hal_print_string

    mov dword [kernel_lba_low], BOOT_KERNEL_LBA_START
    mov dword [kernel_dest], KERNEL_ADDR
    mov word [kernel_remaining], BOOT_KERNEL_SECTOR_COUNT

.read_loop:
    mov cx, [kernel_remaining]
    jcxz .done

    mov ax, cx
    cmp ax, BOOT_HAL_MAX_TRANSFER
    jbe .chunk_ok
    mov ax, BOOT_HAL_MAX_TRANSFER
.chunk_ok:
    mov [kernel_chunk], ax

    mov eax, [kernel_dest]
    mov bx, ax
    and bx, 0x000F
    shr eax, 4
    mov es, ax

    mov eax, [kernel_lba_low]
    xor edx, edx
    mov cx, [kernel_chunk]
    mov dl, [BOOT_DRIVE_ADDR]
    call boot_hal_read_lba
    jc .error

    mov cx, [kernel_remaining]
    sub cx, [kernel_chunk]
    mov [kernel_remaining], cx

    xor ebx, ebx
    mov bx, [kernel_chunk]
    shl ebx, 9
    mov eax, [kernel_dest]
    add eax, ebx
    mov [kernel_dest], eax

    mov eax, [kernel_lba_low]
    xor edx, edx
    mov dx, [kernel_chunk]
    add eax, edx
    mov [kernel_lba_low], eax

    jmp .read_loop

.done:
    mov si, loaded_msg
    call boot_hal_print_string
    popa
    ret

.error:
    mov si, error_msg
    call boot_hal_print_string
    call boot_hal_halt

; -------------------------------------
; TBDS builder
; -------------------------------------

build_tbds:
    pusha
    mov di, TBDS_BASE
    mov eax, TBDS_SIGNATURE
    mov [di], eax
    mov eax, TBDS_HEADER_SIZE
    mov [di + 4], eax
    mov word [di + 8], TBDS_VERSION_1_0
    mov word [di + 10], 0
    mov dword [di + 12], 0
    mov dword [di + 16], 0

    mov word [tbds_descriptor_count], 0
    mov dword [tbds_total_length], TBDS_HEADER_SIZE
    mov word [tbds_cursor], TBDS_BASE + TBDS_HEADER_SIZE

    ; Populate boot device descriptor
    mov al, [BOOT_DRIVE_ADDR]
    mov [boot_device_info + 0], al
    mov word [boot_device_info + 2], 0x0001       ; EDD supported
    mov dword [boot_device_info + 4], STAGE2_LBA_START
    mov dword [boot_device_info + 8], BOOT_KERNEL_LBA_START
    mov word [boot_device_info + 12], BOOT_KERNEL_SECTOR_COUNT

    mov ax, TBDS_TYPE_BOOT_DEVICE
    xor bx, bx
    mov cx, BOOT_DEVICE_INFO_SIZE
    mov si, boot_device_info
    call tbds_append_descriptor

    ; Console descriptor (text console available, serial TBD)
    mov ax, TBDS_TYPE_CONSOLE_INFO
    xor bx, bx
    mov cx, CONSOLE_INFO_SIZE
    mov si, console_info_payload
    call tbds_append_descriptor

    ; Firmware descriptor
    mov ax, TBDS_TYPE_FIRMWARE_INFO
    xor bx, bx
    mov cx, FIRMWARE_INFO_SIZE
    mov si, firmware_info_payload
    call tbds_append_descriptor

    ; Finalise header metadata
    mov ax, [tbds_descriptor_count]
    mov [TBDS_BASE + 10], ax
    mov eax, [tbds_total_length]
    mov [TBDS_BASE + 4], eax

    mov eax, TBDS_BASE
    mov [tbds_pointer], eax
    mov eax, [tbds_total_length]
    mov [tbds_length], eax

    popa
    ret

; AX=type, BX=flags, CX=payload length, SI=payload pointer
tbds_append_descriptor:
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push es

    mov di, [tbds_cursor]
    mov [di], ax
    mov [di + 2], bx
    mov dx, cx
    mov word [di + 4], cx
    mov word [di + 6], 0
    add di, 8

    xor ax, ax
    mov es, ax
    mov cx, dx
    jcxz .skip_copy
    rep movsb
.skip_copy:
    mov di, [tbds_cursor]
    add di, 8
    add di, dx
    mov [tbds_cursor], di

    mov ax, [tbds_descriptor_count]
    inc ax
    mov [tbds_descriptor_count], ax

    mov eax, [tbds_total_length]
    xor ecx, ecx
    mov cx, dx
    add eax, 8
    add eax, ecx
    mov [tbds_total_length], eax

    pop es
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; === 32-BIT PROTECTED MODE ===

[BITS 32]
pm_entry:
    ; Set up all segments to data selector
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack
    mov esp, 0x90000
    mov ebp, esp

    ; Pass TBDS pointer/length to kernel
    mov eax, [tbds_pointer]
    mov ebx, [tbds_length]

    ; Jump to kernel
    mov edx, KERNEL_ADDR
    jmp edx

; === GDT ===

align 8
gdt_start:

gdt_null:
    dd 0, 0

gdt_code:
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x00             ; Base 16-23
    db 10011010b        ; Access: Present, Ring 0, Code, Exec/Read
    db 11001111b        ; Flags: 4K pages, 32-bit, Limit 16-19
    db 0x00             ; Base 24-31

gdt_data:
    dw 0xFFFF           ; Limit 0-15
    dw 0x0000           ; Base 0-15
    db 0x00             ; Base 16-23
    db 10010010b        ; Access: Present, Ring 0, Data, Read/Write
    db 11001111b        ; Flags: 4K pages, 32-bit, Limit 16-19
    db 0x00             ; Base 24-31

gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1              ; Size
    dd (gdt_start - $$) + 0x8000           ; Physical = file_offset + ORG

; === DATA ===

banner:         db '=== TernaryBit OS Stage 2 Bootloader ===', 13, 10, 0
loading_msg:    db 'Loading kernel...', 13, 10, 0
loaded_msg:     db 'Kernel loaded. Entering protected mode...', 13, 10, 0
error_msg:      db 'ERROR: Disk read failed!', 13, 10, 0
kernel_lba_low      dd 0
kernel_dest         dd 0
kernel_remaining    dw 0
kernel_chunk        dw 0
boot_device_info:
    db 0x00                    ; drive number (filled at runtime)
    db 0x02                    ; loader stage identifier
    dw 0x0001                  ; flags (EDD present)
    dd 0                       ; stage2 LBA start (runtime)
    dd 0                       ; kernel LBA start (runtime)
    dw 0                       ; kernel sector count (runtime)
    dw 0                       ; reserved
BOOT_DEVICE_INFO_SIZE equ $ - boot_device_info

console_info_payload:
    db 0x01                    ; text console available
    db 0x00                    ; graphical console
    db 0x00                    ; serial console (0 = not initialised)
    db 0x00
    dw 80                      ; columns
    dw 25                      ; rows
    dw 0                       ; character height (unknown)
    dw 0                       ; character width (unknown)
CONSOLE_INFO_SIZE equ $ - console_info_payload

firmware_info_payload:
    db 'BIOS/INT13H'
FIRMWARE_INFO_SIZE equ $ - firmware_info_payload

tbds_pointer        dd 0
tbds_length         dd 0
tbds_total_length   dd 0
tbds_descriptor_count dw 0
tbds_cursor         dw 0

; === REAL-MODE HELPERS ===

[BITS 16]
enable_a20:
    in al, 0x92
    test al, 0x02
    jnz .enabled
    or al, 0x02
    and al, 0xFE
    out 0x92, al
.enabled:
    ret

; Pad to 4KB
times 4096-($-$$) db 0
