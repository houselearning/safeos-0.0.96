#ifndef FS_H
#define FS_H

#include <stdint.h>

typedef struct {
    char name[64];
    uint8_t is_dir;
    uint32_t size;
} fs_entry_t;

int fs_init(void);                    // mount A:/
int fs_list(const char *path, fs_entry_t *out, int max);
int fs_read_file(const char *path, void *buf, uint32_t max);
int fs_write_file(const char *path, const void *buf, uint32_t size);
int fs_create_file(const char *path);
int fs_delete_file(const char *path);

#endif
