; TernaryBit OS - Universal Hardware Detection
; Detects CPU, Memory, and Device Class
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 16]

;==========================================
; Hardware Detection Main Entry
;==========================================
hardware_detect_init:
    push ax
    push bx
    push cx
    push dx

    ; Display header
    mov si, msg_hw_detect
    call print_string

    ; Detect CPU
    call detect_cpu

    ; Detect Memory
    call detect_memory

    ; Classify Device
    call classify_device

    ; Display summary
    call display_hardware_summary

    pop dx
    pop cx
    pop bx
    pop ax
    ret

;==========================================
; Detect CPU Type and Cores
;==========================================
detect_cpu:
    ; Display CPU detection message
    mov si, msg_cpu_detect
    call print_string

    ; Check if CPUID is supported (386+ detection)
    ; Try to flip ID bit (bit 21) in EFLAGS
    pushfd
    pop eax
    mov ebx, eax        ; Save original
    xor eax, 0x00200000 ; Flip ID bit
    push eax
    popfd
    pushfd
    pop eax
    cmp eax, ebx
    je .no_cpuid

    ; CPUID is supported
    mov byte [cpu_has_cpuid], 1

    ; Get CPU vendor string
    xor eax, eax
    cpuid
    mov [cpu_vendor], ebx
    mov [cpu_vendor+4], edx
    mov [cpu_vendor+8], ecx

    ; Get CPU features and family
    mov eax, 1
    cpuid
    mov [cpu_signature], eax
    mov [cpu_features_edx], edx
    mov [cpu_features_ecx], ecx

    ; Extract processor type from signature
    ; Bits 12-13 of EAX contain processor type
    mov eax, [cpu_signature]
    shr eax, 12
    and eax, 0x3
    mov [cpu_type], al

    ; Display CPU vendor
    mov si, msg_cpu_vendor
    call print_string
    mov si, cpu_vendor
    call print_string
    call print_newline

    ; Display CPU signature
    mov si, msg_cpu_sig
    call print_string
    mov eax, [cpu_signature]
    call print_hex_dword
    call print_newline

    jmp .done

.no_cpuid:
    ; No CPUID - likely 8086/286/386
    mov byte [cpu_has_cpuid], 0
    mov si, msg_cpu_legacy
    call print_string
    call print_newline

.done:
    ret

;==========================================
; Detect Memory using BIOS INT 15h E820
;==========================================
detect_memory:
    mov si, msg_mem_detect
    call print_string

    ; Try E820 first (modern method)
    call detect_memory_e820

    ; Display total memory
    mov si, msg_mem_total
    call print_string
    mov eax, [total_memory_kb]
    call print_hex_dword
    mov si, msg_kb
    call print_string
    call print_newline

    ; Convert to MB and display
    mov eax, [total_memory_kb]
    shr eax, 10             ; Divide by 1024 to get MB
    mov si, msg_mem_mb
    call print_string
    call print_hex_dword
    mov si, msg_mb
    call print_string
    call print_newline

    ret

;==========================================
; E820 Memory Detection
;==========================================
detect_memory_e820:
    push es
    push di

    ; Point ES:DI to memory map buffer
    mov ax, 0
    mov es, ax
    mov di, memory_map

    xor ebx, ebx            ; EBX = 0 for first call
    mov edx, 0x534D4150     ; 'SMAP' signature

.loop:
    mov eax, 0xE820         ; E820 function
    mov ecx, 24             ; Buffer size
    int 0x15                ; BIOS call

    jc .e820_failed         ; If carry, E820 not supported

    ; Check signature
    cmp eax, 0x534D4150
    jne .e820_failed

    ; Process this memory region
    ; Region format:
    ; Offset 0: Base address (8 bytes)
    ; Offset 8: Length (8 bytes)
    ; Offset 16: Type (4 bytes)
    ; Offset 20: Extended attributes (4 bytes)

    ; Get region type (offset 16)
    mov eax, [es:di+16]
    cmp eax, 1              ; Type 1 = available RAM
    jne .next_region

    ; Add this region's length to total
    mov eax, [es:di+8]      ; Low 32 bits of length
    shr eax, 10             ; Convert bytes to KB
    add [total_memory_kb], eax

