; TernaryBit OS - Interrupt Service Routines (ISRs)
; 32-bit protected mode stubs for exceptions and IRQs
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 32]

extern default_isr_handler
extern keyboard_isr_handler

global isr_stub_0
global isr_stub_1
global isr_stub_2
global isr_stub_3
global isr_stub_4
global isr_stub_5
global isr_stub_6
global isr_stub_7
global isr_stub_8
global isr_stub_9
global isr_stub_10
global isr_stub_11
global isr_stub_12
global isr_stub_13
global isr_stub_14
global isr_stub_15
global isr_stub_16
global isr_stub_17
global isr_stub_18
global isr_stub_19
global isr_stub_20
global isr_stub_21
global isr_stub_22
global isr_stub_23
global isr_stub_24
global isr_stub_25
global isr_stub_26
global isr_stub_27
global isr_stub_28
global isr_stub_29
global isr_stub_30
global isr_stub_31

global irq_stub_0
global irq_stub_1
global irq_stub_2
global irq_stub_3
global irq_stub_4
global irq_stub_5
global irq_stub_6
global irq_stub_7
global irq_stub_8
global irq_stub_9
global irq_stub_10
global irq_stub_11
global irq_stub_12
global irq_stub_13
global irq_stub_14
global irq_stub_15

; Common ISR handler wrapper
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call default_isr_handler

    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8          ; discard error code + int number
    iret

; Common IRQ handler wrapper (sends EOI)
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov eax, [esp + 52] ; interrupt number
    sub eax, 32
    cmp eax, 1
    je .keyboard
    jmp .default

.keyboard:
    call keyboard_isr_handler
    jmp .send_eoi

.default:
    push eax
    call default_isr_handler
    add esp, 4

.send_eoi:
    mov al, 0x20
    out 0x20, al

    mov eax, [esp + 52]
    sub eax, 32
    cmp eax, 8
    jl .restore
    mov al, 0x20
    out 0xA0, al

.restore:
    pop gs
    pop fs
    pop es
    pop ds
    popa

    add esp, 8
    iret

; Helper macros
%macro ISR_NOERR 1
isr_stub_%1:
    push 0
    push %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERR 1
isr_stub_%1:
    push %1
    jmp isr_common_stub
%endmacro

; Exceptions 0-31
ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31

; IRQ stubs 32-47
%macro IRQ_STUB 2
irq_stub_%1:
    push 0
    push %2
    jmp irq_common_stub
%endmacro

IRQ_STUB 0, 32
IRQ_STUB 1, 33
IRQ_STUB 2, 34
IRQ_STUB 3, 35
IRQ_STUB 4, 36
IRQ_STUB 5, 37
IRQ_STUB 6, 38
IRQ_STUB 7, 39
IRQ_STUB 8, 40
IRQ_STUB 9, 41
IRQ_STUB 10, 42
IRQ_STUB 11, 43
IRQ_STUB 12, 44
IRQ_STUB 13, 45
IRQ_STUB 14, 46
IRQ_STUB 15, 47
