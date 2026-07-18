// TernaryBit OS - Main Kernel Entry Point
// Complete initialization and boot-to-shell
// 🕉️ Swamiye Saranam Aiyappa 🕉️

#include <stdint.h>
#include <stddef.h>
#include "tbos/config.h"
#include "tbos/blockdev.h"
#include "tbos/hal.h"
#include "tbos/vfs.h"
#include "tbos/fs_drivers.h"
#include "tbos/libc.h"
#include "tbos_boot_descriptor.h"

extern uint32_t g_tbds_pointer;
extern uint32_t g_tbds_length;
// VGA text mode
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

// External function declarations
#if CONFIG_INTERRUPTS
extern void interrupt_init(void);
#endif
#if CONFIG_KEYBOARD
extern void keyboard_init(void);
#endif
#if CONFIG_SHELL
extern void shell_init(void);
extern void shell_loop(void);
#endif
extern void libc_init(void);

// VGA buffer
volatile uint16_t* vga = (volatile uint16_t*)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;
static const hal_dispatch_table_t* volatile hal_dispatch = NULL;

// Clear screen
void clear_screen(void) {
    cursor_x = 0;
    cursor_y = 0;
    uint8_t attr = 0x0F;

    // Check hal_dispatch safely - avoid undefined behavior
    const hal_dispatch_table_t* hal_ptr = hal_dispatch;
    if (hal_ptr != NULL) {
        // Now we can safely access members
        if (hal_ptr->console.clear != NULL) {
            hal_ptr->console.clear(attr);
            return;
        }
    }

    // Fallback: direct VGA write
    uint16_t value = ((uint16_t)attr << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga[i] = value;
    }
}

// Print character at specific position
void vga_putchar(char c, uint8_t color, int x, int y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;

    // Check hal_dispatch safely
    const hal_dispatch_table_t* hal_ptr = hal_dispatch;
    if (hal_ptr != NULL) {
        if (hal_ptr->console.put_char_xy != NULL) {
            hal_ptr->console.put_char_xy((uint32_t)x, (uint32_t)y, c, color);
            return;
        }
    }

    // Fallback: direct VGA write
    vga[y * VGA_WIDTH + x] = ((uint16_t)color << 8) | (uint8_t)c;
}

// Print string at specific position
void vga_print_at(const char* str, uint8_t color, int x, int y) {
    for (int i = 0; str[i] != '\0'; i++) {
        int target_x = x + i;
        if (target_x >= VGA_WIDTH) {
            break;
        }
        vga_putchar(str[i], color, target_x, y);
    }
}

// Scroll screen up
static void scroll_up(void) {
    // Move all lines up
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga[y * VGA_WIDTH + x] = vga[(y + 1) * VGA_WIDTH + x];
        }
    }
    // Clear last line
    for (int x = 0; x < VGA_WIDTH; x++) {
        vga_putchar(' ', 0x0F, x, VGA_HEIGHT - 1);
    }
}

static inline void serial_outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t serial_inb(uint16_t port) {
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static void kernel_serial_init(void) {
    if (hal_dispatch && hal_dispatch->console.write_serial) {
        return;
    }
    serial_outb(0x3F9, 0x00); // Disable interrupts
    serial_outb(0x3FB, 0x80); // Enable DLAB
    serial_outb(0x3F8, 0x01); // Divisor low (DLAB)
    serial_outb(0x3F9, 0x00); // Divisor high (DLAB)
    serial_outb(0x3FB, 0x03); // 8 bits, no parity, one stop
    serial_outb(0x3FA, 0xC7); // Enable FIFO
    serial_outb(0x3FC, 0x0B); // IRQs off, RTS/DSR set
}

static void kernel_serial_write(char c) {
    if (hal_dispatch && hal_dispatch->console.write_serial) {
        hal_dispatch->console.write_serial(c);
        return;
    }
    while ((serial_inb(0x3FD) & 0x20) == 0) {
        /* wait for transmitter ready */
    }
    serial_outb(0x3F8, (uint8_t)c);
}

// Print character (with newline handling)
void kernel_putchar(char c) {
    /* Mirror to serial for headless (-nographic) runs */
    if (c == '\n') {
        kernel_serial_write('\r');
    }
    kernel_serial_write(c);

    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            vga_putchar(' ', 0x0F, cursor_x, cursor_y);
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;
    } else {
        vga_putchar(c, 0x0F, cursor_x, cursor_y);
        cursor_x++;
    }

    // Handle line wrap
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    // Handle screen scroll
    if (cursor_y >= VGA_HEIGHT) {
        scroll_up();
        cursor_y = VGA_HEIGHT - 1;
    }
}

// Print string
void kernel_print(const char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        kernel_putchar(str[i]);
    }
}

// Print hexadecimal number
void kernel_print_hex(uint32_t num) {
    char hex[] = "0123456789ABCDEF";
    char buffer[11] = "0x00000000";

    for (int i = 0; i < 8; i++) {
        buffer[9 - i] = hex[num & 0xF];
        num >>= 4;
    }

    kernel_print(buffer);
}

// Simple delay
static void delay(uint32_t count) {
    if (hal_dispatch && hal_dispatch->timer.sleep_ms) {
        hal_dispatch->timer.sleep_ms(count);
        return;
    }
    for (volatile uint32_t i = 0; i < count * 1000; i++);
}

