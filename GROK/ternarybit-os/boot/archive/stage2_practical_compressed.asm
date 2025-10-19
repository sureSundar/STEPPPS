; TBOS Practical Compression - Actually Works
; Target: ~128 bytes functional bootloader

[BITS 16]
[ORG 0x8000]

s: mov si, m           ; Print start msg
   call p
   lgdt [g+24]         ; Load GDT
   mov si, m+10        ; Print PM msg
   call p
   in al, 0x92         ; Enable A20
   or al, 2
   out 0x92, al
   cli
   mov eax, cr0        ; Enter protected mode
   inc eax
   mov cr0, eax
   jmp 8:pm

p: mov ah, 14          ; Print function
.l: lodsb
    test al, al
    jz .d
    int 16
    jmp .l
.d: mov al, 13
    int 16
    mov al, 10
    int 16
    ret

[BITS 32]
pm: mov ax, 16         ; Setup segments
    mov ds, ax
    mov ss, ax
    mov esp, 589824

    ; Success loop - in real version would load kernel
    mov eax, 0xC0DE     ; Success indicator
.l: jmp .l

m: db 'S2',0,'PM',0    ; Compressed messages

; Ultra-minimal GDT
g: dd 0,0              ; Null
   dw -1,0,0x9A00,0xCF ; Code
   dw -1,0,0x9200,0xCF ; Data
   dw 23,g             ; Descriptor

times 128-($-$$) db 0  ; Pad to 128 bytes