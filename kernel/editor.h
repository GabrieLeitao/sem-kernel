#ifndef EDITOR_H
#define SHELL_H

#include <stdint.h>

typedef enum { 
    MODE_SHELL, 
    MODE_EDIT, 
    MODE_SAVE_PROMPT 
} kernel_mode_t;

extern kernel_mode_t current_kernel_mode;

void editor_init(char *filename, int16_t dir_idx);
void editor_handle_input(char *input);
void editor_trigger_exit();
void editor_draw_ui();

#endif
