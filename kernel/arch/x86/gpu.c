#include "gpu.h"
#include "framebuffer.h"
#include "../../core/paging.h"
#include "../../core/pci.h"
#include <stddef.h>
#include <stdint.h>

#define GPU_VRAM_VADDR ((uint32_t)0xC0000000u)

static uint32_t gpu_fb_phys = 0;
static uint32_t gpu_fb_size = 0;
static uint8_t *gpu_fb_virt = NULL;

static void gpu_serial_puts(const char *s) {
    while (*s) {
        __asm__ __volatile__("outb %%al, %%dx" :: "a"(*s), "d"((unsigned short)0x3f8));
        s++;
    }
}

static void gpu_serial_puthex(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i) {
        uint8_t nib = (v >> (i*4)) & 0xF;
        __asm__ __volatile__("outb %%al, %%dx" :: "a"(hex[nib]), "d"((unsigned short)0x3f8));
    }
}

uint32_t gpu_get_framebuffer_phys(void) {
    return gpu_fb_phys;
}

uint8_t *gpu_get_framebuffer_virt(void) {
    return gpu_fb_virt;
}

int gpu_init(void) {
    extern uint8_t* fb_address;
    extern uint32_t fb_width, fb_height, fb_pitch, fb_bpp;

    if (fb_address) {
        gpu_fb_phys = (uint32_t)(uintptr_t)fb_address;
        gpu_fb_virt = fb_address;
        gpu_fb_size = fb_width * fb_height * ((fb_bpp + 7) / 8);
        gpu_serial_puts("GPU: using existing framebuffer from bootloader\n");
        return 0;
    }

    gpu_serial_puts("GPU: no bootloader framebuffer, scanning PCI VGA devices...\n");

    for (uint16_t bus = 0; bus < 256; ++bus) {
        for (uint8_t slot = 0; slot < 32; ++slot) {
            for (uint8_t func = 0; func < 8; ++func) {
                uint16_t vendor = pci_config_read16(bus, slot, func, 0x00);
                if (vendor == 0xFFFF) continue;

                uint16_t device = pci_config_read16(bus, slot, func, 0x02);
                uint32_t classreg = pci_config_read32(bus, slot, func, 0x08);
                uint8_t class_code = (classreg >> 24) & 0xFF;
                uint8_t subclass = (classreg >> 16) & 0xFF;

                if (class_code != 0x03) continue;
                if (subclass != 0x00 && subclass != 0x01 && subclass != 0x80) continue; // VGA misc supported

                gpu_serial_puts("GPU: VGA device found bus=");
                gpu_serial_puthex(bus);
                gpu_serial_puts(" slot=");
                gpu_serial_puthex(slot);
                gpu_serial_puts(" func=");
                gpu_serial_puthex(func);
                gpu_serial_puts(" vendor=");
                gpu_serial_puthex(vendor);
                gpu_serial_puts(" device=");
                gpu_serial_puthex(device);
                gpu_serial_puts("\n");

                uint32_t bar0 = pci_config_read32(bus, slot, func, 0x10);
                if (bar0 == 0 || (bar0 & 1)) continue; /* skip IO bars */
                uint32_t phys = bar0 & 0xFFFFFFF0;
                if (!phys) continue;

                /* Determine a safe GPU framebuffer size guess (3MB for 1024x768x32) */
                gpu_fb_size = 1024u * 768u * 4u;
                gpu_fb_phys = phys;
                gpu_fb_virt = (uint8_t *)(uintptr_t)GPU_VRAM_VADDR;

                uint32_t pages = (gpu_fb_size + 0x3FFFFFu) >> 22; /* ceil 4MB pages */
                if (pages == 0) pages = 1;
                for (uint32_t p = 0; p < pages; ++p) {
                    paging_map_4mb(GPU_VRAM_VADDR + p * 0x400000u,
                                   (phys & 0xFFC00000u) + p * 0x400000u,
                                   0x03u); /* present, writable */
                }

                uint32_t width = 1024;
                uint32_t height = 768;
                uint32_t pitch = width * 4;
                uint32_t bpp = 32;

                fb_init(gpu_fb_virt, width, height, pitch, bpp);
                fb_width = width;
                fb_height = height;
                fb_pitch = pitch;
                fb_bpp = bpp;

                gpu_serial_puts("GPU: mapped framebuffer virt=0x");
                gpu_serial_puthex((uint32_t)(uintptr_t)gpu_fb_virt);
                gpu_serial_puts(" phys=0x");
                gpu_serial_puthex(gpu_fb_phys);
                gpu_serial_puts("\n");

                return 0;
            }
        }
    }

    gpu_serial_puts("GPU: no VGA framebuffer found in PCI scan\n");
    return -1;
}
