; TernaryBit OS - Timed Bootloader
; First stage bootloader with performance timing

[BITS 16]
[ORG 0x7C00]

; --- Constants ---
%define BOOT_TIMING_BASE 0x7E00  ; Where we'll store timing data
%define TIMING_ENABLED   1       ; Set to 0 to disable timing

section .text
start:
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Initialize timing if enabled
%if TIMING_ENABLED
    ; Record BIOS entry time
    rdtsc
    mov [BOOT_TIMING_BASE + 0], eax  ; Low 32 bits
    mov [BOOT_TIMING_BASE + 4], edx  ; High 32 bits
%endif
    
    ; Set up segments and stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Record first stage start
%if TIMING_ENABLED
    call record_timing
    mov byte [BOOT_TIMING_BASE + 8], 2  ; BOOT_FIRST_STAGE
%endif
    
    ; Fast clear screen
    mov ax, 0xB800
    mov es, ax
    xor di, di
    mov cx, 80*25
    mov ax, 0x0700 | ' '
    rep stosw
    
    ; Display boot message
    mov si, boot_message
    call fast_print
    
    ; Load second stage
    mov ax, 0x8000
    mov es, ax
    xor bx, bx
    mov ah, 0x42        ; Extended Read Sectors
    mov dl, [boot_drive]
    mov si, dap_packet
    int 0x13
    jc disk_error
    
    ; Record second stage start
%if TIMING_ENABLED
    call record_timing
    mov byte [BOOT_TIMING_BASE + 9], 3  ; BOOT_SECOND_STAGE
%endif
    
    ; Jump to second stage
    jmp 0x8000:0x0000

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

; Record timing data
%if TIMING_ENABLED
record_timing:
    pusha
    mov ecx, [BOOT_TIMING_BASE]  ; Get counter
    inc ecx
    mov [BOOT_TIMING_BASE], ecx  ; Store updated counter
    
    ; Calculate offset: 16 (header) + (counter * 8)
    shl ecx, 3
    add ecx, 16
    
    ; Get TSC
    rdtsc
    
    ; Store timestamp
    mov [BOOT_TIMING_BASE + ecx], eax      ; Low 32 bits
    mov [BOOT_TIMING_BASE + ecx + 4], edx  ; High 32 bits
    popa
    ret
%endif

; Error handling
disk_error:
    mov si, disk_error_msg
    call fast_print
    cli
    hlt

; --- Data Section ---

boot_drive      db 0
disk_error_msg  db 'Disk error!', 0
boot_message    db 'TBOS Booting...', 0x0D, 0x0A, 0

; Disk Address Packet (DAP) for LBA disk access
dap_packet:
    db 0x10           ; Size of DAP
    db 0              ; Unused
    dw 4              ; Number of sectors to read
    dw 0x0000         ; Offset
    dw 0x8000         ; Segment
    dq 1              ; Starting LBA (sector 1)

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