#if CONFIG_FS
static void populate_root_fs(void) {
    vfs_mkdir("/etc");
    vfs_mkdir("/home");
    vfs_mkdir("/tmp");
    vfs_mkdir("/var");
    vfs_mkdir("/var/log");
    vfs_mkdir("/docs");

    const char* motd =
        "Welcome to TernaryBit OS Stage 1 Shell\n"
        "Type 'help' for built-in commands.\n";
    vfs_write_file("/etc/motd", motd, strlen(motd));

    const char* guide =
        "Stage 1 Shell Quickstart:\n"
        "  pwd            - print current directory\n"
        "  ls             - list files in current directory\n"
        "  cd /docs       - change directory\n"
        "  cat /etc/motd  - view welcome message\n"
        "  mkdir /tmp/logs, touch /tmp/logs/run.log\n";
    vfs_write_file("/docs/quickstart.txt", guide, strlen(guide));

    const char* log_hint =
        "[log] Ready to capture sacred system events.\n";
    vfs_write_file("/var/log/tbos.log", log_hint, strlen(log_hint));
}
#else
static void populate_root_fs(void) {
    (void)0;
}
#endif

static void kernel_process_tbds(hal_capabilities_t* caps) {
    if (g_tbds_pointer == 0) {
        kernel_print("[WARN] No TBDS pointer provided\n");
        return;
    }

    tbds_header_t* header = (tbds_header_t*)(uintptr_t)g_tbds_pointer;
    if (header->signature != TBDS_SIGNATURE) {
        kernel_print("[WARN] Invalid TBDS signature\n");
        return;
    }

    uint32_t advertised = header->total_length;
    uint32_t available = g_tbds_length ? g_tbds_length : advertised;
    if (available < sizeof(tbds_header_t)) {
        kernel_print("[WARN] TBDS truncated header\n");
        return;
    }

    if (advertised > available) {
        advertised = available;
    }

    uint8_t* cursor = (uint8_t*)(uintptr_t)(g_tbds_pointer + sizeof(tbds_header_t));
    uint32_t remaining = advertised - sizeof(tbds_header_t);
    uint16_t processed = 0;

    while (remaining >= sizeof(tbds_tlv_t) && processed < header->descriptor_count) {
        const tbds_tlv_t* tlv = (const tbds_tlv_t*)cursor;
        uint32_t record_size = sizeof(tbds_tlv_t) + tlv->length;
        if (record_size > remaining) {
            break;
        }

        const uint8_t* payload = cursor + sizeof(tbds_tlv_t);
        switch (tlv->type) {
            case TBDS_TYPE_BOOT_DEVICE:
                if (tlv->length >= 14) {
                    uint8_t drive = payload[0];
                    uint32_t kernel_lba = *(const uint32_t*)(payload + 8);
                    uint16_t kernel_sectors = *(const uint16_t*)(payload + 12);

                    kernel_print("[TBDS] Boot drive=0x");
                    kernel_print_hex(drive);
                    kernel_print(", kernel LBA=0x");
                    kernel_print_hex(kernel_lba);
                    kernel_print(", sectors=0x");
                    kernel_print_hex(kernel_sectors);
                    kernel_print("\n");
                }
                break;
            case TBDS_TYPE_CONSOLE_INFO:
                if (tlv->length >= 8) {
                    uint8_t text = payload[0];
                    uint8_t serial = payload[2];
                    uint16_t cols = *(const uint16_t*)(payload + 4);
                    uint16_t rows = *(const uint16_t*)(payload + 6);

                    kernel_print("[TBDS] Console text=");
                    kernel_print(text ? "yes" : "no");
                    kernel_print(", serial=");
                    kernel_print(serial ? "yes" : "no");
                    kernel_print(", mode=");
                    kernel_print_hex(rows);
                    kernel_print("x");
                    kernel_print_hex(cols);
                    kernel_print("\n");

                    if (caps) {
                        caps->has_console = text ? 1u : 0u;
                    }
                }
                break;
            default:
                break;
        }

        cursor += record_size;
        remaining -= record_size;
        processed++;
    }
}

// Main kernel entry point
void kernel_main(void) {
    // Initialize serial first for early debug output
    kernel_serial_init();
    kernel_serial_write('[');
    kernel_serial_write('K');
    kernel_serial_write('1');
    kernel_serial_write(']');
    kernel_serial_write('\n');

    kernel_serial_write('[');
    kernel_serial_write('K');
    kernel_serial_write('2');
    kernel_serial_write(']');
    kernel_serial_write('\n');

    kernel_serial_write('[');
    kernel_serial_write('K');
    kernel_serial_write('3');
    kernel_serial_write(']');
    kernel_serial_write('\n');

    // Initialize libc first (bump allocator - no-op but keeps compatibility)
    libc_init();

    // Don't use HAL - causes issues
    hal_dispatch = NULL;

    // Clear screen and show banner
    clear_screen();
    kernel_print("=====================================\n");
    kernel_print("  TernaryBit OS v4.0 - Bare Metal\n");
    kernel_print("  Swamiye Saranam Aiyappa\n");
    kernel_print("=====================================\n\n");

#if CONFIG_INTERRUPTS
    kernel_print("[INIT] Setting up interrupts...\n");
    interrupt_init();
    kernel_print("[INIT] Interrupts enabled\n");
#endif

#if CONFIG_KEYBOARD && CONFIG_INTERRUPTS
    kernel_print("[INIT] Initializing keyboard...\n");
    keyboard_init();
    kernel_print("[INIT] Keyboard ready\n");
#endif

    delay(50);

#if CONFIG_FS
    kernel_print("[VFS] Initializing filesystem...\n");
    vfs_init();
    populate_root_fs();
    kernel_print("[VFS] Filesystem ready\n");
#endif

    kernel_print("\n[INIT] System initialization complete!\n\n");

#if CONFIG_SHELL
    kernel_print("Starting shell...\n");
    delay(100);
    shell_init();
    shell_loop();  // This should never return
#else
    kernel_print("No shell configured. System halted.\n");
    while(1) {
        __asm__ volatile("cli; hlt");
    }
#endif
}
