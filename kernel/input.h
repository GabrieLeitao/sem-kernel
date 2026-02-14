#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

void input_init();
void input_handle_scancode(uint8_t scancode);

// For shell/editor to interact with the input system
void input_clear_buffer();
void input_set_buffer(char *str);

#endif
