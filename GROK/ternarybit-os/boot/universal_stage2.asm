; Universal Stage 2 Bootloader
; - Runs in real mode, loads kernel with BIOS EDD (INT 13h extensions)
; - Switches to 32-bit protected mode and jumps to kernel
; - Minimal, hard-coded geometry suitable for QEMU/PC BIOS
; Keep STAGE2_SECTORS in sync with stage1 and build script.

[BITS 16]
[ORG 0x8000]

%ifndef STAGE2_SECTORS
%define STAGE2_SECTORS       8
%endif
%ifndef KERNEL_START_LBA
%define KERNEL_START_LBA     (STAGE2_SECTORS + 1) ; 1 boot sector + stage2
%endif
%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS       64
%endif
%define KERNEL_LOAD_SEG      0x1000               ; physical 0x10000
%define KERNEL_LOAD_OFF      0x0000
%define KERNEL_LOAD_PHYS     0x00010000
%define STACK_PM             0x00090000
%define BOOT_DRIVE_ADDR      0x0500               ; written by stage1

stage2_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00          ; stack below us
    sti

    mov [boot_drive], dl    ; DL preserved by BIOS

    mov si, msg_banner
    call print

    ; enable A20 before loading above 64K to be safe
    call enable_a20

    mov si, msg_load_kernel
    call print
    call load_kernel

    mov si, msg_enter_pm
    call print

    ; set up GDT and jump to protected mode
    cli
    lgdt [gdt_ptr]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; far jump with 32-bit offset to flush prefetch
    db 0x66
    db 0xEA
    dd pm_entry
    dw 0x08                ; code selector

; ---------------------------------------------------------------------------
; BIOS output
; ---------------------------------------------------------------------------
print:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    mov bl, 0x07
    int 0x10
    jmp .loop
.done:
    popa
    ret

; ---------------------------------------------------------------------------
; Enable A20 via keyboard controller (fallback-safe)
; ---------------------------------------------------------------------------
enable_a20:
    pusha
.wait_ibf_clear:
    in al, 0x64
    test al, 0x02
    jnz .wait_ibf_clear

    mov al, 0xD1
    out 0x64, al

.wait_ibf_clear2:
    in al, 0x64
    test al, 0x02
    jnz .wait_ibf_clear2

    mov al, 0xDF
    out 0x60, al
    popa
    ret

; ---------------------------------------------------------------------------
; Load kernel using INT 13h extensions (EDD) to 0x10000
; ---------------------------------------------------------------------------
load_kernel:
    pusha
    mov dl, [BOOT_DRIVE_ADDR]
    mov word [dap.count], KERNEL_SECTORS
    mov word [dap.offset], KERNEL_LOAD_OFF
    mov word [dap.segment], KERNEL_LOAD_SEG
    mov dword [dap.lba_low], KERNEL_START_LBA
    mov dword [dap.lba_high], 0

    mov si, dap
    mov ah, 0x42            ; extended read
    int 0x13
    jc load_fail

    mov si, msg_loaded
    call print
    popa
    ret

load_fail:
    mov si, msg_load_fail
    call print
halt:
    cli
    hlt
    jmp halt

; ---------------------------------------------------------------------------
; 32-bit protected mode entry
; ---------------------------------------------------------------------------
[BITS 32]
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov esp, STACK_PM
    mov ebp, esp

    jmp KERNEL_LOAD_PHYS

; ---------------------------------------------------------------------------
; GDT
; ---------------------------------------------------------------------------
[BITS 16]
align 8
gdt_start:
gdt_null:   dq 0
gdt_code:   dw 0xFFFF, 0x0000, 0x00, 10011010b, 11001111b, 0x00
gdt_data:   dw 0xFFFF, 0x0000, 0x00, 10010010b, 11001111b, 0x00
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ---------------------------------------------------------------------------
; Disk Address Packet for INT 13h extensions (16 bytes)
; ---------------------------------------------------------------------------
dap:
    db 0x10, 0x00           ; size, reserved
dap.count:   dw 0
dap.offset:  dw 0
dap.segment: dw 0
dap.lba_low: dd 0
dap.lba_high:dd 0

; ---------------------------------------------------------------------------
; Data / messages
; ---------------------------------------------------------------------------
boot_drive:       db 0
msg_banner:       db 13,10,'[TBOS] Universal Stage2',13,10,0
msg_load_kernel:  db '[TBOS] Loading kernel...',13,10,0
msg_loaded:       db '[TBOS] Kernel ready',13,10,0
msg_enter_pm:     db '[TBOS] Entering protected mode',13,10,0
msg_load_fail:    db '[ERR] Kernel load failed, halting',13,10,0

; pad to full stage2 size (STAGE2_SECTORS * 512)
times (STAGE2_SECTORS*512)-($-$$) db 0
