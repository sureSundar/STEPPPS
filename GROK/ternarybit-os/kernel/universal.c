// Minimal bare-metal kernel with IRQ-driven timer and keyboard shell
// Boots in 32-bit protected mode, writes to VGA text buffer, uses PIC + PIT + PS/2 IRQs.

#include <stdint.h>
#include <stddef.h>

extern uint8_t _sbss[];
extern uint8_t _ebss[];

#define VGA_MEM ((uint16_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define PS2_DATA 0x60
#define PS2_STATUS 0x64

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

#define PIT_CH0 0x40
#define PIT_CMD 0x43
#define PIT_BASE_HZ 1193182

#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_INT32 0x0E
#define IDT_FLAG_RING0 0x00

#define KEYBUF_SIZE 128

// Forward declarations for ISR stubs (assembly)
extern void irq0_stub(void);
extern void irq1_stub(void);

static uint8_t cursor_x;
static uint8_t cursor_y;
static uint8_t vga_color = 0x07; // light gray on black
static volatile uint32_t tick_count;
static volatile uint8_t key_head;
static volatile uint8_t key_tail;
static volatile char key_buf[KEYBUF_SIZE];
static volatile uint8_t key_overflow;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void memset(void* dst, int value, size_t count) {
    uint8_t* p = (uint8_t*)dst;
    for (size_t i = 0; i < count; i++) {
        p[i] = (uint8_t)value;
    }
}

static size_t strlen(const char* s) {
    size_t n = 0;
    while (s && s[n]) n++;
    return n;
}

static int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return (uint8_t)*a - (uint8_t)*b;
}

static void strcpy(char* dst, const char* src) {
    while ((*dst++ = *src++)) {}
}

/* =========================================================================
 * IDT / PIC / PIT setup
 * ========================================================================= */

struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_high;
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

static struct idt_entry idt[256];

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].offset_low = (uint16_t)(base & 0xFFFF);
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].type_attr = flags;
    idt[num].offset_high = (uint16_t)((base >> 16) & 0xFFFF);
}

static void idt_load(void) {
    struct idt_ptr ptr;
    ptr.limit = sizeof(idt) - 1;
    ptr.base = (uint32_t)idt;
    __asm__ volatile("lidt %0" : : "m"(ptr));
}

static void pic_remap(void) {
    // ICW1
    outb(PIC1_CMD, 0x11);
    outb(PIC2_CMD, 0x11);
    // ICW2: vector offsets
    outb(PIC1_DATA, 0x20); // IRQ0-7 -> 0x20-0x27
    outb(PIC2_DATA, 0x28); // IRQ8-15 -> 0x28-0x2F
    // ICW3: wiring
    outb(PIC1_DATA, 0x04); // slave on IRQ2
    outb(PIC2_DATA, 0x02);
    // ICW4
    outb(PIC1_DATA, 0x01);
    outb(PIC2_DATA, 0x01);
    // Mask: enable only IRQ0/IRQ1 on master, mask all slave
    outb(PIC1_DATA, 0b11111100);
    outb(PIC2_DATA, 0xFF);
}

static void pit_init(uint32_t hz) {
    if (hz == 0) return;
    uint32_t divisor = PIT_BASE_HZ / hz;
    outb(PIT_CMD, 0x36); // channel 0, lo/hi, rate generator
    outb(PIT_CH0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH0, (uint8_t)((divisor >> 8) & 0xFF));
}

/* =========================================================================
 * VGA text output
 * ========================================================================= */

static void vga_clear(void) {
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        VGA_MEM[i] = (uint16_t)vga_color << 8 | ' ';
    }
    cursor_x = 0;
    cursor_y = 0;
}

static void vga_newline(void) {
    cursor_x = 0;
    if (cursor_y + 1 >= VGA_HEIGHT) {
        // scroll up one line
        for (size_t i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
            VGA_MEM[i] = VGA_MEM[i + VGA_WIDTH];
        }
        for (size_t i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
            VGA_MEM[i] = (uint16_t)vga_color << 8 | ' ';
        }
    } else {
        cursor_y++;
    }
}

static void vga_putc(char c) {
    if (c == '\n') {
        vga_newline();
        return;
    }
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            VGA_MEM[cursor_y * VGA_WIDTH + cursor_x] = (uint16_t)vga_color << 8 | ' ';
        }
        return;
    }
    VGA_MEM[cursor_y * VGA_WIDTH + cursor_x] = (uint16_t)vga_color << 8 | (uint8_t)c;
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        vga_newline();
    }
}

static void vga_write(const char* s) {
    while (*s) vga_putc(*s++);
}

static void vga_write_line(const char* s) {
    vga_write(s);
    vga_putc('\n');
}

static void print_hex(uint32_t value) {
    char digits[] = "0123456789ABCDEF";
    char out[11] = "0x00000000";
    for (int i = 9; i >= 2; i--) {
        out[i] = digits[value & 0xF];
        value >>= 4;
    }
    vga_write(out);
}

static void print_dec(uint32_t value) {
    char buf[12];
    int idx = 0;
    if (value == 0) {
        vga_putc('0');
        return;
    }
    while (value && idx < (int)sizeof(buf) - 1) {
        buf[idx++] = '0' + (value % 10);
        value /= 10;
    }
    while (idx--) {
        vga_putc(buf[idx]);
    }
}

/* =========================================================================
 * Keyboard handling
 * ========================================================================= */

