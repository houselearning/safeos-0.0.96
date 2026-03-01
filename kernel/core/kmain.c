#include "arch/x86/interrupts.h"
#include "arch/x86/keyboard.h"
#include "arch/x86/mouse.h"
#include "arch/x86/framebuffer.h"
#include <stdint.h>
#include "core/memory.h"
#include "gui/gui.h"
#include "gui/desktop.h"

void kmain(unsigned long magic, unsigned long addr) {
    (void)magic;
    (void)addr;

    interrupts_init();
    memory_init();
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
                } else {
                    framebuffer_init(1024, 768, 32);
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
    keyboard_init();
    mouse_init();

    /* Tiny serial debug: write a message to COM1 (0x3F8) so headless VMs
       or serial consoles can confirm kernel progress. */
    auto serial_putc = (void (*)(char))0;
    {
        static void _serial_putc(char c) {
            unsigned short port = 0x3f8;
            __asm__ __volatile__("outb %0, %1" : : "a"(c), "d"(port));
        }
        serial_putc = _serial_putc;
    }
    serial_putc('K'); serial_putc('M'); serial_putc('A'); serial_putc('I'); serial_putc('N'); serial_putc('\n');

    gui_init();
    desktop_show_startup_screen("SafeOS 1.0", "Loading system modules...");
    // simulate loading
    for (int i = 0; i < 10000000; ++i) { __asm__ __volatile__("nop"); }

    desktop_init_home();   // icons: Notepad, Calculator, Spreadsheet, Files, Browser
    gui_main_loop();       // event loop
}
