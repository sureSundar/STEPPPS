; TernaryBit OS - Streamlined Stage 2 for Functional Boot
[BITS 16]
[ORG 0x8000]

stage2_start:
    ; Simple banner
    mov al, 'T'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    mov al, 'B'
    int 0x10
    mov al, 'O'
    int 0x10
    mov al, 'S'
    int 0x10
    mov al, ' '
    int 0x10

    ; === KERNEL LOADING ===
    ; Load kernel at 0x1000:0000 (linear address 0x10000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx

    ; Debug: About to load kernel
    mov al, 'L'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Get boot drive from boot sector (it should be preserved)
    mov dl, [0x0500]    ; Boot drive stored by boot sector

    ; Try loading just 1 sector first to test
    mov ah, 0x02        ; BIOS read sectors
    mov al, 1           ; Load 1 sector
    mov ch, 0           ; Cylinder 0
    mov cl, 10          ; Start at sector 10 (kernel starts at sector 9, so 10 in 1-based)
    mov dh, 0           ; Head 0
    ; dl already set to boot drive
    int 0x13
    jc kernel_error

    ; Successfully loaded test sector
    mov al, '1'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Load more sectors
    mov ah, 0x02        ; BIOS read sectors
    mov al, 10          ; Load 10 more sectors
    mov ch, 0           ; Cylinder 0
    mov cl, 11          ; Continue from sector 11
    mov dh, 0           ; Head 0
    mov dl, [0x0500]    ; Use same boot drive
    int 0x13
    jc kernel_error

    mov al, '2'         ; Second part loaded
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Debug: After second load
    mov al, 'K'         ; Kernel loading complete
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; Debug: About to start protected mode transition
    mov al, 'T'         ; Transition starting
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; === PROTECTED MODE TRANSITION ===
    ; Disable interrupts first
    cli

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enable protected mode
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to protected mode
    jmp 0x08:protected_mode_entry

[BITS 32]
protected_mode_entry:
    ; Set up data segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up stack
    mov esp, 0x90000

    ; Jump to kernel
    jmp 0x10000

    ; Should never reach here
    cli
    hlt


[BITS 16]
kernel_error:
    mov al, 'E'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
hang:
    hlt
    jmp hang

; === GDT ===
gdt_start:
    ; Null descriptor
    dd 0x00000000
    dd 0x00000000
    ; Code segment (0x08)
    dd 0x0000FFFF
    dd 0x00CF9A00
    ; Data segment (0x10)
    dd 0x0000FFFF
    dd 0x00CF9200
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; Padding to 4KB
times 4096-($-$$) db 0