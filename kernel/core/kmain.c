#include "../arch/x86/interrupts.h"
#include "../arch/x86/keyboard.h"
#include "../arch/x86/mouse.h"
#include "../arch/x86/framebuffer.h"
#include <stdint.h>
#include "stdio.h"
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

/* Basic inb helper for reading serial port registers */
static unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Poll whether data is available on COM1 (LSR bit 0) */
static int serial_data_available(void) {
    return (inb(0x3FD) & 0x01);
}

/* Blocking getchar from COM1 (0x3F8) */
static char serial_getc(void) {
    while (!(inb(0x3FD) & 0x01)) {
        /* Sleep a little to be nice on CPU */
        __asm__ __volatile__("hlt");
    }
    return (char)inb(0x3F8);
}

/* Simple helpers to compare/scan strings without pulling in headers */
static int starts_with(const char *s, const char *p) {
    while (*p) {
        if (*s++ != *p++) return 0;
    }
    return 1;
}

static int streq(const char *a, const char *b) {
    while (*a && *b && *a == *b) { a++; b++; }
    return (*a == '\0' && *b == '\0');
}

/* Read a line from serial (echoing) into buf, returns length */
static int serial_readline(char *buf, int maxlen) {
    int pos = 0;
    while (pos < maxlen - 1) {
        char c = serial_getc();
        if (c == '\r') c = '\n';
        if (c == '\n') {
            serial_putc('\n');
            buf[pos] = '\0';
            return pos;
        }
        if (c == '\b' || c == 127) {
            if (pos > 0) { pos--; serial_puts("\b \b"); }
            continue;
        }
        if (c >= 32 && c < 127) {
            buf[pos++] = c;
            serial_putc(c);
        }
    }
    buf[pos] = '\0';
    return pos;
}

/* Minimal interactive serial shell (very small command set) */
static void serial_shell(void) {
    char line[128];
    serial_puts("\nWelcome to SafeOS serial shell\n");
    for (;;) {
        serial_puts("safeos> ");
        int n = serial_readline(line, sizeof(line));
        if (n <= 0) continue;

        if (streq(line, "help")) {
            serial_puts("Commands: help, echo <text>, reboot, exit\n");
        } else if (starts_with(line, "echo ")) {
            serial_puts(line + 5); serial_putc('\n');
        } else if (streq(line, "reboot")) {
            serial_puts("Rebooting...\n");
            /* Attempt keyboard-controller reset */
            __asm__ __volatile__("outb %%al, %%dx" :: "a"((unsigned char)0xFE), "d"((unsigned short)0x64));
            for (;;) __asm__ __volatile__("hlt");
        } else if (streq(line, "exit")) {
            serial_puts("Exiting shell\n");
            break;
        } else {
            serial_puts("Unknown command. Type 'help'\n");
        }
    }
}

/* Boot timing */
static uint32_t boot_start_time = 0;

static void boot_progress(int pct) {
    /* Draw progress bar in bottom-left corner if framebuffer is available. */
    if (fb_address) {
        /* Position in bottom-left corner */
        int x = 20;
        int y = fb_height - 60;  /* 60 pixels from bottom */
        int bar_width = 12;      /* Width of progress bar area */
        int bar_height = 10;     /* Height of each progress segment */
        
        /* Clear previous progress bar area */
        gui_draw_rect(x-5, y-5, bar_width + 20, bar_height + 40, 0x000000);
        
        if (pct >= 100) {
            /* Hide progress bar when done */
            return;
        }
        
        /* Draw border with @ symbols */
        gui_draw_text(x-2, y-2, "@@@@@@@@@@@@", 0xFFFFFF, 0x000000);
        gui_draw_text(x-2, y+bar_height*10, "@@@@@@@@@@@@", 0xFFFFFF, 0x000000);
        
        /* Draw side borders */
        for (int i = 0; i < 10; i++) {
            gui_draw_text(x-2, y + i*bar_height, "@", 0xFFFFFF, 0x000000);
            gui_draw_text(x-2 + 10*8, y + i*bar_height, "@", 0xFFFFFF, 0x000000);
        }
        
        /* Fill progress with % symbols */
        int filled_segments = (pct * 10) / 100;
        for (int i = 0; i < 10; i++) {
            uint32_t bg_color = (i < filled_segments) ? 0x00AA00 : 0x333333;
            gui_draw_rect(x, y + i*bar_height, bar_width, bar_height, bg_color);
            
            /* Draw % symbol in each segment */
            char percent = '%';
            gui_draw_text(x + 2, y + i*bar_height + 1, &percent, 0xFFFFFF, bg_color);
        }
        
        /* Draw percentage and timing text below */
        char buf[64];
        int n = snprintf(buf, sizeof(buf), "%3d%% Booting", pct);
        (void)n;
        gui_draw_text(x-2, y + bar_height*10 + 8, buf, 0xFFFFFF, 0x000000);
    }
}

/* LFB raw test removed — use forced framebuffer address instead. */

/* Quick physical-LFB diagnostic: write a small 64x64 checker directly
   to the physical framebuffer address (bypasses any backbuffer) so
   we can verify the emulator displays the physical region. */
