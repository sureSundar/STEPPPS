// TBOS Interrupt Handler - Sprint 9
// 🕉️ Tatvamasi Aiyappa 🕉️
// IDT setup and interrupt handling

#include <stdint.h>

// IDT entry structure
typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed)) idt_entry_t;

// IDT pointer structure
typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_ptr_t;

// IDT with 256 entries
idt_entry_t idt[256];
idt_ptr_t idtp;

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);
extern void keyboard_interrupt_handler(void);

// External ISR stubs from isr.asm
extern void isr_stub_0(void);
extern void isr_stub_1(void);
extern void isr_stub_2(void);
extern void isr_stub_3(void);
extern void isr_stub_4(void);
extern void isr_stub_5(void);
extern void isr_stub_6(void);
extern void isr_stub_7(void);
extern void isr_stub_8(void);
extern void isr_stub_9(void);
extern void isr_stub_10(void);
extern void isr_stub_11(void);
extern void isr_stub_12(void);
extern void isr_stub_13(void);
extern void isr_stub_14(void);
extern void isr_stub_15(void);
extern void isr_stub_16(void);
extern void isr_stub_17(void);
extern void isr_stub_18(void);
extern void isr_stub_19(void);
extern void isr_stub_20(void);
extern void isr_stub_21(void);
extern void isr_stub_22(void);
extern void isr_stub_23(void);
extern void isr_stub_24(void);
extern void isr_stub_25(void);
extern void isr_stub_26(void);
extern void isr_stub_27(void);
extern void isr_stub_28(void);
extern void isr_stub_29(void);
extern void isr_stub_30(void);
extern void isr_stub_31(void);

// External IRQ stubs
extern void irq_stub_0(void);
extern void irq_stub_1(void);
extern void irq_stub_2(void);
extern void irq_stub_3(void);
extern void irq_stub_4(void);
extern void irq_stub_5(void);
extern void irq_stub_6(void);
extern void irq_stub_7(void);
extern void irq_stub_8(void);
extern void irq_stub_9(void);
extern void irq_stub_10(void);
extern void irq_stub_11(void);
extern void irq_stub_12(void);
extern void irq_stub_13(void);
extern void irq_stub_14(void);
extern void irq_stub_15(void);

// Exception names for debugging
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Error",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

// Default ISR handler (called from isr.asm)
void default_isr_handler(uint32_t int_no, uint32_t err_code) {
    kernel_print("\n[EXCEPTION] ");
    if (int_no < 32) {
        kernel_print(exception_messages[int_no]);
    } else {
        kernel_print("Unknown Exception");
    }
    kernel_print(" (");
    kernel_print_hex(int_no);
    kernel_print(") Error Code: ");
    kernel_print_hex(err_code);
    kernel_print("\nSystem Halted\n");

    // Halt the system
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

// Keyboard ISR handler (called from isr.asm)
void keyboard_isr_handler(void) {
    // Call the actual keyboard handler
    keyboard_interrupt_handler();
}

// I/O port functions
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Set IDT gate
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
}

// Initialize IDT
void idt_init(void) {
    // Set IDT pointer
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint32_t)&idt;

    // Clear IDT
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    // Set up exception handlers (0-31)
    idt_set_gate(0, (uint32_t)isr_stub_0, 0x08, 0x8E);
    idt_set_gate(1, (uint32_t)isr_stub_1, 0x08, 0x8E);
    idt_set_gate(2, (uint32_t)isr_stub_2, 0x08, 0x8E);
    idt_set_gate(3, (uint32_t)isr_stub_3, 0x08, 0x8E);
    idt_set_gate(4, (uint32_t)isr_stub_4, 0x08, 0x8E);
    idt_set_gate(5, (uint32_t)isr_stub_5, 0x08, 0x8E);
    idt_set_gate(6, (uint32_t)isr_stub_6, 0x08, 0x8E);
    idt_set_gate(7, (uint32_t)isr_stub_7, 0x08, 0x8E);
    idt_set_gate(8, (uint32_t)isr_stub_8, 0x08, 0x8E);
    idt_set_gate(9, (uint32_t)isr_stub_9, 0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr_stub_10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr_stub_11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr_stub_12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr_stub_13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr_stub_14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr_stub_15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr_stub_16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr_stub_17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr_stub_18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr_stub_19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr_stub_20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr_stub_21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr_stub_22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr_stub_23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr_stub_24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr_stub_25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr_stub_26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr_stub_27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr_stub_28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr_stub_29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr_stub_30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr_stub_31, 0x08, 0x8E);

    // Set up IRQ handlers (32-47)
    idt_set_gate(32, (uint32_t)irq_stub_0, 0x08, 0x8E);
    idt_set_gate(33, (uint32_t)irq_stub_1, 0x08, 0x8E);
    idt_set_gate(34, (uint32_t)irq_stub_2, 0x08, 0x8E);
    idt_set_gate(35, (uint32_t)irq_stub_3, 0x08, 0x8E);
    idt_set_gate(36, (uint32_t)irq_stub_4, 0x08, 0x8E);
    idt_set_gate(37, (uint32_t)irq_stub_5, 0x08, 0x8E);
    idt_set_gate(38, (uint32_t)irq_stub_6, 0x08, 0x8E);
    idt_set_gate(39, (uint32_t)irq_stub_7, 0x08, 0x8E);
    idt_set_gate(40, (uint32_t)irq_stub_8, 0x08, 0x8E);
    idt_set_gate(41, (uint32_t)irq_stub_9, 0x08, 0x8E);
    idt_set_gate(42, (uint32_t)irq_stub_10, 0x08, 0x8E);
    idt_set_gate(43, (uint32_t)irq_stub_11, 0x08, 0x8E);
    idt_set_gate(44, (uint32_t)irq_stub_12, 0x08, 0x8E);
    idt_set_gate(45, (uint32_t)irq_stub_13, 0x08, 0x8E);
    idt_set_gate(46, (uint32_t)irq_stub_14, 0x08, 0x8E);
    idt_set_gate(47, (uint32_t)irq_stub_15, 0x08, 0x8E);

    // Load IDT
    __asm__ volatile("lidt %0" : : "m"(idtp));

    kernel_print("  IDT initialized with 256 entries\n");
}

// Initialize PIC (8259A)
void pic_init(void) {
    // ICW1
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x11), "Nd"((uint16_t)0x20));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x11), "Nd"((uint16_t)0xA0));

    // ICW2 - Remap IRQs to 32-47
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"((uint16_t)0x21));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x28), "Nd"((uint16_t)0xA1));

    // ICW3
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x04), "Nd"((uint16_t)0x21));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x02), "Nd"((uint16_t)0xA1));

    // ICW4
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x01), "Nd"((uint16_t)0x21));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x01), "Nd"((uint16_t)0xA1));

    // Mask all interrupts for now
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0xFF), "Nd"((uint16_t)0x21));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0xFF), "Nd"((uint16_t)0xA1));

    kernel_print("  PIC initialized (IRQs remapped to 32-47)\n");
}

// Initialize interrupt system
void interrupt_init(void) {
    kernel_print("[INT] Initializing interrupt system...\n");

    idt_init();
    pic_init();

    // Enable keyboard interrupt only (unmask IRQ1)
    // PIC1 mask: 11111101 = 0xFD (only IRQ1 enabled)
    outb(0x21, 0xFD);

    // Enable interrupts
    __asm__ volatile("sti");
    kernel_print("  Interrupts enabled\n");
    kernel_print("  Keyboard interrupt registered (IRQ1)\n");
}