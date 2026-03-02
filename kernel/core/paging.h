/* Minimal paging helper: identity-map 0..4GB with 4MB pages so
   the kernel can access physical framebuffer addresses provided by GRUB. */
#ifndef KERNEL_CORE_PAGING_H
#define KERNEL_CORE_PAGING_H

#include <stdint.h>

/* Create identity mapping using 4MB pages and enable paging. */
void paging_enable_identity_4mb(void);

#endif
