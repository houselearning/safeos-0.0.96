#include "fs.h"
#include "string.h"

#define MAX_FILES 10

typedef struct {
    char name[64];
    char content[4096];
    uint32_t size;
    int exists;
} file_t;

static file_t files[MAX_FILES];

int fs_init(void) {
    memset(files, 0, sizeof(files));
    // Add default files
    strcpy(files[0].name, "readme.txt");
    strcpy(files[0].content, "Welcome to SafeOS!\n\nThis OS features:\n- GUI Desktop\n- Apps: Notepad, Calculator, Spreadsheet, File Explorer, Browser\n- Networking\n- Mouse and Keyboard support\n\nEnjoy!");
    files[0].size = strlen(files[0].content);
    files[0].exists = 1;

    strcpy(files[1].name, "config.ini");
    strcpy(files[1].content, "[settings]\nversion=1.0\ntheme=dark\n");
    files[1].size = strlen(files[1].content);
    files[1].exists = 1;

    return 0;
}

int fs_list(const char *path, fs_entry_t *out, int max) {
    if (strcmp(path, "A:/") != 0) return 0;
    int count = 0;
    for (int i = 0; i < MAX_FILES && count < max; i++) {
        if (files[i].exists) {
            strcpy(out[count].name, files[i].name);
            out[count].is_dir = 0;
            out[count].size = files[i].size;
            count++;
        }
    }
    return count;
}

int fs_read_file(const char *path, void *buf, uint32_t max) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].exists && strcmp(files[i].name, path) == 0) {
            uint32_t to_copy = files[i].size < max ? files[i].size : max;
            memcpy(buf, files[i].content, to_copy);
            return to_copy;
        }
    }
    return -1;
}

int fs_write_file(const char *path, const void *buf, uint32_t size) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].exists && strcmp(files[i].name, path) == 0) {
            uint32_t to_copy = size < sizeof(files[i].content) ? size : sizeof(files[i].content);
            memcpy(files[i].content, buf, to_copy);
            files[i].size = to_copy;
            return 0;
        }
    }
    // Create new
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].exists) {
            strcpy(files[i].name, path);
            uint32_t to_copy = size < sizeof(files[i].content) ? size : sizeof(files[i].content);
            memcpy(files[i].content, buf, to_copy);
            files[i].size = to_copy;
            files[i].exists = 1;
            return 0;
        }
    }
    return -1;
}

int fs_create_file(const char *path) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (!files[i].exists) {
            strcpy(files[i].name, path);
            files[i].content[0] = '\0';
            files[i].size = 0;
            files[i].exists = 1;
            return 0;
        }
    }
    return -1;
}

int fs_delete_file(const char *path) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (files[i].exists && strcmp(files[i].name, path) == 0) {
            files[i].exists = 0;
            return 0;
        }
    }
    return -1;
}
