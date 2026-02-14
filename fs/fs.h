#ifndef FS_H
#define FS_H

#include <stdint.h>

#define MAX_FILES 16
#define MAX_FILENAME 32

typedef struct {
    char name[MAX_FILENAME];
    uint32_t start_addr;
    uint32_t size;
    uint8_t used;
    uint8_t is_dir;
    int16_t parent_index; // -1 for root
} file_t;

void init_fs();
int32_t fs_open(char *name, int16_t parent);
int32_t fs_read(int32_t fd, uint8_t *buffer, uint32_t size);
int32_t fs_write(int32_t fd, uint8_t *buffer, uint32_t size);
int32_t fs_create(char *name, int16_t parent, uint8_t is_dir);
int32_t fs_delete(char *name, int16_t parent);
void fs_list(int16_t parent);
uint32_t fs_get_size(int32_t fd);
int16_t fs_get_parent(int16_t fd);
uint8_t fs_is_dir(int16_t fd);
char* fs_get_name(int16_t fd);
void fs_get_path(int16_t fd, char *buffer);

#endif
