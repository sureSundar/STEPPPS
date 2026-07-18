; TBOS Interactive GUI - Sprint 17
; 🕉️ Makara Jyothi Saranam 🕉️
; Rich interactive graphics interface

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

    ; Show text loading screen first
    call show_loading_screen

    ; Switch to VGA graphics mode 13h (320x200x256)
    mov ax, 0x0013
    int 0x10

    ; Enable A20
    call enable_a20

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Far jump to 32-bit GUI
    jmp 0x08:gui_main

;==========================================
; 16-bit Functions
;==========================================
show_loading_screen:
    ; Clear screen
    mov ax, 0x0003
    int 0x10

    ; Show loading messages
    mov si, msg_loading1
    call print_16
    call delay_short

    mov si, msg_loading2
    call print_16
    call delay_short

    mov si, msg_loading3
    call print_16
    call delay_long

    ret

print_16:
    pusha
.loop:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    popa
    ret

delay_short:
    mov cx, 0x3FFF
.wait:
    nop
    loop .wait
    ret

delay_long:
    mov cx, 0xFFFF
.wait:
    nop
    loop .wait
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
    dd 0x00000000, 0x00000000  ; Null descriptor
    dd 0x0000FFFF, 0x00CF9A00  ; Code segment
    dd 0x0000FFFF, 0x00CF9200  ; Data segment
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

;==========================================
; 32-bit GUI System
;==========================================
[BITS 32]
gui_main:
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Initialize GUI system
    call init_gui
    call draw_desktop
    call main_gui_loop

init_gui:
    ; Clear video memory
    mov edi, 0xA0000
    mov ecx, 64000
    xor eax, eax
    rep stosb

    ; Set up palette for better colors
    call setup_palette
    ret

setup_palette:
    ; Enhanced color palette
    mov dx, 0x3C8   ; Palette register
    xor al, al      ; Start with color 0
    out dx, al

    mov dx, 0x3C9   ; Palette data
    ; Color 0: Black
    xor al, al
    out dx, al
    out dx, al
    out dx, al

    ; Color 1: Dark Blue
    mov al, 0
    out dx, al
    mov al, 0
    out dx, al
    mov al, 42
    out dx, al

    ; Continue setting up more colors...
    ret

draw_desktop:
    ; Draw gradient sky background
    call draw_sky

    ; Draw ground/floor
    call draw_ground

    ; Draw TBOS logo
    call draw_tbos_logo

    ; Draw STEPPPS indicators
    call draw_steppps_status

    ; Draw system info panel
    call draw_system_panel

    ; Draw interactive elements
    call draw_buttons

    ret

draw_sky:
    ; Sky gradient from top to middle
    mov edi, 0xA0000
    mov edx, 100    ; Sky height

.sky_rows:
    mov ecx, 320    ; Screen width
    mov al, dl      ; Color based on row
    shr al, 3       ; Divide by 8 for gradient
    add al, 9       ; Offset to blue colors

.sky_pixels:
    stosb
    loop .sky_pixels

    dec edx
    jnz .sky_rows
    ret

draw_ground:
    ; Green ground from middle to bottom
    mov edi, 0xA0000 + (320 * 100)
    mov edx, 100    ; Ground height

.ground_rows:
    mov ecx, 320
    mov al, 2       ; Green color

.ground_pixels:
    stosb
    loop .ground_pixels

    dec edx
    jnz .ground_rows
    ret

draw_tbos_logo:
    ; Large TBOS logo in center
    mov edi, 0xA0000 + (320 * 20) + 100

    ; Draw "TBOS" in large pixels
    call draw_letter_T
    add edi, 20
    call draw_letter_B
    add edi, 20
    call draw_letter_O
    add edi, 20
    call draw_letter_S

    ret

draw_letter_T:
    ; Draw letter T
    push edi
    mov al, 15      ; White
    mov cx, 15      ; Width
.top_line:
    mov [edi], al
    inc edi
    loop .top_line

    pop edi
    add edi, 6      ; Center position
    mov cx, 20      ; Height
.vertical_line:
    mov [edi], al
    add edi, 320    ; Next row
    loop .vertical_line
    ret

draw_letter_B:
    ; Draw letter B (simplified)
    mov al, 15
    mov cx, 25
.b_left:
    mov [edi], al
    add edi, 320
    loop .b_left
    ret

draw_letter_O:
    ; Draw letter O (circle)
    mov al, 15
    ; Draw top/bottom
    mov cx, 10
.o_top:
    mov [edi], al
    inc edi
    loop .o_top

    ; Draw sides (simplified)
    mov cx, 20
.o_sides:
    mov [edi], al
    add edi, 320
    mov [edi-10], al
    loop .o_sides
    ret

draw_letter_S:
    ; Draw letter S (simplified)
    mov al, 15
    mov cx, 15
.s_line:
    mov [edi], al
    inc edi
    loop .s_line
    ret

