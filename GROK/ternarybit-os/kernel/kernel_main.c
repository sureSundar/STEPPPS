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
#include "tbos/bcb.h"

extern uint32_t g_tbds_pointer;
extern uint32_t g_tbds_length;
extern uint32_t g_bcb_pointer;  // V4.0 BCB pointer (passed in ECX)
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

    // Fallback: direct VGA write - use direct pointer instead of global
    volatile uint16_t *vga_mem = (volatile uint16_t*)VGA_MEMORY;
    uint16_t value = ((uint16_t)attr << 8) | ' ';
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_mem[i] = value;
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

    // Fallback: direct VGA write - use direct pointer instead of global
    volatile uint16_t *vga_mem = (volatile uint16_t*)VGA_MEMORY;
    vga_mem[y * VGA_WIDTH + x] = ((uint16_t)color << 8) | (uint8_t)c;
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
    volatile uint16_t *vga_mem = (volatile uint16_t*)VGA_MEMORY;
    // Move all lines up
    for (int y = 0; y < VGA_HEIGHT - 1; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            vga_mem[y * VGA_WIDTH + x] = vga_mem[(y + 1) * VGA_WIDTH + x];
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

// Update hardware cursor position
static void update_cursor(void) {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;

    // Low byte
    serial_outb(0x3D4, 0x0F);
    serial_outb(0x3D5, (uint8_t)(pos & 0xFF));

    // High byte
    serial_outb(0x3D4, 0x0E);
    serial_outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
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

// Print character (with newline handling) - using direct VGA
void kernel_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            volatile uint16_t *vga_mem = (volatile uint16_t*)VGA_MEMORY;
            int pos = cursor_y * VGA_WIDTH + cursor_x;
            vga_mem[pos] = 0x0F20;  // Write space
        }
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~3;
    } else {
        // Write character directly to VGA - get fresh pointer each time
        volatile uint16_t *vga_mem = (volatile uint16_t*)VGA_MEMORY;
        int pos = cursor_y * VGA_WIDTH + cursor_x;
        vga_mem[pos] = 0x0F00 | (uint8_t)c;
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

    // Update hardware cursor position
    update_cursor();
}

// Print string - now with proper loop support!
void kernel_print(const char* str) {
    if (!str) return;

    // Use kernel_putchar which we know works
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

/**
 * @brief Process Boot Capability Block (BCB) - V4.0
 * Traceability: v4.0/ROADMAP.md V4-003
 */
static bool kernel_process_bcb(hal_capabilities_t* caps) {
    // Check if BCB pointer is valid
    if (g_bcb_pointer == 0) {
        return false;  // No BCB, fall back to TBDS
    }

    tbos_bcb_v1_t* bcb = (tbos_bcb_v1_t*)(uintptr_t)g_bcb_pointer;

    // Validate BCB
    if (!bcb_validate(bcb)) {
        kernel_print("[BCB] Invalid BCB structure\n");
        return false;
    }

    kernel_print("[BCB] Boot Capability Block v");
    kernel_print_hex(bcb->version);
    kernel_print(" detected\n");

    // Display boot environment
    kernel_print("[BCB] Boot stage: ");
    kernel_print_hex(bcb->boot_stage);
    kernel_print(", flags: ");
    kernel_print_hex(bcb->stage_flags);
    kernel_print("\n");

    // Display memory info
    kernel_print("[BCB] Memory: low=");
    kernel_print_hex((uint32_t)bcb->mem_low_bytes);
    kernel_print(", high=");
    kernel_print_hex((uint32_t)bcb->mem_high_bytes);
    kernel_print("\n");

    // Get HAL type from BCB
    bcb_hal_type_t hal_type = bcb_get_hal_type(bcb);
    kernel_print("[BCB] HAL type: ");
    kernel_print(bcb_hal_type_name(hal_type));
    kernel_print("\n");

    // Configure HAL based on BCB flags
    if (caps) {
        caps->has_console = BCB_HAS_FLAG(bcb, BCB_FLAG_VGA_CONSOLE) ? 1 : 0;
    }

    // Check for host API (hosted environments)
    if (BCB_IS_HOSTED(bcb) && bcb->host_api_ptr != 0) {
        kernel_print("[BCB] Host API available at ");
        kernel_print_hex((uint32_t)bcb->host_api_ptr);
        kernel_print("\n");
    }

    // If BCB includes TBDS pointer, set globals for backwards compatibility
    if (bcb->tbds_pointer != 0) {
        g_tbds_pointer = bcb->tbds_pointer;
        g_tbds_length = bcb->tbds_length;
    }

    return true;
}

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
    // Clear screen
    volatile uint16_t *vga = (volatile uint16_t*)0xB8000;
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = 0x0F20;  // Space character
    }

    // Initialize cursor position
    cursor_x = 0;
    cursor_y = 0;

    // Display banner
    kernel_print("TernaryBit OS - 64-bit Kernel\n");
    kernel_print("Initializing...\n\n");

    // Initialize libc
    libc_init();
    kernel_print("[OK] Memory allocator initialized\n");

    // Process boot information (BCB first, fall back to TBDS)
    // V4.0: Try BCB first for modern boot protocol
    hal_capabilities_t boot_caps = {0};
    if (kernel_process_bcb(&boot_caps)) {
        kernel_print("[OK] BCB processed successfully\n");
    } else {
        // Fall back to TBDS for legacy boot
        kernel_print("[INFO] No BCB, trying TBDS...\n");
        kernel_process_tbds(&boot_caps);
    }

    // Don't use HAL - causes issues
    hal_dispatch = NULL;

    kernel_print("[DEBUG] About to check CONFIG_FS\n");

