/* Minimal paging helper: identity-map 0..4GB with 4MB pages so
   the kernel can access physical framebuffer addresses provided by GRUB.
   
   Identity mapping means virtual address VA directly maps to physical address PA.
   This is safe to do early in boot and doesn't require complex address translation. */
#ifndef KERNEL_CORE_PAGING_H
#define KERNEL_CORE_PAGING_H

#include <stdint.h>

/* Create identity mapping using 4MB pages and enable paging.
   
   Initialization sequence:
   1. Enable PSE (4MB page support) in CR4
   2. Set up page directory with identity mappings
   3. Load page directory into CR3
   4. Enable paging via CR0.PG bit
   5. Flush TLB to ensure old translations are cleared
   
   After this, virtual addresses 0x00000000-0xFFFFFFFF map to the same
   physical addresses, maintaining all current memory access patterns. */
void paging_enable_identity_4mb(void);

/* Map 4MB-aligned virtual page to 4MB-aligned physical page with given flags.
   Flags should include at least present (0x1) and writable (0x2).
   This allows a GPU driver to map framebuffer physical VRAM into a stable
   virtual aperture (e.g. 0xC0000000). */
void paging_map_4mb(uint32_t virt_addr, uint32_t phys_addr, uint32_t flags);

#endif
