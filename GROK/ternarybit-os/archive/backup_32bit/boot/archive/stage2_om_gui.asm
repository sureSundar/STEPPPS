; TBOS Sacred OM GUI - Sprint 17
; 🕉️ OM NAMAH SHIVAYA 🕉️
; Beautiful Sanskrit OM as first screen

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

    ; Enable A20
    call enable_a20

    ; Load GDT
    lgdt [gdt_descriptor]

    ; Enter protected mode for graphics
    cli
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; Jump to OM graphics
    jmp 0x08:om_main

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
; 32-bit OM Graphics
;==========================================
[BITS 32]
om_main:
    ; Setup segments
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Main OM display sequence
    call setup_sacred_palette
    call draw_cosmic_background
    call draw_illuminated_om
    call add_golden_effects
    call animate_om_forever

setup_sacred_palette:
    ; Set up palette for spiritual colors
    mov dx, 0x3C8   ; Palette register

    ; Color 0: Deep cosmic black
    xor al, al
    out dx, al
    mov dx, 0x3C9
    xor al, al
    out dx, al
    out dx, al
    out dx, al

    ; Color 1: Deep blue cosmos
    mov dx, 0x3C8
    mov al, 1
    out dx, al
    mov dx, 0x3C9
    mov al, 0
    out dx, al
    mov al, 5
    out dx, al
    mov al, 25
    out dx, al

    ; Color 2: Sacred orange/saffron
    mov dx, 0x3C8
    mov al, 2
    out dx, al
    mov dx, 0x3C9
    mov al, 50
    out dx, al
    mov al, 25
    out dx, al
    mov al, 0
    out dx, al

    ; Color 3: Golden yellow
    mov dx, 0x3C8
    mov al, 3
    out dx, al
    mov dx, 0x3C9
    mov al, 63
    out dx, al
    mov al, 63
    out dx, al
    mov al, 0
    out dx, al

    ; Color 4: Bright white/light
    mov dx, 0x3C8
    mov al, 4
    out dx, al
    mov dx, 0x3C9
    mov al, 63
    out dx, al
    mov al, 63
    out dx, al
    mov al, 63
    out dx, al

    ; Color 5: Lotus pink
    mov dx, 0x3C8
    mov al, 5
    out dx, al
    mov dx, 0x3C9
    mov al, 63
    out dx, al
    mov al, 30
    out dx, al
    mov al, 40
    out dx, al

    ret

draw_cosmic_background:
    ; Draw cosmic starfield background
    mov edi, 0xA0000
    mov ecx, 64000

    ; Fill with deep cosmic black
    xor al, al
.fill_space:
    stosb
    loop .fill_space

    ; Add stars
    mov edi, 0xA0000
    mov ecx, 500    ; Number of stars
.star_loop:
    push ecx

    ; Random star position (simplified)
    mov eax, ecx
    imul eax, 67    ; Pseudo-random
    and eax, 63999  ; Keep in screen bounds
    add edi, eax

    ; Star color (white or golden)
    mov al, 4       ; Bright white
    test ecx, 1
    jz .white_star
    mov al, 3       ; Golden
.white_star:
    mov [edi], al

    pop ecx
    mov edi, 0xA0000
    loop .star_loop

    ret

draw_illuminated_om:
    ; Draw the sacred OM symbol in center
    ; OM center position: (160, 100)
    mov edi, 0xA0000 + (320 * 60) + 120

    ; Draw the main curve of OM (simplified but recognizable)
    call draw_om_main_curve
    call draw_om_dot
    call draw_om_crescent
    call draw_om_top_symbol

    ret

draw_om_main_curve:
    ; Draw the main "3" shaped curve of OM
    push edi

    ; Upper curve
    mov esi, edi
    mov al, 3       ; Golden color
    mov ecx, 30     ; Width
.upper_curve:
    mov [esi], al
    mov [esi+1], al
    inc esi
    loop .upper_curve

    ; Middle horizontal line
    add edi, 320 * 15
    mov esi, edi
    mov ecx, 25
.middle_line:
    mov [esi], al
    inc esi
    loop .middle_line

    ; Lower curve
    add edi, 320 * 15
    mov esi, edi
    mov ecx, 30
.lower_curve:
    mov [esi], al
    inc esi
    loop .lower_curve

    ; Vertical connector
    pop edi
    add edi, 5
    mov ecx, 30     ; Height
.vertical:
    mov [edi], al
    mov [edi+1], al
    add edi, 320
    loop .vertical

    ret

draw_om_dot:
    ; Draw the sacred dot (bindu) above OM
    mov edi, 0xA0000 + (320 * 40) + 150

    mov al, 4       ; Bright white
    ; Draw circular dot
    mov [edi], al
    mov [edi+1], al
    mov [edi+2], al
    add edi, 320
    mov [edi], al
    mov [edi+1], al
    mov [edi+2], al
    add edi, 320
    mov [edi], al
    mov [edi+1], al
    mov [edi+2], al

    ret

draw_om_crescent:
    ; Draw the crescent below the dot
    mov edi, 0xA0000 + (320 * 48) + 145

    mov al, 5       ; Lotus pink
    ; Draw crescent shape
    mov ecx, 12
.crescent:
    mov [edi], al
    inc edi
    loop .crescent

    ret

draw_om_top_symbol:
    ; Draw the small symbol at top
    mov edi, 0xA0000 + (320 * 35) + 155

    mov al, 2       ; Saffron
    mov [edi], al
    add edi, 320
    mov [edi-1], al
    mov [edi], al
    mov [edi+1], al

    ret

