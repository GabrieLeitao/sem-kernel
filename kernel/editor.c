#include "editor.h"
#include "shell.h"
#include "input.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
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
static int32_t current_fd = -1;

static int cursor_x = 0;
static int cursor_y = 1;

#define HEADER_ATTR 0x70 // Black on light grey
#define STATUS_ATTR 0x70 // Black on light grey
#define TEXT_ATTR   0x0F // White on black

static void editor_draw_header() {
    char header[81];
    memory_set((uint8_t*)header, ' ', 80);
    header[80] = '\0';
    kprint_at_color(header, 0, 0, HEADER_ATTR);
    
    char path[256];
    fs_get_path(current_fd, path);

    char title[300];
    strcpy(title, " Editing: ");
    strcat(title, current_file);
    strcat(title, " | Path: ");
    strcat(title, path);
    
    if (strlen(title) > 79) title[79] = '\0';
    kprint_at_color(title, 0, 0, HEADER_ATTR);
}

static void editor_draw_content() {
    // Clear text area (lines 1 to 23)
    for (int y = 1; y < 24; y++) {
        char blank[81];
        memory_set((uint8_t*)blank, ' ', 80);
        blank[80] = '\0';
        kprint_at_color(blank, 0, y, TEXT_ATTR);
    }

    int cur_y = 1;
    int cur_x = 0;
    for (uint32_t i = 0; i < buffer_offset; i++) {
        if (file_buffer[i] == '\n') {
            cur_y++;
            cur_x = 0;
            if (cur_y >= 24) break;
            continue;
        }
        
        if (cur_x < 80) {
            char c_str[2] = {file_buffer[i], '\0'};
            kprint_at_color(c_str, cur_x, cur_y, TEXT_ATTR);
            cur_x++;
            if (cur_x >= 80) {
                cur_x = 0;
                cur_y++;
                if (cur_y >= 24) break;
            }
        }
    }
}

static void editor_draw_footer() {
    char footer[81];
    memory_set((uint8_t*)footer, ' ', 80);
    // CRITICAL: only print 79 chars on the last line to avoid triggering screen scroll
    footer[79] = '\0'; 
    kprint_at_color(footer, 0, 24, STATUS_ATTR);
    
    if (current_kernel_mode == MODE_SAVE_PROMPT) {
        kprint_at_color(" Save? (y/n): ", 0, 24, 0x0E);
    } else {
        kprint_at_color(" Ctrl+X: Exit and Save/Discard", 0, 24, STATUS_ATTR);
    }
}

void editor_draw_ui() {
    editor_draw_header();
    editor_draw_content();
    editor_draw_footer();
    if (current_kernel_mode == MODE_SAVE_PROMPT) {
        set_cursor(14, 24);
    } else {
        set_cursor(cursor_x, cursor_y);
    }
}

// Maps screen coordinates to buffer index
static uint32_t get_buffer_index(int x, int y) {
    uint32_t index = 0;
    int cur_x = 0;
    int cur_y = 1;

    while (index < buffer_offset) {
        if (cur_x == x && cur_y == y) return index;

        if (file_buffer[index] == '\n') {
            cur_y++;
            cur_x = 0;
        } else {
            cur_x++;
            if (cur_x >= 80) {
                cur_x = 0;
                cur_y++;
            }
        }
        index++;
    }
    return index;
}

// Sets cursor position based on a buffer index
static void set_cursor_from_index(uint32_t target_index) {
    int cur_x = 0;
    int cur_y = 1;
    for (uint32_t i = 0; i < target_index; i++) {
        if (file_buffer[i] == '\n') {
            cur_y++;
            cur_x = 0;
        } else {
            cur_x++;
            if (cur_x >= 80) {
                cur_x = 0;
                cur_y++;
            }
        }
    }
    cursor_x = cur_x;
    cursor_y = cur_y;
}