#if CONFIG_FS
    kernel_print("[INFO] Initializing VFS...\n");
    vfs_init();

    kernel_print("[INFO] Mounting RAMFS at /...\n");
    extern const vfs_driver_t ramfs_driver;
    int mount_result = vfs_mount("/", &ramfs_driver);
    if (mount_result != 0) {
        kernel_print("[ERROR] Failed to mount RAMFS!\n");
    } else {
        kernel_print("[OK] RAMFS mounted at /\n");
    }

    kernel_print("[INFO] Populating root filesystem...\n");
    populate_root_fs();
    kernel_print("[OK] Filesystem initialized\n");

    // Mount UCFS overlay at /ucfs
    kernel_print("[INFO] Mounting UCFS overlay at /ucfs...\n");
    extern const vfs_driver_t ucfs_driver;
    extern int ucfs_set_backing_driver(void*, const vfs_driver_t*, void*, const char*);

    // Get ramfs context from root mount to use as backing store
    void* ucfs_ctx = ucfs_driver.init();
    if (ucfs_ctx) {
        // Configure UCFS to use ramfs as backing
        void* ramfs_ctx = ramfs_driver.init();
        if (ramfs_ctx && ucfs_set_backing_driver(ucfs_ctx, &ramfs_driver, ramfs_ctx, "/ucfs") == 0) {
            int ucfs_mount_result = vfs_mount_with_context("/ucfs", &ucfs_driver, ucfs_ctx);
            if (ucfs_mount_result == 0) {
                kernel_print("[OK] UCFS mounted at /ucfs\n");
            } else {
                kernel_print("[ERROR] Failed to mount UCFS!\n");
            }
        } else {
            kernel_print("[ERROR] Failed to configure UCFS backing!\n");
        }
    } else {
        kernel_print("[ERROR] Failed to initialize UCFS context!\n");
    }

    // Mount RF2S overlay at /rf2s
    kernel_print("[INFO] Mounting RF2S overlay at /rf2s...\n");
    extern const vfs_driver_t rf2s_driver;
    extern int rf2s_set_backing_driver(void*, const vfs_driver_t*, void*, const char*);

    void* rf2s_ctx = rf2s_driver.init();
    if (rf2s_ctx) {
        // Configure RF2S to use ramfs as backing
        void* rf2s_ramfs_ctx = ramfs_driver.init();
        if (rf2s_ramfs_ctx && rf2s_set_backing_driver(rf2s_ctx, &ramfs_driver, rf2s_ramfs_ctx, "/rf2s") == 0) {
            // Create the rf2s root directory in ramfs
            vfs_mkdir("/rf2s");
            int rf2s_mount_result = vfs_mount_with_context("/rf2s", &rf2s_driver, rf2s_ctx);
            if (rf2s_mount_result == 0) {
                kernel_print("[OK] RF2S mounted at /rf2s\n");
            } else {
                kernel_print("[ERROR] Failed to mount RF2S!\n");
            }
        } else {
            kernel_print("[ERROR] Failed to configure RF2S backing!\n");
        }
    } else {
        kernel_print("[ERROR] Failed to initialize RF2S context!\n");
    }
#else
    kernel_print("[DEBUG] CONFIG_FS is disabled!\n");
#endif

    kernel_print("[DEBUG] After CONFIG_FS block\n");

    // Note: Interrupts and keyboard disabled for now
    kernel_print("[SKIP] Interrupts disabled\n");
    kernel_print("[SKIP] Keyboard disabled\n");

#if CONFIG_SHELL
    kernel_print("\nStarting shell...\n\n");
    shell_init();
    shell_loop();  // This should never return
#else
    kernel_print("\nNo shell available. System halted.\n");
    while(1) {
        __asm__ volatile("cli; hlt");
    }
#endif
}