static void phys_write_test(uint32_t phys) {
    if (!phys) {
        serial_puts("PHYS_TEST: no phys\n");
        return;
    }
    serial_puts("PHYS_TEST: writing test to 0x"); serial_puthex(phys); serial_putc('\n');
    volatile uint32_t *p = (volatile uint32_t *)(uintptr_t)phys;
    uint32_t stride = fb_pitch ? (fb_pitch / 4) : fb_width;
    for (uint32_t y = 0; y < 64 && y < fb_height; ++y) {
        for (uint32_t x = 0; x < 64 && x < fb_width; ++x) {
            uint32_t color = (((x/8) ^ (y/8)) & 1) ? 0x00FF00FF : 0x0000FF00;
            p[y * stride + x] = color;
        }
    }
    serial_puts("PHYS_TEST: done\n");
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

     /* Quick LFB diagnostics removed; we force known-good framebuffer below. */

     /* Do not force a framebuffer yet — prefer bootloader-provided VBE
         information when available. We will fall back later to a safe
         identity-mapped framebuffer if needed. */

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
                
                serial_puts("FB: VBE raw - p=0x"); serial_puthex(pitch); serial_puts(" w=0x"); serial_puthex(width); 
                serial_puts(" h=0x"); serial_puthex(height); serial_puts(" b=0x"); serial_puthex(bpp); 
                serial_puts(" phys=0x"); serial_puthex(physbase); serial_putc('\n');
                
                /* Validate VBE values conservatively before trusting them.
                   Require reasonable resolution and supported bpp, and ensure
                   pitch is at least width*(bpp/8). */
                int bytes_per_pixel = (bpp + 7) / 8;
                uint32_t min_pitch = (uint32_t)width * (uint32_t)bytes_per_pixel;
                if (physbase && width >= 320 && height >= 200 && (bpp == 8 || bpp == 16 || bpp == 24 || bpp == 32) && pitch >= min_pitch) {
                    fb_init((uint8_t *)(uintptr_t)physbase, width, height, pitch, bpp);
                    serial_puts("FB: using VBE phys=0x"); serial_puthex(physbase); serial_puts(" w="); serial_puthex(width); serial_puts(" h="); serial_puthex(height); serial_puts(" p="); serial_puthex(pitch); serial_puts(" b="); serial_puthex(bpp); serial_putc('\n');
                } else {
                    /* Bootloader is in text mode or no valid VBE info - use a safe identity-mapped framebuffer
                       Use a lower memory region that is known to be writable and visible in QEMU */
                    uint32_t graphics_lfb = 0x00300000;
                    uint16_t gfx_w = 1024;
                    uint16_t gfx_h = 768;
                    uint16_t gfx_p = gfx_w * 4;  /* 32-bit pixels */
                    uint8_t gfx_b = 32;
                    fb_init((uint8_t *)(uintptr_t)graphics_lfb, gfx_w, gfx_h, gfx_p, gfx_b);
                    serial_puts("FB: using QEMU graphics LFB at 0x"); serial_puthex(graphics_lfb);
                    serial_puts(" (1024x768x32)\n");
                }
            } else {
                serial_puts("FB: no mode ptr\n");
            }
        } else {
            serial_puts("MB: no vbe_mode_info pointer\n");
        }
    } else {
        serial_puts("MB: no addr\n");
    }

    /* If no usable linear framebuffer was provided (or mapping not available)
       use graphics framebuffer at common QEMU/VGA locations */
    extern uint8_t* fb_address;
    if (!fb_address) {
        /* Fall back to an identity-mapped framebuffer in low memory that QEMU shows reliably */
        uint32_t graphics_fb = 0x00300000;
        fb_init((uint8_t *)(uintptr_t)graphics_fb, 1024, 768, 1024 * 4, 32);
        serial_puts("FB: using fallback framebuffer at 0x"); serial_puthex(graphics_fb);
        serial_puts(" (1024x768x32)\n");
    }

    /* Print framebuffer pointer for debugging */
    serial_puts("FB_ADDRESS=0x"); serial_puthex((uint32_t)(uintptr_t)fb_address); serial_putc('\n');
    boot_progress(40);

    phys_write_test((uint32_t)(uintptr_t)fb_address);

    keyboard_init();
    mouse_init();
    // pci_scan(); // Temporarily disabled - causes page fault on unmapped MMIO
    fs_init();
    net_init();
    serial_puts("DEVICES OK\n");
    boot_progress(60);

    /* Tiny serial debug message */
    serial_puts("KMAIN\n");

    if (gui_init() == 0) {
        serial_puts("GUI OK\n");
        boot_progress(80);

        /* Skip startup screen - go straight to desktop */
        desktop_init_home();
        serial_puts("DESKTOP OK\n");
        boot_progress(100);  /* Hide progress bar */
        
        /* Clear screen to dark gray background */
        framebuffer_clear(0x202020);

        gui_main_loop();       // event loop - displays desktop with icons
    } else {
        /* GUI init failed — stay alive and provide serial diagnostics */
        serial_puts("GUI FAILED - running headless diagnostics\n");
        boot_progress(100);
        /* Run interactive serial shell; if it exits, fall back to heartbeat */
        serial_shell();

        /* Idle loop: show periodic heartbeat so the VM is responsive */
        while (1) {
            serial_puts("safeos: idle\n");
            for (volatile int i = 0; i < 1000000; ++i) { __asm__ __volatile__("nop"); }
            __asm__ __volatile__("hlt");
        }
    }
}
