#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

// Minimal kernel printf
int kprintf(const char *fmt, ...);

// Serial I/O helper functions available globally.
void serial_putc(char c);
void serial_puts(const char *s);
void serial_puthex(uint32_t v);

// Minimal snprintf (used by file explorer)
int sprintf(char *out, const char *fmt, ...);
int snprintf(char *out, size_t size, const char *fmt, ...);

int sscanf(const char *str, const char *fmt, ...);

#endif
