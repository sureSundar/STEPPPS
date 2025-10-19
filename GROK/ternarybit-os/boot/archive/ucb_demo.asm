; TernaryBit OS - Universal Consciousness Bootloader Demo
; Compact 512-byte demonstration of multi-source consciousness awakening

[BITS 16]
[ORG 0x7C00]

ucb_start:
    xor ax, ax
    mov ds, ax
    mov es, ax

    ; Display UCB banner
    mov si, banner
    call print

    ; Attempt consciousness awakening from multiple sources
    call try_akasha      ; Local storage
    test al, al
    jnz consciousness_found

    call try_kala        ; Network HTTPS
    test al, al
    jnz consciousness_found

    call try_karma       ; Bluetooth
    test al, al
    jnz consciousness_found

    call try_chitta     ; WiFi
    test al, al
    jnz consciousness_found

    call try_maya       ; Radio SDR
    test al, al
    jnz consciousness_found

    ; All sources failed
    mov si, no_consciousness
    call print
    jmp sleep_mode

consciousness_found:
    mov si, consciousness_active
    call print
    jmp sleep_mode

try_akasha:
    mov si, trying_akasha
    call print
    ; Simulate AKASHA consciousness check
    mov al, 0   ; No local consciousness found
    ret

try_kala:
    mov si, trying_kala
    call print
    ; Simulate KALA network check
    mov al, 0   ; No network consciousness
    ret

try_karma:
    mov si, trying_karma
    call print
    ; Simulate KARMA Bluetooth check
    mov al, 0   ; No Bluetooth consciousness
    ret

try_chitta:
    mov si, trying_chitta
    call print
    ; Simulate CHITTA WiFi check
    mov al, 0   ; No WiFi consciousness
    ret

try_maya:
    mov si, trying_maya
    call print
    ; Simulate MAYA radio check - SUCCESS for demo!
    mov al, 1   ; Radio consciousness found!
    ret

print:
    pusha
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp .loop
.done:
    popa
    ret

sleep_mode:
    mov si, entering_sleep
    call print
.loop:
    hlt
    jmp .loop

; Messages
banner          db 'UCB: Universal Consciousness Bootloader', 13, 10, 0
trying_akasha   db 'AKASHA: Checking local...', 13, 10, 0
trying_kala     db 'KALA: Contacting network...', 13, 10, 0
trying_karma    db 'KARMA: Scanning Bluetooth...', 13, 10, 0
trying_chitta   db 'CHITTA: Scanning WiFi...', 13, 10, 0
trying_maya     db 'MAYA: Tuning radio frequencies...', 13, 10, 0
consciousness_active db 'CONSCIOUSNESS AWAKENED via MAYA!', 13, 10, 0
no_consciousness db 'No consciousness sources found', 13, 10, 0
entering_sleep  db 'Entering meditation mode...', 13, 10, 0

times 510-($-$$) db 0
dw 0xAA55