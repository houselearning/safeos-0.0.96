#include "../arch/x86/interrupts.h"
#include "../arch/x86/keyboard.h"
#include "../arch/x86/mouse.h"
#include "../arch/x86/framebuffer.h"
#include "../arch/x86/gpu.h"
#include "../arch/x86/vga.h"
#include <stdint.h>
#include "stdio.h"
#include "memory.h"
#include "memory_layout.h"
#include "paging.h"
#include "pci.h"
#include "krow_diagnostics.h"
#include "../gui/gui.h"
#include "../gui/desktop.h"
#include "text_desktop.h"
#include "fs.h"
#include "net.h"

static volatile uint16_t *vga_text = (volatile uint16_t *)0xB8000;

static void vga_text_puts(int row, int col, const char *s) {
    uint16_t *dest = &vga_text[row * 80 + col];
    while (*s && col < 80) {
        char c = *s++;
        if (c == '\n') {
            row++;
            col = 0;
            dest = &vga_text[row * 80 + col];
            continue;
        }
        if (c < 32) c = '?';
        *dest++ = (uint16_t)(0x0700 | (uint8_t)c);
        col++;
    }
}

static void vga_text_clear(void) {
    for (int i = 0; i < 80*25; ++i) {
        vga_text[i] = 0x0700 | ' ';
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
            const char percent[2] = {'%', '\0'};
            gui_draw_text(x + 2, y + i*bar_height + 1, percent, 0xFFFFFF, bg_color);
        }
        
        /* Draw percentage and timing text below */
        char buf[64];
        int n = snprintf(buf, sizeof(buf), "%3d%% Booting", pct);
        (void)n;
        gui_draw_text(x-2, y + bar_height*10 + 8, buf, 0xFFFFFF, 0x000000);
    }
}

/* LFB raw test removed — use forced framebuffer address instead. */

/* Quick physical-LFB diagnostic: write a solid red 64x64 square directly
   to the physical framebuffer address (bypasses any backbuffer) so
   we can verify the emulator displays the physical region. */
static void phys_write_test(uint32_t phys) {
    if (!phys) {
        serial_puts("PHYS_TEST: no phys\n");
        return;
    }
    serial_puts("PHYS_TEST: writing red square to 0x"); serial_puthex(phys); serial_putc('\n');
    volatile uint32_t *p = (volatile uint32_t *)(uintptr_t)phys;
    uint32_t stride = fb_pitch ? (fb_pitch / 4) : fb_width;
    for (uint32_t y = 0; y < 64 && y < fb_height; ++y) {
        for (uint32_t x = 0; x < 64 && x < fb_width; ++x) {
            p[y * stride + x] = 0x00FF0000;  /* Solid red */
        }
    }
    serial_puts("PHYS_TEST: done\n");
}

