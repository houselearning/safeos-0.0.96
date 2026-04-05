#include "framebuffer.h"
/* Use relative paths to core headers so includes resolve when compiling
    files in arch/x86 directory. */
#include "../../core/memory.h"
#include "../../core/string.h"
#include <stdint.h>

/* Convert 24-bit RGB color to an approximate 8-bit VGA palette index
   using 6x6x6 color cube plus grayscale ramp starting at index 232. */
static uint8_t rgb_to_vga8(uint32_t color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    /* Map to 6x6x6 cube indices */
    uint8_t ri = (r * 6) / 256;
    uint8_t gi = (g * 6) / 256;
    uint8_t bi = (b * 6) / 256;

    return (uint8_t)(16 + ri * 36 + gi * 6 + bi);
}

static uint16_t rgb_to_565(uint32_t color) {
    uint16_t r = (uint16_t)((color >> 19) & 0x1F);
    uint16_t g = (uint16_t)((color >> 10) & 0x3F);
    uint16_t b = (uint16_t)((color >> 3) & 0x1F);
    return (uint16_t)((r << 11) | (g << 5) | b);
}

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

uint32_t *backbuffer = NULL;
static int backbuffer_init_attempted = 0;
static int is_text_mode = 0;
static volatile uint16_t *vga_text = (uint16_t *)0xB8000;
static uint32_t fallback_backbuffer[1024 * 768];

/* Attempt to allocate backbuffer from kmalloced heap.
   Safe to call after memory_init(); if heap allocation fails, use a
   preallocated static backbuffer when the display mode is no larger
   than 1024x768. Otherwise render directly to the physical framebuffer. */
void framebuffer_alloc_backbuffer(void) {
    if (backbuffer_init_attempted) return;  /* Only try once */
    backbuffer_init_attempted = 1;

    if (backbuffer) return;  /* Already allocated */
    if (fb_width == 0 || fb_height == 0) {
        fb_serial_puts("FB: backbuffer skipped - framebuffer dimensions not set\n");
        return;
    }

    size_t size = (size_t)fb_width * fb_height * 4;
    
    /* First try heap allocation */
    backbuffer = (uint32_t *)kmalloc(size);
    if (backbuffer) {
        fb_serial_puts("FB: heap backbuffer allocated @ 0x"); 
        fb_serial_puthex((uint32_t)(uintptr_t)backbuffer); 
        fb_serial_puts("\n");
        return;
    }
    
    /* If heap alloc fails, use static fallback for exact 1024x768 */
    if (fb_width == 1024 && fb_height == 768) {
        backbuffer = fallback_backbuffer;
        fb_serial_puts("FB: using static fallback backbuffer (1024x768)\n");
        return;
    }

    /* Mode is too large for static fallback and heap failed */
    fb_serial_puts("FB: backbuffer allocation failed - rendering directly to framebuffer\n");
    backbuffer = NULL;
}

