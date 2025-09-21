// TBOS GUI Framework - Hour 9 Sprints
// 🕉️ Swamiye Saranam Aiyappa 🕉️
// Sacred Visual Interface System

#include <stdint.h>
#include <stddef.h>

// External functions
extern void kernel_print(const char* str);
extern void kernel_print_hex(uint32_t value);

// VGA and VESA definitions
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_BUFFER 0xB8000

// VESA graphics modes
#define VESA_640x480x8    0x101
#define VESA_800x600x8    0x103
#define VESA_1024x768x8   0x105
#define VESA_640x480x16   0x111
#define VESA_800x600x16   0x114
#define VESA_1024x768x16  0x117

// GUI color definitions (16-bit RGB565)
#define GUI_BLACK       0x0000
#define GUI_WHITE       0xFFFF
#define GUI_RED         0xF800
#define GUI_GREEN       0x07E0
#define GUI_BLUE        0x001F
#define GUI_YELLOW      0xFFE0
#define GUI_MAGENTA     0xF81F
#define GUI_CYAN        0x07FF
#define GUI_ORANGE      0xFC00
#define GUI_PURPLE      0x8010
#define GUI_SACRED_OM   0xFD20  // Sacred saffron color

// GUI structures
typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t bpp;
    uint8_t* framebuffer;
    uint32_t pitch;
    uint8_t mode;
} display_mode_t;

typedef struct {
    int16_t x;
    int16_t y;
    uint16_t width;
    uint16_t height;
} rect_t;

typedef struct {
    int16_t x;
    int16_t y;
} point_t;

typedef struct {
    rect_t bounds;
    uint16_t color;
    uint16_t border_color;
    uint8_t border_width;
    uint8_t visible;
    char title[32];
} window_t;

// GUI system state
static display_mode_t current_mode = {0};
static window_t windows[16];
static uint8_t window_count = 0;
static uint8_t active_window = 0;

//============================================
// LOW-LEVEL GRAPHICS FUNCTIONS
//============================================

// Set VESA graphics mode
int set_vesa_mode(uint16_t mode) {
    // In real implementation, this would use VESA BIOS calls
    // For demo, simulate mode setting

    kernel_print("[GUI] Setting VESA mode 0x");
    kernel_print_hex(mode);
    kernel_print("\n");

    switch (mode) {
        case VESA_640x480x16:
            current_mode.width = 640;
            current_mode.height = 480;
            current_mode.bpp = 16;
            current_mode.framebuffer = (uint8_t*)0xA0000; // VGA framebuffer
            current_mode.pitch = 640 * 2;
            current_mode.mode = mode;
            break;

        case VESA_800x600x16:
            current_mode.width = 800;
            current_mode.height = 600;
            current_mode.bpp = 16;
            current_mode.framebuffer = (uint8_t*)0xA0000;
            current_mode.pitch = 800 * 2;
            current_mode.mode = mode;
            break;

        default:
            kernel_print("[GUI] Unsupported mode, defaulting to 640x480x16\n");
            current_mode.width = 640;
            current_mode.height = 480;
            current_mode.bpp = 16;
            current_mode.framebuffer = (uint8_t*)0xA0000;
            current_mode.pitch = 640 * 2;
            current_mode.mode = VESA_640x480x16;
            break;
    }

    kernel_print("[GUI] Resolution: ");
    kernel_print_hex(current_mode.width);
    kernel_print("x");
    kernel_print_hex(current_mode.height);
    kernel_print("x");
    kernel_print_hex(current_mode.bpp);
    kernel_print("\n");

    return 0;
}

// Put pixel at coordinates
void put_pixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= current_mode.width || y < 0 || y >= current_mode.height) {
        return; // Bounds check
    }

    if (current_mode.bpp == 16) {
        uint16_t* framebuffer = (uint16_t*)current_mode.framebuffer;
        framebuffer[y * (current_mode.pitch / 2) + x] = color;
    }
}

// Get pixel color at coordinates
uint16_t get_pixel(int16_t x, int16_t y) {
    if (x < 0 || x >= current_mode.width || y < 0 || y >= current_mode.height) {
        return 0;
    }

    if (current_mode.bpp == 16) {
        uint16_t* framebuffer = (uint16_t*)current_mode.framebuffer;
        return framebuffer[y * (current_mode.pitch / 2) + x];
    }

    return 0;
}

//============================================
// BASIC DRAWING FUNCTIONS
//============================================

// Clear screen with color
void clear_screen(uint16_t color) {
    for (int y = 0; y < current_mode.height; y++) {
        for (int x = 0; x < current_mode.width; x++) {
            put_pixel(x, y, color);
        }
    }
}

