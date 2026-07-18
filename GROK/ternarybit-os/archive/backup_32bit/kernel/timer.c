// TBOS Timer (PIT) - Sprint 10
// 🕉️ Swamiye Saranam 🕉️
// Programmable Interval Timer configuration

#include <stdint.h>

// PIT constants
#define PIT_FREQ 1193182
#define PIT_CMD  0x43
#define PIT_CH0  0x40

// Timer state
static uint32_t ticks = 0;
static uint32_t frequency = 100; // 100Hz = 10ms per tick

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);
extern void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

// Timer interrupt handler
void timer_handler(void) {
    ticks++;

    // Every second, print tick count
    if (ticks % frequency == 0) {
        kernel_print("[TIMER] Tick: ");
        kernel_print_hex(ticks / frequency);
        kernel_print(" seconds\n");
    }

    // Send EOI to PIC
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x20), "Nd"((uint16_t)0x20));
}

// Timer ISR wrapper (will be called from assembly)
__attribute__((naked)) void timer_isr(void) {
    __asm__ volatile(
        "pusha\n"
        "call timer_handler\n"
        "popa\n"
        "iret\n"
    );
}

// Initialize PIT
void timer_init(uint32_t freq) {
    frequency = freq;

    // Calculate divisor
    uint32_t divisor = PIT_FREQ / freq;

    // Send command byte
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)0x36), "Nd"((uint16_t)PIT_CMD));

    // Send divisor
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)(divisor & 0xFF)), "Nd"((uint16_t)PIT_CH0));
    __asm__ volatile("outb %0, %1" : : "a"((uint8_t)((divisor >> 8) & 0xFF)), "Nd"((uint16_t)PIT_CH0));

    // Register timer interrupt handler (IRQ0 = INT 32)
    idt_set_gate(32, (uint32_t)timer_isr, 0x08, 0x8E);

    // Enable timer interrupt (unmask IRQ0)
    uint8_t mask;
    __asm__ volatile("inb %1, %0" : "=a"(mask) : "Nd"((uint16_t)0x21));
    mask &= ~0x01; // Clear bit 0 (IRQ0)
    __asm__ volatile("outb %0, %1" : : "a"(mask), "Nd"((uint16_t)0x21));

    kernel_print("  Timer initialized at ");
    kernel_print_hex(freq);
    kernel_print(" Hz\n");
}

// Get system ticks
uint32_t timer_get_ticks(void) {
    return ticks;
}

// Sleep for milliseconds
void timer_sleep(uint32_t ms) {
    uint32_t target = ticks + (ms * frequency / 1000);
    while (ticks < target) {
        __asm__ volatile("hlt");
    }
}