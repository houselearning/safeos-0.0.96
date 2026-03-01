#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int strcmp(const char *a, const char *b);
size_t strlen(const char *s);

#endif
