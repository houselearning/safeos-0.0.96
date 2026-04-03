#include "string.h"

void *memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

void *memset(void *dst, int c, size_t n) {
    unsigned char *d = dst;
    while (n--) *d++ = (unsigned char)c;
    return dst;
}

int strcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return *(unsigned char*)a - *(unsigned char*)b;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (*s++) len++;
    return len;
}

char *strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

int strncmp(const char *a, const char *b, size_t n) {
    while (n && *a && (*a == *b)) {
        a++; b++; n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)a - *(unsigned char*)b;
}

char *strchr(const char *s, int c) {
    while (*s && *s != (char)c) {
        s++;
    }
    if (*s == (char)c) return (char *)s;
    return 0;
}