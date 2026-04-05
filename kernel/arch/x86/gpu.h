#ifndef GPU_H
#define GPU_H

#include <stdint.h>

/* Initialize GPU framebuffer support.
   - If multiboot framebuffer already exists, it wraps it.
   - Otherwise scans PCI VGA class devices and maps the first framebuffer BAR.
   Returns 0 on success, non-zero on failure. */
int gpu_init(void);

/* Returns the PCI-detected VRAM physical base (0 if unknown) */
uint32_t gpu_get_framebuffer_phys(void);

/* Returns the logged framebuffer virtual address (0 if unmapped). */
uint8_t *gpu_get_framebuffer_virt(void);

#endif