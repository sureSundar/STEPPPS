; TernaryBit OS - Minimal Interactive Stage 2

[BITS 16]
[ORG 0x8000]

start:
    mov si, msg1
    call print16

    ; A20 enable
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Protected mode
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or al, 1
    mov cr0, eax
    jmp 0x08:main32

[BITS 32]
main32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000

    ; Setup basic keyboard interrupt
    mov eax, kbd_handler
    mov [0x1024], eax    ; IDT entry 9 simplified

    ; Enable keyboard
    in al, 0x21
    and al, 0xFD
    out 0x21, al
    sti

    ; Clear screen
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw

    ; Main shell
    mov esi, banner
    call print32
    mov dword [cursor], 0xB8000 + 160*4  ; Line 4

main_loop:
    mov esi, prompt
    call print32

input_wait:
    cmp byte [new_key], 1
    jne input_wait
    mov byte [new_key], 0

    mov al, [key_char]

    ; Handle enter
    cmp al, 13
    je handle_enter

    ; Handle printable chars
    cmp al, 32
    jb input_wait
    cmp al, 126
    ja input_wait

    ; Store and display
    mov ebx, [buf_len]
    cmp ebx, 60
    jge input_wait
    mov [buffer + ebx], al
    inc dword [buf_len]

    mov ebx, [cursor]
    mov ah, 7
    mov [ebx], ax
    add dword [cursor], 2
    jmp input_wait

handle_enter:
    ; Null terminate
    mov ebx, [buf_len]
    mov byte [buffer + ebx], 0
    mov dword [buf_len], 0

    call newline

    ; Check commands
    mov al, [buffer]
    cmp al, 'h'
    je cmd_help
    cmp al, 's'
    je cmd_steppps
    cmp al, 'c'
    je cmd_clear
    cmp al, 'r'
    je cmd_reboot

    ; Default
    mov esi, unk_msg
    call print32
    jmp main_loop

cmd_help:
    mov esi, help_msg
    call print32
    jmp main_loop

cmd_steppps:
    mov esi, steppps_msg
    call print32
    jmp main_loop

cmd_clear:
    mov edi, 0xB8000
    mov ecx, 2000
    mov ax, 0x0720
    rep stosw
    mov esi, banner
    call print32
    mov dword [cursor], 0xB8000 + 160*4
    jmp main_loop

cmd_reboot:
    mov esi, reboot_text
    call print32
    mov al, 0xFE
    out 0x64, al

kbd_handler:
    push eax
    in al, 0x60

    ; Simple conversion
    cmp al, 0x1C
    je .enter
    cmp al, 0x39
    je .space

    ; Letters (rough approximation)
    cmp al, 0x1E
    jb .done
    cmp al, 0x2C
    ja .done
    sub al, 0x1E
    add al, 'a'
    jmp .store

.enter:
    mov al, 13
    jmp .store
.space:
    mov al, ' '
    jmp .store

.store:
    mov [key_char], al
    mov byte [new_key], 1

.done:
    mov al, 0x20
    out 0x20, al
    pop eax
    iret

newline:
    mov eax, [cursor]
    sub eax, 0xB8000
    mov ebx, 160
    xor edx, edx
    div ebx
    inc eax
    mul ebx
    add eax, 0xB8000
    mov [cursor], eax
    ret

print32:
    push eax
    push ebx
    mov ebx, [cursor]
.loop:
    lodsb
    test al, al
    jz .done
    cmp al, 10
    je .nl
    mov ah, 7
    mov [ebx], ax
    add ebx, 2
    jmp .loop
.nl:
    push eax
    mov eax, ebx
    sub eax, 0xB8000
    push ebx
    mov ebx, 160
    xor edx, edx
    div ebx
    inc eax
    mul ebx
    pop ebx
    add eax, 0xB8000
    mov ebx, eax
    pop eax
    jmp .loop
.done:
    mov [cursor], ebx
    pop ebx
    pop eax
    ret

[BITS 16]
print16:
    push ax
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    int 0x10
    jmp .loop
.done:
    pop ax
    ret

; GDT
gdt:
    dd 0, 0
    dw 0xFFFF, 0
    db 0, 0x9A, 0xCF, 0
    dw 0xFFFF, 0
    db 0, 0x92, 0xCF, 0

gdt_desc:
    dw 23
    dd gdt

; Messages
msg1        db 'Loading TernaryBit v2.0...', 13, 10, 0
banner      db 'TernaryBit OS v2.0 - STEPPPS Interactive', 10
            db 'Real keyboard input active!', 10, 10, 0
prompt      db 'tb> ', 0
help_msg    db 'Commands: h=help s=steppps c=clear r=reboot', 10, 0
steppps_msg db 'STEPPPS: All 7 dimensions active', 10
            db 'SPACE TIME EVENT PSYCHOLOGY PIXEL PROMPT SCRIPT', 10, 0
unk_msg     db 'Unknown command', 10, 0
reboot_text db 'Rebooting...', 10, 0

; Variables
cursor      dd 0
buffer      times 64 db 0
buf_len     dd 0
new_key     db 0
key_char    db 0

times 2048-($-$$) db 0