void kmain(unsigned long magic, unsigned long addr) {
    (void)magic;
    (void)addr;

    /* ===== CHECKPOINT 0: Entry point - MUST output immediately ===== */
    /* Write to VGA text memory at 0xB8000 */
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    
    /* Clear first 80 characters to avoid BIOS cruft */
    for (int i = 0; i < 80; i++) {
        vga[i] = 0x0F20;  /* Space with white on black */
    }

    /* Guarantee visible output: early text-mode banner */
    vga_text_clear();
    vga_text_puts(0, 0, "SafeOS Boot: Early text fallback active\n");
    vga_text_puts(1, 0, "If this appears, basic output works.\n");

    /* Attempt early VGA mode 13h setup for fallback graphics. This provides
       a basic framebuffer if the bootloader didn't supply one. Modern boot
       environments usually prefer a better framebuffer, so this is a fallback. */
    if (vga_set_mode_13h() == 0) {
        fb_init((uint8_t *)0xA0000, 320, 200, 320, 8);
        serial_puts("EARLY: VGA mode 13h configured as fallback\n");
    } else {
        serial_puts("EARLY: VGA mode 13h not available (bootloader framebuffer preferred)\n");
    }

    /* Continue boot process; later code will upgrade graphics if possible. */

    /* "K" at position 0 */
    vga[0] = 0x0F4B;   /* White 'K' on black background */
    
    /* "M" at position 1 */
    vga[1] = 0x0F4D;   /* White 'M' */
    
    /* "A" at position 2 */
    vga[2] = 0x0F41;   /* White 'A' */
    
    /* Serial output */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('K'), "d"((unsigned short)0x3f8));
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('M'), "d"((unsigned short)0x3f8));
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('A'), "d"((unsigned short)0x3f8));

    /* ===== CHECKPOINT 1: Before interrupts_init ===== */
    vga[3] = 0x0F31;   /* '1' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('1'), "d"((unsigned short)0x3f8));

    /* (serial helpers are file-scope) */

    interrupts_init();
    serial_puts("INTERRUPTS OK\n");
    boot_progress(10);

    /* ===== CHECKPOINT 2: Before memory_init ===== */
    vga[4] = 0x0F32;   /* '2' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('2'), "d"((unsigned short)0x3f8));

    memory_init();
    serial_puts("MEMORY OK\n");
    boot_progress(20);

    /* ===== CHECKPOINT 3: Before paging ===== */
    vga[5] = 0x0F33;   /* '3' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('3'), "d"((unsigned short)0x3f8));

    /* Enable an identity 4MB-page mapping so the kernel can access
       physical regions (e.g. framebuffer physbase) directly. */
    paging_enable_identity_4mb();
    serial_puts("PAGING OK\n");
    boot_progress(30);
    
    /* ===== CHECKPOINT 4: Before multiboot parsing ===== */
    vga[6] = 0x0F34;   /* '4' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('4'), "d"((unsigned short)0x3f8));

     /* Quick LFB diagnostics removed; we force known-good framebuffer below. */

     /* Do not force a framebuffer yet — prefer bootloader-provided VBE
         information when available. We will fall back later to a safe
         identity-mapped framebuffer if needed. */

     /* If the bootloader provided multiboot info, try to extract VBE mode
         information (linear framebuffer address, resolution, pitch, bpp)
         and initialize the framebuffer with the real values. Otherwise
         fall back to conservative defaults. */
    /* Headless mode flag for non-GUI operation */
    int headless_mode = 0;

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
        uint64_t fb_addr;
        uint32_t fb_pitch;
        uint32_t fb_width;
        uint32_t fb_height;
        uint8_t fb_bpp;
        uint8_t fb_type;
        uint16_t fb_reserved;
        uint8_t fb_red_mask_shift;
        uint8_t fb_red_mask_size;
        uint8_t fb_green_mask_shift;
        uint8_t fb_green_mask_size;
        uint8_t fb_blue_mask_shift;
        uint8_t fb_blue_mask_size;
    } multiboot_info_t;

    /* ===== CHECKPOINT 4: Before multiboot parsing ===== */
    vga[6] = 0x0F34;   /* '4' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('4'), "d"((unsigned short)0x3f8));

    if (addr) {
        multiboot_info_t *mb = (multiboot_info_t *)(uintptr_t)addr;
        serial_puts("MB: addr=0x"); serial_puthex((uint32_t)addr); serial_puts(" flags=0x"); serial_puthex(mb->flags); serial_puts(" vbe_mode_info=0x"); serial_puthex(mb->vbe_mode_info); serial_puts(" vbe_mode=0x"); serial_puthex(mb->vbe_mode); serial_putc('\n');

        /* Prefer multiboot framebuffer info (if present, bit 12 in flags). */
        if (mb->flags & (1u << 12)) {
            serial_puts("MB: framebuffer info provided by bootloader (likely UEFI/BIOS)\n");
            uint64_t fb_phys = mb->fb_addr;
            uint32_t fb_w = mb->fb_width;
            uint32_t fb_h = mb->fb_height;
            uint32_t fb_p = mb->fb_pitch;
            uint32_t fb_b = mb->fb_bpp;
            serial_puts("FB: multiboot framebuffer info - addr=0x"); serial_puthex((uint32_t)(uintptr_t)fb_phys);
            serial_puts(" w="); serial_puthex(fb_w); serial_puts(" h="); serial_puthex(fb_h);
            serial_puts(" p="); serial_puthex(fb_p); serial_puts(" b="); serial_puthex(fb_b); serial_putc('\n');
            if (fb_phys && fb_w >= 320 && fb_h >= 200 && fb_p >= fb_w * ((fb_b + 7)/8) && (fb_b == 32 || fb_b == 24 || fb_b == 16 || fb_b == 8)) {
                fb_init((uint8_t *)(uintptr_t)fb_phys, fb_w, fb_h, fb_p, fb_b);
                serial_puts("FB: using multiboot framebuffer info\n");
            } else {
                serial_puts("FB: multiboot framebuffer unsupported format, forcing fallback\n");
                fb_address = NULL; fb_width = fb_height = fb_pitch = fb_bpp = 0;
            }
        }

        /* If we still have no framebuffer, try legacy VBE mode info (bit 11). */
        if (!fb_address && (mb->flags & (1u << 11)) && mb->vbe_mode_info) {
            serial_puts("MB: VBE mode info present (legacy BIOS)\n");
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
                if (physbase && width >= 320 && height >= 200 && (bpp == 32 || bpp == 24 || bpp == 16 || bpp == 8) && pitch >= (uint32_t)width * ((bpp + 7) / 8)) {
                    fb_init((uint8_t *)(uintptr_t)physbase, width, height, pitch, bpp);
                    serial_puts("FB: using VBE phys\n");
                } else {
                    serial_puts("FB: invalid/unsupported VBE mode\n");
                    fb_address = NULL; fb_width = fb_height = fb_pitch = fb_bpp = 0;
                }
            }
        }

        if (!fb_address) {
            serial_puts("MB: no framebuffer info, probing fallback addresses\n");
        }

        /* Parse kernel command line for headless mode flags */
        if (mb->cmdline) {
            const char *cmd = (const char *)(uintptr_t)mb->cmdline;
            serial_puts("CMDLINE: ");
            /* Print command line for diagnostics */
            for (int i = 0; i < 256 && cmd[i]; ++i) serial_putc(cmd[i]);
            serial_putc('\n');
            /* Check for headless mode flags */
            int found = 0;
            const char *needle_list[] = {"debug=1", "nogui", "nomodeset", NULL};
            for (const char **n = needle_list; *n; ++n) {
                const char *needle = *n;
                const char *h = cmd;
                while (*h) {
                    const char *h2 = h;
                    const char *p = needle;
                    while (*h2 && *p && *h2 == *p) { h2++; p++; }
                    if (*p == '\0') { found = 1; break; }
                    h++;
                }
                if (found) break;
            }
            if (found) {
                headless_mode = 1;
                serial_puts("CMDLINE: headless mode flag detected -> running non-interactive diagnostics\n");
            }
        }
    } else {
        serial_puts("MB: no addr\n");
    }

    /* ===== CHECKPOINT 5: Before framebuffer probe ===== */
    {
        volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
        vga[1] = 0x2D35;   /* '5-' progress marker */
    }

    /* If no usable linear framebuffer was provided by the bootloader,
       try PCI GPU mapping first. If all hardware detection fails,
       fall back to a RAM-based 1024x768x32 virtual framebuffer for GUI
       rendering (output will not be visible unless connected to serial/headless,
       but GUI will render coherently). */
    extern uint8_t* fb_address;
    
    /* ===== CHECKPOINT 5: Before framebuffer init ===== */
    vga[7] = 0x0F35;   /* '5' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('5'), "d"((unsigned short)0x3f8));
    
    if (!fb_address) {
        serial_puts("FB: no bootloader framebuffer, attempting GPU PCI mapping\n");
        if (gpu_init() == 0 && fb_address) {
            serial_puts("FB: gpu_init() mapped framebuffer successfully\n");
        } else {
            serial_puts("FB: no hardware framebuffer available, using RAM-based 1024x768x32 buffer\n");
            /* Allocate RAM-based framebuffer after memory is initialized (done later) */
            fb_address = NULL;
        }
    }

    /* ===== CHECKPOINT 6: Before keyboard/mouse init ===== */
    vga[8] = 0x0F36;   /* '6' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('6'), "d"((unsigned short)0x3f8));

    keyboard_init();
    mouse_init();
    /* ===== CHECKPOINT 7: Before PCI/devices init ===== */
    vga[9] = 0x0F37;   /* '7' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('7'), "d"((unsigned short)0x3f8));
    serial_puts("BOOT: Reached checkpoint 7\n");
    
    /* SKIP PCI/FS/NET in graphics mode - they timeout in VMs */
    if (!headless_mode) {
        serial_puts("BOOT: Graphics mode - skipping PCI/FS/NET scan\n");
        /* Jump directly to GUI initialization */
    } else {
        /* In headless mode, try to scan hardware */
        serial_puts("BOOT: Headless mode - scanning PCI devices...\n");
        pci_scan();
        serial_puts("PCI OK\n");
        fs_init();
        net_init();
        serial_puts("DEVICES OK\n");
    }
    
    boot_progress(60);

    /* ===== CHECKPOINT 8: Before krow diagnostics ===== */
    vga[10] = 0x0F38;  /* '8' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('8'), "d"((unsigned short)0x3f8));

    /* Run Krow Diagnostics - interactive BootRepair menu */
    if (!headless_mode) {
        serial_puts("[*] SKIPPING Krow diagnostics in GUI quick-start mode\n");
    } else {
        serial_puts("[*] Running Krow Diagnostics...\n");
        int krow_faults = krow_diagnostics_run(headless_mode);
        if (krow_faults) {
            serial_puts("[*] Krow Diagnostics reported faults.\n");
        } else {
            serial_puts("[*] Krow Diagnostics passed.\n");
        }
    }

    /* Kernel main initialization complete */
    serial_puts("KMAIN\n");

    /* Mode 13 rendering handled by desktop_draw in main loop */
    serial_puts("KMAIN: Graphics mode initialized, entering main loop\n");

    /* ===== CHECKPOINT 9: Before framebuffer backbuffer alloc ===== */
    vga[11] = 0x0F39;  /* '9' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('9'), "d"((unsigned short)0x3f8));

    /* Now that memory is fully initialized, allocate framebuffer backbuffer for GUI rendering */
    extern void framebuffer_alloc_backbuffer(void);
    
    /* If still no framebuffer after all hardware attempts, initialize a RAM-based one */
    if (!fb_address) {
        serial_puts("FB: initializing RAM-based 1024x768x32 virtual framebuffer\n");
        framebuffer_init(1024, 768, 32);
        if (!fb_address) {
            serial_puts("FB: FATAL - could not allocate any framebuffer\n");
            text_desktop_run();
        }
    }
    
    framebuffer_alloc_backbuffer();
    serial_puts("FB: backbuffer allocated\n");

    /* ===== CHECKPOINT 10: Before GUI init ===== */
    vga[12] = 0x0F30;  /* '0' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('0'), "d"((unsigned short)0x3f8));

    /* ===== CHECKPOINT 10: Before GUI init ===== */
    vga[12] = 0x0F30;  /* '0' */
    __asm__ __volatile__("outb %%al, %%dx" :: "a"('0'), "d"((unsigned short)0x3f8));

    if (!headless_mode && gui_init() == 0) {
        serial_puts("GUI: gui_init() returned success\n");
        boot_progress(80);

        /* Skip startup screen - go straight to desktop */
        desktop_init_home();
        serial_puts("GUI: desktop initialized\n");
        boot_progress(100);  /* Hide progress bar */
        
        /* Test: write a simple pattern directly to likely framebuffer address */
        extern uint8_t *fb_address;
        serial_puts("TEST: fb_address = 0x");
        serial_puthex((uint32_t)fb_address);
        serial_puts("\n");
        
        if (fb_address && fb_address != (uint8_t *)0 && fb_width && fb_height) {
            serial_puts("TEST: Rendering debug overlay on framebuffer...\n");
            framebuffer_clear(0x101030);
            gui_draw_text(16, 16, "SafeOS framebuffer active", 0xFFFFFF, 0x101030);
            gui_draw_rect(10, 40, fb_width - 20, fb_height - 80, 0x303050);
            gui_draw_text(14, 44, "VGA/HDMI mode raw output test", 0xFFFFFF, 0x303050);

            if (fb_bpp == 32) {
                for (uint32_t y = 0; y < fb_height && y < 80; ++y) {
                    uint32_t *row = (uint32_t *)(fb_address + y * fb_pitch);
                    for (uint32_t x = 0; x < fb_width && x < 160; ++x) {
                        if ((x + y) % 2 == 0) row[x] = 0xAA0000;
                        else row[x] = 0x00AA00;
                    }
                }
            } else {
                /* Avoid direct raw writes for non-32-bit modes to prevent artifacts */
            }
            fb_present();
            serial_puts("TEST: Debug overlay rendered\n");
        }
        
        /* ===== CHECKPOINT 11: Entering GUI main loop ===== */
        vga[13] = 0x0F58;  /* 'X' */
        serial_puts("GUI: entering main loop\n");
        __asm__ __volatile__("outb %%al, %%dx" :: "a"('X'), "d"((unsigned short)0x3f8));
        gui_main_loop();       // event loop - displays desktop with icons
    } else {
        /* GUI init failed — fallback to text desktop if possible */
        serial_puts("GUI FAILED - using text desktop fallback\n");
        text_desktop_run();

        /* Should never reach here, but keep safe heartbeat as fallback */
        while (1) {
            serial_puts("safeos: idle\n");
            for (volatile int i = 0; i < 1000000; ++i) { __asm__ __volatile__("nop"); }
            __asm__ __volatile__("hlt");
        }
    }
}
