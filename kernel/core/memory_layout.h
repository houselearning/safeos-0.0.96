/* Memory layout definitions for SafeOS
 * 
 * This header defines the physical memory layout to prevent collisions
 * between kernel code, data, heap, stack, and device MMIO regions.
 */

#ifndef KERNEL_CORE_MEMORY_LAYOUT_H
#define KERNEL_CORE_MEMORY_LAYOUT_H

#include <stdint.h>

/* extern symbols from linker script */
extern uint32_t _etext;   /* End of .text section (code) */
extern uint32_t _edata;   /* End of .data section (initialized data) */
extern uint32_t _bss_end; /* End of .bss section (uninitialized data) */
extern uint32_t _end;     /* End of kernel image (same as _bss_end) */

/* Memory region definitions */

/* Kernel load address (GRUB multiboot standard) */
#define KERNEL_START 0x00100000  /* 1 MB */

/* Kernel image ends at _end symbol; heap starts after it */
#define HEAP_START   0x00400000  /* 4 MB - safe distance from kernel */
#define HEAP_SIZE    0x00400000  /* 4 MB heap */
#define HEAP_END     (HEAP_START + HEAP_SIZE)  /* 8 MB */

/* Stack is in .bss section, grows downward from high memory in BSS */
/* Stack size is defined in boot.s as 64 KB */
#define STACK_SIZE   0x00010000  /* 64 KB */

/* Framebuffer fallback address - placed after heap */
/* Using identity 4MB-page mapping, so any aligned region works */
#define FRAMEBUFFER_FALLBACK 0x00800000  /* 8 MB - after heap */
#define FRAMEBUFFER_SIZE     0x00400000  /* 4 MB (1024x768x32) */

/* Verify layout doesn't overlap */
#if HEAP_END > FRAMEBUFFER_FALLBACK
#error "Heap overlaps with framebuffer fallback region!"
#endif

/* Helper to check if pointer is in valid heap range */
static inline int is_valid_heap_ptr(const void *ptr) {
    uintptr_t p = (uintptr_t)ptr;
    return (p >= HEAP_START && p < HEAP_END);
}

#endif
