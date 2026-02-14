#include "editor.h"
#include "shell.h"
#include "../drivers/screen.h"
#include "../fs/fs.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include <stdint.h>

extern void shell_print_prompt();

kernel_mode_t current_kernel_mode = MODE_SHELL;

static char current_file[MAX_FILENAME];
static char file_buffer[2048];
static uint32_t buffer_offset = 0;
static int16_t current_dir = -1;

#define HEADER_ATTR 0x70 // Black on light grey
#define STATUS_ATTR 0x70 // Black on light grey
#define TEXT_ATTR   0x0F // White on black

void editor_draw_ui() {
    clear_screen();
    
    // Draw Header
    char header[81];
    memory_set((uint8_t*)header, ' ', 80);
    header[80] = '\0';
    kprint_at_color(header, 0, 0, HEADER_ATTR);
    kprint_at_color(" My-Kernel Editor: ", 0, 0, HEADER_ATTR);
    kprint_at_color(current_file, 19, 0, HEADER_ATTR);

    // Draw Buffer (limit to 22 lines)
    set_cursor(0, 1);
    if (buffer_offset > 0) {
        char temp[2049];
        memory_copy((uint8_t*)file_buffer, (uint8_t*)temp, buffer_offset);
        temp[buffer_offset] = '\0';
        kprint(temp);
    }

    // Set backspace limit to the line after the header
    set_backspace_limit(get_offset(0, 1));

    // Draw Status Bar at the bottom (line 24)
    char footer[81];
    memory_set((uint8_t*)footer, ' ', 80);
    footer[80] = '\0';
    kprint_at_color(footer, 0, 24, STATUS_ATTR);
    kprint_at_color("Ctrl+X - Exit and Save/Discard", 0, 24, STATUS_ATTR);

    // Position cursor at the end of the text
    // (This is implicitly done by the last kprint(temp) if it ended there)
}

void editor_init(char *filename, int16_t dir_idx) {
    strcpy(current_file, filename);
    current_dir = dir_idx;
    current_kernel_mode = MODE_EDIT;
    buffer_offset = 0;

    int32_t fd = fs_open(current_file, current_dir);
    if (fd != -1) {
        buffer_offset = fs_read(fd, (uint8_t*)file_buffer, 2048);
    }
    editor_draw_ui();
}

void editor_trigger_exit() {
    if (current_kernel_mode == MODE_EDIT) {
        current_kernel_mode = MODE_SAVE_PROMPT;
        // Print prompt on line 23 to avoid messing with footer or text
        kprint_at_color(" Save changes? (y/n): ", 0, 23, 0x0E); // Yellow on black
        set_backspace_limit(get_cursor_offset());
    }
}

void editor_handle_input(char *input) {
    if (current_kernel_mode == MODE_SAVE_PROMPT) {
        if (strcmpi(input, "y") == 0) {
            int32_t fd = fs_open(current_file, current_dir);
            if (fd == -1) fd = fs_create(current_file, current_dir, 0);
            fs_write(fd, (uint8_t*)file_buffer, buffer_offset);
            current_kernel_mode = MODE_SHELL;
            clear_screen();
            shell_print_prompt();
        } else if (strcmpi(input, "n") == 0) {
            current_kernel_mode = MODE_SHELL;
            clear_screen();
            shell_print_prompt();
        } else {
            kprint_at_color(" Invalid. Save changes? (y/n): ", 0, 23, 0x0E);
        }
    } else if (current_kernel_mode == MODE_EDIT) {
        size_t len = strlen(input);
        for (size_t i = 0; i < len; i++) {
            if (buffer_offset < 2047) {
                file_buffer[buffer_offset++] = input[i];
            }
        }
        if (buffer_offset < 2047) {
            file_buffer[buffer_offset++] = '\n';
        }
        // Redraw to keep UI clean
        editor_draw_ui();
    }
}
