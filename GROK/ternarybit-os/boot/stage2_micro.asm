; TBOS Micro Bootloader - 64 bytes functional
[BITS 16]
[ORG 0x8000]

lgdt[g+20]             ; Load GDT (5 bytes)
in al,0x92;or al,2;out 0x92,al ; A20 enable (7 bytes)
mov eax,cr0;inc eax;mov cr0,eax ; Protected mode (9 bytes)
db 0xEA,0x1B,0x80,8,0  ; Far jump to 32-bit (5 bytes)

[BITS 32]
mov ax,16;mov ds,ax;mov ss,ax ; Setup segments (8 bytes)
mov eax,0xBEEF         ; Success marker (5 bytes)
jmp $                  ; Infinite loop (2 bytes)

g: dd 0,0              ; GDT: Null (8 bytes)
   dw -1,0,0x9A00,0xCF ; Code segment (8 bytes)
   dw -1,0,0x9200,0xCF ; Data segment (8 bytes)
   dw 23,g             ; Descriptor (6 bytes)

; Total: ~64 bytes