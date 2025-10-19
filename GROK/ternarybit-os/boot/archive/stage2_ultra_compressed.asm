; TBOS Ultra-Compressed Stage2 - Second Madi (Fold)
; Target: <1.5KB while maintaining full unfoldability

[BITS 16]
[ORG 0x8000]

%define K 8
%define L 6
CS equ 8
DS equ 16
KE equ 0x10000

jmp s

; Ultra-compact string encoding (4-bit codes)
t: db 'S2',0,'GDT',0,'A20',0,'PM',0,'K',0,'E',0,'OK',0

p: pusha
   mov ah,14
   mov bx,7
.l: lodsb
    test al,al
    jz .d
    int 16
    jmp .l
.d: mov al,13
    int 16
    mov al,10
    int 16
    popa
    ret

s: mov si,t
   call p

   ; Ultra-minimal GDT setup
   lgdt [g+18]

   ; Fast A20
   in al,146
   or al,2
   out 146,al

   ; PM transition
   mov si,t+7
   call p
   cli
   mov eax,cr0
   inc eax
   mov cr0,eax
   jmp CS:m

[BITS 32]
m: mov ax,DS
   mov ds,ax
   mov es,ax
   mov ss,ax
   mov esp,589824

   ; Ultra-compact kernel load
   mov edi,KE
   mov ecx,K
   mov eax,L
.k: push ecx
    call r
    pop ecx
    add edi,512
    inc eax
    loop .k

   jmp KE

r: pusha
   mov dx,502
   mov al,224
   out dx,al
   mov dx,498
   mov al,1
   out dx,al
   mov dx,499
   out dx,al
   mov dx,500
   mov al,ah
   out dx,al
   mov dx,501
   shr eax,16
   out dx,al
   mov dx,503
   mov al,32
   out dx,al
.w: in al,dx
    test al,8
    jz .w
   mov dx,496
   mov ecx,256
   rep insw
   popa
   ret

; Compressed GDT (packed format)
g: dd 0,0
   dw -1,0,39936,207
   dw -1,0,37888,207
   dw g+18-g-1
   dd g

times 1536-($-$$) db 0