#include "../cpu/isr.h"
#include "../cpu/gdt.h"
#include "../cpu/tss.h"
#include "../cpu/paging.h"
#include "../cpu/syscall.h"
#include "../fs/fs.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../drivers/vga_color.h"
#include "kernel.h"
#include "shell.h"
#include "editor.h"
#include "input.h"
#include "../libc/string.h"
#include <stdint.h>

void kernel_main() {
    clear_screen();
    init_gdt();
    set_kernel_stack(0x90000);
    isr_install();
    irq_install();
    initialize_paging();
    init_fs();
    init_syscalls();
    
    input_init();
    init_keyboard();

    shell_init();

    kprint_color("Sem Kernel", VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    kprint(" version 0.1\n");
    kprint_info("Kernel initialized successfully.\n");
    kprint("type 'help' for commands.\n");
    shell_print_prompt();
    set_backspace_limit(get_cursor_offset());
}

void user_input(char *input) {
    if (current_kernel_mode == MODE_SHELL) {
        shell_handle_input(input);
        if (current_kernel_mode == MODE_SHELL) {
            shell_print_prompt();
        }
    } else {
        editor_handle_input(input);
    }
    
    // Always set backspace limit based on the current prompt/status
    set_backspace_limit(get_cursor_offset());
}

void user_key_press(uint8_t scancode) {
    if (current_kernel_mode == MODE_SHELL) {
        if (scancode == 0x48) { // UP
            shell_history_up();
        } else if (scancode == 0x50) { // DOWN
            shell_history_down();
        }
    } else if (current_kernel_mode == MODE_EDIT) {
        editor_handle_key(scancode);
    }
}
