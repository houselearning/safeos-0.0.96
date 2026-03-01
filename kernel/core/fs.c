#include "fs.h"
#include "string.h"

int fs_init(void) { (void)0; return 0; }
int fs_list(const char *path, fs_entry_t *out, int max) { (void)path; (void)out; (void)max; return 0; }
int fs_read_file(const char *path, void *buf, uint32_t max) { (void)path; (void)buf; (void)max; return 0; }
int fs_write_file(const char *path, const void *buf, uint32_t size) { (void)path; (void)buf; (void)size; return 0; }
int fs_create_file(const char *path) { (void)path; return 0; }
int fs_delete_file(const char *path) { (void)path; return 0; }
