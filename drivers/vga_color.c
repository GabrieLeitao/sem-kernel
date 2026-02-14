#include "vga_color.h"
#include "screen.h"

void kprint_color(char *message, enum vga_color fg, enum vga_color bg) {
    uint8_t color = vga_entry_color(fg, bg);
    kprint_at_color(message, -1, -1, color);
}

void kprint_info(char *message) {
    kprint_color("[INFO] ", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    kprint(message);
}

void kprint_error(char *message) {
    kprint_color("[ERROR] ", VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    kprint(message);
}

void kprint_warning(char *message) {
    kprint_color("[WARN] ", VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
    kprint(message);
}