draw_steppps_status:
    ; Draw 7 STEPPPS dimension indicators
    mov edi, 0xA0000 + (320 * 170)
    mov ebx, 7      ; 7 dimensions

.dimension_loop:
    ; Draw colored rectangle for each dimension
    push ebx
    mov ecx, 40     ; Width
    mov edx, 10     ; Height
    mov al, bl      ; Color based on dimension number

.draw_rect:
    push ecx
    push edi
.rect_row:
    mov [edi], al
    inc edi
    loop .rect_row
    pop edi
    add edi, 320
    pop ecx
    dec edx
    jnz .draw_rect

    pop ebx
    add edi, 5      ; Space between indicators
    dec ebx
    jnz .dimension_loop

    ret

draw_system_panel:
    ; System info panel in corner
    mov edi, 0xA0000 + (320 * 10) + 250

    ; Panel background
    mov edx, 40     ; Height
.panel_rows:
    mov ecx, 60     ; Width
    mov al, 8       ; Gray
.panel_pixels:
    mov [edi], al
    inc edi
    loop .panel_pixels
    add edi, 260    ; Next row
    dec edx
    jnz .panel_rows

    ; Add some "text" patterns
    mov edi, 0xA0000 + (320 * 15) + 255
    mov al, 15      ; White text
    mov cx, 50
.text_pixels:
    mov [edi], al
    add edi, 2
    loop .text_pixels

    ret

draw_buttons:
    ; Interactive buttons at bottom
    mov edi, 0xA0000 + (320 * 180) + 20

    ; Button 1: STEPPPS
    call draw_button
    add edi, 80

    ; Button 2: Status
    call draw_button
    add edi, 80

    ; Button 3: Shutdown
    call draw_button

    ret

draw_button:
    ; Draw a raised button
    push edi
    mov al, 12      ; Light color
    mov ecx, 60     ; Width
    mov edx, 15     ; Height

.button_rows:
    push ecx
    push edi
.button_pixels:
    mov [edi], al
    inc edi
    loop .button_pixels
    pop edi
    add edi, 320
    pop ecx
    dec edx
    jnz .button_rows

    pop edi
    ret

main_gui_loop:
    ; Animate elements
    call animate_steppps
    call animate_cursor
    call animate_background

    ; Wait between frames
    call gui_delay

    jmp main_gui_loop

animate_steppps:
    ; Animate STEPPPS indicators
    inc dword [animation_counter]
    mov eax, [animation_counter]
    and eax, 255

    ; Change colors cyclically
    mov edi, 0xA0000 + (320 * 170)
    mov ebx, 7

.animate_loop:
    push ebx
    mov ecx, 40
    mov al, bl
    add al, byte [animation_counter]
    and al, 15      ; Keep in color range

.animate_rect:
    mov [edi], al
    inc edi
    loop .animate_rect

    add edi, 5
    pop ebx
    dec ebx
    jnz .animate_loop

    ret

animate_cursor:
    ; Bouncing cursor
    mov eax, [cursor_x]
    add eax, [cursor_dx]
    cmp eax, 310
    jl .no_bounce_x
    neg dword [cursor_dx]
    mov eax, 310
.no_bounce_x:
    cmp eax, 10
    jg .store_x
    neg dword [cursor_dx]
    mov eax, 10
.store_x:
    mov [cursor_x], eax

    mov eax, [cursor_y]
    add eax, [cursor_dy]
    cmp eax, 190
    jl .no_bounce_y
    neg dword [cursor_dy]
    mov eax, 190
.no_bounce_y:
    cmp eax, 10
    jg .store_y
    neg dword [cursor_dy]
    mov eax, 10
.store_y:
    mov [cursor_y], eax

    ; Draw cursor
    mov edi, 0xA0000
    mov eax, [cursor_y]
    imul eax, 320
    add eax, [cursor_x]
    add edi, eax

    mov al, 15      ; White cursor
    mov [edi], al
    mov [edi+1], al
    mov [edi+320], al
    mov [edi+321], al

    ret

animate_background:
    ; Subtle background animation
    mov eax, [animation_counter]
    and eax, 511    ; Slow cycle

    ; Animate some sky pixels
    mov edi, 0xA0000 + (320 * 10)
    mov ecx, 100
.bg_animate:
    mov al, byte [animation_counter]
    shr al, 3
    add al, 9
    mov [edi], al
    add edi, 7
    loop .bg_animate

    ret

gui_delay:
    mov ecx, 100000
.delay_loop:
    nop
    loop .delay_loop
    ret

;==========================================
; Data Section
;==========================================
; 16-bit messages
msg_loading1 db 'TBOS GUI System Loading...', 13, 10, 0
msg_loading2 db 'Initializing STEPPPS Graphics...', 13, 10, 0
msg_loading3 db 'Entering Sacred GUI Mode...', 13, 10, 0

; GUI variables
animation_counter dd 0
cursor_x dd 160
cursor_y dd 100
cursor_dx dd 2
cursor_dy dd 1

; Padding
times 4096-($-$$) db 0