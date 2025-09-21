// WINDSURF: Minimal PS/2 keyboard stub
#include "ps2.h"

// Simple ring buffer for scancodes (stubbed, no IRQ wiring yet)
#define BUF_SZ 32
static volatile uint8_t buf[BUF_SZ];
static volatile uint8_t head = 0, tail = 0;

static bool buf_empty(void){ return head == tail; }
static bool buf_full(void){ return (uint8_t)(head + 1) % BUF_SZ == tail; }

void ps2_init(void) {
    head = tail = 0;
}

bool ps2_has_key(void) {
    return !buf_empty();
}

uint8_t ps2_read_scancode(void) {
    if (buf_empty()) return 0;
    uint8_t v = buf[tail];
    tail = (uint8_t)((tail + 1) % BUF_SZ);
    return v;
}

char ps2_translate(uint8_t sc) {
    // Very minimal US layout subset mapping
    static const char map[128] = {
        ['\x1E'] = 'a', ['\x30'] = 'b', ['\x2E'] = 'c', ['\x20'] = 'd',
        ['\x12'] = 'e', ['\x21'] = 'f', ['\x22'] = 'g', ['\x23'] = 'h',
        ['\x17'] = 'i', ['\x24'] = 'j', ['\x25'] = 'k', ['\x26'] = 'l',
        ['\x32'] = 'm', ['\x31'] = 'n', ['\x18'] = 'o', ['\x19'] = 'p',
        ['\x10'] = 'q', ['\x13'] = 'r', ['\x1F'] = 's', ['\x14'] = 't',
        ['\x16'] = 'u', ['\x2F'] = 'v', ['\x11'] = 'w', ['\x2D'] = 'x',
        ['\x15'] = 'y', ['\x2C'] = 'z',
        ['\x02'] = '1', ['\x03'] = '2', ['\x04'] = '3', ['\x05'] = '4',
        ['\x06'] = '5', ['\x07'] = '6', ['\x08'] = '7', ['\x09'] = '8',
        ['\x0A'] = '9', ['\x0B'] = '0', ['\x1C'] = '\n', ['\x39'] = ' ',
        ['\x0E'] = '\b'
    };
    if (sc < 128 && map[sc]) return map[sc];
    return 0;
}
