; Universal Bootloader - Stage 2
; Design: Works from minimal hardware to supercomputers
; Phase: Protected Mode Transition
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 16]
[ORG 0x8000]

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
    call print

    ; Load kernel from disk
    call load_kernel

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

print:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x07
    int 0x10
    jmp .loop
.done:
    popa
    ret

load_kernel:
    pusha
    mov si, loading_msg
    call print

    ; Read kernel: 50 sectors starting at sector 10
    mov ax, KERNEL_ADDR >> 4
    mov es, ax          ; ES = 0x1000 (for loading to 0x10000)
    xor bx, bx          ; Offset 0

    mov ah, 0x02        ; Read function
    mov al, 50          ; 50 sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 10          ; Sector 10
    mov dh, 0           ; Head 0
    mov dl, [0x0500]    ; Boot drive (saved by stage1)
    int 0x13
    jc error

    ; CRITICAL: Reset ES to 0 before returning!
    xor ax, ax
    mov es, ax

    mov si, loaded_msg
    call print
    popa
    ret

error:
    mov si, error_msg
    call print
.halt:
    hlt
    jmp .halt

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

    ; Jump to kernel
    jmp KERNEL_ADDR

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

; Pad to 4KB
times 4096-($-$$) db 0
