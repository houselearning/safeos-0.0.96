#include "../arch/x86/interrupts.h"
#include "../arch/x86/keyboard.h"
#include "../arch/x86/mouse.h"
#include "../arch/x86/framebuffer.h"
#include <stdint.h>
#include "memory.h"
#include "paging.h"
#include "../gui/gui.h"
#include "../gui/desktop.h"
#include "text_desktop.h"

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

void kmain(unsigned long magic, unsigned long addr) {
    (void)magic;
    (void)addr;

    /* Very early serial tick so headless consoles know the kernel started. */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('!'), "d"((unsigned short)0x3f8));

    /* (serial helpers are file-scope) */

    interrupts_init();
    memory_init();
     /* Enable an identity 4MB-page mapping so the kernel can access
         physical regions (e.g. framebuffer physbase) directly. */
     paging_enable_identity_4mb();
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
        if (mb->flags & (1 << 12)) {
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
                    framebuffer_init(1024, 768, 32);
                    serial_puts("FB: fallback init\n");
                }
            } else {
                framebuffer_init(1024, 768, 32);
            }
        } else {
            framebuffer_init(1024, 768, 32);
        }
    } else {
        framebuffer_init(1024, 768, 32);
    }
    /* If no usable linear framebuffer was provided (or mapping not available)
       use the simple text-mode desktop fallback which writes to 0xB8000. */
    extern uint8_t* fb_address;
    if (!fb_address) {
        text_desktop_run();
        /* text_desktop_run does not return */
    }

    keyboard_init();
    mouse_init();

    /* Tiny serial debug message */
    serial_puts("KMAIN\n");

    gui_init();
    desktop_show_startup_screen("SafeOS 1.0", "Loading system modules...");
    // simulate loading
    for (int i = 0; i < 10000000; ++i) { __asm__ __volatile__("nop"); }

    desktop_init_home();   // icons: Notepad, Calculator, Spreadsheet, Files, Browser
    gui_main_loop();       // event loop
}
