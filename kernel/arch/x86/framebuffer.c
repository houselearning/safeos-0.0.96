#include "framebuffer.h"

static uint32_t *fb = 0;
static uint32_t fb_width, fb_height;

void framebuffer_init(uint32_t w, uint32_t h, uint32_t bpp) {
    (void)bpp;
    // In a real system, read from multiboot VBE info.
    fb = (uint32_t*)0xE0000000; // example mapped address
    fb_width = w;
    fb_height = h;
}

void framebuffer_clear(uint32_t color) {
    for (uint32_t y = 0; y < fb_height; ++y)
        for (uint32_t x = 0; x < fb_width; ++x)
            fb[y * fb_width + x] = color;
}

void framebuffer_putpixel(uint32_t x, uint32_t y, uint32_t color) {
    if (x >= fb_width || y >= fb_height) return;
    fb[y * fb_width + x] = color;
}