.next_region:
    add di, 24              ; Move to next entry
    test ebx, ebx           ; EBX = 0 means last entry
    jnz .loop

    pop di
    pop es
    ret

.e820_failed:
    ; Fallback: Use INT 15h AX=E801h
    call detect_memory_e801
    pop di
    pop es
    ret

;==========================================
; E801 Memory Detection (Fallback)
;==========================================
detect_memory_e801:
    mov ax, 0xE801
    int 0x15
    jc .e801_failed

    ; AX = KB between 1MB and 16MB
    ; BX = 64KB blocks above 16MB
    ; Convert to total KB
    movzx eax, ax
    add [total_memory_kb], eax

    movzx eax, bx
    shl eax, 6              ; Multiply by 64 to get KB
    add [total_memory_kb], eax

    ret

.e801_failed:
    ; Last resort: Assume 640KB conventional + 384KB
    mov dword [total_memory_kb], 1024
    ret

;==========================================
; Classify Device Based on Memory
;==========================================
classify_device:
    mov si, msg_classify
    call print_string

    mov eax, [total_memory_kb]

    ; Calculator: < 2KB
    cmp eax, 2
    jl .calculator

    ; Embedded: 2KB - 64KB
    cmp eax, 64
    jl .embedded

    ; Mobile: 64KB - 4MB (4096 KB)
    cmp eax, 4096
    jl .mobile

    ; Desktop: 4MB - 16MB (16384 KB)
    cmp eax, 16384
    jl .desktop

    ; Workstation: 16MB - 32MB
    cmp eax, 32768
    jl .workstation

    ; Server: 32MB - 64MB
    cmp eax, 65536
    jl .server

    ; Cluster: 64MB - 128MB
    cmp eax, 131072
    jl .cluster

    ; Supercomputer: >= 128MB
    jmp .supercomputer

.calculator:
    mov byte [device_class], 0
    mov si, device_class_calculator
    jmp .display

.embedded:
    mov byte [device_class], 1
    mov si, device_class_embedded
    jmp .display

.mobile:
    mov byte [device_class], 2
    mov si, device_class_mobile
    jmp .display

.desktop:
    mov byte [device_class], 3
    mov si, device_class_desktop
    jmp .display

.workstation:
    mov byte [device_class], 4
    mov si, device_class_workstation
    jmp .display

.server:
    mov byte [device_class], 5
    mov si, device_class_server
    jmp .display

.cluster:
    mov byte [device_class], 6
    mov si, device_class_cluster
    jmp .display

.supercomputer:
    mov byte [device_class], 7
    mov si, device_class_supercomputer

.display:
    call print_string
    call print_newline
    ret

;==========================================
; Display Hardware Summary
;==========================================
display_hardware_summary:
    mov si, msg_summary
    call print_string

    ; Display device class
    mov si, msg_summary_class
    call print_string
    xor eax, eax
    mov al, [device_class]
    call print_hex_byte
    mov si, msg_bracket
    call print_string

    ; Print class name
    movzx bx, byte [device_class]
    shl bx, 1               ; Multiply by 2 (word size)
    mov si, [class_name_table + bx]
    call print_string
    mov si, msg_close_bracket
    call print_string
    call print_newline

    ; Display memory
    mov si, msg_summary_mem
    call print_string
    mov eax, [total_memory_kb]
    shr eax, 10             ; Convert to MB
    call print_hex_dword
    mov si, msg_mb
    call print_string
    call print_newline

    ; Display CPUID support
    mov si, msg_summary_cpuid
    call print_string
    cmp byte [cpu_has_cpuid], 1
    je .has_cpuid
    mov si, msg_no
    jmp .print_cpuid