// Scancode → ASCII (set 1), returns 0 for unsupported keys.
static char scancode_to_ascii(uint8_t sc) {
    switch (sc) {
        case 0x02 ... 0x0A: return '1' + (sc - 0x02);
        case 0x0B: return '0';
        case 0x10: return 'q';
        case 0x11: return 'w';
        case 0x12: return 'e';
        case 0x13: return 'r';
        case 0x14: return 't';
        case 0x15: return 'y';
        case 0x16: return 'u';
        case 0x17: return 'i';
        case 0x18: return 'o';
        case 0x19: return 'p';
        case 0x1E: return 'a';
        case 0x1F: return 's';
        case 0x20: return 'd';
        case 0x21: return 'f';
        case 0x22: return 'g';
        case 0x23: return 'h';
        case 0x24: return 'j';
        case 0x25: return 'k';
        case 0x26: return 'l';
        case 0x2C: return 'z';
        case 0x2D: return 'x';
        case 0x2E: return 'c';
        case 0x2F: return 'v';
        case 0x30: return 'b';
        case 0x31: return 'n';
        case 0x32: return 'm';
        case 0x39: return ' ';
        case 0x1C: return '\n';
        case 0x0E: return '\b';
        case 0x0C: return '-';
        case 0x0D: return '=';
        case 0x27: return ';';
        case 0x28: return '\'';
        case 0x33: return ',';
        case 0x34: return '.';
        case 0x35: return '/';
        default: return 0;
    }
}

/* =========================================================================
 * Interrupt handlers (C side, invoked from assembly stubs)
 * ========================================================================= */

void irq0_handler_c(void) {
    tick_count++;
}

void irq1_handler_c(void) {
    uint8_t sc = inb(PS2_DATA);
    if (sc & 0x80) {
        return; // key release
    }
    char c = scancode_to_ascii(sc);
    if (!c) return;
    uint8_t next = (uint8_t)((key_head + 1) % KEYBUF_SIZE);
    if (next != key_tail) {
        key_buf[key_head] = c;
        key_head = next;
    } else {
        key_overflow = 1;
    }
}

/* =========================================================================
 * Shell helpers
 * ========================================================================= */

static void shell_prompt(void) {
    vga_write("tbos> ");
}

static void shell_readline(char* buf, size_t max_len) {
    size_t len = 0;
    while (len + 1 < max_len) {
        while (key_head == key_tail) {
            __asm__ volatile("hlt");
        }
        char c = key_buf[key_tail];
        key_tail = (uint8_t)((key_tail + 1) % KEYBUF_SIZE);
        if (c == '\n') {
            vga_putc('\n');
            break;
        }
        if (c == '\b') {
            if (len > 0) {
                len--;
                vga_putc('\b');
            }
            continue;
        }
        vga_putc(c);
        buf[len++] = c;
    }
    buf[len] = '\0';
}

static void cmd_help(void) {
    vga_write_line("Commands: help, about, cls, echo <txt>, mem, ticks, reboot, halt");
}

static void cmd_about(void) {
    vga_write_line("TernaryBit OS - universal bare-metal shell");
    vga_write_line("Stage2 + 32-bit kernel with IRQ keyboard + VGA text");
}

static void cmd_echo(const char* args) {
    if (args && *args) vga_write_line(args);
    else vga_write_line("");
}

static void cmd_mem(void) {
    vga_write("BSS: ");
    print_hex((uint32_t)(uintptr_t)_sbss);
    vga_write(" - ");
    print_hex((uint32_t)(uintptr_t)_ebss);
    vga_write_line("");
    vga_write("Stack top: ");
    print_hex(0x00090000);
    vga_write_line("");
}

static void cmd_ticks(void) {
    vga_write("Ticks: ");
    print_dec(tick_count);
    vga_write_line("");
}

static void cmd_reboot(void) {
    // wait until controller ready
    while (inb(PS2_STATUS) & 0x02) { }
    outb(0x64, 0xFE);
    for (;;) { __asm__ volatile("hlt"); }
}

static void cmd_halt(void) {
    __asm__ volatile("cli; hlt");
}

static void shell_execute(char* line) {
    // trim leading spaces
    while (*line == ' ') line++;
    if (*line == '\0') return;

    // find first space to split command/args
    char* args = line;
    while (*args && *args != ' ') args++;
    if (*args) {
        *args++ = '\0';
        while (*args == ' ') args++;
    }

    if (strcmp(line, "help") == 0) {
        cmd_help();
    } else if (strcmp(line, "about") == 0) {
        cmd_about();
    } else if (strcmp(line, "cls") == 0) {
        vga_clear();
    } else if (strcmp(line, "echo") == 0) {
        cmd_echo(args);
    } else if (strcmp(line, "mem") == 0) {
        cmd_mem();
    } else if (strcmp(line, "ticks") == 0) {
        cmd_ticks();
    } else if (strcmp(line, "reboot") == 0) {
        cmd_reboot();
    } else if (strcmp(line, "halt") == 0 || strcmp(line, "quit") == 0) {
        cmd_halt();
    } else {
        vga_write("unknown command: ");
        vga_write_line(line);
    }
}

static void zero_bss(void) {
    memset(_sbss, 0, (size_t)(_ebss - _sbss));
}

void kernel_entry(void) {
    zero_bss();
    vga_clear();
    vga_write_line("=====================================");
    vga_write_line(" TernaryBit OS - Universal Bare Metal");
    vga_write_line(" Stage2 -> Protected Mode -> IRQ Shell");
    vga_write_line("=====================================");
    vga_write_line("");
    cmd_help();
    vga_write_line("");

    // IDT + PIC + PIT
    idt_set_gate(0x20, (uint32_t)irq0_stub, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_set_gate(0x21, (uint32_t)irq1_stub, 0x08, IDT_FLAG_PRESENT | IDT_FLAG_RING0 | IDT_FLAG_INT32);
    idt_load();
    pic_remap();
    pit_init(100); // 100 Hz
    __asm__ volatile("sti");

    char line[128];
    for (;;) {
        shell_prompt();
        shell_readline(line, sizeof(line));
        shell_execute(line);
    }
}
