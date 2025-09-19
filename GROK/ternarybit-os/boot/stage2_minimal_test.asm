; TernaryBit OS - Minimal Stage 2 for Memory Management Testing
[BITS 16]
[ORG 0x8000]

; Memory Management Configuration
%define TOTAL_RAM_KB           1024     ; 1MB baseline
%define STEPPPS_PERCENT        10       ; 10% for STEPPPS
%define TBDS_PERCENT           20       ; 20% for TBDS

; Calculate sizes
%define STEPPPS_SIZE_KB        (TOTAL_RAM_KB * STEPPPS_PERCENT / 100)
%define TBDS_SIZE_KB           (TOTAL_RAM_KB * TBDS_PERCENT / 100)

; Memory layout
%define STEPPPS_BASE           0x1000
%define STEPPPS_MAX_SIZE       (STEPPPS_SIZE_KB * 1024)
%define TBDS_BASE              (STEPPPS_BASE + STEPPPS_MAX_SIZE)
%define TBDS_MAX_SIZE          (TBDS_SIZE_KB * 1024)

; TBDS constants
%define TBDS_SIGNATURE         0x53444454
%define TBDS_VERSION_1_0       0x0100
%define TBDS_HEADER_SIZE       20

stage2_start:
    mov si, banner
    call print_string

    ; Detect and allocate memory
    call detect_memory
    call display_layout

    ; Initialize TBDS at safe location
    call tbds_init

    ; Test TBDS display
    mov si, tbds_display_msg
    call print_string
    mov ax, [tbds_base]
    call print_hex16
    call print_newline

    ; Show TBDS signature
    mov di, [tbds_base]
    mov ax, [di]
    call print_hex16
    mov ax, [di+2]
    call print_hex16
    call print_newline

    ; Enter infinite loop
.hang:
    hlt
    jmp .hang

; Memory detection
detect_memory:
    mov ah, 0x88
    int 0x15
    jc .fallback
    add ax, 1024
    mov [total_ram], ax
    jmp .calc
.fallback:
    mov word [total_ram], 1024
.calc:
    ; Calculate allocations
    mov ax, [total_ram]
    mov bx, 10
    mul bx
    mov bx, 100
    div bx
    mov [steppps_size], ax

    mov ax, [total_ram]
    mov bx, 20
    mul bx
    mov bx, 100
    div bx
    mov [tbds_size], ax

    ; Set base addresses
    mov word [steppps_base_addr], STEPPPS_BASE
    mov ax, STEPPPS_BASE
    mov bx, [steppps_size]
    shl bx, 10
    add ax, bx
    mov [tbds_base], ax
    ret

; Display memory layout
display_layout:
    mov si, mem_msg
    call print_string
    mov ax, [total_ram]
    call print_decimal
    mov si, kb_msg
    call print_string

    mov si, steppps_msg
    call print_string
    mov ax, [steppps_size]
    call print_decimal
    mov si, kb_msg
    call print_string

    mov si, tbds_msg
    call print_string
    mov ax, [tbds_size]
    call print_decimal
    mov si, kb_msg
    call print_string
    ret

; Initialize TBDS
tbds_init:
    mov di, [tbds_base]
    mov dword [di], TBDS_SIGNATURE
    mov dword [di+4], TBDS_HEADER_SIZE
    mov word [di+8], TBDS_VERSION_1_0
    mov word [di+10], 0
    ret

; Print functions
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

print_decimal:
    push ax
    push bx
    push cx
    push dx
    mov cx, 0
    mov bx, 10
.convert:
    xor dx, dx
    div bx
    add dl, '0'
    push dx
    inc cx
    test ax, ax
    jnz .convert
.print:
    pop dx
    mov ah, 0x0E
    mov al, dl
    mov bx, 0x0007
    int 0x10
    loop .print
    pop dx
    pop cx
    pop bx
    pop ax
    ret

print_hex16:
    push ax
    push bx
    push cx
    mov cx, 4
.digit:
    rol ax, 4
    push ax
    and al, 0x0F
    cmp al, 9
    jle .decimal
    add al, 'A' - 10
    jmp .char
.decimal:
    add al, '0'
.char:
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    pop ax
    loop .digit
    pop cx
    pop bx
    pop ax
    ret

print_newline:
    push ax
    push bx
    mov ah, 0x0E
    mov al, 0x0D
    mov bx, 0x0007
    int 0x10
    mov al, 0x0A
    int 0x10
    pop bx
    pop ax
    ret

; Data
banner           db 'STEPPPS Memory Test', 0x0D, 0x0A, 0x00
mem_msg          db 'RAM: ', 0x00
steppps_msg      db 'STEPPPS: ', 0x00
tbds_msg         db 'TBDS: ', 0x00
tbds_display_msg db 'TBDS@', 0x00
kb_msg           db 'KB ', 0x00

; Variables
total_ram        dw 0
steppps_size     dw 0
tbds_size        dw 0
steppps_base_addr dw 0
tbds_base        dw 0

; Padding to fill 4KB
times 4096-($-$$) db 0