void fb_init(uint8_t* address, uint32_t width, uint32_t height, uint32_t pitch, uint32_t bpp) {
    if (address == 0) {
        /* Text mode fallback */
        is_text_mode = 1;
        fb_address = (uint8_t*)0xB8000; /* Dummy address */
        fb_width = 640; /* 80 cols * 8 */
        fb_height = 400; /* 25 rows * 16 */
        fb_pitch = 640 * 4; /* Dummy */
        fb_bpp = 32;
        fb_serial_puts("FB: text mode initialized\n");
        return;
    }
    
    is_text_mode = 0;
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
    if (fb_width == 0 || fb_height == 0) return; /* dimensions not set */

    if (is_text_mode) {
        /* Clear VGA text screen */
        for (int i = 0; i < 80*25; ++i) {
            vga_text[i] = (uint16_t)((0x07 << 8) | ' '); /* White on black space */
        }
        return;
    }

    if (backbuffer) {
        /* Only clear if backbuffer is allocated and not too large */
        uint32_t pixel_count = fb_width * fb_height;
        if (pixel_count > 0 && pixel_count <= (16 * 1024 * 1024)) { /* Limit to 16M pixels */
            for (uint32_t i = 0; i < pixel_count; ++i) 
                backbuffer[i] = color;
        }
        return;
    }

    /* Clearing physical framebuffer is risky, so limit to small operations */
    if (fb_width > 2048 || fb_height > 2048) {
        fb_serial_puts("FB: clear skipped - framebuffer too large\n");
        return;
    }

    if (fb_bpp == 32) {
        uint32_t* fb = (uint32_t*)fb_address;
        uint32_t stride = fb_pitch / 4;  /* Convert byte pitch to 32-bit stride */
        for (uint32_t y = 0; y < fb_height; ++y) {
            for (uint32_t x = 0; x < fb_width; ++x) {
                fb[y * stride + x] = color;
            }
        }
    } else if (fb_bpp == 24) {
        uint8_t* fb = (uint8_t*)fb_address;
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        for (uint32_t y = 0; y < fb_height; ++y) {
            for (uint32_t x = 0; x < fb_width; ++x) {
                uint8_t* p = fb + y * fb_pitch + x * 3;
                p[0] = b;
                p[1] = g;
                p[2] = r;
            }
        }
    } else if (fb_bpp == 16) {
        uint16_t* fb = (uint16_t*)fb_address;
        uint16_t c = rgb_to_565(color);
        for (uint32_t y = 0; y < fb_height; ++y) {
            for (uint32_t x = 0; x < fb_width; ++x) {
                fb[y * (fb_pitch / 2) + x] = c;
            }
        }
    } else if (fb_bpp == 8) {
        uint8_t* fb = (uint8_t*)fb_address;
        uint8_t c = rgb_to_vga8(color);
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

    if (backbuffer && fb_bpp == 32) {
        /* Backbuffer is always 32-bit width-stride */
        backbuffer[(uint32_t)y * fb_width + (uint32_t)x] = color;
        return;
    }

    if (fb_bpp == 32) {
        uint32_t* fb = (uint32_t*)fb_address;
        uint32_t stride = fb_pitch / 4;
        fb[(uint32_t)y * stride + (uint32_t)x] = color;
    } else if (fb_bpp == 24) {
        uint8_t* fb = (uint8_t*)fb_address;
        uint32_t offset = (uint32_t)y * fb_pitch + (uint32_t)x * 3;
        fb[offset + 0] = (color) & 0xFF;
        fb[offset + 1] = (color >> 8) & 0xFF;
        fb[offset + 2] = (color >> 16) & 0xFF;
    } else if (fb_bpp == 16) {
        uint16_t* fb = (uint16_t*)fb_address;
        fb[(uint32_t)y * (fb_pitch / 2) + (uint32_t)x] = rgb_to_565(color);
    } else if (fb_bpp == 8) {
        uint8_t* fb = (uint8_t*)fb_address;
        fb[(uint32_t)y * fb_pitch + (uint32_t)x] = rgb_to_vga8(color);
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

    /* If framebuffer and backbuffer are the same memory region, no copy is needed. */
    if ((uint8_t *)backbuffer == fb_address) return;

    uint32_t row_pixels = fb_width;

    if (fb_bpp == 32) {
        uint32_t bytes_per_pixel = 4;
        uint32_t row_bytes = row_pixels * bytes_per_pixel;
        for (uint32_t y = 0; y < fb_height; ++y) {
            uint8_t *dst = fb_address + y * fb_pitch;
            uint8_t *src = ((uint8_t *)backbuffer) + (y * row_bytes);
            memcpy(dst, src, row_bytes);
        }
    } else if (fb_bpp == 24) {
        for (uint32_t y = 0; y < fb_height; ++y) {
            uint8_t *dst = fb_address + y * fb_pitch;
            uint32_t *src = backbuffer + (y * row_pixels);
            for (uint32_t x = 0; x < row_pixels; ++x) {
                uint32_t c = src[x];
                dst[x*3 + 0] = c & 0xFF;
                dst[x*3 + 1] = (c >> 8) & 0xFF;
                dst[x*3 + 2] = (c >> 16) & 0xFF;
            }
        }
    } else if (fb_bpp == 16) {
        for (uint32_t y = 0; y < fb_height; ++y) {
            uint16_t *dst = (uint16_t *)(fb_address + y * fb_pitch);
            uint32_t *src = backbuffer + (y * row_pixels);
            for (uint32_t x = 0; x < row_pixels; ++x) {
                uint32_t c = src[x];
                uint16_t r = (uint16_t)((c >> 19) & 0x1F);
                uint16_t g = (uint16_t)((c >> 10) & 0x3F);
                uint16_t b = (uint16_t)((c >> 3) & 0x1F);
                dst[x] = (uint16_t)((r << 11) | (g << 5) | b);
            }
        }
    } else if (fb_bpp == 8) {
        for (uint32_t y = 0; y < fb_height; ++y) {
            uint8_t *dst = fb_address + y * fb_pitch;
            uint32_t *src = backbuffer + (y * row_pixels);
            for (uint32_t x = 0; x < row_pixels; ++x) {
                dst[x] = rgb_to_vga8(src[x]);
            }
        }
    }
}
