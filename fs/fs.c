#include "fs.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include "../drivers/screen.h"
#include "../drivers/vga_color.h"

file_t files[MAX_FILES];

void init_fs() {
    for (int i = 0; i < MAX_FILES; i++) {
        files[i].used = 0;
    }

    // Create /home/user structure
    int32_t home = fs_create("home", -1, 1);
    int32_t user = fs_create("user", home, 1);

    // Create a dummy file in /home/user
    int32_t fd = fs_create("hello.txt", user, 0);
    fs_write(fd, (uint8_t*)"Hello from the RAM filesystem!", 31);
}

int32_t fs_open(char *name, int16_t parent) {
    if (strcmp(name, "..") == 0) return fs_get_parent(parent);
    if (strcmp(name, ".") == 0) return parent;

    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && files[i].parent_index == parent && strcmpi(files[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int32_t fs_create(char *name, int16_t parent, uint8_t is_dir) {
    if (fs_open(name, parent) != -1) return -1;

    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].used) {
            strcpy(files[i].name, name);
            files[i].is_dir = is_dir;
            files[i].parent_index = parent;
            if (!is_dir) {
                files[i].start_addr = kmalloc(2048, 0, NULL);
            } else {
                files[i].start_addr = 0;
            }
            files[i].size = 0;
            files[i].used = 1;
            return i;
        }
    }
    return -1;
}

int32_t fs_delete(char *name, int16_t parent) {
    int32_t fd = fs_open(name, parent);
    if (fd == -1) return -1;
    
    // If it's a directory, check if empty
    if (files[fd].is_dir) {
        for (int i = 0; i < MAX_FILES; i++) {
            if (files[i].used && files[i].parent_index == fd) return -2; // Not empty
        }
    }

    files[fd].used = 0;
    return 0;
}

int32_t fs_write(int32_t fd, uint8_t *buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used || files[fd].is_dir) return -1;
    
    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t*)files[fd].start_addr)[i] = buffer[i];
    }
    files[fd].size = size;
    return size;
}

uint32_t fs_get_size(int32_t fd) {
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used) return 0;
    return files[fd].size;
}

int32_t fs_read(int32_t fd, uint8_t *buffer, uint32_t size) {
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used || files[fd].is_dir) return -1;
    
    uint32_t read_size = size < files[fd].size ? size : files[fd].size;
    for (uint32_t i = 0; i < read_size; i++) {
        buffer[i] = ((uint8_t*)files[fd].start_addr)[i];
    }
    return read_size;
}

void fs_list(int16_t parent) {
    kprint("Listing directory content:\n");
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].used && files[i].parent_index == parent) {
            if (files[i].is_dir) kprint_color("[DIR] ", VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
            else kprint("      ");
            if (files[i].is_dir) kprint_color(files[i].name, VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
            else kprint(files[i].name);
            kprint("\n");
        }
    }
}

int16_t fs_get_parent(int16_t fd) {
    if (fd == -1) return -1;
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used) return -1;
    return files[fd].parent_index;
}

uint8_t fs_is_dir(int16_t fd) {
    if (fd == -1) return 1; // Root is a dir
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used) return 0;
    return files[fd].is_dir;
}

char* fs_get_name(int16_t fd) {
    if (fd == -1) return "/";
    if (fd < 0 || fd >= MAX_FILES || !files[fd].used) return "";
    return files[fd].name;
}