// Draw horizontal line
void draw_hline(int16_t x1, int16_t x2, int16_t y, uint16_t color) {
    if (x1 > x2) {
        int16_t temp = x1;
        x1 = x2;
        x2 = temp;
    }

    for (int16_t x = x1; x <= x2; x++) {
        put_pixel(x, y, color);
    }
}

// Draw vertical line
void draw_vline(int16_t x, int16_t y1, int16_t y2, uint16_t color) {
    if (y1 > y2) {
        int16_t temp = y1;
        y1 = y2;
        y2 = temp;
    }

    for (int16_t y = y1; y <= y2; y++) {
        put_pixel(x, y, color);
    }
}

// Draw rectangle outline
void draw_rect(rect_t rect, uint16_t color) {
    // Top and bottom
    draw_hline(rect.x, rect.x + rect.width - 1, rect.y, color);
    draw_hline(rect.x, rect.x + rect.width - 1, rect.y + rect.height - 1, color);

    // Left and right
    draw_vline(rect.x, rect.y, rect.y + rect.height - 1, color);
    draw_vline(rect.x + rect.width - 1, rect.y, rect.y + rect.height - 1, color);
}

// Fill rectangle
void fill_rect(rect_t rect, uint16_t color) {
    for (int16_t y = rect.y; y < rect.y + rect.height; y++) {
        for (int16_t x = rect.x; x < rect.x + rect.width; x++) {
            put_pixel(x, y, color);
        }
    }
}

