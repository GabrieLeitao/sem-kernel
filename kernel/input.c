#include "input.h"
#include "kernel.h"
#include "shell.h"
#include "editor.h"
#include "../drivers/screen.h"
#include "../libc/string.h"
#include <stdbool.h>

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LSHIFT_REL 0xAA
#define RSHIFT_REL 0xB6
#define LCTRL 0x1D
#define LCTRL_REL 0x9D

#define SC_UP 0x48
#define SC_DOWN 0x50
#define SC_LEFT 0x4B
#define SC_RIGHT 0x4D
#define SC_X 0x2D

#define SC_MAX 57
static const char sc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y', 
        'u', 'i', 'o', 'p', '[', ']', '?', '?', 'a', 's', 'd', 'f', 'g', 
        'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
        'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

static const char sc_ascii_shift[] = { '?', '?', '!', '@', '#', '$', '%', '^',     
    '&', '*', '(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ':', '\"', '~', '?', '|', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' '};

static bool shift_pressed = false;
static bool ctrl_pressed = false;
static char line_buffer[256];

void input_init() {
    shift_pressed = false;
    ctrl_pressed = false;
    line_buffer[0] = '\0';
}

void input_clear_buffer() {
    line_buffer[0] = '\0';
}

void input_set_buffer(char *str) {
    // 1. Clear current buffer from screen
    int len = strlen(line_buffer);
    for (int i = 0; i < len; i++) {
        kprint_backspace();
    }
    // 2. Update buffer
    strcpy(line_buffer, str);
    // 3. Print new buffer
    kprint(line_buffer);
}

void input_handle_scancode(uint8_t scancode) {
    // Handle Modifiers
    if (scancode == LSHIFT || scancode == RSHIFT) {
        shift_pressed = true;
        return;
    }
    if (scancode == LSHIFT_REL || scancode == RSHIFT_REL) {
        shift_pressed = false;
        return;
    }
    if (scancode == LCTRL) {
        ctrl_pressed = true;
        return;
    }
    if (scancode == LCTRL_REL) {
        ctrl_pressed = false;
        return;
    }

    // Handle CTRL shortcuts
    if (ctrl_pressed && scancode == SC_X) {
        editor_trigger_exit();
        return;
    }

    // Handle special keys (Arrows)
    if (scancode == SC_UP || scancode == SC_DOWN || scancode == SC_LEFT || scancode == SC_RIGHT) {
        user_key_press(scancode);
        return;
    }

    if (scancode > SC_MAX) return;

    // Handle Backspace
    if (scancode == BACKSPACE) {
        if (current_kernel_mode == MODE_EDIT) {
            editor_handle_char(0x08);
        } else if (strlen(line_buffer) > 0) {
            str_pop_back(line_buffer);
            kprint_backspace();
        }
        return;
    }

    // Handle Enter
    if (scancode == ENTER) {
        if (current_kernel_mode == MODE_EDIT) {
            editor_handle_char('\n');
        } else {
            kprint("\n");
            char temp[256];
            strcpy(temp, line_buffer);
            line_buffer[0] = '\0';
            user_input(temp);
        }
        return;
    }

    // Handle ASCII characters
    char ascii = shift_pressed ? sc_ascii_shift[(int)scancode] : sc_ascii[(int)scancode];
    if (ascii != '?') {
        if (current_kernel_mode == MODE_EDIT) {
            editor_handle_char(ascii);
        } else {
            char str[2] = {ascii, '\0'};
            append(line_buffer, ascii);
            kprint(str);
        }
    }
}