// Returns the length (x position of end of text) of a given screen row
static int get_row_len(int y) {
    int cur_x = 0;
    int cur_y = 1;
    if (y == 1 && buffer_offset == 0) return 0;

    for (uint32_t i = 0; i < buffer_offset; i++) {
        if (file_buffer[i] == '\n') {
            if (cur_y == y) return cur_x;
            cur_y++;
            cur_x = 0;
        } else {
            cur_x++;
            if (cur_x >= 80) {
                if (cur_y == y) return 80;
                cur_y++;
                cur_x = 0;
            }
        }
    }
    if (cur_y == y) return cur_x;
    return -1; // Row beyond text
}

void editor_init(char *filename, int16_t dir_idx) {
    strcpy(current_file, filename);
    current_dir = dir_idx;
    current_kernel_mode = MODE_EDIT;
    buffer_offset = 0;
    cursor_x = 0;
    cursor_y = 1;

    current_fd = fs_open(current_file, current_dir);
    if (current_fd != -1) {
        buffer_offset = fs_read(current_fd, (uint8_t*)file_buffer, 2048);
        if (buffer_offset > 2047) buffer_offset = 2047;
        file_buffer[buffer_offset] = '\0';
    } else {
        current_fd = fs_create(current_file, current_dir, 0);
    }
    
    clear_screen();
    editor_draw_ui();
}

void editor_trigger_exit() {
    if (current_kernel_mode == MODE_EDIT) {
        current_kernel_mode = MODE_SAVE_PROMPT;
        input_clear_buffer();
        editor_draw_footer();
        set_cursor(14, 24);
    }
}

void editor_handle_char(char c) {
    if (current_kernel_mode != MODE_EDIT) return;

    uint32_t index = get_buffer_index(cursor_x, cursor_y);

    if (c == 0x08) { // Backspace
        if (index > 0) {
            for (uint32_t i = index - 1; i < buffer_offset; i++) {
                file_buffer[i] = file_buffer[i+1];
            }
            buffer_offset--;
            set_cursor_from_index(index - 1);
        }
    } else {
        if (buffer_offset < 2047) {
            for (uint32_t i = buffer_offset; i > index; i--) {
                file_buffer[i] = file_buffer[i-1];
            }
            file_buffer[index] = c;
            buffer_offset++;
            file_buffer[buffer_offset] = '\0';
            set_cursor_from_index(index + 1);
        }
    }
    
    if (cursor_y > 23) cursor_y = 23;
    if (cursor_y < 1) cursor_y = 1;

    editor_draw_content();
    set_cursor(cursor_x, cursor_y);
}

void editor_handle_key(uint8_t scancode) {
    if (current_kernel_mode != MODE_EDIT) return;

    int len;
    if (scancode == 0x48) { // UP
        if (cursor_y > 1) {
            cursor_y--;
            len = get_row_len(cursor_y);
            if (cursor_x > len) cursor_x = len;
        }
    } else if (scancode == 0x50) { // DOWN
        len = get_row_len(cursor_y + 1);
        if (len != -1) {
            cursor_y++;
            if (cursor_x > len) cursor_x = len;
        }
    } else if (scancode == 0x4B) { // LEFT
        if (cursor_x > 0) {
            cursor_x--;
        } else if (cursor_y > 1) {
            cursor_y--;
            cursor_x = get_row_len(cursor_y);
        }
    } else if (scancode == 0x4D) { // RIGHT
        len = get_row_len(cursor_y);
        if (cursor_x < len) {
            cursor_x++;
        } else if (get_row_len(cursor_y + 1) != -1) {
            cursor_y++;
            cursor_x = 0;
        }
    }

    set_cursor(cursor_x, cursor_y);
}

void editor_handle_input(char *input) {
    if (current_kernel_mode == MODE_SAVE_PROMPT) {
        if (input[0] == 'y' || input[0] == 'Y') {
            fs_write(current_fd, (uint8_t*)file_buffer, buffer_offset);
            current_kernel_mode = MODE_SHELL;
            clear_screen();
            shell_print_prompt();
        } else if (input[0] == 'n' || input[0] == 'N') {
            current_kernel_mode = MODE_SHELL;
            clear_screen();
            shell_print_prompt();
        } else {
            input_clear_buffer();
            editor_draw_footer();
            set_cursor(14, 24);
        }
    }
}
