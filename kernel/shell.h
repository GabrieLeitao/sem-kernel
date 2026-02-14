#ifndef SHELL_H
#define SHELL_H

#include <stdint.h>

void shell_init();
void shell_print_prompt();
void shell_handle_input(char *input);

#endif
