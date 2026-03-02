#include "paging.h"

/* Page directory must be 4KB-aligned. We'll create 1024 PDEs mapping
   the entire 4GB address space with 4MB pages (PSE). This is simple
   and acceptable for early boot/testing in QEMU/VMs. */
static uint32_t page_directory[1024] __attribute__((aligned(4096)));

void paging_enable_identity_4mb(void) {
    const uint32_t flags = 0x83; /* present(1) | rw(1) | PS(1<<7) */
    for (uint32_t i = 0; i < 1024; ++i) {
        /* For 4MB pages, PDE base is bits 22..31 (entry = physi_addr & 0xFFC00000) */
        page_directory[i] = (i << 22) | flags;
    }

    /* Load page directory into CR3 */
    asm volatile("mov %0, %%cr3" :: "r"(&page_directory));

    /* Enable PSE (Page Size Extensions) in CR4 so 4MB pages are allowed */
    uint32_t cr4;
    asm volatile("mov %%cr4, %0" : "=r" (cr4));
    cr4 |= (1 << 4); /* PSE bit */
    asm volatile("mov %0, %%cr4" :: "r" (cr4));

    /* Finally enable paging by setting PG bit in CR0 */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1u << 31); /* PG */
    asm volatile("mov %0, %%cr0" :: "r" (cr0));
}
