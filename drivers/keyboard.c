#include "keyboard.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "../kernel/input.h"
#include <stdint.h>

static void keyboard_callback(registers_t *regs) {
    uint8_t scancode = port_byte_in(0x60);
    input_handle_scancode(scancode);
    (void)regs;
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, keyboard_callback); 
}