.has_cpuid:
    mov si, msg_yes
.print_cpuid:
    call print_string
    call print_newline

    ret

;==========================================
; Utility Functions
;==========================================

; Print newline
print_newline:
    push ax
    push bx
    mov ah, 0x0E
    mov al, 13
    mov bx, 0x0007
    int 0x10
    mov al, 10
    int 0x10
    pop bx
    pop ax
    ret

; Print hex byte (AL)
print_hex_byte:
    push ax
    push bx
    push cx

    mov cl, al
    shr al, 4
    call .print_nibble
    mov al, cl
    and al, 0x0F
    call .print_nibble

    pop cx
    pop bx
    pop ax
    ret

.print_nibble:
    cmp al, 9
    jle .digit
    add al, 'A' - 10
    jmp .print
.digit:
    add al, '0'
.print:
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    ret

; Print hex dword (EAX)
print_hex_dword:
    push eax
    push ebx

    ; Print high word
    shr eax, 16
    call print_hex_word

    ; Print low word
    pop ebx
    push ebx
    mov eax, ebx
    and eax, 0xFFFF
    call print_hex_word

    pop ebx
    pop eax
    ret

; Print hex word (AX)
print_hex_word:
    push ax
    mov al, ah
    call print_hex_byte
    pop ax
    call print_hex_byte
    ret

;==========================================
; Data Section
;==========================================

; Messages
msg_hw_detect:          db '=== Hardware Detection ===', 13, 10, 0
msg_cpu_detect:         db '[CPU] Detecting processor...', 13, 10, 0
msg_cpu_vendor:         db '  Vendor: ', 0
msg_cpu_sig:            db '  Signature: 0x', 0
msg_cpu_legacy:         db '  Legacy CPU (no CPUID)', 0
msg_mem_detect:         db '[MEMORY] Detecting RAM...', 13, 10, 0
msg_mem_total:          db '  Total: 0x', 0
msg_kb:                 db ' KB', 0
msg_mem_mb:             db '  Total: 0x', 0
msg_mb:                 db ' MB', 0
msg_classify:           db '[CLASS] Classifying device...', 13, 10, 0
msg_summary:            db 13, 10, '=== Hardware Summary ===', 13, 10, 0
msg_summary_class:      db '  Device Class: 0x', 0
msg_bracket:            db ' (', 0
msg_close_bracket:      db ')', 0
msg_summary_mem:        db '  Memory (MB): 0x', 0
msg_summary_cpuid:      db '  CPUID Support: ', 0
msg_yes:                db 'Yes', 0
msg_no:                 db 'No', 0

; Device class strings
device_class_calculator:    db '  Class: CALCULATOR (512B-2KB)', 0
device_class_embedded:      db '  Class: EMBEDDED (2KB-64KB)', 0
device_class_mobile:        db '  Class: MOBILE (64KB-4MB)', 0
device_class_desktop:       db '  Class: DESKTOP (4MB-16MB)', 0
device_class_workstation:   db '  Class: WORKSTATION (16MB-32MB)', 0
device_class_server:        db '  Class: SERVER (32MB-64MB)', 0
device_class_cluster:       db '  Class: CLUSTER (64MB-128MB)', 0
device_class_supercomputer: db '  Class: SUPERCOMPUTER (128MB+)', 0

; Device class name table (for summary)
class_name_table:
    dw device_class_calculator
    dw device_class_embedded
    dw device_class_mobile
    dw device_class_desktop
    dw device_class_workstation
    dw device_class_server
    dw device_class_cluster
    dw device_class_supercomputer

; Hardware detection data
cpu_has_cpuid:      db 0
cpu_vendor:         times 13 db 0
cpu_signature:      dd 0
cpu_features_edx:   dd 0
cpu_features_ecx:   dd 0
cpu_type:           db 0

total_memory_kb:    dd 0
device_class:       db 0

; Memory map buffer (for E820)
memory_map:         times 512 db 0
