#include "../arch/x86/interrupts.h"
#include "../arch/x86/keyboard.h"
#include "../arch/x86/mouse.h"
#include "../arch/x86/framebuffer.h"
#include <stdint.h>
#include "memory.h"
#include "paging.h"
#include "pci.h"
#include "../gui/gui.h"
#include "../gui/desktop.h"
#include "text_desktop.h"
#include "fs.h"
#include "net.h"

/* Simple serial helpers for runtime diagnostics (file-scope) */
static void serial_putc(char c) {
    __asm__ __volatile__("outb %%al, %%dx" :: "a"(c), "d"((unsigned short)0x3f8));
}
static void serial_puts(const char *s) {
    while (*s) serial_putc(*s++);
}
/* print 32-bit hex (no prefix) */
static void serial_puthex(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i) {
        uint8_t nib = (v >> (i*4)) & 0xF;
        serial_putc(hex[nib]);
    }
}

static void boot_progress(int pct) {
    /* Draw a small progress bar in the top-left corner if framebuffer is available. */
    if (fb_address) {
        int x = 10;
        int y = 10;
        int w = 220;
        int h = 18;
        gui_draw_rect(x, y, w, h, 0x202020);
        gui_draw_rect(x+2, y+2, w-4, h-4, 0x404040);

        int steps = 10;
        int filled = (pct * steps) / 100;
        for (int i = 0; i < steps; i++) {
            uint32_t color = (i < filled) ? 0x00FF00 : 0x303030;
            gui_draw_rect(x+4 + i*( (w-8)/steps ), y+4, (w-8)/steps - 2, h-10, color);
        }

        char buf[64];
        if (pct >= 100) {
            int n = snprintf(buf, sizeof(buf), "Ready");
            (void)n;
            gui_draw_text(x+4, y+h+4, buf, 0x00FF00, 0x202020);
        } else {
            int n = snprintf(buf, sizeof(buf), "Booting: [%3d%%]", pct);
            (void)n;
            gui_draw_text(x+4, y+h+4, buf, 0xFFFFFF, 0x202020);
        }
    }
}

void kmain(unsigned long magic, unsigned long addr) {
    (void)magic;
    (void)addr;

    /* Very early serial tick so headless consoles know the kernel started. */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('!'), "d"((unsigned short)0x3f8));

    /* Log multiboot registers passed by the bootloader (always print addr) */
    serial_puts("ENTRY: magic=0x"); serial_puthex((uint32_t)magic); serial_puts(" addr=0x"); serial_puthex((uint32_t)addr); serial_putc('\n');

    /* Also print raw addr even if zero to aid debugging */
    serial_puts("ENTRY_RAW_ADDR=0x"); serial_puthex((uint32_t)addr); serial_putc('\n');

    /* (serial helpers are file-scope) */

    interrupts_init();
    serial_puts("INTERRUPTS OK\n");
    boot_progress(10);

    memory_init();
    serial_puts("MEMORY OK\n");
    boot_progress(20);

    /* Enable an identity 4MB-page mapping so the kernel can access
       physical regions (e.g. framebuffer physbase) directly. */
    paging_enable_identity_4mb();
    serial_puts("PAGING OK\n");
    boot_progress(30);

     /* If the bootloader provided multiboot info, try to extract VBE mode
         information (linear framebuffer address, resolution, pitch, bpp)
         and initialize the framebuffer with the real values. Otherwise
         fall back to conservative defaults. */
     if (addr) {
        typedef struct {
            uint32_t flags;
            uint32_t mem_lower, mem_upper;
            uint32_t boot_device;
            uint32_t cmdline;
            uint32_t mods_count, mods_addr;
            union {
                struct { uint32_t tabsize, strsize, addr, shndx; } aout_sym;
                struct { uint32_t num, size, addr, shndx; } elf_sec;
            } u;
            uint32_t mmap_length, mmap_addr;
            uint32_t drives_length, drives_addr;
            uint32_t config_table;
            uint32_t boot_loader_name;
            uint32_t apm_table;
            uint32_t vbe_control_info;
            uint32_t vbe_mode_info;
            uint16_t vbe_mode;
            uint16_t vbe_interface_seg;
            uint16_t vbe_interface_off;
            uint16_t vbe_interface_len;
        } multiboot_info_t;

        multiboot_info_t *mb = (multiboot_info_t *)(uintptr_t)addr;
        serial_puts("MB: addr=0x"); serial_puthex((uint32_t)addr); serial_puts(" flags=0x"); serial_puthex(mb->flags); serial_puts(" vbe_mode_info=0x"); serial_puthex(mb->vbe_mode_info); serial_puts(" vbe_mode=0x"); serial_puthex(mb->vbe_mode); serial_putc('\n');

        /* Prefer using the vbe_mode_info pointer directly when provided by
           the bootloader; some boot environments may not set the flags
           bit consistently. */
        if (mb->vbe_mode_info) {
            uint8_t *mode = (uint8_t *)(uintptr_t)mb->vbe_mode_info;
            if (mode) {
                uint16_t pitch = *(uint16_t *)(mode + 0x10);
                uint16_t width = *(uint16_t *)(mode + 0x12);
                uint16_t height = *(uint16_t *)(mode + 0x14);
                uint8_t bpp = *(uint8_t *)(mode + 0x19);
                uint32_t physbase = *(uint32_t *)(mode + 0x28);
                if (physbase && width && height && bpp) {
                    fb_init((uint8_t *)(uintptr_t)physbase, width, height, pitch, bpp);
                    /* Report framebuffer params over serial */
                    serial_puts("FB: phys=0x"); serial_puthex(physbase); serial_puts(" w="); serial_puthex(width); serial_puts(" h="); serial_puthex(height); serial_puts(" p="); serial_puthex(pitch); serial_puts(" b="); serial_puthex(bpp); serial_putc('\n');
                } else {
                    serial_puts("FB: invalid VBE\n");
                }
            } else {
                serial_puts("FB: no mode\n");
            }
        } else {
            serial_puts("MB: no vbe_mode_info pointer\n");
        }
    } else {
        serial_puts("MB: no addr\n");
    }

    /* If no usable linear framebuffer was provided (or mapping not available)
       use a conservative fallback address that works in QEMU/GRUB. */
    extern uint8_t* fb_address;
    if (!fb_address) {
        /* Common QEMU/GRUB linear framebuffer address for 800x600x32 */
        fb_init((uint8_t*)0xE0000000, 800, 600, 3200, 32);
        serial_puts("FB: using fallback address 0xE0000000\n");
    }

    /* Print framebuffer pointer for debugging */
    serial_puts("FB_ADDRESS=0x"); serial_puthex((uint32_t)(uintptr_t)fb_address); serial_putc('\n');
    boot_progress(40);

    keyboard_init();
    mouse_init();
    pci_scan();
    fs_init();
    net_init();
    serial_puts("DEVICES OK\n");
    boot_progress(60);

    /* Tiny serial debug message */
    serial_puts("KMAIN\n");

    gui_init();
    serial_puts("GUI OK\n");
    boot_progress(80);

    desktop_show_startup_screen("SafeOS 1.0", "Loading system modules...");
    // simulate loading
    for (int i = 0; i < 10000000; ++i) { __asm__ __volatile__("nop"); }

    desktop_init_home();   // icons: Notepad, Calculator, Spreadsheet, Files, Browser
    serial_puts("DESKTOP OK\n");
    boot_progress(100);

    gui_main_loop();       // event loop
}
