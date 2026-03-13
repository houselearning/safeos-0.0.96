#ifndef CORE_MEMORY_H
#define CORE_MEMORY_H

#include <stdint.h>
#include <stddef.h>

// Initialize memory management
void memory_init(void);

// Kernel heap allocation
void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
