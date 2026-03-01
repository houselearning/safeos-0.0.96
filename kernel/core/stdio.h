#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>

// Minimal kernel printf
int kprintf(const char *fmt, ...);

// Minimal snprintf (used by file explorer)
int ksprintf(char *out, const char *fmt, ...);
int ksnprintf(char *out, size_t size, const char *fmt, ...);

#endif
