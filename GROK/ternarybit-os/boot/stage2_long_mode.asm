; TernaryBit OS - Long Mode (64-bit) Stage 2 Bootloader
; Transitions: Real Mode (16-bit) → Protected Mode (32-bit) → Long Mode (64-bit)
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 16]
[ORG 0x8000]

; === CONSTANTS ===
KERNEL_LOAD_SEGMENT equ 0x1000     ; Load kernel at 0x10000 (64KB) - segment
KERNEL_LOAD_OFFSET  equ 0x0000     ; Offset within segment
KERNEL_LOAD_ADDR    equ 0x10000    ; Flat address: 0x1000:0x0000 = 0x10000
%ifndef KERNEL_SECTORS
KERNEL_SECTORS      equ 50         ; Load 50 sectors (~25KB) - overridden by build script
%endif
%ifndef KERNEL_START_SECTOR
KERNEL_START_SECTOR equ 10         ; Kernel starts at sector 10 - overridden by build script
%endif
BOOT_DRIVE_ADDR     equ 0x0500     ; Where stage1 saved boot drive

; Page table locations
PML4_ADDR  equ 0x1000   ; Page Map Level 4
PDPT_ADDR  equ 0x2000   ; Page Directory Pointer Table
PD_ADDR    equ 0x3000   ; Page Directory
PT_ADDR    equ 0x4000   ; Page Table (optional, using 2MB pages)

; GDT segment selectors (index * 8)
CODE32_SEG equ 0x08     ; 32-bit code segment
DATA32_SEG equ 0x10     ; 32-bit data segment
CODE64_SEG equ 0x18     ; 64-bit code segment
DATA64_SEG equ 0x20     ; 64-bit data segment

; === ENTRY POINT ===
stage2_start:
    ; Setup segments for real mode
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x8000
    sti

    ; Display banner
    mov si, msg_banner
    call print_string

    ; Initialize STEPPPS framework
    call init_steppps

    ; Enable A20 line
    call enable_a20_line

    ; Load kernel into memory at 1MB
    call load_kernel

    ; Setup page tables for long mode
    call setup_page_tables

    ; Setup GDT (includes 64-bit descriptors)
    call setup_gdt

    ; Enter protected mode first
    call enter_protected_mode

    ; Should never reach here
    jmp halt_system

; === A20 LINE ENABLE ===
enable_a20_line:
    mov si, msg_a20
    call print_string

    ; Try BIOS method first
    mov ax, 0x2401
    int 0x15
    jnc .a20_done

    ; Try keyboard controller method
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

    call wait_8042

.a20_done:
    mov si, msg_a20_done
    call print_string
    ret

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

; === LOAD KERNEL ===
; Load kernel in chunks of max 64 sectors to avoid BIOS limitations
load_kernel:
    mov si, msg_loading
    call print_string

    ; Initialize counters
    mov word [sectors_remaining], KERNEL_SECTORS
    mov word [current_lba_lo], KERNEL_START_SECTOR
    mov word [current_lba_hi], 0
    mov word [buffer_segment], KERNEL_LOAD_SEGMENT

.load_loop:
    ; Calculate sectors to read this iteration (min of 64 and remaining)
    mov ax, [sectors_remaining]
    cmp ax, 64
    jbe .use_remaining
    mov ax, 64
.use_remaining:
    mov [disk_packet_count], ax

    ; Update disk packet with current segment (offset always 0)
    mov ax, [buffer_segment]
    mov [disk_packet_segment], ax
    mov ax, [current_lba_lo]
    mov [disk_packet_lba], ax
    mov ax, [current_lba_hi]
    mov [disk_packet_lba+2], ax

    ; Read sectors
    mov ah, 0x42           ; Extended read
    mov dl, [BOOT_DRIVE_ADDR]
    mov si, disk_packet
    int 0x13
    jc disk_error

    ; Update counters for next iteration
    mov ax, [disk_packet_count]

    ; Update sectors_remaining
    sub [sectors_remaining], ax

    ; Update current_lba
    add [current_lba_lo], ax
    adc word [current_lba_hi], 0

    ; Update buffer_segment (ax * 512 / 16 = ax * 32 = ax << 5)
    shl ax, 5
    add [buffer_segment], ax

    ; Check if done
    cmp word [sectors_remaining], 0
    jne .load_loop

    mov si, msg_kernel_loaded
    call print_string
    ret

