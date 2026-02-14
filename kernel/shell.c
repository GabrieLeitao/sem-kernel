#include "shell.h"
#include "editor.h"
#include "../drivers/screen.h"
#include "../drivers/vga_color.h"
#include "../fs/fs.h"
#include "../libc/string.h"
#include "../cpu/ports.h"
#include <stdint.h>

static int16_t current_dir_idx = -1;

extern void jump_to_user_mode();

void shell_init() {
    int32_t home = fs_open("home", -1);
    int32_t user = fs_open("user", home);
    current_dir_idx = user;
}

void shell_print_prompt() {
    if (current_dir_idx == -1) {
        kprint_color("/ > ", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    } else {
        kprint_color(fs_get_name(current_dir_idx), VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        kprint_color(" > ", VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    }
}

void shell_handle_input(char *input) {
    if (strcmp(input, "exit") == 0 || strcmp(input, "exit") == 0) {
        kprint("Shutting down...\n");
        port_word_out(0x604, 0x2000);
        port_word_out(0xB004, 0x2000);
        port_word_out(0x4004, 0x3400);
        asm volatile("hlt");
    } else if (strcmp(input, "help") == 0) {
        kprint("Commands: ls, cd <dir>, mkdir <dir>, touch <file>, rm <file/dir>, cat <file>, edit <file>, user, clear, exit\n");
    } else if (strcmp(input, "ls") == 0) {
        fs_list(current_dir_idx);
    } else if (strncmp(input, "touch ", 6) == 0) {
        char *file = input + 6;
        if (fs_create(file, current_dir_idx, 0) == -1) {
            kprint("Error creating file.\n");
        }
    } else if (strncmp(input, "cd ", 3) == 0) {
        char *dir = input + 3;
        if (strcmp(dir, "/") == 0) {
            current_dir_idx = -1;
        } else {
            int32_t fd = fs_open(dir, current_dir_idx);
            if (fd != -1 && fs_is_dir(fd)) {
                current_dir_idx = fd;
            } else if (fd != -1 && !fs_is_dir(fd)) {
                kprint("Not a directory.\n");
            } else {
                kprint("Directory not found.\n");
            }
        }
    } else if (strncmp(input, "mkdir ", 6) == 0) {
        char *dir = input + 6;
        if (fs_create(dir, current_dir_idx, 1) == -1) {
            kprint("Error creating directory.\n");
        }
    } else if (strncmp(input, "rm ", 3) == 0) {
        char *file = input + 3;
        int32_t res = fs_delete(file, current_dir_idx);
        if (res == -1) {
            kprint("Not found.\n");
        }
        else if (res == -2) kprint("Error deleting directory: Directory not empty.\n");
    } else if (strncmp(input, "cat ", 4) == 0) {
        char *filename = input + 4;
        int32_t fd = fs_open(filename, current_dir_idx);
        if (fd == -1 || fs_is_dir(fd)) {
            kprint("File not found.\n");
        } else {
            uint32_t size = fs_get_size(fd);
            char content[2048];
            fs_read(fd, (uint8_t*)content, size);
            content[size] = '\0';
            kprint(content);
            kprint("\n");
        }
    } else if (strncmp(input, "edit ", 5) == 0) {
        char *filename = input + 5;
        editor_init(filename, current_dir_idx);
    } else if (strcmp(input, "user") == 0) {
        kprint("Jumping to User Mode...\n");
        jump_to_user_mode();
        kprint("\nReturned from User Mode.\n");
        shell_print_prompt();
        set_backspace_limit(get_cursor_offset());
    } else if (strcmp(input, "clear") == 0) {
        clear_screen();
    } else if (strlen(input) > 0) {
        kprint("Unknown command: ");
        kprint(input);
        kprint("\n");
    }
}
