; TBOS GUI Stage 2 - Sprint 16
; 🕉️ Swamiye Saranam Aiyappa 🕉️
; Graphics mode with true GUI

[BITS 16]
[ORG 0x8000]

stage2_start:
    ; Setup segments
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7000
    sti

    ; Switch to VGA graphics mode 13h (320x200x256)
    mov ax, 0x0013
    int 0x10

    ; Display GUI boot message
    call draw_gui_boot

    ; Enable A20
    call enable_a20

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit code
    jmp 0x08:protected_mode

;==========================================
; 16-bit GUI Functions
;==========================================
draw_gui_boot:
    ; Clear screen with gradient background
    mov ax, 0xA000  ; VGA memory segment
    mov es, ax
    xor di, di

    ; Draw gradient background
    mov cx, 64000   ; 320x200 pixels
    mov al, 1       ; Blue color
.fill_bg:
    stosb
    inc al
    and al, 15      ; Cycle through colors 0-15
    loop .fill_bg

    ; Draw TBOS logo box
    call draw_logo_box

    ; Draw sacred symbols
    call draw_symbols

    ; Wait a moment
    mov cx, 0xFFFF
.wait:
    nop
    loop .wait

    ret

draw_logo_box:
    ; Draw a box for TBOS logo (simple rectangle)
    mov ax, 0xA000
    mov es, ax

    ; Draw top border
    mov di, 320 * 50 + 80  ; Start position
    mov cx, 160            ; Width
    mov al, 15             ; White color
.top_border:
    mov [es:di], al
    inc di
    loop .top_border

    ; Draw sides and fill
    mov dx, 40             ; Height
.box_rows:
    mov di, 320 * 50 + 80
    add di, dx
    shl di, 9              ; Multiply by 320
    shr di, 9              ; Restore
    mov [es:di], al        ; Left border
    add di, 159
    mov [es:di], al        ; Right border

    ; Fill center with pattern
    sub di, 79
    mov cx, 78
    mov al, 9              ; Light blue
.fill_row:
    mov [es:di], al
    inc di
    loop .fill_row

    dec dx
    jnz .box_rows

    ret

draw_symbols:
    ; Draw OM symbol (simplified as circles)
    mov ax, 0xA000
    mov es, ax

    ; Draw circles for OM symbol
    mov di, 320 * 70 + 160  ; Center position
    mov al, 14              ; Yellow
    mov cx, 20
.om_circle:
    mov [es:di], al
    add di, 320
    mov [es:di], al
    add di, 320
    mov [es:di], al
    sub di, 640
    inc di
    loop .om_circle

    ret

enable_a20:
    in al, 0x92
    or al, 2
    out 0x92, al
    ret

;==========================================
; GDT
;==========================================
gdt_start:
    ; Null descriptor
    dd 0x00000000
    dd 0x00000000

    ; Code segment (0x08)
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10011010b    ; Access
    db 11001111b    ; Flags + Limit high
    db 0x00         ; Base high

    ; Data segment (0x10)
    dw 0xFFFF       ; Limit low
    dw 0x0000       ; Base low
    db 0x00         ; Base middle
    db 10010010b    ; Access
    db 11001111b    ; Flags + Limit high
    db 0x00         ; Base high

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

;==========================================
; 32-bit GUI Mode
;==========================================
[BITS 32]
protected_mode:
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Continue in graphics mode
    call gui_main

gui_main:
    ; Draw desktop
    call draw_desktop

    ; Draw windows
    call draw_windows

    ; Draw mouse cursor
    call draw_cursor

    ; Main GUI loop
    jmp gui_loop

draw_desktop:
    ; Fill screen with desktop pattern
    mov edi, 0xA0000
    mov ecx, 64000
    mov al, 3        ; Cyan desktop
.fill:
    stosb
    loop .fill

    ; Draw taskbar
    mov edi, 0xA0000 + (320 * 180)  ; Bottom of screen
    mov ecx, 320 * 20                ; Taskbar height
    mov al, 8                        ; Gray taskbar
.taskbar:
    stosb
    loop .taskbar

    ret

draw_windows:
    ; Draw STEPPPS window
    call draw_steppps_window

    ; Draw Ayyappa shrine window
    call draw_shrine_window

    ret

draw_steppps_window:
    ; Window at position (50, 30)
    mov edi, 0xA0000 + (320 * 30) + 50

    ; Window border
    mov ecx, 150    ; Width
    mov al, 15      ; White border
.top_border:
    mov [edi], al
    inc edi
    loop .top_border

    ; Window title bar
    mov edi, 0xA0000 + (320 * 31) + 50
    mov ecx, 150
    mov al, 1       ; Blue title bar
.title_bar:
    mov [edi], al
    inc edi
    loop .title_bar

    ; Window content
    mov edx, 60     ; Height
.window_rows:
    mov edi, 0xA0000 + (320 * 32) + 50
    add edi, edx
    shl edi, 9      ; Multiply by 320
    shr edi, 9
    mov ecx, 150
    mov al, 7       ; Light gray content
.content_row:
    mov [edi], al
    inc edi
    loop .content_row

    dec edx
    jnz .window_rows

    ret

draw_shrine_window:
    ; Small shrine window at (200, 100)
    mov edi, 0xA0000 + (320 * 100) + 200

    ; Draw shrine pattern
    mov edx, 40     ; Height
.shrine_rows:
    mov ecx, 80     ; Width
    mov al, 14      ; Yellow/gold
.shrine_row:
    mov [edi], al
    inc edi
    loop .shrine_row

    add edi, 240    ; Next row
    dec edx
    jnz .shrine_rows

    ret

draw_cursor:
    ; Simple cursor at center
    mov edi, 0xA0000 + (320 * 100) + 160
    mov al, 15      ; White cursor
    mov [edi], al
    add edi, 320
    mov [edi], al
    add edi, 320
    mov [edi], al

    ret

gui_loop:
    ; Animate cursor (simple movement)
    call animate_cursor

    ; Wait
    mov ecx, 1000000
.wait:
    nop
    loop .wait

    jmp gui_loop

animate_cursor:
    ; Simple cursor animation - use global variables

    ; Erase old cursor
    mov edi, 0xA0000
    add edi, [cursor_y]
    shl edi, 6      ; Multiply by 64 (approximate)
    shl edi, 2      ; Multiply by 256 total
    add edi, [cursor_x]
    mov al, 3       ; Desktop color
    mov [edi], al

    ; Move cursor
    inc dword [cursor_x]
    cmp dword [cursor_x], 300
    jl .no_wrap
    mov dword [cursor_x], 20
    inc dword [cursor_y]
    cmp dword [cursor_y], 180
    jl .no_wrap
    mov dword [cursor_y], 20
.no_wrap:

    ; Draw new cursor
    mov edi, 0xA0000
    add edi, [cursor_y]
    shl edi, 6
    shl edi, 2
    add edi, [cursor_x]
    mov al, 15      ; White cursor
    mov [edi], al

    ret

; Data
cursor_x dd 160
cursor_y dd 100

; Padding
times 4096-($-$$) db 0