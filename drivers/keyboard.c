#include "keyboard.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"
#include "../libc/function.h"
#include "../kernel/kernel.h"
#include <stdint.h>
#include <stdbool.h>

#define BACKSPACE 0x0E
#define ENTER 0x1C
#define LSHIFT 0x2A
#define RSHIFT 0x36
#define LSHIFT_REL 0xAA
#define RSHIFT_REL 0xB6
#define LCTRL 0x1D
#define LCTRL_REL 0x9D

static char key_buffer[256];
static bool shift_pressed;
static bool ctrl_pressed;

extern void editor_trigger_exit();

#define SC_MAX 57
const char sc_ascii[] = { '?', '?', '1', '2', '3', '4', '5', '6',     
    '7', '8', '9', '0', '-', '=', '?', '?', 'q', 'w', 'e', 'r', 't', 'y', 
        'u', 'i', 'o', 'p', '[', ']', '?', '?', 'a', 's', 'd', 'f', 'g', 
        'h', 'j', 'k', 'l', ';', '\'', '`', '?', '\\', 'z', 'x', 'c', 'v', 
        'b', 'n', 'm', ',', '.', '/', '?', '?', '?', ' '};

const char sc_ascii_shift[] = { '?', '?', '!', '@', '#', '$', '%', '^',     
    '&', '*', '(', ')', '_', '+', '?', '?', 'Q', 'W', 'E', 'R', 'T', 'Y', 
        'U', 'I', 'O', 'P', '{', '}', '?', '?', 'A', 'S', 'D', 'F', 'G', 
        'H', 'J', 'K', 'L', ':', '\"', '~', '?', '|', 'Z', 'X', 'C', 'V', 
        'B', 'N', 'M', '<', '>', '?', '?', '?', '?', ' '};

static void keyboard_callback(registers_t *regs) {
    uint8_t scancode = port_byte_in(0x60);
    
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

    if (ctrl_pressed && scancode == 0x2D) { // 'x'
        editor_trigger_exit();
        return;
    }

    if (scancode > SC_MAX) return;

    if (scancode == BACKSPACE) {
        if (strlen(key_buffer) > 0) {
            str_pop_back(key_buffer);
            kprint_backspace();
        }
    } else if (scancode == ENTER) {
        kprint("\n");
        char temp_buffer[256];
        strcpy(temp_buffer, key_buffer);
        key_buffer[0] = '\0';
        user_input(temp_buffer);
    } else {
        char letter = shift_pressed ? sc_ascii_shift[(int)scancode] : sc_ascii[(int)scancode];
        if (letter != '?') {
            char str[2] = {letter, '\0'};
            append(key_buffer, letter);
            kprint(str);
        }
    }
    UNUSED(regs);
}

void init_keyboard() {
   shift_pressed = false;
   ctrl_pressed = false;
   key_buffer[0] = '\0';
   register_interrupt_handler(IRQ1, keyboard_callback); 
}