add_golden_effects:
    ; Add glowing effects around OM
    call draw_om_aura
    call draw_sanskrit_text
    call draw_sacred_geometry

    ret

draw_om_aura:
    ; Draw glowing aura around OM
    mov edi, 0xA0000 + (320 * 50) + 110

    mov edx, 40     ; Aura height
.aura_rows:
    mov ecx, 40     ; Aura width
    mov al, 1       ; Deep blue
    push edi

.aura_pixels:
    ; Fade effect based on distance from center
    mov bl, cl
    shr bl, 2
    add al, bl
    and al, 7       ; Keep in range
    mov [edi], al
    inc edi
    loop .aura_pixels

    pop edi
    add edi, 320
    dec edx
    jnz .aura_rows

    ret

draw_sanskrit_text:
    ; Draw "OM NAMAH SHIVAYA" below the symbol
    mov edi, 0xA0000 + (320 * 130) + 100

    ; Simplified Sanskrit-style text representation
    mov al, 3       ; Golden
    mov ecx, 120    ; Text width
.text_line1:
    mov [edi], al
    add edi, 2
    loop .text_line1

    ; Second line
    mov edi, 0xA0000 + (320 * 140) + 80
    mov ecx, 160
.text_line2:
    mov [edi], al
    add edi, 2
    loop .text_line2

    ret

draw_sacred_geometry:
    ; Draw sacred geometric patterns around OM
    ; Lotus petals
    call draw_lotus_petals

    ; Mandala circles
    call draw_mandala_circles

    ret

draw_lotus_petals:
    ; Draw stylized lotus petals around OM
    mov edi, 0xA0000 + (320 * 100) + 80

    mov al, 5       ; Lotus pink
    mov ebx, 8      ; Number of petals

.petal_loop:
    push ebx
    push edi

    ; Draw petal shape (simplified)
    mov ecx, 15     ; Petal length
.petal_pixels:
    mov [edi], al
    add edi, 319    ; Diagonal direction
    loop .petal_pixels

    pop edi
    add edi, 10     ; Space between petals
    pop ebx
    dec ebx
    jnz .petal_loop

    ret

draw_mandala_circles:
    ; Draw concentric circles for mandala effect
    mov edi, 0xA0000 + (320 * 100) + 160

    mov ebx, 3      ; Number of circles
    mov al, 1       ; Starting color

.circle_loop:
    push ebx
    push edi

    ; Draw circle (simplified as square)
    mov ecx, 20
    mov edx, 20
.circle_rows:
    push ecx
    push edi
.circle_pixels:
    mov [edi], al
    inc edi
    loop .circle_pixels
    pop edi
    add edi, 320
    pop ecx
    dec edx
    jnz .circle_rows

    pop edi
    sub edi, 320 * 5 + 5  ; Move inward
    inc al              ; Next color
    pop ebx
    dec ebx
    jnz .circle_loop

    ret

animate_om_forever:
    ; Eternal OM animation
.eternal_loop:
    call pulse_om_glow
    call rotate_aura
    call twinkle_stars
    call meditation_delay
    jmp .eternal_loop

pulse_om_glow:
    ; Pulse the OM symbol with golden light
    inc dword [pulse_counter]
    mov eax, [pulse_counter]
    and eax, 63     ; Cycle every 64 frames

    ; Calculate intensity
    cmp eax, 32
    jl .rising
    neg eax
    add eax, 63
.rising:
    shr eax, 2      ; Reduce intensity
    add eax, 3      ; Base golden color

    ; Apply glow to OM
    mov edi, 0xA0000 + (320 * 60) + 120
    mov ecx, 30
.glow_apply:
    mov [edi], al
    inc edi
    loop .glow_apply

    ret

rotate_aura:
    ; Rotate the aura colors
    inc dword [rotation_counter]
    mov eax, [rotation_counter]
    shr eax, 3      ; Slow rotation

    mov edi, 0xA0000 + (320 * 50) + 110
    mov edx, 40

.rotate_rows:
    mov ecx, 40
    push edi
.rotate_pixels:
    mov bl, cl
    add bl, al      ; Add rotation offset
    and bl, 7       ; Color cycle
    inc bl          ; Avoid black
    mov [edi], bl
    inc edi
    loop .rotate_pixels

    pop edi
    add edi, 320
    dec edx
    jnz .rotate_rows

    ret

twinkle_stars:
    ; Make stars twinkle
    mov ecx, 50     ; Number of stars to update
.twinkle_loop:
    push ecx

    ; Pseudo-random star position
    mov eax, ecx
    imul eax, [pulse_counter]
    and eax, 63999

    mov edi, 0xA0000
    add edi, eax

    ; Twinkle effect
    mov al, [edi]
    cmp al, 0       ; Only twinkle existing stars
    jz .skip_twinkle

    inc al
    and al, 7       ; Cycle brightness
    mov [edi], al

.skip_twinkle:
    pop ecx
    loop .twinkle_loop

    ret

meditation_delay:
    ; Peaceful meditation delay
    mov ecx, 200000
.meditate:
    nop
    loop .meditate
    ret

;==========================================
; Sacred Data
;==========================================
pulse_counter    dd 0
rotation_counter dd 0

; Sacred mantras (not displayed but present)
mantra1 db 'OM NAMAH SHIVAYA', 0
mantra2 db 'GATE GATE PARAGATE PARASAMGATE BODHI SVAHA', 0
mantra3 db 'OM MANI PADME HUM', 0

; Padding
times 4096-($-$$) db 0