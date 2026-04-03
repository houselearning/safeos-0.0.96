#include "paging.h"

/* Page directory must be 4KB-aligned. We'll create 1024 PDEs mapping
   the entire 4GB address space with 4MB pages (PSE). This is simple
   and acceptable for early boot/testing in QEMU/VMs. */
static uint32_t page_directory[1024] __attribute__((aligned(4096)));

/* Macro for TLB flush - invalidates all entries in Translation Lookaside Buffer */
#define FLUSH_TLB() asm volatile("mov %%cr3, %%eax; mov %%eax, %%cr3" ::: "eax")

void paging_enable_identity_4mb(void) {
    /* Step 0: Ensure PSE is available in CR4 first, before enabling paging */
    uint32_t cr4;
    asm volatile("mov %%cr4, %0" : "=r" (cr4));
    /* Check if PSE is already supported but not enabled */
    cr4 |= (1 << 4);  /* PSE (Page Size Extensions) bit */
    asm volatile("mov %0, %%cr4" :: "r" (cr4));

    /* Step 1: Set up identity mapping - each 4MB page maps to its own physical address
       Flags: present(0x1) | writable(0x2) | 4MB page(0x80) = 0x83 */
    const uint32_t flags = 0x83;  /* P | RW | PS */
    for (uint32_t i = 0; i < 1024; ++i) {
        /* For 4MB pages, the page directory entry stores bits 31..22 of the base address
           Entry i maps virtual 4MB*i to physical 4MB*i */
        page_directory[i] = (i << 22) | flags;
    }

    /* Step 2: Load page directory into CR3 (also flushes TLB on this write) */
    asm volatile("mov %0, %%cr3" :: "r"(&page_directory) : "memory");

    /* Step 3: Enable paging via CR0.PG bit (after PSE setup and page directory loaded) */
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1u << 31);  /* PG (Paging Enable) bit */
    asm volatile("mov %0, %%cr0" :: "r" (cr0) : "memory");

    /* Step 4: Explicit TLB flush to ensure all old TLB entries are cleared */
    FLUSH_TLB();
}
