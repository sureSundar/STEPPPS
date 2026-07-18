; TernaryBit OS - Compressed Stage 2 Bootloader
; COMPRESSION TARGET: <3KB (from 19KB source)
; Techniques: String tables, code merging, minimal debug

[BITS 16]
[ORG 0x8000]

; === COMPRESSED CONSTANTS ===
%define K_SEC 8
%define K_LBA 6
CODE_SEG equ 0x08
DATA_SEG equ 0x10
KERNEL_ENTRY equ 0x0010000

; === COMPRESSED ENTRY ===
jmp stage2_start

; === COMPRESSED STRING TABLE ===
str_tbl:
    db 'S2', 0          ; 0: Stage2 start
    db 'GDT', 0         ; 3: GDT loading
    db 'A20', 0         ; 7: A20 gate
    db 'PM', 0          ; 11: Protected mode
    db 'K', 0           ; 14: Kernel
    db 'ERR', 0         ; 16: Error
    db 'OK', 0          ; 20: Success

; === MICRO PRINT FUNCTION ===
print_code:
    pusha
    mov ah, 0x0E
    mov bx, 0x0007
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    mov al, 0x0D
    int 0x10
    mov al, 0x0A
    int 0x10
    popa
    ret

; === COMPRESSED MAIN LOGIC ===
stage2_start:
    ; Print start indicator
    mov si, str_tbl + 0
    call print_code

    ; Setup GDT
    mov si, str_tbl + 3
    call print_code
    lgdt [gdt_desc]

    ; Enable A20
    mov si, str_tbl + 7
    call print_code
    call enable_a20_fast

    ; Enter protected mode
    mov si, str_tbl + 11
    call print_code
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp CODE_SEG:protected_mode

enable_a20_fast:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

[BITS 32]
protected_mode:
    ; Setup segments
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Load kernel
    call load_kernel_lba

    ; Jump to kernel
    jmp KERNEL_ENTRY

load_kernel_lba:
    ; Simplified kernel loading
    mov edi, KERNEL_ENTRY
    mov ecx, K_SEC
    mov eax, K_LBA
.load_loop:
    ; Read sector using simplified LBA
    push ecx
    call read_sector_lba
    pop ecx
    add edi, 512
    inc eax
    loop .load_loop
    ret

read_sector_lba:
    ; Minimal LBA read implementation
    pusha
    mov dx, 0x1F6
    mov al, 0xE0
    out dx, al

    mov dx, 0x1F2
    mov al, 1
    out dx, al

    mov dx, 0x1F3
    out dx, al

    mov dx, 0x1F4
    mov al, ah
    out dx, al

    mov dx, 0x1F5
    shr eax, 16
    out dx, al

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

.wait:
    in al, dx
    test al, 8
    jz .wait

    mov dx, 0x1F0
    mov ecx, 256
    rep insw

    popa
    ret

; === COMPRESSED GDT ===
gdt_start:
    ; Null descriptor
    dd 0, 0

    ; Code descriptor: base=0, limit=0xfffff, present, ring 0, code, readable
    dw 0xffff, 0x0000, 0x9a00, 0x00cf

    ; Data descriptor: base=0, limit=0xfffff, present, ring 0, data, writable
    dw 0xffff, 0x0000, 0x9200, 0x00cf

gdt_desc:
    dw gdt_desc - gdt_start - 1
    dd gdt_start

; === PADDING ===
times 4096-($-$$) db 0