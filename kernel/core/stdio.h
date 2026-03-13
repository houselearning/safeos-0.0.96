#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>

// Minimal kernel printf
int kprintf(const char *fmt, ...);

// Minimal snprintf (used by file explorer)
int sprintf(char *out, const char *fmt, ...);
int snprintf(char *out, size_t size, const char *fmt, ...);

int sscanf(const char *str, const char *fmt, ...);

#endif
