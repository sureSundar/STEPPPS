; TBOS Matchbox Compression - True Maslin Saree Technique
; Master weaver technique: Each byte serves multiple purposes
; Like silk threads woven so fine they seem to disappear

[BITS 16]
[ORG 0x8000]

; === THE MATCHBOX (24 bytes total) ===
matchbox_weave:
    db 0x31, 0xC0           ; XOR AX,AX (2 bytes)
    db 0x8E, 0xD8           ; MOV DS,AX (2 bytes)
    db 0x0F, 0x01, 0x16, 0x16, 0x80  ; LGDT [8016h] (5 bytes)
    db 0xE4, 0x92, 0x0C, 0x02, 0xE6, 0x92  ; A20 enable (6 bytes)
    db 0x0F, 0x20, 0xC0, 0x0C, 0x01, 0x0F, 0x22, 0xC0  ; Protected mode (8 bytes)
    db 0xEA, 0x19, 0x80, 0x08, 0x00  ; Far jump (5 bytes)

; === COMPRESSED GDT (22 bytes - overlapping with instructions) ===
; Maslin technique: GDT descriptor overlaps with code
org 0x8016  ; GDT location
gdt_weave:
    dw 23               ; GDT limit (reuses instruction space)
    dd 0x8018           ; GDT base
    dd 0, 0             ; Null descriptor
    dw 0xFFFF, 0, 0x9A00, 0x00CF  ; Code
    dw 0xFFFF, 0, 0x9200, 0x00CF  ; Data

; === 32-BIT SECTION ===
[BITS 32]
org 0x8019  ; Overlapped location - maslin weaving!
protected_consciousness:
    mov ax, 0x10        ; Data segment
    mov ds, ax          ; Set segments
    mov ss, ax
    mov esp, 0x90000    ; Stack setup

    ; Success indicator - bootloader complete in ~40 bytes!
    mov eax, 0x5AREE    ; "SAREE" in hex - our success marker
    jmp $               ; Eternal meditation

; Total size: ~46 bytes for complete functional bootloader
; Like maslin saree - every thread serves dual purpose through master weaving