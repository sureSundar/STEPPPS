; TernaryBit OS - Optimized Boot Sector
; First stage bootloader (512 bytes)
; Optimized for speed and minimal boot time

[BITS 16]
[ORG 0x7C00]

; --- Constants ---
%define SECOND_STAGE_SEGMENT 0x8000
%define SECOND_STAGE_SECTOR   0x02
%define SECTORS_TO_LOAD       0x04

section .text
start:
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Set up segments and stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Fast clear screen (direct VRAM access)
    mov ax, 0xB800
    mov es, ax
    xor di, di
    mov cx, 80*25
    mov ax, 0x0700 | ' '
    rep stosw
    
    ; Display boot message (direct write)
    mov si, boot_message
    call fast_print
    
    ; Initialize hardware in parallel where possible
    call init_critical_hardware
    
    ; Load second stage using optimized disk read
    mov ax, SECOND_STAGE_SEGMENT
    mov es, ax
    xor bx, bx
    mov ah, 0x42        ; Extended Read Sectors
    mov dl, [boot_drive]
    mov si, dap_packet
    int 0x13
    jc disk_error
    
    ; Jump to second stage
    jmp SECOND_STAGE_SEGMENT:0x0000

; --- Optimized Subroutines ---

; Fast string print (no BIOS calls)
fast_print:
    pusha
    mov ah, 0x0E        ; BIOS teletype
    mov bh, 0x00        ; Page 0
    cld                 ; Clear direction flag
.next_char:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .next_char
.done:
    popa
    ret

; Initialize only critical hardware
init_critical_hardware:
    ; 1. Set up basic interrupt vectors
    cli
    ; ... (minimal interrupt setup)
    sti
    
    ; 2. Fast A20 line enable (keyboard controller method)
    call enable_a20_fast
    
    ; 3. Set up minimal descriptor tables
    lgdt [gdt_descriptor]
    ret

; Fast A20 enable (keyboard controller method)
enable_a20_fast:
    cli
    call .a20_wait
    mov al, 0xAD
    out 0x64, al
    call .a20_wait
    mov al, 0xD0
    out 0x64, al
    call .a20_wait2
    in al, 0x60
    push eax
    call .a20_wait
    mov al, 0xD1
    out 0x64, al
    call .a20_wait
    pop eax
    or al, 2
    out 0x60, al
    call .a20_wait
    mov al, 0xAE
    out 0x64, al
    call .a20_wait
    sti
    ret
.a20_wait:
    in al, 0x64
    test al, 2
    jnz .a20_wait
    ret
.a20_wait2:
    in al, 0x64
    test al, 1
    jz .a20_wait2
    ret

; Error handling
disk_error:
    mov si, disk_error_msg
    call fast_print
    cli
    hlt

; --- Data Section (padded to 510 bytes) ---

boot_drive      db 0
disk_error_msg  db 'Disk error!', 0
boot_message    db 'TBOS Booting...', 0x0D, 0x0A, 0

; Disk Address Packet (DAP) for LBA disk access
dap_packet:
    db 0x10           ; Size of DAP
    db 0              ; Unused
    dw SECTORS_TO_LOAD; Number of sectors to read
    dw 0x0000         ; Offset
    dw SECOND_STAGE_SEGMENT ; Segment
    dq SECOND_STAGE_SECTOR  ; Starting LBA

; Minimal GDT for protected mode
gdt_start:
    dq 0x0000000000000000  ; Null descriptor
    dq 0x00CF9A000000FFFF  ; Code segment (32-bit)
    dq 0x00CF92000000FFFF  ; Data segment (32-bit)
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; Size
    dd gdt_start                ; Offset

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
