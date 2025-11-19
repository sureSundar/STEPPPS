; TernaryBit OS - Interrupt Service Routines (ISRs)
; Proper interrupt handlers that save/restore CPU state
; 🕉️ Swamiye Saranam Aiyappa 🕉️

[BITS 32]

; External C function declarations
extern default_isr_handler
extern keyboard_isr_handler

; Export ISR symbols
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

; IRQ stubs (32-47)
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

;==========================================
; ISR Common Stub
; Saves CPU state, calls handler, restores state
;==========================================
isr_common_stub:
    ; Save all registers
    pusha                   ; Pushes EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI

    ; Save segment registers
    push ds
    push es
    push fs
    push gs

    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Call C handler
    ; Stack now contains: interrupt number and error code (if any)
    call default_isr_handler

    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    ; Restore all registers
    popa

    ; Clean up error code and interrupt number
    add esp, 8

    ; Return from interrupt
    iret

;==========================================
; IRQ Common Stub
; Same as ISR but sends EOI to PIC
;==========================================
irq_common_stub:
    ; Save all registers
    pusha

    ; Save segment registers
    push ds
    push es
    push fs
    push gs

    ; Load kernel data segment
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Get IRQ number from stack (interrupt number - 32)
    mov eax, [esp + 52]     ; Get interrupt number
    sub eax, 32             ; Convert to IRQ number

    ; Call appropriate IRQ handler
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
    ; Send EOI to PIC
    mov al, 0x20
    out 0x20, al            ; Master PIC

    ; If IRQ >= 8, send EOI to slave PIC too
    mov eax, [esp + 52]
    sub eax, 32
    cmp eax, 8
    jl .restore
    mov al, 0x20
    out 0xA0, al            ; Slave PIC

.restore:
    ; Restore segment registers
    pop gs
    pop fs
    pop es
    pop ds

    ; Restore all registers
    popa

    ; Clean up error code and interrupt number
    add esp, 8

    ; Return from interrupt
    iret

;==========================================
; Exception ISR Stubs (0-31)
; Some push error code, others don't
;==========================================

; ISR 0: Divide by Zero
isr_stub_0:
    push 0              ; Dummy error code
    push 0              ; Interrupt number
    jmp isr_common_stub

; ISR 1: Debug
isr_stub_1:
    push 0
    push 1
    jmp isr_common_stub

; ISR 2: Non-Maskable Interrupt
isr_stub_2:
    push 0
    push 2
    jmp isr_common_stub

; ISR 3: Breakpoint
isr_stub_3:
    push 0
    push 3
    jmp isr_common_stub

; ISR 4: Overflow
isr_stub_4:
    push 0
    push 4
    jmp isr_common_stub

; ISR 5: Bound Range Exceeded
isr_stub_5:
    push 0
    push 5
    jmp isr_common_stub

; ISR 6: Invalid Opcode
isr_stub_6:
    push 0
    push 6
    jmp isr_common_stub

; ISR 7: Device Not Available
isr_stub_7:
    push 0
    push 7
    jmp isr_common_stub

; ISR 8: Double Fault (has error code)
isr_stub_8:
    ; Error code already pushed by CPU
    push 8
    jmp isr_common_stub

; ISR 9: Coprocessor Segment Overrun
isr_stub_9:
    push 0
    push 9
    jmp isr_common_stub

; ISR 10: Invalid TSS (has error code)
isr_stub_10:
    push 10
    jmp isr_common_stub

; ISR 11: Segment Not Present (has error code)
isr_stub_11:
    push 11
    jmp isr_common_stub

; ISR 12: Stack-Segment Fault (has error code)
isr_stub_12:
    push 12
    jmp isr_common_stub

; ISR 13: General Protection Fault (has error code)
isr_stub_13:
    push 13
    jmp isr_common_stub

; ISR 14: Page Fault (has error code)
isr_stub_14:
    push 14
    jmp isr_common_stub

; ISR 15: Reserved
isr_stub_15:
    push 0
    push 15
    jmp isr_common_stub

; ISR 16: x87 Floating Point Exception
isr_stub_16:
    push 0
    push 16
    jmp isr_common_stub

; ISR 17: Alignment Check (has error code)
isr_stub_17:
    push 17
    jmp isr_common_stub

; ISR 18: Machine Check
isr_stub_18:
    push 0
    push 18
    jmp isr_common_stub

; ISR 19: SIMD Floating Point Exception
isr_stub_19:
    push 0
    push 19
    jmp isr_common_stub

; ISR 20: Virtualization Exception
isr_stub_20:
    push 0
    push 20
    jmp isr_common_stub

; ISR 21-29: Reserved
isr_stub_21:
    push 0
    push 21
    jmp isr_common_stub

isr_stub_22:
    push 0
    push 22
    jmp isr_common_stub

isr_stub_23:
    push 0
    push 23
    jmp isr_common_stub

isr_stub_24:
    push 0
    push 24
    jmp isr_common_stub

isr_stub_25:
    push 0
    push 25
    jmp isr_common_stub

isr_stub_26:
    push 0
    push 26
    jmp isr_common_stub

isr_stub_27:
    push 0
    push 27
    jmp isr_common_stub

isr_stub_28:
    push 0
    push 28
    jmp isr_common_stub

isr_stub_29:
    push 0
    push 29
    jmp isr_common_stub

; ISR 30: Security Exception (has error code)
isr_stub_30:
    push 30
    jmp isr_common_stub

; ISR 31: Reserved
isr_stub_31:
    push 0
    push 31
    jmp isr_common_stub

;==========================================
; IRQ Stubs (32-47)
;==========================================

; IRQ 0: System Timer
irq_stub_0:
    push 0
    push 32
    jmp irq_common_stub

; IRQ 1: Keyboard
irq_stub_1:
    push 0
    push 33
    jmp irq_common_stub

; IRQ 2: Cascade
irq_stub_2:
    push 0
    push 34
    jmp irq_common_stub

; IRQ 3: COM2
irq_stub_3:
    push 0
    push 35
    jmp irq_common_stub

; IRQ 4: COM1
irq_stub_4:
    push 0
    push 36
    jmp irq_common_stub

; IRQ 5: LPT2
irq_stub_5:
    push 0
    push 37
    jmp irq_common_stub

; IRQ 6: Floppy
irq_stub_6:
    push 0
    push 38
    jmp irq_common_stub

; IRQ 7: LPT1
irq_stub_7:
    push 0
    push 39
    jmp irq_common_stub

; IRQ 8: RTC
irq_stub_8:
    push 0
    push 40
    jmp irq_common_stub

; IRQ 9: Free
irq_stub_9:
    push 0
    push 41
    jmp irq_common_stub

; IRQ 10: Free
irq_stub_10:
    push 0
    push 42
    jmp irq_common_stub

; IRQ 11: Free
irq_stub_11:
    push 0
    push 43
    jmp irq_common_stub

; IRQ 12: PS/2 Mouse
irq_stub_12:
    push 0
    push 44
    jmp irq_common_stub

; IRQ 13: FPU
irq_stub_13:
    push 0
    push 45
    jmp irq_common_stub

; IRQ 14: Primary ATA
irq_stub_14:
    push 0
    push 46
    jmp irq_common_stub

; IRQ 15: Secondary ATA
irq_stub_15:
    push 0
    push 47
    jmp irq_common_stub
