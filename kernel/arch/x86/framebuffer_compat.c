#include "framebuffer.h"
#include "gpu.h"
#include "vga.h"
#include "../../core/text_desktop.h"
#include <stddef.h>
#include <stdint.h>

static void fb_serial_putc(char c) {
    __asm__ __volatile__("outb %%al, %%dx" :: "a"(c), "d"((unsigned short)0x3f8));
}

static void fb_serial_puthex(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i) {
        uint8_t nib = (v >> (i*4)) & 0xF;
        fb_serial_putc(hex[nib]);
    }
}

static void fb_serial_puts(const char *s) {
    while (*s) fb_serial_putc(*s++);
}

/*
 * RAM-based fallback framebuffer for SafeOS GUI rendering
 * when no hardware framebuffer is available from bootloader/GPU.
 */

#define FALLBACK_WIDTH 1024
#define FALLBACK_HEIGHT 768

static uint32_t fallback_fb[FALLBACK_WIDTH * FALLBACK_HEIGHT];

void framebuffer_init(int width, int height, int bpp) {
    (void)width; (void)height; (void)bpp;
    extern uint8_t* fb_address;
    extern uint32_t fb_bpp;
    
    /* If we already have a framebuffer from bootloader/gpu_init, keep it */
    if (fb_address && fb_bpp >= 8) {
        fb_serial_puts("FB: using existing framebuffer\n");
        return;
    }

    /* No hardware framebuffer available, use RAM fallback for GUI rendering */
    fb_serial_puts("FB: initializing RAM-based 1024x768x32 fallback\n");
    fb_init((uint8_t *)fallback_fb, FALLBACK_WIDTH, FALLBACK_HEIGHT,
            FALLBACK_WIDTH * 4, 32);
}

void framebuffer_clear(uint32_t color) {
    fb_clear(color);
}
