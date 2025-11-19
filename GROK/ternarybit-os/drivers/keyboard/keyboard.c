// TBOS Keyboard Driver
// Real keyboard input for interactive shell

#include <stdint.h>
#include <stddef.h>

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

// Keyboard scan codes
// Scancode to ASCII - normal (no shift)
static const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

// Scancode to ASCII - with shift
static const char scancode_to_ascii_shift[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

// Shift key scancodes
#define SCANCODE_LEFT_SHIFT  0x2A
#define SCANCODE_RIGHT_SHIFT 0x36

static uint8_t keyboard_buffer[256];
static uint8_t buffer_head = 0;
static uint8_t buffer_tail = 0;

// Read from I/O port
static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Write to I/O port
static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Check if keyboard has data
static int keyboard_has_data(void) {
    return inb(KEYBOARD_STATUS_PORT) & 0x01;
}

// Keyboard interrupt handler
void keyboard_interrupt_handler(void) {
    if (!keyboard_has_data()) return;

    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // Convert scan code to ASCII
    if (scancode < sizeof(scancode_to_ascii)) {
        char ch = scancode_to_ascii[scancode];
        if (ch != 0) {
            // Add to buffer
            uint8_t next_tail = (buffer_tail + 1) % 256;
            if (next_tail != buffer_head) {
                keyboard_buffer[buffer_tail] = ch;
                buffer_tail = next_tail;
            }
        }
    }
}

// Read character from keyboard (non-blocking)
uint8_t keyboard_read_char(void) {
    if (buffer_head == buffer_tail) {
        return 0;  // No data
    }

    uint8_t ch = keyboard_buffer[buffer_head];
    buffer_head = (buffer_head + 1) % 256;
    return ch;
}

// Read character from keyboard with polling (no interrupts needed)
uint8_t keyboard_read_char_poll(void) {
    static uint8_t last_scancode = 0;
    static uint32_t debounce_counter = 0;
    static uint8_t shift_pressed = 0;  // Track shift state

    // Simple debouncing - skip if we read too recently
    if (debounce_counter > 0) {
        debounce_counter--;
        return 0;
    }

    // Check if keyboard has data available
    if (!keyboard_has_data()) {
        return 0;  // No data available
    }

    // Read scan code from keyboard
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    // Handle key release events (scan codes >= 0x80)
    if (scancode >= 0x80) {
        uint8_t released_key = scancode & 0x7F;

        // Check if shift was released
        if (released_key == SCANCODE_LEFT_SHIFT || released_key == SCANCODE_RIGHT_SHIFT) {
            shift_pressed = 0;
        }

        // Reset last scancode if it was released
        if (released_key == last_scancode) {
            last_scancode = 0;
            debounce_counter = 0;  // Allow immediate next key
        }
        return 0;  // Key release event, ignore
    }

    // Check if shift was pressed
    if (scancode == SCANCODE_LEFT_SHIFT || scancode == SCANCODE_RIGHT_SHIFT) {
        shift_pressed = 1;
        return 0;  // Don't return a character for shift itself
    }

    // Ignore if same as last scancode (key still held)
    if (scancode == last_scancode) {
        return 0;
    }

    last_scancode = scancode;
    debounce_counter = 10000;  // Longer debounce delay

    // Convert scan code to ASCII using appropriate table
    if (scancode < sizeof(scancode_to_ascii)) {
        if (shift_pressed) {
            return scancode_to_ascii_shift[scancode];
        } else {
            return scancode_to_ascii[scancode];
        }
    }

    return 0;  // Unknown scan code
}

// Initialize keyboard driver
void keyboard_init(void) {
    // Clear buffer
    buffer_head = 0;
    buffer_tail = 0;

    // Enable keyboard
    outb(KEYBOARD_STATUS_PORT, 0xAE);  // Enable keyboard interface

    // Set scan code set 1
    outb(KEYBOARD_DATA_PORT, 0xF0);
    outb(KEYBOARD_DATA_PORT, 0x01);

    // Enable keyboard interrupts (IRQ1)
    // This would be done in interrupt_init()
}