disk_error:
    mov si, msg_disk_error
    call print_string
    jmp halt_system

; Variables for chunked loading
sectors_remaining:  dw 0
current_lba_lo:     dw 0
current_lba_hi:     dw 0
buffer_segment:     dw 0

disk_packet:
    db 0x10                          ; Packet size
    db 0                             ; Reserved
disk_packet_count:
    dw 0                             ; Number of sectors (filled at runtime)
    dw 0                             ; Buffer offset (always 0)
disk_packet_segment:
    dw KERNEL_LOAD_SEGMENT           ; Buffer segment (updated each iteration)
disk_packet_lba:
    dq KERNEL_START_SECTOR           ; Starting LBA (updated each iteration)

; === SETUP PAGE TABLES ===
setup_page_tables:
    push ax
    push cx
    push di

    mov si, msg_paging
    call print_string

    ; Clear page table memory (16KB)
    mov di, PML4_ADDR
    mov cx, 0x4000         ; 16KB
    xor ax, ax
    rep stosb

    ; Setup PML4[0] → PDPT
    mov di, PML4_ADDR
    mov dword [di], PDPT_ADDR | 3  ; Present + RW

    ; Setup PDPT[0] → PD
    mov di, PDPT_ADDR
    mov dword [di], PD_ADDR | 3    ; Present + RW

    ; Setup PD entries - identity map first 8MB (4 x 2MB pages)
    ; This covers: bootloader (low mem) + kernel (1MB) + BSS + stack (2MB)
    mov di, PD_ADDR
    mov eax, 0x83                  ; Present + RW + 2MB page
    mov ecx, 4                     ; Map 4 entries = 8MB
.setup_pd_loop:
    mov dword [di], eax
    mov dword [di+4], 0
    add eax, 0x200000              ; Next 2MB
    add di, 8
    loop .setup_pd_loop

    mov si, msg_paging_done
    call print_string

    pop di
    pop cx
    pop ax
    ret

; === SETUP GDT ===
setup_gdt:
    mov si, msg_gdt
    call print_string

    lgdt [gdt_descriptor]

    mov si, msg_gdt_done
    call print_string
    ret

; === ENTER PROTECTED MODE ===
enter_protected_mode:
    cli

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Jump to 32-bit protected mode code
    jmp CODE32_SEG:protected_mode_entry

; === STRING PRINTING (Real Mode) ===
print_string:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

; === INIT STEPPPS ===
init_steppps:
    mov si, msg_steppps
    call print_string
    mov si, msg_steppps_items
    call print_string
    mov si, msg_steppps_done
    call print_string
    ret

; === HALT SYSTEM ===
halt_system:
    cli
.hang:
    hlt
    jmp .hang

; === MESSAGES ===
msg_banner:         db '=====================================', 13, 10
                    db '     TBOS x STEPPPS Framework', 13, 10
                    db '=====================================', 13, 10
                    db '[BOOT] Initializing Sacred System...', 13, 10
                    db '[BOOT] Stage 2 loaded successfully', 13, 10
                    db '[BOOT] Transferring control...', 13, 10
                    db '=== TernaryBit OS Stage 2 Bootloader (64-bit) ===', 13, 10, 0

msg_steppps:        db 'Initializing STEPPPS Framework...', 13, 10, 0
msg_steppps_items:  db '  [SPACE] Hardware detection', 13, 10
                    db '  [TIME] Timing systems', 13, 10
                    db '  [EVENT] Event handling', 13, 10
                    db '  [PSYCHOLOGY] AI framework', 13, 10
                    db '  [PIXEL] Display systems', 13, 10
                    db '  [PROMPT] Command interface', 13, 10
                    db '  [SCRIPT] Automation', 13, 10, 0
msg_steppps_done:   db 'STEPPPS Framework Ready!', 13, 10, 0

