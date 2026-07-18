; Timed Bootloader for TBOS
; Measures and reports boot time

[BITS 16]
[ORG 0x7C00]

; Memory locations for timing data
%define TIMING_DATA 0x7E00  ; 512 bytes after boot sector
%define TSC_LOW     0x7E00  ; Low 32 bits of TSC
%define TSC_HIGH    0x7E04  ; High 32 bits of TSC

start:
    ; Save boot drive
    mov [boot_drive], dl
    
    ; Record initial timestamp
    rdtsc
    mov [TSC_LOW], eax
    mov [TSC_HIGH], edx
    
    ; Set up segments and stack
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    
    ; Clear screen
    mov ax, 0x0003  ; 80x25 text mode
    int 0x10
    
    ; Print boot message
    mov si, boot_msg
    call print_string
    
    ; Calculate and display boot time
    call measure_boot_time
    
    ; Halt system
    cli
    hlt

; Measure boot time using TSC
measure_boot_time:
    pusha
    
    ; Get current TSC
    rdtsc
    
    ; Subtract initial TSC
    sub eax, [TSC_LOW]
    sbb edx, [TSC_HIGH]
    
    ; Convert to microseconds (assuming 2.5 GHz CPU)
    ; TSC / (2.5 * 1000) = microseconds
    mov ecx, 2500  ; 2.5 * 1000
    div ecx
    
    ; Print time in microseconds
    mov ebx, eax
    mov si, time_msg1
    call print_string
    
    ; Print the time
    mov eax, ebx
    call print_number
    
    mov si, time_msg2
    call print_string
    
    popa
    ret

; Print number in EAX (up to 32 bits)
print_number:
    pusha
    mov ebx, 10     ; Base 10
    mov ecx, 0      ; Digit counter
    
.digit_loop:
    xor edx, edx
    div ebx         ; Divide by 10
    add dl, '0'     ; Convert to ASCII
    push dx         ; Save digit
    inc ecx         ; Increment digit count
    test eax, eax   ; Done?
    jnz .digit_loop
    
.print_loop:
    pop ax          ; Get digit
    mov ah, 0x0E    ; BIOS teletype
    int 0x10        ; Print character
    loop .print_loop
    
    popa
    ret

; Print null-terminated string
print_string:
    pusha
    mov ah, 0x0E    ; BIOS teletype
    mov bh, 0x00    ; Page 0
    mov bl, 0x07    ; White on black
    
.next_char:
    lodsb           ; Load next character
    or al, al       ; Null terminator?
    jz .done
    int 0x10        ; Print character
    jmp .next_char
    
.done:
    popa
    ret

; Data
boot_drive  db 0
boot_msg    db 'TBOS Bootloader (Timed)', 0x0D, 0x0A, 0
time_msg1   db 'Boot time: ', 0
time_msg2   db ' microseconds', 0x0D, 0x0A, 0

; Boot signature
times 510-($-$$) db 0
dw 0xAA55
