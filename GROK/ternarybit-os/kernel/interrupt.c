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

// External ISR handlers
extern void isr0(void);
extern void isr1(void);
extern void keyboard_interrupt_handler(void);

// IRQ handlers
void irq1_handler(void);

// Default interrupt handler
void default_handler(uint32_t int_no, uint32_t err_code) {
    kernel_print("[INT] Interrupt: ");
    kernel_print_hex(int_no);
    kernel_print(" Error: ");
    kernel_print_hex(err_code);
    kernel_print("\n");
}

// IRQ1 - Keyboard handler
void irq1_handler(void) {
    keyboard_interrupt_handler();
    // Send EOI to PIC
    outb(0x20, 0x20);
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
    // For now, point to default handler placeholder
    for (int i = 0; i < 32; i++) {
        idt_set_gate(i, (uint32_t)default_handler, 0x08, 0x8E);
    }

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
    idt_init();
    pic_init();

    // Set up keyboard interrupt (IRQ1 = interrupt 33)
    idt_set_gate(33, (uint32_t)irq1_handler, 0x08, 0x8E);

    // Enable keyboard interrupt only (unmask IRQ1)
    outb(0x21, 0xFD);  // 11111101 - enable IRQ1

    // Enable interrupts
    __asm__ volatile("sti");
    kernel_print("  Interrupts enabled\n");
    kernel_print("  Keyboard interrupt registered (IRQ1)\n");
}