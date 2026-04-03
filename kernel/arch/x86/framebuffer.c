#include "framebuffer.h"
/* Use relative paths to core headers so includes resolve when compiling
    files in arch/x86 directory. */
#include "../../core/memory.h"
#include "../../core/string.h"
#include <stdint.h>

/* Minimal serial helpers for diagnostics (avoid depending on kmain helpers) */
static void fb_serial_putc(char c) {
    __asm__ __volatile__("outb %%al, %%dx" :: "a"(c), "d"((unsigned short)0x3f8));
}
static void fb_serial_puts(const char *s) {
    while (*s) fb_serial_putc(*s++);
}
static void fb_serial_puthex(uint32_t v) {
    const char *hex = "0123456789ABCDEF";
    for (int i = 7; i >= 0; --i) {
        uint8_t nib = (v >> (i*4)) & 0xF;
        fb_serial_putc(hex[nib]);
    }
}

uint8_t* fb_address = 0;
uint32_t fb_width = 0;
uint32_t fb_height = 0;
uint32_t fb_pitch = 0;
uint32_t fb_bpp = 0;

static uint32_t *backbuffer = NULL;
static int backbuffer_init_attempted = 0;

/* Attempt to allocate backbuffer from kmalloced heap.
   Safe to call after memory_init(); returns silently if malloc fails. */
void framebuffer_alloc_backbuffer(void) {
    if (backbuffer_init_attempted) return;  /* Only try once */
    backbuffer_init_attempted = 1;
    
    if (backbuffer) return;  /* Already allocated */
    if (fb_width == 0 || fb_height == 0) return;  /* Invalid framebuffer */
    
    size_t size = (size_t)fb_width * fb_height * 4;
    backbuffer = (uint32_t *)kmalloc(size);
    if (backbuffer) {
        /* Clear backbuffer to black */
        for (uint32_t i = 0; i < fb_width * fb_height; ++i) backbuffer[i] = 0;
        fb_serial_puts("FB: backbuffer allocated @ 0x"); 
        fb_serial_puthex((uint32_t)(uintptr_t)backbuffer); 
        fb_serial_puts(" ("); 
        fb_serial_puthex((uint32_t)size); 
        fb_serial_puts(" bytes)\n");
    } else {
        fb_serial_puts("FB: backbuffer allocation FAILED - rendering to physical framebuffer\n");
    }
}

void fb_init(uint8_t* address, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
    fb_address = address;
    fb_width = width;
    fb_height = height;
    fb_pitch = pitch;
    fb_bpp = bpp;
    /* Do NOT allocate backbuffer here - defer until memory is ready.
       Framebuffer functions will work directly to physical memory if needed. */
    fb_serial_puts("FB: fb_init addr=0x"); 
    fb_serial_puthex((uint32_t)(uintptr_t)address); 
    fb_serial_puts(" w="); fb_serial_puthex(width); 
    fb_serial_puts(" h="); fb_serial_puthex(height); 
    fb_serial_puts(" p="); fb_serial_puthex(pitch); 
    fb_serial_puts(" b="); fb_serial_puthex(bpp); 
    fb_serial_puts(" (backbuffer deferred)\n");
}

void fb_clear(uint32_t color) {
    if (!fb_address) return; /* no framebuffer provided by bootloader */
    
    if (fb_bpp == 32) {
        /* clear backbuffer if present */
        if (backbuffer) {
            uint32_t *b = backbuffer;
            for (uint32_t i = 0; i < fb_width * fb_height; ++i) b[i] = color;
            return;
        }

        uint32_t* fb = (uint32_t*)fb_address;
        for (uint32_t y = 0; y < fb_height; ++y) {
            for (uint32_t x = 0; x < fb_width; ++x) {
                fb[y * (fb_pitch / 4) + x] = color;
            }
        }
    } else if (fb_bpp == 16) {
        uint16_t* fb = (uint16_t*)fb_address;
        uint16_t c = color & 0xFFFF;
        for (uint32_t y = 0; y < fb_height; ++y) {
            for (uint32_t x = 0; x < fb_width; ++x) {
                fb[y * (fb_pitch / 2) + x] = c;
            }
        }
    } else if (fb_bpp == 8) {
        uint8_t* fb = (uint8_t*)fb_address;
        uint8_t c = color & 0xFF;
        for (uint32_t y = 0; y < fb_height; ++y) {
            for (uint32_t x = 0; x < fb_width; ++x) {
                fb[y * fb_pitch + x] = c;
            }
        }
    }
}

void fb_putpixel(int x, int y, uint32_t color) {
    if (!fb_address) return;
    if (x < 0 || y < 0) return;
    if ((uint32_t)x >= fb_width || (uint32_t)y >= fb_height) return;

    if (fb_bpp == 32) {
        if (backbuffer) {
            backbuffer[(uint32_t)y * fb_width + (uint32_t)x] = color;
            return;
        }
        uint32_t* fb = (uint32_t*)fb_address;
        fb[(uint32_t)y * (fb_pitch / 4) + (uint32_t)x] = color;
    } else if (fb_bpp == 16) {
        uint16_t* fb = (uint16_t*)fb_address;
        fb[(uint32_t)y * (fb_pitch / 2) + (uint32_t)x] = color & 0xFFFF;
    } else if (fb_bpp == 8) {
        uint8_t* fb = (uint8_t*)fb_address;
        fb[(uint32_t)y * fb_pitch + (uint32_t)x] = color & 0xFF;
    }
}

void fb_putchar(int x, int y, char c, uint32_t fg, uint32_t bg) {
    if (!fb_address) return;
    extern const uint8_t (*current_font)[8];
    const uint8_t *glyph = current_font[(uint8_t)c];
    for (int gy = 0; gy < 8; gy++) {
        for (int gx = 0; gx < 8; gx++) {
            uint32_t color = (glyph[gy] & (1 << (7 - gx))) ? fg : bg;
            fb_putpixel(x + gx, y + gy, color);
        }
    }
}

void fb_present(void) {
    if (!fb_address || !backbuffer) return;
    /* Copy per-scanline to respect pitch: copy exactly fb_width*bytes_per_pixel */
    uint32_t bytes_per_pixel = 4;
    uint32_t row_bytes = fb_width * bytes_per_pixel;
    for (uint32_t y = 0; y < fb_height; ++y) {
        uint8_t *dst = fb_address + y * fb_pitch;
        uint8_t *src = ((uint8_t *)backbuffer) + (y * row_bytes);
        memcpy(dst, src, row_bytes);
    }
    /* Avoid spamming serial with a message on every present. Log only
       the first present to confirm the framebuffer is being updated. */
    static int printed_once = 0;
    if (!printed_once) {
        fb_serial_puts("FB: presented\n");
        printed_once = 1;
    }
}