msg_a20:            db 'Enabling A20 line...', 13, 10, 0
msg_a20_done:       db 'A20 enabled', 13, 10, 0
msg_loading:        db 'Loading TBOS kernel...', 13, 10, 0
msg_kernel_loaded:  db 'Kernel loaded successfully!', 13, 10, 0
msg_disk_error:     db 'ERROR: Disk read failed!', 13, 10, 0
msg_paging:         db 'Setting up page tables...', 13, 10, 0
msg_paging_done:    db 'Page tables configured', 13, 10, 0
msg_gdt:            db 'Setting up GDT...', 13, 10, 0
msg_gdt_done:       db 'GDT configured', 13, 10, 0

; ========================================
; 32-BIT PROTECTED MODE CODE
; ========================================
[BITS 32]
protected_mode_entry:
    ; Disable interrupts
    cli

    ; Setup segment registers
    mov ax, DATA32_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Setup stack
    mov esp, 0x90000
    mov ebp, esp

    ; Serial debug output
    mov dx, 0x3F8
    mov al, '['
    out dx, al
    mov al, 'P'
    out dx, al
    mov al, 'M'
    out dx, al
    mov al, ']'
    out dx, al
    mov al, 10
    out dx, al

    ; Enable PAE (Physical Address Extension)
    mov eax, cr4
    or eax, (1 << 5)       ; Set PAE bit (bit 5)
    mov cr4, eax

    ; Load CR3 with PML4 address
    mov eax, PML4_ADDR
    mov cr3, eax

    ; Enable long mode in EFER MSR
    mov ecx, 0xC0000080    ; EFER MSR number
    rdmsr
    or eax, (1 << 8)       ; Set LME bit (Long Mode Enable)
    wrmsr

    ; Enable paging (this activates long mode)
    mov eax, cr0
    or eax, (1 << 31)      ; Set PG bit
    mov cr0, eax

    ; Now in compatibility mode (32-bit code in long mode)
    ; Jump to 64-bit code segment
    jmp CODE64_SEG:long_mode_entry

; ========================================
; 64-BIT LONG MODE CODE
; ========================================
[BITS 64]
long_mode_entry:
    ; Load data segments with 64-bit data selector
    mov ax, DATA64_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Serial debug
    mov dx, 0x3F8
    mov al, '['
    out dx, al
    mov al, 'L'
    out dx, al
    mov al, 'M'
    out dx, al
    mov al, ']'
    out dx, al
    mov al, 10
    out dx, al

    ; Setup 64-bit stack
    mov rsp, 0x200000
    mov rbp, rsp

    ; Debug: About to jump to kernel
    mov dx, 0x3F8
    mov al, '['
    out dx, al
    mov al, 'J'
    out dx, al
    mov al, 'M'
    out dx, al
    mov al, 'P'
    out dx, al
    mov al, ']'
    out dx, al
    mov al, 10
    out dx, al

    ; Jump to kernel at 1MB
    ; Use absolute 64-bit jump
    mov rax, KERNEL_LOAD_ADDR
    jmp rax

    ; Should never return
    cli
.halt:
    hlt
    jmp .halt

; === GDT (Global Descriptor Table) ===
align 8
gdt_start:
    ; Null descriptor
    dq 0

    ; 32-bit code segment (for protected mode transition)
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10011010b    ; Access: present, ring 0, code, executable, readable
    db 11001111b    ; Flags: 4KB gran, 32-bit
    db 0x00         ; Base high

    ; 32-bit data segment
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b    ; Access: present, ring 0, data, writable
    db 11001111b
    db 0x00

    ; 64-bit code segment
    dw 0xFFFF       ; Limit (ignored in long mode, but some CPUs check it)
    dw 0x0000       ; Base (ignored in long mode)
    db 0x00
    db 10011010b    ; Access: present, ring 0, code, executable, readable
    db 10101111b    ; Flags: G=1, L=1 (long mode), D=0, AVL=1
    db 0x00

    ; 64-bit data segment
    dw 0x0000
    dw 0x0000
    db 0x00
    db 10010010b    ; Access: present, ring 0, data
    db 10100000b    ; Flags: Long mode
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Pad to sector boundary
times 8192-($-$$) db 0
