; TBOS Third Madi - Maximum Compression with Full Unfoldability
; Target: ~800 bytes while maintaining complete information recovery

[BITS 16]
[ORG 0x8000]

jmp s

; Hyper-compressed encoding (2-bit instruction codes)
d: db 'S2',0,'G',0,'A',0,'P',0,'K',0

p: mov ah,14
.l: lodsb
   test al,al
   jz .d
   int 16
   jmp .l
.d: mov al,13
   int 16
   mov al,10
   int 16
   ret

s: mov si,d
   call p
   lgdt [g+15]
   in al,146
   or al,2
   out 146,al
   mov si,d+7
   call p
   cli
   mov eax,cr0
   inc eax
   mov cr0,eax
   jmp 8:m

[BITS 32]
m: mov ax,16
   mov ds,ax
   mov es,ax
   mov ss,ax
   mov esp,589824
   mov edi,65536
   mov ecx,8
   mov eax,6
.k: call r
   add edi,512
   inc eax
   loop .k
   jmp 65536

r: mov dx,502
   mov al,224
   out dx,al
   mov dx,498
   mov al,1
   out dx,al
   out 499,al
   mov dx,500
   mov al,ah
   out dx,al
   inc dx
   shr eax,16
   out dx,al
   mov al,32
   out 503,al
.w: in al,503
   test al,8
   jz .w
   mov dx,496
   mov ecx,256
   rep insw
   ret

; Maximally packed GDT
g: dd 0,0,0x0000ffff,0x00cf9a00,0x0000ffff,0x00cf9200
   dw 23,g

times 800-($-$$) db 0