// Draw circle (Bresenham's algorithm)
void draw_circle(point_t center, uint16_t radius, uint16_t color) {
    int16_t x = 0;
    int16_t y = radius;
    int16_t d = 3 - 2 * radius;

    while (x <= y) {
        // Draw 8 octants
        put_pixel(center.x + x, center.y + y, color);
        put_pixel(center.x - x, center.y + y, color);
        put_pixel(center.x + x, center.y - y, color);
        put_pixel(center.x - x, center.y - y, color);
        put_pixel(center.x + y, center.y + x, color);
        put_pixel(center.x - y, center.y + x, color);
        put_pixel(center.x + y, center.y - x, color);
        put_pixel(center.x - y, center.y - x, color);

        if (d <= 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

//============================================
// SACRED OM SYMBOL DRAWING
//============================================

// Draw the sacred OM symbol
void draw_om_symbol(point_t center, uint16_t size, uint16_t color) {
    kernel_print("[GUI] Drawing sacred OM symbol\n");

    // OM symbol is complex, simplified representation
    uint16_t radius = size / 2;

    // Main curve of OM
    draw_circle(center, radius, color);

    // Upper curve
    point_t upper = {center.x - radius/2, center.y - radius/2};
    draw_circle(upper, radius/3, color);

    // Lower curve
    point_t lower = {center.x + radius/3, center.y + radius/3};
    draw_circle(lower, radius/4, color);

    // Central line
    draw_hline(center.x - radius/2, center.x + radius/2, center.y, color);

    // Dot above (bindu)
    point_t bindu = {center.x, center.y - radius - 10};
    fill_rect((rect_t){bindu.x - 3, bindu.y - 3, 6, 6}, color);

    kernel_print("[GUI] Sacred OM symbol rendered\n");
}

//============================================
// WINDOW MANAGEMENT
//============================================

// Create a new window
int create_window(const char* title, rect_t bounds, uint16_t color) {
    if (window_count >= 16) {
        kernel_print("[GUI] Maximum windows reached\n");
        return -1;
    }

    window_t* win = &windows[window_count];
    win->bounds = bounds;
    win->color = color;
    win->border_color = GUI_WHITE;
    win->border_width = 2;
    win->visible = 1;

    // Copy title
    int i;
    for (i = 0; i < 31 && title[i]; i++) {
        win->title[i] = title[i];
    }
    win->title[i] = '\0';

    int window_id = window_count;
    window_count++;

    kernel_print("[GUI] Created window: ");
    kernel_print(title);
    kernel_print(" (ID ");
    kernel_print_hex(window_id);
    kernel_print(")\n");

    return window_id;
}

// Draw a window
void draw_window(int window_id) {
    if (window_id >= window_count) return;

    window_t* win = &windows[window_id];
    if (!win->visible) return;

    // Fill window background
    fill_rect(win->bounds, win->color);

    // Draw border
    for (int i = 0; i < win->border_width; i++) {
        rect_t border = {
            win->bounds.x - i,
            win->bounds.y - i,
            win->bounds.width + 2*i,
            win->bounds.height + 2*i
        };
        draw_rect(border, win->border_color);
    }

    // Draw title bar
    rect_t title_bar = {
        win->bounds.x,
        win->bounds.y - 20,
        win->bounds.width,
        20
    };
    fill_rect(title_bar, GUI_SACRED_OM);
    draw_rect(title_bar, win->border_color);
}

// Redraw all visible windows
void redraw_windows(void) {
    for (int i = 0; i < window_count; i++) {
        draw_window(i);
    }
}

//============================================
// GUI DESKTOP ENVIRONMENT
//============================================

// Draw desktop background
void draw_desktop(void) {
    kernel_print("[GUI] Drawing sacred desktop\n");

    // Gradient background (simplified)
    for (int y = 0; y < current_mode.height; y++) {
        uint16_t color = GUI_BLUE + (y * 32 / current_mode.height);
        draw_hline(0, current_mode.width - 1, y, color);
    }

    // Draw central OM symbol
    point_t center = {current_mode.width / 2, current_mode.height / 2};
    draw_om_symbol(center, 100, GUI_SACRED_OM);

    // Draw corner decorations
    draw_circle((point_t){50, 50}, 30, GUI_YELLOW);
    draw_circle((point_t){current_mode.width - 50, 50}, 30, GUI_YELLOW);
    draw_circle((point_t){50, current_mode.height - 50}, 30, GUI_YELLOW);
    draw_circle((point_t){current_mode.width - 50, current_mode.height - 50}, 30, GUI_YELLOW);

    kernel_print("[GUI] Sacred desktop rendered\n");
}

// Create default windows
void create_default_windows(void) {
    kernel_print("[GUI] Creating default sacred windows\n");

    // TBOS Info window
    rect_t info_rect = {50, 50, 300, 200};
    create_window("TBOS Info", info_rect, GUI_WHITE);

    // STEPPPS Status window
    rect_t steppps_rect = {400, 100, 200, 150};
    create_window("STEPPPS", steppps_rect, GUI_CYAN);

    // Sacred Console window
    rect_t console_rect = {100, 300, 400, 120};
    create_window("Sacred Console", console_rect, GUI_BLACK);

    // Divine Mantras window
    rect_t mantra_rect = {200, 150, 250, 180};
    create_window("Divine Mantras", mantra_rect, GUI_SACRED_OM);
}

//============================================
// GUI SYSTEM INITIALIZATION
//============================================

// Initialize GUI system
void gui_init(void) {
    kernel_print("[GUI] Initializing Sacred Visual Interface...\n");

    // Initialize window system
    for (int i = 0; i < 16; i++) {
        windows[i].visible = 0;
    }
    window_count = 0;
    active_window = 0;

    // Set graphics mode
    set_vesa_mode(VESA_640x480x16);

    // Clear screen
    clear_screen(GUI_BLACK);

    kernel_print("[GUI] Sacred GUI framework initialized\n");
}

// Full GUI system demo
void gui_demo(void) {
    kernel_print("\n=== GUI FRAMEWORK DEMONSTRATION ===\n");

    // Draw desktop
    draw_desktop();

    // Create windows
    create_default_windows();

    // Draw all windows
    redraw_windows();

    // Draw some graphics demonstrations
    kernel_print("[GUI] Demonstrating graphics primitives\n");

    // Draw test shapes in corners
    fill_rect((rect_t){10, 10, 30, 30}, GUI_RED);
    draw_circle((point_t){current_mode.width - 30, 30}, 15, GUI_GREEN);

    kernel_print("[GUI] Sacred desktop environment active!\n");
}

// GUI system information
void gui_info(void) {
    kernel_print("\n=== GUI SYSTEM INFORMATION ===\n");

    kernel_print("Resolution: ");
    kernel_print_hex(current_mode.width);
    kernel_print("x");
    kernel_print_hex(current_mode.height);
    kernel_print("\nColor depth: ");
    kernel_print_hex(current_mode.bpp);
    kernel_print(" bits\nFramebuffer: 0x");
    kernel_print_hex((uint32_t)current_mode.framebuffer);
    kernel_print("\nPitch: ");
    kernel_print_hex(current_mode.pitch);
    kernel_print("\nActive windows: ");
    kernel_print_hex(window_count);
    kernel_print("\n");

    // List windows
    for (int i = 0; i < window_count; i++) {
        kernel_print("Window ");
        kernel_print_hex(i);
        kernel_print(": ");
        kernel_print(windows[i].title);
        kernel_print("\n");
    }
}

// Initialize GUI management system
void gui_management_init(void) {
    kernel_print("\n=== HOUR 9: GUI FRAMEWORK ===\n");

    // Initialize GUI system
    gui_init();

    // Run GUI demonstration
    gui_demo();

    // Show GUI information
    gui_info();

    kernel_print("\n[GUI] Sacred Visual Interface ready!\n");
    kernel_print("Hour 9 Complete - Divine Graphics Active\n");
}